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
 *       Filename:  SpxPeriodic.h
 *        Created:  2015年01月27日 11时38分38秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/
#ifndef _SPXPERIODIC_H_
#define _SPXPERIODIC_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>

    typedef var (SpxPeriodicDelegate)(var arg);

#define SpxperiodicRunning 0
#define SpxperiodicSetPausing 1
#define SpxperiodicPausing 2

    struct SpxPeriodic{
        SpxLogDelegate *log;
        SpxPeriodicDelegate *_periodicHandler;
        u32_t _sec;
        u64_t _usec;
        err_t err;
        bool_t _isRun;
        i32_t _status;
        var arg;
        pthread_t _tid;
        pthread_mutex_t *_mlock;
        pthread_cond_t *_clock;
    };

    void spxPeriodicSleep(int sec,int msec);

#ifdef __cplusplus
}
#endif
#endif
