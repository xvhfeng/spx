#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "include/spx_types.h"
#include "include/spx_alloc.h"
#include "include/spx_string.h"
#include "include/spx_errno.h"
#include "include/spx_defs.h"

#define MaxReallocSize (1024*1024)
/*
spx_private spx_inline err_t spx_split_string_free(void **v);
spx_private spx_inline err_t spx_split_string_add(struct spx_vector *dest,char *p,size_t len);

spx_private uchar_t *spx_vnprintf(uchar_t *buf,uchar_t *last, \
        uchar_t *fmt,va_list args);
spx_private u_char * spx_num_tostring(u_char *buf, \
        u_char *last, u64_t ui64, \
        u_char zero,u_int hexadecimal,\
        u_int width);



err_t spx_string_split(SpxLogDelegate *log,const string_t s,\
        const string_t d,const bool_t isrs,struct spx_vector **dest){
    err_t rc = 0;
    if(SpxStringIsNullOrEmpty(s)){
        return EINVAL;
    }
    if(SpxStringIsNullOrEmpty(d)){
        return ENODLMT;
    }
    if(NULL == (*dest)){
        spx_vector_init(log,dest,spx_split_string_free);
    }
    char *p = SpxString2Char1(s);//cast the ptr
    char *pe = p + SpxStringLength(s);
    size_t ds = SpxStringLength(d);
    char *ptr = NULL;
    while(NULL != (ptr = strstr(p,SpxString2Char2(d)))){
        size_t diff =(size_t)(ptr - p);
        if( 0 == diff){
            if(!isrs){
                if(0 != (rc = spx_split_string_add(*dest,p,0))){
                    SpxLog2(log,SpxLogError,rc,\
                            "add split string to vector is fail.");
                    goto r1;
                }
            }
        }
        if(0 != (rc = spx_split_string_add(*dest,p,diff))){
            SpxLog2(log,SpxLogError,rc,\
                    "add split string to vector is fail.");
            goto r1;
        }
        p += diff + ds;
    }
    size_t last = (size_t) (pe - p);//the last
    if(0 != last){
        if(0 != (rc = spx_split_string_add(*dest,p,last))){
            SpxLog2(log,SpxLogError,rc,\
                    "add split string to vector is fail.");
            goto r1;
        }
    }
    return rc;
r1:
    spx_vector_destory(dest);
    return rc;
}

spx_private spx_inline err_t spx_split_string_add(struct spx_vector *dest,char *p,size_t len){
    string_t sp = NULL;
    err_t rc = 0;
    if(0 != (rc = spx_alloc_string(len,&sp))){
        SpxLog2(dest->log,SpxLogError,rc,\
                "alloc split string is fail.");
        return rc;
    }
    if((0 != len) && (0 != (rc = spx_strcpy(sp,SpxChar2String1(p),len)))){
        SpxLog2(dest->log,SpxLogError,rc,\
                "copy string to split string is fail.");
        SpxFree(sp);
        return rc;
    }
    if(0 != (rc = spx_vector_add(dest,sp))){
        SpxLog2(dest->log,SpxLogError,rc,\
                "add split string to vector is fail.");
        SpxFree(sp);
        return rc;
    }
    return rc;
}

spx_private spx_inline err_t spx_split_string_free(void **v){//{{{
    if(NULL != *v){
        SpxFree(*v);
    }
    return 0;
}//}}}

spx_private uchar_t *spx_vnprintf(uchar_t *buf,uchar_t *last, \
        uchar_t *fmt,va_list args){
    u_char *p, zero;
    int d;
    double f;
    size_t len, slen;
    i64_t i64;
    u64_t ui64, frac;
    u_int width, sign, hex, max_width, frac_width, scale, n;

    string_t v;

    while (*fmt && buf < last) {


         * "buf < last" means that we could copy at least one character:
         * the plain character, "%%", "%c", and minus without the checking
        if (*fmt == '%') {
            i64 = 0;
            ui64 = 0;
            zero = (u_char) ((*++fmt == '0') ? '0' : ' ');
            width = 0;
            sign = 1;
            hex = 0;
            max_width = 0;
            frac_width = 0;
            slen = (size_t) -1;
            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + *fmt++ - '0';
            }
            for ( ;; ) {
                switch (*fmt) {
                    case 'u':
                        sign = 0;
                        fmt++;
                        continue;
                    case 'm':
                        max_width = 1;
                        fmt++;
                        continue;
                    case 'X':
                        hex = 2;
                        sign = 0;
                        fmt++;
                        continue;
                    case 'x':
                        hex = 1;
                        sign = 0;
                        fmt++;
                        continue;
                    case '.':
                        fmt++;
                        while (*fmt >= '0' && *fmt <= '9') {
                            frac_width = frac_width * 10 + *fmt++ - '0';
                        }
                        break;
                    case '*':
                        slen = va_arg(args, size_t);
                        fmt++;
                        continue;
                    default:
                        break;
                }
                break;
            }
            switch (*fmt) {
                case 'v':
                    v = va_arg(args, string_t);
                    char *vp = SpxString2Char1(v);
                    if (slen == (size_t) -1) {
                        while (*vp && buf < last) {
                            *buf++ = *vp++;
                        }
                    } else {
                        len = SpxMin(((size_t) (last - buf)), slen);
                        buf = SpxMemcpy(buf,vp,len);
                    }
                    fmt++;
                    continue;
                case 's':
                    p = va_arg(args, u_char *);
                    if (slen == (size_t) -1) {
                        while (*p && buf < last) {
                            *buf++ = *p++;
                        }
                    } else {
                        len = SpxMin(((size_t) (last - buf)), slen);
                        buf = SpxMemcpy(buf,p,len);
                    }
                    fmt++;
                    continue;
                case 'O':
                    i64 = (i64_t) va_arg(args, off_t);
                    sign = 1;
                    break;
                case 'P':
                    i64 = (i64_t) va_arg(args, pid_t);
                    sign = 1;
                    break;
                case 'T':
                    i64 = (i64_t) va_arg(args, time_t);
                    sign = 1;
                    break;
                case 'z':
                    if (sign) {
                        i64 = (i64_t) va_arg(args, ssize_t);
                    } else {
                        ui64 = (u64_t) va_arg(args, size_t);
                    }
                    break;
                case 'i':
                    if (sign) {
                        i64 = (i64_t) va_arg(args, int);
                    } else {
                        ui64 = (u64_t) va_arg(args, u_int);
                    }
                    if (max_width) {
                        width = SpxIntSize;
                    }
                    break;
                case 'd':
                    if (sign) {
                        i64 = (i64_t) va_arg(args, int);
                    } else {
                        ui64 = (u64_t) va_arg(args, u_int);
                    }
                    break;
                case 'l':
                    if (sign) {
                        i64 = (i64_t) va_arg(args, long);
                    } else {
                        ui64 = (u64_t) va_arg(args, u_long);
                    }
                    break;
                case 'D':
                    if (sign) {
                        i64 = (i64_t) va_arg(args, int32_t);
                    } else {
                        ui64 = (u64_t) va_arg(args, u_int32_t);
                    }
                    break;
                case 'L':
                    if (sign) {
                        i64 = va_arg(args, i64_t);
                    } else {
                        ui64 = va_arg(args, u64_t);
                    }
                    break;
                case 'A':
                    if (sign) {
                        i64 = (i64_t) va_arg(args, spx_atomic_t);
                    } else {
                        ui64 = (u64_t) va_arg(args, spx_atomic_t);
                    }
                    if (max_width) {
                        width = SpxAtomicSize;
                    }
                    break;
                case 'f':
                    f = va_arg(args, double);
                    if (f < 0) {
                        *buf++ = '-';
                        f = -f;
                    }
                    ui64 = (i64_t) f;
                    frac = 0;
                    if (frac_width) {
                        scale = 1;
                        for (n = frac_width; n; n--) {
                            scale *= 10;
                        }
                        frac = (u64_t) ((f - (double) ui64) * scale + 0.5);
                        if (frac == scale) {
                            ui64++;
                            frac = 0;
                        }
                    }
                    buf = spx_num_tostring(buf, last, ui64, zero, 0, width);
                    if (frac_width) {
                        if (buf < last) {
                            *buf++ = '.';
                        }
                        buf = spx_num_tostring(buf, last, frac, '0', 0, frac_width);
                    }
                    fmt++;
                    continue;
                case 'p':
                    ui64 = (u64_t) va_arg(args, void *);
                    hex = 2;
                    sign = 0;
                    zero = '0';
                    width = SpxPtrSize * 2;
                    break;
                case 'c':
                    d = va_arg(args, int);
                    *buf++ = (u_char) (d & 0xff);
                    fmt++;
                    continue;
                case 'Z':
                    *buf++ = '\0';
                    fmt++;
                    continue;

                case 'N':
#if (NGX_WIN32)
                    *buf++ = CR;
#endif
                    *buf++ = LF;
                    fmt++;
                    continue;
                case '%':
                    *buf++ = '%';
                    fmt++;
                    continue;
                default:
                    *buf++ = *fmt++;
                    continue;
            }
            if (sign) {
                if (i64 < 0) {
                    *buf++ = '-';
                    ui64 = (u64_t) -i64;
                } else {
                    ui64 = (u64_t) i64;
                }
            }
            buf = spx_num_tostring(buf, last, ui64, zero, hex, width);
            fmt++;
        } else {
            *buf++ = *fmt++;
        }
    }
    return buf;
}


spx_private u_char * spx_num_tostring(u_char *buf, \
        u_char *last, u64_t ui64, \
        u_char zero,u_int hexadecimal,\
        u_int width){
    u_char         *p, temp[SpxI64Size + 1];
    size_t          len;
    u_int32_t        ui32;
    static u_char   hex[] = "0123456789abcdef";
    static u_char   HEX[] = "0123456789ABCDEF";
    p = temp + SpxI64Size;
    if (hexadecimal == 0) {
        if (ui64 <= SpxI32Max) {
            ui32 = (u_int32_t) ui64;
            do {
                *--p = (u_char) (ui32 % 10 + '0');
            } while (ui32 /= 10);
        } else {
            do {
                *--p = (u_char) (ui64 % 10 + '0');
            } while (ui64 /= 10);
        }
    } else if (hexadecimal == 1) {
        do {
            *--p = hex[(u_int32_t) (ui64 & 0xf)];
        } while (ui64 >>= 4);
    } else {// hexadecimal == 2
        do {
            *--p = HEX[(u_int32_t) (ui64 & 0xf)];
        } while (ui64 >>= 4);
    }

    len = (temp + SpxI64Size) - p;
    while (len++ < width && buf < last) {
        *buf++ = zero;
    }
    len = (temp + SpxI64Size) - p;
    if (buf + len > last) {
        len = last - buf;
    }
    buf = SpxMemcpy(buf,p,len);
    return buf;
}


string_t  spx_snprintf(string_t buf,\
            size_t max, const string_t fmt, ...){
        string_t p;
        va_list   args;
        va_start(args, fmt);
        p = spx_vnprintf(buf, buf + max, fmt, args);
        va_end(args);
        return p;
    }

string_t spx_vsnprintf(string_t buf,\
        const size_t max,const string_t fmt,\
        va_list args){
    string_t p;
    p =spx_vnprintf(buf,buf + max, fmt, args);
    return p;
}


err_t spx_strcpy(string_t dest,string_t src,size_t len){
    if(NULL == dest) return EINVAL;
    if(SpxStringIsNullOrEmpty(src)) return 0;
    memcpy(SpxString2Char1(dest),SpxString2Char1(src),len);
    return 0;
}

string_t spx_numb_tostring(string_t buf,\
        size_t size,u64_t n){
    spx_num_tostring(buf,buf + size, n,0,0,0);
    return buf;
}
 */
