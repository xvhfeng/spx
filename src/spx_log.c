#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>


#include "spx_alloc.h"
#include "spx_errno.h"
#include "spx_log.h"
#include "spx_string.h"
#include "spx_types.h"
#include "spx_path.h"
#include "spx_defs.h"
#include "spx_time.h"



spx_private struct spx_log *g_log = NULL;

spx_private spx_inline string_t get_log_line(err_t *err,\
        u8_t level,string_t fmt,va_list ap);
spx_private FILE *logf_create(SpxLogDelegate log,\
        const string_t path,\
        const string_t name,u64_t max_size,\
        err_t *err);
//        int *fd,void **p);
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
    g_log = spx_alloc_alone(sizeof(*g_log),&err);
    if(NULL == g_log){
        return err;
    }
    g_log->size =size;
    g_log->level = level;
    g_log->log = log;
    g_log->path = spx_string_dup(path,&err);
    if(NULL == g_log->path){
        goto r1;
    }
    g_log->name = spx_string_dup(name,&err);
    if(NULL == g_log->name){
        goto r1;
    }

    g_log->fp = logf_create(log,g_log->path,g_log->name,
            g_log->size,&err);
//    if(0 != (err = logf_create(log,g_log->path,g_log->name,
//                    g_log->size,&(g_log->fd),&(g_log->ptr)))){
//        goto r1;
//    }

    return err;
r1:
    if(NULL != g_log->path)
        spx_string_free(g_log->path);
    if(NULL != g_log->name)
        spx_string_free(g_log->name);
    SpxFree(g_log);
    return err;
}/*}}}*/

void spx_log(int level,char *fmt,...){/*{{{*/
    if((level < 0 || (int)sizeof(SpxLogDesc) < level) || NULL == fmt) return;

    err_t err = 0;
    va_list ap;
    va_start(ap,fmt);
    string_t line = get_log_line(&err,level,fmt,ap);
    va_end(ap);
    if(NULL == line){
        return;
    }
    string_t newline = spx_string_cat(line,SpxLineEndDlmtString,&err);
    if(NULL == newline){
        spx_string_free(line);
        return;
    }

    if(NULL == g_log || 0 == g_log->fp ){
        fprintf(stdout,"%s",SpxString2Char2(newline));
        spx_string_free(newline);
        return;
    }

    if(level < g_log->level){
        spx_string_free(newline);
        return;
    }

    size_t s = spx_string_len(newline);
    if(g_log->offset + s > g_log->size){
        logf_close(g_log);
        g_log->fp = logf_create(g_log->log,g_log->path,g_log->name\
                ,g_log->size,&err);
        if(NULL == g_log->fp){
            return;
        }
    }
//    fprintf(stdout,"%s",SpxString2Char2(newline));
//    memcpy(((char *) g_log->ptr) + g_log->offset,newline,s);
    //write(g_log->fd,line,s);
    fwrite(newline,s,sizeof(char),g_log->fp);
    g_log->offset += s;
    spx_string_free(newline);
    return;
}/*}}}*/

void spx_log_free(){
    logf_close();
    if(NULL != g_log->path)
        spx_string_free(g_log->path);
    if(NULL != g_log->name)
        spx_string_free(g_log->name);
    SpxFree(g_log);
}

spx_private FILE *logf_create(SpxLogDelegate log,\
        const string_t path,\
        const string_t name,u64_t max_size,\
        err_t *err){/*{{{*/
    if(SpxStringIsNullOrEmpty(path)
            || SpxStringIsNullOrEmpty(name)){
        *err =  EINVAL;
        return NULL;
    }
    string_t fp = NULL;
    string_t newfp = NULL;
    bool_t is_dir = false;
    FILE *f = NULL;
    is_dir = spx_is_dir(path,err);
    if(0 != *err){
        return NULL;
    }
    if(!is_dir){
        if(0 != (*err = spx_mkdir(log,path,SpxPathMode))){
            return NULL;
        }
    }
    fp = spx_string_empty(err);
    if(NULL == fp){
        return NULL;
    }
    struct spx_datetime dt;
    SpxZero(dt);
    spx_get_curr_datetime(&dt);
    newfp = spx_string_cat_printf(err,fp,\
            "%s%s%s%-4d%02d%02d-%02d%02d%02d.log",\
            SpxString2Char1(path),\
            SpxStringEndWith(path,SpxPathDlmt) ? "" : SpxPathDlmtString,\
            name,\
            SpxYear(&dt),SpxMonth(&dt),SpxDay(&dt),\
            SpxHour(&dt),SpxMinute(&dt),SpxSecond(&dt));
    if(NULL == newfp){
        goto r1;
    }
    fp = newfp;
//    *fd = open(SpxString2Char2(fp),O_RDWR|O_APPEND|O_CREAT,SpxFileMode);
//    if(0 >= *fd) {
    f = fopen(SpxString2Char2(fp),"a+");
    if(NULL == f){
        *err = 0 == errno ? EACCES : errno;
        goto r1;
    }
    setlinebuf(f);
//    if(0 != (err = ftruncate(*fd,max_size))){
//        goto r1;
//    }
//    *p = mmap(NULL,max_size,PROT_READ | PROT_WRITE , MAP_SHARED,*fd,0);
//    if(MAP_FAILED == *p){
//        err = errno;
//        goto r1;
//    }
    if(NULL != newfp){
        spx_string_free(newfp);
    } else {
        if(NULL != fp){
            spx_string_free(fp);
        }
    }
    return f;

r1:
    if(NULL != newfp){
        spx_string_free(newfp);
    } else {
        if(NULL != fp){
            spx_string_free(fp);
        }
    }
    if(NULL != f){
        fclose(f);
    }
    return NULL;
//    if(0 != *fd){
//        close(*fd);
//        *fd = 0;
//    }
//    return err;
}/*}}}*/

spx_private spx_inline string_t get_log_line(err_t *err,\
        u8_t level,char *fmt,va_list ap){/*{{{*/
    string_t line = spx_string_newlen(SpxLogDesc[level],SpxLogDescSize[level],err);
    if(NULL == line){
        return NULL;
    }

    struct spx_datetime dt;
    SpxZero(dt);
    spx_get_curr_datetime(&dt);

    string_t newline = spx_string_cat_printf(err,line,"%04d-%02d-%2d %02d:%02d:%02d.",
            dt.d.year,dt.d.month,dt.d.day,dt.t.hour,dt.t.min,dt.t.sec);
    if(NULL == newline){
        spx_string_free(line);
        return NULL;
    }
    line = newline;
    newline = spx_string_cat_vprintf(err,line,fmt,ap);
    if(NULL == newline){
        spx_string_free(line);
        return NULL;
    }
    line = newline;
    return line;
}/*}}}*/

spx_private spx_inline void logf_close(){
    if(NULL != g_log->fp){
        fflush(g_log->fp);
        fclose(g_log->fp);
    }
//    if(NULL != g_log->ptr){
//        fprintf(stdout,"%s",(char *) g_log->ptr);
//        msync(g_log->ptr,g_log->offset,MS_SYNC);
//        munmap(g_log->ptr,g_log->size);
//        g_log->ptr = NULL;
//    }
//    if(0 != g_log->fd){
//        close(g_log->fd);
//    }
    g_log->offset = 0;
}
