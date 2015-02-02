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
 *       Filename:  SpxModule.h
 *        Created:  2015年01月21日 17时05分55秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/
#ifndef _SPXMODULE_H_
#define _SPXMODULE_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "SpxTypes.h"
#include "SpxEventLoop.h"
#include "SpxList.h"

    struct SpxModuleThreadContext{
        struct SpxEventLoop *loop;
        //in here,loop and watcher can in the same struct
        //because they are the 1:1,and notify with pipe
        //you can read with loop while no the data attach again
        struct SpxWatcher watcher;
        SpxWatcherDelegate *handler;
        SpxLogDelegate *log;
        size_t _idx;
        pthread_t _tid;
        int _pipe[2];
    };

    struct SpxModuleContext{
        struct SpxList *_threadpool;//struct SpxModuleThreadContext
        size_t _threadSize;
        SpxLogDelegate *log;
    };

struct SpxModuleContext *spxModuleNew(
        SpxLogDelegate *log,
        u32_t threadSize,
        size_t stackSize,
        size_t maxEvents,
        SpxWatcherDelegate *handler,
        err_t *err);

    err_t _spxModuleFree(struct SpxModuleContext *smc);
    err_t spxModuleThreadWakeup(struct SpxModuleThreadContext *smtc,var arg);
    struct SpxModuleThreadContext *spxModuleGetThreadContext(struct SpxModuleContext *smc,size_t idx);

#ifdef __cplusplus
}
#endif
#endif
