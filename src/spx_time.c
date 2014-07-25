/*
 * =====================================================================================
 *
 *       Filename:  spx_time.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/05/20 10时17分06秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "include/spx_types.h"
#include "include/spx_defs.h"
#include "include/spx_time.h"
#include "include/spx_string.h"
#include "include/spx_alloc.h"

void spx_get_curr_datetime(struct spx_datetime *dt){
    time_t timep;
    struct tm *p;
    time(&timep);
    p=localtime(&timep);
    dt->d.year = 1900 + p->tm_year;
    dt->d.month = 1 + p->tm_mon;
    dt->d.day = p->tm_mday;
    dt->t.hour = p->tm_hour;
    dt->t.min = p->tm_min;
    dt->t.sec = p->tm_sec;
}
time_t spx_now() {/*{{{*/
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    timep = mktime(p);
    return timep;
} /*}}}*/

time_t spx_mktime(struct spx_datetime *dt){
    time_t timep;
    struct tm p;
    SpxZero(p);
    p.tm_year = SpxYear(dt) - 1900;
    p.tm_mon = SpxMonth(dt) - 1;
    p.tm_mday = SpxDay(dt);
    p.tm_hour = SpxHour(dt);
    p.tm_min = SpxMinute(dt);
    p.tm_sec = SpxSecond(dt);
    timep = mktime(&p);
    return timep;
}

struct spx_datetime *spx_datetime_dup(struct spx_datetime *dt,err_t *err){
    if(NULL == dt){
        *err = EINVAL;
        return NULL;
    }
    struct spx_datetime *new = spx_alloc_alone(sizeof(*new),err);
    if(NULL == new){
        return NULL;
    }
    SpxMemcpy(new,dt,sizeof(*new));
    return new;
}


struct spx_datetime *spx_get_datetime(time_t *t,struct spx_datetime *dt){
    struct tm *p;
    p=localtime(t);
    dt->d.year = 1900 + p->tm_year;
    dt->d.month = 1 + p->tm_mon;
    dt->d.day = p->tm_mday;
    dt->t.hour = p->tm_hour;
    dt->t.min = p->tm_min;
    dt->t.sec = p->tm_sec;
    return dt;
}

struct spx_datetime *spx_datetime_add_days(struct spx_datetime *dt,int days){
    time_t secs = spx_mktime(dt);
    secs += days * 24 * 3600;
    dt = spx_get_datetime(&secs,dt);
    return dt;
}
