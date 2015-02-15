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
 *       Filename:  SpxMsg.c
 *        Created:  2015年01月21日 12时09分01秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "SpxTypes.h"
#include "SpxMFunc.h"
#include "SpxVars.h"
#include "SpxError.h"
#include "SpxObject.h"
#include "SpxMsg.h"
#include "SpxString.h"

#define _SpxMsgHeaderOffsetOfVersion 0
#define _SpxMsgHeaderOffsetOfProtocol sizeof(u32_t)
#define _SpxMsgHeaderOffsetOfCharset (2 * sizeof(u32_t))
#define _SpxMsgHeaderOffsetOfBodylen (3 * sizeof(u32_t))
#define _SpxMsgHeaderOffsetOfOffset (3 * sizeof(u32_t) + sizeof(u64_t))
#define _SpxMsgHeaderOffsetOfIsKeepalive (3 * sizeof(u32_t) + 2 * sizeof(u64_t))
#define _SpxMsgHeaderOffsetOfErr (3 * sizeof(u32_t) + 2 * sizeof(u64_t) + SpxBoolTransportSize)


private void _spxU32ToMsg(string_t s,const u32_t n){/*{{{*/
    *s++ = (n >> 24) & 0xFF;
    *s++ = (n >> 16) & 0xFF;
    *s++ = (n >> 8) & 0xFF;
    *s++ = n & 0xFF;
}/*}}}*/

private void _spxU64ToMsg(string_t s,const u64_t n){/*{{{*/
    *s++ = (n >> 56) & 0xFF;
    *s++ = (n >> 48) & 0xFF;
    *s++ = (n >> 40) & 0xFF;
    *s++ = (n >> 32) & 0xFF;
    *s++ = (n >> 24) & 0xFF;
    *s++ = (n >> 16) & 0xFF;
    *s++ = (n >> 8) & 0xFF;
    *s++ = n & 0xFF;
}/*}}}*/

private u32_t _spxMsgToU32(string_t s){/*{{{*/
    u32_t n =  (u32_t ) ((((u32_t) (*s)) << 24)
            | (((u32_t) (*(s + 1))) << 16)
            | (((u32_t) (*(s+2))) << 8)
            | ((u32_t) (*(s+3))));
    return n;
}/*}}}*/

private u64_t _spxMsgToU64(string_t s){/*{{{*/
    u64_t n =  (((u64_t) (*s)) << 56)
        | (((u64_t) (*(s+1))) << 48)
        | (((u64_t) (*(s + 2))) << 40)
        | (((u64_t) (*(s + 3))) << 32)
        | (((u64_t) (*(s + 4))) << 24)
        | (((u64_t) (*(s + 5))) << 16)
        | (((u64_t) (*(s + 6))) << 8)
        | ((u64_t) (*(s + 7)));
    return n;
}/*}}}*/

string_t spxMsgPackU32(string_t s,i32_t offset,u32_t n,err_t *err){/*{{{*/
    if(NULL == s){
        *err =  EINVAL;
        return NULL;
    }
    char buff[sizeof(u32_t)] = {0};
    _spxU32ToMsg(buff,n);
    return spxStringInsert(s,offset,buff,sizeof(u32_t),err);
}/*}}}*/

string_t spxMsgPackU64(string_t s,i32_t offset,u64_t n,err_t *err){/*{{{*/
    if(NULL == s){
        *err =  EINVAL;
        return NULL;
    }
    char buff[sizeof(u64_t)] = {0};
    _spxU32ToMsg(buff,n);
    return spxStringInsert(s,offset,buff,sizeof(u64_t),err);
}/*}}}*/

string_t spxMsgPackBool(string_t s,i32_t offset, bool_t b,err_t *err){/*{{{*/
    if(NULL == s){
        *err = EINVAL;
        return NULL;
    }
    char buff[SpxBoolTransportSize] = {0};
    buff[0] = (char) (b ? 1 : 0);
    return spxStringInsert(s,offset,buff,SpxBoolTransportSize,err);
}/*}}}*/

