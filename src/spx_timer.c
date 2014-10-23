/*************************************************************
 *                     _ooOoo_
 *                    o8888888o
 *                    88" . "88
 *                    (| -_- |)
 *                    O\  =  /O
 *                 ____/`---'\____
 *               .'  \\|     |//  `.
 *              /  \\|||  :  |||//  \
 *             /  _||||| -:- |||||-  \
 *             |   | \\\  -  /// |   |
 *             | \_|  ''\---/''  |   |
 *             \  .-\__  `-`  ___/-. /
 *           ___`. .'  /--.--\  `. . __
 *        ."" '<  `.___\_<|>_/___.'  >'"".
 *       | | :  `- \`.;`\ _ /`;.`/ - ` : | |
 *       \  \ `-.   \_ __\ /__ _/   .-` /  /
 *  ======`-.____`-.___\_____/___.-`____.-'======
 *                     `=---='
 *  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *           佛祖保佑       永无BUG
 *
 * ==========================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  spx_timer.c
 *        Created:  2014/10/22 10时44分15秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "spx_alloc.h"
#include "spx_types.h"
#include "spx_defs.h"
#include "spx_timer.h"
#include "spx_atomic.h"
#include "spx_thread.h"


spx_private void *spx_timer_run_async(void *arg);
spx_private void *spx_timer_exec_and_run_async(void *arg);

spx_private void *spx_timer_run_async(void *arg){/*{{{*/
    TypeConvert2(struct spx_timer,t,arg);
    t->status = SpxTimerRunning;
    struct timeval timespan;
    do{
        if(SpxAtomicVCas(t->status,SpxTimerSetPausing,SpxTimerPausing)){
            pthread_mutex_lock(t->mlock);
            pthread_cond_wait(t->clock,t->mlock);
            pthread_mutex_unlock(t->mlock);
        }
        timespan.tv_sec = t->sec;
        timespan.tv_usec = t->usec;
        pthread_testcancel();
        t->err = (select(0,NULL,NULL,NULL,&timespan));
        pthread_testcancel();
        if(0 > t->err) {
            SpxLog2(t->log,SpxLogError,t->err,
                    "async timer by select is error.");
        } else if(0 == t->err) {
            pthread_testcancel();
            t->timeout_handler(t->arg);
            pthread_testcancel();
        } else {
            if(!(EAGAIN == errno || EWOULDBLOCK == errno)) {
                SpxLog2(t->log,SpxLogError,t->err,
                        "async timer by select is error.");
            }
        }
    }while(t->is_run);
    return NULL;
}/*}}}*/

spx_private void *spx_timer_exec_and_run_async(void *arg){/*{{{*/
    TypeConvert2(struct spx_timer,t,arg);
    t->status = SpxTimerRunning;
    t->timeout_handler(t->arg);
    struct timeval timespan;
    do{
        if(SpxAtomicVCas(t->status,SpxTimerSetPausing,SpxTimerPausing)){
            pthread_mutex_lock(t->mlock);
            pthread_cond_wait(t->clock,t->mlock);
            pthread_mutex_unlock(t->mlock);
        }
        timespan.tv_sec = t->sec;
        timespan.tv_usec = t->usec;
        pthread_testcancel();
        t->err = (select(0,NULL,NULL,NULL,&timespan));
        pthread_testcancel();
        if(0 > t->err) {
            SpxLog2(t->log,SpxLogError,t->err,
                    "async timer by select is error.");
        } else if(0 == t->err) {
            pthread_testcancel();
            t->timeout_handler(t->arg);
            pthread_testcancel();
        } else {
            if(!(EAGAIN == errno || EWOULDBLOCK == errno)) {
                SpxLog2(t->log,SpxLogError,t->err,
                        "async timer by select is error.");
            }
        }
    }while(t->is_run);
    return NULL;
}/*}}}*/

void spx_sleep(int sec,int usec) {/*{{{*/
    struct timeval timespan;
    timespan.tv_sec = sec;
    timespan.tv_usec = usec;
    select(0,NULL,NULL,NULL,&timespan);
    return;
}/*}}}*/

