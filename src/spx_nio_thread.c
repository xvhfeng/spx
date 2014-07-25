/*
 * =====================================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  spx_nio_threaed.c
 *        Created:  2014/07/24 11时05分58秒
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
#include "include/spx_nio.h"

void spx_nio_thread_receive_notification_handler(struct ev_loop *loop,ev_io *w,int revents){
    struct spx_nio_context *nio_context = NULL;
    size_t len = 0;
    struct spx_notice *sn = (struct spx_notice *) w;//magic,yeah
    err_t err= 0;
    err = spx_read_nb(w->fd,(byte_t *) &nio_context,sizeof(nio_context),&len);
    if(0 != err || len != sizeof(nio_context)){
        SpxLog2(sn->log,SpxLogError,err,\
                "read the nio context is fail.");
        return;
    }

    if (NULL == nio_context) {
        SpxLog1(sn->log,SpxLogError,
                "read the nio context is fail,"\
                "tne nio context is null.");
        return;
    }

    switch(nio_context->moore){
        case SpxNioMooreRequest:{
                                    err = spx_nio_regedit_reader(loop,nio_context->fd,nio_context);
                                    break;
                                }
        case SpxNioMooreResponse:{
                                     err = spx_nio_regedit_writer(loop,nio_context->fd,nio_context);
                                     break;
                                 }
        case SpxNioMooreNormal:
        default:
                                 {
                                     break;
                                 }
    }
    return ;
}


void spx_dispatch_notice_to_nio_thread_handler(struct ev_loop *loop,ev_io *w,int revents){
    err_t err = 0;
    struct spx_nio_context *nio_context = (struct spx_nio_context *)w->data;
    size_t len = 0;
    struct spx_notice *sn = (struct spx_notice *) w;//magic,yeah
    err = spx_write_nb(w->fd,(byte_t *) nio_context,sizeof(nio_context),&len);
    if (0 != err || sizeof(nio_context) != len) {
        SpxLog1(sn->log,SpxLogError,\
                "send client socket to sio thread is fail.");
        spx_nio_context_pool_push(g_spx_nio_context_pool,nio_context);
    }
}
