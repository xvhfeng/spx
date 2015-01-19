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

#include "SpxTypes.h"

#define SpxEvNone 0x00
#define SpxEvRead 0x01
#define SpxEvWrite 0x02
#define SpxEvTimeout 0x04
#define SpxEvError 0x08

#define SpxWatcherInited 0x01
#define SpxWatcherAttached 0x02
#define SpxWatcherPending 0x04
#define SpxWatcherFired 0x08
#define SpxWatcherOver 0x10

#define SpxEventLoopDefault 0x00
#define SpxEventLoopWait SpxLoopDefault
#define SpxEventLoopOnce 0x01

#define SpxTimerTick 10
#define SpxTimerSlotCount 10

    struct SpxEventLoop;
    struct SpxWatcher;
    struct SpxAsyncWatcher;
    struct SpxTimerWatcher;

    typedef void SpxWatcherDelegate(struct SpxEventLoop *loop,
            int fd,int events,struct SpxWatcher *w);
    typedef void SpxAsyncWatcherDelegate(struct SpxEventLoop *loop, struct SpxAsyncWatcher *w);
    typedef void SpxTimerWatcherDelegate(struct SpxEventLoop *loop, struct SpxTimerWatcher *w);

    struct SpxWatcher{
        int fd;
        u64_t sec;
        u64_t msec;
        int _flags;
        int _events;//events to poll
        int _marks;//events use SpxEvent
        int _firedMarks;
        int _egen;
        int _priority;
        SpxWatcherDelegate *_handler;
        //if the nio with timeout
        //the callback function of the te is null
        struct SpxTimerWatcher *_timer;
        struct SpxWatcher *_next;
        var arg;
    };

    struct SpxAsyncWatcher{
        int _pipe[2];
        struct SpxWatcher _watcher;
        bool_t _touched;
        int _flags;
        SpxAsyncWatcherDelegate *_handler;
        var *arg;
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
    };

    err_t spxWatcherResize(struct SpxEventLoop *loop, int fd);
    err_t spxWatcherInit(struct SpxWatcher *w,
            int fd,u64_t sec,u64_t msec,
            int marks,SpxWatcherDelegate *handler,var arg);
    void spxWatcherSetPriority(struct SpxWatcher *w,u32_t priority);
    err_t spxWatcherAttach(struct SpxEventLoop *loop,struct SpxWatcher *w);
    err_t spxWatcherDetach(struct SpxEventLoop *loop,int fd);

    void spxTimerWatcherInit(struct SpxTimerWatcher *w,u64_t sec,u64_t msec,
            SpxTimerWatcherDelegate *handler,var arg);

    err_t spxTimerWatcherAttach(struct SpxEventLoop *loop,struct SpxTimerWatcher *w);
    err_t spxTimerWatcherModify(struct SpxEventLoop *loop,struct SpxTimerWatcher *w,
            u64_t sec,u64_t msec,SpxTimerWatcherDelegate *handler,var arg);
    err_t spxTimerWatcherDetach(struct SpxEventLoop *loop, struct SpxTimerWatcher *w);



    struct SpxEventLoop *SpxEventLoopNew();
    int SpxEventLoopSuspend(struct SpxEventLoop *loop);
    int SpxEventLoopResume(struct SpxEventLoop *loop);
    int SpxEventLoopBreak(struct SpxEventLoop *loop,int how);
    int SpxEventLoopRunning(struct SpxEventLoop *loop,int flags);


#ifdef __cplusplus
}
#endif
#endif
