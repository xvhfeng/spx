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

#include "spx_types.h"
#include "spx_io.h"
#include "spx_defs.h"
#include "spx_module.h"
#include "spx_network_module.h"
#include "spx_job.h"
#include "spx_socket.h"
#include "spx_nio.h"
#include "spx_task.h"

 struct spx_module_context *g_spx_task_module = NULL;

void spx_task_module_receive_handler(struct ev_loop *loop,ev_io *w,int revents){
    struct spx_task_context *tcontext = NULL;
    struct spx_receive_context *tc = (struct spx_receive_context *) w;//magic,yeah
    size_t len = 0;
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

    tcontext->dio_process_handler(loop,tc->idx,tcontext);
    /*  here no deal the error from dio_process_handler
     *  as we donot know deal flow whether use noblacking or blacking
     *  so, we must deal the error in the handler by yourself
    if(0 != err){
        struct spx_job_context *jcontext = tcontext->jcontext;
        SpxLog2(tc->log,SpxLogError,err,\
                "deal dio process handler is fail.");
        jcontext->err = err;
        spx_task_pool_push(g_spx_task_pool,tcontext);
        jcontext->moore = SpxNioMooreResponse;
        size_t idx = jcontext->idx % g_spx_network_module->threadpool->curr_size;
        err = spx_module_dispatch(g_spx_network_module,idx,jcontext);
    }
    */

    return ;
}


//void spx_task_module_wakeup_handler(struct ev_loop *loop,ev_io *w,int revents){
void spx_task_module_wakeup_handler(int revents,void *arg){
    //    ev_io_stop(loop,w);
    //    err_t err = 0;
    //    struct spx_task_context *tcontext = (struct spx_task_context *)w->data;
    //    size_t len = 0;
    //    struct spx_trigger_context *tc = (struct spx_trigger_context *) w;//magic,yeah
    //    struct spx_job_context *jcontext = tcontext->jcontext;
    //    err = spx_write_nb(w->fd,(byte_t *) &tcontext,sizeof(tcontext),&len);
    //    if (0 != err || sizeof(tcontext) != len) {
    //        SpxLog1(tc->log,SpxLogError,
    //                "send tcontext to dio thread is fail."
    //                "then dispatch notice to nio thread deal.");
    //        spx_task_pool_push(g_spx_task_pool,tcontext);
    //        jcontext->err = err;
    //        jcontext->moore = SpxNioMooreResponse;
    //        size_t idx = spx_network_module_wakeup_idx(jcontext);
    //        err = spx_module_dispatch(g_spx_network_module,idx,jcontext);
    //    }
    //    spx_module_dispatch_trigger_push(g_spx_task_module,tc);

    err_t err = 0;
    struct spx_task_context *tcontext = (struct spx_task_context *) arg;
    struct spx_job_context *jc = tcontext->jcontext;
    if((revents & EV_TIMEOUT) || (revents & EV_ERROR)){
        SpxLog1(tcontext->log,SpxLogError,
                "wakeup task thread is fail.");
        spx_task_pool_push(g_spx_task_pool,tcontext);
        spx_job_pool_push(g_spx_job_pool,jc);
    }
    if(revents & EV_WRITE){
        SpxLogFmt1(jc->log,SpxLogDebug,"task is well.tc idx:%d",tcontext->idx);
        size_t len = 0;
        err = spx_write_nb(jc->tc->pipe[1],(byte_t *) &tcontext,sizeof(tcontext),&len);
        if (0 != err || sizeof(jc) != len) {
            SpxLog1(jc->log,SpxLogError,\
                    "send tcontext to dio thread is fail."\
                    "then dispatch notice to nio thread deal.");
            spx_task_pool_push(g_spx_task_pool,tcontext);
            jc->err = err;
            jc->moore = SpxNioMooreResponse;
            size_t idx = spx_network_module_wakeup_idx(jc);
            struct spx_thread_context *tc = spx_get_thread(g_spx_network_module,idx);
            jc->tc = tc;
            err = spx_module_dispatch(tc,spx_network_module_wakeup_handler,jc);
        }
    }
}
