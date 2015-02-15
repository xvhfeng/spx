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
 *       Filename:  SpxServerContext.h
 *        Created:  2015年01月22日 10时33分46秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/
#ifndef _SPXSERVERCONTEXT_H_
#define _SPXSERVERCONTEXT_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>

#include "SpxTypes.h"
#include "SpxEventLoop.h"
#include "SpxModule.h"
#include "SpxMFunc.h"


#define SpxLifeCycleNormal 0
#define SpxLifeCycleHeader 1
#define SpxLifeCycleBody 2

#define SpxMooreNormal 0
#define SpxMooreRead 1
#define SpxMooreWrite 2

#define SpxProtocolTypeTcp 0
#define SpxProtocolTypeHttp 1

    extern struct SpxServerContextPool *gSpxServerContextPool;


    struct SpxServerContext{
        struct SpxWatcher watcher;
        int fd;
        int use;
        size_t idx;
        err_t err;
        u64_t sec;
        u64_t msec;
        SpxLogDelegate *log;
        u32_t lifecycle;
        u32_t moore;
        u32_t protocolType;

        struct SpxModuleThreadContext *currentThreadContext;

        struct SpxMsgHeader *readerHeader;
        struct SpxMsgHeader *writerHeader;
        string_t readerHeaderMsg;
        string_t writerHeaderMsg;
        string_t readerBodyMsg;
        string_t writerBodyMsg;

        SpxWatcherDelegate *writerHandler;
        SpxWatcherDelegate *readerHandler;
        SpxWatcherDelegate *headerValidatorHandler;
        SpxWatcherDelegate *headerValidatorFailHandler;
        SpxWatcherDelegate *ptBodyHandler;
        SpxWatcherDelegate *readerBodyHandler;
        SpxWatcherDelegate *writerBodyHandler;

        struct SpxAsyncWatcher jobWatcher;
        SpxAsyncWatcherDelegate *jobHandler;

        string_t clientIp;
        u64_t batchTimespan;
        var c;//config
        var jobContext;//the argument for job,this is the client data

        bool_t isLazyRecv;
        bool_t isSendfile;
        int sfd;//sendfile fd
        u64_t sbegin;//sendfile begin
        u64_t slength;//sendfile length
    };

    struct SpxServerContextPool{
        SpxLogDelegate *log;
        struct SpxFixedVector *_pool;
    };

    var spxServerContextNew(size_t idx,var arg,err_t *err);
    err_t spxServerContextFree(var arg);
    void spxServerContextClear(struct SpxServerContext *ssc);
    void spxServerContextReset(struct SpxServerContext *ssc);

    struct SpxServerContextPool *spxServerContextPoolNew(SpxLogDelegate *log,
            var c,size_t size,u64_t sec,u64_t msec,
            SpxWatcherDelegate *readerHandler,
            SpxWatcherDelegate *writerHandler,
            SpxWatcherDelegate *headerValidatorHandler,
            SpxWatcherDelegate *headerValidatorFailHandler,
            SpxWatcherDelegate *ptBodyHandler,
            SpxWatcherDelegate *readerBodyHandler,
            SpxWatcherDelegate *writerBodyHandler,
            SpxAsyncWatcherDelegate *jobHandler,
            err_t *err);

    struct SpxServerContext *spxServerContextPoolPop(struct SpxServerContextPool *sscp,err_t *err);
    err_t spxServerContextPoolPush(struct SpxServerContextPool *sscp,struct SpxServerContext *ssc);
    bool_t spxServerContextPoolFree(struct SpxServerContextPool *sscp);
    err_t spxServerContextInit(struct SpxServerContext *ssc,int fd,int moore);

    #define __SpxServerContextSetThreadContext(ssc,tc) __SpxSSet(ssc,currentThreadContext,tc)

#ifdef __cplusplus
}
#endif
#endif
