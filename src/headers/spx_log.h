
#ifndef SPX_LOG_H
#define SPX_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "spx_types.h"

//the macro to string,it will use to debug
#define spx_m2s_d(m) #m
#define spx_m2s(m) spx_m2s_d(m)

    struct spx_log{
        int fd;//the file desc
        off_t offset;
        void *ptr;//the mmap ptr
        size_t size;
        size_t csize;//current size of context
    };

#ifdef __cplusplus
}
#endif
#endif
