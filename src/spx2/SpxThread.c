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
 *       Filename:  SpxThread.c
 *        Created:  2015年01月24日 19时38分26秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "SpxTypes.h"
#include "SpxMFunc.h"
#include "SpxVars.h"
#include "SpxObject.h"
#include "SpxCounter.h"
#include "SpxAlloc.h"
#include "SpxAtomic.h"


struct _SpxThreadTransport{
    var (*startRoutine)(var);
    var arg;
};

pthread_t spxThreadNew(SpxLogDelegate *log,
        size_t stackSize,
        var (*startRoutine)(var),
        var arg,
        err_t *err){
    pthread_t tid = 0;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    size_t ostackSize = 0;
    pthread_attr_getstacksize(&attr, &ostackSize);
    do{
        if (ostackSize != stackSize
                && (0 != (*err = pthread_attr_setstacksize(&attr,stackSize)))){
            __SpxLog2(log,SpxLogError,*err,\
                    "set thread stack size is fail.");
            pthread_attr_destroy(&attr);
            break;
        }
        if (0 !=(*err =  pthread_create(&(tid), &attr, startRoutine,
                        arg))){
            __SpxLog2(log,SpxLogError,*err,\
                    "create nio thread is fail.");
            pthread_attr_destroy(&attr);
            break;
        }
    }while(false);
    pthread_attr_destroy(&attr);
    return tid;
}

pthread_t spxThreadNewDetached(SpxLogDelegate *log,
        size_t stackSize,
        var (*startRoutine)(var),
        var arg,
        err_t *err){
    pthread_t tid = 0;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    size_t ostackSize = 0;
    pthread_attr_getstacksize(&attr, &ostackSize);
    do{
        if (ostackSize != stackSize
                && (0 != (*err = pthread_attr_setstacksize(&attr,stackSize)))){
            __SpxLog2(log,SpxLogError,*err,\
                    "set thread stack size is fail.");
            pthread_attr_destroy(&attr);
            break;
        }
        if (0 !=(*err =  pthread_create(&(tid), &attr, startRoutine,
                        arg))){
            __SpxLog2(log,SpxLogError,*err,\
                    "create nio thread is fail.");
            pthread_attr_destroy(&attr);
            break;
        }
    }while(false);
    pthread_attr_destroy(&attr);
    return tid;
}

private var spxThreadCancelabilityStartRoutine(var arg){
    if(NULL == arg) return NULL;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);//run to next cannel-pointer
    __SpxTypeConvert(struct _SpxThreadTransport,stt,arg);
    var wantArg = stt->arg;
    var (*wantStartRoutine)(var) = stt->startRoutine;
    __SpxObjectFree(stt);
    wantStartRoutine(wantArg);
    return NULL;
}

pthread_t spxThreadNewCancelability(SpxLogDelegate *log,
        size_t stackSize,
        var (*startRoutine)(var),
        var arg,
        err_t *err){
    struct _SpxThreadTransport *stt = __SpxObjectNew(struct _SpxThreadTransport,err);
    if(NULL == stt){
        return 0;
    }
    stt->arg = arg;
    stt->startRoutine = startRoutine;
    return spxThreadNew(log,stackSize,
            spxThreadCancelabilityStartRoutine,(var) stt,err);
}


pthread_mutex_t *spxThreadMutexNew(SpxLogDelegate *log,
        err_t *err){
    pthread_mutex_t *m = spxObjectNew(sizeof(*m),err);
    if(NULL == m){
        __SpxLog2(log,SpxLogError,*err,
                "alloc mutext locker is fail.");
    }
    pthread_mutex_init(m,NULL);
    return m;
}

bool_t spxThreadMutexFree(pthread_mutex_t *m){
    __SpxTypeConvert(struct SpxObject,o,m);
    if(0 == __SpxAtomicVDecr(o->_spxObjectRefs)){
        pthread_mutex_destroy(m);
        __SpxAtomicVSub(gSpxMemoryUseSize, SpxObjectAlignSize + o->_spxObjectSize);
        __SpxFree(o);
        return true;
    }
    return false;
}

pthread_cond_t *spxThreadCondNew(SpxLogDelegate *log,
        err_t *err){
    pthread_cond_t *m = spxObjectNew(sizeof(*m),err);
    if(NULL == m){
        __SpxLog2(log,SpxLogError,*err,
                "alloc cond locker is fail.");
    }
    pthread_cond_init(m,NULL);
    return m;

}

bool_t spxThreadCondFree(pthread_cond_t *c){
    __SpxTypeConvert(struct SpxObject,o,c);
    if(0 == __SpxAtomicVDecr(o->_spxObjectRefs)){
        pthread_cond_destroy(c);
        __SpxAtomicVSub(gSpxMemoryUseSize, SpxObjectAlignSize + o->_spxObjectSize);
        __SpxFree(o);
        return true;
    }
    return false;
}
