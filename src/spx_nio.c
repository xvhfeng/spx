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

#include "spx_types.h"
#include "spx_job.h"
#include "spx_io.h"
#include "spx_defs.h"
#include "spx_nio.h"
#include "spx_errno.h"
#include "spx_message.h"
#include "spx_alloc.h"
#include "spx_task.h"


err_t  spx_nio_regedit_reader(struct ev_loop *loop,int fd,struct spx_job_context *jcontext){/*{{{*/
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
    return 0;
}/*}}}*/

err_t  spx_nio_regedit_writer(struct ev_loop *loop,int fd,struct spx_job_context *jcontext){/*{{{*/
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
    return 0;
}/*}}}*/

err_t  spx_dio_regedit_reader(struct ev_loop *loop,int fd,ev_io *watcher,
        SpxNioDelegate *dio_reader,void *data){/*{{{*/
    ev_io_init(watcher,dio_reader,fd,EV_READ);
    watcher->data = data;//libev not the set function
    ev_io_start(loop,watcher);
    return 0;
}/*}}}*/

err_t  spx_dio_regedit_writer(struct ev_loop *loop,int fd,ev_io *watcher,
        SpxNioDelegate *dio_writer,void *data){/*{{{*/
    ev_io_init(watcher,dio_writer,fd,EV_WRITE);
    watcher->data = data;
    ev_io_start(loop,watcher);
    return 0;
}/*}}}*/

err_t  spx_dio_regedit_async(ev_async *w,
        SpxAsyncDelegate *reader,void *data){/*{{{*/
    ev_async_init(w,reader);
    w->data = data;
    return 0;
}/*}}}*/

void spx_nio_reader(struct ev_loop *loop,ev_io *watcher,int revents){/*{{{*/
    if(NULL == loop || NULL == watcher){
        return ;
    }
    ev_io_stop(loop,watcher);
    size_t len = 0;
    err_t err = 0;
    struct spx_job_context *jcontext =(struct spx_job_context *) watcher->data;
    if(NULL == jcontext){
        return;
    }

    if(SpxNioLifeCycleHeader == jcontext->lifecycle){
        struct spx_msg *ctx = spx_msg_new(SpxMsgHeaderSize,&err);
        if(NULL == ctx){
            jcontext->err = err;
            SpxLog2(jcontext->log,SpxLogError,err,\
                    "alloc reader header msg is fail.");
            goto r1;
        }
        jcontext->reader_header_ctx = ctx;
        err = spx_read_to_msg_nb(watcher->fd,ctx,SpxMsgHeaderSize,&len);
        if(0 != err){
            jcontext->err = err;
            SpxLogFmt2(jcontext->log,SpxLogError,err,\
                    "read reader header msg is fail.client:%s.",\
                    jcontext->client_ip);
            goto r1;
        }
        if(SpxMsgHeaderSize != len){
            jcontext->err = 0 == err ? EIO : err;
            SpxLogFmt1(jcontext->log,SpxLogError,\
                    "read reader header msg is fail,read len is %d.",\
                    len);
            goto r1;
        }
        spx_msg_seek(ctx,0,SpxMsgSeekSet);
        struct spx_msg_header *header = spx_msg_to_header(ctx,&err);
        if(NULL == header){
            jcontext->err = err;
            SpxLog2(jcontext->log,SpxLogError,err,\
                    "parser msg to header is fail.");
            goto r1;
        }
        jcontext->reader_header = header;
        jcontext->lifecycle = SpxNioLifeCycleBody;
    }

    if(NULL != jcontext->reader_header_validator \
            && !jcontext->reader_header_validator(jcontext)){
        // must send msg to client
        jcontext->err = EBADHEADER;
        SpxLogFmt1(jcontext->log,SpxLogError,\
                "validate header is fail.client:%s.",\
                jcontext->client_ip);
        if(NULL != jcontext->reader_header_validator_fail){
            jcontext->reader_header_validator_fail(jcontext);
        }
        goto r1;
    }

    if(NULL != jcontext->reader_body_process_before){
        jcontext->reader_body_process_before(jcontext);
    }

    len = 0;
    if((SpxNioLifeCycleBody == jcontext->lifecycle) \
            && (0 != jcontext->reader_header->bodylen)){
        jcontext->reader_body_process(loop,watcher->fd, jcontext);
        if(0 != jcontext->err){
            SpxLog2(jcontext->log,SpxLogError,jcontext->err,\
                    "recv the body buffer is fail.");
            goto r1;
        }
    }
    return;
r1:
        spx_job_pool_push(g_spx_job_pool,jcontext);
}/*}}}*/

