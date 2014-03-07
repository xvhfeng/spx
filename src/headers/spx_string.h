
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
#define SpxZero(v) memset(v,0,sizeof(*(v)))
#define SpxBinary2String(s) ((string_t) (s + sizeof(size_t)))
#define SpxString2Char1(s) ((char *)(s))
#define SpxString2Char2(s) ((const char *)(s))
#define SpxChar2String1(s) ((string_t) s)
#define SpxChar2String2(s) ((const string_t) s)
#define SpxStringLength(s) (strlen(SpxString2Char2(s)))
#define SpxStringIsNullOrEmpty(s) (NULL == s || 0 == strlen(SpxString2Char2(s)))
#define SpxBinaryIsNullOrEmpty(s) (NULL == s || NULL == s->v || 0 == s->s)
#define SpxStringEndWith(s,c) (c == *(s + SpxStringLength(s)))
#define SpxStringBeginWith(s,c) ( c == *s)
#define SpxStringCpy1(d,s,l) memcpy(SpxString2Char1(d),SpxString2Char1(s),l)
#define SpxStringCpy2(d,offset,s,l) memcpy(SpxString2Char1(d) + offset,SpxString2Char1(s),l)
#define SpxStringAllCpy1(d,s) memcpy(SpxString2Char1(d),SpxString2Char1(s),SpxStringLength(s))
#define SpxStringAllCpy2(d,offset,s) (SpxString2Char2(d) + offset,SpxString2Char1(s),SpxStringLength(s))
#define SpxStringAllCpy3(d,s) ((string_t) memcpy(SpxString2Char1(d),SpxString2Char1(s),SpxStringLength(s)) + SpxStringLength(s))
#define SpxSnprintf(dest,size,fmt,...) snprintf(SpxString2Char1(dest),size,fmt,__VA_ARGS__)




    err_t spx_string_split(const log_t log,const string_t s,\
            const string_t d,const bool_t isrs,struct spx_vector **dest);

    spx_inline err_t spx_strcpy(string_t dest,string_t src,size_t len){
        if(NULL == dest) return EINVAL;
        if(SpxStringIsNullOrEmpty(src)) return 0;
        memcpy(SpxString2Char1(dest),SpxString2Char1(src),len);
        return 0;
    }

#ifdef __cplusplus
}
#endif
#endif
