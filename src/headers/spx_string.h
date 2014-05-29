
#ifndef SPX_STRING_H
#define SPX_STRING_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include "spx_types.h"
#include "spx_vector.h"


#define SpxMemcpy(d,s,l) (((uchar_t *) memcpy(d,s,l)) + l)

#define SpxZeroPtr(v) memset(v,0,sizeof(*(v)))
#define SpxZero(v) memset(&(v),0,sizeof(v))

#define SpxBinary2String(s) ((string_t) (s + sizeof(size_t)))
#define SpxString2Char1(s) ((char *)(s))
#define SpxString2Char2(s) ((const char *)(s))
#define SpxChar2String1(s) ((string_t) s)
#define SpxChar2String2(s) ((const string_t) s)

#define SpxStringLength(s) (strlen(SpxString2Char2(s)))

#define SpxStringIsNullOrEmpty(s) (NULL == s || '\0' == *s)
#define SpxBinaryIsNullOrEmpty(s) (NULL == s || NULL == s->v || 0 == s->s)
#define SpxStringIsEmpty(s) ('\0' == *s)

#define SpxStringEndWith(s,c) (c == *(s + SpxStringLength(s)))
#define SpxStringBeginWith(s,c) ( c == *s)

#define SpxStringCpy1(d,s,l) memcpy(SpxString2Char1(d),SpxString2Char1(s),l)
#define SpxStringCpy2(d,s, offset,l) \
    memcpy(SpxString2Char1(d) + offset,SpxString2Char1(s),l)

#define SpxStringAllCpy1(d,s) memcpy(SpxString2Char1(d),SpxString2Char1(s),SpxStringLength(s))
#define SpxStringAllCpy2(d,offset,s) memcpy(SpxString2Char2(d) + offset,SpxString2Char1(s),SpxStringLength(s))
#define SpxStringAllCpy3(d,s) ((string_t) memcpy(SpxString2Char1(d),SpxString2Char1(s),SpxStringLength(s)) + SpxStringLength(s))

#define SpxSnprintf(dest,size,fmt,...) snprintf(SpxString2Char1(dest),size,fmt,__VA_ARGS__)
#define SpxVSnprintf(dest,size,fmt,ap) vsnprintf(SpxString2Char1(dest),size,fmt,ap)
#define SpxVSnprintf2(dest,size,offset,fmt,ap) \
    vsnprintf((SpxString2Char1(dest) + offset),\
            (size - offset),SpxString2Char2(fmt),ap)

#define SpxString(buf,len)\
    char _spx_string_##buf[len] = {0}; \
    string_t buf = SpxChar2String1(_spx_string_##buf)
#define SpxStringRealSize(l) ((l) + 1)

#define SpxStringCmp(s1,s2,l) memcmp(SpxString2Char2(s1),\
                                    SpxString2Char2(s2),l)




    err_t spx_string_split(SpxLogDelegate *log,const string_t s,\
            const string_t d,const bool_t isrs,struct spx_vector **dest);

    spx_inline err_t spx_strcpy(string_t dest,string_t src,size_t len);

    string_t  spx_snprintf(string_t buf,\
            size_t max, const string_t fmt, ...);

    string_t spx_vsnprintf(string_t buf,\
            const size_t max,const string_t fmt,\
            va_list args);
string_t spx_numb_tostring(string_t buf,\
        size_t size,u64_t n);
#ifdef __cplusplus
}
#endif
#endif
