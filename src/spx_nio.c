/*
 * =====================================================================================
 *
 *       Filename:  spx_nio.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/06/09 17时43分46秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <ev.h>

#include "include/spx_types.h"
#include "include/spx_nio_context.h"
#include "include/spx_socket.h"
#include "include/spx_io.h"
#include "include/spx_defs.h"
#include "include/spx_nio.h"

err_t  spx_nio_regedit_reader(struct spx_nio_context *nio_context){
    if (NULL == nio_context) {
        return EINVAL;
    }

    err_t err = 0;
    if (0!= (err = spx_socket_nb(nio_context->watcher.fd))) {
        return err;
    }

    nio_context->lifecycle = SpxNioLifeCycleHeader;
    ev_io_init(&(nio_context->watcher),nio_context->nio_reader,nio_context->watcher.fd,EV_READ);
    nio_context->watcher.data = nio_context;//libev not the set function
    ev_io_start(nio_context->loop,&(nio_context->watcher));
    ev_run(nio_context->loop,0);
    return err;
}

err_t  spx_nio_regedit_writer(struct spx_nio_context *nio_context){
    if (NULL == nio_context) {
        return EINVAL;
    }

    if (0 >= nio_context->watcher.fd) {
        return EINVAL;
    }

    err_t err = 0;
    if (0!= (err = spx_socket_nb(nio_context->watcher.fd))) {
        return err;
    }

    nio_context->lifecycle = SpxNioLifeCycleHeader;
    ev_io_init(&(nio_context->watcher),nio_context->nio_writer,nio_context->watcher.fd,EV_WRITE);
    nio_context->watcher.data = nio_context;
    ev_io_start(nio_context->loop,&(nio_context->watcher));
    ev_run(nio_context->loop,0);
    return err;
}


void spx_nio_reader(struct ev_loop *loop,ev_io *watcher,int revents){
    if(NULL == loop || NULL == watcher){
        return ;
    }
    size_t len = 0;
    err_t err = 0;
    struct spx_nio_context *nio_context =(struct spx_nio_context *) watcher->data;
    ev_io_stop(nio_context->loop,watcher);
    if(SpxNioLifeCycleHeader == nio_context->lifecycle){
        struct spx_msg *ctx = spx_msg_new(SpxMsgHeaderSize,&err);
        if(NULL == ctx){
            SpxLog2(nio_context->log,SpxLogError,err,\
                    "alloc request header msg is fail.");
            nio_context->err = err;
            spx_nio_context_pool_push(g_spx_nio_context_pool,nio_context);
            return;
        }
        nio_context->request_header_ctx = ctx;
        err = spx_read_to_msg_nb(watcher->fd,ctx,SpxMsgHeaderSize,&len);
        if(0 != err){
            SpxLogFmt2(nio_context->log,SpxLogError,err,\
                    "read request header msg is fail.client:%s.",\
                    nio_context->client_ip);
            nio_context->err = err;
            spx_nio_context_pool_push(g_spx_nio_context_pool,nio_context);
            return;
        }
        if(SpxMsgHeaderSize != len){
            SpxLogFmt1(nio_context->log,SpxLogError,\
                    "read request header msg is fail,read len is %d.",\
                    len);
            nio_context->err = err;
            spx_nio_context_pool_push(g_spx_nio_context_pool,nio_context);
            return;
        }
        struct spx_msg_header *header = spx_msg_to_header(ctx,&err);
        if(NULL == header){
            SpxLog2(nio_context->log,SpxLogError,err,\
                    "parser msg to header is fail.");
            nio_context->err = err;
            spx_nio_context_pool_push(g_spx_nio_context_pool,nio_context);
            return;
        }
        nio_context->request_header = header;
        nio_context->lifecycle = SpxNioLifeCycleBody;
    }

    if(NULL != nio_context->request_header_validator \
            && !nio_context->request_header_validator(nio_context)){
        SpxLogFmt1(nio_context->log,SpxLogError,\
                "validate header is fail.client:%s.",\
                nio_context->client_ip);
        // must send msg to client
        spx_nio_regedit_writer(nio_context);
        return;
    }

    len = 0;
    if(SpxNioLifeCycleBody == nio_context->lifecycle){
        nio_context->request_body_process(nio_context);
    }
    return;
}

void spx_nio_writer(struct ev_loop *loop,ev_io *watcher,int revents){
    if(NULL == loop || NULL == watcher){
        return ;
    }
    size_t len = 0;
    err_t err = 0;
    struct spx_nio_context *nio_context =(struct spx_nio_context *) watcher->data;
    ev_io_stop(nio_context->loop,watcher);
    if(SpxNioLifeCycleHeader == nio_context->lifecycle){
        struct spx_msg *ctx = spx_header_to_msg(nio_context->response_header,SpxMsgHeaderSize,&err);
        if(NULL == ctx){
            SpxLog2(nio_context->log,SpxLogError,err,\
                    "response serial to ctx is fail.");
            nio_context->err = err;
            spx_nio_context_pool_push(g_spx_nio_context_pool,nio_context);
            return;
        }
        nio_context->response_header_ctx = ctx;
        err =  spx_write_from_msg_nb(watcher->fd,ctx,SpxMsgHeaderSize,&len);
        if(0 != err){
            SpxLogFmt2(nio_context->log,SpxLogError,err,\
                    "write response header is fail.client:&s.",\
                    nio_context->client_ip);
            nio_context->err = err;
            spx_nio_context_pool_push(g_spx_nio_context_pool,nio_context);
            return;
        }

        if(SpxMsgHeaderSize != len){
            SpxLogFmt1(nio_context->log,SpxLogError,\
                    "write response header is fail.len:%d.",\
                    len);
            nio_context->err = err;
            spx_nio_context_pool_push(g_spx_nio_context_pool,nio_context);
            return;
        }
        nio_context->lifecycle = SpxNioLifeCycleBody;
    }

    len = 0;
    if(SpxNioLifeCycleBody == nio_context->lifecycle){
        nio_context->response_body_process(nio_context);
    }
    return;
}
