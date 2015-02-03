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
 *       Filename:  SpxPeriodic.c
 *        Created:  2015年01月27日 11时38分27秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "SpxMFunc.h"
#include "SpxLimits.h"
#include "SpxVars.h"
#include "SpxAtomic.h"
#include "SpxPeriodic.h"
#include "SpxThread.h"
#include "SpxObject.h"

void spxPeriodicSleep(int sec,int msec){
    struct timeval timespan;
    timespan.tv_sec = sec;
    timespan.tv_usec = msec * SpxMSecToTimerClock;
    select(0,NULL,NULL,NULL,&timespan);
    return;
}

private var _spxPeriodicRunAsync(var arg);
private var _spxPeriodicExecAndRunAsync(var arg);

private var _spxPeriodicRunAsync(var arg){/*{{{*/
    __SpxTypeConvert(struct SpxPeriodic,t,arg);
    t->_status = SpxperiodicRunning;
    struct timeval timespan;
    do{
        if(__SpxAtomicVCas(t->_status,SpxperiodicSetPausing,SpxperiodicPausing)){
            pthread_mutex_lock(t->_mlock);
            pthread_cond_wait(t->_clock,t->_mlock);
            pthread_mutex_unlock(t->_mlock);
        }
        timespan.tv_sec = t->_sec;
        timespan.tv_usec = t->_usec;
        pthread_testcancel();
        t->err = select(0,NULL,NULL,NULL,&timespan);
        pthread_testcancel();
        if(0 > t->err) {
            __SpxLog2(t->log,SpxLogError,t->err,
                    "async periodic by select is error.");
        } else if(0 == t->err) {
            pthread_testcancel();
            t->_periodicHandler(t->arg);
            pthread_testcancel();
        } else {
            if(!(EAGAIN == errno || EWOULDBLOCK == errno)) {
                __SpxLog2(t->log,SpxLogError,t->err,
                        "async periodic by select is error.");
            }
        }
    }while(t->_isRun);
    return NULL;
}/*}}}*/

private var _spxPeriodicExecAndRunAsync(var arg){/*{{{*/
    __SpxTypeConvert(struct SpxPeriodic,t,arg);
    t->_status = SpxperiodicRunning;
    t->_periodicHandler(t->arg);
    struct timeval timespan;
    do{
        if(__SpxAtomicVCas(t->_status,SpxperiodicSetPausing,SpxperiodicPausing)){
            pthread_mutex_lock(t->_mlock);
            pthread_cond_wait(t->_clock,t->_mlock);
            pthread_mutex_unlock(t->_mlock);
        }
        timespan.tv_sec = t->_sec;
        timespan.tv_usec = t->_usec;
        pthread_testcancel();
        t->err = (select(0,NULL,NULL,NULL,&timespan));
        pthread_testcancel();
        if(0 > t->err) {
            __SpxLog2(t->log,SpxLogError,t->err,
                    "async periodic by select is error.");
        } else if(0 == t->err) {
            pthread_testcancel();
            t->_periodicHandler(t->arg);
            pthread_testcancel();
        } else {
            if(!(EAGAIN == errno || EWOULDBLOCK == errno)) {
                __SpxLog2(t->log,SpxLogError,t->err,
                        "async periodic by select is error.");
            }
        }
    }while(t->_isRun);
    return NULL;
}/*}}}*/

void spxPeriodicRun(SpxLogDelegate *log,
        u32_t secs,u64_t usecs,
        SpxPeriodicDelegate *periodicHandler,
        var arg,
        err_t *err){/*{{{*/
    if(NULL == periodicHandler
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
            __SpxLog2(log,SpxLogError,*err,
                    "periodic by select is error.");
        } else if(0 == *err) {
            periodicHandler(arg);
        } else {
            if(!(EAGAIN == errno || EWOULDBLOCK == errno)) {
                __SpxLog2(log,SpxLogError,*err,
                        "periodic by select is error.");
            }
        }
    }
}/*}}}*/

