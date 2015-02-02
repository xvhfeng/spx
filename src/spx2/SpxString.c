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
 *       Filename:  SpxString.c
 *        Created:  2015年01月19日 12时43分06秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

#include "SpxTypes.h"
#include "SpxMFunc.h"
#include "SpxVars.h"
#include "SpxError.h"
#include "SpxObject.h"
#include "SpxString.h"
#include "SpxAlloc.h"
#include "SpxAtomic.h"
#include "SpxLimits.h"
#include "SpxCounter.h"

#define _SpxMaxReallocSize (1024*1024)
#define _SpxMinReallocSize 16

private string_t _spxStringExtend(string_t s, size_t addlen,err_t *err);

string_t spxStringNew(const var init,size_t len,err_t *err){/*{{{*/
    struct SpxString *ss = NULL;

    size_t realSize = __SpxAligned(len + 1);
    realSize = __SpxMax(realSize,_SpxMinReallocSize);
    size_t totalSize = sizeof(struct SpxString) + realSize;
    ss = spxAlloc(totalSize,err);
    if(NULL == ss){
        return NULL;
    }
    if(len && init){
        memcpy(ss->_buf,init,len);
        ss->_spxObjectSize = realSize;
        ss->_freeSize = realSize - len;
        ss->_buf[len] = 0;
    } else {
        ss->_spxObjectSize = realSize;
        ss->_freeSize = realSize;
    }
    __SpxAtomicVAdd(gSpxMemoryUseSize,totalSize);
    ss->_spxObjectRefs = 1;
    return (string_t) ss->_buf;
}/*}}}*/

string_t spxStringConvert(const char *init,err_t *err){/*{{{*/
    size_t len = init ? strlen(init) : 0;
    return spxStringNew((const var) init,len,err);
}/*}}}*/

var spxStringRef(string_t s) {/*{{{*/
    if(NULL == s){
        return NULL;
    }
    __SpxStringConvert(ss,s);
    if(0 == ss->_spxObjectRefs ){
        return NULL;
    }
    __SpxAtomicVIncr(ss->_spxObjectRefs);
    return s;
}/*}}}*/

bool_t _spxStringFree(string_t s){/*{{{*/
    if(NULL == s){
        return 0;
    }
    __SpxStringConvert(ss,s);
    if(0 == __SpxAtomicVDecr(ss->_spxObjectRefs)){
        __SpxAtomicVSub(gSpxMemoryUseSize,
                sizeof(struct SpxString) + ss->_spxObjectSize);
        __SpxFree(ss);
        return true;
    }
    return false;
}/*}}}*/

bool_t _spxStringFreeForce(string_t s){/*{{{*/
    if(NULL == s){
        return true;
    }
    __SpxStringConvert(ss,s);
    __SpxAtomicVSub(gSpxMemoryUseSize,
            sizeof(struct SpxString) + ss->_spxObjectSize);
    __SpxFree(ss);
    return true;
}/*}}}*/

private string_t _spxStringExtend(string_t s, size_t addlen,err_t *err) {/*{{{*/
    struct SpxString  *newss;
    size_t free = __spxStringFreeSize(s);
    size_t len, newlen;

    if (free >= addlen + 1) return s;
    len = __spxStringSize(s);
    newlen = len + addlen + 1;
    if (newlen < _SpxMaxReallocSize) {
        newlen *= 2;
        newlen = __SpxMax(newlen,_SpxMinReallocSize);
    } else {
        newlen += _SpxMaxReallocSize;
    }
    newlen = __SpxAligned(newlen);
    __SpxStringConvert(ss,s);
    newss = spxRealloc(ss, sizeof *newss + newlen,err);
    if (newss == NULL) return NULL;

    newss->_freeSize = newlen - len;
    newss->_spxObjectSize = newlen;
    __SpxAtomicVAdd(gSpxMemoryUseSize,newlen - len);
    memset(newss->_buf + len,0,newss->_freeSize);
    return newss->_buf;
}/*}}}*/


string_t spxStringAppend(string_t s,const var t,size_t len,err_t *err){/*{{{*/
    if(NULL == s){
        *err = EINVAL;
        return NULL;
    }
    size_t curlen = __spxStringLength(s);
    string_t news = _spxStringExtend(s,len,err);
    if(NULL == news) return NULL;
    __SpxStringConvert(newss,news);
    memcpy(news + curlen, t, len);
    newss->_freeSize -= len;
    news[curlen+len] = '\0';
    return news;
}/*}}}*/