/******************
 * sds lib begin
 * ****************/

string_t spx_string_newlen(const void *init, size_t initlen,err_t *err){/*{{{*/
    struct sds *sh;

    if (init) {
        sh = spx_alloc_alone(sizeof *sh+SpxStringRealSize(initlen),err);
    } else {
        sh = spx_alloc_alone(sizeof *sh+ SpxStringRealSize(initlen),err);
    }
    if (sh == NULL) return NULL;
    sh->len = initlen;
    sh->free = 0;
    if (initlen && init)
        memcpy(sh->buf, init, initlen);
    sh->buf[initlen] = '\0';
    return (string_t) sh->buf;
}/*}}}*/

string_t spx_string_new(const char *init,err_t *err){/*{{{*/
    size_t initlen = (init == NULL) ? 0 : strlen(init);
    return spx_string_newlen(init, initlen,err);
}/*}}}*/

string_t spx_string_empty(err_t *err){/*{{{*/
    return spx_string_newlen("",0,err);
}/*}}}*/

string_t spx_string_dup(const string_t s,err_t *err){/*{{{*/
    return spx_string_newlen(s, spx_string_len(s),err);
}/*}}}*/

void spx_string_free(string_t s){/*{{{*/
    if (s == NULL) return;
    struct sds *p = (struct sds *) (s-sizeof(struct sds));
    SpxFree(p);
}/*}}}*/

