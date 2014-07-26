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
#include "include/spx_module.h"
#include "include/spx_network_module.h"
#include "include/spx_job.h"
#include "include/spx_socket.h"
#include "include/spx_nio.h"
#include "include/spx_task.h"

 struct spx_module *g_spx_task_module = NULL;
void spx_task_module_receive_handler(struct ev_loop *loop,ev_io *w,int revents){
    struct spx_task_context *tcontext = NULL;
    size_t len = 0;
    struct spx_trigger_context *tc = (struct spx_trigger_context *) w;//magic,yeah
    err_t err= 0;
    err = spx_read_nb(w->fd,(byte_t *) &tcontext,sizeof(tcontext),&len);
    if(0 != err || len != sizeof(tcontext)){
        SpxLog2(tc->log,SpxLogError,err,\
                "read the dio context is fail.");
        return;
    }

    if (NULL == tcontext) {
        SpxLog1(tc->log,SpxLogError,
                "read the dio context is fail,"\
                "tne dio context is null.");
        return;
    }

    //deal io
    if(tcontext->noblacking){
        if(NULL != tcontext->dio_process_handler){
            tcontext->dio_process_handler(0,tcontext->arg);
        }
    }else{
        struct spx_dio_file *dio_file = (struct spx_dio_file *) tcontext->arg;
        ev_io_init(&(tcontext->watcher),tcontext->dio_handler,dio_file->fd,tcontext->events);
        tcontext->watcher.data = tcontext;//libev not the set function
        ev_io_start(loop,&(tcontext->watcher));
        ev_run(loop,0);
    }

    return ;
}


void spx_task_module_wakeup_handler(struct ev_loop *loop,ev_io *w,int revents){
    err_t err = 0;
    struct spx_task_context *tcontext = (struct spx_task_context *)w->data;
    size_t len = 0;
    struct spx_trigger_context *tc = (struct spx_trigger_context *) w;//magic,yeah
    err = spx_write_nb(w->fd,(byte_t *) tcontext,sizeof(tcontext),&len);
    if (0 != err || sizeof(tcontext) != len) {
        SpxLog1(tc->log,SpxLogError,\
                "send tcontext to dio thread is fail."\
                "then dispatch notice to nio thread deal.");
        spx_task_pool_push(g_spx_task_pool,tcontext);
        struct spx_job_context *jcontext = tcontext->jcontext;
        jcontext->err = err;
        jcontext->moore = SpxNioMooreResponse;
        size_t idx = jcontext->idx % g_spx_network_module->threadpool->curr_size;
        err = spx_module_dispatch(g_spx_network_module,idx,jcontext);
    }
}
