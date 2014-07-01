/*
 * =====================================================================================
 *
 *       Filename:  spx_socket.h
 *
 *    Description:
 *
 *
 *        Version:  1.0
 *        Created:  2014/06/11 14时33分39秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef _SPX_SOCKET_H_
#define _SPX_SOCKET_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "spx_types.h"


    struct spx_host{
        string_t ip;
        int port;
    };

int spx_socket_new(err_t *err);
void spx_socket_accept_nb(int fd);
err_t spx_socket_start(const int fd,\
        string_t ip,const int port,\
        bool_t is_keepalive,size_t alive_timeout,\
        size_t detect_times,size_t detect_timeout,\
        bool_t is_linger,size_t linger_timeout,\
        bool_t is_nodelay,\
        bool_t is_timeout,size_t timeout,\
        size_t listens);
string_t spx_ip_get(int sock,err_t *err);

string_t spx_host_tostring(struct spx_host *host,err_t *err);

#ifdef __cplusplus
}
#endif
#endif