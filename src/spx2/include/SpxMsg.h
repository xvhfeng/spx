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
 *       Filename:  SpxMsg.h
 *        Created:  2015年01月21日 12时08分55秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/
#ifndef _SPXMSG_H_
#define _SPXMSG_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>

#include "SpxTypes.h"
#include "SpxObject.h"
#include "SpxMFunc.h"
#include "SpxMsg.h"

#define SpxMsgHeaderSize (4 * sizeof(u32_t) + 2 * sizeof(u64_t) + sizeof(char))

    struct SpxMsgHeader{
        u32_t version;
        u32_t protocol;
        u64_t bodylen;
        u64_t offset;
        u32_t charset;
        bool_t isKeepalive;
        u32_t err;
    };

#define __SpxMsgPackI32(s,offset,i,err) spxMsgPackU32(s,offset,(u32_t) (i),err)
#define __SpxMsgPackI64(s,offset,i,err) spxMsgPackU64(s,offset,(u64_t) (i),err)
#define __SpxMsgUnpackI32(s,offset,err) ((i32_t) spxMsgUnpackU32(s,offset,err))
#define __SpxMsgUnpackI64(s,offset,err) ((i64_t) spxMsgUnpackU64(s,offset,err))


string_t spxMsgPackU32(string_t s,i32_t offset,u32_t n,err_t *err);
string_t spxMsgPackU64(string_t s,i32_t offset,u64_t n,err_t *err);
string_t spxMsgPackBool(string_t s,i32_t offset, bool_t b,err_t *err);
string_t spxMsgPackString(string_t s,i32_t offset,string_t v,err_t *err);
string_t spxMsgPackAlignedString(string_t s,i32_t offset,string_t v,size_t align,err_t *err);
string_t spxMsgPackChars(string_t s,i32_t offset,char *v,err_t *err);
string_t spxMsgPackAlginedChars(string_t s,i32_t offset,char *v,size_t algin,err_t *err);
string_t spxMsgPackDouble(string_t s,i32_t offset,double d,err_t *err);
string_t spxMsgPackFloat(string_t s,i32_t offset,float f,err_t *err);

u32_t spxMsgUnpackU32(string_t s,i32_t offset,err_t *err);
u64_t spxMsgUnpackU64(string_t s,i32_t offset,err_t *err);
bool_t spxMsgUnpackBool(string_t s,i32_t offset,err_t *err);
string_t spxMsgUnpackString(string_t s,i32_t offset,size_t len,err_t *err);
double spxMsgUnpackDouble(string_t s,i32_t offset,err_t *err);
float spxMsgUnpackFloat(string_t s,i32_t offset,err_t *err);

struct SpxMsgHeader *spxMsgUnpackHeader(string_t s,i32_t offset,err_t *err);
string_t spxMsgPackHeader(string_t s,struct SpxMsgHeader *header,err_t *err);

#ifdef __cplusplus
}
#endif
#endif
