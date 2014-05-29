#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "headers/spx_types.h"
#include "headers/spx_alloc.h"
#include "headers/spx_string.h"
#include "headers/spx_errno.h"
#include "headers/spx_defs.h"

spx_private spx_inline err_t spx_split_string_free(void **v);
spx_private spx_inline err_t spx_split_string_add(struct spx_vector *dest,char *p,size_t len);

spx_private uchar_t *spx_vnprintf(uchar_t *buf,uchar_t *last, \
        uchar_t *fmt,va_list args);
spx_private u_char * spx_num_tostring(u_char *buf, \
        u_char *last, u64_t ui64, \
        u_char zero,u_int hexadecimal,\
        u_int width);



err_t spx_string_split(SpxLogDelegate *log,const string_t s,\
        const string_t d,const bool_t isrs,struct spx_vector **dest){/*{{{*/
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
}/*}}}*/

spx_private spx_inline err_t spx_split_string_add(struct spx_vector *dest,char *p,size_t len){/*{{{*/
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
}/*}}}*/

spx_private spx_inline err_t spx_split_string_free(void **v){/*{{{*/
    if(NULL != *v){
        SpxFree(*v);
    }
    return 0;
}/*}}}*/

spx_private uchar_t *spx_vnprintf(uchar_t *buf,uchar_t *last, \
        uchar_t *fmt,va_list args){/*{{{*/
    u_char *p, zero;
    int d;
    double f;
    size_t len, slen;
    i64_t i64;
    u64_t ui64, frac;
    u_int width, sign, hex, max_width, frac_width, scale, n;

    string_t v;

    while (*fmt && buf < last) {

        /*
         * "buf < last" means that we could copy at least one character:
         * the plain character, "%%", "%c", and minus without the checking
         */
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
}/*}}}*/


spx_private u_char * spx_num_tostring(u_char *buf, \
        u_char *last, u64_t ui64, \
        u_char zero,u_int hexadecimal,\
        u_int width){/*{{{*/
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
    } else { /* hexadecimal == 2 */
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
}/*}}}*/


string_t  spx_snprintf(string_t buf,\
        size_t max, const string_t fmt, ...){/*{{{*/
    string_t p;
    va_list   args;
    va_start(args, fmt);
    p = spx_vnprintf(buf, buf + max, fmt, args);
    va_end(args);
    return p;
}/*}}}*/

string_t spx_vsnprintf(string_t buf,\
        const size_t max,const string_t fmt,\
        va_list args){/*{{{*/
    string_t p;
    p =spx_vnprintf(buf,buf + max, fmt, args);
    return p;
}/*}}}*/


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

