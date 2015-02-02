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
 *       Filename:  SpxList.c
 *        Created:  2015年01月22日 11时54分45秒
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
#include "SpxList.h"
#include "SpxObject.h"
#include "SpxAtomic.h"

#define _SpxListMaxExtendSize 32

struct SpxList *spxListNew(SpxLogDelegate *log,\
        size_t size,\
        SpxListNodeFreeDelegate *nodeFreeHandler,
        err_t *err){
    struct SpxList *list = spxObjectNew(sizeof(*list),err);
    if(NULL == list){
        return NULL;
    }
    if(0 == size){
        size = 16;
    }
    list->log = log;
    list->_busySize = 0;
    list->_freeSize = size;
    list->_size = size;
    list->_nodeFreeHandler = nodeFreeHandler;
    if(0 != size){
        list->_nodes = spxObjectNewNumbs(size,sizeof(var),err);
        if(NULL == list->_nodes){
            __SpxObjectFree(list);
            return NULL;
        }
    }
    return list;
}


struct SpxList *spxListInit(SpxLogDelegate *log,
        size_t size,
        SpxListNodeNewDelegate *newHandler,
        var arg,
        SpxListNodeFreeDelegate *freeHandler,
        err_t *err){
    struct SpxList *list = __SpxObjectNew(struct SpxList,err);
    if(NULL == list){
        return NULL;
    }
    if(NULL != newHandler) {
        size_t i = 0;
        for(;i < size; i++){
            var v = newHandler(i,arg,err);
            if(NULL == v){
                spxListFree(list);
                return NULL;
            }
            *err =  spxListAdd(list,v);
            if(0 != *err){
                spxListFree(list);
                return NULL;
            }
        }
    }
    return list;
}

var spxListGet(struct SpxList *list,int idx){
    size_t i = idx < 0 ? list->_size + idx :(size_t) idx;
    if(i >= list->_size){
        return NULL;
    }
    return list->_nodes + i;
}

var spxListOut(struct SpxList *list,int idx){
    size_t i = idx < 0 ? list->_busySize + idx : (size_t) idx;
    if(i >= list->_size){
        return NULL;
    }
    var v = list->_nodes + i;
    size_t j = i;
    for(j = i; j < list->_size; j++){
        if(NULL != list->_nodes + j + 1){
            list->_nodes[j] = list->_nodes + ( j + 1);
        }
    }
    list->_busySize --;
    list->_freeSize ++;
    return v;
}

err_t spxListRemove(struct SpxList *list,int idx){
    size_t i = idx < 0 ? list->_busySize + idx : (size_t) idx;
    if(i >= list->_busySize){
        return EINVAL;
    }
    var v = list->_nodes + i;
    if(NULL != v){
        if(NULL != list->_nodeFreeHandler){
            list->_nodeFreeHandler(v);
        }
    }
    size_t j = i;
    for(; j < list->_size; j++){
        if(NULL != list->_nodes + (j + 1)){
            list->_nodes[j] = list->_nodes + ( j + 1);
        }
    }
    list->_busySize --;
    list->_freeSize ++;
    return 0;
}

err_t spxListInsert(struct SpxList *list,int idx,var v) {
    size_t i = idx < 0 ? list->_size + idx : (size_t) idx;
    err_t err = 0;
    if(i >= list->_size){
        size_t size = i > 2 * list->_size ? i : 2 * list->_size;
        if(size > _SpxListMaxExtendSize) size = _SpxListMaxExtendSize;
        var *newNodes = spxObjectReNewNumbs(list->_nodes,size,sizeof(var),&err);
        if(NULL == newNodes){
            return err;
        }
        list->_nodes = newNodes;
        list->_size = size;
        list->_freeSize = size - list->_busySize;
    }
    size_t  j = list->_size;
    for(;j > i;j--){
        list->_nodes[j] = list->_nodes + (j - 1);
    }
    list->_nodes[i] = v;
    list->_busySize ++;
    list->_freeSize --;
    return 0;
}

err_t spxListSet(struct SpxList *list,int idx,void *v){
    size_t i = idx < 0 ? list->_size + idx : (size_t) idx;
    err_t err = 0;
    if(i >= list->_size){
        size_t size = i > 2 * list->_size ? i : 2 * list->_size;
        if(size > _SpxListMaxExtendSize) size = _SpxListMaxExtendSize;
        var *newNodes = spxObjectReNewNumbs(list->_nodes,size,sizeof(var),&err);
        if(NULL == newNodes){
            return err;
        }
        list->_nodes = newNodes;
        list->_size = size;
        list->_freeSize = size - list->_busySize;
    }
    var node =  list->_nodes + i;
    if(NULL != node){
        if(NULL != list->_nodeFreeHandler) {
            list->_nodeFreeHandler(node);
        }
    }
    list->_nodes[i] = v;
    return 0;
}

err_t spxListAdd(struct SpxList *list,var v){
    err_t err = 0;
    if(list->_busySize == list->_size){
        size_t size = list->_size * 2;
        if(size > _SpxListMaxExtendSize) size = _SpxListMaxExtendSize;
        var* newNodes = spxObjectReNewNumbs(list->_nodes,size,sizeof(var),&err);
        if(NULL == newNodes){
            return err;
        }
        list->_nodes = newNodes;
        list->_size = size;
        list->_freeSize = size - list->_busySize;
    }
    list->_nodes[list->_busySize] = v;
    list->_busySize ++;
    list->_freeSize --;
    return 0;
}

bool_t spxListFree(struct SpxList *list){
    if(NULL == list) return true;
    __SpxObjectConvert(o,list);
    if(0 != __SpxAtomicVDecr(o->_spxObjectRefs))
        return false;
    size_t i = 0;
    for(;i < list->_size;i++){
        if(NULL != list->_nodeFreeHandler){
            list->_nodeFreeHandler(list->_nodes[i]);
        }
    }
    __SpxObjectFree(list->_nodes);
    __SpxObjectFreeForce(list);
    return true;
}
