/*
 * =====================================================================================
 *
 *       Filename:  spx_job_context_.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/06/09 17时42分46秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>

#include "include/spx_defs.h"
#include "include/spx_alloc.h"
#include "include/spx_fixed_vector.h"
#include "include/spx_errno.h"
#include "include/spx_string.h"
#include "include/spx_message.h"
#include "include/spx_job.h"


struct spx_job_pool *g_spx_job_pool = NULL;


void *spx_job_context_new(size_t idx,void *arg,err_t *err){

    struct spx_job_context_transport *jct = (struct spx_job_context_transport *) arg;
    struct spx_job_context *jcontext = NULL;
    jcontext = spx_alloc_alone(sizeof(*jcontext),err);
    if(NULL == jcontext){
        return NULL;
    }

    jcontext->log = jct->log;
    jcontext->idx = idx;
    jcontext->timeout = jct->timeout;
    jcontext->nio_reader = jct->nio_reader;
    jcontext->nio_writer = jct->nio_writer;
    jcontext->reader_body_process = jct->reader_body_process;
    jcontext->reader_header_validator = jct->reader_header_validator;
    jcontext->reader_body_process_before = jct->reader_body_process_before;
    jcontext->writer_body_process = jct->writer_body_process;
    jcontext->reader_header_validator_fail = jct->reader_header_validator_fail;
    jcontext->config = jct->config;
    return jcontext;

}

err_t spx_job_context__free(void **arg){
    struct spx_job_context **jcontext = (struct spx_job_context **) arg;
    spx_job_context_clear(*jcontext);
    SpxFree(*jcontext);
    return 0;
}

void spx_job_context__clear(struct spx_job_context *jcontext){

    if(NULL != jcontext->client_ip){
        spx_string_clear(jcontext->client_ip);
    }

    if(NULL != jcontext->reader_header){
        SpxFree(jcontext->reader_header);
    }
    if(NULL != jcontext->reader_header_ctx){
        spx_msg_free(&((jcontext)->reader_header_ctx));
    }
    if(NULL != jcontext->reader_body_ctx){
        spx_msg_free(&((jcontext)->reader_body_ctx));
    }

    if(NULL != (jcontext)->writer_header){
        SpxFree((jcontext)->writer_header);
    }
    if(NULL != (jcontext)->writer_header_ctx){
        spx_msg_free(&((jcontext)->writer_header_ctx));
    }
    if(NULL != (jcontext)->writer_body_ctx){
        spx_msg_free(&((jcontext)->writer_body_ctx));
    }

    if(jcontext->is_sendfile){
        if(0 != jcontext->sendfile_fd){
            SpxClose(jcontext->sendfile_fd);
        }
        jcontext->sendfile_begin = 0;
        jcontext->sendfile_size = 0;
    }
    SpxClose(jcontext->fd);
//    jcontext->lazy_recv_offet = 0;
//    jcontext->lazy_recv_size = 0;
    jcontext->err = 0;
    jcontext->moore = SpxNioMooreNormal;
}

struct spx_job_pool *spx_job_pool_new(SpxLogDelegate *log,\
        void *config,\
        size_t size,u32_t timeout,\
        SpxNioDelegate *nio_reader,\
        SpxNioDelegate *nio_writer,\
        SpxNioHeaderValidatorDelegate *reader_header_validator,\
        SpxNioHeaderValidatorFailDelegate *reader_header_validator_fail,\
        SpxNioBodyProcessBeforeDelegate *reader_body_process_before,\
        SpxNioBodyProcessDelegate *reader_body_process,\
        SpxNioBodyProcessDelegate *writer_body_process,\
        err_t *err){
    if(0 == size){
        *err = EINVAL;
    }
    struct spx_job_pool *pool = NULL;
    pool = spx_alloc_alone(sizeof(*pool),err);
    if(NULL == pool){
        return NULL;
    }

    struct spx_job_context_transport arg;
    SpxZero(arg);
    arg.timeout = timeout;
    arg.nio_reader = nio_reader;
    arg.nio_writer = nio_writer;
    arg.log = log;
    arg.reader_header_validator = reader_header_validator;
    arg.reader_body_process = reader_body_process;
    arg.writer_body_process = writer_body_process;
    arg.reader_header_validator_fail = reader_header_validator_fail;
    arg.reader_body_process_before = reader_body_process_before;
    arg.config = config;

    pool->pool = spx_fixed_vector_new(log,size,\
            spx_job_context_new,\
            &arg,\
            spx_job_context_free,\
            err);

    if(NULL == pool->pool){
        SpxFree(pool);
        return NULL;
    }
    return pool;
}

struct spx_job_context *spx_job_pool_pop(struct spx_job_pool *pool,err_t *err){
    struct spx_job_context *jcontext = spx_fixed_vector_pop(pool->pool,err);
    return jcontext;
}

err_t spx_job_pool_push(struct spx_job_pool *pool,struct spx_job_context *jcontext){
    spx_job_context__clear(jcontext);
    return spx_fixed_vector_push(pool->pool,jcontext);
}

err_t spx_job_pool_free(struct spx_job_pool **pool){
    err_t err = 0;
    err = spx_fixed_vector_free(&((*pool)->pool));
    SpxFree(*pool);
    return err;
}


struct spx_msg_header *spx_msg_to_header(struct spx_msg *ctx,err_t *err){
    struct spx_msg_header *header = NULL;
    if(NULL == ctx){
        *err = EINVAL;
        return NULL;
    }

    header = spx_alloc_alone(sizeof(*header),err);
    if(NULL == header){
        return NULL;
    }
    header->version = spx_msg_unpack_u32(ctx);
    header->protocol = spx_msg_unpack_u32(ctx);
    header->bodylen = spx_msg_unpack_u64(ctx);
    header->offset = spx_msg_unpack_u64(ctx);
    header->err = spx_msg_unpack_u32(ctx);
    return header;
}

struct spx_msg *spx_header_to_msg(struct spx_msg_header *header,size_t len,err_t *err){
    if(NULL == header){
        *err = EINVAL;
        return NULL;
    }
    struct spx_msg *ctx = spx_msg_new(len,err);
    if(NULL == ctx){
        return NULL;
    }
    spx_msg_pack_u32(ctx,header->version);
    spx_msg_pack_u32(ctx,header->protocol);
    spx_msg_pack_u64(ctx,header->bodylen);
    spx_msg_pack_u64(ctx,header->offset);
    spx_msg_pack_u32(ctx,header->err);
    return ctx;
}

