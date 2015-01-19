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
 *       Filename:  SpxString.h
 *        Created:  2015年01月19日 12时43分13秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/
#ifndef _SPXSTRING_H_
#define _SPXSTRING_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>

#include "SpxTypes.h"
#include "SpxObject.h"
#include "SpxMFunc.h"

#define _SpxStringConvert(n,s) __SpxTypeConvert((struct SpxString),n, \
                                __SpxDecr(s,sizeof(struct SpxString)))


#define __SpxStringFree(s) \
    do { \
        if(NULL != s && _spxStringFree(s)) { \
            s = NULL; \
        } \
    }while(false)

#define __SpxStringFreeForce(s) \
    do { \
        if(NULL != s) {\
            _spxStringFreeForce(s);\
            s = NULL; \
        } \
    }while(false)

    struct SpxString{
        SpxObjectBase;
        size_t _freeSize;
        char _buf[0];
    };


    private u32_t __spxStringRefCount(string_t s){
        if(NULL == s){
            return 0;
        }
        _SpxStringConvert(ss,s);
        return ss->_spxObjectRefs;
    }

    private size_t __spxStringLength(string_t s){
        if(NULL == s){
            return 0;
        }
        _SpxStringConvert(ss,s);
        return ss->_spxObjectSize - ss->_freeSize;
    }

    private size_t __spxStringSize(string_t s){
        if(NULL == s){
            return 0;
        }
        _SpxStringConvert(ss,s);
        return ss->_spxObjectSize;
    }

    private size_t __spxStringFreeSize(string_t s){
        if(NULL == s){
            return 0;
        }
        _SpxStringConvert(ss,s);
        return ss->_freeSize;
    }





#ifdef __cplusplus
}
#endif
#endif
