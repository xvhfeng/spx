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
 *       Filename:  SpxEvEpoll.c
 *        Created:  2015年01月15日 10时55分24秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/epoll.h>

#include "SpxTypes.h"
#include "SpxError.h"
#include "SpxMFunc.h"
#include "SpxEventLoop.h"
#include "SpxAlloc.h"
#include "SpxObject.h"

err_t _spxCreateEvent(struct SpxEventLoop *loop){/*{{{*/
    err_t err = 0;
    loop->_fd = epoll_create(1024);
    if(-1 == loop->_fd){
        return errno;
    }
    loop->_events = spxObjectNewNumbs(loop->_maxEvents,sizeof(struct epoll_event),&err);
    if(NULL == loop->_events){
        __SpxClose(loop->_fd);
    }
    return err;
}/*}}}*/

err_t spxWatcherAttach(struct SpxEventLoop *loop,
        struct SpxWatcher *w){/*{{{*/
    if(NULL == loop || NULL == w){
        return EINVAL;
    }

    //that can not add watcher when flags is pendig or fired
    if((SpxWatcherPending & w->_flags)
            || (SpxWatcherFired & w->_flags)){
        return SpxEFlags;
    }

    err_t err = 0;
    int fd = w->fd;

    if(0 != (err = spxWatcherResize(loop,fd))){
        return err;
    }

    struct epoll_event ev;
    w->_events = w->_events | EPOLLET
        | (SpxEvRead & w->_mask) ? EPOLLIN : 0
        | (SpxEvWrite & w->_mask) ? EPOLLOUT : 0;
    ev.events = w->_events;

    ev.data.u64 = (uint64_t)(uint32_t)fd
        | ((uint64_t)(uint32_t)++ w->_egen << 32);

    if(-1 == epoll_ctl (loop->_fd,
                SpxWatcherAttached & w->_flags ? EPOLL_CTL_MOD : EPOLL_CTL_ADD,
                fd, &ev)){
        err = errno;
        __SpxLogFmt2(loop->log,SpxLogError,err,
                "attach events:%d fd:%d to epoll is fail.",
                w->_events,fd);
        return err;
    }

    if(SpxEvRead & w->_mask){
        if(0 != w->_r.sec || 0 != w->_r.msec){
            w->_r._timer = spxObjectNew(sizeof(struct SpxTimerWatcher),&err);
            if(NULL == w->_r._timer){
                epoll_ctl (loop->_fd,EPOLL_CTL_DEL, fd, &ev);
                goto r1;
            }
            spxTimerWatcherInit(w->_r._timer,w->_r.sec,w->_r.msec,NULL,w);
            err = spxTimerWatcherAttach(loop,w->_r._timer);
            if(0 != err){
                __SpxLog2(loop->log,SpxLogError,err,
                        "add fd:%d timeout watcher for reader is fail.");
                epoll_ctl (loop->_fd,EPOLL_CTL_DEL, fd, &ev);
                goto r1;
            }
        }
    }
    if(SpxEvWrite & w->_mask){
        if(0 != w->_w.sec || 0 != w->_w.msec){
            w->_w._timer = spxObjectNew(sizeof(struct SpxTimerWatcher),&err);
            if(NULL == w->_w._timer){
                epoll_ctl (loop->_fd,EPOLL_CTL_DEL, fd, &ev);
                goto r1;
            }
            spxTimerWatcherInit(w->_w._timer,w->_w.sec,w->_w.msec,NULL,w);
            err = spxTimerWatcherAttach(loop,w->_w._timer);
            if(0 != err){
                __SpxLog2(loop->log,SpxLogError,err,
                        "add fd:%d timeout watcher for reader is fail.");
                epoll_ctl (loop->_fd,EPOLL_CTL_DEL, fd, &ev);
                goto r1;
            }
        }
    }
    w->_flags = SpxWatcherAttached;
    loop->_watchers[fd] = w;
    return 0;
r1:
    if(NULL != w->_r._timer && SpxWatcherAttached == w->_r._timer->_flags){
        spxTimerWatcherDetach(loop,w->_r._timer);
    }
    if(NULL != w->_w._timer && SpxWatcherAttached == w->_w._timer->_flags){
        spxTimerWatcherDetach(loop,w->_w._timer);
    }
    return err;
}/*}}}*/