string_t spxMsgPackString(string_t s,i32_t offset,string_t v,err_t *err){/*{{{*/
    if(NULL == s){
        *err = EINVAL;
        return NULL;
    }
    return spxStringInsert(s,offset,v,__spxStringLength(v),err);
}/*}}}*/

string_t spxMsgPackAlignedString(string_t s,i32_t offset,string_t v,size_t align,err_t *err){/*{{{*/
    if(NULL == s){
        *err = EINVAL;
        return NULL;
    }

    return spxStringAlignedInsert(s,-1,v,__spxStringLength(v),align,err);
}/*}}}*/

string_t spxMsgPackChars(string_t s,i32_t offset,char *v,err_t *err){/*{{{*/
    if(NULL == s){
        *err = EINVAL;
        return NULL;
    }
    return spxStringInsert(s,offset,v,strlen(v),err);
}/*}}}*/

string_t spxMsgPackAlginedChars(string_t s,i32_t offset,char *v,size_t align,err_t *err){/*{{{*/
    if(NULL == s){
        *err = EINVAL;
        return NULL;
    }
    return spxStringAlignedInsert(s,-1,v,strlen(v),align,err);
}/*}}}*/

string_t spxMsgPackDouble(string_t s,i32_t offset,double d,err_t *err){/*{{{*/
    if(NULL == s){
        *err = EINVAL;
        return 0;
    }
    union SpxDoubleToU64 dtu;
    __SpxZero(dtu);
    dtu.d = d;
    return spxMsgPackU64(s,offset,(u64_t) dtu.u,err);
}/*}}}*/

string_t spxMsgPackFloat(string_t s,i32_t offset,float f,err_t *err){/*{{{*/
    if(NULL == s){
        *err = EINVAL;
        return 0;
    }
    union SpxFloatToU32 ftu;
    __SpxZero(ftu);
    ftu.f = f;
    return spxMsgPackU32(s,offset,(u32_t) ftu.u,err);
}/*}}}*/

u32_t spxMsgUnpackU32(string_t s,i32_t offset,err_t *err){/*{{{*/
    if(NULL == s){
        *err = EINVAL;
        return 0;
    }
    size_t len = __spxStringLength(s);
    if(offset < 0){
        offset = len + offset;
    }
    if(offset >(i32_t) len || 0 > offset){
        *err = SpxEOverFlow;
        return 0;
    }
    return _spxMsgToU32(s + offset);
}/*}}}*/

u64_t spxMsgUnpackU64(string_t s,i32_t offset,err_t *err){/*{{{*/
    if(NULL == s){
        *err = EINVAL;
        return 0;
    }
    size_t len = __spxStringLength(s);
    if(offset < 0){
        offset = len + offset;
    }
    if(offset >(i32_t) len || 0 > offset){
        *err = SpxEOverFlow;
        return 0;
    }
    return _spxMsgToU64(s + offset);
}/*}}}*/

bool_t spxMsgUnpackBool(string_t s,i32_t offset,err_t *err){/*{{{*/
    if(NULL == s){
        *err = EINVAL;
        return 0;
    }
    size_t len = __spxStringLength(s);
    if(offset < 0){
        offset = len + offset;
    }
    if(offset >(i32_t) len || 0 > offset){
        *err = SpxEOverFlow;
        return 0;
    }
    return ((bool_t) ('1' == s[offset]));
}/*}}}*/

string_t spxMsgUnpackString(string_t s,i32_t offset,size_t len,err_t *err){/*{{{*/
    if(NULL == s){
        *err = EINVAL;
        return NULL;
    }
    size_t curlen = __spxStringLength(s);
    if(offset < 0){
        offset = curlen + offset;
    }
    if(offset >(i32_t) curlen || 0 > offset){
        *err = SpxEOverFlow;
        return 0;
    }
    return spxStringRangeNew(s,offset,offset + len,err);
}/*}}}*/

