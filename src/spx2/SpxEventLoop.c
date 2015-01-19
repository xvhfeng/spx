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
 *       Filename:  SpxEventLoop.c
 *        Created:  2015年01月12日 17时25分14秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "SpxMFunc.h"
#include "SpxError.h"
#include "SpxVars.h"
#include "SpxDateTime.h"
#include "SpxAtomic.h"
#include "SpxObject.h"
#include "SpxEventLoop.h"

private SpxInline u64_t _spxTimerClock();
private SpxInline u64_t _spxTimerClockConvert(u64_t sec,u64_t msecs);
private struct _SpxTimer *_spxTimerNew(SpxLogDelegate *log,
        u64_t tick, u32_t slotsCount,err_t *err);
private struct SpxTimerWatcher *_spxTimerRunning(
        struct _SpxTimer *timer,u32_t *count);
private err_t _spxTimerFree(struct _SpxTimer **timer);
private void _spxAsyncWatcherHandler(struct SpxEventLoop *loop,
            int fd,int events,struct SpxWatcher *w);

//------------io---------------
#if SpxEpoll
#include "_SpxEvEpoll.c"
#elif SpxKqueue
#include "_SpxEvKqueue.c"
#endif

err_t spxWatcherInit(struct SpxWatcher *w,
        int fd,u64_t sec,u64_t msec,
        int marks,SpxWatcherDelegate *handler,var arg){/*{{{*/
    if(NULL == w || 0 > fd || 0 > marks){
        return EINVAL;
    }
    if(SpxEvNone & marks){
        return ENOENT;
    }
    w->_marks = SpxEvNone;
    w->fd = fd;
    w->_events = 0;
    w->_priority = 5;
    if(SpxEvRead & marks){
        w->_handler = handler;
        w->arg = arg;
    }
    if(SpxEvWrite & marks){
        w->_handler = handler;
        w->arg = arg;
    }
    w->sec = sec;
    w->msec = msec;
    if(0 != sec || 0 != msec){
        w->_marks |= SpxEvTimeout;
    }
    w->_flags = SpxWatcherInited;
    return 0;
}/*}}}*/

void spxWatcherSetPriority(struct SpxWatcher *w,u32_t priority){/*{{{*/
    if(NULL == w){
        return;
    }
    w->_priority = 0 == priority
        ? 5
        : 10 <= priority
        ? 10
        : priority;
}/*}}}*/

err_t spxWatcherResize(struct SpxEventLoop *loop, int fd) {/*{{{*/
    err_t err = 0;
    if(loop->_maxfd <(size_t) fd){
        struct SpxWatcher **l = (struct SpxWatcher **)
            spxObjectReNewNumbs(loop->_watchers,2 * fd,sizeof(struct SpxWatcher *),&err);
        if(NULL == l){
            __SpxLog2(loop->log,SpxLogError,err,
                    "renew wahcter-pointers is fail.");
            return err;
        }
        loop->_watchers = l;
        loop->_maxfd = 2 * fd;
    }
    return 0;
}/*}}}*/

//------------io---------------
#include "SpxEventLoop.h"

//------------timer---------------
private SpxInline u64_t _spxTimerClock(){/*{{{*/
    u64_t usecs = spxClock();
    return (u64_t) usecs / SpxMSecToTimerClock;
}/*}}}*/

private SpxInline u64_t _spxTimerClockConvert(u64_t sec,u64_t msecs){/*{{{*/
    return sec * 1000 + msecs;
}/*}}}*/

void spxTimerWatcherInit(struct SpxTimerWatcher *w,u64_t sec,u64_t msec,
        SpxTimerWatcherDelegate *handler,var arg){/*{{{*/
    w->sec = sec;
    w->msec = msec;
    w->_expired = _spxTimerClockConvert(sec,msec);
    w->arg = arg;
    w->_handler = handler;
    w->_flags = SpxWatcherInited;
    return;
}/*}}}*/

