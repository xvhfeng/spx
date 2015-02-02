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
 *       Filename:  SpxDateTime.h
 *        Created:  2015年01月17日 22时23分58秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/
#ifndef _SPXDATETIME_H_
#define _SPXDATETIME_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>

#include "SpxTypes.h"

u64_t spxClock();

struct SpxDateTime *spxCurrentDateTime(err_t *err);
struct SpxDate *spxToday(err_t *err);
time_t spxNow() ;
time_t spxZeroClock(struct SpxDate *d);
time_t spxMakeTime(struct SpxDateTime *dt);
time_t spxTodayZeroClock();
time_t spxMakeZeroClock(struct SpxDate *dt);
struct SpxDateTime *spxTimeToDateTime(time_t *t,err_t *err);
struct SpxDate *spxTimeToDate(time_t *t,err_t *err) ;
void spxDateTimeAddDays(struct SpxDateTime *dt,int days);
void spxDateAddDays(struct SpxDate *d,int days);
err_t spxCurrentDateTimeReFresh(struct SpxDateTime *sdt);
err_t spxDateReFresh(struct SpxDate *sdt);

/*
 * -1 : the day is before today
 *  0 : the day is today
 *  1 : the day after today
 */
int spxDateIsBAT(struct SpxDate *d);

int spxDateTimeIsBAT(struct SpxDateTime *dt);

err_t spxModifyFiletime(const string_t filename,u64_t secs);

/*
 * fmt:yyyy-MM-dd
 */
struct SpxDate *spxDateConvert(string_t s,char *fmt,err_t *err);

/*
 * fmt:hh:mm:ss
 */
struct SpxTime *spxTimeConvert(string_t s,char *fmt,err_t *err) ;

struct SpxDateTime *spxDateTimeConvert(string_t s,char *fmt,err_t *err) ;


#ifdef __cplusplus
}
#endif
#endif
