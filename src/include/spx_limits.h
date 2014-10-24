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
 *       Filename:  spx_limits.h
 *        Created:  2014/10/24 17时30分29秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/
#ifndef _SPX_LIMITS_H_
#define _SPX_LIMITS_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>

#define i8_MAX 127
#define i8_MIN (-128)
#define u8_MAX 255
#define i8_MAX_LENGTH sizeof("-127")

#define i16_MAX 32767
#define i16_MIN (-32768)
#define u16_MAX 65535
#define i16_MAX_LENGTH sizeof("-32768")


#define i32_MIN (-2147483647 - 1)
#define i32_MAX 2147483647
#define u32_MAX 4294967295
#define i32_MAX_LENGTH sizeof("-4294967295")

#define i64_MAX 9223372036854775807
#define i64_MIN -9223372036854775808
#define u64_MAX 18446744073709551615L
#define u64_MAX_LENGTH sizeof("-9223372036854775808")

#define CHAR_BIT 8
#define CHAR_MIN i8_MIN
#define CHAR_MAX i8_MAX
#define UCHAR_MAX u8_MAX

#define BYTE_MIN i8_MIN
#define BYTE_MAX i8_MAX
#define UBYTE_MAX u8_MAX


#ifdef __cplusplus
}
#endif
#endif