err_t spxTimerWatcherAttach(struct SpxEventLoop *loop,struct SpxTimerWatcher *w){/*{{{*/
    if(NULL == w){
        return EINVAL;
    }
    if(NULL == loop || NULL == loop->_timer){
        return EINVAL;
    }

    if(SpxWatcherInited != w->_flags){
        return SpxEFlags;
    }

    struct _SpxTimer *timer = loop->_timer;
    u64_t now = _spxTimerClock();
    u64_t expired = w->_expired;
    u32_t idx = 0 == expired ? timer->_current->_idx
        : ((0 == expired % timer->_tick
                    ? expired / timer->_tick
                    : expired / timer->_tick + 1)  + timer->_current->_idx )
        % timer->_slotsCount;

    struct _SpxTimerSlot *s = timer->_header + idx;
    w->_basetime = now;
    w->_id = __SpxAtomicLazyVIncr(timer->_idx);
    w->_idx = idx;
    if(NULL == s->_header){
        s->_header = w;
        s->_tail = w;
    } else {
        w->_prev = s->_tail;
        s->_tail->_next = w;
        s->_tail= w;
    }
    w->_flags = SpxWatcherAttached;
    __SpxAtomicVIncr(s->_count);
    return 0;
}/*}}}*/

err_t spxTimerWatcherModify(struct SpxEventLoop *loop,struct SpxTimerWatcher *w,
        u64_t sec,u64_t msec,SpxTimerWatcherDelegate *handler,var arg){/*{{{*/
    if(NULL == w){
        return EINVAL;
    }
    if(NULL == loop || NULL == loop->_timer){
        return EINVAL;
    }

    if(SpxWatcherPending == w->_flags
            || SpxWatcherFired == w->_flags) {
        return SpxEFlags;
    }
    //not attach to timers
    if(SpxWatcherInited == w->_flags){
        w->_handler = handler;
        w->sec = sec;
        w->msec = msec;
        w->arg = arg;
        return spxTimerWatcherAttach(loop,w);
    }

    struct _SpxTimer *timer = loop->_timer;
    u64_t now = _spxTimerClock();
    u64_t expired ;
    if(0 == sec && 0 == msec){
        expired = 0;
    } else {
        expired = _spxTimerClockConvert(sec,msec);
    }
    u32_t idx = 0;
    idx = 0 == expired ? timer->_current->_idx
        : ((0 == expired % timer->_tick
                    ? expired / timer->_tick
                    : expired / timer->_tick + 1)  + timer->_current->_idx )
        % timer->_slotsCount;

    u32_t oidx = w->_idx;
    w->_basetime = now;
    w->_expired = expired;
    if(oidx != idx){
        struct _SpxTimerSlot *s = timer->_header + oidx;
        if(NULL == w->_prev){
            s->_header = w->_next;
        } else {
            w->_prev->_next = w->_next;
        }
        if(NULL == w->_next){
            s->_tail = w->_prev;
        } else {
            w->_next->_prev = w->_prev;
        }
        __SpxAtomicVDecr(s->_count);

        w->_idx = idx;
        s = timer->_header + idx;
        if(NULL == s->_header){
            s->_header = w;
            s->_tail = w;
        } else {
            w->_prev = s->_tail;
            s->_tail->_next = w;
            s->_tail= w;
        }
        __SpxAtomicVIncr(s->_count);
    }
    return 0;
}/*}}}*/

err_t spxTimerWatcherDetach(struct SpxEventLoop *loop, struct SpxTimerWatcher *w){/*{{{*/
    if(NULL == w){
        return EINVAL;
    }
    if(NULL == loop || NULL == loop->_timer){
        return EINVAL;
    }

    if(SpxWatcherPending == w->_flags
            || SpxWatcherFired == w->_flags) {
        return SpxEFlags;
    }
    struct _SpxTimer *timer = loop->_timer;
    struct _SpxTimerSlot *s = timer->_header + w->_idx;
    if(NULL == w->_prev){
        s->_header = w->_next;
    } else {
        w->_prev->_next = w->_next;
    }
    if(NULL == w->_next){
        s->_tail = w->_prev;
    } else {
        w->_next->_prev = w->_prev;
    }
    __SpxAtomicVDecr(s->_count);
    return 0;
}/*}}}*/

