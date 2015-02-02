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
 *       Filename:  SpxFixedVector.h
 *        Created:  2015年01月22日 12时00分05秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/
#ifndef _SPXFIXEDVECTOR_H_
#define _SPXFIXEDVECTOR_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>

#include "SpxTypes.h"
#include "SpxFixedVector.h"
#include "SpxVector.h"

    typedef void *SpxFixedVectorValueNewDeledate(size_t idx,void *arg,err_t *err);

    struct SpxFixedVector{
        SpxLogDelegate *log;
        struct SpxVectorNode *_header;
        struct SpxVectorNode *_tail;
        struct SpxVectorNode *_busyHeader;
        struct SpxVectorNode *_busyTail;
        SpxVectorValueFreeDelegate *_nodeFreeHandler;
        size_t _size;
        size_t _busySize;
    };


    struct SpxFixedVector *spxFixedVectorNew(SpxLogDelegate *log,
            size_t size,
            SpxFixedVectorValueNewDeledate *nodeNewHandler,
            var arg,
            SpxVectorValueFreeDelegate *nodeFreeHandler,
            err_t *err);
    bool_t spxFixedVectorFree(struct SpxFixedVector *vector);
    err_t spxFixedVectorPush(struct SpxFixedVector *vector,var v);
    var spxFixedVectorPop(struct SpxFixedVector *vector, err_t *err);


    struct SpxFixedVectorIter{
        struct SpxFixedVector *_vector;
        struct SpxVectorNode *_curr;
    };
    struct SpxFixedVectorIter *spxFixedVectorIterNew(struct SpxFixedVector *vector,
            err_t *err);
    err_t spxFixedVectorIterFree(struct SpxFixedVectorIter *iter);
    var spxFixedVectorIterNext(struct SpxFixedVectorIter *iter) ;

#define __SpxFixedVectorFree(v) \
    do { \
        if(NULL != v && spxFixedVectorFree(v)){ \
            v = NULL;\
        } \
    }while(false)




#ifdef __cplusplus
}
#endif
#endif
