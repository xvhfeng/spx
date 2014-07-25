/*
 * =====================================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  spx_dio_context.c
 *        Created:  2014/07/25 10时28分18秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <stdio.h>


#include "include/spx_types.h"
#include "include/spx_dio_context.h"
#include "include/spx_alloc.h"
#include "include/spx_fixed_vector.h"

void *spx_dio_context_new(size_t idx,void *arg,err_t *err){
    struct spx_dio_context_node *n = (struct spx_dio_context_node *) arg;
    struct spx_dio_context *dio_context = (struct spx_dio_context *) \
                                          spx_alloc_alone(sizeof(*dio_context),err);
    if(NULL == dio_context){
        return NULL;
    }
    dio_context->log = n->log;
    dio_context->events = n->events;
    dio_context->noblacking = n->noblacking;
    dio_context->idx = idx;
    dio_context->dio_process_handler = n->dio_process_handler;
    dio_context->dio_handler = n->dio_handler;
    dio_context->arg = n->arg;
    return dio_context;
}

err_t spx_dio_context_free(void **arg){
    struct spx_dio_context **dio_context = (struct spx_dio_context **) arg;
    if(NULL != *dio_context){
        SpxFree(*dio_context);
    }
    return 0;
}
void spx_dio_context_clear(struct spx_dio_context *dio_context){
    dio_context->events = 0;
    return;
}

struct spx_dio_context_pool *spx_dio_context_pool_new(\
        SpxLogDelegate *log,\
        size_t size,\
        bool_t noblacking,\
        SpxDioDelegate *dio_handler,\
        SpxDioProcessDelegate *dio_process_handler,\
        int events,\
        void *arg,\
        err_t *err){
    struct spx_dio_context_pool *pool = NULL;
    pool = spx_alloc_alone(sizeof(*pool),err);
    if(NULL == pool){
        return NULL;
    }
    struct spx_dio_context_node n;
    SpxZero(n);
    n.arg = arg;
    n.log = log;
    n.dio_handler =dio_handler;
    n.dio_process_handler = dio_process_handler;
    n.events = events;
    n.noblacking = noblacking;

    pool->pool = spx_fixed_vector_new(log,size,\
            spx_dio_context_new,\
            &n,\
            spx_dio_context_free,\
            err);

    if(NULL == pool->pool){
        SpxFree(pool);
        return NULL;
    }
    return pool;
}

struct spx_dio_context *spx_dio_context_pool_pop(struct spx_dio_context_pool *pool,err_t *err){
    struct spx_dio_context *dio_context = spx_fixed_vector_pop(pool->pool,err);
    return dio_context;
}

err_t spx_dio_context_pool_push(struct spx_dio_context_pool *pool,struct spx_dio_context *dio_context){
    spx_dio_context_clear(dio_context);
    return spx_fixed_vector_push(pool->pool,dio_context);
}

err_t spx_dio_context_pool_free(struct spx_dio_context_pool **pool){
    err_t err = 0;
    err = spx_fixed_vector_free(&((*pool)->pool));
    SpxFree(*pool);
    return err;
}


