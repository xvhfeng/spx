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
 *       Filename:  SpxFixedVector.c
 *        Created:  2015年01月22日 11时51分24秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "SpxTypes.h"
#include "SpxMFunc.h"
#include "SpxVars.h"
#include "SpxFixedVector.h"
#include "SpxVector.h"
#include "SpxObject.h"
#include "SpxAtomic.h"

struct SpxFixedVector *spxFixedVectorNew(SpxLogDelegate *log,
        size_t size,
        SpxFixedVectorValueNewDeledate *nodeNewHandler,
        void *arg,
        SpxVectorValueFreeDelegate *nodeFreeHandler,
        err_t *err){/*{{{*/
    err_t rc = 0;
    struct SpxFixedVector *vector = NULL;
    vector = spxObjectNew(sizeof(*vector),err);
    if(NULL == vector) {
        __SpxLog2(log,SpxLogError,rc,\
                "new vector object is fail.");
        return NULL;
    }
    size_t i = 0;
    for(i = 0;i < size;i++){
        struct SpxVectorNode *n = spxObjectNew(sizeof(*n),err);
        if(NULL == n){
            goto r1;
        }
        var v = NULL;
        if(NULL != nodeNewHandler){
            v = nodeNewHandler(i,arg,err);
        }
        if(NULL == v){
            __SpxObjectFree(n);
            goto r1;
        }
        n->_v = v;
        if(NULL == vector->_header){
            vector->_header = n;
            vector->_tail = n;
        }else {
            n->_prev = vector->_tail;
            vector->_tail->_next = n;
            vector->_tail = n;
        }
    }
    vector->log = log;
    vector->_nodeFreeHandler = nodeFreeHandler;
    vector->_size = size;
    vector->_busyTail = NULL;
    vector->_busyHeader = NULL;
    return vector;
r1:
    {
        struct SpxVectorNode *node = NULL;
        while(NULL != (node = vector->_header)){
            vector->_nodeFreeHandler(node->_v);
            vector->_header = node->_next;
            __SpxObjectFree(node);
        }
        __SpxObjectFree(vector);
    }
    return NULL;
}/*}}}*/

bool_t spxFixedVectorFree(struct SpxFixedVector *vector){
    if(NULL == vector){
        return true;
    }
    __SpxObjectConvert(o,vector);
    if(0 != __SpxAtomicVDecr(o->_spxObjectRefs))
        return false;
    struct SpxVectorNode *node = NULL;
    while(NULL !=(node =  vector->_header)){
        vector->_header = vector->_header->_next;
        if(NULL != vector->_nodeFreeHandler){
            vector->_nodeFreeHandler(node->_v);
        }
        __SpxObjectFree(node);
    }
    while(NULL !=(node =  vector->_busyHeader)){
        vector->_busyHeader = vector->_busyHeader->_next;
        __SpxObjectFree(node);
    }
    vector->_size = 0;
    vector->_tail = NULL;
    __SpxObjectFreeForce(vector);
    return true;
}

err_t spxFixedVectorPush(struct SpxFixedVector *vector,var v){
    err_t rc = 0;
    struct SpxVectorNode *node = NULL;
    if(NULL == vector->_busyHeader){
        __SpxLog1(vector->log,SpxLogError,"the busy vector is null.");
        return ENOENT;
    }
    node = vector->_busyHeader;
    vector->_busyHeader = node->_next;
    node->_v = v;
    node->_next = NULL;
    if(NULL == vector->_header){
        vector->_header = node;
        vector->_tail = node;
    }else {
        vector->_tail->_next = node;
        node->_prev = vector->_tail;
        vector->_tail = node;
    }
    vector->_busySize --;
    return rc;
}

var spxFixedVectorPop(struct SpxFixedVector *vector, err_t *err){
    if(NULL == vector || 0 ==  vector->_size){
        *err = EINVAL;
        __SpxLog1(vector->log,SpxLogError,"the vector argument is fail.");
        return NULL;
    }

    struct SpxVectorNode *node = NULL;
    var v = NULL;
    if(NULL != vector->_header){
        node = vector->_header;
        vector->_header = node->_next;
        if(NULL != vector->_header){
            vector->_header->_prev = NULL;
        }
        v = node->_v;
        node->_prev = NULL;
        node->_next = NULL;
        if(NULL == vector->_busyHeader){
            vector->_busyHeader = node;
            vector->_busyTail = node;
        }else{
            node->_prev = vector->_busyTail;
            vector->_busyTail->_next = node;
            vector->_busyTail = node;
        }
        node->_v = NULL;
        vector->_busySize ++;
    } else {
        v = NULL;
    }
    if(NULL == vector->_header){
        vector->_tail = NULL;
    }
    return v;
}


struct SpxFixedVectorIter *spxFixedVectorIterNew(struct SpxFixedVector *vector,\
        err_t *err){
    if(NULL == vector){
        *err = EINVAL;
        return NULL;
    }
    struct SpxFixedVectorIter *iter = NULL;
    iter = spxObjectNew(sizeof(*iter),err);
    if(NULL == iter){
        __SpxLog2(vector->log,SpxLogError,*err,\
                "allo the vector iter is fail.");
        return NULL;
    }
    iter->_vector = vector;
    iter->_curr = NULL;
    return iter;
}

err_t spxFixedVectorIterFree(struct SpxFixedVectorIter *iter){
    if(NULL == iter){
        return 0;
    }
    __SpxObjectFree(iter);
    return 0;
}

var spxFixedVectorIterNext(struct SpxFixedVectorIter *iter) {
    if(NULL == iter){
        return NULL;
    }
    struct SpxFixedVector *vector = iter->_vector;
    if(NULL == vector){
        return NULL;
    }
    if(iter->_curr == iter->_vector->_tail){
        return NULL;
    }
    void *v= NULL;
    iter->_curr = NULL == iter->_curr \
                 ? vector->_header : iter->_curr->_next;
    v = iter->_curr->_v;
    return v;
}

