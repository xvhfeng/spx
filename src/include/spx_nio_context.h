/*
 * =====================================================================================
 *
 *       Filename:  spx_nio_context.h
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
#ifndef _SPX_NIO_CONTEXT_H_
#define _SPX_NIO_CONTEXT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <ev.h>

#include "spx_types.h"
#include "spx_defs.h"
#include "spx_message.h"
#include "spx_properties.h"


    struct spx_nio_context;
    typedef void (SpxNioDelegate)(struct ev_loop *loop,ev_io *watcher,int revents);
    typedef void (SpxNioBodyProcessDelegate)(int fd,struct spx_nio_context *nio_context);
    typedef bool_t (SpxNioHeaderValidatorDelegate)(struct spx_nio_context *nio_context);
    typedef void (SpxNioHeaderValidatorFailDelegate)(struct spx_nio_context *nio_context);
    typedef void (SpxNotifyDelegate)(ev_io *watcher,int revents);

    extern struct spx_nio_context_pool *g_spx_nio_context_pool;

#define SpxMsgHeaderSize (3 * sizeof(u32_t) + 2 * sizeof(u64_t))
#define SpxNioLifeCycleNormal 0
#define SpxNioLifeCycleHeader 1
#define SpxNioLifeCycleBody 2

    struct spx_msg_header{
        u32_t version;
        u32_t protocol;
        u64_t bodylen;
        u64_t offset;
        u32_t err;
    };
    struct spx_nio_context{
        ev_io watcher;
        ev_async notify;
        int fd;
        int use;
        err_t err;
        u32_t timeout;
        struct ev_loop *loop;
        ev_timer timer;
        SpxNioDelegate *nio_writer;
        SpxNioDelegate *nio_reader;
        SpxLogDelegate *log;
        u32_t lifecycle;

        struct spx_msg_header *request_header;
        struct spx_msg *request_header_ctx;
        struct spx_msg *request_body_ctx;
        struct spx_msg_header *response_header;
        struct spx_msg *response_header_ctx;
        struct spx_msg *response_body_ctx;

        SpxNioHeaderValidatorDelegate *request_header_validator;
        SpxNioHeaderValidatorFailDelegate *request_header_validator_fail;
        SpxNioBodyProcessDelegate *request_body_process;
        SpxNioBodyProcessDelegate *response_body_process;

        string_t client_ip;

        struct spx_properties *config;

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

    struct spx_nio_context_pool {
        SpxLogDelegate *log;
        struct spx_fixed_vector *pool;
    };

    struct spx_nio_context_pool *spx_nio_context_pool_new(SpxLogDelegate *log,\
            struct spx_properties *config,\
            size_t size,u32_t timeout,\
            SpxNioDelegate *nio_reader,\
            SpxNioDelegate *nio_writer,\
            SpxNioHeaderValidatorDelegate *request_header_validator,\
            SpxNioHeaderValidatorFailDelegate *request_header_validator_fail,\
            SpxNioBodyProcessDelegate *request_body_process,\
            SpxNioBodyProcessDelegate *response_body_process,\
            err_t *err);

    struct spx_nio_context *spx_nio_context_pool_pop(struct spx_nio_context_pool *pool,err_t *err);
    err_t spx_nio_context_pool_push(struct spx_nio_context_pool *pool,struct spx_nio_context *nio_context);
    err_t spx_nio_context_pool_free(struct spx_nio_context_pool **pool);

    struct spx_msg_header *spx_msg_to_header(struct spx_msg *ctx,err_t *err);
    struct spx_msg *spx_header_to_msg(struct spx_msg_header *header,size_t len,err_t *err);

#ifdef __cplusplus
}
#endif
#endif
