/*************************************************************
 *                     _ooOoo_
 *                    o8888888o
 *                    88" . "88
 *                    (| -_- |)
 *                    O\  =  /O
 *                 ____/`---'\____
 *               .'  \\|     |//  `.
 *              /  \\|||  :  |||//  \
 *             /  _||||| -:- |||||-  \
 *             |   | \\\  -  /// |   |
 *             | \_|  ''\---/''  |   |
 *             \  .-\__  `-`  ___/-. /
 *           ___`. .'  /--.--\  `. . __
 *        ."" '<  `.___\_<|>_/___.'  >'"".
 *       | | :  `- \`.;`\ _ /`;.`/ - ` : | |
 *       \  \ `-.   \_ __\ /__ _/   .-` /  /
 *  ======`-.____`-.___\_____/___.-`____.-'======
 *                     `=---='
 *  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *           佛祖保佑       永无BUG
 *
 * ==========================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  SpxDateTime.c
 *        Created:  2015年01月17日 22时24分03秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <utime.h>

#include "SpxTypes.h"
#include "SpxMFunc.h"
#include "SpxLimits.h"
#include "SpxObject.h"
#include "SpxString.h"
#include "SpxDateTime.h"

struct _SpxDateTimeContext{
    int start;
    int count;
    char op;
};


err_t spxCurrentDateTimeReFresh(struct SpxDateTime *sdt){/*{{{*/
    if(NULL == sdt){
        return EINVAL;
    }
    time_t timep;
    struct tm *p = NULL;
    time(&timep);
    p = localtime(&timep);
    sdt->year = 1900 + p->tm_year;
    sdt->month = 1 + p->tm_mon;
    sdt->day = p->tm_mday;
    sdt->hour = p->tm_hour;
    sdt->min = p->tm_min;
    sdt->sec = p->tm_sec;
    return 0;
}/*}}}*/

err_t spxDateReFresh(struct SpxDate *sdt){/*{{{*/
    if(NULL == sdt){
        return EINVAL;
    }
    time_t timep;
    struct tm *p = NULL;
    time(&timep);
    p = localtime(&timep);
    sdt->year = 1900 + p->tm_year;
    sdt->month = 1 + p->tm_mon;
    sdt->day = p->tm_mday;
    return 0;
}/*}}}*/

u64_t spxClock(){/*{{{*/
    struct timeval tv;
    __SpxZero(tv);
    gettimeofday (&tv , NULL);
    u64_t sec = tv.tv_sec;
    u64_t usec = tv.tv_usec;
    return (u64_t) (sec * SpxSecToTimerClock + usec);
}/*}}}*/

struct SpxDateTime *spxCurrentDateTime(err_t *err){/*{{{*/
    struct SpxDateTime *sdt = NULL;
    sdt = __SpxObjectNew(struct SpxDateTime,err);
    if(NULL == sdt){
        return NULL;
    }
    time_t timep;
    struct tm *p = NULL;
    time(&timep);
    p = localtime(&timep);
    sdt->year = 1900 + p->tm_year;
    sdt->month = 1 + p->tm_mon;
    sdt->day = p->tm_mday;
    sdt->hour = p->tm_hour;
    sdt->min = p->tm_min;
    sdt->sec = p->tm_sec;
    return sdt;
}/*}}}*/

struct SpxDate *spxToday(err_t *err){/*{{{*/
    struct SpxDate *sd = NULL;
    sd = __SpxObjectNew(struct SpxDate,err);
    if(NULL == sd){
        return NULL;
    }
    time_t timep;
    struct tm *p;
    time(&timep);
    p=localtime(&timep);
    sd->year = 1900 + p->tm_year;
    sd->month = 1 + p->tm_mon;
    sd->day = p->tm_mday;
    return sd;
}/*}}}*/

time_t spxNow() {/*{{{*/
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    timep = mktime(p);
    return timep;
} /*}}}*/

time_t spxZeroClock(struct SpxDate *d){/*{{{*/
    time_t timep;
    struct tm p;
    __SpxZero(p);
    p.tm_year = d->year - 1900;
    p.tm_mon = d->month - 1;
    p.tm_mday = d->day;
    timep = mktime(&p);
    return timep;
}/*}}}*/

time_t spxMakeTime(struct SpxDateTime *dt){/*{{{*/
    time_t timep;
    struct tm p;
    __SpxZero(p);
    p.tm_year = dt->year - 1900;
    p.tm_mon = dt->month - 1;
    p.tm_mday = dt->day;
    p.tm_hour = dt->hour;
    p.tm_min = dt->min;
    p.tm_sec = dt->sec;
    timep = mktime(&p);
    return timep;
}/*}}}*/

time_t spxTodayZeroClock(){/*{{{*/
    struct SpxDate sd;
    __SpxZero(sd);
    time_t timep;
    struct tm *p;
    time(&timep);
    p=localtime(&timep);
    sd.year = 1900 + p->tm_year;
    sd.month = 1 + p->tm_mon;
    sd.day = p->tm_mday;
    return spxMakeZeroClock(&sd);
}/*}}}*/

