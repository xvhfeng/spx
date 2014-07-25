/*
 * =====================================================================================
 *
 *       Filename:  spx_sio.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/06/16 14时07分35秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <pthread.h>
#include <ev.h>

#include "include/spx_types.h"
#include "include/spx_defs.h"
#include "include/spx_sio_context.h"
#include "include/spx_nio_context.h"
#include "include/spx_io.h"
#include "include/spx_nio.h"
#include "include/spx_socket.h"

spx_private void *spx_sio_listening(void *arg);
spx_private err_t spx_sio_regedit_reader(struct spx_sio_context *sio_context);


spx_private void *spx_sio_listening(void *arg){
    struct  spx_sio_context *sio_context = (struct spx_sio_context *) arg;
    if (0 != spx_sio_regedit_reader(sio_context)) {
        return NULL ;
    }
    return NULL;
}

spx_private err_t spx_sio_regedit_reader(struct spx_sio_context *sio_context){
    if(NULL == sio_context){
        return EINVAL;
    }
    err_t err = 0;
    if(0 != (err = spx_set_nb(sio_context->pipes[0]))){
        return err;
    }
    ev_io_init(&(sio_context->watcher),sio_context->sio_reader,sio_context->pipes[0],EV_READ);
    sio_context->watcher.data = sio_context;//libev not the set function
    ev_io_start(sio_context->loop,&(sio_context->watcher));
    ev_run(sio_context->loop,0);
    return err;
}

err_t spx_sio_init(SpxLogDelegate *log,\
        size_t size,size_t stack_size,\
        SpxSioDelegate *sio_reader){
    err_t err = 0;

    struct spx_sio_context_pool * sio_context_pool = spx_sio_context_pool_new(log,\
            size,sio_reader,&err);
    if(NULL == sio_context_pool){
        return err;
    }
    size_t i = 0;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    size_t ostack_size = 0;
    pthread_attr_getstacksize(&attr, &ostack_size);
    if (ostack_size != stack_size
            && (0 != (err = pthread_attr_setstacksize(&attr,stack_size)))){
        goto r2;
    }
    for(;i < size;i++){
        struct spx_sio_context *sio_context = spx_sio_context_pool_pop(sio_context_pool,&err);
        if(NULL == sio_context){
            goto r1;
        }
        if (0 !=(err =  pthread_create(&(sio_context->tid), &attr, spx_sio_listening,
                        sio_context))){
            goto r2;
        }
    }
    return 0;
r2:
    pthread_attr_destroy(&attr);
r1:
    spx_sio_context_pool_free(&sio_context_pool);
    return err;
}


void spx_sio_reader(struct ev_loop *loop,ev_io *watcher,int revents){
    int client_sock;
    size_t len = 0;
    struct spx_sio_context *sio_context = NULL;
    sio_context = (struct spx_sio_context *) watcher->data;
    while(true){
        sio_context->err = spx_read_nb(watcher->fd,(byte_t *) &client_sock,sizeof(client_sock) -len,&len);
        if(0 != sio_context->err || len != sizeof(client_sock)){
            return;
        }

        if (0 >= client_sock) {
            return;
        }

        struct spx_nio_context *nio_context =  spx_nio_context_pool_pop(g_spx_nio_context_pool,&(sio_context->err));
        if(NULL == nio_context){
            SpxClose(client_sock);
            goto r1;
        }
        if (0!= (sio_context->err = spx_set_nb(nio_context->fd))) {
            SpxClose(client_sock);
            goto r1;
        }
        nio_context->fd = client_sock;
        nio_context->client_ip = spx_ip_get(client_sock,&(sio_context->err));
        sio_context->err = spx_nio_regedit_reader(client_sock,nio_context);
r1:
        spx_nio_context_pool_push(g_spx_nio_context_pool,nio_context);
    }
    return ;
}
