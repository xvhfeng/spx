#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "headers/spx_types.h"
#include "headers/spx_mapf.h"
#include "headers/spx_string.h"
#include "headers/spx_errno.h"
#include "headers/spx_alloc.h"
#include "headers/spx_defs.h"
#include "headers/spx_path.h"

err_t spx_mapf_writer_init(const log_t log,const string_t filename,\
        const size_t max,const bool_t isshare,\
        const bool_t isbry,const bool_t isstp,\
        const uchar_t stp,const bool_t isappend,\
        const off_t offset,struct spx_mapf **f){/*{{{*/
    if(0 == max || max < ((size_t) offset) \
            || SpxStringIsNullOrEmpty(filename))
        return EINVAL;

    err_t rc = 0;
    SpxString(path,SpxPathSize);
    spx_basepath(filename,path);
    if(0 != (rc = spx_mkdir(log,path,SpxPathMode))){
        if(NULL != log){
            log(SpxLogError,SpxLogInfo3("create dir:%s is fail.",path,rc));
        }
        return rc;
    }
    if(0 != (rc = spx_alloc_alone(sizeof(struct spx_mapf),(void **)f))){
        if(NULL != log) {
            log(SpxLogError,SpxLogInfo2("alloc map-file is fail.",rc));
        }
        return rc;
    }
    (*f)->isbry = isbry;
    (*f)->is_stp = isstp;
    (*f)->stp = stp;
    (*f)->log = log;
    (*f)->max = max;
    (*f)->isappend = isappend;
    (*f)->offset = offset;
    int flags = O_WRONLY | O_CREAT | isappend ? O_APPEND : O_TRUNC;
    SpxErrReset;
    (*f)->fd = open(SpxString2Char2(filename),flags,SpxFileMode);
    if(SpxErr2(rc)){
        if(NULL != log) {
            log(SpxLogError,SpxLogInfo3("open the file:%s is fail.",filename,rc));
        }
        return rc;
    }
    SpxErrReset;
    (*f)->start = mmap(NULL,max,PROT_READ | PROT_WRITE,\
            isshare ? MAP_SHARED : MAP_PRIVATE,(*f)->fd,0);
    if(MAP_FAILED == (*f)->start){
        rc = errno;
        if(NULL != log){
            log(SpxLogError,SpxLogInfo3("mmap the file:%s is fail.",filename,rc));
        }
        goto r1;
    }
    (*f)->ptr = ((uchar_t *) (*f)->start) + offset;
    return rc;
r1:
    if(0 != (*f)->fd)
        close((*f)->fd);
    return rc;
}/*}}}*/

err_t spx_mapf_write(struct spx_mapf *f,const byte_t *buf,\
        const size_t size){
    f->ptr = SpxMemcpy(f->ptr,(uchar_t *) buf,size);
    return 0;
}
err_t spx_mapf_write_string(const struct spx_mapf *f,const string_t buf,\
        const size_t size,size_t len){

}

err_t spx_mapf_write_wstring(struct spx_mapf,const wstring_t buf,\
        const size_t size,size_t len){

}
err_t spx_mapf_read(const struct spx_mapf *f,const off_t offset,\
        const size_t len,uchar_t **buf);
err_t spx_mapf_sync(const struct spx_mapf *f);
err_t spx_mapf_close(const struct spx_mapf *f);
