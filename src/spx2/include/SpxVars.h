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
 *       Filename:  SpxVars.h
 *        Created:  2015年01月12日 10时20分37秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/
#ifndef _SPXVARS_H_
#define _SPXVARS_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>

    private char *SpxBoolDesc[] = {
        "false",
        "true"
    };

    private int SpxBoolDescLength[] = {
        5,
        4
    };

#define SpxLogDebug 0
#define SpxLogInfo 1
#define SpxLogWarn 2
#define SpxLogError 3
#define SpxLogMark 4

    private char *SpxLogDesc[] = {
        "Debug.",
        "Info.",
        "Warn.",
        "Error.",
        "Mark."
    };

    private int SpxLogDescLength[] = {
        6,
        5,
        5,
        6,
        5
    };

    private char *SpxDiskUnitDesc[]={
        "PB",
        "TB",
        "GB",
        "MB",
        "KB",
        "B"
    };


#define SpxDiskUnitPB 0
#define SpxDiskUnitTB 1
#define SpxDiskUnitGB 2
#define SpxDiskUnitMB 3
#define SpxDiskUnitKB 4
#define SpxDiskUnitB 5

#define SpxHttpLineEnd "\r\n"

#ifdef __cplusplus
}
#endif
#endif
