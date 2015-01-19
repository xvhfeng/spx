/*************************************************************
 *                     _ooOoo_
 *                    o8888888o
 *                    88" . "88
 *                    (| -_- |)
 *                    O\  =  /O
 *                 ____/`---'\____
 *               .'  \\|     |//  `.
 *              /  \\|||  :  |||//  \
 *             /  _||||| -:- |||||-  \
 *             |   | \\\  -  /// |   |
 *             | \_|  ''\---/''  |   |
 *             \  .-\__  `-`  ___/-. /
 *           ___`. .'  /--.--\  `. . __
 *        ."" '<  `.___\_<|>_/___.'  >'"".
 *       | | :  `- \`.;`\ _ /`;.`/ - ` : | |
 *       \  \ `-.   \_ __\ /__ _/   .-` /  /
 *  ======`-.____`-.___\_____/___.-`____.-'======
 *                     `=---='
 *  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *           佛祖保佑       永无BUG
 *
 * ==========================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  SpxObject.c
 *        Created:  2014/12/03 13时06分30秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "SpxTypes.h"
#include "SpxAlloc.h"
#include "SpxAtomic.h"
#include "SpxObject.h"
#include "SpxError.h"
#include "SpxLimits.h"

u64_t gSpxMemoryUseSize = 0;

var spxObjectNew(const size_t s,err_t *err){/*{{{*/
    if(0 == s){
        *err = EINVAL;
        return NULL;
    }
    size_t realSize = __SpxAlign(s,SpxAlignSize);
    size_t totalSize = SpxObjectAlignSize + realSize;
    struct SpxObject *o = spxAlloc(totalSize,err);
    if(NULL == o){
        *err = 0 == errno ? SpxEAlloc : errno;
        return NULL;
    }
    __SpxAtomicVAdd(gSpxMemoryUseSize,totalSize);
    o->_spxObjectRefs = 1;
    o->_spxObjectSize = realSize;
    return o->_buf;
}/*}}}*/

//i allways guess the malloc anf calloc is the same expect clear 0
//but today,i konw they are not the same then not need the clear 0
var spxObjectNewNumbs(const size_t numbs,const size_t s,err_t *err){/*{{{*/
    if(0 == s || 0 == numbs){
        *err = EINVAL;
        return NULL;
    }
    //warn:not __SpxAlign(numbs * s,SpxAlignSize);
    //why? you can think the memory
    size_t realSize = __SpxAlign((s) ,SpxAlignSize) * numbs;
    size_t totalSize = realSize + SpxObjectAlignSize;
    struct SpxObject *o = spxAlloc(totalSize,err);
    if(NULL == o){
        *err = 0 == errno ? SpxEAlloc : errno;
        return NULL;
    }
    __SpxAtomicVAdd(gSpxMemoryUseSize,totalSize);
    o->_spxObjectRefs = 1;
    o->_spxObjectSize = realSize;
    return o->_buf;
}/*}}}*/

var spxObjectReNew(var p,const size_t s,err_t *err){/*{{{*/
    if(0 == s){
        *err = EINVAL;
        return NULL;
    }
    if(NULL == p)
        return spxObjectNew(s,err);

    __SpxTypeConvert(struct SpxObject,o,__SpxDecr(p,SpxObjectAlignSize));
    size_t oRealSize = o->_spxObjectSize;
    size_t realSize = __SpxAlign(s,SpxAlignSize);
    if(oRealSize >= realSize) {
        return p;
    }
    size_t totalSize = SpxObjectAlignSize + realSize;
    struct SpxObject *ptr = spxRealloc(o,totalSize,err);
    if(NULL == ptr){
        *err = 0 == errno ? SpxEAlloc : errno;
        return NULL;
    } else {
        __SpxAtomicVAdd(gSpxMemoryUseSize,realSize - oRealSize);
        ptr->_spxObjectSize = realSize;
        return ptr->_buf;
    }
    return NULL;
}/*}}}*/

var spxObjectReNewNumbs(var p,const size_t numbs,const size_t s,err_t *err){
    size_t size = __SpxAlign(s,SpxAlignSize) * numbs;
    return spxObjectReNew(p,size,err);
}

var spxObjectRef(var p) {/*{{{*/
    if(NULL == p){
        return NULL;
    }
    __SpxTypeConvert(struct SpxObject,o,__SpxDecr(p,SpxObjectAlignSize));
    if(0 == o->_spxObjectRefs ){
        return NULL;
    }
    __SpxAtomicVIncr(o->_spxObjectRefs);
    return p;
}/*}}}*/

bool_t _spxObjectFree(var p){/*{{{*/
    if(NULL == p){
        return true;
    }
    __SpxTypeConvert(struct SpxObject,o,__SpxDecr(p,SpxObjectAlignSize));
    if(0 == __SpxAtomicVDecr(o->_spxObjectRefs)){
        __SpxAtomicVSub(gSpxMemoryUseSize, SpxObjectAlignSize + o->_spxObjectSize);
        __SpxFree(o);
        return true;
    }
    return false;
}/*}}}*/

bool_t _spxObjectFreeForce(var p){/*{{{*/
    if(NULL == p){
        return true;
    }
    __SpxTypeConvert(struct SpxObject,o,__SpxDecr(p,SpxObjectAlignSize));
    __SpxAtomicVSub(gSpxMemoryUseSize, SpxObjectAlignSize + o->_spxObjectSize);
    __SpxFree(o);
    return true;
}/*}}}*/

