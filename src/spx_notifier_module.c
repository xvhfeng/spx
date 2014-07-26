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
#include "include/spx_module.h"
#include "include/spx_notifier_module.h"
#include "include/spx_network_module.h"
#include "include/spx_socket.h"
#include "include/spx_job.h"

    void spx_notifier_module_receive_handler(struct ev_loop *loop,ev_io *w,int revents){
    int client_sock;
    size_t len = 0;
    struct spx_trigger_context *tc = (struct spx_trigger_context *) w;//magic,yeah
    err_t err= 0;
    while(true){
        err = spx_read_nb(w->fd,(byte_t *) &client_sock,sizeof(client_sock),&len);
        if(0 != err || len != sizeof(client_sock)){
            SpxLog2(tc->log,SpxLogError,err,\
                    "read the client socket fd is fail.");
            return;
        }

        if (0 >= client_sock) {
            SpxLogFmt1(tc->log,SpxLogError,
                    "read the client socket fd is fail,"\
                    "the fd is less 0 and value is :%d.",\
                    client_sock);
            return;
        }


        struct spx_job_context *jcontext =  spx_job_pool_pop(g_spx_job_pool,&err);
        if(NULL == jcontext){
            SpxClose(client_sock);
            SpxLog1(tc->log,SpxLogError,\
                    "pop nio context is fail.");
            goto r1;
        }
        jcontext->fd = client_sock;
        jcontext->client_ip = spx_ip_get(client_sock,&err);
        jcontext->moore = SpxNioMooreRequest;
        size_t idx = jcontext->idx % g_spx_network_module->threadpool->curr_size;
        SpxLogFmt1(tc->log,SpxLogDebug,\
                "recv the client:%s connection."\
                "and send to thread:%d to deal.",
                jcontext->client_ip,idx);
        err = spx_module_dispatch(g_spx_network_module,idx,jcontext);
        return;
r1:
        spx_job_pool_push(g_spx_job_pool,jcontext);
    }
    return ;

}

    void spx_notifier_module_wakeup_handler(struct ev_loop *loop,ev_io *w,int revents){
    err_t err = 0;
    int *fd = (int *) w->data;
    size_t len = 0;
    struct spx_trigger_context *tc = (struct spx_trigger_context *) w;//magic,yeah
    err = spx_write_nb(w->fd,(byte_t *) fd,sizeof(fd),&len);
    if (0 != err || sizeof(fd) != len) {
        SpxLog1(tc->log,SpxLogError,\
                "wake up network module is fail.");
        SpxClose(*fd);
    }
 }
