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
 *       Filename:  SpxSocket.c
 *        Created:  2015年01月19日 12时42分17秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
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
#include "SpxVars.h"
#include "SpxMFunc.h"
#include "SpxObejct.h"
#include "SpxError.h"

private err_t _spxSocketReUseAddr(int sock){/*{{{*/
	int optval = 1;
    if(0 != setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&optval,(socklen_t) sizeof(int))){
        return errno;
    }
    return 0;
}/*}}}*/

private err_t _spxSocketKeepAlive(int fd,bool_t enable,\
        size_t aliveTimeout,size_t detectTimes,size_t detectTimeout){/*{{{*/
    if(!enable) return 0;
#ifdef SpxLinux
    //open the keepalive
    int optval = 1;
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval,
                (socklen_t) sizeof(int))) {
        return errno;
    }
    if (-1 == setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, &aliveTimeout,
                sizeof(size_t))) {
        return errno;
    }
    if (-1 == setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &detectTimes,\
                sizeof(size_t))) {
        return errno;
    }
    if (-1 == setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &detectTimeout,\
                sizeof(size_t))) {
        return errno;
    }
#endif

#ifdef SpxMac
    //open the keepalive
    int optval = 1;
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval,
                (socklen_t) sizeof(int))) {
        return errno;
    }
#endif
    return 0;
}/*}}}*/


private err_t _spxSocketLinger(const int fd,bool_t enable, size_t timeout) {/*{{{*/
	struct linger optval;
	optval.l_onoff = enable;
	optval.l_linger = timeout;
	if (-1 == setsockopt(fd, SOL_SOCKET, SO_LINGER, &optval,\
					(socklen_t) sizeof(struct linger))) {
		return errno;
	}
	return 0;
}/*}}}*/


err_t spxTcpNoDelay(int fd,bool_t enable) {/*{{{*/
#ifdef SpxLinux
	if (-1 == setsockopt(fd, SOL_TCP, TCP_NODELAY, &enable, sizeof(enable))) {
	    return errno;
	}
#endif
	return 0;
}/*}}}*/

private err_t _spxSocketTimeout(int fd, size_t timeout) {/*{{{*/
	struct timeval waittime;
	waittime.tv_sec = timeout;
	waittime.tv_usec = 0;
	if (-1 == setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &waittime,
					sizeof(struct timeval))) {
		return errno;
	}
	if (-1 == setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &waittime,
					sizeof(struct timeval))) {
		return errno;
	}
	return 0;
}/*}}}*/


int spxSocketNew(err_t *err){/*{{{*/
    int fd = 0;
    fd = socket(AF_INET, SOCK_STREAM,0);
    if(-1 == fd){
        *err = errno;
        return -1;
    }
    return fd;
}/*}}}*/