void spx_timer_run(SpxLogDelegate *log,
        u32_t secs,u64_t usecs,
        SpxExpiredDelegate *timeout_handler,
        void *arg,
        err_t *err){/*{{{*/
    if(NULL == timeout_handler
            || (0 == secs && 0 == usecs)){
        *err = EINVAL;
        return;
    }
    struct timeval timespan;
    while(true) {
        timespan.tv_sec = secs;
        timespan.tv_usec = usecs;
        *err = (select(0,NULL,NULL,NULL,&timespan));
        if(0 > *err) {
            SpxLog2(log,SpxLogError,*err,
                    "timer by select is error.");
        } else if(0 == *err) {
            timeout_handler(arg);
        } else {
            if(!(EAGAIN == errno || EWOULDBLOCK == errno)) {
                SpxLog2(log,SpxLogError,*err,
                        "timer by select is error.");
            }
        }
    }
}/*}}}*/

void spx_timer_run_once(SpxLogDelegate *log,
        u32_t secs,u64_t usecs,
        SpxExpiredDelegate *timeout_handler,
        void *arg,
        err_t *err){/*{{{*/
    if(NULL == timeout_handler
            || (0 == secs && 0 == usecs)){
        *err = EINVAL;
        return;
    }
    struct timeval timespan;
    timespan.tv_sec = secs;
    timespan.tv_usec = usecs;
    *err = (select(0,NULL,NULL,NULL,&timespan));
    if(0 > *err) {
        SpxLog2(log,SpxLogError,*err,
                "timer by select is error.");
    } else if(0 == *err) {
        timeout_handler(arg);
    } else {
        if(!(EAGAIN == errno || EWOULDBLOCK == errno)) {
            SpxLog2(log,SpxLogError,*err,
                    "timer by select is error.");
        }
    }
}/*}}}*/

void spx_timer_exec_and_run(SpxLogDelegate *log,
        u32_t secs,u64_t usecs,
        SpxExpiredDelegate *timeout_handler,
        void *arg,
        err_t *err){/*{{{*/
    if(NULL == timeout_handler
            || (0 == secs && 0 == usecs)){
        *err = EINVAL;
        return;
    }
    timeout_handler(arg);
    spx_timer_run(log,secs,usecs,timeout_handler,arg,err);
}/*}}}*/


struct spx_timer *spx_timer_async_run(SpxLogDelegate *log,
        u32_t secs,u64_t usecs,
        SpxExpiredDelegate *timeout_handler,
        void *arg,
        size_t stacksize,
        err_t *err){/*{{{*/
    if(NULL == timeout_handler
            || (0 == secs && 0 == usecs)){
        *err = EINVAL;
        return NULL;
    }
    struct spx_timer *t = (struct spx_timer *)
        spx_alloc_alone(sizeof(*t),err);
    if(NULL == t) {
        return NULL;
    }
    t->is_run = true;
    t->sec = secs;
    t->usec = usecs;
    t->log = log;
    t->arg = arg;
    t->timeout_handler = timeout_handler;
    t->mlock = spx_thread_mutex_new(log,err);
    if(NULL == t->mlock){
        goto r1;
    }
    t->clock = spx_thread_cond_new(log,err);
    if(NULL == t->clock){
        goto r1;
    }

    t->tid = spx_thread_new_cancelability(log,
            stacksize,spx_timer_run_async,(void *) t,err);
    if(0 == t->tid){
        goto r1;
    }
    return t;
r1:
    if(NULL != t->clock){
        spx_thread_cond_free(&(t->clock));
    }
    if(NULL != t->mlock){
        spx_thread_mutex_free(&(t->mlock));
    }
    if(NULL != t){
        SpxFree(t);
    }
    return NULL;
}/*}}}*/

