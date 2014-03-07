
#ifndef SPX_DEFS_H
#define SPX_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "spx_types.h"
#define SpxLogDebug 0
#define SpxLogWarn 1
#define SpxLogError 2
#define SpxLogMark 3


const char *SpxLogInfo[] = {
    "Debug",
    "Warn",
    "Error",
    "Mark"
};

#define SpxStackFmt "File:%s,line:%d,func:%s."
#define SpxStackInfo __FILE__,__LINE__,__FUNCTION__
#define SpxErrFmt "errno:%d,err info:%s."
#define SpxErrInfo(err) err,spx_strerror(err)

#define SpxLogInfo1(info) "File:%s,Line:%s,Func:%s."info,\
                            __FILE__,__LINE__,__FUNCTION__
#define SpxLogInfo2(info,err) "File:%s,Line:%s,Func:%s."info"errno:%d,info:%s.",\
                            __FILE__,__LINE__,__FUNCTION__,err,spx_strerror(err)
#define SpxLogInfo3(fmt,info,err) "File:%s,Line:%s,Func:%s."fmt"errno:%d,info:%s.",\
                            __FILE__,__LINE__,__FUNCTION__,info,err,spx_strerror(err)

#define SpxErrReset errno = 0
#define SpxPathMode 0777
#define SpxPathSize 1023
#define SpxPathDlmt '/'

#ifdef __cplusplus
}
#endif
#endif
