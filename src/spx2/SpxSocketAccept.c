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
#include <ev.h>

#include "SpxTypes.h"
#include "SpxMFunc.h"
#include "SpxVars.h"
#include "SpxEventLoop.h"
#include "SpxString.h"
#include "SpxObject.h"


private struct SpxWatcher _mainWatcher;
private void _spxMainSocketHandler(struct SpxEventLoop *loop,
       int fd,int revents, struct SpxWatcher *w);

void spxSocketAcceptNoBlocking(SpxLogDelegate *log,
        struct SpxEventLoop *loop,int fd);
    __SpxZero(_mainWatcher);

    //no the timeout
    err = spxWatcherInit(&(_mainWatcher),fd,0,0,
            SpxEvRead,_spxMainSocketHandler,log);
    spxWatcherAttach(loop,&(_mainWatcher));
    SpxEventLoopRunning(loop,0);
}

private void _spxMainSocketHandler(struct SpxEventLoop *loop,
       int fd,int revents, struct SpxWatcher *w){
    if(SpxEvRead & ~revents){
        spxWatcherAttach(loop,&(_mainWatcher));
        return;
    }
    SpxLogDelegate *log = (SpxLogDelegate *) watcher->arg;
    err_t err = 0;
    while(true){
        struct sockaddr_in client_addr;
        unsigned int socket_len = 0;
        int client_sock = 0;
        socket_len = sizeof(struct sockaddr_in);
        client_sock = accept(watcher->fd, (struct sockaddr *) &client_addr,
                &socket_len);
        if (0 > client_sock) {
            if (!__SpxIsSocketReAttach(errno)) {
                __SpxLog2(log,SpxLogError,errno,
                        "accept is fail,but re-attach."):
            }
                break;
        }

        if (0 == client_sock) {
            break;
        }

        size_t idx = client_sock % g_spx_notifier_module->threadpool->size;
        struct spx_job_context *jc =  spx_job_pool_pop(g_spx_job_pool,&err);
        if(NULL == jc){
            __SpxClose(client_sock);
            __SpxLog1(log,SpxLogError,\
                    "pop nio context is fail.");
            break;
        }

        __SpxLogFmt1(log,SpxLogDebug,"recv socket conntect.wakeup notifier module idx:%d.jc idx:%d."
                ,idx,jc->idx);

        jc->fd = client_sock;
        jc->request_timespan = spxNow();

        struct spx_thread_context *tc = spx_get_thread(g_spx_notifier_module,idx);
        jc->tc = tc;
        SpxModuleDispatch(spx_notifier_module_wakeup_handler,jc);
    }
    spxWatcherAttach(loop,&(_mainWatcher));
}

