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
 *       Filename:  SpxTypes.h
 *        Created:  2015年01月09日 22时15分35秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/
#ifndef _SPXTYPES_H_
#define _SPXTYPES_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <wchar.h>
#include <sys/types.h>

#ifndef string_t
    typedef char *string_t;
#endif

#ifndef var
    typedef void *var;
#endif

#ifndef byte_t
    typedef signed char byte_t;
#endif

#ifndef ubyte_t
    typedef unsigned char ubyte_t;
#endif

#ifndef uchar_t
    typedef unsigned char uchar_t;
#endif

#ifndef u64_t
    typedef u_int64_t u64_t;
#endif

#ifndef u32_t
    typedef u_int32_t u32_t;
#endif

#ifndef u16_t
    typedef u_int16_t u16_t;
#endif

#ifndef u8_t
    typedef u_int8_t u8_t;
#endif

#ifndef i64_t
    typedef int64_t i64_t;
#endif

#ifndef i32_t
    typedef int32_t i32_t;
#endif

#ifndef i16_t
    typedef int16_t i16_t;
#endif

#ifndef i8_t
    typedef int8_t i8_t;
#endif

#ifndef SpxLogDelegate
    typedef void (SpxLogDelegate)(int level,string_t fmt,...);
#endif

#ifndef ptr_t
    typedef char *ptr_t;
#endif


#ifndef err_t
#define err_t int
#endif

#ifndef SpxInline
#define SpxInline inline
#endif

#ifndef private
#define private static
#endif

#ifndef public
#define public
#endif

    struct SpxHost{
        string_t ip;
        int port;
    };

    struct SpxDate{
        int year;
        int month;
        int day;
    };

    struct SpxTime{
        int hour;
        int min;
        int second;
    };

    struct SpxDateTime{
        int year;
        int month;
        int day;
        int hour;
        int min;
        int second;
    };

#ifndef bool_t
    typedef enum {
        false = 0,
        true = 1
    }bool_t;
#endif


#ifdef __cplusplus
}
#endif
#endif