void spx_nio_writer(struct ev_loop *loop,ev_io *watcher,int revents){/*{{{*/
    if(NULL == loop || NULL == watcher){
        return ;
    }
    ev_io_stop(loop,watcher);
    size_t len = 0;
    err_t err = 0;
    struct spx_job_context *jcontext =(struct spx_job_context *) watcher->data;
    if(NULL == jcontext){
        return;
    }
    if(SpxNioLifeCycleHeader == jcontext->lifecycle){
        struct spx_msg *ctx = spx_header_to_msg(jcontext->writer_header,SpxMsgHeaderSize,&err);
        if(NULL == ctx){
            jcontext->err = err;
            SpxLog2(jcontext->log,SpxLogError,err,\
                    "convert writer header to msg ctx is fail."\
                    "and forced push jcontext to pool.");
            goto r1;
        }
        jcontext->writer_header_ctx = ctx;
        err =  spx_write_from_msg_nb(watcher->fd,ctx,SpxMsgHeaderSize,&len);
        if(0 != err){
            jcontext->err = err;
            SpxLogFmt2(jcontext->log,SpxLogError,err,\
                    "write writer header is fail.client:&s."\
                    "and forced push jcontext to pool.",\
                    jcontext->client_ip);

            goto r1;
        }

        if(SpxMsgHeaderSize != len){
            jcontext->err = EIO;
            SpxLogFmt2(jcontext->log,SpxLogError,err,\
                    "write writer header to client:%s is fail.len:%d."\
                    "and forced push jcontext to pool.",\
                    jcontext->client_ip,len);
            goto r1;
        }
        jcontext->lifecycle = SpxNioLifeCycleBody;
    }

    len = 0;
    if((SpxNioLifeCycleBody == jcontext->lifecycle) \
            && (0 != jcontext->writer_header->bodylen)){
        jcontext->writer_body_process(loop,watcher->fd,jcontext);
        if(0 != jcontext->err){
            SpxLogFmt2(jcontext->log,SpxLogError,jcontext->err,\
                    "call write body handler to client:%s is fail."\
                    "and forced push jcontext to pool.",\
                    jcontext->client_ip);
            goto r1;
        }
    }
    return;
r1:
    spx_job_pool_push(g_spx_job_pool,jcontext);

}/*}}}*/

void spx_nio_reader_body_handler(struct ev_loop *loop,int fd,struct spx_job_context *jcontext){/*{{{*/
    struct spx_msg_header *header = jcontext->reader_header;
    size_t len = 0;
    if(jcontext->is_lazy_recv){
        struct spx_msg *ctx = spx_msg_new(header->offset,&(jcontext->err));
        if(NULL == ctx){
            SpxLogFmt2(jcontext->log,SpxLogError,jcontext->err,\
                    "alloc body buffer is fail.client:%s.",\
                    jcontext->client_ip);
            goto r1;
        }
        jcontext->reader_body_ctx = ctx;
        jcontext->err = spx_read_to_msg_nb(fd,ctx,header->offset,&len);
        if(header->offset != len){
            jcontext->err = ENOENT;
            SpxLogFmt2(jcontext->log,SpxLogError,jcontext->err,\
                    "reader body is fail.bodylen:%lld,real body len:%lld.",
                    header->offset,len);
            goto r1;
        }
    } else {
        struct spx_msg *ctx = spx_msg_new(header->bodylen,&(jcontext->err));
        if(NULL == ctx){
            SpxLogFmt2(jcontext->log,SpxLogError,jcontext->err,\
                    "alloc body buffer is fail.client:%s.",\
                    jcontext->client_ip);
            goto r1;
        }
        jcontext->reader_body_ctx = ctx;
        jcontext->err = spx_read_to_msg_nb(fd,ctx,header->bodylen,&len);
        if(header->bodylen != len){
            jcontext->err = ENOENT;
            SpxLogFmt2(jcontext->log,SpxLogError,jcontext->err,\
                    "reader body is fail.bodylen:%lld,real body len:%lld.",
                    header->bodylen,len);
            goto r1;
        }
    }
    return;
r1:
    spx_job_pool_push(g_spx_job_pool,jcontext);
}/*}}}*/

void spx_nio_writer_body_handler(struct ev_loop *loop,int fd,struct spx_job_context *jcontext){/*{{{*/
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
    if(jcontext->is_sendfile){
        if(NULL != jcontext->writer_body_ctx && 0 != jcontext->writer_header->offset) {//no metedata
            jcontext->err =  spx_write_from_msg_nb(fd,jcontext->writer_body_ctx,\
                    jcontext->writer_header->offset,&len);
            if(0 != jcontext->err || jcontext->writer_header->offset != len){
                SpxLogFmt2(jcontext->log,SpxLogError,jcontext->err,\
                        "write  header to client :%s is fail.len:%d."\
                        "and forced push jcontext to pool.",\
                        jcontext->client_ip,len);
                goto r1;
            }
        }
        size_t sendbytes = 0;
        jcontext->err = spx_sendfile(jcontext->fd,jcontext->sendfile_fd,\
                jcontext->sendfile_begin,jcontext->sendfile_size,&sendbytes);
        if(0 != jcontext->err || jcontext->sendfile_size != sendbytes){
            SpxLogFmt2(jcontext->log,SpxLogError,jcontext->err,\
                    "sndfile size:%lld is no equal sendbytes:%lld."
                    "and forced push jcontext to pool.",
                    jcontext->sendfile_size,sendbytes);
            goto r1;
        }
    }else {
        jcontext->err =  spx_write_from_msg_nb(fd,jcontext->writer_body_ctx,\
                jcontext->writer_header->bodylen,&len);
        if(0 != jcontext->err || jcontext->writer_header->bodylen != len){
            SpxLogFmt2(jcontext->log,SpxLogError,jcontext->err,\
                    "write  header to client :%s is fail.len:%d."\
                    "and forced push jcontext to pool.",\
                    jcontext->client_ip,len);
            goto r1;
        }
    }
    jcontext->lifecycle = SpxNioLifeCycleNormal;
    return;
r1:
    spx_job_pool_push(g_spx_job_pool,jcontext);
}/*}}}*/



