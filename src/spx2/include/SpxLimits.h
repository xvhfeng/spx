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
 *       Filename:  SpxLimits.h
 *        Created:  2015年01月12日 09时07分56秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/
#ifndef _SPXLIMITS_H_
#define _SPXLIMITS_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>

#define SpxBoolTransportLength (sizeof(char))

#define SpxI8Max 127
#define SpxI8Min (-128)
#define SpxU8Mac 255
#define SpxI8Length sizeof("-127")
#define SpxU8Length SpxI8Length

#define SpxI16Max 32767
#define SpxI16Min (-32768)
#define SpxU16Max 65535
#define SpxI16Length sizeof("-32768")
#define SpxU16Length SpxI16Length


#define SpxI32Max 2147483647
#define SpxI32Min (-2147483648)
#define SpxU32Max 4294967295
#define SpxI32Length sizeof("-4294967295")
#define SpxU32Length SpxI32Length

#define SpxI64Max 9223372036854775807
#define SpxI64Min -9223372036854775808
#define SpxU64Max 18446744073709551615L
#define SpxI64Length sizeof("-9223372036854775808")
#define SpxU64Length SpxI64Length

#define SpxBoolTransportSize 1



#define SpxPB (1024 * 1024 * 1024 * 1024 * 1024)
#define SpxTB (1024 * 1024 * 1024 * 1024)
#define SpxGB (1024 * 1024 * 1024)
#define SpxMB (1024 * 1024)
#define SpxKB (1024)

//the NClock is 1 nanosecond
//it's the 1G/Hz cpu running 1 clock-single
#define SpxCpuClock 1
#define SpxNSec SpxCpuClock
#define SpxUSecToCpuClock 1000
#define SpxMSecToCpuCloxk ( 1000 * 1000)
#define SpxSecToCpuClock (1000 * 1000 * 1000)

#define SpxTimerClock 1
#define SpxMSecToTimerClock 1000
#define SpxSecToTimerClock (1000 * 1000)

#define SpxSecondClock 1
#define SpxMinuteClock 60
#define SpxHourClock (60 * 60)
#define SpxDayClock (24 * 60 * 60)
#define SpxSecondsOfDay (24 * 60 * 60)

#define SpxPathMode 0666
#define SpxFileMode 0666
#define SpxPathLength 1023
#define SpxFileNameLength 127
#define SpxPathDlmt '/'
#define SpxSuffixDlmt '.'
#define SpxSuffixDlmtString "."
#define SpxSuffixDlmtLength 1
#define SpxPathDlmtLength 1
#define SpxPathDlmtString "/"
#define SpxLineLength 2047
#define SpxLineEndDlmtString "\n"
#define SpxLineEndDlmt '\n'
#define SpxLineEndDlmtLength 1
#define SpxKeyStringLength 255
#define SpxHostNameLength 255

#define SpxIpv4Length 15

    //note:default system is x64bit
#ifdef Spx64
#define SpxPSize 8
#elif Spx32
#define SpxPSize 4
#else
#define SpxPSize 8
#endif

#if Spx32
#define SpxAlignSize 4
#elif Spx64
#define SpxAlignSize 8
#else
#define SpxAlignSize 8
#endif

#ifdef __cplusplus
}
#endif
#endif
