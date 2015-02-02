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
 *       Filename:  SpxModule.c
 *        Created:  2015年01月21日 17时05分47秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "SpxTypes.h"
#include "SpxObject.h"
#include "SpxModule.h"
#include "SpxEventLoop.h"
#include "SpxMFunc.h"
#include "SpxStdio.h"
#include "SpxServerContext.h"
#include "SpxModule.h"
#include "SpxThread.h"
#include "SpxVars.h"
#include "SpxList.h"

struct _SpxModuleThreadContextTransport{
    SpxLogDelegate *log;
    size_t maxEvents;
    size_t stackSize;
    SpxWatcherDelegate *handler;
};

private var _spxModuleThreadContextListening(var arg);
private var _spxModuleThreadContextNew(size_t idx,var arg,err_t *err);
private err_t _spxModuleThreadContextFree(var arg);


private var _spxModuleThreadContextNew(size_t idx,var arg,err_t *err){
    __SpxTypeConvert(struct _SpxModuleThreadContextTransport,smtct,arg);
    struct SpxModuleThreadContext *smtc = __SpxObjectNew(struct SpxModuleThreadContext,err);
    if(NULL == smtc){
        __SpxLog2(smtct->log,SpxLogError,*err,\
                "alloc thread context is fail.");
        return NULL;
    }
    smtc->_idx = idx;
    smtc->loop = spxEventLoopNew(smtct->log,smtct->maxEvents,err);
    smtc->log = smtct->log;
    if(-1 == pipe(smtc->_pipe)){
        *err = errno;
        __SpxEventLoopFree(smtc->loop);
        __SpxObjectFree(smtc);
        return NULL;
    }
    spxFileDescSetNoBlocking(smtc->_pipe[0]);
    spxFileDescSetNoBlocking(smtc->_pipe[1]);
    smtc->_tid = spxThreadNewCancelability(smtct->log,
            smtct->stackSize,_spxModuleThreadContextListening,
            smtc,err);
    return smtc;
}

private err_t _spxModuleThreadContextFree(var arg){/*{{{*/
    __SpxTypeConvert(struct SpxModuleThreadContext,smtc,arg);
    if(NULL == smtc) return 0;
    spxEventLoopBreak(smtc->loop);
    spxEventLoopFree(smtc->loop);
    __SpxClose(smtc->_pipe[0]);
    __SpxClose(smtc->_pipe[1]);
    __SpxObjectFree(smtc);
    return 0;
}/*}}}*/


private var _spxModuleThreadContextListening(var arg){
    __SpxTypeConvert(struct SpxModuleThreadContext,smtc,arg);
    spxWatcherInit(&(smtc->watcher),smtc->_pipe[0],0);
    spxWatcherAddHandler(&(smtc->watcher),SpxEvRead,0,0,smtc->handler,smtc);
    spxWatcherAttach(smtc->loop,&(smtc->watcher));
    spxEventLoopStart(smtc->loop,0);
    return NULL;
}

err_t spxModuleContextFree(struct SpxModuleContext *mc){
    //must free thread pool first
    if(NULL != mc->_threadpool){
        spxListFree(mc->_threadpool);
    }
    __SpxObjectFree(mc);
    return 0;
}

struct SpxModuleContext *spxModuleNew(
        SpxLogDelegate *log,
        u32_t threadSize,
        size_t stackSize,
        size_t maxEvents,
        SpxWatcherDelegate *handler,
        err_t *err){/*{{{*/
    struct SpxModuleContext *smc = __SpxObjectNew(struct SpxModuleContext,err);
    if(NULL == smc){
        __SpxLog2(log,SpxLogError,*err,
                "new module-context is fail.");
        return NULL;
    }

    smc->log = log;
    smc->_threadSize = threadSize;

    struct _SpxModuleThreadContextTransport smtct;
    __SpxZero(smtct);
    smtct.log = log;
    smtct.maxEvents = maxEvents;
    smtct.stackSize = stackSize;
    smtct.handler = handler;
    smc->_threadpool = spxListInit(log,
            threadSize,_spxModuleThreadContextNew,&smtct,
            _spxModuleThreadContextFree,err);



    if(NULL == smc->_threadpool){
        __SpxLog2(log,SpxLogError,*err,
                "new thread-context of module is fail.");
        goto r1;
    }
    return smc;
r1:
    spxModuleContextFree(smc);
    return NULL;
}


err_t spxModuleThreadWakeup(struct SpxModuleThreadContext *smtc,var arg) {
    size_t len = 0;
    err_t err = 0;
    err = spxWriteWithNoAck(smtc->_pipe[1],(string_t) &arg,sizeof(arg),&len);
    if (0 != err || sizeof(arg) != len) {
        return err;
    }
    return err;
}

struct SpxModuleThreadContext *spxModuleGetThreadContext(struct SpxModuleContext *smc,size_t idx){
    return (struct SpxModuleThreadContext *) spxListGet(smc->_threadpool,idx);
}


