#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <sys/vfs.h>
#include <sys/sendfile.h>


#include "spx_types.h"
#include "spx_path.h"
#include "spx_defs.h"
#include "spx_string.h"
#include "spx_errno.h"

err_t spx_is_dir(const string_t path,bool_t *isdir) {/*{{{*/
    SpxErrReset;
	struct stat buf;
	if (-1 == stat(SpxString2Char2(path), &buf)) {
	    return 0 == errno ? EACCES : errno;
	}
	*isdir = S_ISDIR(buf.st_mode);
	return 0;
}/*}}}*/

err_t spx_mkdir(const log_t log,const string_t path,const mode_t mode){/*{{{*/
    err_t rc = 0;
    if(SpxStringIsNullOrEmpty(path)) {
        return EINVAL;
    }
    char ptr[SpxPathSize + 1] = {0};
    memcpy(ptr,SpxString2Char2(path),SpxStringLength(path));
    bool_t isdir = false;
    if(0 != (rc = spx_is_dir(path,&isdir))){
        if(NULL != log) {
            log(SpxLogError,SpxLogInfo3("check dir:%s is exist is fail.",path,rc));
        }
        return rc;
    }
    if(isdir) return rc;
    char *p = ptr;
    while(NULL != (p = strchr(p,SpxPathDlmt))){
        *p = 0;
        bool_t isdir = false;
        if(0 != (rc = spx_is_dir(SpxChar2String2(ptr),&isdir))){
            if(NULL != log) {
                log(SpxLogError,SpxLogInfo3("check dir:%s is fail.",ptr,rc));
            }
            break;
        }
        if(!isdir && (0 != (rc = mkdir(ptr,mode)))){
            if(NULL != log){
                log(SpxLogError,SpxLogInfo3("create dir is fail,dir:%s.",ptr,rc));
            }
            break;
        }
        *p = SpxPathDlmt;
        p += sizeof(SpxPathDlmt);
    }
    return rc;
}/*}}}*/

err_t spx_fullname(const string_t path,const string_t filename,\
        string_t fullname){/*{{{*/
    if(SpxStringIsNullOrEmpty(path)
            || SpxStringIsNullOrEmpty(filename)){
        return EINVAL;
    }
    if(SpxStringEndWith(path,SpxPathDlmt)){
        SpxSnprintf(fullname,SpxPathSize,"%s%s",path,filename);
    } else {
        SpxSnprintf(fullname,SpxPathSize,"%s%c%s",path,SpxPathDlmt,filename);
    }
    return 0;
}/*}}}*/

err_t spx_basepath(const string_t path,string_t basepath){/*{{{*/
    if(SpxStringIsNullOrEmpty(path)){
        return EINVAL;
    }
    const char *tmp = SpxString2Char2(path);
    ptr_t p = NULL;
    if(NULL != (p =(ptr_t) rindex(tmp,(int) SpxPathDlmt))){
        ptrdiff_t pd = p - tmp;
        memcpy(basepath,path,pd + sizeof(SpxPathDlmt));//add the path dlmt
        return 0;
    }
    return ENODLMT;
}/*}}}*/
