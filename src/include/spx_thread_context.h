/*
 * =====================================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  spx_thread_context.h
 *        Created:  2014/07/22 16时09分11秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 * =====================================================================================
 */
#ifndef _SPX_THREAD_CONTEXT_H_
#define _SPX_THREAD_CONTEXT_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <ev.h>

#include "spx_types.h"
#include "spx_list.h"
#include "spx_fixed_vector.h"


typedef void (SpxNoticeDelegate)(struct ev_loop *loop,ev_io *watcher,int revents);

struct spx_thread_context{
    size_t idx;
    pthread_t tid;
    struct ev_loop *loop;
    SpxLogDelegate *log;
    int pipe[2];
};

struct spx_notice{
    ev_io watcher;
    SpxNoticeDelegate *notify_handler;
    size_t idx;
    SpxLogDelegate *log;
};

struct spx_notice_context{
    SpxLogDelegate *log;
    struct spx_list *thread_contexts;
    //usually,receive notification watcher size is equal to thread size
    //but dispatch notice watcher size maybe hanving to more than thread size,
    //because,receive watcher follow loop and banding with loop by 1:1
    //dispatch watcher is maybe n:1 banding with loop when loop is busy
    //so we set the n equal 2,donot ask me why,this is a magic
    struct spx_list *receive_notification_watchers;
    struct spx_fixed_vector *dispatch_notice_watchers;
};

//today is 2014-07-22,I say to xj about the ydb work over this month,
//but now,I make a mistaken and redo thread-notify,so I can not over the work.
//if he kown this,he want to kill me.is not it??

struct spx_notice_context *spx_notice_context_new(\
        SpxLogDelegate *log,\
        u32_t threadsize,\
        size_t stack_size,\
        SpxNoticeDelegate *dispatch_notice_handler,\
        SpxNoticeDelegate *receive_notification_handler,\
        err_t *err);

err_t spx_notice_context_free(struct spx_notice_context **nc);

err_t spx_dispatch_notice(struct spx_notice_context *nc,size_t idx,void *msg);

#ifdef __cplusplus
}
#endif
#endif
