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
#include "include/spx_io.h"
#include "include/spx_defs.h"
#include "include/spx_nio.h"
#include "include/spx_errno.h"


err_t  spx_nio_regedit_reader(struct spx_nio_context *nio_context){
    if (NULL == nio_context) {
        return EINVAL;
    }

    if (0 >= nio_context->fd) {
        return EINVAL;
    }


    nio_context->lifecycle = SpxNioLifeCycleHeader;
    ev_io_init(&(nio_context->watcher),nio_context->nio_reader,nio_context->fd,EV_READ);
    nio_context->watcher.data = nio_context;//libev not the set function
    ev_io_start(nio_context->loop,&(nio_context->watcher));
    ev_run(nio_context->loop,0);
    return 0;
}

err_t  spx_nio_regedit_writer(struct spx_nio_context *nio_context){
    if (NULL == nio_context) {
        return EINVAL;
    }

    if (0 >= nio_context->fd) {
        return EINVAL;
    }

    nio_context->lifecycle = SpxNioLifeCycleHeader;
    ev_io_init(&(nio_context->watcher),nio_context->nio_writer,nio_context->fd,EV_WRITE);
    nio_context->watcher.data = nio_context;
    ev_io_start(nio_context->loop,&(nio_context->watcher));
    ev_run(nio_context->loop,0);
    return 0;
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
                    "alloc reader header msg is fail.");
            nio_context->err = err;
            return;
        }
        nio_context->reader_header_ctx = ctx;
        err = spx_read_to_msg_nb(watcher->fd,ctx,SpxMsgHeaderSize,&len);
        if(0 != err){
            SpxLogFmt2(nio_context->log,SpxLogError,err,\
                    "read reader header msg is fail.client:%s.",\
                    nio_context->client_ip);
            nio_context->err = err;
            return;
        }
        if(SpxMsgHeaderSize != len){
            SpxLogFmt1(nio_context->log,SpxLogError,\
                    "read reader header msg is fail,read len is %d.",\
                    len);
            nio_context->err = err;
            return;
        }
        struct spx_msg_header *header = spx_msg_to_header(ctx,&err);
        if(NULL == header){
            SpxLog2(nio_context->log,SpxLogError,err,\
                    "parser msg to header is fail.");
            nio_context->err = err;
            return;
        }
        nio_context->reader_header = header;
        nio_context->lifecycle = SpxNioLifeCycleBody;
    }

    if(NULL != nio_context->reader_header_validator \
            && !nio_context->reader_header_validator(nio_context)){
        SpxLogFmt1(nio_context->log,SpxLogError,\
                "validate header is fail.client:%s.",\
                nio_context->client_ip);
        // must send msg to client
        nio_context->err = EBADHEADER;
        if(NULL != nio_context->reader_header_validator_fail){
            nio_context->reader_header_validator_fail(nio_context);
        }
        return;
    }

    len = 0;
    if(SpxNioLifeCycleBody == nio_context->lifecycle){
        nio_context->reader_body_process(watcher->fd, nio_context);
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
        struct spx_msg *ctx = spx_header_to_msg(nio_context->writer_header,SpxMsgHeaderSize,&err);
        if(NULL == ctx){
            SpxLog2(nio_context->log,SpxLogError,err,\
                    "writer serial to ctx is fail.");
            nio_context->err = err;
            return;
        }
        nio_context->writer_header_ctx = ctx;
        err =  spx_write_from_msg_nb(watcher->fd,ctx,SpxMsgHeaderSize,&len);
        if(0 != err){
            SpxLogFmt2(nio_context->log,SpxLogError,err,\
                    "write writer header is fail.client:&s.",\
                    nio_context->client_ip);
            nio_context->err = err;
            return;
        }

        if(SpxMsgHeaderSize != len){
            SpxLogFmt1(nio_context->log,SpxLogError,\
                    "write writer header is fail.len:%d.",\
                    len);
            nio_context->err = err;
            return;
        }
        nio_context->lifecycle = SpxNioLifeCycleBody;
    }

    len = 0;
    if(SpxNioLifeCycleBody == nio_context->lifecycle){
        nio_context->writer_body_process(watcher->fd,nio_context);
    }
    return;
}


void spx_nio_writer_body_handler(int fd,struct spx_nio_context *nio_context){

    if(SpxNioLifeCycleBody != nio_context->lifecycle){
        return;
    }
    size_t len = 0;
    nio_context->err =  spx_write_from_msg_nb(fd,nio_context->writer_body_ctx,nio_context->writer_header->bodylen,&len);
    if(0 != nio_context->err){
        SpxLogFmt2(nio_context->log,SpxLogError,nio_context->err,\
                "write  header is fail.client:&s.",\
                nio_context->client_ip);
        return;
    }

    if(SpxMsgHeaderSize != len){
        SpxLogFmt1(nio_context->log,SpxLogError,\
                "write  header is fail.len:%d.",\
                len);
        return;
    }
    nio_context->lifecycle = SpxNioLifeCycleNormal;
    return;
}


void spx_nio_reader_body_handler(int fd,struct spx_nio_context *nio_context){
    struct spx_msg_header *header = nio_context->reader_header;
    struct spx_msg *ctx = spx_msg_new(header->bodylen,&(nio_context->err));
    if(NULL == ctx){
        SpxLogFmt2(nio_context->log,SpxLogError,nio_context->err,\
                "reader body is fail.client:%s.",\
                nio_context->client_ip);
        return;
    }
    nio_context->reader_body_ctx = ctx;
    size_t len = 0;
    nio_context->err = spx_read_to_msg_nb(fd,ctx,header->bodylen,&len);
    if(0 != nio_context->err){
        SpxLogFmt2(nio_context->log,SpxLogError,nio_context->err,\
                "reader body is fail.client:%s.",\
                nio_context->client_ip);
        return;
    }
    if(header->bodylen != len){
        nio_context->err = ENOENT;
        SpxLogFmt2(nio_context->log,SpxLogError,nio_context->err,\
                "reader body is fail.bodylen:%lld,real body len:%lld.",
                header->bodylen,len);
        return;
    }
}
