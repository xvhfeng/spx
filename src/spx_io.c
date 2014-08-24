/*
 * =====================================================================================
 *
 *       Filename:  spx_io.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/06/12 11时36分55秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include  <string.h>

#ifdef SpxMac
    #include <sys/types.h>
     #include <sys/socket.h>
     #include <sys/uio.h>
#endif

#ifdef SpxLinux
#include <sys/sendfile.h>
#endif



#include "include/spx_types.h"
#include "include/spx_defs.h"
#include "include/spx_errno.h"
#include "include/spx_message.h"

err_t spx_read(int fd,byte_t *buf,const size_t size,size_t *len){
    SpxErrReset;
    *len = 0;
    err_t err = 0;
    i64_t rc = 0;
    while(*len < size){
        rc = read(fd,((char *) buf) + *len,size - *len);
        if(0 > rc){
            if(EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno){
                continue;
            }
            err = errno;
            break;
        }else if(0 == rc){
            break;
        }else {
            *len += rc;
        }
    }
    return err;
}

err_t spx_write(int fd,byte_t *buf,const size_t size,size_t *len){
    SpxErrReset;
    *len = 0;
    err_t err = 0;
    i64_t rc = 0;
    while(*len < size){
        rc = write(fd,((char *) buf) + *len,size - *len);
        if(0 > rc){
            if(EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno){
                continue;
            }
            err = errno;
            break;
        }else if(0 == rc){
            break;
        }else {
            *len += rc;
        }
    }
    return err;
}


err_t spx_read_nb(int fd,byte_t *buf,const size_t size,size_t *len){
    SpxErrReset;
    *len = 0;
    err_t err = 0;
    i64_t rc = 0;
    while(*len < size){
        rc = read(fd,((char *) buf) + *len,size - *len);
        if(0 > rc){
            if(EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno){
                continue;
            }
            err = errno;
            break;
        }else if(0 == rc){
            break;
        }else {
            *len += rc;
        }
    }
    return err;
}

err_t spx_write_nb(int fd,byte_t *buf,const size_t size,size_t *len){
    SpxErrReset;
    *len = 0;
    err_t err = 0;
    i64_t rc = 0;
    while(*len < size){
        rc = write(fd,((char *) buf) + *len,size - *len);
        if(0 > rc){
            if(EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno){
                continue;
            }
            err = errno;
            break;
        }else if(0 == rc){
            break;
        }else {
            *len += rc;
        }
    }
    return err;
}

err_t spx_read_to_msg(int fd,struct spx_msg *ctx,const size_t size,size_t *len){
    err_t err = spx_read(fd,(byte_t *) ctx->last ,size,len);
    spx_msg_seek(ctx,spx_msg_len(ctx),SpxMsgSeekSet);
    return err;
}


err_t spx_read_to_msg_nb(int fd,struct spx_msg *ctx,const size_t size,size_t *len){
    err_t err = spx_read_nb(fd,(byte_t *) ctx->last,size,len);
    spx_msg_seek(ctx,spx_msg_len(ctx),SpxMsgSeekSet);
    return err;
}

err_t spx_write_from_msg(int fd,struct spx_msg *ctx,const size_t size,size_t *len){
    return  spx_write(fd,(byte_t *) ctx->buf,size,len);
}

err_t spx_write_from_msg_nb(int fd,struct spx_msg *ctx,const size_t size,size_t *len){
    return  spx_write_nb(fd,(byte_t *) ctx->buf,size,len);
}


err_t spx_set_nb(int fd) {
	int flags;
	err_t err = 0;
	if (-1 == (flags = fcntl(fd, F_GETFL))) {
		err = errno;
		return -1;
	}
	flags |= O_NONBLOCK;
	if (-1 == fcntl(fd, F_SETFL, flags)) {
		err = errno;
		return -1;
	}
	return err;
}

err_t spx_fwrite_string(FILE *fp,string_t s,size_t size,size_t *len){
    SpxErrReset;
    *len = 0;
    err_t err = 0;
    i64_t rc = 0;
    while(*len < size){
        rc = fwrite(s + *len,size,sizeof(char),fp);
        if(0 > rc){
            if(EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno){
                continue;
            }
            err = errno;
            break;
        }else if(0 == rc){
            break;
        }else {
            *len += rc;
        }
    }
    return err;
}

err_t spx_sendfile(int sock,int fd,off_t offset,size_t size,size_t *len){
    off_t offset_new = offset;
    size_t want = size;
    err_t err = 0;
    *len = 0;
#ifdef SpxMac
    while(true){
        if(0 != sendfile(fd,sock,offset_new,&want,NULL, 0)){
            if(EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno){
                *len += want;
                if(*len == size){
                    err = 0;
                    break;
                }
                offset_new = offset + want;
                want = size - offset_new;
                continue;
            }else {
                err = errno;
                return err;
            }
        }
        *len += want;
        if(*len == size){
            err = 0;
            break;
        }
        offset_new = offset + want;
        want = size - offset_new;
    }

#endif

#ifdef SpxLinux
    while(true){
        size_t sendbytes = 0;
        sendbytes = sendfile(sock,fd,&offset_new,want);
        if(-1 == sendbytes){
            if(EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno){
                continue;
            }else {
                err = errno;
                return err;
            }
        }
        *len += sendbytes;
        if(*len == size){
            break;
        }
        offset_new += sendbytes;
        want = size - offset_new;
    }
#endif
    return err;
}