time_t spxMakeZeroClock(struct SpxDate *dt){/*{{{*/
    time_t timep;
    struct tm p;
    __SpxZero(p);
    p.tm_year = dt->year - 1900;
    p.tm_mon = dt->month - 1;
    p.tm_mday = dt->day;
    timep = mktime(&p);
    return timep;
}/*}}}*/

struct SpxDateTime *spxTimeToDateTime(time_t *t,err_t *err){/*{{{*/
    struct SpxDateTime *dt = NULL;
    dt = __SpxObjectNew(struct SpxDateTime,err);
    if(NULL == dt){
        return NULL;
    }
    struct tm *p;
    p=localtime(t);
    dt->year = 1900 + p->tm_year;
    dt->month = 1 + p->tm_mon;
    dt->day = p->tm_mday;
    dt->hour = p->tm_hour;
    dt->min = p->tm_min;
    dt->sec = p->tm_sec;
    return dt;
}/*}}}*/

struct SpxDate *spxTimeToDate(time_t *t,err_t *err) {/*{{{*/
    struct SpxDate *dt = NULL;
    dt = __SpxObjectNew(struct SpxDate,err);
    if(NULL == dt){
        return NULL;
    }
    struct tm *p;
    p=localtime(t);
    dt->year = 1900 + p->tm_year;
    dt->month = 1 + p->tm_mon;
    dt->day = p->tm_mday;
    return dt;
}/*}}}*/

void spxDateTimeAddDays(struct SpxDateTime *dt,int days){/*{{{*/
    time_t secs = spxMakeTime(dt);
    secs += days * SpxSecondsOfDay;
    struct tm *p;
    p=localtime(&secs);
    dt->year = 1900 + p->tm_year;
    dt->month = 1 + p->tm_mon;
    dt->day = p->tm_mday;
    dt->hour = p->tm_hour;
    dt->min = p->tm_min;
    dt->sec = p->tm_sec;
}/*}}}*/

void spxDateAddDays(struct SpxDate *d,int days){/*{{{*/
    time_t secs = spxZeroClock(d);
    secs += days * SpxSecondsOfDay;
    struct tm *p;
    p=localtime(&secs);
    d->year = 1900 + p->tm_year;
    d->month = 1 + p->tm_mon;
    d->day = p->tm_mday;
}/*}}}*/

/*
 * -1 : the day is before today
 *  0 : the day is today
 *  1 : the day after today
 */
int spxDateIsBAT(struct SpxDate *d){/*{{{*/
    time_t time = spxZeroClock(d);
    time_t todayClock = spxTodayZeroClock();
    if(time < todayClock) return -1;
    if(time > todayClock) return 1;
    return 0;
}/*}}}*/

int spxDateTimeIsBAT(struct SpxDateTime *dt){/*{{{*/
    time_t time = spxMakeTime(dt);
    time_t todayClock = spxTodayZeroClock();
    if(time < todayClock) return -1;
    if(time > todayClock) return 1;
    return 0;
}/*}}}*/

err_t spxModifyFiletime(const string_t filename,u64_t secs){/*{{{*/
    struct timeval val[2];
    memset(&val,0,2 * sizeof(struct timeval));
    val[0].tv_sec = secs;
    val[1].tv_sec = secs;
    if(0 != utimes(filename,val)){
        return errno;
    }
    return 0;
}/*}}}*/

/*
 * fmt:yyyy-MM-dd
 */
struct SpxDate *spxDateConvert(string_t s,char *fmt,err_t *err){/*{{{*/
    struct SpxDateTime *dt = spxDateTimeConvert(s,fmt,err);
    if(NULL == dt){
        return NULL;
    }
    struct SpxDate *d = NULL;
    d = __SpxObjectNew(struct SpxDate,err);
    if(NULL == d){
        __SpxObjectFree(dt);
        return NULL;
    }
    d->year = dt->year;
    d->month = dt->month;
    d->day = dt->day;
    __SpxObjectFree(dt);
    return d;
}/*}}}*/

/*
 * fmt:hh:mm:ss
 */
struct SpxTime *spxTimeConvert(string_t s,char *fmt,err_t *err) {/*{{{*/
    struct SpxDateTime *dt = spxDateTimeConvert(s,fmt,err);
    if(NULL == dt){
        return NULL;
    }
    struct SpxTime *t = NULL;
    t = __SpxObjectNew(struct SpxTime,err);
    if(NULL == t){
        __SpxObjectFree(dt);
        return NULL;
    }
    t->hour = dt->hour;
    t->min = dt->min;
    t->sec = dt->sec;
    __SpxObjectFree(dt);
    return t;
}/*}}}*/

