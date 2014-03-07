#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "headers/spx_types.h"
#include "headers/spx_mapf.h"
#include "headers/spx_string.h"
#include "headers/spx_errno.h"
#include "headers/spx_alloc.h"
#include "headers/spx_defs.h"
#include "headers/spx_path.h"

err_t spx_mapf_init(const bool_t isbry,const bool_t isstp,\
        const uchar_t stp,const size_t max,const string_t path,\
        const bool_t isshare,const int flags,\
        const string_t fname,const log_t log ,\
        struct spx_mapf **f){
    if(0 == max
            || SpxStringIsNullOrEmpty(path)
            || SpxStringIsNullOrEmpty(fname))
        return EINVAL;
    err_t rc = 0;
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
    return rc;
}
err_t spx_mapf_write(const struct spx_mapf *f,const uchar_t *buf,\
        const size_t size,size_t *len);
err_t spx_mapf_read(const struct spx_mapf *f,const off_t offset,\
        const size_t len,uchar_t **buf);
err_t spx_mapf_sync(const struct spx_mapf *f);
err_t spx_mapf_close(const struct spx_mapf *f);
