#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>


#include "headers/spx_alloc.h"
#include "headers/spx_errno.h"
#include "headers/spx_log.h"
#include "headers/spx_string.h"
#include "headers/spx_types.h"
#include "headers/spx_path.h"
#include "headers/spx_defs.h"
#include "headers/spx_time.h"

const char *SpxLogDesc[] = {
    "Debug.",
    "Info.",
    "Warn.",
    "Error.",
    "Mark."
};
const int SpxLogDescSize[] = {
    6,
    5,
    5,
    6,
    5
};


spx_private struct spx_log *g_log = NULL;

spx_private spx_inline void get_log_line(u8_t level,string_t line,size_t size,string_t fmt,va_list ap);
spx_private err_t logf_create(SpxLogDelegate log,\
        const string_t path,\
        const string_t name,u64_t max_size,\
        int *fd,void **p);
spx_private spx_inline void logf_close();


err_t spx_log_new(SpxLogDelegate log,\
        const string_t path,\
        const string_t name,const u64_t max_size,\
        const u8_t level){/*{{{*/
    if(SpxStringIsNullOrEmpty(path)
            || SpxStringIsNullOrEmpty(name)){
        return EINVAL;
    }
    err_t err = 0;
    u64_t size = 0 == max_size ? 10 * SpxMB : max_size;
    if(0!= (err = spx_alloc_alone(sizeof(struct spx_log),(void **) &g_log))){
        return err;
    }
    g_log->size =size;
    g_log->level = level;
    g_log->log = log;
    SpxStringCpy1(g_log->path,path,SpxMin(SpxStringLength(path), SpxPathSize));
    SpxStringCpy1(g_log->name,name,SpxMin(SpxStringLength(name), SpxFileNameSize));
    if(0 != (err = logf_create(log,g_log->path,g_log->name,\
                    g_log->size,&(g_log->fd),&(g_log->ptr)))){
        goto r1;
    }

    return err;
r1:
    if(NULL != g_log){
        SpxFree(g_log);
    }
    return err;
}/*}}}*/

void spx_log(int level,string_t fmt,...){/*{{{*/
    if((level < 0 || (int)sizeof(SpxLogDesc) <= level) || NULL == fmt) return;

    va_list ap;
    va_start(ap,fmt);
    SpxString(line,SpxStringRealSize(SpxLineSize));
    get_log_line(level,line,SpxLineSize,fmt,ap);
    va_end(ap);

//    if(NULL == g_log || 0 == g_log->fd ){
    if(NULL == g_log || 0 == g_log->fd || NULL == g_log->ptr){
        fprintf(stdout,"%s",SpxString2Char2(line));
        return;
    }

    if(level < g_log->level){
        return;
    }

    size_t s = SpxStringLength(line);
    if(g_log->offset + s > g_log->size){
        logf_close(g_log);
        logf_create(g_log->log,g_log->path,g_log->name\
                ,g_log->size,&(g_log->fd),&(g_log->ptr));
    }
    fprintf(stdout,"%s",SpxString2Char2(line));
    memcpy(((char *) g_log->ptr) + g_log->offset,line,s);
    //write(g_log->fd,line,s);
    g_log->offset += s;
}/*}}}*/

void spx_log_destory(){
    logf_close();
    SpxFree(g_log);
}

spx_private err_t logf_create(SpxLogDelegate log,\
        const string_t path,\
        const string_t name,u64_t max_size,\
        int *fd,void **p){/*{{{*/
    if(SpxStringIsNullOrEmpty(path)
            || SpxStringIsNullOrEmpty(name)){
        return EINVAL;
    }

    err_t err = 0;
    bool_t is_dir = false;
    if(0 != (err = spx_is_dir(path,&is_dir))){
        if(ENOENT != err){
            return err;
        }
        SpxErrReset;
    }
    if(!is_dir){
        if(0 != (err = spx_mkdir(log,path,SpxPathMode))){
            return err;
        }
    }
    SpxString(fp,SpxStringRealSize(SpxPathSize));
    struct spx_datetime dt;
    SpxZero(dt);
    spx_get_curr_datetime(&dt);
    SpxSnprintf(fp,SpxPathSize,"%s%s%s%-4d%02d%02d-%02d%02d%02d.log",\
            SpxString2Char1(path),\
            SpxStringEndWith(path,SpxPathDlmt) ? "" : SpxPathDlmtString,\
            name,\
            SpxYear(&dt),SpxMonth(&dt),SpxDay(&dt),\
            SpxHour(&dt),SpxMinute(&dt),SpxSecond(&dt));

    *fd = open(SpxString2Char2(fp),O_RDWR|O_APPEND|O_CREAT);
    if(0 == *fd) {
        err = 0 == errno ? EACCES : errno;
        return err;
    }
    if(0 != (err = ftruncate(*fd,max_size))){
        err = 0 == errno ? EACCES : errno;
        goto r1;
    }
    *p = mmap(NULL,max_size,PROT_READ | PROT_WRITE , MAP_PRIVATE,*fd,0);
    if(MAP_FAILED == *p){
        err = errno;
        goto r1;
    }
    return err;

r1:
    if(0 != *fd){
        close(*fd);
        *fd = 0;
    }
    return err;
}/*}}}*/

spx_private spx_inline void get_log_line(u8_t level,string_t line,size_t size,string_t fmt,va_list ap){/*{{{*/
    SpxStringCpy1(line,SpxLogDesc[level],SpxLogDescSize[level]);
    SpxVSnprintf2(line,size,SpxLogDescSize[level],fmt,ap);
    size_t l = SpxStringLength(line);
    size_t s = SpxMin(l,SpxLineSize);
    SpxStringCpy2(line,SpxLineEndDlmtString,s,sizeof(SpxLineEndDlmtString));
}/*}}}*/

spx_private spx_inline void logf_close(){
    if(NULL != g_log->ptr){
        fprintf(stdout,"%s",(char *) g_log->ptr);
        msync(g_log->ptr,g_log->offset,MS_SYNC);
        munmap(g_log->ptr,g_log->size);
        g_log->ptr = NULL;
    }
    if(0 != g_log->fd){
        close(g_log->fd);
    }
    g_log->offset = 0;
}
