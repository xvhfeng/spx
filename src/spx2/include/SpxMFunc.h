/***********************************************************************
 *                              _ooOoo_
 *                             o8888888o
 *                             88" . "88
 *                             (| -_- |)
 *                              O\ = /O
 *                          ____/`---'\____
 *                        .   ' \\| |// `.
 *                         / \\||| : |||// \
 *                       / _||||| -:- |||||- \
 *                         | | \\\ - /// | |
 *                       | \_| ''\---/'' | |
 *                        \ .-\__ `-` ___/-. /
 *                     ___`. .' /--.--\ `. . __
 *                  ."" '< `.___\_<|>_/___.' >'"".
 *                 | | : `- \`.;`\ _ /`;.`/ - ` : | |
 *                   \ \ `-. \_ __\ /__ _/ .-` / /
 *           ======`-.____`-.___\_____/___.-`____.-'======
 *                              `=---='
 *           .............................................
 *                    佛祖镇楼                  BUG辟易
 *            佛曰:
 *                    写字楼里写字间，写字间里程序员；
 *                    程序人员写程序，又拿程序换酒钱。
 *                    酒醒只在网上坐，酒醉还来网下眠；
 *                    酒醉酒醒日复日，网上网下年复年。
 *                    但愿老死电脑间，不愿鞠躬老板前；
 *                    奔驰宝马贵者趣，公交自行程序员。
 *                    别人笑我忒疯癫，我笑自己命太贱；
 *                    不见满街漂亮妹，哪个归得程序员？
 * ==========================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  SpxMFunc.h
 *        Created:  2015年01月12日 09时19分51秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/
#ifndef _SPXMFUNC_H_
#define _SPXMFUNC_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "SpxTypes.h"
#include "SpxVars.h"
#include "SpxError.h"
#include "SpxLimits.h"


#define __SpxLog1(log,level,info) \
    if(NULL != (log)) {\
        (log)(level,((string_t) "File:%s,Line:%d,Func:%s.%s."), \
                __FILE__,__LINE__,__FUNCTION__,info); \
    }

#define __SpxLog2(log,level,err,info) \
    if(NULL != (log)) {\
        (log)(level,((string_t) "File:%s,Line:%d,Func:%s.errno:%d,info:%s.%s."),\
                __FILE__,__LINE__,__FUNCTION__,err,err >= SpxSuccess ?  spxErrorToString(err) : strerror(err),info);\
    }

#define __SpxLogFmt1(log,level,fmt,...) \
    if(NULL != (log)) {\
        (log)(level,((string_t) "File:%s,Line:%d,Func:%s."fmt),\
                __FILE__,__LINE__,__FUNCTION__,__VA_ARGS__);\
    }

#define __SpxLogFmt2(log,level,err,fmt,...) \
    if(NULL != (log)) {\
        (log)(level,((string_t) "File:%s,Line:%d,Func:%s.errno:%d,info:%s."fmt),\
                __FILE__,__LINE__,__FUNCTION__,err,err >= SpxSuccess ? spxErrorToString(err) : strerror(err),__VA_ARGS__);\
    }

#define __SpxReset(e) ((e) = 0)
#define __SpxIsSocketRetry(e) (EAGAIN == (e) || EWOULDBLOCK == (e) || EINTR == (e))
#define __SpxIsSocketReAttach(e) __SpxIsSocketRetry(e)
#define __SpxIsErr(e) (0 != (e))

#define __SpxMin(a,b) ((a) < (b) ? (a) : (b))
#define __SpxMax(a,b) ((a) > (b) ? (a) : (b))
#define __SpxAbs(a) ((a) < 0 ? -(a) : a)

#define __SpxTypeConvert(t,newp,old) t *newp = (t *) (old)

#define __SpxClose(fd)  \
    do { \
        if(0 != fd) { \
            close(fd);\
            fd = 0;\
        } \
    }while(false)


#define __SpxAlign(d, a)     (((d) + (a - 1)) & ~(a - 1))
#define __SpxAligned(d) __SpxAlign(s,SpxAlignSize);

#define __SpxIncr(p,s) (((char *) p) + (s))
#define __SpxDecr(p,s) (((char *) p) - (s))
#define __SpxOffset(p1,p2) ((size_t ) (((char *) p1) - ((char *) p2)))

#define __SpxSSet(o,p,v) o->p = (v)
#define __SpxZero(v) memset(&v,0,sizeof(v));

#ifdef __cplusplus
}
#endif
#endif