private struct _SpxTimer *_spxTimerNew(SpxLogDelegate *log,
        u64_t tick,u32_t slotsCount,err_t *err){/*{{{*/
    struct _SpxTimer *t = (struct _SpxTimer *) spxObjectNew(sizeof(*t),err);
    if(NULL == t){
        __SpxLog2(log,SpxLogError,*err,
                "alloc timer is fail.");
        return NULL;
    }
    t->_slotsCount = slotsCount;
    t->_tick = tick;
    t->_basetime = _spxTimerClock();
    t->_header = spxObjectNewNumbs(slotsCount,sizeof(struct _SpxTimerSlot),err);
    if(NULL == t->_header){
        __SpxLog2(log,SpxLogError,*err,
                "alloc slots for timer is fail.");
        goto r1;
    }
    t->_current = t->_header;
    u32_t i = 0;
    for( ; i < slotsCount; i++){
        struct _SpxTimerSlot *s = t->_header + i;
        s->_header = NULL;
        s->_idx = i;
        s->_count = 0;
    }
    return t;
r1:
    __SpxObjectFree(t);
    return NULL;

}/*}}}*/

private struct SpxTimerWatcher *_spxTimerRunning(struct _SpxTimer *timer,u32_t *count){/*{{{*/
    u64_t basetime = _spxTimerClock();
    struct SpxTimerWatcher *l = NULL;
    struct SpxTimerWatcher *lt = NULL;
    struct SpxTimerWatcher *w = NULL;
    struct _SpxTimerSlot *s = timer->_current;
    struct SpxTimerWatcher *p = s->_header;
    while(NULL != (w = p)){
        if(basetime >= w->_basetime + w->_expired){
            if(NULL == w->_prev){
                s->_header = w->_next;
            } else {
                w->_prev->_next = w->_next;
            }
            if(NULL == w->_next){
                s->_tail = w->_prev;
            } else {
                w->_next->_prev = w->_prev;
            }
            w->_prev = NULL;
            w->_next = NULL;
            if(NULL == l){
                l = w;
                lt = w;
            } else {
                w->_prev = lt;
                lt->_next = w;
                lt = w;
            }
            w->_flags = SpxWatcherPending;
            __SpxAtomicVDecr(s->_count);
        }
        p = p->_next;
    }

    timer->_current = timer->_header
        + (timer->_current->_idx + 1) % timer->_slotsCount;
    return l;
}/*}}}*/

private err_t _spxTimerFree(struct _SpxTimer **timer){/*{{{*/
    if(NULL == timer || NULL == *timer){
        return 0;
    }
    __SpxObjectFree((*timer)->_header);
    __SpxObjectFree(*timer);
    return 0;
}/*}}}*/

//------------timer---------------

//------------async---------------
private void _spxAsyncWatcherHandler(struct SpxEventLoop *loop,
            int fd,int events,struct SpxWatcher *w){/*{{{*/
    if(NULL == w || NULL == w->_handler){
        return;
    }
    if(SpxEvRead & events){
        struct SpxAsyncWatcher *aw = w->arg;
        if(NULL == aw || NULL == aw->_handler){
            return;
        }
        aw->_handler(loop,aw);
    }
}/*}}}*/

err_t spxAsyncWatcherInit(struct SpxAsyncWatcher *w,u32_t priority,SpxAsyncWatcherDelegate *handler,var arg){/*{{{*/
    if(NULL == w){
        return EINVAL;
    }
    err_t err = 0;
    w->_handler = handler;
    w->arg = arg;
    w->_flags = SpxWatcherInited;
    if(0 != pipe(w->_pipe)){
        return errno;
    }
    if(0 != (err = spxWatcherInit(&(w->_watcher),w->_pipe[1],0,0,SpxEvRead,_spxAsyncWatcherHandler,w))){
        __SpxClose(w->_pipe[0]);
        __SpxClose(w->_pipe[1]);
    }
    if(0 != priority) {
        spxWatcherSetPriority(&(w->_watcher),priority);
    }
    return 0;

}/*}}}*/

err_t spxAsyncWatcherAttach(struct SpxEventLoop *loop,struct SpxAsyncWatcher *w){/*{{{*/
    if(NULL == loop || NULL == w){
        return EINVAL;
    }
    return spxWatcherAttach(loop,&(w->_watcher));
}/*}}}*/

