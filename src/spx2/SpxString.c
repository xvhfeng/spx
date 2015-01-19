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


union _DoubleToI64
    double v;
    uint64_t i;
};

union _FloatToi32{
    float v;
    uint32_t i;
};


#define MaxReallocSize (1024*1024)

spx_private void spx_i2b(uchar_t *b,const i32_t n);
spx_private void spx_l2b(uchar_t *b,const i64_t n);

string_t spxStringNew(const var init,size_t len,err_t *err){/*{{{*/
    struct SpxString *ss = NULL;

    size_t realSize = __SpxAligned(len + 1);
    size_t totalSize = sizeof(struct SpxString) + realSize;
    ss = spxAlloc(totalSize,err);
    if(NULL == ss){
        return NULL:
    }
    if(len && init){
        memcpy(ss->_buf,init,len);
        ss->_spxObjectSize = realSize;
        ss->_freeSize = realSize - len;
        ss->_buf[len] = 0;
    } else {
        ss->_spObjectSize = realSize;
        ss->_freeSize = realSize;
    }
    __SpxAtomicVAdd(gSpxMemoryUseSize,totalSize);
    ss->_spxObjectRefs = 1;
    return (string_t) ss->_buf;
}/*}}}*/

string_t spxStringNewLength(size_t len,err_t *err){/*{{{*/
    return spxStringNew(NULL,len,err);
}/*}}}*/

string_t spxStringNewEmpty(err_t *err){/*{{{*/
    return spxStringNew(NULL,0,err);
}/*}}}*/

string_t spxStringConvert(const char *init,err_t *err){/*{{{*/
    size_t len = init ? strlen(init) : 0;
    return spxStringNew(init,len,err);
}/*}}}*/

string_t spxStringDup(const string_t s,err_t *err){/*{{{*/
    return spxStringNew(s,spxStringLength(s),err);
}/*}}}*/

bool_t _spxStringFree(string_t s){/*{{{*/
    if(NULL == s){
        return 0;
    }
    _SpxStringConvert(ss,s);
    if(0 == __SpxAtomicVDecr(ss->_spxObjectRefs)){
        __SpxAtomicVSub(gSpxMemoryUseSize,
                sizeof(struct SpxString) + ss->_spxObjectSize);
        __SpxFree(ss);
        return true;
    }
}/*}}}*/

bool_t _spxStringFreeForce(string_t s){/*{{{*/
    if(NULL == s){
        return true;
    }
    _SpxStringConvert(ss,s);
    __SpxAtomicVSub(gSpxMemoryUseSize,
            sizeof(struct SpxString) + ss->_spxObjectSize);
    __SpxFree(ss);
    return true;
}/*}}}*/

private string_t _spxStringExtend(string_t s, size_t addlen,err_t *err) {/*{{{*/
    struct SpxString *ss, *newss;
    size_t free = __spxStringFreeSize(s);
    size_t len, newlen;

    if (free >= addlen + 1) return s;
    len = __spxStringSize(s);
    newlen = len + addlen + 1;
    if (newlen < MaxReallocSize)
        newlen *= 2;
    else
        newlen += MaxReallocSize;
    newlen = __SpxAligned(newlen);
    _SpxStringConvert(ss,s);
    newss = spxReAlloc(ss, sizeof *newss + newlen,err);
    if (newss == NULL) return NULL;

    newss->_freeSize = newlen - len;
    newss->_spxObjectSize = newlen;
    __SpxAtomicVAdd(gSpxMemoryUseSize,newlen - len);
    memset(newss->_buf + len,0,newss->_freeSize);
    return newss->_buf;
}/*}}}*/

private err_t _spxStringIncrLen(string_t s, int incr){/*{{{*/
    _SpxStringConvert(ss,s);
    if(incr > ss->_freeSize) {
        return SpxENOSpace;
    }
    ss->_freeSize -= incr;
    s[ss->_spxObjectSize - ss->_freeSize] = '\0';
    return 0;
}/*}}}*/