string_t spxStringAlignedAppend(string_t s,const var t,const size_t len,
        const size_t align,err_t *err){/*{{{*/
    if(NULL == s){
        *err = EINVAL;
        return NULL;
    }
    size_t curlen = __spxStringLength(s);
    string_t news = _spxStringExtend(s,align,err);
    if(NULL == news) return NULL;
    memcpy(news + curlen,t,__SpxMin(len,align));
    __SpxStringConvert(newss,news);
    newss->_freeSize -= align;
    news[curlen + align] = 0;
    return news;
}/*}}}*/

string_t spxStringInsert(string_t s,const i32_t offset,
        const var t,const size_t len,err_t *err){/*{{{*/
    if(-1 == offset){
        return spxStringAppend(s,t,len,err);
    }
    size_t curlen = __spxStringLength(s);
    i32_t newOffset = 0;
    newOffset = 0 > offset ? (i32_t) curlen + offset : offset;

    string_t news = NULL;
    if((i32_t) curlen < newOffset){
        size_t size = __spxStringSize(s);
        if(size <= newOffset + len){
            size_t addlen = newOffset + len - size;
            news = _spxStringExtend(s,addlen,err);
            if(NULL == news) return NULL;
        } else {
            news = s;
        }
        if(t){
            memcpy(news + newOffset,t,len);
        }
        __SpxStringConvert(ss,news);
        ss->_freeSize = ss->_spxObjectSize - (newOffset + len);
        news[newOffset + len] = 0;
        return news;
    }

    size_t freeSize = __spxStringFreeSize(s);
    if(freeSize <= len){
        size_t addlen = len - freeSize;
        news = _spxStringExtend(s,addlen,err);
        if(NULL == news) return NULL;
    } else {
        news = s;
    }
    memmove(news + (newOffset + len),news + newOffset,curlen - newOffset);
    if(t){
        memcpy(news + newOffset,t,len);
    }
    __SpxStringConvert(ss,news);
    ss->_freeSize = ss->_spxObjectSize - (curlen + len);
    news[curlen + len] = 0;
    return news;
}/*}}}*/

string_t spxStringAlignedInsert(string_t s,const i32_t offset,
        const var t,const size_t len,
        size_t align,err_t *err){/*{{{*/
    if(-1 == offset){
        return spxStringAlignedAppend(s,t,len,align,err);
    }

    size_t curlen = __spxStringLength(s);
    i32_t newOffset = 0;
    newOffset = 0 > offset ? (i32_t) curlen + offset : offset;
    string_t news = NULL;
    if((i32_t) curlen < newOffset){
        size_t size = __spxStringSize(s);
        if(size <= newOffset + align){
            size_t addlen = newOffset + align - size;
            news = _spxStringExtend(s,addlen,err);
            if(NULL == news) return NULL;
        } else {
            news = s;
        }
        if(t){
            memcpy(news + newOffset,t,__SpxMin(len,align));
        }
        __SpxStringConvert(ss,news);
        ss->_freeSize = ss->_spxObjectSize - (newOffset + align);
        news[newOffset + align] = 0;
        return news;
    }

    size_t freeSize = __spxStringFreeSize(s);
    if(freeSize <= align){
        size_t addlen = align - freeSize;
        news = _spxStringExtend(s,addlen,err);
        if(NULL == news) return NULL;
    } else {
        news = s;
    }
    memmove(news + (newOffset + align),news + newOffset,curlen - newOffset);
    if(t){
        memcpy(news + newOffset,t,__SpxMin(len,align));
    }
    __SpxStringConvert(ss,news);
    ss->_freeSize = ss->_spxObjectSize - (curlen + align);
    news[curlen + align] = 0;
    return news;
}/*}}}*/

string_t spxStringCopy(string_t s,int begin,const var t,size_t len,err_t *err){/*{{{*/
    size_t curlen = __spxStringLength(s);
    size_t size = __spxStringSize(s);
    if(0 > begin){
        begin = curlen + begin;
    }
    size_t keepSize = size - begin;
    string_t news = NULL;
    if(keepSize < len){
        news = _spxStringExtend(s,len - keepSize,err);
        if(NULL == news) return NULL;
    }else {
        news = s;
    }
    __SpxStringConvert(newss,news);
    memcpy(news + begin,t,len);
    news[begin + len] = 0;
    newss->_freeSize = newss->_spxObjectSize - begin - len;
    return news;
}/*}}}*/

