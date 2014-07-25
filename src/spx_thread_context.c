/*
 * =====================================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  spx_thread_context.c
 *        Created:  2014/07/22 17时19分04秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <ev.h>

#include "include/spx_types.h"
#include "include/spx_list.h"
#include "include/spx_fixed_vector.h"
#include "include/spx_thread_context.h"
#include "include/spx_alloc.h"
#include "include/spx_defs.h"
#include "include/spx_string.h"

struct spx_notice_node{
    SpxNoticeDelegate *notify_handler;
    int event;
    SpxLogDelegate *log;
};

struct spx_thread_listen_node{
    size_t idx;
    struct spx_notice_context *nc;
};


spx_private void *spx_thread_context_new(size_t idx,void *arg,err_t *err);
spx_private err_t spx_thread_context_free(void **arg);
spx_private void *spx_notify_watcher_new(size_t idx,void *arg,err_t *err);
spx_private err_t spx_notify_watcher_free(void **arg);
spx_private void *spx_thread_listen(void *arg);

spx_private void *spx_thread_context_new(size_t idx,void *arg,err_t *err){
    SpxLogDelegate *log = (SpxLogDelegate *) arg;
    struct spx_thread_context *tc = (struct spx_thread_context *) \
                                    spx_alloc_alone(sizeof(*tc),err);
    if(NULL == tc){
        SpxLog2(log,SpxLogError,*err,\
                "alloc thread context is fail.");
        return NULL;
    }
    tc->idx = idx;
    tc->loop = ev_loop_new(EVFLAG_AUTO);
    tc->log = log;
    if(-1 == pipe(tc->pipe)){
        *err = errno;
        ev_loop_destroy(tc->loop);
        SpxFree(tc);
        return NULL;
    }
    return tc;
}
spx_private err_t spx_thread_context_free(void **arg){

    struct spx_thread_context **tc = (struct spx_thread_context **) arg;
    ev_break((*tc)->loop,EVBREAK_ALL);
    ev_loop_destroy((*tc)->loop);
    SpxClose((*tc)->pipe[0]);
    SpxClose((*tc)->pipe[1]);
    SpxFree(*tc);
    return 0;
}

spx_private void *spx_notify_watcher_new(size_t idx,void *arg,err_t *err){
    struct spx_notice_node *nn = (struct spx_notice_node *) arg;
    if(NULL == nn){
        *err = EINVAL;
        return NULL;
    }
    struct spx_notice *n = (struct spx_notice *) spx_alloc_alone(sizeof(*n),err);
    if(NULL == n){
        SpxLog2(nn->log,SpxLogError,*err,\
                "alloc notice is fail.");
        return NULL;
    }
    n->log = nn->log;
    n->idx = idx;
    n->notify_handler = nn->notify_handler;
    return n;
}

spx_private err_t spx_notify_watcher_free(void **arg){
    struct spx_notice **n = (struct spx_notice **) arg;
    SpxFree(*n);
    return 0;
}

spx_private void *spx_thread_listen(void *arg){
    struct spx_thread_listen_node * n = (struct spx_thread_listen_node *) arg;
    size_t idx = n->idx;
    struct spx_notice_context *nc = n->nc;
    SpxFree(n);//free the memory
    struct spx_notice *sn = spx_list_get(nc->receive_notification_watchers,idx);
    struct spx_thread_context *stc = spx_list_get(nc->thread_contexts,idx);
    ev_io_init(&(sn->watcher),sn->notify_handler,stc->pipe[0],EV_READ);
    ev_io_start(stc->loop,&(sn->watcher));
    ev_run(stc->loop,0);
    return NULL;
}

struct spx_notice_context *spx_notice_context_new(\
        SpxLogDelegate *log,\
        u32_t threadsize,\
        size_t stack_size,\
        SpxNoticeDelegate *dispatch_notice_handler,\
        SpxNoticeDelegate *receive_notification_handler,\
        err_t *err){
    struct spx_notice_context *nc = (struct spx_notice_context *) spx_alloc_alone(sizeof(*nc),err);
    if(NULL == nc){
        SpxLog2(log,SpxLogError,*err,\
                "alloc notice context is fail.");
        return NULL;
    }

    nc->log = log;
    nc->thread_contexts = spx_list_init(log,\
            threadsize,\
            spx_thread_context_new,\
            log,\
            spx_thread_context_free,\
            err);
    if(NULL == nc->thread_contexts){
        SpxLog2(log,SpxLogError,*err,\
                "alloc thread contexts is fail.");
        goto r2;
    }

    struct spx_notice_node nn;
    SpxZero(nn);

    nn.log = log;
    nn.event = EV_READ;
    nn.notify_handler = receive_notification_handler;
    nc->receive_notification_watchers = spx_list_init(log,\
            threadsize,\
            spx_notify_watcher_new,\
            &nn,\
            spx_notify_watcher_free,\
            err);
    if(NULL == nc->receive_notification_watchers){
        SpxLog2(log,SpxLogError,*err,\
                "alloc receive notification watchers is fail.");
        goto r2;
    }

    nn.event = EV_WRITE;
    nn.notify_handler = dispatch_notice_handler;
    nc->dispatch_notice_watchers = spx_fixed_vector_new(log,\
            2 * threadsize,\
            spx_notify_watcher_new,\
            &nn,\
            spx_notify_watcher_free,\
            err);
    if(NULL == nc->dispatch_notice_watchers){
        SpxLog2(log,SpxLogError,*err,\
                "alloc dispatch notice watchers is fail.");
        goto r2;
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    size_t ostack_size = 0;
    pthread_attr_getstacksize(&attr, &ostack_size);
    do{
        if (ostack_size != stack_size
                && (0 != (*err = pthread_attr_setstacksize(&attr,stack_size)))){
            SpxLog2(log,SpxLogError,*err,\
                    "set thread stack size is fail.");
            goto r1;
        }
        u32_t i = 0;
        struct spx_thread_listen_node *tln;
        for( ; i < threadsize; i++){
            tln = spx_alloc_alone(sizeof(*tln),err);
            tln->nc = nc;
            tln->idx = i;
            struct spx_thread_context *n = spx_list_get(nc->thread_contexts,i);
            if (0 !=(*err =  pthread_create(&(n->tid), &attr, spx_thread_listen,
                            tln))){
                SpxLog2(log,SpxLogError,*err,\
                        "create nio thread is fail.");
                goto r1;
            }
        }
    }while(false);
    pthread_attr_destroy(&attr);
    return nc;
r1:
    pthread_attr_destroy(&attr);
r2:
    spx_notice_context_free(&nc);
    return NULL;
}

err_t spx_notice_context_free(struct spx_notice_context **nc){
    if(NULL != (*nc)->receive_notification_watchers) {
        spx_list_free(&((*nc)->receive_notification_watchers));
    }
    if(NULL != (*nc)->dispatch_notice_watchers){
        spx_fixed_vector_free(&((*nc)->dispatch_notice_watchers));
    }
    if(NULL != (*nc)->dispatch_notice_watchers){
        spx_list_free(&((*nc)->thread_contexts));
    }
    SpxFree(*nc);
    return 0;
}

err_t spx_dispatch_notice(struct spx_notice_context *nc,size_t idx,void *msg){
    err_t err = 0;
    struct spx_notice *sn = spx_fixed_vector_pop(nc->dispatch_notice_watchers,&err);
    if(NULL == sn){
        SpxLog2(nc->log,SpxLogWarn,err,\
                "pop a dispatch notice watcher is fail.");
        return err;
    }
    struct spx_thread_context *stc = spx_list_get(nc->thread_contexts,idx);
    ev_io_init(&(sn->watcher),sn->notify_handler,stc->pipe[1],EV_WRITE);
    sn->watcher.data = msg;
    ev_io_start(stc->loop,&(sn->watcher));
    ev_run(stc->loop,0);
    return err;
}

/*

void spxreceive_notification_handler(struct ev_loop *loop,ev_io *w,int revents){
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

void spx_dispatch_notice_handler(struct ev_loop *loop,ev_io *w,int revents){
    void *msg = w->data;
    if (sizeof(void *)
            != write(w->fd, msg,
                sizeof(msg) )) {
    }
}
*/