string_t spxStringCatLength(string_t s,const var t,size_t len,err_t *err){/*{{{*/
    if(NULL == s){
        *err = EINVAL;
        return NULL;
    }
    size_t curlen = __spxStringLength(s);
    string_t news = _spxStringExtend(s,len,err);
    if(NULL == news) return NULL;
    _SpxStringConvert(newss,news);
    memcpy(news + curlen, t, len);
    newss->_freeSize -= len;
    news[curlen+len] = '\0';
    return news;
}/*}}}*/

string_t spxStringCatAlign(string_t s,const var t,const size_t len,
        const size_t align,err_t *err){/*{{{*/
    if(NULL == s){
        *err = EINVAL;
        return NULL;
    }
    size_t curlen = spxStringLength(s);
    string_t news = spxStringExtend(s,align,err);
    if(NULL == news) return NULL;
    memcpy(news + curlen,t,__SpxMin(len,align));
    newss->_freeSize -= align;
    news[curlen + align] = 0;
    return news;
}/*}}}*/

string_t spxStringCat(string_t s,const char *t,err_t *err){/*{{{*/
    return spxStringCatLength(s,t,strlen(t),err);
}/*}}}*/

string_t spxStringCatString(string_t s,const string_t t,err_t *err){/*{{{*/
    return spxStringCatLength(s,t,__spxStringLength(t),err);
}/*}}}*/

string_t spxStringCopyLength(string_t s,const var t,size_t len,err_t *err){

}








string_t spx_string_cpylen(string_t s, const char *t, size_t len,err_t *err){/*{{{*/
    struct sds *sh = (void*) (s-sizeof *sh);;
    size_t totlen = sh->free+sh->len;

    if (totlen < len) {
        s = spxStringMakeRoomFor(s,len-sh->len,err);
        if (s == NULL) return NULL;
        sh = (void*) (s-sizeof *sh);;
        totlen = sh->free+sh->len;
    }
    memcpy(s, t, len);
    s[len] = '\0';
    sh->len = len;
    sh->free = totlen-len;
    return s;

}/*}}}*/

string_t spx_string_cpy(string_t s, const char *t,err_t *err){/*{{{*/
    return spx_string_cpylen(s, t, strlen(t),err);
}/*}}}*/

string_t spx_string_cat_vprintf(err_t *err,string_t s, \
        const char *fmt, va_list ap){/*{{{*/
    va_list cpy;
    char *buf, *t;
    size_t buflen = 16;

    while(1) {
        buf = spx_alloc_alone(buflen,err);
        if (buf == NULL) return NULL;
        buf[buflen-2] = '\0';
        va_copy(cpy,ap);
        vsnprintf(buf, buflen, fmt, cpy);
        if (buf[buflen-2] != '\0') {
            SpxFree(buf);
            buflen *= 2;
            continue;
        }
        break;
    }
    t = spx_string_cat(s, buf,err);
    SpxFree(buf);
    return t;
}/*}}}*/

string_t spx_string_cat_printf(err_t *err,string_t s, const char *fmt, ...){/*{{{*/
    va_list ap;
    char *t;
    va_start(ap, fmt);
    t = spx_string_cat_vprintf(err,s,fmt,ap);
    va_end(ap);
    return t;
}/*}}}*/

void spx_string_trim(string_t s, const char *cset){/*{{{*/
    struct sds *sh = (void*) (s-sizeof *sh);;
    char *start, *end, *sp, *ep;
    size_t len;

    sp = start = s;
    ep = end = s+spx_string_len(s)-1;
    while(sp <= end && strchr(cset, *sp)) sp++;
    while(ep > start && strchr(cset, *ep)) ep--;
    len = (sp > ep) ? 0 : ((ep-sp)+1);
    if (sh->buf != sp) memmove(sh->buf, sp, len);
    sh->buf[len] = '\0';
    sh->free = sh->free+(sh->len-len);
    sh->len = len;
}/*}}}*/

void spx_string_rtrim(string_t s, const char *cset){/*{{{*/
    struct sds *sh = (void*) (s-sizeof *sh);;
    char *start, *end, *sp, *ep;
    size_t len;

    sp = start = s;
    ep = end = s+spx_string_len(s)-1;
    while(ep >= start && strchr(cset, *ep)) ep--;
    len = (sp > ep) ? 0 : ((ep-sp)+1);
    if (sh->buf != sp) memmove(sh->buf, sp, len);
    sh->buf[len] = '\0';
    sh->free = sh->free+(sh->len-len);
    sh->len = len;
}/*}}}*/