//notice :
//this two faster function is not the best
//int thr version 2
//please open the errno(EAGIN and all of retry) to return
//and please regedit writer to loop when errno == eagin
//so faster means try direct write first if write is error,then use loop
//in the version 1,we just deal with error and over the write

void spx_nio_writer_faster(struct ev_loop *loop,int fd,struct spx_job_context *jc){/*{{{*/
    size_t len = 0;
    err_t err = 0;
    if(NULL == jc){
        return;
    }
    if(SpxNioLifeCycleHeader == jc->lifecycle){
        struct spx_msg *ctx = spx_header_to_msg(jc->writer_header,SpxMsgHeaderSize,&err);
        if(NULL == ctx){
            jc->err = err;
            SpxLog2(jc->log,SpxLogError,err,\
                    "convert writer header to msg ctx is fail."\
                    "and forced push jc to pool.");
            goto r1;
        }
        jc->writer_header_ctx = ctx;
        err =  spx_write_from_msg_nb(fd,ctx,SpxMsgHeaderSize,&len);
        if(0 != err){
            jc->err = err;
            SpxLogFmt2(jc->log,SpxLogError,err,\
                    "write writer header is fail.client:&s."\
                    "and forced push jc to pool.",\
                    jc->client_ip);
            goto r1;
        }

        if(SpxMsgHeaderSize != len){
            jc->err = EIO;
            SpxLogFmt2(jc->log,SpxLogError,err,\
                    "write writer header to client:%s is fail.len:%d."\
                    "and forced push jc to pool.",\
                    jc->client_ip,len);
            goto r1;
        }
        jc->lifecycle = SpxNioLifeCycleBody;
    }

    len = 0;
    if((SpxNioLifeCycleBody == jc->lifecycle) \
            && (0 != jc->writer_header->bodylen)){
        jc->writer_body_process(loop,fd,jc);
        if(0 != jc->err){
            SpxLogFmt2(jc->log,SpxLogError,jc->err,\
                    "call write body handler to client:%s is fail."\
                    "and forced push jc to pool.",\
                    jc->client_ip);
            goto r1;
        }
    }
    return;
r1:
    spx_job_pool_push(g_spx_job_pool,jc);

}/*}}}*/

void spx_nio_writer_body_faster_handler(struct ev_loop *loop,int fd,struct spx_job_context *jc){/*{{{*/
    if(0 == fd || NULL == jc){
        return;
    }
    if(SpxNioLifeCycleBody != jc->lifecycle){
        SpxLog1(jc->log,SpxLogError,\
                "the jc lifecycle is not body."\
                "and forced push jc to pool.");
        goto r1;
    }
    size_t len = 0;
    if(jc->is_sendfile){
        jc->err =  spx_write_from_msg_nb(fd,jc->writer_body_ctx,\
                jc->writer_header->offset,&len);
        if(0 != jc->err || jc->writer_header->offset != len){
            SpxLogFmt2(jc->log,SpxLogError,jc->err,\
                    "write  header to client :%s is fail.len:%d."\
                    "and forced push jc to pool.",\
                    jc->client_ip,len);
            goto r1;
        }
        size_t sendbytes = 0;
        jc->err = spx_sendfile(jc->fd,jc->sendfile_fd,\
                jc->sendfile_begin,jc->sendfile_size,&sendbytes);
        if(0 != jc->err || jc->sendfile_size != sendbytes){
            SpxLogFmt2(jc->log,SpxLogError,jc->err,\
                    "sndfile size:%lld is no equal sendbytes:%lld."
                    "and forced push jc to pool.",\
                    jc->sendfile_size,sendbytes);
            goto r1;
        }
    }else {
        jc->err =  spx_write_from_msg_nb(fd,jc->writer_body_ctx,\
                jc->writer_header->bodylen,&len);
        if(0 != jc->err || jc->writer_header->bodylen != len){
            SpxLogFmt2(jc->log,SpxLogError,jc->err,\
                    "write  header to client :%s is fail.len:%d."\
                    "and forced push jc to pool.",\
                    jc->client_ip,len);
            goto r1;
        }
    }
    jc->lifecycle = SpxNioLifeCycleNormal;
    return;
r1:
    spx_job_pool_push(g_spx_job_pool,jc);
}/*}}}*/

