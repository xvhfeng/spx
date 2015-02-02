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
 *       Filename:  SpxNetworkModule.c
 *        Created:  2015年01月21日 17时13分13秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "SpxTypes.h"
#include "SpxModule.h"
#include "SpxEventLoop.h"
#include "SpxVars.h"
#include "SpxMFunc.h"
#include "SpxStdio.h"
#include "SpxServerContext.h"
#include "SpxNetworkModule.h"


struct SpxModuleContext *gSpxNetworkModule = NULL;

void spxNetworkModuleListening(struct SpxEventLoop *loop,
        struct SpxWatcher *w,int revents){
    if(SpxEvRead & revents){
        while(true){
            struct SpxServerContext *ssc = NULL;
            size_t len = 0;
            err_t err =0;
            if(0 != (err = spxReadWithAck(w->fd,(char *) &ssc,sizeof(ssc),&len))){
                if(__SpxFileDescIsReAttach(err)){
                    break;
                }
                __SpxLog2(loop->log,SpxLogError,err,
                        "network thread listen server-context is fail.");
                break;
            }

            switch(ssc->moore){
                case SpxMooreRead:
                    {
                        spxWatcherAttach(loop,&(ssc->watcher));
                        break;
                    }
                case SpxMooreWrite:
                    {
                        spxWatcherAttach(loop,&(ssc->watcher));
                        break;
                    }
                default:
                    {

                    }
            }

        }
    }
    spxWatcherAttach(loop,w);
    return;
}

