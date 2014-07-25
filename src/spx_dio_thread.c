/*
 * =====================================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  spx_dio_thread.c
 *        Created:  2014/07/24 13时14分46秒
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
#include "include/spx_dio_context.h"

void spx_dio_thread_receive_notification_handler(struct ev_loop *loop,ev_io *w,int revents){
    struct spx_dio_context *dio_context = NULL;
    size_t len = 0;
    struct spx_notice *sn = (struct spx_notice *) w;//magic,yeah
    err_t err= 0;
    err = spx_read_nb(w->fd,(byte_t *) &dio_context,sizeof(dio_context),&len);
    if(0 != err || len != sizeof(dio_context)){
        SpxLog2(sn->log,SpxLogError,err,\
                "read the dio context is fail.");
        return;
    }

    if (NULL == dio_context) {
        SpxLog1(sn->log,SpxLogError,
                "read the dio context is fail,"\
                "tne dio context is null.");
        return;
    }

    //deal io
    if(dio_context->noblacking){
        if(NULL != dio_context->dio_process_handler){
            dio_context->dio_process_handler(0,dio_context->arg);
        }
    }else{
        struct spx_dio_file *dio_file = (struct spx_dio_file *) dio_context->arg;
        ev_io_init(&(dio_context->watcher),dio_context->dio_handler,dio_file->fd,dio_context->events);
        dio_context->watcher.data = dio_context;//libev not the set function
        ev_io_start(loop,&(dio_context->watcher));
        ev_run(loop,0);
    }

    return ;
}


void spx_dispatch_notice_to_dio_thread_handler(struct ev_loop *loop,ev_io *w,int revents){
    err_t err = 0;
    struct spx_dio_context *dio_context = (struct spx_dio_context *)w->data;
    size_t len = 0;
    struct spx_notice *sn = (struct spx_notice *) w;//magic,yeah
    err = spx_write_nb(w->fd,(byte_t *) dio_context,sizeof(dio_context),&len);
    if (0 != err || sizeof(dio_context) != len) {
        SpxLog1(sn->log,SpxLogError,\
                "send dio_context to dio thread is fail."\
                "then dispatch notice to nio thread deal.");
        spx_dio_context_pool_push(g_spx_dio_context_pool,dio_context);
        struct spx_nio_context *nio_context = dio_context->nio_context;
        nio_context->err = err;
        nio_context->moore = SpxNioMooreResponse;
        size_t idx = nio_context->idx % g_spx_nio_thread_context->thread_contexts->curr_size;
        err = spx_dispatch_notice(g_spx_nio_thread_context,idx,nio_context);
    }
}
