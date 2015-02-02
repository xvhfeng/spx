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
 *       Filename:  SpxVector.c
 *        Created:  2015年01月22日 11时50分38秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "SpxTypes.h"
#include "SpxAtomic.h"
#include "SpxMFunc.h"
#include "SpxVars.h"
#include "SpxVector.h"
#include "SpxObject.h"

struct SpxVector *spx_vector_init(SpxLogDelegate *log,\
        SpxVectorValueFreeDelegate *handle,err_t *err){/* {{{*/
    err_t rc = 0;
    struct SpxVector *vector = NULL;
    vector = spxObjectNew(sizeof(*vector),err);
    if(NULL == vector) {
        __SpxLog2(log,SpxLogError,rc,\
                "new vector obejct is fail.");
        return NULL;
    }
    vector->log = log;
    vector->_handle = handle;
    vector->_size = 0;
    vector->_header = NULL;
    return vector;
}/* }}}*/

bool_t spxVectorFree(struct SpxVector *vector) {/* {{{*/
    if(NULL == vector){
        return true;
    }
    __SpxObjectConvert(o,vector);
    if(0 != __SpxAtomicVDecr(o->_spxObjectRefs))
        return false;
    struct SpxVectorNode *node = NULL;
    while(NULL !=(node =  vector->_header)){
        vector->_header = vector->_header->_next;
        if(NULL != vector->_handle){
            vector->_handle(&(node->_v));
        }
        __SpxObjectFree(node);
    }
    vector->_size = 0;
    vector->_tail = NULL;
    __SpxObjectFreeForce(vector);
    return true;
}/* }}}*/

err_t spxVectorAdd(struct SpxVector *vector,var v){/* {{{*/
    err_t rc = 0;
    struct SpxVectorNode *node = NULL;
    node = spxObjectNew(sizeof(*node),&rc);
    if(NULL == node) {
        __SpxLog2(vector->log,SpxLogError,rc,\
                "alloc vector node is fail.");
        return rc;
    }
    node->_v = v;
    node->_next = NULL;
    if(NULL == vector->_header){
        vector->_header = node;
        vector->_tail = node;
    }else {
        vector->_tail->_next = node;
        node->_prev = vector->_tail;
        vector->_tail = node;
        vector->_size++;
    }
    return rc;
}/* }}}*/

var spxVectorGet(struct SpxVector *vector,size_t idx,err_t *err){/* {{{*/
    if(idx > vector->_size){
        *err = EINVAL;
        __SpxLog1(vector->log,SpxLogError,"the idx overflow the vector size.");
        return NULL;
    }
    struct SpxVectorNode *node = vector->_header;
    void *v = NULL;
    size_t i = 0;
    for (i = 0; i < vector->_size; i++) {
        if(i == idx){
            v = node->_v;
            break;
        }
        node = node->_next;
    }
    return v;
}/* }}}*/

err_t spxVectorPush(struct SpxVector *vector,void *v){
    err_t rc = 0;
    struct SpxVectorNode *node = NULL;
    node = spxObjectNew(sizeof(*node),&rc);
    if(NULL == node){
        __SpxLog2(vector->log,SpxLogError,rc,\
                "alloc vector node is fail.");
        return rc;
    }
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
    vector->_size++;
    return rc;
}

void *spxVectorPop(struct SpxVector *vector, err_t *err){
    if(NULL == vector || 0 ==  vector->_size){
        *err = EINVAL;
        __SpxLog1(vector->log,SpxLogError,"the vector argument is fail.");
        return NULL;
    }

    struct SpxVectorNode *node = NULL;
    void *v = NULL;
    if(NULL != vector->_header){
        node = vector->_header;
        vector->_header = node->_next;
        if(NULL != vector->_header){
            vector->_header->_prev = NULL;
        }
        v = node->_v;
        vector->_size --;
        __SpxObjectFree(node);
    } else {
        v = NULL;
    }
    if(NULL == vector->_header){
        vector->_tail = NULL;
    }
    return v;
}

struct SpxVectorIter  *spxVectorIterNew(struct SpxVector *vector,err_t *err){/* {{{*/
    if(NULL == vector){
        *err = EINVAL;
        return NULL;
    }
    struct SpxVectorIter *iter = NULL;
    iter = spxObjectNew(sizeof(*iter),err);
    if(NULL == iter){
        __SpxLog2(vector->log,SpxLogError,*err,\
                "allo the vector iter is fail.");
        return NULL;
    }
    iter->_vector = vector;
    iter->_curr = NULL;
    return iter;
}/* }}}*/

err_t spxVectorIterFree(struct SpxVectorIter *iter){/* {{{*/
    if(NULL == iter){
        return 0;
    }
    if(NULL == iter->_vector){
        return 0;
    }
    __SpxObjectFree(iter);
    return 0;
}/* }}}*/

void *spxVectorIterNext(struct SpxVectorIter *iter) {/* {{{*/
    if(NULL == iter){
        return NULL;
    }
    struct SpxVector *vector = iter->_vector;
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
}/* }}}*/

void spxVectorIterReset(struct SpxVectorIter *iter){/* {{{*/
    if(NULL == iter){
        return;
    }
    iter->_curr = NULL;
    return;
}/* }}}*/

