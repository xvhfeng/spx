
#ifndef SPX_DEFS_H
#define SPX_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <unistd.h>

#include "spx_types.h"
#include "spx_errno.h"

#define SpxLogDebug 0
#define SpxLogInfo 1
#define SpxLogWarn 2
#define SpxLogError 3
#define SpxLogMark 4


#define SpxStackFmt "File:%s,line:%d,func:%s."
#define SpxStackInfo __FILE__,__LINE__,__FUNCTION__
#define SpxErrFmt "errno:%d,err info:%s."
#define SpxErrInfo(err) err,spx_strerror(err)

    /*
#define SpxLogInfo1(info) ((string_t) "File:%s,Line:%s,Func:%s.%s"),\
    __FILE__,__LINE__,__FUNCTION__,info
#define SpxLogInfo2(info,err) \
    ((string_t) "File:%s,Line:%d,Func:%s.errno:%d,info:%s.%s"),\
    __FILE__,__LINE__,__FUNCTION__,err,spx_strerror(err),info
#define SpxLogInfo3(fmt,info,err)((string_t) \
        "File:%s,Line:%d,Func:%s.errno:%d,info:%s.%s"),\
    __FILE__,__LINE__,__FUNCTION__,err,spx_strerror(err),info
#define SpxLogInfo4(fmt,info)\
    ((string_t) "File:%s,Line:%d,Func:%s."fmt),\
    __FILE__,__LINE__,__FUNCTION__,info
 */
#define SpxLog1(log,level,info) \
    if(NULL != (log)) {\
        (log)(level,((string_t) "File:%s,Line:%d,Func:%s.%s."), \
                __FILE__,__LINE__,__FUNCTION__,info); \
    }

#define SpxLog2(log,level,err,info) \
    if(NULL != (log)) {\
        (log)(level,((string_t) "File:%s,Line:%d,Func:%s.errno:%d,info:%s.%s."),\
                __FILE__,__LINE__,__FUNCTION__,err,spx_strerror(err),info);\
    }

#define SpxLogFmt1(log,level,fmt,...) \
    if(NULL != (log)) {\
        (log)(level,((string_t) "File:%s,Line:%d,Func:%s."fmt),\
                __FILE__,__LINE__,__FUNCTION__,__VA_ARGS__);\
    }

#define SpxLogFmt2(log,level,err,fmt,...) \
    if(NULL != (log)) {\
        (log)(level,((string_t) "File:%s,Line:%d,Func:%s.errno:%d,info:%s."fmt),\
                __FILE__,__LINE__,__FUNCTION__,err,spx_strerror(err),__VA_ARGS__);\
    }

#define SpxErrReset errno = 0
#define SpxErr1 (0 != errno)
#define SpxErr2(rc) (0 != (rc = errno))

#define SpxPathMode 0777
#define SpxFileMode 0777
#define SpxPathSize 1023
#define SpxFileNameSize 127
#define SpxPathDlmt '/'
#define SpxPathDlmtString "/"
#define SpxLineSize 2047
#define SpxLineEndDlmtString "\n"
#define SpxKeyStringSize 255

#define SpxGB (1024 * 1024 * 1024)
#define SpxMB (1024 * 1024)
#define SpxKB (1024)

#define SpxMin(a,b) ((a) < (b) ? (a) : (b))
#define SpxMax(a,b) ((a) > (b) ? (a) : (b))

#ifdef Spx64
#define SpxPtrSize 8
#elif Spx32
#define SpxPtrSize 4
#else
#define SpxPtrSize 4
#endif

#define SpxI32Size   (sizeof("-2147483648") - 1)
#define SpxI64Size   (sizeof("-9223372036854775808") - 1)

#define LF     (u_char) 10
#define CR     (u_char) 13
#define CRLF   "\x0d\x0a"


#if Spx32
#define SpxIntSize   SpxI32Size
#else
#define SpxIntSize   SpxI64Size
#endif

#if (Spx64)
#define SpxAtomicSize            (sizeof("-9223372036854775808") - 1)
#else
#define SpxAtomicSize            (sizeof("-2147483648") - 1)
#endif


#if ((__GNU__ == 2) && (__GNUC_MINOR__ < 8))
#define SpxU32Max  (u32_t) 0xffffffffLL
#else
#define SpxI32Max  (u32_t) 0xffffffff
#endif

#define SpxClose(fd) close(fd);fd = 0

#ifdef __cplusplus
}
#endif
#endif
