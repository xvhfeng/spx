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
 *       Filename:  SpxVector.h
 *        Created:  2015年01月22日 11时50分30秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/
#ifndef _SPXVECTOR_H_
#define _SPXVECTOR_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>

#include "SpxTypes.h"

struct SpxVectorNode{
    struct SpxVectorNode *_prev;
    struct SpxVectorNode *_next;
    void *_v;
};

typedef err_t (SpxVectorValueFreeDelegate)(var v);

struct SpxVector{
    struct SpxVectorNode *_header;
    struct SpxVectorNode *_tail;
    SpxVectorValueFreeDelegate *_handle;
    size_t _size;
    SpxLogDelegate *log;
};


struct SpxVectorIter{
    struct SpxVector *_vector;
    struct SpxVectorNode *_curr;
};

struct SpxVector *spxVectorInit(SpxLogDelegate *log,\
        SpxVectorValueFreeDelegate *handle,err_t *err);
bool_t spxVectorFree(struct SpxVector *vector);
err_t spxVectorAdd(struct SpxVector *vector,void *v);
var spxVectorGet(struct SpxVector *vector,size_t idx,err_t *err);
err_t spxVectorPush(struct SpxVector *vector,void *v);
var spxVectorPop(struct SpxVector *vector, err_t *err);


struct SpxVectorIter *spxVectorNew(struct SpxVector *vector,\
        err_t *err);
err_t spxVectorIterFree(struct SpxVectorIter *iter);
var spxVectorIterNext(struct SpxVectorIter *iter) ;
void spxVectorIterReset(struct SpxVectorIter *iter);

#define __SpxVectorFree(v) \
    do { \
        if(NULL != v && spxVectorFree(v)){ \
            v = NULL;\
        } \
    }while(false)

#ifdef __cplusplus
}
#endif
#endif
