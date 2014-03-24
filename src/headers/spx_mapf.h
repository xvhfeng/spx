/*
 * the file operation support 2 ways.
 * 1:mmap
 * 2:stdio write
 */
#ifndef SPX_MAPF_H
#define SPX_MAPF_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <pthread.h>

#include "spx_types.h"

//must to define for io-functions in 32 bit system to 64 bit
#define _FILE_OFFSET_BITS 64 //must to define for io-functions in 32 bit system to 64 bit
#define SpxMapReadOnly 0
#define SpxMapWriteOnly 1
#define SpxMapWR 2

    struct spx_mapf {
        int fd;
        bool_t isbry;//whether to write into file use binary
        bool_t is_stp;//whether to use the line separator
        bool_t isshare;
        bool_t isappend;
        int flags;//the operator flags
        size_t max;//the file max size
        off_t offset;
        void *start;//the mmap ptr
        uchar_t *ptr;
        uchar_t stp;
        log_t log;
    };

err_t spx_mapf_writer_init(const log_t log,const string_t filename,\
        const size_t max,const bool_t isshare,\
        const bool_t isbry,const bool_t isstp,\
        const uchar_t stp,const bool_t isappend,\
        const off_t offset,struct spx_mapf **f);
    err_t spx_mapf_write(const struct spx_mapf *f,const uchar_t *buf,\
            const size_t size);
    err_t spx_mapf_read(const struct spx_mapf *f,const off_t offset,\
            const size_t len,uchar_t **buf);
    err_t spx_mapf_sync(const struct spx_mapf *f);
    err_t spx_mapf_close(const struct spx_mapf *f);

#ifdef __cplusplus
}
#endif
#endif
