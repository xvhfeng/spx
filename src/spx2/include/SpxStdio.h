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
 *       Filename:  SpxStdio.h
 *        Created:  2015年01月27日 15时55分27秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/
#ifndef _SPXSTDIO_H_
#define _SPXSTDIO_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>

#include "SpxTypes.h"


#define __SpxWriteOpen(filename,is_clear) \
    open(filename,is_clear \
            ? O_RDWR|O_CREAT|O_TRUNC \
            : O_RDWR|O_CREAT|O_APPEND,0744)
#define __SpxReadOpen(filename) open(filename,O_RDONLY)
#define __SpxFWriteOpen(filename,is_clear) fopen(filename,is_clear ? "w" : "a")
#define __SpxFReadOpen(filename) fopen(filename,"r")

#define __SpxFileExist(filename) ( 0 == access(filename,F_OK))
#define __SpxFileReadable(filename) (0 == access(filename,R_OK))
#define __SpxFileWritable(filename) (0 == access(filename,W_OK))
#define __SpxFileExecutable(filename) (0 == access(filename,X_OK))


err_t spxFileDescSetNoBlocking(int fd);
err_t spxWriteWithNoAck(int fd,string_t buff,size_t size,size_t *len);
err_t spxWriteWithAck(int fd,string_t buff,size_t size,size_t *len);
err_t spxReadWithNoAck(int fd,string_t buff,size_t size,size_t *len);
err_t spxReadWithAck(int fd,string_t buff,size_t size,size_t *len);
err_t spxSendfileWithNoAck(int sock,int fd,off_t offset,size_t size,size_t *len);
err_t spxSendfileWithAck(int sock,int fd,off_t offset,size_t size,size_t *len);

#ifdef __cplusplus
}
#endif
#endif