string_t spx_string_grow_zero(string_t s, size_t len,err_t *err){/*{{{*/
    struct sds *sh = (void*) (s-sizeof *sh);
    size_t totlen, curlen = sh->len;

    if (len <= curlen) return s;
    s = spxStringMakeRoomFor(s,len-curlen,err);
    if (s == NULL) return NULL;

    /* Make sure added region doesn't contain garbage */
    sh = (void*)(s-sizeof *sh);
    memset(s+curlen,0,(len-curlen+1)); /* also set trailing \0 byte */
    totlen = sh->len+sh->free;
    sh->len = len;
    sh->free = totlen-sh->len;
    return s;
}/*}}}*/

string_t spx_string_catlen(string_t s, const void *t, size_t len,err_t *err){/*{{{*/
    struct sds *sh;
    size_t curlen = spx_string_len(s);

    s = spxStringMakeRoomFor(s,len,err);
    if (s == NULL) return NULL;
    sh = (void*) (s-sizeof *sh);;
    memcpy(s+curlen, t, len);
    sh->len = curlen+len;
    sh->free = sh->free-len;
    s[curlen+len] = '\0';
    return s;
}/*}}}*/

string_t spx_string_cat(string_t s, const char *t,err_t *err){/*{{{*/
    return spx_string_catlen(s, t, strlen(t),err);
}/*}}}*/

