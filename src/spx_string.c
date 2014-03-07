#include <stdlib.h>
#include <string.h>

#include "headers/spx_types.h"
#include "headers/spx_alloc.h"
#include "headers/spx_string.h"
#include "headers/spx_errno.h"
#include "headers/spx_defs.h"

spx_private spx_inline err_t spx_split_string_free(void **v);
spx_private spx_inline err_t spx_split_string_add(struct spx_vector *dest,char *p,size_t len);

err_t spx_string_split(const log_t log,const string_t s,\
        const string_t d,const bool_t isrs,struct spx_vector **dest){/*{{{*/
    err_t rc = 0;
    if(spx_string_is_null_or_empty(s)){
        return EINVAL;
    }
    if(spx_string_is_null_or_empty(d)){
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
                    if(NULL != log){
                        log(SpxLogError,SpxLogInfo2("add split string to vector is fail.",rc));
                    }
                    goto r1;
                }
            }
        }
        if(0 != (rc = spx_split_string_add(*dest,p,diff))){
            if(NULL != log){
                log(SpxLogError,SpxLogInfo2("add split string to vector is fail.",rc));
            }
            goto r1;
        }
        p += diff + ds;
    }
    size_t last = (size_t) (pe - p);//the last
    if(0 != last){
        if(0 != (rc = spx_split_string_add(*dest,p,last))){
            if(NULL != log){
                log(SpxLogError,SpxLogInfo2("add split string to vector is fail.",rc));
            }
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
        if(NULL != dest->log){
            dest->log(SpxLogError,SpxLogInfo2("alloc split string is fail.",rc));
        }
        return rc;
    }
    if((0 != len) && (0 != (rc = spx_strcpy(sp,SpxChar2String1(p),len)))){
        if(NULL != dest->log){
            dest->log(SpxLogError,SpxLogInfo2("copy string to split string is fail.",rc));
        }
        spx_free(sp);
        return rc;
    }
    if(0 != (rc = spx_vector_add(dest,sp))){
        if(NULL != dest->log){
            dest->log(SpxLogError,SpxLogInfo2("add split string to vector is fail.",rc));
        }
        spx_free(sp);
        return rc;
    }
    return rc;
}/*}}}*/

spx_private spx_inline err_t spx_split_string_free(void **v){/*{{{*/
    if(NULL != *v){
        spx_free(*v);
    }
    return 0;
}/*}}}*/