void spxPeriodicRunOnce(SpxLogDelegate *log,
        u32_t secs,u64_t usecs,
        SpxPeriodicDelegate *periodicHandler,
        var arg,
        err_t *err){/*{{{*/
    if(NULL == periodicHandler
            || (0 == secs && 0 == usecs)){
        *err = EINVAL;
        return;
    }
    struct timeval timespan;
    timespan.tv_sec = secs;
    timespan.tv_usec = usecs;
    *err = (select(0,NULL,NULL,NULL,&timespan));
    if(0 > *err) {
        __SpxLog2(log,SpxLogError,*err,
                "periodic by select is error.");
    } else if(0 == *err) {
        periodicHandler(arg);
    } else {
        if(!(EAGAIN == errno || EWOULDBLOCK == errno)) {
            __SpxLog2(log,SpxLogError,*err,
                    "periodic by select is error.");
        }
    }
}/*}}}*/

void spxPeriodicExecAndRun(SpxLogDelegate *log,
        u32_t secs,u64_t usecs,
        SpxPeriodicDelegate *periodicHandler,
        var arg,
        err_t *err){/*{{{*/
    if(NULL == periodicHandler
            || (0 == secs && 0 == usecs)){
        *err = EINVAL;
        return;
    }
    periodicHandler(arg);
    spxPeriodicRun(log,secs,usecs,periodicHandler,arg,err);
}/*}}}*/


struct SpxPeriodic *spxPeriodicAsyncRun(SpxLogDelegate *log,
        u32_t secs,u64_t usecs,
        SpxPeriodicDelegate *periodicHandler,
        void *arg,
        size_t stacksize,
        err_t *err){/*{{{*/
    if(NULL == periodicHandler
            || (0 == secs && 0 == usecs)){
        *err = EINVAL;
        return NULL;
    }
    struct SpxPeriodic *t =  __SpxObjectNew(struct SpxPeriodic,err);
    if(NULL == t) {
        return NULL;
    }
    t->_isRun = true;
    t->_sec = secs;
    t->_usec = usecs;
    t->log = log;
    t->arg = arg;
    t->_periodicHandler = periodicHandler;
    t->_mlock = spxThreadMutexNew(log,err);
    if(NULL == t->_mlock){
        goto r1;
    }
    t->_clock = spxThreadCondNew(log,err);
    if(NULL == t->_clock){
        goto r1;
    }

    t->_tid = spxThreadNewCancelability(log,
            stacksize,_spxPeriodicRunAsync,(var) t,err);
    if(0 == t->_tid){
        goto r1;
    }
    return t;
r1:
    if(NULL != t->_clock){
        __SpxThreadCondFree(t->_clock);
    }
    if(NULL != t->_mlock){
        __SpxThreadMutexFree(t->_mlock);
    }
    if(NULL != t){
        __SpxObjectFree(t);
    }
    return NULL;
}/*}}}*/

struct SpxPeriodic *spxPeriodicAsyncRunOnce(SpxLogDelegate *log,
        u32_t secs,u64_t usecs,
        SpxPeriodicDelegate *periodicHandler,
        void *arg,
        size_t stacksize,
        err_t *err){/*{{{*/
    if(NULL == periodicHandler
            || (0 == secs && 0 == usecs)){
        *err = EINVAL;
        return NULL;
    }
    struct SpxPeriodic *t = __SpxObjectNew(struct SpxPeriodic,err);
    if(NULL == t) {
        return NULL;
    }
    t->_isRun = false;
    t->_sec = secs;
    t->_usec = usecs;
    t->log = log;
    t->arg = arg;
    t->_periodicHandler = periodicHandler;
    t->_mlock = spxThreadMutexNew(log,err);
    if(NULL == t->_mlock){
        goto r1;
    }
    t->_clock = spxThreadCondNew(log,err);
    if(NULL == t->_clock){
        goto r1;
    }

    t->_tid = spxThreadNewCancelability(log,
            stacksize,_spxPeriodicRunAsync,(var) t,err);
    if(0 == t->_tid){
        goto r1;
    }
    return t;
r1:
    if(NULL != t->_clock){
        __SpxThreadCondFree(t->_clock);
    }
    if(NULL != t->_mlock){
        __SpxThreadMutexFree(t->_mlock);
    }
    if(NULL != t){
        __SpxObjectFree(t);
    }
    return NULL;
}/*}}}*/