void spx_string_ltrim(string_t s, const char *cset){/*{{{*/
    struct sds *sh = (void*) (s-sizeof *sh);;
    char *start, *end, *sp, *ep;
    size_t len;

    sp = start = s;
    ep = end = s+spx_string_len(s)-1;
    while(sp <= end && strchr(cset, *sp)) sp++;
    len = (sp > ep) ? 0 : ((ep-sp)+1);
    if (sh->buf != sp) memmove(sh->buf, sp, len);
    sh->buf[len] = '\0';
    sh->free = sh->free+(sh->len-len);
    sh->len = len;
}/*}}}*/

void spx_string_strip_linefeed(string_t s){
    spx_string_rtrim(s," ");
    spx_string_rtrim(s, SpxLineEndDlmtString);
}

void spx_string_range(string_t s, int start, int end){/*{{{*/
    struct sds *sh = (void*) (s-sizeof *sh);;
    size_t newlen, len = spx_string_len(s);

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
    if (start && newlen) memmove(sh->buf, sh->buf+start, newlen);
    sh->buf[newlen] = 0;
    sh->free = sh->free+(sh->len-newlen);
    sh->len = newlen;
}/*}}}*/


string_t spx_string_range_new(string_t s, int start, int end,err_t *err){/*{{{*/
    struct sds *sh = (void*) (s-sizeof *sh);;
    size_t newlen, len = spx_string_len(s);

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
    string_t new = spx_string_newlen(sh->buf + start,newlen,err);
    return new;
}/*}}}*/

void spx_string_updatelen(string_t s){/*{{{*/
    struct sds *sh = (void*) (s-sizeof *sh);;
    int reallen = strlen(s);
    sh->free += (sh->len-reallen);
    sh->len = reallen;
}/*}}}*/

void spx_string_clear(string_t s){/*{{{*/
    struct sds *sh = (void*) (s-sizeof *sh);;
    sh->free += sh->len;
    sh->len = 0;
    sh->buf[0] = '\0';
}/*}}}*/

int spx_string_cmp(const string_t s1, const string_t s2){/*{{{*/
    size_t l1, l2, minlen;
    int cmp;

    l1 = spx_string_len(s1);
    l2 = spx_string_len(s2);
    minlen = (l1 < l2) ? l1 : l2;
    cmp = memcmp(s1,s2,minlen);
    if (cmp == 0) return l1-l2;
    return cmp;
}/*}}}*/

