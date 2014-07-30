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
#include "include/spx_job.h"
#include "include/spx_io.h"
#include "include/spx_defs.h"
#include "include/spx_nio.h"
#include "include/spx_errno.h"
#include "include/spx_message.h"
#include "include/spx_alloc.h"


err_t  spx_nio_regedit_reader(struct ev_loop *loop,int fd,struct spx_job_context *jcontext){
    if (NULL == jcontext) {
        return EINVAL;
    }

    if (0 >= fd) {
        return EINVAL;
    }


    jcontext->lifecycle = SpxNioLifeCycleHeader;
    ev_io_init(&(jcontext->watcher),jcontext->nio_reader,fd,EV_READ);
    jcontext->watcher.data = jcontext;//libev not the set function
    ev_io_start(loop,&(jcontext->watcher));
    ev_run(loop,0);
    return 0;
}

err_t  spx_nio_regedit_writer(struct ev_loop *loop,int fd,struct spx_job_context *jcontext){
    if (NULL == jcontext) {
        return EINVAL;
    }

    if (0 >= fd) {
        return EINVAL;
    }

    jcontext->lifecycle = SpxNioLifeCycleHeader;
    ev_io_init(&(jcontext->watcher),jcontext->nio_writer,fd,EV_WRITE);
    jcontext->watcher.data = jcontext;
    ev_io_start(loop,&(jcontext->watcher));
    ev_run(loop,0);
    return 0;
}


void spx_nio_reader(struct ev_loop *loop,ev_io *watcher,int revents){
    if(NULL == loop || NULL == watcher){
        return ;
    }
    size_t len = 0;
    err_t err = 0;
    struct spx_job_context *jcontext =(struct spx_job_context *) watcher->data;
    if(NULL == jcontext){
        return;
    }

    if(SpxNioLifeCycleHeader == jcontext->lifecycle){
        struct spx_msg *ctx = spx_msg_new(SpxMsgHeaderSize,&err);
        if(NULL == ctx){
            SpxLog2(jcontext->log,SpxLogError,err,\
                    "alloc reader header msg is fail.");
            jcontext->err = err;
            goto r1;
        }
        jcontext->reader_header_ctx = ctx;
        err = spx_read_to_msg_nb(watcher->fd,ctx,SpxMsgHeaderSize,&len);
        if(0 != err){
            SpxLogFmt2(jcontext->log,SpxLogError,err,\
                    "read reader header msg is fail.client:%s.",\
                    jcontext->client_ip);
            jcontext->err = err;
            goto r1;
        }
        if(SpxMsgHeaderSize != len){
            SpxLogFmt1(jcontext->log,SpxLogError,\
                    "read reader header msg is fail,read len is %d.",\
                    len);
            jcontext->err = err;
            goto r1;
        }
        struct spx_msg_header *header = spx_msg_to_header(ctx,&err);
        if(NULL == header){
            SpxLog2(jcontext->log,SpxLogError,err,\
                    "parser msg to header is fail.");
            jcontext->err = err;
            goto r1;
        }
        jcontext->reader_header = header;
        jcontext->lifecycle = SpxNioLifeCycleBody;
    }

    if(NULL != jcontext->reader_header_validator \
            && !jcontext->reader_header_validator(jcontext)){
        SpxLogFmt1(jcontext->log,SpxLogError,\
                "validate header is fail.client:%s.",\
                jcontext->client_ip);
        // must send msg to client
        jcontext->err = EBADHEADER;
        if(NULL != jcontext->reader_header_validator_fail){
            jcontext->reader_header_validator_fail(jcontext);
        }
        goto r1;
    }

    len = 0;
    if((SpxNioLifeCycleBody == jcontext->lifecycle) \
            && (0 != jcontext->reader_header->bodylen)){
        jcontext->reader_body_process(watcher->fd, jcontext);
        if(0 != jcontext->err){
            SpxLog2(jcontext->log,SpxLogError,jcontext->err,\
                    "recv the body buffer is fail.");
            goto r1;
        }
    }
    return;
r1://when progame is error
    jcontext->writer_header =(struct spx_msg_header *)\
                             spx_alloc_alone(sizeof(struct spx_msg_header),&(jcontext->err));
    if(NULL == jcontext->writer_header){ // no way to deal the error ,otherside can not send to client
        SpxLog2(jcontext->log,SpxLogError,jcontext->err,\
                "alloc the writer header for error is fail.so can not send error msg to client."\
                "then push jcontext to pool and close the connection.");
        spx_job_pool_push(g_spx_job_pool,jcontext);
        return;
    }
    if(NULL != jcontext->reader_header){
        jcontext->writer_header->protocol = jcontext->reader_header->protocol;
    } else{
        jcontext->writer_header->protocol = 0;
    }
    jcontext->writer_header->err = jcontext->err;
    spx_nio_regedit_writer(loop,watcher->fd,jcontext);
}

