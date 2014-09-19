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

#include "spx_types.h"
#include "spx_io.h"
#include "spx_defs.h"
#include "spx_module.h"
#include "spx_notifier_module.h"
#include "spx_network_module.h"
#include "spx_socket.h"
#include "spx_job.h"

struct spx_module_context *g_spx_notifier_module = NULL;

void spx_notifier_module_receive_handler(struct ev_loop *loop,ev_io *w,int revents){
    struct spx_job_context *jc = NULL;

    size_t len = 0;
    struct spx_trigger_context *tc = (struct spx_trigger_context *) w;//magic,yeah
    err_t err= 0;
        err = spx_read_nb(w->fd,(byte_t *) &jc,sizeof(jc),&len);
        if(NULL == jc){
            SpxLog2(tc->log,SpxLogError,err,
                    "recv job context is fail.");
            return;
        }

        jc->client_ip = spx_ip_get(jc->fd,&err);
        jc->moore = SpxNioMooreRequest;
        size_t idx = jc->idx % g_spx_network_module->threadpool->size;
        SpxLogFmt1(tc->log,SpxLogDebug,\
                "recv the client:%s connection."\
                "and send to thread:%d to deal.",
                jc->client_ip,idx);
        err = spx_module_dispatch(g_spx_network_module,idx,jc);
        if(0 != err){
            SpxLog2(jc->log,SpxLogError,err,\
                    "dispatch to network module is fail."\
                    "and forced push jc to pool.");
            spx_job_pool_push(g_spx_job_pool,jc);
        }
    return ;

}

void spx_notifier_module_wakeup_handler(struct ev_loop *loop,ev_io *w,int revents){
    err_t err = 0;
    struct spx_job_context *jc = (struct spx_job_context *) w->data;
    size_t len = 0;
    struct spx_trigger_context *tc = (struct spx_trigger_context *) w;//magic,yeah
    err = spx_write_nb(w->fd,(byte_t *) &jc,sizeof(jc),&len);
    if (0 != err || sizeof(jc) != len) {
        spx_job_pool_push(g_spx_job_pool,jc);
        SpxLog1(tc->log,SpxLogError,\
                "wake up network module is fail.");
    }
    spx_module_dispatch_trigger_push(g_spx_notifier_module,tc);
    ev_io_stop(loop,w);
}
