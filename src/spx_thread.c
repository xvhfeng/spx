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
 *       Filename:  spx_thread.c
 *        Created:  2014/09/01 17时27分21秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "spx_types.h"
#include "spx_alloc.h"
#include "spx_defs.h"

pthread_mutex_t *spx_thread_mutex_new(SpxLogDelegate *log,
        err_t *err){
    pthread_mutex_t *m = spx_alloc_alone(sizeof(*m),err);
    if(NULL == m){
        SpxLog2(log,SpxLogError,*err,
                "alloc mutext locker is fail.");
    }
    pthread_mutex_init(m,NULL);
    return m;
}

void spx_thread_mutex_free(pthread_mutex_t **mlock){
    if(NULL != *mlock){
        pthread_mutex_destroy(*mlock);
        SpxFree(*mlock);
    }
}

pthread_t spx_thread_new(SpxLogDelegate *log,
        size_t stacksize,
    void *(*start_routine)(void*),
    void *arg,
    err_t *err){
    pthread_t tid = 0;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    size_t ostack_size = 0;
    pthread_attr_getstacksize(&attr, &ostack_size);
    do{
        if (ostack_size != stacksize
                && (0 != (*err = pthread_attr_setstacksize(&attr,stacksize)))){
            SpxLog2(log,SpxLogError,*err,\
                    "set thread stack size is fail.");
            pthread_attr_destroy(&attr);
            break;
        }
        if (0 !=(*err =  pthread_create(&(tid), &attr, start_routine,
                        arg))){
            SpxLog2(log,SpxLogError,*err,\
                    "create nio thread is fail.");
            pthread_attr_destroy(&attr);
            break;
        }
    }while(false);
    pthread_attr_destroy(&attr);
    return tid;
}

