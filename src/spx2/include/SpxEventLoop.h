/***********************************************************************
 *                              _ooOoo_
 *                             o8888888o
 *                             88" . "88
 *                             (| -_- |)
 *                              O\ = /O
 *                          ____/`---'\____
 *                        .   ' \\| |// `.
 *                         / \\||| : |||// \
 *                       / _||||| -:- |||||- \
 *                         | | \\\ - /// | |
 *                       | \_| ''\---/'' | |
 *                        \ .-\__ `-` ___/-. /
 *                     ___`. .' /--.--\ `. . __
 *                  ."" '< `.___\_<|>_/___.' >'"".
 *                 | | : `- \`.;`\ _ /`;.`/ - ` : | |
 *                   \ \ `-. \_ __\ /__ _/ .-` / /
 *           ======`-.____`-.___\_____/___.-`____.-'======
 *                              `=---='
 *           .............................................
 *                    佛祖镇楼                  BUG辟易
 *            佛曰:
 *                    写字楼里写字间，写字间里程序员；
 *                    程序人员写程序，又拿程序换酒钱。
 *                    酒醒只在网上坐，酒醉还来网下眠；
 *                    酒醉酒醒日复日，网上网下年复年。
 *                    但愿老死电脑间，不愿鞠躬老板前；
 *                    奔驰宝马贵者趣，公交自行程序员。
 *                    别人笑我忒疯癫，我笑自己命太贱；
 *                    不见满街漂亮妹，哪个归得程序员？
 * ==========================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  SpxEventLoop.h
 *        Created:  2015年01月12日 16时40分13秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/
#ifndef _SPXEVENTLOOP_H_
#define _SPXEVENTLOOP_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>

#if SpxEpoll
#include <sys/epoll.h>
#elif SpxKqueue
#endif

#include "SpxTypes.h"

#define SpxEvNone 0x00
#define SpxEvRead 0x01
#define SpxEvWrite 0x02
#define SpxEvTimeout 0x04
#define SpxEvError 0x08

#define SpxWatcherNormal 0x00
#define SpxWatcherInited 0x01
#define SpxWatcherAttached 0x02
#define SpxWatcherPending 0x04
#define SpxWatcherPartOfFired 0x08
#define SpxWatcherFired 0x10
#define SpxWatcherOver 0x20

#define SpxEventLoopDefault 0x00
#define SpxEventLoopWait SpxLoopDefault
#define SpxEventLoopOnce 0x01

#define SpxEventLoopInited 0x01
#define SpxEventLoopRunning 0x02
#define SpxEventLoopBreaked 0x03

#define SpxTimerTick 10
#define SpxTimerSlotCount 10

    struct SpxEventLoop;
    struct SpxWatcher;
    struct SpxAsyncWatcher;
    struct SpxTimerWatcher;

    typedef void SpxWatcherDelegate(struct SpxEventLoop *loop,
            struct SpxWatcher *w,int revents,var arg);
    typedef void SpxAsyncWatcherDelegate(struct SpxEventLoop *loop, struct SpxAsyncWatcher *w,var arg);
    typedef void SpxTimerWatcherDelegate(struct SpxEventLoop *loop, struct SpxTimerWatcher *w,var arg);

    struct SpxWatcher{
        int fd;
        int _events;//events to poll
        u32_t _priority;
        u32_t _mask;//events use SpxEvent
        u32_t _firedMask;
        u32_t _pendingMask;
        u32_t _flags;
        int _egen;

        struct {
            u64_t sec;
            u64_t msec;
            SpxWatcherDelegate *handler;
            struct SpxTimerWatcher *_timer;
            var arg;
        }_r,_w;
        //if the nio with timeout
        //the callback function of the te is null
        struct SpxWatcher *_next;
    };

    struct SpxAsyncWatcher{
        int _pipe[2];
        struct SpxWatcher _watcher;
        bool_t _touched;
        int _flags;
        SpxAsyncWatcherDelegate *_handler;
        var arg;
    };

    struct SpxTimerWatcher {
        u64_t sec;
        u64_t msec;
        u64_t _id;
        u64_t _idx;
        u64_t _basetime;
        u64_t _expired;
        int _flags;
        struct SpxTimerWatcher *_prev;
        struct SpxTimerWatcher *_next;
        SpxTimerWatcherDelegate *_handler;
        var arg;
    };

    struct _SpxTimerSlot{
        u64_t _count;
        u32_t _idx;
        struct SpxTimerWatcher *_header;
        struct SpxTimerWatcher *_tail;
    };

    struct _SpxTimer{
        u64_t _idx;
        u64_t _basetime;
        //the timespan for the timepointer move to next
        //unit is msec
        u64_t _tick;
        u32_t _slotsCount;
        struct _SpxTimerSlot *_header;
        struct _SpxTimerSlot *_current;
    };

    struct SpxEventLoop{
        SpxLogDelegate *log;
        //loop fd
        int _fd;
        //max fired events pre poll
        size_t _maxEvents;

        //the max fd attach into eventpoll
        size_t _maxfd;
        //the ptr-list of attached watchers
        struct SpxWatcher **_watchers;
        struct SpxWatcher *_firedWatchers[10];

        u64_t _tick;

#if SpxEpoll
        struct epoll_event *_events;
#elif SpxKqueue
        struct kevent *_events;
#endif

        struct _SpxTimer *_timer;
        bool_t _isBreak;
        int _flags;
    };

    err_t spxWatcherResize(struct SpxEventLoop *loop, int fd);
    err_t spxWatcherInit(struct SpxWatcher *w,int fd,u32_t priority);
    err_t spxWatcherRelive(struct SpxWatcher *w);
    err_t spxWatcherAddHandler(struct SpxWatcher *w,u32_t mask,u64_t sec,u64_t msec,
            SpxWatcherDelegate *handler,var arg);
    err_t spxWatcherAttach(struct SpxEventLoop *loop,struct SpxWatcher *w);
    err_t spxWatcherRemove(struct SpxEventLoop *loop,int fd,int delmasks);
    err_t spxWatcherDetach(struct SpxEventLoop *loop,int fd);

    void spxTimerWatcherInit(struct SpxTimerWatcher *w,u64_t sec,u64_t msec,
            SpxTimerWatcherDelegate *handler,var arg);
    err_t spxTimerWatcherAttach(struct SpxEventLoop *loop,struct SpxTimerWatcher *w);
    err_t spxTimerWatcherModify(struct SpxEventLoop *loop,struct SpxTimerWatcher *w,
            u64_t sec,u64_t msec,SpxTimerWatcherDelegate *handler,var arg);
    err_t spxTimerWatcherDetach(struct SpxEventLoop *loop, struct SpxTimerWatcher *w);


    err_t spxAsyncWatcherInit(struct SpxAsyncWatcher *w,u32_t priority,SpxAsyncWatcherDelegate *handler,var arg);
    err_t spxAsyncWatcherAttach(struct SpxEventLoop *loop,struct SpxAsyncWatcher *w);
    err_t spxAsyncWatcherDetach(struct SpxEventLoop *loop,struct SpxAsyncWatcher *w);
    void spxAsyncWatcherClear(struct SpxAsyncWatcher *w);

    struct SpxEventLoop *spxEventLoopNew(SpxLogDelegate *log,
            size_t maxEvents,err_t *err);
    int spxEventLoopSuspend(struct SpxEventLoop *loop);
    int spxEventLoopResume(struct SpxEventLoop *loop);
    void spxEventLoopBreak(struct SpxEventLoop *loop);

    err_t spxEventLoopStart(struct SpxEventLoop *loop,int how);

    bool_t spxEventLoopFree(struct SpxEventLoop *loop);

#define __SpxEventLoopFree(loop) \
    do {\
        if(NULL != loop && spxEventLoopFree(loop)) {\
            loop = NULL; \
        } \
    }while(false)

#ifdef __cplusplus
}
#endif
#endif