string_t spx_string_cat_string(string_t s, const string_t t,err_t *err){/*{{{*/
    return spx_string_catlen(s, t, spx_string_len(t),err);
}/*}}}*/

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
    char buf[SpxI64Size], *p;
    unsigned long long v;

    v = (value < 0) ? -value : value;
    p = buf+(SpxI64Size - 1); /* point to the last character */
    do {
        *p-- = '0'+(v%10);
        v /= 10;
    } while(v);
    if (value < 0) *p-- = '-';
    p++;
    return spx_string_newlen(p,SpxI64Size-(p-buf),err);
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

/* Low level functions exposed to the user API */
string_t spxStringMakeRoomFor(string_t s, size_t addlen,err_t *err) {/*{{{*/
    struct sds *sh, *newsh;
    size_t free = spx_string_avail(s);
    size_t len, newlen;

    if (free >= addlen) return s;
    len = spx_string_len(s);
    sh = (void*) (s-sizeof *sh);;
    newlen = (len+addlen);
    if (newlen < MaxReallocSize)
        newlen *= 2;
    else
        newlen += MaxReallocSize;
    newsh = spx_realloc(sh, sizeof *newsh+SpxStringRealSize(newlen),err);
    if (newsh == NULL) return NULL;

    newsh->free = newlen - len;
    return newsh->buf;
}/*}}}*/

void spxStringIncrLen(string_t s, int incr){/*{{{*/
    struct sds *sh = (void*) (s-sizeof *sh);
//    assert(sh->free >= incr);
    sh->len += incr;
    sh->free -= incr;
//    assert(sh->free >= 0);
    s[sh->len] = '\0';
}/*}}}*/

string_t spxStringRemoveFreeSpace(string_t s,err_t *err){/*{{{*/
    struct sds *sh;
    sh = (void*) (s-sizeof *sh);;
    sh = spx_realloc(sh, sizeof *sh+SpxStringRealSize(sh->len),err);
    sh->free = 0;
    return sh->buf;
}/*}}}*/

size_t spxStringAllocSize(string_t s){/*{{{*/
    struct sds *sh = (void*) (s-sizeof *sh);;
    return sizeof(*sh)+sh->len+sh->free+1;
}/*}}}*/