err_t spxWatcherRemove(struct SpxEventLoop *loop,int fd,int delmasks){/*{{{*/
    struct SpxWatcher *w = loop->_watchers[fd];
    if(NULL == w){
        return EINVAL;
    }

    if(SpxWatcherAttached != w->_flags
            || SpxWatcherPartOfFired != w->_flags){
        return SpxEFlags;
    }

    //the watcher is fired
    if(delmasks & w->_firedMask){
        return SpxEOptr;
    }

    int mask = w->_mask & (~delmasks);

    struct epoll_event ev;
    __SpxZero(ev);
    int events = EPOLLET
        | (SpxEvRead & w->_mask) ? EPOLLIN : 0
        | (SpxEvWrite & w->_mask) ? EPOLLOUT : 0;
    ev.events = events;
    w->_events = events;
    ev.data.u64 = (uint64_t)(uint32_t)fd
        | ((uint64_t)(uint32_t)w->_egen << 32);
    if (mask != SpxEvNone) {
        epoll_ctl(loop->_fd,EPOLL_CTL_MOD,fd,&ev);
    } else {
        epoll_ctl(loop->_fd,EPOLL_CTL_DEL,fd,&ev);
    }
    if((SpxEvRead & delmasks) && NULL != w->_r._timer
            && SpxWatcherAttached == w->_r._timer->_flags){
        spxTimerWatcherDetach(loop,w->_r._timer);
        __SpxObjectFree(w->_r._timer);
        w->_r._timer = NULL;
    }
    if((SpxEvWrite & delmasks) && NULL != w->_w._timer
            && SpxWatcherAttached == w->_w._timer->_flags){
        spxTimerWatcherDetach(loop,w->_w._timer);
        __SpxObjectFree(w->_w._timer);
        w->_w._timer = NULL;
    }
    return 0;

}/*}}}*/

err_t spxWatcherDetach(struct SpxEventLoop *loop,int fd){/*{{{*/
    struct SpxWatcher *w = loop->_watchers[fd];
    if(NULL == w){
        return EINVAL;
    }

    if(SpxWatcherAttached != w->_flags
            || SpxWatcherPartOfFired != w->_flags){
        return SpxEFlags;
    }

    if((SpxEvRead & w->_mask) && NULL != w->_r._timer
            && SpxWatcherAttached == w->_r._timer->_flags){
        spxTimerWatcherDetach(loop,w->_r._timer);
        __SpxObjectFree(w->_r._timer);
        w->_r._timer = NULL;
    }
    if((SpxEvWrite & w->_mask) && NULL != w->_w._timer
            && SpxWatcherAttached == w->_w._timer->_flags){
        spxTimerWatcherDetach(loop,w->_w._timer);
        __SpxObjectFree(w->_w._timer);
        w->_w._timer = NULL;
    }
    epoll_ctl(loop->_fd, EPOLL_CTL_DEL, fd, NULL);
    w->_flags = SpxWatcherOver;
    w->_events = 0;
    w->_mask = SpxEvNone;
    return 0;
}/*}}}*/

err_t _spxWatcherWait(struct SpxEventLoop *loop,int *fds){/*{{{*/
    if(NULL == loop){
        return EINVAL;
    }
    *fds = 0;
    *fds = epoll_wait(loop->_fd, loop->_events, loop->_maxEvents, loop->_tick);
    if(0 == *fds){
        return EAGAIN;
    }
    int i = 0;
    struct SpxWatcher *tail[10] = {0};
    for( ; i < *fds; i++) {
        struct epoll_event *ev = loop->_events + i;
        int fd = (uint32_t)ev->data.u64; /*  mask out the lower 32 bits */
        struct SpxWatcher *w = loop->_watchers[fd];
        if(NULL == w) {
            continue;
        }
        loop->_watchers[fd] = NULL;
        if((uint32_t)w->_egen != (uint32_t)(ev->data.u64 >> 32)) {
            continue;
        }
        int got  = (ev->events & (EPOLLOUT | EPOLLERR | EPOLLHUP) ? SpxEvWrite : 0)
            | (ev->events & (EPOLLIN  | EPOLLERR | EPOLLHUP) ? SpxEvRead  : 0);

        //if in-event is attached and the out-event is attached in the same time
        if(w->_mask & ~got){
            continue;
        }

        w->_pendingMask = got;
        w->_firedMask |= got;

        if((SpxEvRead & got) && NULL != w->_r._timer
                && SpxWatcherAttached == w->_r._timer->_flags){
            spxTimerWatcherDetach(loop,w->_r._timer);
            __SpxObjectFree(w->_r._timer);
            w->_r._timer = NULL;
        }
        if((SpxEvWrite & got) && NULL != w->_w._timer
                && SpxWatcherAttached == w->_w._timer->_flags){
            spxTimerWatcherDetach(loop,w->_w._timer);
            __SpxObjectFree(w->_w._timer);
            w->_w._timer = NULL;
        }

        w->_flags = SpxWatcherPending;
        int idx = w->_priority - 1;
        if(NULL == tail[idx]){
            loop->_firedWatchers[idx] = w;
        } else {
            tail[idx]->_next = w;
        }
        tail[idx] = w;
    }
    return 0;
}/*}}}*/


