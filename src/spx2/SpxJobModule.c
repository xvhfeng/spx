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
 *       Filename:  SpxJobModule.c
 *        Created:  2015年01月22日 10时36分42秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "SpxObject.h"
#include "SpxTypes.h"
#include "SpxVars.h"
#include "SpxModule.h"
#include "SpxEventLoop.h"
#include "SpxMFunc.h"
#include "SpxStdio.h"
#include "SpxServerContext.h"
#include "SpxJobModule.h"

struct SpxModuleContext *gSpxJobModule = NULL;

void spxJobModuleListening(struct SpxEventLoop *loop,
        struct SpxWatcher *w,int revents){
    err_t err = 0;
    if(SpxEvRead & revents){
        while(true){
            struct SpxServerContext *ssc = NULL;
            size_t len = 0;
            if(0 != (err = spxReadWithAck(w->fd,(char *) &ssc,sizeof(ssc),&len))){
                if(__SpxFileDescIsReAttach(err)){
                    break;
                }
                __SpxLog2(loop->log,SpxLogError,err,
                        "job thread listen server-context is fail.");
                break;
            }

            if(NULL != ssc->jobHandler){
                spxAsyncWatcherInit(&(ssc->jobWatcher),0,ssc->jobHandler,ssc);
                spxAsyncWatcherAttach(loop,&(ssc->jobWatcher));
            }
        }
    }
    spxWatcherAttach(loop,w);
    return;
}

