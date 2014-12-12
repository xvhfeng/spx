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

#include "spx_defs.h"
#include "spx_alloc.h"
#include "spx_fixed_vector.h"
#include "spx_errno.h"
#include "spx_string.h"
#include "spx_message.h"
#include "spx_job.h"
#include "spx_time.h"


struct spx_job_pool *g_spx_job_pool = NULL;


void *spx_job_context_new(size_t idx,void *arg,err_t *err){

    struct spx_job_context_transport *jct = (struct spx_job_context_transport *) arg;
    struct spx_job_context *jc = NULL;
    jc = spx_alloc_alone(sizeof(*jc),err);
    if(NULL == jc){
        return NULL;
    }

    jc->log = jct->log;
    jc->idx = idx;
    jc->timeout = jct->timeout;
    jc->nio_reader = jct->nio_reader;
    jc->nio_writer = jct->nio_writer;
    jc->reader_body_process = jct->reader_body_process;
    jc->reader_header_validator = jct->reader_header_validator;
    jc->reader_body_process_before = jct->reader_body_process_before;
    jc->writer_body_process = jct->writer_body_process;
    jc->reader_header_validator_fail = jct->reader_header_validator_fail;
    jc->config = jct->config;
    jc->is_lazy_recv = false;
    jc->is_sendfile = false;
    return jc;

}

err_t spx_job_context_free(void **arg){
    struct spx_job_context **jc = (struct spx_job_context **) arg;
    spx_job_context_clear(*jc);
    SpxFree(*jc);
    return 0;
}

void spx_job_context_clear(struct spx_job_context *jc){/*{{{*/

    if(NULL != jc->reader_body_ctx){
        SpxMsgFree(jc->reader_body_ctx);
    }

    if(NULL != jc->client_ip){
        SpxStringFree(jc->client_ip);
    }

    if(NULL != jc->reader_header){
        SpxFree(jc->reader_header);
    }
    if(NULL != jc->reader_header_ctx){
        SpxMsgFree(jc->reader_header_ctx);
    }

    if(NULL != (jc)->writer_header){
        SpxFree((jc)->writer_header);
    }
    if(NULL != (jc)->writer_header_ctx){
        SpxMsgFree(jc->writer_header_ctx);
    }
    if(NULL != (jc)->writer_body_ctx){
        SpxMsgFree(jc->writer_body_ctx);
    }

    if(jc->is_sendfile){
        if(0 != jc->sendfile_fd){
            SpxClose(jc->sendfile_fd);
        }
        jc->sendfile_begin = 0;
        jc->sendfile_size = 0;
    }
    SpxClose(jc->fd);
    jc->err = 0;
    jc->moore = SpxNioMooreNormal;
    jc->request_timespan = 0;
}/*}}}*/


void spx_job_context_reset(struct spx_job_context *jc){

    if(NULL != jc->reader_header){
        SpxFree(jc->reader_header);
    }
    if(NULL != jc->reader_header_ctx){
        spx_msg_free(&((jc)->reader_header_ctx));
    }
    if(NULL != jc->reader_body_ctx){
        spx_msg_free(&((jc)->reader_body_ctx));
    }

    if(NULL != (jc)->writer_header){
        SpxFree((jc)->writer_header);
    }
    if(NULL != (jc)->writer_header_ctx){
        spx_msg_free(&((jc)->writer_header_ctx));
    }
    if(NULL != (jc)->writer_body_ctx){
        spx_msg_free(&((jc)->writer_body_ctx));
    }

    if(jc->is_sendfile){
        if(0 != jc->sendfile_fd){
            SpxClose(jc->sendfile_fd);
        }
        jc->sendfile_begin = 0;
        jc->sendfile_size = 0;
    }
    jc->err = 0;
    jc->moore = SpxNioMooreNormal;
    jc->request_timespan = spx_now();
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
    struct spx_job_context *jc = spx_fixed_vector_pop(pool->pool,err);
    if(NULL == jc){
        *err = 0 == *err ? ENOENT : *err;
        return NULL;
    }
    jc->request_timespan = spx_now();
    return jc;
}

err_t spx_job_pool_push(struct spx_job_pool *pool,struct spx_job_context *jc){
    spx_job_context_clear(jc);
    return spx_fixed_vector_push(pool->pool,jc);
}

err_t spx_job_pool_free(struct spx_job_pool **pool){
    err_t err = 0;
    err = spx_fixed_vector_free(&((*pool)->pool));
    SpxFree(*pool);
    return err;
}



