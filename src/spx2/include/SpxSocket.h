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
 *       Filename:  SpxSocket.h
 *        Created:  2015年01月19日 12时42分03秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/
#ifndef _SPXSOCKET_H_
#define _SPXSOCKET_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>


#include "SpxTypes.h"

err_t spxTcpNoDelay(int fd,bool_t enable);
int spxSocketNew(err_t *err);
err_t spxSocketStart(const int fd,
        string_t ip,const int port,
        bool_t isKeepAlive,size_t aliveTimeout,
        size_t detectTimes,size_t detectTimeout,
        bool_t isLinger,size_t lingerTimeout,
        bool_t isNoDelay,
        bool_t isTimeout,size_t timeout,
        size_t listens);
string_t spxSocketIp(int sock,err_t *err);
string_t spxHostToString(struct SpxHost *host,err_t *err);
err_t spxSocketConnect(int fd,string_t ip,int port);
err_t spxSocketConnectNoBlocking(int fd,string_t ip,int port,u32_t timeout);
err_t spxSocketSet(const int fd,
        bool_t isKeepAlive,size_t aliveTimeout,
        size_t detecTtimes,size_t detectTimeout,
        bool_t isLinger,size_t lingerTimeout,
        bool_t isNoDelay,
        bool_t isTimeout,size_t timeout);
string_t spxSocketGetNameByHostName(string_t name,err_t *err);
bool_t spxSocketIsIp(string_t ip);
bool_t spxSocketWaitRead(int fd,u32_t timeout);
bool_t spxSocketTest(int fd);


#ifdef __cplusplus
}
#endif
#endif
