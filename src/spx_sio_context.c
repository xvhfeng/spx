/*
 * =====================================================================================
 *
 *       Filename:  spx_sio_context.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/06/16 14时08分50秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>

#include "include/spx_sio_context.h"
#include "include/spx_types.h"
#include "include/spx_fixed_vector.h"
#include "include/spx_defs.h"
#include "include/spx_alloc.h"


struct spx_sio_context_pool *g_spx_sio_context_pool = NULL;
struct spx_sio_context_pool_node_arg{
    SpxSioDelegate *sio_reader;
    SpxLogDelegate *log;
};

spx_private void *spx_sio_context_pool_node_new(void *arg,err_t *err);
spx_private err_t spx_sio_context_pool_node_free(void **arg);

spx_private void *spx_sio_context_pool_node_new(void *arg,err_t *err){

    struct spx_sio_context_pool_node_arg *n = (struct spx_sio_context_pool_node_arg *) arg;
    struct spx_sio_context *sio_context = NULL;
    sio_context = spx_alloc_alone(sizeof(*sio_context),err);
    if(NULL == sio_context){
        return NULL;
    }
    sio_context->loop = ev_loop_new(EVFLAG_AUTO);
    sio_context->log = n->log;
    sio_context->sio_reader = n->sio_reader;
    if(-1 == pipe(sio_context->pipes)){
        *err = errno;
        ev_loop_destroy(sio_context->loop);
        SpxFree(sio_context);
        return NULL;
    }
    return sio_context;
}

spx_private err_t spx_sio_context_pool_node_free(void **arg){
    struct spx_sio_context **sio_context = (struct spx_sio_context **) arg;
    ev_break((*sio_context)->loop,EVBREAK_ALL);
    ev_loop_destroy((*sio_context)->loop);
    SpxClose((*sio_context)->pipes[0]);
    SpxClose((*sio_context)->pipes[1]);
    SpxFree(*sio_context);
    return 0;
}


struct spx_sio_context_pool *spx_sio_context_pool_new(SpxLogDelegate *log,\
        size_t size,\
        SpxSioDelegate *sio_reader,\
        err_t *err){
    if(0 == size){
        *err = EINVAL;
    }
    struct spx_sio_context_pool *pool = NULL;
    pool = spx_alloc_alone(sizeof(*pool),err);
    if(NULL == pool){
        return NULL;
    }

    struct spx_sio_context_pool_node_arg arg;
    SpxZero(arg);
    arg.sio_reader = sio_reader;
    arg.log = log;

    pool->pool = spx_fixed_vector_new(log,size,\
            spx_sio_context_pool_node_new,\
            &arg,\
            spx_sio_context_pool_node_free,\
            err);
    if(NULL == pool->pool){
        SpxFree(pool);
        return NULL;
    }
    return pool;
}

struct spx_sio_context *spx_sio_context_pool_pop(struct spx_sio_context_pool *pool,err_t *err){
    struct spx_sio_context *sio_context = spx_fixed_vector_pop(pool->pool,err);
    return sio_context;
}

err_t spx_sio_context_pool_push(struct spx_sio_context_pool *pool,struct spx_sio_context *sio_context){
    return spx_fixed_vector_push(pool->pool,sio_context);
}

err_t spx_sio_context_pool_free(struct spx_sio_context_pool **pool){
    err_t err = 0;
    err = spx_fixed_vector_free(&((*pool)->pool));
    SpxFree(*pool);
    return err;
}

