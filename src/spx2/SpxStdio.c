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
 *       Filename:  SpxStdio.c
 *        Created:  2015年01月27日 15时55分20秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
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

#include "SpxTypes.h"
#include "SpxMFunc.h"
#include "SpxStdio.h"
#include "SpxVars.h"

err_t spxWriteWithNoAck(int fd,string_t buf,size_t size,size_t *len){/*{{{*/
    *len = 0;
    err_t err = 0;
    i64_t rc = 0;
    while(*len < size){
        rc = write(fd,buf + *len,size - *len);
        if(0 > rc){
            if(__SpxFileDescIsRetry(errno)){
                __SpxReset(errno);
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
    if(0 != err) return err;
    if(size != *len) return EIO;
    return 0;
}/*}}}*/

err_t spxWriteWithAck(int fd,string_t buf,size_t size,size_t *len){/*{{{*/
    *len = 0;
    err_t err = 0;
    i64_t rc = 0;
    while(*len < size){
        rc = write(fd,buf + *len,size - *len);
        if(0 > rc){
            err = errno;
            break;
        }else if(0 == rc){
            break;
        }else {
            *len += rc;
        }
    }
    if(0 != err) return err;
    if(size != *len) return EIO;
    return 0;
}/*}}}*/

err_t spxReadWithNoAck(int fd,string_t buf,size_t size,size_t *len){/*{{{*/
    *len = 0;
    err_t err = 0;
    i64_t rc = 0;
    while(*len < size){
        rc = read(fd,buf + *len,size - *len);
        if(0 > rc){
            if(__SpxFileDescIsRetry(errno)){
                __SpxReset(errno);
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
    if(0 != err) return err;
    if(size != *len) return EIO;
    return 0;
}/*}}}*/

err_t spxReadWithAck(int fd,string_t buf,size_t size,size_t *len){/*{{{*/
    *len = 0;
    err_t err = 0;
    i64_t rc = 0;
    while(*len < size){
        rc = read(fd,buf + *len,size - *len);
        if(0 > rc){
            err = errno;
            break;
        }else if(0 == rc){
            break;
        }else {
            *len += rc;
        }
    }
    if(0 != err) return err;
    if(size != *len) return EIO;
    return 0;
}/*}}}*/

err_t spxFileDescSetNoBlocking(int fd) {/*{{{*/
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
}/*}}}*/

err_t spxSendfileWithNoAck(int sock,int fd,off_t offset,size_t size,size_t *len){/*{{{*/
    off_t offset_new = offset;
    off_t want = (off_t) size;
    err_t err = 0;
    *len = 0;
#ifdef SpxMac
    while(true){
        //want is a in/out paras means want to send length when in and sended
        //length when return
        if(0 != sendfile(fd,sock,offset_new,&want,NULL, 0)){
            if(__SpxFileDescIsRetry(errno)){
                SpxErrReset;
                *len += want;
                if(*len == size){
                    err = 0;
                    break;
                }
                offset_new += want;
                want = size - *len;
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
        offset_new += want;
        want = size - *len;
    }

#endif

#ifdef SpxLinux
    while(true){
        i64_t sendbytes = 0;
        sendbytes = sendfile(sock,fd,&offset_new,want);
        if(-1 == sendbytes){
            if(__SpxFileDescIsRetry(errno)){
                __SpxReset(errno);
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
        offset_new = offset + *len;
        want = size - *len;
    }
#endif
    if(0 != err) return err;
    if(size != *len) return EIO;
    return 0;
}/*}}}*/

err_t spxSendfileWithAck(int sock,int fd,off_t offset,size_t size,size_t *len){/*{{{*/
    off_t offset_new = offset;
    off_t want = (off_t) size;
    err_t err = 0;
    *len = 0;
#ifdef SpxMac
    while(true){
        if(0 != sendfile(fd,sock,offset_new,&want,NULL, 0)){
            err = errno;
            return err;
        }
        *len += want;
        if(*len == size){
            err = 0;
            break;
        }
        offset_new += want;
        want = size - *len;
    }

#endif

#ifdef SpxLinux
    while(true){
        i64_t sendbytes = 0;
        sendbytes = sendfile(sock,fd,&offset_new,want);
        if(-1 == sendbytes){
            err = errno;
            return err;
        }
        *len += sendbytes;
        if(*len == size){
            break;
        }
        offset_new = offset + *len;
        want = size - *len;
    }
#endif
    if(0 != err) return err;
    if(size != *len) return EIO;
    return 0;
}/*}}}*/


