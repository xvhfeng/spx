
#ifndef SPX_LOG_H
#define SPX_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "spx_types.h"
#include "spx_string.h"
#include "spx_defs.h"

    //the macro to string,it will use to debug
#define spx_m2s_d(m) #m
#define spx_m2s(m) spx_m2s_d(m)

    struct spx_log{
        int fd;//the file desc
        off_t offset;
        void *ptr;//the mmap ptr
        size_t size;
//        size_t csize;//current size of context
        u8_t level;
        uchar_t path[SpxStringRealSize(SpxPathSize)];
        uchar_t name[SpxStringRealSize(SpxFileNameSize)];
        SpxLogDelegate *log;
    };

    err_t spx_log_new(SpxLogDelegate log,\
            const string_t path,\
            const string_t name,const u64_t max_size,\
            const u8_t level);
    void spx_log(int level,string_t fmt,...);
    void spx_log_destory();

#ifdef __cplusplus
}
#endif
#endif
