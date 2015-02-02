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
 *       Filename:  SpxList.h
 *        Created:  2015年01月22日 11时55分37秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/
#ifndef _SPXLIST_H_
#define _SPXLIST_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>

#include "SpxTypes.h"

    typedef err_t (SpxListNodeFreeDelegate)(var arg);
    typedef var (SpxListNodeNewDelegate)(size_t i,var arg,err_t *err);

    struct SpxList{
        SpxLogDelegate *log;
        size_t _freeSize;
        size_t _busySize;
        size_t _size;
        SpxListNodeFreeDelegate *_nodeFreeHandler;
        var *_nodes;
    };

    struct SpxList *spxListNew(SpxLogDelegate *log,\
            size_t size,\
            SpxListNodeFreeDelegate *nodeFreeHandler,\
            err_t *err);

    struct SpxList *spxListInit(SpxLogDelegate *log,
        size_t size,
        SpxListNodeNewDelegate *newHandler,
        var arg,
        SpxListNodeFreeDelegate *freeHandler,
        err_t *err);

    var spxListGet(struct SpxList *list,int idx);
    var spxListOut(struct SpxList *list,int idx);
    err_t spxListRemove(struct SpxList *list,int idx);
    err_t spxListInsert(struct SpxList *list,int idx,var v);
    err_t spxListAdd(struct SpxList *list,var v);
    bool_t spxListFree(struct SpxList *list);
    err_t spxListSet(struct SpxList *list,int idx,var v);

#define __SpxListFree(v) \
    do { \
        if(NULL != v && spxListFree(v)){ \
            v = NULL;\
        } \
    }while(false)


#ifdef __cplusplus
}
#endif
#endif