err_t spxStringAppendPrint(string_t *dest,string_t s,const char *fmt,va_list ap){/*{{{*/
    va_list cpy;
    char *buf, *t;
    size_t buflen = 16;
    err_t err = 0;
    int size = 0;

    while(1) {
        buf = spxAllocAlone(buflen,&err);
        if (buf == NULL) return err;
        buf[buflen-2] = '\0';
        va_copy(cpy,ap);
        size = vsnprintf(buf, buflen, fmt, cpy);
        if (buf[buflen-2] != '\0') {
            __SpxReset(size);
            __SpxFree(buf);
            buflen *= 2;
            continue;
        }
        break;
    }
    *dest = spxStringAppend(s, buf,size,&err);
    __SpxFree(buf);
    return 0;

}/*}}}*/

err_t spxStringAppendFormat(string_t *dest,string_t s,const char *fmt,...){/*{{{*/
    va_list ap;
    char *t;
    err_t err = 0;
    va_start(ap, fmt);
    err = spxStringAppendPrint(dest,s,fmt,ap);
    if(0 != err){

    }
    va_end(ap);
    return err;
}/*}}}*/

void spxStringRange(string_t s,int start,int end){/*{{{*/
    __SpxStringConvert(ss,s);
    size_t newlen, len = __spxStringLength(s);

    if (len == 0) return;
    if (start < 0) {
        start = len+start;
        if (start < 0) start = 0;
    }
    if (end < 0) {
        end = len+end;
        if (end < 0) end = 0;
    }
    newlen = (start > end) ? 0 : (end-start)+1;
    if (newlen != 0) {
        if (start >= (signed)len) {
            newlen = 0;
        } else if (end >= (signed)len) {
            end = len-1;
            newlen = (start > end) ? 0 : (end-start)+1;
        }
    } else {
        start = 0;
    }
    if (start && newlen) memmove(ss->_buf, ss->_buf+start, newlen);
    ss->_buf[newlen] = 0;
    ss->_freeSize = ss->_spxObjectSize - newlen;
}/*}}}*/

string_t spxStringRangeNew(string_t s,int start,int end,err_t *err){/*{{{*/
    __SpxStringConvert(ss,s);
    size_t newlen, len = __spxStringLength(s);

    if (len == 0) return NULL;
    if (start < 0) {
        start = len+start;
        if (start < 0) start = 0;
    }
    if (end < 0) {
        end = len+end;
        if (end < 0) end = 0;
    }
    newlen = (start > end) ? 0 : (end-start)+1;
    if (newlen != 0) {
        if (start >= (signed)len) {
            newlen = 0;
        } else if (end >= (signed)len) {
            end = len-1;
            newlen = (start > end) ? 0 : (end-start)+1;
        }
    } else {
        start = 0;
    }
    string_t new = spxStringNew(ss->_buf + start,newlen,err);
    return new;
}/*}}}*/

void spxStringTrimSet(string_t s,const char *cset){/*{{{*/
    __SpxStringConvert(ss,s);
    char *start, *end, *sp, *ep;
    size_t len;

    sp = start = s;
    ep = end = s + __spxStringLength(s)-1;
    while(sp <= end && strchr(cset, *sp)) sp++;
    while(ep > start && strchr(cset, *ep)) ep--;
    len = (sp > ep) ? 0 : ((ep-sp)+1);
    if (ss->_buf != sp) memmove(ss->_buf, sp, len);
    ss->_buf[len] = '\0';
    ss->_freeSize = ss->_spxObjectSize - len;
}/*}}}*/

void spxStringLeftTrimSet(string_t s,const char *cset){/*{{{*/
    __SpxStringConvert(ss,s);
    char *start, *end, *sp, *ep;
    size_t len;

    sp = start = s;
    ep = end = s+__spxStringLength(s)-1;
    while(sp <= end && strchr(cset, *sp)) sp++;
    len = (sp > ep) ? 0 : ((ep-sp)+1);
    if (ss->_buf != sp) memmove(ss->_buf, sp, len);
    ss->_buf[len] = '\0';
    ss->_freeSize = ss->_spxObjectSize - len;
}/*}}}*/

void spxStringRightTrimSet(string_t s,const char *cset){/*{{{*/
    __SpxStringConvert(ss,s);
    char *start, *end, *sp, *ep;
    size_t len;

    sp = start = s;
    ep = end = s+__spxStringLength(s)-1;
    while(ep >= start && strchr(cset, *ep)) ep--;
    len = (sp > ep) ? 0 : ((ep-sp)+1);
    if (ss->_buf != sp) memmove(ss->_buf, sp, len);
    ss->_buf[len] = '\0';
    ss->_freeSize = ss->_spxObjectSize - len;
}/*}}}*/

void spxStringUpdate(string_t s){/*{{{*/
    __SpxStringConvert(ss,s);
    ss->_freeSize = ss->_spxObjectSize - strlen(s);
}/*}}}*/

