/*
 * =====================================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  spx_thread_context.c
 *        Created:  2014/07/22 17时19分04秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <ev.h>

#include "include/spx_types.h"
#include "include/spx_list.h"
#include "include/spx_fixed_vector.h"
#include "include/spx_module.h"
#include "include/spx_alloc.h"
#include "include/spx_defs.h"
#include "include/spx_string.h"

struct spx_trigger_context_transport{
    SpxTriggerDelegate *trigger_handler;
    int event;
    SpxLogDelegate *log;
};

struct spx_thread_pending_transport{
    size_t idx;
    struct spx_module_context *mc;
};


spx_private void *spx_thread_context_new(size_t idx,void *arg,err_t *err);
spx_private err_t spx_thread_context_free(void **arg);
spx_private void *spx_trigger_new(size_t idx,void *arg,err_t *err);
spx_private err_t spx_trigger_free(void **arg);
spx_private void *spx_thread_listening(void *arg);

spx_private void *spx_thread_context_new(size_t idx,void *arg,err_t *err){
    SpxLogDelegate *log = (SpxLogDelegate *) arg;
    struct spx_thread_context *tc = (struct spx_thread_context *) \
                                    spx_alloc_alone(sizeof(*tc),err);
    if(NULL == tc){
        SpxLog2(log,SpxLogError,*err,\
                "alloc thread context is fail.");
        return NULL;
    }
    tc->idx = idx;
    tc->loop = ev_loop_new(EVFLAG_AUTO);
    tc->log = log;
    if(-1 == pipe(tc->pipe)){
        *err = errno;
        ev_loop_destroy(tc->loop);
        SpxFree(tc);
        return NULL;
    }
    return tc;
}

spx_private err_t spx_thread_context_free(void **arg){
    struct spx_thread_context **tc = (struct spx_thread_context **) arg;
    ev_break((*tc)->loop,EVBREAK_ALL);
    ev_loop_destroy((*tc)->loop);
    SpxClose((*tc)->pipe[0]);
    SpxClose((*tc)->pipe[1]);
    SpxFree(*tc);
    return 0;
}

spx_private void *spx_trigger_new(size_t idx,void *arg,err_t *err){
    struct spx_trigger_context_transport *tct = (struct spx_trigger_context_transport *) arg;
    if(NULL == tct){
        *err = EINVAL;
        return NULL;
    }
    struct spx_trigger_context *t = (struct spx_trigger_context *) \
                                    spx_alloc_alone(sizeof(*t),err);
    if(NULL == t){
        SpxLog2(tct->log,SpxLogError,*err,\
                "alloc trigger context is fail.");
        return NULL;
    }
    t->log = tct->log;
    t->idx = idx;
    t->trigger_handler = tct->trigger_handler;
    return t;
}

spx_private err_t spx_trigger_free(void **arg){
    struct spx_trigger_context **t = (struct spx_trigger_context **) arg;
    SpxFree(*t);
    return 0;
}

spx_private void *spx_thread_listening(void *arg){
    struct spx_thread_pending_transport *tpt = (struct spx_thread_pending_transport *) arg;
    size_t idx = tpt->idx;
    struct spx_module_context *mc = tpt->mc;
    SpxFree(tpt);//free the memory
    struct spx_trigger_context *tc = spx_list_get(mc->receive_triggers,idx);
    struct spx_thread_context *stc = spx_list_get(mc->threadpool,idx);
    ev_io_init(&(tc->watcher),tc->trigger_handler,stc->pipe[0],EV_READ);
    ev_io_start(stc->loop,&(tc->watcher));
    ev_run(stc->loop,0);
    return NULL;
}

struct spx_module_context *spx_module_new(\
        SpxLogDelegate *log,\
        u32_t threadsize,\
        size_t stack_size,\
        SpxTriggerDelegate *dispatch_notice_handler,\
        SpxTriggerDelegate *receive_notification_handler,\
        err_t *err){
    struct spx_module_context *mc = (struct spx_module_context *)\
                                    spx_alloc_alone(sizeof(*mc),err);
    if(NULL == mc){
        SpxLog2(log,SpxLogError,*err,\
                "alloc module context is fail.");
        return NULL;
    }

    mc->log = log;
    mc->threadpool = spx_list_init(log,\
            threadsize,\
            spx_thread_context_new,\
            log,\
            spx_thread_context_free,\
            err);
    if(NULL == mc->threadpool){
        SpxLog2(log,SpxLogError,*err,\
                "alloc threadpool for module is fail.");
        goto r2;
    }

    struct spx_trigger_context_transport tct;
    SpxZero(tct);

    tct.log = log;
    tct.event = EV_READ;
    tct.trigger_handler = receive_notification_handler;
    mc->receive_triggers = spx_list_init(log,\
            threadsize,\
            spx_trigger_new,\
            &tct,\
            spx_trigger_free,\
            err);
    if(NULL == mc->receive_triggers){
        SpxLog2(log,SpxLogError,*err,\
                "alloc receive triggers are fail.");
        goto r2;
    }

    tct.event = EV_WRITE;
    tct.trigger_handler = dispatch_notice_handler;
    mc->dispatch_triggers = spx_fixed_vector_new(log,\
            2 * threadsize,\
            spx_trigger_new,\
            &tct,\
            spx_trigger_free,\
            err);
    if(NULL == mc->dispatch_triggers){
        SpxLog2(log,SpxLogError,*err,\
                "alloc dispatch triggers are fail.");
        goto r2;
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    size_t ostack_size = 0;
    pthread_attr_getstacksize(&attr, &ostack_size);
    do{
        if (ostack_size != stack_size
                && (0 != (*err = pthread_attr_setstacksize(&attr,stack_size)))){
            SpxLog2(log,SpxLogError,*err,\
                    "set thread stack size is fail.");
            goto r1;
        }
        u32_t i = 0;
        struct spx_thread_pending_transport *tpt;
        for( ; i < threadsize; i++){
            tpt = spx_alloc_alone(sizeof(*tpt),err);
            tpt->mc = mc;
            tpt->idx = i;
            struct spx_thread_context *n = spx_list_get(mc->threadpool,i);
            if (0 !=(*err =  pthread_create(&(n->tid), &attr, spx_thread_listening,
                            tpt))){
                SpxLog2(log,SpxLogError,*err,\
                        "create nio thread is fail.");
                goto r1;
            }
        }
    }while(false);
    pthread_attr_destroy(&attr);
    return mc;
r1:
    pthread_attr_destroy(&attr);
r2:
    spx_module_free(&mc);
    return NULL;
}

err_t spx_module_free(struct spx_module_context **mc){
    //must free thread pool first
    if(NULL != (*mc)->threadpool){
        spx_list_free(&((*mc)->threadpool));
    }
    if(NULL != (*mc)->receive_triggers) {
        spx_list_free(&((*mc)->receive_triggers));
    }
    if(NULL != (*mc)->dispatch_triggers){
        spx_fixed_vector_free(&((*mc)->dispatch_triggers));
    }
    SpxFree(*mc);
    return 0;
}


struct spx_trigger_context *spx_module_dispatch_trigger_pop(struct spx_module_context *mc,err_t *err){
    return (struct spx_trigger_context *) spx_fixed_vector_pop(mc->dispatch_triggers,err);
}

err_t spx_module_dispatch_trigger_push(struct spx_module_context *mc,struct spx_trigger_context *tc){
    return spx_fixed_vector_push(mc->dispatch_triggers,tc);
}

err_t spx_module_dispatch(struct spx_module_context *mc,size_t idx,void *msg){
    err_t err = 0;
    struct spx_trigger_context *tc = spx_module_dispatch_trigger_pop(mc,&err);
    if(NULL == tc){
        SpxLog2(mc->log,SpxLogWarn,err,\
                "pop a dispatch trigger is fail.");
        return err;
    }
    struct spx_thread_context *stc = spx_list_get(mc->threadpool,idx);
    ev_io_init(&(tc->watcher),tc->trigger_handler,stc->pipe[1],EV_WRITE);
    tc->watcher.data = msg;
    ev_io_start(stc->loop,&(tc->watcher));
    ev_run(stc->loop,0);
    return err;
}

