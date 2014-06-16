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
            if(EAGAIN == errno || EWOULDBLOCK == errno){
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
            if(EAGAIN == errno || EWOULDBLOCK == errno){
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
