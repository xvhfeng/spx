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
 *       Filename:  SpxSocketAccept.c
 *        Created:  2015年01月19日 12时42分55秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "SpxTypes.h"
#include "SpxMFunc.h"
#include "SpxVars.h"
#include "SpxEventLoop.h"
#include "SpxString.h"
#include "SpxObject.h"
#include "SpxModule.h"
#include "SpxNetworkModule.h"
#include "SpxServerContext.h"


private struct SpxWatcher _mainWatcher;
private void _spxMainSocketHandler(struct SpxEventLoop *loop,
            struct SpxWatcher *w,int revents,var arg);

void spxSocketAcceptNoBlocking(SpxLogDelegate *log,
        struct SpxEventLoop *loop,int fd){
    __SpxZero(_mainWatcher);
    err_t err = 0;
    //no the timeout
    err = spxWatcherInit(&(_mainWatcher),fd,0);
    if(0 != err) {

    }
    err = spxWatcherAddHandler(&(_mainWatcher),SpxEvRead,0,0,
            _spxMainSocketHandler,log);
    if(0 != err) {

    }
    err = spxWatcherAttach(loop,&(_mainWatcher));
    if(0 != err) {

    }
    spxEventLoopStart(loop,0);
}

private void _spxMainSocketHandler(struct SpxEventLoop *loop,
            struct SpxWatcher *w,int revents,var arg){
    if(SpxEvRead & ~revents){
        spxWatcherAttach(loop,&(_mainWatcher));
        return;
    }
    SpxLogDelegate *log = (SpxLogDelegate *) w->_r.arg;
    err_t err = 0;
    while(true){
        struct sockaddr_in client_addr;
        unsigned int socket_len = 0;
        int client_sock = 0;
        socket_len = sizeof(struct sockaddr_in);
        client_sock = accept(w->fd, (struct sockaddr *) &client_addr,
                &socket_len);
        if (0 > client_sock) {
            if (!__SpxMainSocketIsReAttach(errno)) {
                __SpxLog2(log,SpxLogError,errno,
                        "accept is fail,but re-attach.");
            }
            break;
        }

        if (0 == client_sock) {
            break;
        }

        struct SpxModuleThreadContext *smtc = spxModuleGetThreadContext(gSpxNetworkModule,client_sock);
        struct SpxServerContext *ssc = spxServerContextPoolPop(gSpxServerContextPool,&err);
        if(NULL == ssc){
            __SpxClose(client_sock);
            __SpxLog1(log,SpxLogError,\
                    "pop nio context is fail.");
            break;
        }

        spxServerContextInit(ssc,w->fd,SpxMooreRead);
        ssc->protocolType = SpxProtocolTypeTcp;
        __SpxServerContextSetThreadContext(ssc,smtc);
        spxModuleThreadWakeup(smtc,ssc);
    }
    spxWatcherAttach(loop,&(_mainWatcher));
}