double spxMsgUnpackDouble(string_t s,i32_t offset,err_t *err){/*{{{*/
    if(NULL == s){
        *err = EINVAL;
        return 0;
    }
    size_t len = __spxStringLength(s);
    if(offset < 0){
        offset = len + offset;
    }
    if(offset > (i32_t) len || 0 > offset){
        *err = SpxEOverFlow;
        return 0;
    }
    union SpxDoubleToU64 dtu;
    __SpxZero(dtu);
    dtu.u = spxMsgUnpackU64(s,offset,err);
    return dtu.d;
}/*}}}*/

float spxMsgUnpackFloat(string_t s,i32_t offset,err_t *err){/*{{{*/
    if(NULL == s){
        *err = EINVAL;
        return 0;
    }
    size_t len = __spxStringLength(s);
    if(offset < 0){
        offset = len + offset;
    }
    if(offset >(i32_t) len || 0 > offset){
        *err = SpxEOverFlow;
        return 0;
    }
    union SpxFloatToU32 ftu;
    __SpxZero(ftu);
    ftu.u = spxMsgUnpackU32(s,offset,err);
    return ftu.f;
}/*}}}*/


struct SpxMsgHeader *spxMsgUnpackHeader(string_t s,i32_t offset,err_t *err){/*{{{*/
    struct SpxMsgHeader *header = __SpxObjectNew(struct SpxMsgHeader ,err);
    if(NULL == header) return NULL;
    header->version = spxMsgUnpackU32(s,_SpxMsgHeaderOffsetOfVersion,err);
    if(0 != err) goto r1;
    header->protocol = spxMsgUnpackU32(s,_SpxMsgHeaderOffsetOfProtocol,err);
    if(0 != err) goto r1;
    header->charset = spxMsgUnpackU32(s,_SpxMsgHeaderOffsetOfCharset,err);
    if(0 != err) goto r1;
    header->bodylen = spxMsgUnpackU64(s,_SpxMsgHeaderOffsetOfBodylen,err);
    if(0 != err) goto r1;
    header->offset = spxMsgUnpackU64(s,_SpxMsgHeaderOffsetOfOffset,err);
    if(0 != err) goto r1;
    header->isKeepalive = spxMsgUnpackBool(s,_SpxMsgHeaderOffsetOfIsKeepalive,err);
    if(0 != err) goto r1;
    header->err = spxMsgUnpackU32(s,_SpxMsgHeaderOffsetOfErr,err);
    if(0 != err) goto r1;
    return header;
r1:
    __SpxObjectFree(header);
    return NULL;
}/*}}}*/

string_t spxMsgPackHeader(string_t s,struct SpxMsgHeader *header,err_t *err){/*{{{*/
    string_t news = NULL,newst = NULL;
    bool_t isAlloc = false;
    if(NULL == s){
        news = __SpxStringNewLength(SpxMsgHeaderSize,err);
        if(NULL == news){
            return NULL;
        }
        isAlloc = true;
    }else {
        news = s;
    }

    newst = spxMsgPackU32(news,_SpxMsgHeaderOffsetOfVersion,header->version,err);
    if(NULL == newst) goto r1;
    news = newst;
    newst = spxMsgPackU32(news,_SpxMsgHeaderOffsetOfProtocol,header->protocol,err);
    if(NULL == newst) goto r1;
    news = newst;
    newst = spxMsgPackU32(news,_SpxMsgHeaderOffsetOfCharset,header->charset,err);
    if(NULL == newst) goto r1;
    news = newst;
    newst = spxMsgPackU64(news,_SpxMsgHeaderOffsetOfBodylen,header->bodylen,err);
    if(NULL == newst) goto r1;
    news = newst;
    newst = spxMsgPackU64(news,_SpxMsgHeaderOffsetOfOffset,header->offset,err);
    if(NULL == newst) goto r1;
    news = newst;
    newst = spxMsgPackBool(news,_SpxMsgHeaderOffsetOfIsKeepalive,header->isKeepalive,err);
    if(NULL == newst) goto r1;
    news = newst;
    newst = spxMsgPackU32(news,_SpxMsgHeaderOffsetOfErr,header->err,err);
    if(NULL == newst) goto r1;
    return newst;
r1:
    if(isAlloc){
        __SpxStringFree(news);
    }
    return NULL;
}/*}}}*/

