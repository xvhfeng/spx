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

#include "spx_types.h"
#include "spx_io.h"
#include "spx_defs.h"
#include "spx_module.h"
#include "spx_network_module.h"
#include "spx_job.h"
#include "spx_socket.h"
#include "spx_nio.h"

struct spx_module_context *g_spx_network_module = NULL;


void spx_network_module_receive_handler(struct ev_loop *loop,ev_io *w,int revents){
    struct spx_job_context *jcontext = NULL;
    size_t len = 0;
    struct spx_trigger_context *tc = (struct spx_trigger_context *) w;//magic,yeah
    err_t err= 0;
    err = spx_read_nb(w->fd,(byte_t *) &jcontext,sizeof(jcontext),&len);
    if(0 != err || len != sizeof(jcontext)){
        SpxLog2(tc->log,SpxLogError,err,\
                "read the nio context is fail."\
                "forced push jcontext to pool.");
        return;
    }

    if (NULL == jcontext) {
        SpxLog1(tc->log,SpxLogError,
                "read the nio context is fail,"\
                "tne nio context is null."\
                "forced push jcontext to pool.");
        return;
    }

    switch(jcontext->moore){
        case SpxNioMooreRequest:{
                                    err = spx_nio_regedit_reader(loop,jcontext->fd,jcontext);
                                    if(0 != err){
                                        SpxLog2(jcontext->log,SpxLogError,err,\
                                                "regedit read event for request is fail."\
                                                "and forced push jcontext to pool.");
                                        goto r1;
                                    }
                                    break;
                                }
        case SpxNioMooreResponse:{
                                     err = spx_nio_regedit_writer(loop,jcontext->fd,jcontext);
                                     if(0 != err){
                                         SpxLog2(jcontext->log,SpxLogError,err,\
                                                 "regedit write event for resopnse is fail."\
                                                 "and forced push jcontext to pool.");
                                         goto r1;
                                     }
                                     break;
                                 }
        case SpxNioMooreNormal:
        default:
                                 {
                                     SpxLog1(jcontext->log,SpxLogError,\
                                             "the jcontext moore is normal,and no the handler."\
                                             "forced push jcontext to pool.");
                                     goto r1;
                                 }
    }
    return ;
r1:
    spx_job_pool_push(g_spx_job_pool,jcontext);
    return;
}


void spx_network_module_wakeup_handler(struct ev_loop *loop,ev_io *w,int revents){
    err_t err = 0;
    struct spx_job_context *jcontext = (struct spx_job_context *)w->data;
    size_t len = 0;
    struct spx_trigger_context *tc = (struct spx_trigger_context *) w;//magic,yeah
    err = spx_write_nb(w->fd,(byte_t *) jcontext,sizeof(jcontext),&len);
    if (0 != err || sizeof(jcontext) != len) {
        SpxLog1(tc->log,SpxLogError,\
                "wakeup network module is fail.");
        spx_job_pool_push(g_spx_job_pool,jcontext);
    }
    spx_module_dispatch_trigger_push(g_spx_network_module,tc);
}
