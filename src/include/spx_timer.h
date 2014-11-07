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
 *       Filename:  spx_timer.h
 *        Created:  2014/10/22 10时42分36秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *
 ****************************************************************************/
#ifndef _SPX_TIMER_H_
#define _SPX_TIMER_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include"spx_types.h"

    /**
     * Remark:this timer is not acuracy,because timer and callback
     * in the same poll,and if callback is blocking or the time-conuming
     * handler,such as:disk-io,network io,bigger-compute...
     * they are all need more cpu time to deal.
     * so if you want to the timer become acuracy,you maybe having noblocking.
     * and if you want to control everyting and you are not er-ba-dao in C,
     * you can use spx_sleep or naked select DIY.
     * well,usually,the timer timeout is later than the appointed time.
     * if you use functions by named *aync*,you will create a pthread
     *  to deal async.
     * */

    typedef void *(SpxExpiredDelegate)(void *arg);

#define SpxTimerRunning 0
#define SpxTimerSetPausing 1
#define SpxTimerPausing 2

    struct spx_timer{
        SpxLogDelegate *log;
        SpxExpiredDelegate *timeout_handler;
        u32_t sec;
        u64_t usec;
        err_t err;
        bool_t is_run;
        i32_t status;
        void *arg;
        pthread_t tid;
        pthread_mutex_t *mlock;
        pthread_cond_t *clock;
    };


    void spx_sleep(int sec,int usec);

    void spx_timer_run(SpxLogDelegate *log,
            u32_t secs,u64_t usecs,
            SpxExpiredDelegate *timeout_handler,
            void *arg,
            err_t *err);

    struct spx_timer *spx_timer_async_run(SpxLogDelegate *log,
            u32_t secs,u64_t usecs,
            SpxExpiredDelegate *timeout_handler,
            void *arg,
            size_t stacksize,
            err_t *err);

    void spx_timer_run_once(SpxLogDelegate *log,
            u32_t secs,u64_t usecs,
            SpxExpiredDelegate *timeout_handler,
            void *arg,
            err_t *err);

    struct spx_timer *spx_timer_async_run_once(SpxLogDelegate *log,
            u32_t secs,u64_t usecs,
            SpxExpiredDelegate *timeout_handler,
            void *arg,
            size_t stacksize,
            err_t *err);

    void spx_timer_exec_and_run(SpxLogDelegate *log,
            u32_t secs,u64_t usecs,
            SpxExpiredDelegate *timeout_handler,
            void *arg,
            err_t *err);

    struct spx_timer *spx_timer_async_exec_and_run(SpxLogDelegate *log,
            u32_t secs,u64_t usecs,
            SpxExpiredDelegate *timeout_handler,
            void *arg,
            size_t stacksize,
            err_t *err);

    struct spx_timer *spx_timer_exec_and_async_run(SpxLogDelegate *log,
            u32_t secs,u64_t usecs,
            SpxExpiredDelegate *timeout_handler,
            void *arg,
            size_t stacksize,
            err_t *err);


    /**
     * pausing the timer at next poll
     */
bool_t spx_timer_async_suspend(struct spx_timer *t);
bool_t spx_timer_async_resume(struct spx_timer *t);
void spx_timer_stop(struct spx_timer **timer);

#ifdef __cplusplus
}
#endif
#endif
