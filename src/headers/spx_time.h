/*
 * =====================================================================================
 *
 *       Filename:  spx_time.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/05/20 10时20分41秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */

#ifndef SPX_TIME_H
#define SPX_TIME_H
#if __cplusplus
extern "C" {
#endif

#include "spx_types.h"

#define SpxYear(dt) ((dt)->d.year)
#define SpxMonth(dt) ((dt)->d.month)
#define SpxDay(dt) ((dt)->d.day)
#define SpxHour(dt) ((dt)->t.hour)
#define SpxMinute(dt) ((dt)->t.min)
#define SpxSecond(dt) ((dt)->t.sec)

    struct spx_datetime{
        struct spx_date{
            int year;
            int month;
            int day;
        }d;
        struct spx_time{
            int hour;
            int min;
            int sec;
        }t;
    };


    void spx_get_curr_datetime(struct spx_datetime *dt);

#if __cplusplus
}
#endif
#endif