struct SpxDateTime *spxDateTimeConvert(string_t s,char *fmt,err_t *err) {/*{{{*/
    struct _SpxDateTimeContext sdtc[6];
    memset(sdtc,0,sizeof(struct _SpxDateTimeContext));
    char op = 0;
    int idx = -1;
    char lastop = 0;
    int start = 0 ;
    while('\0' != (op = *(fmt++))){
        switch(op){
            case 'y':
            case 'Y':
                {
                    if(op != lastop) {
                        idx++;
                        sdtc[idx].op = 'y';
                        sdtc[idx].start = start;
                    }
                    sdtc[idx].count++;
                    start++;
                    lastop = 'y';
                    break;
                }
            case 'M':
                {
                    if(op != lastop) {
                        idx++;
                        sdtc[idx].op = op;
                        sdtc[idx].start = start;
                    }
                    sdtc[idx].count++;
                    start++;
                    lastop = op;
                    break;
                }
            case 'd':
            case 'D':
                {
                    if(op != lastop) {
                        idx++;
                        sdtc[idx].op = 'd';
                        sdtc[idx].start = start;
                    }
                    sdtc[idx].count++;
                    start++;
                    lastop = 'd';
                    break;
                }
            case 'h':
            case 'H':
                {
                    if(op != lastop) {
                        idx++;
                        sdtc[idx].op = 'h';
                        sdtc[idx].start = start;
                    }
                    sdtc[idx].count++;
                    start++;
                    lastop = 'h';
                    break;
                }
            case 'm':
                {
                    if(op != lastop) {
                        idx++;
                        sdtc[idx].op = op;
                        sdtc[idx].start = start;
                    }
                    sdtc[idx].count++;
                    start++;
                    lastop = op;
                    break;
                }
            case 's':
            case 'S':
                {
                    if(op != lastop) {
                        idx++;
                        sdtc[idx].op = 's';
                        sdtc[idx].start = start;
                    }
                    sdtc[idx].count++;
                    start++;
                    lastop = 's';
                    break;
                }
            default:{
                        lastop = op;
                        start++;
                    }
        }
    }

    struct SpxDateTime *dt = NULL;
    string_t news = __SpxStringClone(s,err);
    if(NULL == news){
        return NULL;
    }
    dt = __SpxObjectNew(struct SpxDateTime,err);
    if(NULL == dt){
        goto r1;
    }
    size_t len = __spxStringLength(news);

    int i = 0;
    for( ; i < idx; i++){
        switch(sdtc[i].op) {
            case 'y':
                {
                    size_t end = sdtc[i].start + sdtc[i].count;
                    if(end > len){
                        goto r1;
                    }
                    string_t year = NULL;
                    year = news + sdtc[i].start;
                    char p = 0;
                    if(end != len){
                        p = news[end];
                        news[end] = 0;
                    }
                    dt->year = atoi(year);
                    if(!p) news[end] = p;
                    break;
                }
            case 'M':
                {
                    size_t end = sdtc[i].start + sdtc[i].count;
                    if(end > len){
                        goto r1;
                    }
                    string_t month = NULL;
                    month = news + sdtc[i].start;
                    char p = 0;
                    if(end != len){
                        p = news[end];
                        news[end] = 0;
                    }
                    dt->month = atoi(month);
                    if(!p) news[end] = p;
                    break;
                }
            case 'd':
                {
                    size_t end = sdtc[i].start + sdtc[i].count;
                    if(end > len){
                        goto r1;
                    }
                    string_t day = NULL;
                    day = news + sdtc[i].start;
                    char p = 0;
                    if(end != len){
                        p = news[end];
                        news[end] = 0;
                    }
                    dt->day = atoi(day);
                    if(!p) news[end] = p;
                    break;
                }
            case 'h':
                {
                    size_t end = sdtc[i].start + sdtc[i].count;
                    if(end > len){
                        goto r1;
                    }
                    string_t hour = NULL;
                    hour = news + sdtc[i].start;
                    char p = 0;
                    if(end != len){
                        p = news[end];
                        news[end] = 0;
                    }
                    dt->hour = atoi(hour);
                    if(!p) news[end] = p;
                    break;
                }
            case 'm':
                {
                    size_t end = sdtc[i].start + sdtc[i].count;
                    if(end > len){
                        goto r1;
                    }
                    string_t min = NULL;
                    min = news + sdtc[i].start;
                    char p = 0;
                    if(end != len){
                        p = news[end];
                        news[end] = 0;
                    }
                    dt->min = atoi(min);
                    if(!p) news[end] = p;
                    break;
                }
            case 's':
                {
                    size_t end = sdtc[i].start + sdtc[i].count;
                    if(end > len){
                        goto r1;
                    }
                    string_t sec = NULL;
                    sec = news + sdtc[i].start;
                    char p = 0;
                    if(end != len){
                        p = news[end];
                        news[end] = 0;
                    }
                    dt->sec = atoi(sec);
                    if(!p) news[end] = p;
                    break;
                }
        }
    }
    if(NULL != news){
        __SpxStringFree(news);
    }
    return dt;
r1:
    if(NULL != news){
        __SpxStringFree(news);
    }
    if(NULL != dt){
        __SpxObjectFree(dt);
    }
    return NULL;
}/*}}}*/




