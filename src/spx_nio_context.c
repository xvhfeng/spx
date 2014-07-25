/*
 * =====================================================================================
 *
 *       Filename:  spx_nio_context.c
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
#include "include/spx_nio_context.h"
#include "include/spx_string.h"
#include "include/spx_message.h"


struct spx_nio_context_pool *g_spx_nio_context_pool = NULL;


 void *spx_nio_context_new(size_t idx,void *arg,err_t *err){

    struct spx_nio_context_arg *n = (struct spx_nio_context_arg *) arg;
    struct spx_nio_context *nio_context = NULL;
    nio_context = spx_alloc_alone(sizeof(*nio_context),err);
    if(NULL == nio_context){
        return NULL;
    }

    nio_context->log = n->log;
    nio_context->idx = idx;
    nio_context->timeout = n->timeout;
    nio_context->nio_reader = n->nio_reader;
    nio_context->nio_writer = n->nio_writer;
    nio_context->reader_body_process = n->reader_body_process;
    nio_context->reader_header_validator = n->reader_header_validator;
    nio_context->writer_body_process = n->writer_body_process;
    nio_context->reader_header_validator_fail = n->reader_header_validator_fail;
    nio_context->config = n->config;
    return nio_context;

}

err_t spx_nio_context_free(void **arg){
    struct spx_nio_context **nio_context = (struct spx_nio_context **) arg;
    spx_nio_context_clear(*nio_context);
    SpxFree(*nio_context);
    return 0;
}

void spx_nio_context_clear(struct spx_nio_context *nio_context){

    if(NULL != nio_context->client_ip){
        spx_string_clear(nio_context->client_ip);
    }

    if(NULL != nio_context->reader_header){
        SpxFree(nio_context->reader_header);
    }
    if(NULL != nio_context->reader_header_ctx){
        spx_msg_free(&((nio_context)->reader_header_ctx));
    }
    if(NULL != nio_context->reader_body_ctx){
        spx_msg_free(&((nio_context)->reader_body_ctx));
    }

    if(NULL != (nio_context)->writer_header){
        SpxFree((nio_context)->writer_header);
    }
    if(NULL != (nio_context)->writer_header_ctx){
        spx_msg_free(&((nio_context)->writer_header_ctx));
    }
    if(NULL != (nio_context)->writer_body_ctx){
        spx_msg_free(&((nio_context)->writer_body_ctx));
    }

    if(nio_context->is_sendfile){
        if(0 != nio_context->sendfile_fd){
            SpxClose(nio_context->sendfile_fd);
        }
        nio_context->sendfile_begin = 0;
        nio_context->sendfile_size = 0;
    }
    SpxClose(nio_context->fd);
    nio_context->lazy_recv_offet = 0;
    nio_context->lazy_recv_size = 0;
    nio_context->err = 0;
    nio_context->moore = SpxNioMooreNormal;
}

struct spx_nio_context_pool *spx_nio_context_pool_new(SpxLogDelegate *log,\
        void *config,\
        size_t size,u32_t timeout,\
        SpxNioDelegate *nio_reader,\
        SpxNioDelegate *nio_writer,\
        SpxNioHeaderValidatorDelegate *reader_header_validator,\
        SpxNioHeaderValidatorFailDelegate *reader_header_validator_fail,\
        SpxNioBodyProcessDelegate *reader_body_process,\
        SpxNioBodyProcessDelegate *writer_body_process,\
        err_t *err){
    if(0 == size){
        *err = EINVAL;
    }
    struct spx_nio_context_pool *pool = NULL;
    pool = spx_alloc_alone(sizeof(*pool),err);
    if(NULL == pool){
        return NULL;
    }

    struct spx_nio_context_arg arg;
    SpxZero(arg);
    arg.timeout = timeout;
    arg.nio_reader = nio_reader;
    arg.nio_writer = nio_writer;
    arg.log = log;
    arg.reader_header_validator = reader_header_validator;
    arg.reader_body_process = reader_body_process;
    arg.writer_body_process = writer_body_process;
    arg.reader_header_validator_fail = reader_header_validator_fail;
    arg.config = config;

    pool->pool = spx_fixed_vector_new(log,size,\
            spx_nio_context_new,\
            &arg,\
            spx_nio_context_free,\
            err);

    if(NULL == pool->pool){
        SpxFree(pool);
        return NULL;
    }
    return pool;
}

struct spx_nio_context *spx_nio_context_pool_pop(struct spx_nio_context_pool *pool,err_t *err){
    struct spx_nio_context *nio_context = spx_fixed_vector_pop(pool->pool,err);
    return nio_context;
}

err_t spx_nio_context_pool_push(struct spx_nio_context_pool *pool,struct spx_nio_context *nio_context){
    spx_nio_context_clear(nio_context);
    return spx_fixed_vector_push(pool->pool,nio_context);
}

err_t spx_nio_context_pool_free(struct spx_nio_context_pool **pool){
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