int spx_string_casecmp(const string_t s1, const char *s2){/*{{{*/
    size_t l1, l2, minlen;
    int cmp;

    l1 = spx_string_len(s1);
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

bool_t spx_string_begin_with_string(const string_t s1,const string_t s2){/*{{{*/
    size_t l1, l2;

    l1 = spx_string_len(s1);
    l2 = spx_string_len(s2);
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


bool_t spx_string_begin_with(const string_t s1,const char *s2){/*{{{*/
    size_t l1, l2;

    l1 = spx_string_len(s1);
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


bool_t spx_string_end_with_string(const string_t s1,const string_t s2){/*{{{*/
    size_t l1, l2;

    l1 = spx_string_len(s1);
    l2 = spx_string_len(s2);
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


bool_t spx_string_begin_casewith_string(const string_t s1,const string_t s2){/*{{{*/
    size_t l1, l2;

    l1 = spx_string_len(s1);
    l2 = spx_string_len(s2);
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


bool_t spx_string_begin_casewith(const string_t s1,const char *s2){/*{{{*/
    size_t l1, l2;

    l1 = spx_string_len(s1);
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


bool_t spx_string_end_casewith_string(const string_t s1,const string_t s2){/*{{{*/
    size_t l1, l2;

    l1 = spx_string_len(s1);
    l2 = spx_string_len(s2);
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

int spx_string_casecmp_string(const string_t s1, const string_t s2){/*{{{*/
    size_t l1, l2, minlen;
    int cmp;

    l1 = spx_string_len(s1);
    l2 = spx_string_len(s2);
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

string_t *spx_string_split(string_t s,
        const char *sep,int seplen,\
        int *count,err_t *err){
    return spx_string_splitlen(s,spx_string_len(s),sep,seplen,count,err);
}

string_t *spx_string_split_string(string_t s,string_t sep,int *count,err_t *err){
    return spx_string_splitlen(s,spx_string_len(s),sep,spx_string_len(sep),count,err);
}

string_t *spx_string_splitlen(const char *s,\
        int len, const char *sep, int seplen, \
        int *count,err_t *err){/*{{{*/
    int elements = 0, slots = 5, start = 0, j;
    string_t *tokens;

    if (seplen < 1 || len < 0) return NULL;

    tokens = spx_alloc_alone(sizeof(string_t)*slots,err);
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
            newtokens = spx_realloc(tokens,sizeof(struct sds)*slots,err);
            if (newtokens == NULL) goto cleanup;
            tokens = newtokens;
        }
        /* search the separator */
        if ((seplen == 1 && *(s+j) == sep[0]) || (memcmp(s+j,sep,seplen) == 0)) {
            tokens[elements] = spx_string_newlen(s+start,j-start,err);
            if (tokens[elements] == NULL) goto cleanup;
            elements++;
            start = j+seplen;
            j = j+seplen-1; /* skip the separator */
        }
    }
    /* Add the final element. We are sure there is room in the tokens array. */
    tokens[elements] = spx_string_newlen(s+start,len-start,err);
    if (tokens[elements] == NULL) goto cleanup;
    elements++;
    *count = elements;
    return tokens;

cleanup:
    {
        int i;
        for (i = 0; i < elements; i++) spx_string_free(tokens[i]);
        SpxFree(tokens);
        *count = 0;
        return NULL;
    }
}/*}}}*/

void spx_string_free_splitres(string_t *tokens, int count){/*{{{*/
    if (!tokens) return;
    while(count--)
        spx_string_free(tokens[count]);
    SpxFree(tokens);
}/*}}}*/

void spx_string_tolower(string_t s){/*{{{*/
    int len = spx_string_len(s), j;
    for (j = 0; j < len; j++) s[j] = tolower(s[j]);
}/*}}}*/

void spx_string_toupper(string_t s){/*{{{*/
    int len = spx_string_len(s), j;
    for (j = 0; j < len; j++) s[j] = toupper(s[j]);
}/*}}}*/

string_t spx_string_from_i64(i64_t value,err_t *err){/*{{{*/
    char buf[SpxI64MaxLength + 1], *p;
    unsigned long long v;

    v = (value < 0) ? -value : value;
    p = buf+(SpxI64MaxLength); /* point to the last character */
    do {
        *p-- = '0'+(v%10);
        v /= 10;
    } while(v);
    if (value < 0) *p-- = '-';
    p++;
    return spx_string_newlen(p,SpxI64MaxLength -(p-buf),err);
}/*}}}*/

string_t spx_string_catrepr(string_t s, const char *p, size_t len,err_t *err){/*{{{*/
    s = spx_string_catlen(s,"\"",1,err);
    while(len--) {
        switch(*p) {
            case '\\':
            case '"':
                s = spx_string_cat_printf(err,s,"\\%c",*p);
                break;
            case '\n': s = spx_string_catlen(s,"\\n",2,err); break;
            case '\r': s = spx_string_catlen(s,"\\r",2,err); break;
            case '\t': s = spx_string_catlen(s,"\\t",2,err); break;
            case '\a': s = spx_string_catlen(s,"\\a",2,err); break;
            case '\b': s = spx_string_catlen(s,"\\b",2,err); break;
            default:
                       if (isprint(*p))
                           s = spx_string_cat_printf(err,s,"%c",*p);
                       else
                           s = spx_string_cat_printf(err,s,"\\x%02x",(unsigned char)*p);
                       break;
        }
        p++;
    }
    return spx_string_catlen(s,"\"",1,err);
}/*}}}*/


/* Helper function for sdssplitargs() that returns non zero if 'c'
 * is a valid hex digit. */
int is_hex_digit(char c) {/*{{{*/
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}/*}}}*/

/* Helper function for sdssplitargs() that converts a hex digit into an
 * integer from 0 to 15 */
int hex_digit_to_int(char c) {/*{{{*/
    switch(c) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a': case 'A': return 10;
    case 'b': case 'B': return 11;
    case 'c': case 'C': return 12;
    case 'd': case 'D': return 13;
    case 'e': case 'E': return 14;
    case 'f': case 'F': return 15;
    default: return 0;
    }
}/*}}}*/


string_t *spx_string_splitargs(const char *line, int *argc,err_t *err){/*{{{*/
    const char *p = line;
    char *current = NULL;
    char **vector = NULL;

    *argc = 0;
    while(1) {
        /* skip blanks */
        while(*p && isspace(*p)) p++;
        if (*p) {
            /* get a token */
            int inq=0;  /* set to 1 if we are in "quotes" */
            int insq=0; /* set to 1 if we are in 'single quotes' */
            int done=0;

            if (current == NULL) current = spx_string_empty(err);
            while(!done) {
                if (inq) {
                    if (*p == '\\' && *(p+1) == 'x' &&
                            is_hex_digit(*(p+2)) &&
                            is_hex_digit(*(p+3)))
                    {
                        unsigned char byte;
                        byte = (hex_digit_to_int(*(p+2))*16)+
                            hex_digit_to_int(*(p+3));
                        current = spx_string_catlen(current,(char*)&byte,1,err);
                        p += 3;
                    } else if (*p == '\\' && *(p+1)) {
                        char c;
                        p++;
                        switch(*p) {
                            case 'n': c = '\n'; break;
                            case 'r': c = '\r'; break;
                            case 't': c = '\t'; break;
                            case 'b': c = '\b'; break;
                            case 'a': c = '\a'; break;
                            default: c = *p; break;
                        }
                        current =spx_string_catlen(current,&c,1,err);
                    } else if (*p == '"') {
                        /* closing quote must be followed by a space or
                         * nothing at all. */
                        if (*(p+1) && !isspace(*(p+1))) goto err;
                        done=1;
                    } else if (!*p) {
                        /* unterminated quotes */
                        goto err;
                    } else {
                        current = spx_string_catlen(current,p,1,err);
                    }
                } else if (insq) {
                    if (*p == '\\' && *(p+1) == '\'') {
                        p++;
                        current = spx_string_catlen(current,"'",1,err);
                    } else if (*p == '\'') {
                        /* closing quote must be followed by a space or
                         * nothing at all. */
                        if (*(p+1) && !isspace(*(p+1))) goto err;
                        done=1;
                    } else if (!*p) {
                        /* unterminated quotes */
                        goto err;
                    } else {
                        current = spx_string_catlen(current,p,1,err);
                    }
                } else {
                    switch(*p) {
                        case ' ':
                        case '\n':
                        case '\r':
                        case '\t':
                        case '\0':
                            done=1;
                            break;
                        case '"':
                            inq=1;
                            break;
                        case '\'':
                            insq=1;
                            break;
                        default:
                            current = spx_string_catlen(current,p,1,err);
                            break;
                    }
                }
                if (*p) p++;
            }
            /* add the token to the vector */
            vector = spx_realloc(vector,((*argc)+1)*sizeof(char*),err);
            vector[*argc] = current;
            (*argc)++;
            current = NULL;
        } else {
            /* Even on empty input string return something not NULL. */
            if (vector == NULL) vector = spx_alloc_alone(sizeof(void*),err);
            return vector;
        }
    }

err:
    while((*argc)--)
        spx_string_free(vector[*argc]);
    free(vector);
    if (current) spx_string_free(current);
    *argc = 0;
    return NULL;
}/*}}}*/

string_t spx_string_map_chars(string_t s,\
        const char *from, const char *to, size_t setlen){/*{{{*/
    size_t j, i, l = spx_string_len(s);

    for (j = 0; j < l; j++) {
        for (i = 0; i < setlen; i++) {
            if (s[j] == from[i]) {
                s[j] = to[i];
                break;
            }
        }
    }
    return s;
}/*}}}*/

string_t spx_string_join(char **argv, int argc, char *sep, size_t seplen,err_t *err){/*{{{*/
    string_t join = spx_string_empty(err);
    int j;

    for (j = 0; j < argc; j++) {
        join = spx_string_cat(join, argv[j],err);
        if (j != argc-1) join = spx_string_catlen(join,sep,seplen,err);
    }
    return join;
}/*}}}*/

string_t spx_string_join_string(string_t *argv,\
        int argc, const char *sep, size_t seplen,err_t *err){/*{{{*/
    string_t join = spx_string_empty(err);
    int j;

    for (j = 0; j < argc; j++) {
        join = spx_string_cat_string(join, argv[j],err);
        if (j != argc-1) join = spx_string_catlen(join,sep,seplen,err);
    }
    return join;
}/*}}}*/

bool_t spx_string_exist(string_t s,char c){/*{{{*/
    struct sds *sh = NULL;
    sh = (void*) (s-sizeof *sh);;
    int i = 0;
    for( ; i < sh->len; i++){
        if(c == sh->buf[i]){
            return true;
        }
    }
    return false;
}/*}}}*/



string_t spx_string_pack_int(string_t s,const int v,err_t *err){/*{{{*/
    return spx_string_pack_i32(s,(i32_t) v,err);
}/*}}}*/

string_t spx_string_pack_i32(string_t s,const i32_t v,err_t *err){/*{{{*/
    if(NULL == s){
        *err =  EINVAL;
        return NULL;
    }

    struct sds *sh;
    size_t curlen = spx_string_len(s);

    s = spxStringMakeRoomFor(s,sizeof(i32_t),err);
    if (s == NULL) return NULL;
    sh = (void*) (s-sizeof *sh);;
    spx_i2b((uchar_t *) s + curlen,v);

    sh->len = curlen + sizeof(i32_t);
    sh->free = sh->free - sizeof(i32_t);
    s[sh->len] = '\0';
    return s;
}/*}}}*/


string_t spx_string_pack_i64(string_t s,const i64_t v,err_t *err){/*{{{*/
    if(NULL == s){
        *err =  EINVAL;
        return NULL;
    }

    struct sds *sh;
    size_t curlen = spx_string_len(s);

    s = spxStringMakeRoomFor(s,sizeof(i64_t),err);
    if (s == NULL) return NULL;
    sh = (void*) (s-sizeof *sh);;
    spx_l2b((uchar_t *) s + curlen,v);

    sh->len = curlen + sizeof(i64_t);
    sh->free = sh->free - sizeof(i64_t);
    s[sh->len] = '\0';
    return s;
}/*}}}*/

string_t spx_string_pack_u64(string_t s,const u64_t v,err_t *err){
    return spx_string_pack_i64(s,(i64_t) v,err);
}

spx_private void spx_i2b(uchar_t *b,const i32_t n){/*{{{*/
    *b++ = (n >> 24) & 0xFF;
    *b++ = (n >> 16) & 0xFF;
    *b++ = (n >> 8) & 0xFF;
    *b++ = n & 0xFF;
}/*}}}*/
spx_private i32_t spx_b2i(uchar_t *b){/*{{{*/
    i32_t n =  (i32_t ) ((((i32_t) (*b)) << 24)
            | (((i32_t) (*(b + 1))) << 16)
            | (((i32_t) (*(b+2))) << 8)
            | ((i32_t) (*(b+3))));
    b += sizeof(i32_t);
    return n;
}/*}}}*/
spx_private void spx_l2b(uchar_t *b,const i64_t n){/*{{{*/
    *b++ = (n >> 56) & 0xFF;
    *b++ = (n >> 48) & 0xFF;
    *b++ = (n >> 40) & 0xFF;
    *b++ = (n >> 32) & 0xFF;
    *b++ = (n >> 24) & 0xFF;
    *b++ = (n >> 16) & 0xFF;
    *b++ = (n >> 8) & 0xFF;
    *b++ = n & 0xFF;
}/*}}}*/
spx_private i64_t spx_b2l(uchar_t *b){/*{{{*/
    i64_t n =  (((i64_t) (*b)) << 56)
        | (((i64_t) (*(b+1))) << 48)
        | (((i64_t) (*(b + 2))) << 40)
        | (((i64_t) (*(b + 3))) << 32)
        | (((i64_t) (*(b + 4))) << 24)
        | (((i64_t) (*(b + 5))) << 16)
        | (((i64_t) (*(b + 6))) << 8)
        | ((i64_t) (*(b + 7)));
    b += sizeof(i64_t);
    return n;
}/*}}}*/

