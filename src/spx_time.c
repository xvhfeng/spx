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



