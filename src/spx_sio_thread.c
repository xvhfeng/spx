/*
 * =====================================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  spx_sio_thread.c
 *        Created:  2014/07/23 15时31分32秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <ev.h>

#include "include/spx_types.h"
#include "include/spx_io.h"
#include "include/spx_defs.h"
#include "include/spx_thread_context.h"
#include "include/spx_nio_thread.h"
#include "include/spx_nio_context.h"
#include "include/spx_socket.h"

void spx_sio_thread_receive_notification_handler(struct ev_loop *loop,ev_io *w,int revents){
    int client_sock;
    size_t len = 0;
    struct spx_notice *sn = (struct spx_notice *) w;//magic,yeah
    err_t err= 0;
    while(true){
        err = spx_read_nb(w->fd,(byte_t *) &client_sock,sizeof(client_sock),&len);
        if(0 != err || len != sizeof(client_sock)){
            SpxLog2(sn->log,SpxLogError,err,\
                    "read the client socket fd is fail.");
            return;
        }

        if (0 >= client_sock) {
            SpxLogFmt1(sn->log,SpxLogError,
                    "read the client socket fd is fail,"\
                    "the fd is less 0 and value is :%d.",\
                    client_sock);
            return;
        }


        struct spx_nio_context *nio_context =  spx_nio_context_pool_pop(g_spx_nio_context_pool,&err);
        if(NULL == nio_context){
            SpxClose(client_sock);
            SpxLog1(sn->log,SpxLogError,\
                    "pop nio context is fail.");
            goto r1;
        }
        nio_context->fd = client_sock;
        nio_context->client_ip = spx_ip_get(client_sock,&err);
        nio_context->moore = SpxNioMooreRequest;
        size_t idx = nio_context->idx % g_spx_nio_thread_context->thread_contexts->curr_size;
        SpxLogFmt1(sn->log,SpxLogDebug,\
                "recv the client:%s connection."\
                "and send to thread:%d to deal.",
                nio_context->client_ip,idx);
        err = spx_dispatch_notice(g_spx_nio_thread_context,idx,nio_context);
        return;
r1:
        spx_nio_context_pool_push(g_spx_nio_context_pool,nio_context);
    }
    return ;

}

void spx_dispatch_notice_to_sio_thread_handler(struct ev_loop *loop,ev_io *w,int revents){
    err_t err = 0;
    int *fd = (int *) w->data;
    size_t len = 0;
    struct spx_notice *sn = (struct spx_notice *) w;//magic,yeah
    err = spx_write_nb(w->fd,(byte_t *) fd,sizeof(fd),&len);
    if (0 != err || sizeof(fd) != len) {
        SpxLog1(sn->log,SpxLogError,\
                "send client socket to sio thread is fail.");
        SpxClose(*fd);
    }
 }