struct spx_timer *spx_timer_async_run_once(SpxLogDelegate *log,
        u32_t secs,u64_t usecs,
        SpxExpiredDelegate *timeout_handler,
        void *arg,
        size_t stacksize,
        err_t *err){/*{{{*/
    if(NULL == timeout_handler
            || (0 == secs && 0 == usecs)){
        *err = EINVAL;
        return NULL;
    }
    struct spx_timer *t = (struct spx_timer *)
        spx_alloc_alone(sizeof(*t),err);
    if(NULL == t) {
        return NULL;
    }
    t->is_run = false;
    t->sec = secs;
    t->usec = usecs;
    t->log = log;
    t->arg = arg;
    t->timeout_handler = timeout_handler;
    t->mlock = spx_thread_mutex_new(log,err);
    if(NULL == t->mlock){
        goto r1;
    }
    t->clock = spx_thread_cond_new(log,err);
    if(NULL == t->clock){
        goto r1;
    }

    t->tid = spx_thread_new_cancelability(log,
            stacksize,spx_timer_run_async,(void *) t,err);
    if(0 == t->tid){
        goto r1;
    }
    return t;
r1:
    if(NULL != t->clock){
        spx_thread_cond_free(&(t->clock));
    }
    if(NULL != t->mlock){
        spx_thread_mutex_free(&(t->mlock));
    }
    if(NULL != t){
        SpxFree(t);
    }
    return NULL;
}/*}}}*/

struct spx_timer *spx_timer_async_exec_and_run(SpxLogDelegate *log,
        u32_t secs,u64_t usecs,
        SpxExpiredDelegate *timeout_handler,
        void *arg,
        size_t stacksize,
        err_t *err){/*{{{*/
    if(NULL == timeout_handler
            || (0 == secs && 0 == usecs)){
        *err = EINVAL;
        return NULL;
    }
    struct spx_timer *t = (struct spx_timer *)
        spx_alloc_alone(sizeof(*t),err);
    if(NULL == t) {
        return NULL;
    }
    t->is_run = true;
    t->sec = secs;
    t->usec = usecs;
    t->log = log;
    t->arg = arg;
    t->timeout_handler = timeout_handler;
    t->mlock = spx_thread_mutex_new(log,err);
    if(NULL == t->mlock){
        goto r1;
    }
    t->clock = spx_thread_cond_new(log,err);
    if(NULL == t->clock){
        goto r1;
    }

    t->tid = spx_thread_new_cancelability(log,
            stacksize,spx_timer_run_async,(void *) t,err);
    if(0 == t->tid){
        goto r1;
    }
    return t;
r1:
    if(NULL != t->clock){
        spx_thread_cond_free(&(t->clock));
    }
    if(NULL != t->mlock){
        spx_thread_mutex_free(&(t->mlock));
    }
    if(NULL != t){
        SpxFree(t);
    }
    return NULL;
}/*}}}*/

struct spx_timer *spx_timer_exec_and_async_run(SpxLogDelegate *log,
        u32_t secs,u64_t usecs,
        SpxExpiredDelegate *timeout_handler,
        void *arg,
        size_t stacksize,
        err_t *err){/*{{{*/
    if(NULL == timeout_handler
            || (0 == secs && 0 == usecs)){
        *err = EINVAL;
        return NULL;
    }
    timeout_handler(arg);
    return spx_timer_async_run(log,secs,usecs,timeout_handler,arg,stacksize,err);
}/*}}}*/

bool_t spx_timer_async_suspend(struct spx_timer *t){/*{{{*/
    return SpxAtomicVCas(t->status,SpxTimerRunning,SpxTimerSetPausing);
}/*}}}*/

bool_t spx_timer_async_resume(struct spx_timer *t){/*{{{*/
    if(SpxAtomicVCas(t->status,SpxTimerSetPausing,SpxTimerRunning)){
        return true;
    }else {
        if(SpxAtomicVCas(t->status,SpxTimerPausing,SpxTimerRunning)){
            pthread_mutex_lock(t->mlock);
            pthread_cond_signal(t->clock);
            pthread_mutex_unlock(t->mlock);
            return true;
        }
    }
    return false;
}/*}}}*/

void spx_timer_stop(struct spx_timer **timer){/*{{{*/
    struct spx_timer *t = *timer;
    SpxAtomicVSet(t->is_run,false);
    pthread_cancel(t->tid);
    if(SpxTimerSetPausing == t->status
            || SpxTimerPausing == t->status){
        spx_timer_async_resume(t);
    }
    pthread_join(t->tid,NULL);
    if(NULL != t->mlock){
        spx_thread_mutex_free(&(t->mlock));
    }
    if(NULL != t->clock){
        spx_thread_cond_free(&(t->clock));
    }
    SpxFree(*timer);
}/*}}}*/