void spx_nio_writer(struct ev_loop *loop,ev_io *watcher,int revents){
    if(NULL == loop || NULL == watcher){
        return ;
    }
    size_t len = 0;
    err_t err = 0;
    struct spx_job_context *jcontext =(struct spx_job_context *) watcher->data;
    if(NULL == jcontext){
        return;
    }
    if(SpxNioLifeCycleHeader == jcontext->lifecycle){
        struct spx_msg *ctx = spx_header_to_msg(jcontext->writer_header,SpxMsgHeaderSize,&err);
        if(NULL == ctx){
            SpxLog2(jcontext->log,SpxLogError,err,\
                    "convert writer header to msg ctx is fail."\
                    "and forced push jcontext to pool.");
            jcontext->err = err;
            goto r1;
        }
        jcontext->writer_header_ctx = ctx;
        err =  spx_write_from_msg_nb(watcher->fd,ctx,SpxMsgHeaderSize,&len);
        if(0 != err){
            SpxLogFmt2(jcontext->log,SpxLogError,err,\
                    "write writer header is fail.client:&s."\
                    "and forced push jcontext to pool.",\
                    jcontext->client_ip);
            jcontext->err = err;
            goto r1;
        }

        if(SpxMsgHeaderSize != len){
            SpxLogFmt2(jcontext->log,SpxLogError,err,\
                    "write writer header to client:%s is fail.len:%d."\
                    "and forced push jcontext to pool.",\
                    jcontext->client_ip,len);
            jcontext->err = err;
            goto r1;
        }
        jcontext->lifecycle = SpxNioLifeCycleBody;
    }

    len = 0;
    if((SpxNioLifeCycleBody == jcontext->lifecycle) \
            && (0 != jcontext->writer_header->bodylen)){
        jcontext->writer_body_process(watcher->fd,jcontext);
        if(0 != jcontext->err){
            SpxLogFmt2(jcontext->log,SpxLogError,jcontext->err,\
                    "call write body handler to client:%s is fail."\
                    "and forced push jcontext to pool.",\
                    jcontext->client_ip);
                goto r1;
        }
    }
r1:
    spx_job_pool_push(g_spx_job_pool,jcontext);
}

void spx_nio_reader_body_handler(int fd,struct spx_job_context *jcontext){
    struct spx_msg_header *header = jcontext->reader_header;
    struct spx_msg *ctx = spx_msg_new(header->bodylen,&(jcontext->err));
    if(NULL == ctx){
        SpxLogFmt2(jcontext->log,SpxLogError,jcontext->err,\
                "alloc body buffer is fail.client:%s.",\
                jcontext->client_ip);
        return;
    }
    jcontext->reader_body_ctx = ctx;
    size_t len = 0;
    jcontext->err = spx_read_to_msg_nb(fd,ctx,header->bodylen,&len);
    if(0 != jcontext->err){
        SpxLogFmt2(jcontext->log,SpxLogError,jcontext->err,\
                "reader body buffer is fail.client:%s.",\
                jcontext->client_ip);
        return;
    }
    if(header->bodylen != len){
        jcontext->err = ENOENT;
        SpxLogFmt2(jcontext->log,SpxLogError,jcontext->err,\
                "reader body is fail.bodylen:%lld,real body len:%lld.",
                header->bodylen,len);
        return;
    }
}


void spx_nio_writer_body_handler(int fd,struct spx_job_context *jcontext){
    if(0 == fd || NULL == jcontext){
        return;
    }
    if(SpxNioLifeCycleBody != jcontext->lifecycle){
        SpxLog1(jcontext->log,SpxLogError,\
                "the jcontext lifecycle is not body."\
                "and forced push jcontext to pool.");
        return;
    }
    size_t len = 0;
    jcontext->err =  spx_write_from_msg_nb(fd,jcontext->writer_body_ctx,jcontext->writer_header->bodylen,&len);
    if(0 != jcontext->err){
        SpxLogFmt2(jcontext->log,SpxLogError,jcontext->err,\
                "write  header is fail.client:&s."\
                "and forced push jcontext to pool.",\
                jcontext->client_ip);
        return;
    }

    if(SpxMsgHeaderSize != len){
        SpxLogFmt1(jcontext->log,SpxLogError,\
                "write  header to client :%s is fail.len:%d."\
                "and forced push jcontext to pool.",\
                jcontext->client_ip,len);
        return;
    }
    jcontext->lifecycle = SpxNioLifeCycleNormal;
    return;
}
