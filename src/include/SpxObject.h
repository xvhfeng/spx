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
 *       Filename:  SpxObject.h
 *        Created:  2014/12/03 11时30分48秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:  this is alloctor for SpxObject
 *                  and all the object of alloctor is memory-algin
 *                  and all memory-buffer is filled-zero
 *
 ****************************************************************************/
#ifndef _SPXOBJECT_H_
#define _SPXOBJECT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>

#include "spx_types.h"
#include "spx_defs.h"

#define SpxObjectBase \
    bool_t _spxObjectIsPooling;\
    u32_t _spxObjectRefs;\
    size_t _spxObjectSize;\
    size_t _spxObjectAvail

    struct SpxObject{
        SpxObjectBase;
        char buf[0];
    };

#define SpxObjectAlignSize SpxAlign(sizeof(struct SpxObject),SpxAlignSize)

    void *spxObjectNew(const size_t s,err_t *err);
    void *spxObjectNewNumbs(const size_t numbs,const size_t s,err_t *err);
    void *spxObjectReNew(void *p,const size_t s,err_t *err);
    bool_t spxObjectFree(void *p);
    bool_t spxObjectFreeForce(void *p);
    void *spxObjectRef(void *p);
    void *spxObjectUnRef(void *p);

    spx_private u32_t spxObjectRefCount(void *p){
        if(NULL == p){
            return 0;
        }
        struct SpxObject *o = (struct SpxObject *) ((char *) p - SpxObjectAlignSize);
        return o->_spxObjectRefs;
    }

#define SpxObjectFree(p) \
    do { \
        if(NULL != p && spxObjectFree(p)) { \
            p = NULL; \
        } \
    }while(false)

#define SpxObjectFreeForce(p) \
    do { \
        if(NULL != p) {\
            spxObjectFreeForce(p);\
            p = NULL; \
        } \
    }while(false)
#ifdef __cplusplus
}
#endif
#endif

#ifdef __cplusplus
}
#endif
#endif