void spxStringClear(string_t s){/*{{{*/
    __SpxStringConvert(ss,s);
    ss->_freeSize = ss->_spxObjectSize;
    memset(ss->_buf,0,ss->_spxObjectSize);
}/*}}}*/

string_t spxStringFromNumber(i64_t value,err_t *err){/*{{{*/
    char buf[SpxI64Length + 1], *p;
    unsigned long long v;

    v = (value < 0) ? -value : value;
    p = buf+(SpxI64Length); /* point to the last character */
    do {
        *p-- = '0'+(v%10);
        v /= 10;
    } while(v);
    if (value < 0) *p-- = '-';
    p++;
    return spxStringNew(p,SpxI64Length -(p-buf),err);
}/*}}}*/

string_t *spxStringSplitChars(const char *s,int len,
        const char *sep,int seplen,int *count,err_t *err){/*{{{*/
    int elements = 0, slots = 5, start = 0, j;
    string_t *tokens;

    if (seplen < 1 || len < 0) return NULL;

    tokens = spxObjectNewNumbs(sizeof(string_t),slots,err);
    if (tokens == NULL) return NULL;

    if (len == 0) {
        *count = 0;
        return tokens;
    }
    for (j = 0; j < (len-(seplen-1)); j++) {
        /* make sure there is room for the next element and the final one */
        if (slots < elements+2) {
            string_t *newtokens;

            slots *= 2;
            newtokens = spxObjectReNewNumbs(tokens,sizeof(string_t),slots,err);
            if (newtokens == NULL) goto cleanup;
            tokens = newtokens;
        }
        /* search the separator */
        if ((seplen == 1 && *(s+j) == sep[0]) || (memcmp(s+j,sep,seplen) == 0)) {
            tokens[elements] = spxStringNew((var) (s+start),j-start,err);
            if (tokens[elements] == NULL) goto cleanup;
            elements++;
            start = j+seplen;
            j = j+seplen-1; /* skip the separator */
        }
    }
    /* Add the final element. We are sure there is room in the tokens array. */
    tokens[elements] = spxStringNew((var) (s+start),len-start,err);
    if (tokens[elements] == NULL) goto cleanup;
    elements++;
    *count = elements;
    return tokens;
cleanup:
    {
        int i;
        for (i = 0; i < elements; i++) __SpxStringFree(tokens[i]);
        __SpxObjectFree(tokens);
        *count = 0;
        return NULL;
    }
}/*}}}*/

void spxStringSplitFree(string_t *tokens, int count){/*{{{*/
    if (!tokens) return;
    while(count--)
        __SpxStringFree(tokens[count]);
    __SpxObjectFree(tokens);
}/*}}}*/

void spxStringToLower(string_t s){/*{{{*/
    size_t len = __spxStringLength(s), j;
    for (j = 0; j < len; j++) s[j] = tolower(s[j]);
}/*}}}*/

void spxStringToUpper(string_t s){/*{{{*/
    int len = __spxStringLength(s), j;
    for (j = 0; j < len; j++) s[j] = toupper(s[j]);
}/*}}}*/

string_t spxStringJoin(char **argv,int argc,char *sep,size_t seplen,err_t *err){/*{{{*/
    string_t join = __SpxStringNewEmpty(err);
    int j;

    for (j = 0; j < argc; j++) {
        join = __SpxStringAppendChars(join, argv[j],err);
        if (j != argc-1) join = spxStringAppend(join,sep,seplen,err);
    }
    return join;
}/*}}}*/

string_t spxStringJoinString(string_t *argv,int argc,const char *sep,size_t seplen,err_t *err){/*{{{*/
    string_t join = __SpxStringNewEmpty(err);
    int j;

    for (j = 0; j < argc; j++) {
        join = __SpxStringAppendString(join, argv[j],err);
        if (j != argc-1) join = spxStringAppend(join,(var) sep,seplen,err);
    }
    return join;
}/*}}}*/


int spxStringCmp(const string_t s1, const string_t s2){/*{{{*/
    size_t l1, l2, minlen;
    int cmp;

    l1 = __spxStringLength(s1);
    l2 = __spxStringLength(s2);
    minlen = (l1 < l2) ? l1 : l2;
    cmp = memcmp(s1,s2,minlen);
    if (cmp == 0) return l1-l2;
    return cmp;
}/*}}}*/