struct SpxPeriodic *spxPeriodicAsyncExecAndRun(SpxLogDelegate *log,
        u32_t secs,u64_t usecs,
        SpxPeriodicDelegate *periodicHandler,
        void *arg,
        size_t stacksize,
        err_t *err){/*{{{*/
    if(NULL == periodicHandler
            || (0 == secs && 0 == usecs)){
        *err = EINVAL;
        return NULL;
    }
    struct SpxPeriodic *t = __SpxObjectNew(struct SpxPeriodic,err);
    if(NULL == t) {
        return NULL;
    }
    t->_isRun = true;
    t->_sec = secs;
    t->_usec = usecs;
    t->log = log;
    t->arg = arg;
    t->_periodicHandler = periodicHandler;
    t->_mlock = spxThreadMutexNew(log,err);
    if(NULL == t->_mlock){
        goto r1;
    }
    t->_clock = spxThreadCondNew(log,err);
    if(NULL == t->_clock){
        goto r1;
    }

    t->_tid = spxThreadNewCancelability(log,
            stacksize,_spxPeriodicRunAsync,(var) t,err);
    if(0 == t->_tid){
        goto r1;
    }
    return t;
r1:
    if(NULL != t->_clock){
        __SpxThreadCondFree(t->_clock);
    }
    if(NULL != t->_mlock){
        __SpxThreadMutexFree(t->_mlock);
    }
    if(NULL != t){
        __SpxObjectFree(t);
    }
    return NULL;
}/*}}}*/

struct SpxPeriodic *spxPeriodicExecAndAsyncRun(SpxLogDelegate *log,
        u32_t secs,u64_t usecs,
        SpxPeriodicDelegate *periodicHandler,
        void *arg,
        size_t stacksize,
        err_t *err){/*{{{*/
    if(NULL == periodicHandler
            || (0 == secs && 0 == usecs)){
        *err = EINVAL;
        return NULL;
    }
    periodicHandler(arg);
    return spxPeriodicAsyncRun(log,secs,usecs,periodicHandler,arg,stacksize,err);
}/*}}}*/






bool_t spxPeriodicAsyncSuspend(struct SpxPeriodic *t){/*{{{*/
    return __SpxAtomicVCas(t->_status,SpxperiodicRunning,SpxperiodicSetPausing);
}/*}}}*/

bool_t spxPeriodicAsyncResume(struct SpxPeriodic *t){/*{{{*/
    if(__SpxAtomicVCas(t->_status,SpxperiodicSetPausing,SpxperiodicRunning)){
        return true;
    }else {
        if(__SpxAtomicVCas(t->_status,SpxperiodicPausing,SpxperiodicRunning)){
            pthread_mutex_lock(t->_mlock);
            pthread_cond_signal(t->_clock);
            pthread_mutex_unlock(t->_mlock);
            return true;
        }
    }
    return false;
}/*}}}*/

void spxPeriodicStop(
        struct SpxPeriodic *periodic,
        bool_t isblocking
        ){/*{{{*/
    __SpxAtomicVSet(periodic->_isRun,false);
    pthread_cancel(periodic->_tid);
    if(isblocking){
        pthread_join(periodic->_tid,NULL);
    }
    if(SpxperiodicSetPausing == periodic->_status
            || SpxperiodicPausing == periodic->_status){
        spxPeriodicAsyncResume(periodic);
    }
    pthread_join(periodic->_tid,NULL);
    if(NULL != periodic->_mlock){
        __SpxThreadMutexFree(periodic->_mlock);
    }
    if(NULL != periodic->_clock){
        spxThreadCondFree(periodic->_clock);
    }
    __SpxObjectFree(periodic);
}/*}}}*/

