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

err_t spx_socket_nb(int sock) {
	int flags;
	err_t err = 0;
	if (-1 == (flags = fcntl(sock, F_GETFL))) {
		err = errno;
		return -1;
	}
	flags |= O_NONBLOCK;
	if (-1 == fcntl(sock, F_SETFL, flags)) {
		err = errno;
		return -1;
	}
	return err;
}