int spxStringCaseCmp(const string_t s1, const char *s2){/*{{{*/
    size_t l1, l2, minlen;
    int cmp;

    l1 = __spxStringLength(s1);
    l2 = strlen(s2);
    minlen = (l1 < l2) ? l1 : l2;
    char a,b;
    size_t i = 0;
    for(; i < minlen; i++){
        a = *(s1 + i);
        b = *(s2 + i);
        a = (65 <= a && 90 >= a) ? a + 32 : a;
        b = (65 <= b && 90 >= b) ? b + 32 : b;
        cmp = a -b;
        if(0 != cmp) return cmp;
    }
    if (cmp == 0) return l1-l2;
    return cmp;
}/*}}}*/

int spxStringCaseCmpString(const string_t s1, const string_t s2){/*{{{*/
    size_t l1, l2, minlen;
    int cmp;

    l1 = __spxStringLength(s1);
    l2 = __spxStringLength(s2);
    minlen = (l1 < l2) ? l1 : l2;
    char a,b;
    size_t i = 0;
    for(; i < minlen; i++){
        a = *(s1 + i);
        b = *(s2 + i);
        a = (65 <= a && 90 >= a) ? a + 32 : a;
        b = (65 <= b && 90 >= b) ? b + 32 : b;
        cmp = a -b;
        if(0 != cmp) return cmp;
    }
    if (cmp == 0) return l1-l2;
    return cmp;
}/*}}}*/

bool_t spxStringBeginWithString(const string_t s1,const string_t s2){/*{{{*/
    size_t l1, l2;

    l1 = __spxStringLength(s1);
    l2 = __spxStringLength(s2);
    if(l1 < l2) return false;
    char a,b;
    size_t i = 0;
    for(; i < l2; i++){
        a = *(s1 + i);
        b = *(s2 + i);
        if(a != b) return false;
    }
    return true;
}/*}}}*/

bool_t spxStringBeginWith(const string_t s1,const char *s2){/*{{{*/
    size_t l1, l2;

    l1 = __spxStringLength(s1);
    l2 = strlen(s2);
    if(l1 < l2) return false;
    char a,b;
    size_t i = 0;
    for(; i < l2; i++){
        a = *(s1 + i);
        b = *(s2 + i);
        if(a != b) return false;
    }
    return true;
}/*}}}*/

bool_t spxStringBeginCaseWithString(const string_t s1,const string_t s2){/*{{{*/
    size_t l1, l2;

    l1 = __spxStringLength(s1);
    l2 = __spxStringLength(s2);
    if(l1 < l2) return false;
    char a,b;
    size_t i = 0;
    for(; i < l2; i++){
        a = *(s1 + i);
        b = *(s2 + i);
        a = (65 <= a && 90 >= a) ? a + 32 : a;
        b = (65 <= b && 90 >= b) ? b + 32 : b;
        if(a != b) return false;
    }
    return true;
}/*}}}*/

bool_t spxStringBeginCaseWith(const string_t s1,const char *s2){/*{{{*/
    size_t l1, l2;

    l1 = __spxStringLength(s1);
    l2 = strlen(s2);
    if(l1 < l2) return false;
    char a,b;
    size_t i = 0;
    for(; i < l2; i++){
        a = *(s1 + i);
        b = *(s2 + i);
        a = (65 <= a && 90 >= a) ? a + 32 : a;
        b = (65 <= b && 90 >= b) ? b + 32 : b;
        if(a != b) return false;
    }
    return true;
}/*}}}*/

bool_t spxStringEndWithString(const string_t s1,const string_t s2){/*{{{*/
    size_t l1, l2;

    l1 = __spxStringLength(s1);
    l2 = __spxStringLength(s2);
    if(l1 < l2) return false;
    char *p1 = s1 + l1;
    char *p2 = s2 + l2;
    char a,b;
    size_t i = 0;
    for(; i < l2; i++){
        a = *(p1 - i);
        b = *(p2 - i);
        if(a != b) return false;
    }
    return true;
}/*}}}*/

bool_t spxStringEndCaseWithString(const string_t s1,const string_t s2){/*{{{*/
    size_t l1, l2;

    l1 = __spxStringLength(s1);
    l2 = __spxStringLength(s2);
    if(l1 < l2) return false;
    char *p1 = s1 + l1;
    char *p2 = s2 + l2;
    char a,b;
    size_t i = 0;
    for(; i < l2; i++){
        a = *(p1 - i);
        b = *(p2 - i);
        a = (65 <= a && 90 >= a) ? a + 32 : a;
        b = (65 <= b && 90 >= b) ? b + 32 : b;
        if(a != b) return false;
    }
    return true;
}/*}}}*/

bool_t spxStringExist(string_t s,char c){/*{{{*/
    size_t len = __spxStringLength(s);
    size_t i = 0;
    for( ; i < len; i++){
        if(c == s[i]){
            return true;
        }
    }
    return false;
}/*}}}*/