err_t spxSocketStart(const int fd,\
        string_t ip,const int port,\
        bool_t isKeepAlive,size_t aliveTimeout,\
        size_t detectTimes,size_t detectTimeout,\
        bool_t isLinger,size_t lingerTimeout,\
        bool_t isNoDelay,\
        bool_t isTimeout,size_t timeout,\
        size_t listens){/*{{{*/

    err_t err = 0;
    if(0 != (err = _spxSocketReUseAddr(fd))){
        return err;
    }

    if(0 != (err = _spxSocketAlive(fd,isKeepAlive,
                    aliveTimeout,detectTimes,detectTimeout))){
        return err;
    }
    if(0 != (err = _spxSocketLinger(fd,isLinger,lingerTimeout))){
        return err;
    }
    if(0 != (err = spxTcpNoDelay(fd,isNoDelay))){
        return err;
    }
    if(isTimeout && (0 != (err = _spxSOcketTimeout(fd,timeout)))){
        return err;
    }

    struct sockaddr_in addr;
    SpxZero(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if(SpxStringIsNullOrEmpty(ip)){
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else{
        inet_aton(ip,&(addr.sin_addr));
    }

    if(-1 == bind(fd, (struct sockaddr *) &addr,
                sizeof(struct sockaddr_in))) {
        err = errno;
        return err;
    }

	if (-1 == listen(fd, listens)) {
        return errno;
	}
    return 0;
}/*}}}*/

string_t spxSocketIp(int sock,err_t *err) {/*{{{*/
	string_t ip = spxStringNewLength(SpxIpv4Size,err);
	if(NULL == ip){
        return NULL;
    }

	socklen_t len;
	struct sockaddr_in addr;
	len = sizeof(struct sockaddr_in);
	memset(&addr, 0, len);
	getpeername(sock, (struct sockaddr *) &addr, &len);

	char *tmp;
	tmp = inet_ntoa(addr.sin_addr);
	spxStringCat(ip,tmp,err);
	if(0 != *err){
	    __SpxStringFree(ip);
        return NULL;
    }
	return ip;
}/*}}}*/


string_t spxHostToString(struct SpxHost *host,err_t *err){/*{{{*/
    string_t shost = spxStringNewEmpty(err);
    if(NULL == shost){
        return NULL;
    }

    string_t new = spxStringCatPrintf(err,shost,"%s:%d",host->ip,host->port);
    if(NULL == new){
        goto r1;
    }
    return new;

r1:
    __SpxStringFree(shost);
    return NULL;
}/*}}}*/

err_t spxSocketConnect(int fd,string_t ip,int port){/*{{{*/
    struct sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    if(0 > connect(fd,(struct sockaddr *) &addr,sizeof(addr))){
        return errno;
    }
    return 0;
}/*}}}*/

err_t spxSocketConnectNoBlocking(int fd,string_t ip,int port,u32_t timeout){/*{{{*/
    struct sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    err_t err = 0;
    if(0 > connect(fd,(struct sockaddr *) &addr,sizeof(addr))){
        //filter this errno,socket is not connect to server and return at once
        if (EINPROGRESS == errno) {
            struct timeval tv;
            SpxZero(tv);
            tv.tv_sec = timeout;
            tv.tv_usec = 0;
            fd_set frd;
            FD_ZERO(&frd);
            FD_SET(fd,&frd);
            socklen_t len = sizeof(err);
            if (0 < select (fd+1 , NULL,&frd,NULL,&tv)) {
                if(0 > getsockopt(fd,SOL_SOCKET,SO_ERROR,(void*)(&err),&len)) {
                    err = errno;
                    return err;
                }
                if (0 != err) {
                    return err;
                }
            } else {
                err = EXDEV;
                return err;
            }
            SpxErrReset;
            return 0;
        } else {
            return errno;
        }
    }
    return 0;
}/*}}}*/

err_t spxSocketSet(const int fd,\
        bool_t isKeepAlive,size_t aliveTimeout,\
        size_t detecTtimes,size_t detectTimeout,\
        bool_t isLinger,size_t lingerTimeout,\
        bool_t isNoDelay,\
        bool_t isTimeout,size_t timeout){/*{{{*/
    err_t err = 0;
    if(0 != (err = _spxSocketReUseAddr(fd))){
        return err;
    }

    if(0 != (err = _spxSocketKeepAlive(fd,isKeepAlive,
                    aliveTimeout,detectTimes,detectTimeout))){
        return err;
    }
    if(0 != (err = _spxSocketLinger(fd,isLinger,lingerTimeout))){
        return err;
    }
    if(0 != (err = spxTcpNoDelay(fd,isNoDelay))){
        return err;
    }
    if(isTimeout && (0 != (err = _spxSOcketTimeout(fd,timeout)))){
        return err;
    }

    return 0;
}/*}}}*/

string_t spxSocketGetNameByHostName(string_t name,err_t *err){/*{{{*/
    string_t hostname = NULL;
    string_t ip = NULL;
    if(NULL == name){
        hostname = spxStringNew(NULL,SpxHostNameSize,err);
        if(NULL == hostname){
            return NULL;
        }
        *err = gethostname(hostname,SpxHostNameSize);
        if(0 != *err){
            goto r1;
        }
    }else {
        hostname = name;
    }
    struct hostent *hosts = gethostbyname(hostname);
    if(NULL == hosts){
        *err = h_errno;
        goto r1;
    }
    struct in_addr addr;
    memcpy(&addr, hosts->h_addr_list[0], sizeof(struct in_addr));
    ip = spxStringNew(NULL,SpxIpv4Size,err);
    if(NULL == ip){
        goto r1;
    }
    string_t newip = spxStringCat(ip,inet_ntoa(addr),err);
    if(__SpxStringIsNullOrEmpty(newip)){
        __SpxStringFree(ip);
        goto r1;
    }
r1:
    if(NULL == name){
        __SpxStringFree(hostname);
    }
    return ip;
}/*}}}*/

bool_t spxSocketIsIp(string_t ip){/*{{{*/
    struct sockaddr_in sa;
    int rc = inet_pton(AF_INET, ip, &(sa.sin_addr));
    return 1 == rc ? true : false;
}/*}}}*/

bool_t spxSocketWaitRead(int fd,u32_t timeout){/*{{{*/
    struct timeval tv;
    SpxZero(tv);
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    fd_set frd;
    FD_ZERO(&frd);
    FD_SET(fd,&frd);
    int n = select (fd+1 , &frd,NULL,NULL,&tv);
    if(0 < n && FD_ISSET(fd,&frd)){
        return true;
    }
    return false;
}/*}}}*/

bool_t spxSocketTest(int fd){/*{{{*/
    struct tcp_info info;
    int len=sizeof(info);
    getsockopt(fd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
    if((info.tcpi_state==TCP_ESTABLISHED))
        return true;
    return false;
}/*}}}*/

