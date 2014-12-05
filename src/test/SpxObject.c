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

#include "spx_types.h"
#include "spx_defs.h"
#include "spx_alloc.h"
#include "spx_atomic.h"
#include "SpxObject.h"


void *spxObjectNew(const size_t s,err_t *err){/*{{{*/
    if(0 == s){
        *err = EINVAL;
        return NULL;
    }
    return spxObjectNewNumbs(s,sizeof(char),err);
}/*}}}*/

void *spxObjectNewNumbs(const size_t numbs,const size_t s,err_t *err){/*{{{*/
    if(0 == s || 0 == numbs){
        *err = EINVAL;
        return NULL;
    }
    size_t realSize = SpxAlign((s * numbs),SpxAlignSize);
    size_t totalSize = realSize + SpxObjectAlignSize;
    struct SpxObject *o = spx_alloc(totalSize,sizeof(char),err);
    if(NULL == o){
        *err = 0 == errno ? ENOMEM : errno;
        return NULL;
    }
    o->_spxObjectRefs = 1;
    o->_spxObjectIsPooling = false;
    o->_spxObjectSize = realSize;
    o->_spxObjectAvail = 0;
    return o->buf;
}/*}}}*/

void *spxObjectReNew(void *p,const size_t s,err_t *err){/*{{{*/
    if(0 == s){
        *err = EINVAL;
        return NULL;
    }
    if(NULL == p)
        return spxObjectNew(s,err);

    struct SpxObject *o = (struct SpxObject *) ((char *) p - SpxObjectAlignSize);
    size_t oRealSize = o->_spxObjectSize;
    size_t realSize = SpxAlign(s,SpxAlignSize);
    if(oRealSize >= realSize) {
        o->_spxObjectAvail = oRealSize - realSize;
        return p;
    }
    size_t totalSize = SpxObjectAlignSize + realSize;
    struct SpxObject *ptr = spx_realloc(o,totalSize,err);
    if(NULL == ptr){
        *err = 0 == errno ? ENOMEM : errno;
        return NULL;
    } else {
        ptr->_spxObjectSize = realSize;
        ptr->_spxObjectAvail = 0;
        return ptr->buf;
    }
    return NULL;
}/*}}}*/

bool_t spxObjectFree(void *p){/*{{{*/
    struct SpxObject *o = (struct SpxObject *) SpxMemDecr(p,SpxObjectAlignSize);
    if(!o->_spxObjectIsPooling){
        if(0 == SpxAtomicDecr(&(o->_spxObjectRefs))){
            SpxFree(o);
            return true;
        }
    }
    return false;
}/*}}}*/

bool_t spxObjectFreeForce(void *p){/*{{{*/
    struct SpxObject *o = (struct SpxObject *) SpxMemDecr(p,SpxObjectAlignSize);
    if(!o->_spxObjectIsPooling){
        SpxFree(o);
        return true;
    }
    return false;
}/*}}}*/

void *spxObjectRef(void *p) {/*{{{*/
    if(NULL == p){
        return NULL;
    }
    struct SpxObject *o = (struct SpxObject *) ((char *) p - SpxObjectAlignSize);
    if(0 == o->_spxObjectRefs ){
        return NULL;
    }
    SpxAtomicVIncr(o->_spxObjectRefs);
    return p;
}/*}}}*/

void *spxObjectUnRef(void *p){/*{{{*/
    if(NULL == p){
        return NULL;
    }
    struct SpxObject *o = (struct SpxObject *) ((char *) p - SpxObjectAlignSize);
    if(0 == o->_spxObjectRefs){
        return NULL;
    }
    if(0 == SpxAtomicVDecr(o->_spxObjectRefs)){
        if(!o->_spxObjectIsPooling){
            SpxFree(o);
            return NULL;
        }
    }
    return p;
}/*}}}*/