err_t spxAsyncWatcherTouch(struct SpxEventLoop *loop,struct SpxAsyncWatcher *w) {/*{{{*/
    if(w->_touched){
        return SpxEOptr;
    }
    if(__SpxAtomicVIsCas(w->_touched,false,true)){
        if(sizeof(w) != (write(w->_pipe[0],w,sizeof(w)))){
            __SpxAtomicVCas(w->_touched,true,false);
        }
    }
    return 0;
}/*}}}*/

err_t spxAsyncWatcherDetach(struct SpxEventLoop *loop,struct SpxAsyncWatcher *w){/*{{{*/
    if(SpxWatcherPending == w->_flags
            || SpxWatcherFired == w->_flags){
        return SpxEOptr;
    }
    spxWatcherDetach(loop,w->_pipe[1]);
    w->_touched = 0;
    w->_flags = SpxWatcherInited;
    return 0;
}/*}}}*/

void spxAsyncWatcherClear(struct SpxAsyncWatcher *w){/*{{{*/
    __SpxClose(w->_pipe[0]);
    __SpxClose(w->_pipe[1]);
}/*}}}*/

//------------async---------------


struct SpxEventLoop *SpxEventLoopNew(
        SpxLogDelegate *log,
        size_t maxEvents,err_t *err){/*{{{*/
    if(0 == maxEvents){
        *err = EINVAL;
        return NULL;
    }

    struct SpxEventLoop *loop = spxObjectNew(sizeof(*loop),err);
    if(NULL == loop){
        __SpxLog2(log,SpxLogError,*err,
                "new eventloop object is fail.");
        return NULL;
    }

    loop->_tick = SpxTimerTick;
    loop->_maxEvents = maxEvents;
    loop->log = log;
    loop->_isBreak = false;

    if(0 != (*err = _spxCreateEvent(loop))) {
        __SpxObjectFree(loop);
        __SpxLog2(log,SpxLogError,*err,
                "create events of eventloop is fail.");
        return NULL;
    }

    loop->_timer = _spxTimerNew(log,SpxTimerTick,SpxTimerSlotCount,err);
    if(NULL == loop->_events){
        *err = 0 == errno ? SpxECreateEventLoop : errno;
        __SpxClose(loop->_fd);
        __SpxObjectFree(loop);
        __SpxObjectFree(loop->_events);
        __SpxLog2(log,SpxLogError,*err,
                "create timer of eventloop is fail.");
        return NULL;
    }

    return loop;
}/*}}}*/

int SpxEventLoopRunning(struct SpxEventLoop *loop,int how){/*{{{*/
    u32_t count = 0;
    err_t err = 0;
    int fds = 0;
    do{
        err =  _spxWatcherWait(loop,&fds);
        if(0 != err && EAGAIN != err){
            __SpxLog2(loop->log,SpxLogError,err,
                    "wait event loop is fail."
                    "and break the loop,leave the timeout and async.");
            break;
        }
        int i = 0;
        for( ; i < 10; i++){
            struct SpxWatcher *w = loop->_firedWatchers[i];
            while(NULL != w){
                w->_flags = SpxWatcherFired;
                w->_handler(loop,w->fd,w->_firedMarks,w);
                w = w->_next;
            }
        }

        if(NULL != loop->_timer) {
            struct SpxTimerWatcher *timers = _spxTimerRunning(loop->_timer, &count);
            struct SpxTimerWatcher *timer = NULL;
            struct SpxTimerWatcher *header = timers;
            while(NULL != (timer = header)){
                if(NULL == timer->_handler){
                    //delete event from kqueue/epoll
                    struct SpxWatcher *w = timer->arg;
                    if(SpxEvTimeout & w->_marks) {
                        spxWatcherDetach(loop,w->fd);
                        w->_flags = SpxWatcherFired;
                        w->_handler(loop,w->fd,SpxEvTimeout,w);
                    }
                } else {
                    timer->_flags = SpxWatcherFired;
                    timer->_handler(loop,timer);
                }
                header = timer->_next;
            }
        }

    }while(loop->_isBreak && (!(how & SpxEventLoopOnce)));
    return 0;
}/*}}}*/


void SpxEventLoopFree(struct SpxEventLoop *loop){/*{{{*/
    __SpxClose(loop->_fd);
    _spxTimerFree(&(loop->_timer));
    __SpxObjectFree(loop);
    return;
}/*}}}*/

