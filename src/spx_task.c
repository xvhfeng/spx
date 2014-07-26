/*
 * =====================================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  spx_task_context.c
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
#include "include/spx_task.h"
#include "include/spx_alloc.h"
#include "include/spx_fixed_vector.h"

struct spx_task_pool *g_spx_task_pool = NULL;

void *spx_task_context_new(size_t idx,void *arg,err_t *err){
    struct spx_task_context_transport *tct = (struct spx_task_context_transport *) arg;
    struct spx_task_context *tcontext = (struct spx_task_context *) \
                                        spx_alloc_alone(sizeof(*tcontext),err);
    if(NULL == tcontext){
        return NULL;
    }
    tcontext->log = tct->log;
    tcontext->events = tct->events;
    tcontext->noblacking = tct->noblacking;
    tcontext->idx = idx;
    tcontext->dio_process_handler = tct->dio_process_handler;
    tcontext->dio_handler = tct->dio_handler;
    tcontext->arg = tct->arg;
    return tcontext;
}

err_t spx_task_context_free(void **arg){
    struct spx_task_context **tcontext = (struct spx_task_context **) arg;
    if(NULL != *tcontext){
        SpxFree(*tcontext);
    }
    return 0;
}

void spx_task_context_clear(struct spx_task_context *tcontext){
    tcontext->events = 0;
    return;
}

struct spx_task_pool *spx_task_pool_new(\
        SpxLogDelegate *log,\
        size_t size,\
        bool_t noblacking,\
        SpxDioDelegate *dio_handler,\
        SpxDioProcessDelegate *dio_process_handler,\
        int events,\
        void *arg,\
        err_t *err){
    struct spx_task_pool *pool = NULL;
    pool = spx_alloc_alone(sizeof(*pool),err);
    if(NULL == pool){
        return NULL;
    }
    struct spx_task_context_transport tct;
    SpxZero(tct);
    tct.arg = arg;
    tct.log = log;
    tct.dio_handler =dio_handler;
    tct.dio_process_handler = dio_process_handler;
    tct.events = events;
    tct.noblacking = noblacking;

    pool->pool = spx_fixed_vector_new(log,size,\
            spx_task_context_new,\
            &tct,\
            spx_task_context_free,\
            err);

    if(NULL == pool->pool){
        SpxFree(pool);
        return NULL;
    }
    return pool;
}

struct spx_task_context *spx_task_pool_pop(\
        struct spx_task_pool *pool,\
        err_t *err){
    struct spx_task_context *tcontext = spx_fixed_vector_pop(pool->pool,err);
    return tcontext;
}

err_t spx_task_pool_push(struct spx_task_pool *pool,struct spx_task_context *tcontext){
    spx_task_context_clear(tcontext);
    return spx_fixed_vector_push(pool->pool,tcontext);
}

err_t spx_task_pool_free(struct spx_task_pool **pool){
    err_t err = 0;
    err = spx_fixed_vector_free(&((*pool)->pool));
    SpxFree(*pool);
    return err;
}


