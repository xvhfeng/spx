#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
//#include <sys/vfs.h>
//#include <sys/sendfile.h>


#include "include/spx_types.h"
#include "include/spx_path.h"
#include "include/spx_defs.h"
#include "include/spx_string.h"
#include "include/spx_errno.h"

bool_t spx_is_dir(const string_t path,err_t *err) {/*{{{*/
    SpxErrReset;
    struct stat buf;
    if (-1 == stat(SpxString2Char2(path), &buf)) {
        if(ENOENT == errno){
            SpxErrReset;
            return false;
        }
        *err = 0 == errno ? EACCES : errno;
        return false;
    }
    return S_ISDIR(buf.st_mode);
}/*}}}*/

err_t spx_mkdir(SpxLogDelegate *log,const string_t path,const mode_t mode){/*{{{*/
    err_t rc = 0;
    if(SpxStringIsNullOrEmpty(path)) {
        return EINVAL;
    }
    char ptr[SpxPathSize + 1] = {0};
    memcpy(ptr,SpxString2Char2(path),spx_string_len(path));
    bool_t isdir = false;
    isdir = spx_is_dir(path,&rc);
    if(0 != rc) {
        SpxLogFmt2(log,SpxLogError,rc,\
                "check dir:%s is exist is fail.",path);
        return rc;
    }
    if(isdir) return rc;
    char *p = ptr;
    while(NULL != (p = strchr(p,SpxPathDlmt))){
        *p = 0;
        isdir = spx_is_dir(ptr,&rc);
        if(0 != rc) {
            SpxLogFmt2(log,SpxLogError,rc,\
                    "check dir:%s is fail.",ptr);
            break;
        }
        if(!isdir && (0 != (rc = mkdir(ptr,mode)))){
            SpxLogFmt2(log,SpxLogError,rc,\
                    "create dir:%s is fail.",ptr);
            break;
        }
        *p = SpxPathDlmt;
        p += sizeof(SpxPathDlmt);
    }
    isdir = spx_is_dir(ptr,&rc);
    if(0 != rc) {
        SpxLogFmt2(log,SpxLogError,rc,\
                "check dir:%s is fail.",ptr);
        return rc;
    }
    if(!isdir && (0 != (rc = mkdir(ptr,mode)))){
        SpxLogFmt2(log,SpxLogError,rc,\
                "create dir:%s is fail.",ptr);
        return rc;
    }
    return rc;
}/*}}}*/


string_t spx_fullname(const string_t path,const string_t filename,\
        err_t *err){/*{{{*/
    if(SpxStringIsNullOrEmpty(path)
            || SpxStringIsNullOrEmpty(filename)){
        *err = EINVAL;
        return NULL;
    }
    string_t fn = spx_string_empty(err);
    string_t newfn = NULL;
    if(NULL == fn) return NULL;
    if(SpxStringEndWith(path,SpxPathDlmt)){
       newfn = spx_string_cat_printf(err,fn,"%s%s",path,filename);
    } else {
       newfn = spx_string_cat_printf(err,fn,"%s%c%s",path,SpxPathDlmt,filename);
    }
    if(NULL == newfn){
        spx_string_free(fn);
    }
    return newfn;
}/*}}}*/

string_t spx_basepath(const string_t path,err_t *err){/*{{{*/
    if(SpxStringIsNullOrEmpty(path)){
        *err = EINVAL;
        return NULL;
    }
    const char *tmp = SpxString2Char2(path);
    string_t newbp = NULL;
    ptr_t p = NULL;
    if(NULL != (p =(ptr_t) rindex(tmp,(int) SpxPathDlmt))){
        ptrdiff_t pd = p - tmp;
        newbp = spx_string_newlen(path,pd,err);
    }
    return newbp;
}/*}}}*/
