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
 *       Filename:  SpxServerContext.c
 *        Created:  2015年01月23日 10时08分44秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "SpxTypes.h"
#include "SpxMFunc.h"
#include "SpxFixedVector.h"
#include "SpxSocket.h"
#include "SpxStdio.h"
#include "SpxObject.h"
#include "SpxVars.h"
#include "SpxEventLoop.h"
#include "SpxServerContext.h"
#include "SpxString.h"
#include "SpxDateTime.h"

struct SpxServerContextPool *gSpxServerContextPool = NULL;

struct _SpxServerContextTransport{
    SpxLogDelegate *log;
    var c;
    u64_t sec;
    u64_t msec;
    SpxWatcherDelegate *readerHandler;
    SpxWatcherDelegate *writerHandler;
    SpxWatcherDelegate *headerValidatorHandler;
    SpxWatcherDelegate *headerValidatorFailHandler;
    SpxWatcherDelegate *ptBodyHandler;
    SpxWatcherDelegate *readerBodyHandler;
    SpxWatcherDelegate *writerBodyHandler;
    SpxAsyncWatcherDelegate *jobHandler;
};

var spxServerContextNew(size_t idx,var arg,err_t *err) {
    __SpxTypeConvert(struct _SpxServerContextTransport,ssct,arg);
    struct SpxServerContext *ssc = __SpxObjectNew(struct SpxServerContext,err);
    if(NULL == ssc){
        return NULL;
    }

    ssc->log = ssct->log;
    ssc->idx = idx;
    ssc->sec = ssct->sec;
    ssc->msec = ssct->msec;
    ssc->readerHandler = ssct->readerHandler;
    ssc->writerHandler = ssct->writerHandler;
    ssc->readerBodyHandler = ssct->readerBodyHandler;
    ssc->headerValidatorHandler = ssct->headerValidatorHandler;
    ssc->ptBodyHandler = ssct->ptBodyHandler;
    ssc->writerBodyHandler = ssct->writerBodyHandler;
    ssc->headerValidatorFailHandler = ssct->headerValidatorFailHandler;
    ssc->jobHandler = ssct->jobHandler;
    ssc->c = ssct->c;
    ssc->isLazyRecv = false;
    ssc->isSendfile = false;
    return ssc;
}

err_t spxServerContextFree(var arg){
    __SpxTypeConvert(struct SpxServerContext,ssc,arg);
    spxServerContextClear(ssc);
    __SpxObjectFree(ssc);
    return 0 ;
}

void spxServerContextClear(struct SpxServerContext *ssc){
    if(NULL != ssc->currentThreadContext){
        ssc->currentThreadContext = NULL;
    }
    if(NULL != ssc->readerHeader){
        __SpxObjectFree(ssc->readerHeader);
    }
    if(NULL != ssc->writerHeader){
        __SpxObjectFree(ssc->writerHeader);
    }
    if(NULL != ssc->readerHeaderMsg){
        __SpxStringFree(ssc->readerHeaderMsg);
    }
    if(NULL != ssc->writerHeaderMsg){
        __SpxStringFree(ssc->writerHeaderMsg);
    }
    if(NULL != ssc->readerBodyMsg){
        __SpxStringFree(ssc->readerBodyMsg);
    }
    if(NULL != ssc->writerBodyMsg){
        __SpxStringFree(ssc->writerBodyMsg);
    }
    if(NULL != ssc->clientIp){
        __SpxStringFree(ssc->clientIp);
    }
    if(NULL != ssc->jobContext){
        ssc->jobContext = NULL;
    }
    if(ssc->isSendfile){
        __SpxClose(ssc->sfd);
        ssc->sbegin = 0;
        ssc->slength = 0;
    }
    __SpxClose(ssc->fd);
    ssc->err = 0;
    ssc->moore = SpxMooreNormal;
    ssc->batchTimespan = 0;
}


void spxServerContextReset(struct SpxServerContext *ssc) {
    if(NULL != ssc->currentThreadContext){
        ssc->currentThreadContext = NULL;
    }
    if(NULL != ssc->readerHeader){
        __SpxObjectFree(ssc->readerHeader);
    }
    if(NULL != ssc->writerHeader){
        __SpxObjectFree(ssc->writerHeader);
    }
    if(NULL != ssc->readerHeaderMsg){
        __SpxStringFree(ssc->readerHeaderMsg);
    }
    if(NULL != ssc->writerHeaderMsg){
        __SpxStringFree(ssc->writerHeaderMsg);
    }
    if(NULL != ssc->readerBodyMsg){
        __SpxStringFree(ssc->readerBodyMsg);
    }
    if(NULL != ssc->writerBodyMsg){
        __SpxStringFree(ssc->writerBodyMsg);
    }
    if(NULL != ssc->jobContext){
        ssc->jobContext = NULL;
    }
    if(ssc->isSendfile){
        __SpxClose(ssc->sfd);
        ssc->sbegin = 0;
        ssc->slength = 0;
    }
    ssc->err = 0;
    ssc->moore = SpxMooreNormal;
    ssc->batchTimespan = spxClock();
}


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
        err_t *err) {
    if(0 == size){
        *err = EINVAL;
    }
    struct SpxServerContextPool *sscp = NULL;
    sscp = spxObjectNew(sizeof(*sscp),err);
    if(NULL == sscp){
        return NULL;
    }

    struct _SpxServerContextTransport ssct;
    __SpxZero(ssct);
    ssct.log = log;
    ssct.c = c;
    ssct.sec = sec;
    ssct.msec = msec;
    ssct.readerHandler = readerHandler;
    ssct.writerHandler = writerHandler;
    ssct.headerValidatorHandler = headerValidatorHandler;
    ssct.headerValidatorFailHandler = headerValidatorFailHandler;
    ssct.ptBodyHandler = ptBodyHandler;
    ssct.readerBodyHandler = readerBodyHandler;
    ssct.writerBodyHandler = writerBodyHandler;
    ssct.jobHandler = jobHandler;
    sscp->_pool = spxFixedVectorNew(log,size,
            spxServerContextNew,&ssct,
            spxServerContextFree,err);
    if(NULL == sscp->_pool){
        __SpxObjectFree(sscp);
        return NULL;
    }
    return sscp;
}

struct SpxServerContext *spxServerContextPoolPop(struct SpxServerContextPool *sscp,err_t *err){
    struct SpxServerContext *ssc = spxFixedVectorPop(sscp->_pool,err);
    if(NULL == ssc){
        *err = 0 == *err ? ENOENT : *err;
        return NULL;
    }
    return ssc;
}

err_t spxServerContextPoolPush(struct SpxServerContextPool *sscp,struct SpxServerContext *ssc){
    spxServerContextClear(ssc);
    return spxFixedVectorPush(sscp->_pool,ssc);
}

bool_t spxServerContextPoolFree(struct SpxServerContextPool *sscp){
    __SpxFixedVectorFree(sscp->_pool);
    return spxObjectFree(sscp);
}

err_t spxServerContextInit(struct SpxServerContext *ssc,int fd,int moore){
    if(0 > fd) return EINVAL;
    err_t err = 0;
    ssc->fd = fd;
    ssc->clientIp = spxSocketIp(ssc->fd,&err);
    if(0 != err) return err;
    spxFileDescSetNoBlocking(ssc->fd);
    spxTcpNoDelay(ssc->fd,true);
    ssc->moore = moore;
    return 0;
}

