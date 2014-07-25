/*
 * =====================================================================================
 *
 *       Filename:  spx_sio_context.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/06/16 14时52分13秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef _SPX_SIO_CONTEXT_H_
#define _SPX_SIO_CONTEXT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <ev.h>

#include "spx_log.h"

    struct spx_sio_context;
    typedef void (SpxSioDelegate)(struct ev_loop *loop,ev_io *watcher,int revents);

    struct spx_sio_context{
        ev_io watcher;
        struct ev_loop *loop;
        pthread_t tid;
        int pipes[2];
        err_t err;
        size_t idx;
        SpxSioDelegate *sio_reader;
        SpxLogDelegate *log;
    };

    struct spx_sio_context_pool {
        SpxLogDelegate *log;
        struct spx_fixed_vector *pool;
    };

    extern struct spx_sio_context_pool *g_spx_sio_context_pool;

    struct spx_sio_context_pool *spx_sio_context_pool_new(SpxLogDelegate *log,\
            size_t size,\
            SpxSioDelegate *sio_reader,\
            err_t *err);

    struct spx_sio_context *spx_sio_context_pool_pop(struct spx_sio_context_pool *pool,err_t *err);
    err_t spx_sio_context_pool_push(struct spx_sio_context_pool *pool,struct spx_sio_context *sio_context);
    err_t spx_sio_context_pool_free(struct spx_sio_context_pool **pool);

#ifdef __cplusplus
}
#endif
#endif
