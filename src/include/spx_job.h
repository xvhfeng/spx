/*
 * =====================================================================================
 *
 *       Filename:  spx_job.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/06/09 17时43分11秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef _SPX_JOB_H_
#define _SPX_JOB_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <ev.h>

#include "spx_types.h"
#include "spx_defs.h"
#include "spx_message.h"
#include "spx_properties.h"


    struct spx_job_context;
    typedef void (SpxNioDelegate)(struct ev_loop *loop,ev_io *watcher,int revents);
    typedef void (SpxNioBodyProcessDelegate)(int fd,struct spx_job_context *nio_context);
    typedef bool_t (SpxNioHeaderValidatorDelegate)(struct spx_job_context *nio_context);
    typedef void (SpxNioHeaderValidatorFailDelegate)(struct spx_job_context *nio_context);
    typedef void (SpxNotifyDelegate)(ev_io *watcher,int revents);

    extern struct spx_job_pool *g_spx_job_pool;

#define SpxMsgHeaderSize (3 * sizeof(u32_t) + 2 * sizeof(u64_t))
#define SpxNioLifeCycleNormal 0
#define SpxNioLifeCycleHeader 1
#define SpxNioLifeCycleBody 2

#define SpxNioMooreNormal 0
#define SpxNioMooreRequest 1
#define SpxNioMooreResponse 2

    struct spx_msg_header{
        u32_t version;
        u32_t protocol;
        u64_t bodylen;
        u64_t offset;
        u32_t err;
    };


    struct spx_job_context_transport{
        u32_t timeout;
        SpxNioDelegate *nio_reader;
        SpxNioDelegate *nio_writer;
        SpxNioHeaderValidatorDelegate *reader_header_validator;
        SpxNioHeaderValidatorFailDelegate *reader_header_validator_fail;
        SpxNioBodyProcessDelegate *reader_body_process;
        SpxNioBodyProcessDelegate *writer_body_process;
        void *config;
        SpxLogDelegate *log;
    };

    struct spx_job_context{
        ev_io watcher;
        int fd;
        int use;
        size_t idx;
        err_t err;
        u32_t timeout;
        ev_timer timer;
        SpxNioDelegate *nio_writer;
        SpxNioDelegate *nio_reader;
        SpxLogDelegate *log;
        u32_t lifecycle;
        u32_t moore;

        struct spx_msg_header *reader_header;
        struct spx_msg *reader_header_ctx;
        struct spx_msg *reader_body_ctx;
        struct spx_msg_header *writer_header;
        struct spx_msg *writer_header_ctx;
        struct spx_msg *writer_body_ctx;

        SpxNioHeaderValidatorDelegate *reader_header_validator;
        SpxNioHeaderValidatorFailDelegate *reader_header_validator_fail;
        SpxNioBodyProcessDelegate *reader_body_process;
        SpxNioBodyProcessDelegate *writer_body_process;

        string_t client_ip;

        void *config;

        /*
         * if lazy recv,must set the offset in the header by client
         * and the part of recved must in the end of the body
         */
        bool_t is_lazy_recv;
        off_t lazy_recv_offet;
        size_t lazy_recv_size;

        bool_t is_sendfile;
        int sendfile_fd;
        off_t sendfile_begin;
        size_t sendfile_size;
    };

    struct spx_job_pool {
        SpxLogDelegate *log;
        struct spx_fixed_vector *pool;
    };

    void *spx_job_context_new(size_t idx,void *arg,err_t *err);
    err_t spx_job_context_free(void **arg);
    void spx_job_context_clear(struct spx_job_context *jcontext);

    struct spx_job_pool *spx_job_pool_new(SpxLogDelegate *log,\
            void *config,\
            size_t size,u32_t timeout,\
            SpxNioDelegate *nio_reader,\
            SpxNioDelegate *nio_writer,\
            SpxNioHeaderValidatorDelegate *reader_header_validator,\
            SpxNioHeaderValidatorFailDelegate *reader_header_validator_fail,\
            SpxNioBodyProcessDelegate *reader_body_process,\
            SpxNioBodyProcessDelegate *write_body_process,\
            err_t *err);

    struct spx_job_context *spx_job_pool_pop(struct spx_job_pool *pool,err_t *err);
    err_t spx_job_pool_push(struct spx_job_pool *pool,struct spx_job_context *jcontext);
    err_t spx_job_pool_free(struct spx_job_pool **pool);

    struct spx_msg_header *spx_msg_to_header(struct spx_msg *ctx,err_t *err);
    struct spx_msg *spx_header_to_msg(struct spx_msg_header *header,size_t len,err_t *err);

#ifdef __cplusplus
}
#endif
#endif
