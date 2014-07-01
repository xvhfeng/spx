/*
 * =====================================================================================
 *
 *       Filename:  spx_socket.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/06/11 14时33分30秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
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

#include "include/spx_types.h"
#include "include/spx_string.h"
#include "include/spx_defs.h"
#include "include/spx_sio_context.h"
#include "include/spx_socket.h"

spx_private err_t spx_socket_reuseaddr(int sock) ;
spx_private err_t spx_socket_keepalive(int fd,bool_t enable,\
        size_t alive_timeout,size_t detect_times,size_t detect_timeout);
spx_private err_t spx_socket_linger(const int fd,bool_t enable, size_t timeout);
spx_private err_t spx_tcp_nodelay(int fd,bool_t enable);
spx_private err_t spx_socket_timout(int fd, size_t timeout);


spx_private err_t spx_socket_reuseaddr(int sock) {
	int optval = 1;
    if(0 != setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&optval,(socklen_t) sizeof(int))){
        return errno;
    }
    return 0;
}

spx_private err_t spx_socket_keepalive(int fd,bool_t enable,\
        size_t alive_timeout,size_t detect_times,size_t detect_timeout){

    int err = 0;
    if(!enable) return 0;
#ifdef SpxLinux
    //open the keepalive
    int optval = 1;
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval,
                (socklen_t) sizeof(int))) {
        err = errno;
        return err;
    }
    if (-1 == setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, &alive_timeout
                sizeof(keepidle))) {
        err = errno;
        return err;
    }
    if (-1 == setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &detect_times,\
                sizeof(keepcnt))) {
        err = errno;
        return err;
    }
    if (-1 == setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &detect_timeout,\
                sizeof(keepintvl))) {
        err = errno;
        return err;
    }
#endif

#ifdef SpxMac
    //open the keepalive
    int optval = 1;
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval,
                (socklen_t) sizeof(int))) {
        err = errno;
        return err;
    }
#endif
    return err;
}


spx_private err_t spx_socket_linger(const int fd,bool_t enable, size_t timeout) {/*{{{*/
	struct linger optval;
	optval.l_onoff = enable;
	optval.l_linger = timeout;
	if (-1 == setsockopt(fd, SOL_SOCKET, SO_LINGER, &optval,\
					(socklen_t) sizeof(struct linger))) {
		return errno;
	}
	return 0;
}/*}}}*/


spx_private err_t spx_tcp_nodelay(int fd,bool_t enable) {/*{{{*/
#ifdef SpxLinux
	if (-1 == setsockopt(fd, SOL_TCP, TCP_NODELAY, &enable, sizeof(enable))) {
	    return errno;
	}
#endif
	return 0;
}/*}}}*/

spx_private err_t spx_socket_timout(int fd, size_t timeout) {/*{{{*/
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


int spx_socket_new(err_t *err){
    int fd = 0;
    fd = socket(AF_INET, SOCK_STREAM,0);
    if(-1 == fd){
        *err = errno;
        return -1;
    }
    return fd;
}
err_t spx_socket_start(const int fd,\
        string_t ip,const int port,\
        bool_t is_keepalive,size_t alive_timeout,\
        size_t detect_times,size_t detect_timeout,\
        bool_t is_linger,size_t linger_timeout,\
        bool_t is_nodelay,\
        bool_t is_timeout,size_t timeout,\
        size_t listens){

    err_t err = 0;
    if(0 != (err = spx_socket_reuseaddr(fd))){
        return err;
    }

    if(0 != (err = spx_socket_keepalive(fd,is_keepalive,
                    alive_timeout,detect_times,detect_timeout))){
        return err;
    }
    if(0 != (err = spx_socket_linger(fd,is_linger,linger_timeout))){
        return err;
    }
    if(0 != (err = spx_tcp_nodelay(fd,is_nodelay))){
        return err;
    }
    if(is_timeout && (0 != (err = spx_socket_timout(fd,timeout)))){
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
}

void spx_socket_accept_nb(int fd){
    err_t err = 0;
    while(true){
        struct sockaddr_in client_addr;
        unsigned int socket_len;
        int client_sock = 0;
        socket_len = sizeof(struct sockaddr_in);
        client_sock = accept(fd, (struct sockaddr *) &client_addr,
                &socket_len);
        if (0 > client_sock) {
            if (EWOULDBLOCK == errno || EAGAIN == errno) {
                continue;
            }
            continue;
        }

        if (0 == client_sock) {
            continue;
        }

        struct spx_sio_context *sio_context = spx_sio_context_pool_pop(g_spx_sio_context_pool,&err);

        if (sizeof(client_sock)
                != write(sio_context->pipes[1], &client_sock,
                    sizeof(client_sock))) {
            close(client_sock);
         }
    }
}


string_t spx_ip_get(int sock,err_t *err) {

	string_t ip = spx_string_empty(err);
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
	spx_string_cat(ip,tmp,err);
	if(0 != err){
	    spx_string_free(ip);
        return NULL;
    }
	return ip;
}


string_t spx_host_tostring(struct spx_host *host,err_t *err){
    string_t shost = spx_string_empty(err);
    if(NULL == shost){
        return NULL;
    }

    string_t new = spx_string_cat_printf(err,shost,"%s:%d",host->ip,host->port);
    if(NULL == new){
        goto r1;
    }
    return new;

r1:
    spx_string_free(shost);
}