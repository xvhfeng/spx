/*
 * =====================================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  spx_threadpool.c
 *        Created:  2014/09/01 11时40分27秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "spx_types.h"
#include "spx_alloc.h"
#include "spx_defs.h"
#include "spx_threadpool.h"

spx_private bool_t push2pool(struct spx_threadpool *p, struct spx_thread *t);
spx_private bool_t push2pool(struct spx_threadpool *p, struct spx_thread *t) {/*{{{*/
    bool_t rc = false;
    do {
        pthread_mutex_lock(&p->mutex_locker);
        if (p->idx <(int) p->totalsize) {
            p->threads[p->idx] = t;
            p->idx++;
            rc = true;
            pthread_cond_signal(&p->run_locker);

            if (p->idx >=(int) p->currsize) {
                pthread_cond_signal(&p->full_locker);
            }
        }
    } while (0);
    pthread_mutex_unlock(&p->mutex_locker);
    return rc;
}/*}}}*/

spx_private void *spx_pooling_thread_listening(void *arg);

spx_private void *spx_pooling_thread_listening(void *arg){/*{{{*/
    struct spx_thread *t = (struct spx_thread *) arg;
    struct spx_threadpool *p = t->parent;
    while (initialized == p->state) {
        t->func(t->arg);

        if (p == NULL || initialized != p->state)
            break;

        pthread_mutex_lock(&t->mutex_locker);

        if (push2pool(p, t)) {
            pthread_cond_wait(&t->run_locker, &t->mutex_locker);
            pthread_mutex_unlock(&t->mutex_locker);
        } else {
            pthread_mutex_unlock(&t->mutex_locker);
            pthread_cond_destroy(&t->run_locker);
            pthread_mutex_destroy(&t->mutex_locker);
            pthread_cancel(t->id);
            SpxFree(t);
            break;
        }
    }

    pthread_mutex_lock(&p->mutex_locker);
    p->currsize --;
    if (0 >= p->currsize)
        pthread_cond_signal(&p->empty_locker);
    pthread_mutex_unlock(&p->mutex_locker);
    return NULL ;
}/*}}}*/

struct spx_threadpool *spx_threadpool_new(SpxLogDelegate *log,size_t max,
        size_t thread_stack_size,err_t *err) {/*{{{*/
    if(0 == max){
        *err = EINVAL;
        return NULL;
    }

    struct spx_threadpool *p = (struct spx_threadpool *) spx_alloc_alone(sizeof(*p),err);
    if(NULL == p){
        SpxLog2(log,SpxLogError,*err,
                "new threadpool is fail.");
        return NULL;
    }

    p->state = initializing;
    p->log = log;
    p->totalsize = max;
    p->currsize = 0;
    p->idx = 0;
    p->thread_stack_size = thread_stack_size;

    pthread_mutex_init(&(p->mutex_locker), NULL );
    pthread_cond_init(&(p->run_locker), NULL );
    pthread_cond_init(&(p->empty_locker), NULL );
    pthread_cond_init(&(p->full_locker), NULL );

    p->threads = (struct spx_thread **) spx_alloc(max,sizeof(struct spx_thread *),err);
    if (NULL == p->threads) {
        SpxLog2(p->log,SpxLogError,*err,
                "init threads list is fail.");
        pthread_cond_destroy(&(p->run_locker));
        pthread_cond_destroy(&(p->empty_locker));
        pthread_cond_destroy(&(p->full_locker));
        pthread_mutex_destroy(&(p->mutex_locker));
        SpxFree(p);
        return NULL;
    }

    p->state = initialized;
    return p;
}/*}}}*/

err_t spx_threadpool_execute(struct spx_threadpool *p,
        SpxThreadpoolExecuteDelegate *func,void *arg){/*{{{*/
    if(NULL == p || NULL == func){
        return EINVAL;
    }

    err_t err = 0;
    if(0 != (err = pthread_mutex_lock(&(p->mutex_locker)))){
        SpxLog2(p->log,SpxLogError,err,
                "lock threadpool mutex is fail.");
        return err;
    }

    while(0 > p->idx && p->totalsize == p->currsize){
        SpxLog1(p->log,SpxLogWarn,
                "threadpool is all busy,waitting...");
        pthread_cond_wait(&(p->run_locker),&(p->mutex_locker));
    }

    if(0 > p->idx && p->totalsize > p->currsize){
        struct spx_thread *t = (struct spx_thread *) spx_alloc_alone(sizeof(*t),&err);
        if(NULL == t){
            SpxLog2(p->log,SpxLogError,err,
                    "new pooling thread is fail.");
            goto r1;
        }
        t->log = p->log;
        t->arg = arg;
        t->func = func;
        t->parent = p;

        pthread_mutex_init(&(t->mutex_locker), NULL );
        pthread_cond_init(&(t->run_locker), NULL );
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        size_t ostack_size = 0;
        pthread_attr_getstacksize(&attr, &ostack_size);
        if (ostack_size != p->thread_stack_size
                && (0 != (err = pthread_attr_setstacksize(&attr,p->thread_stack_size)))){
            pthread_attr_destroy(&attr);
            SpxLog2(p->log,SpxLogError,err,\
                    "set thread stack size is fail.");
            goto r1;
        }

        if (0 == pthread_create(&(t->id), &attr, spx_pooling_thread_listening,t)) {
            p->currsize++;
        } else {
            pthread_mutex_destroy(&(t->mutex_locker));
            pthread_cond_destroy(&(t->run_locker));
            SpxFree(t);
        }
        pthread_attr_destroy(&attr);
    } else {
        p->idx--; //because the array begin with 0
        struct spx_thread *t = p->threads[p->idx];
        p->threads[p->idx] = NULL;

        t->func = func;
        t->arg = arg;

        pthread_mutex_lock(&(t->mutex_locker));
        pthread_cond_signal(&(t->run_locker));
        pthread_mutex_unlock(&(t->mutex_locker));
    }
r1:
    pthread_mutex_unlock(&(p->mutex_locker));
    return err;
}/*}}}*/
