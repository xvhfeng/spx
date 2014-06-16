/*
 * =====================================================================================
 *
 *       Filename:  spx_fixed_vector.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/06/10 23时00分54秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>

#include "include/spx_types.h"
#include "include/spx_errno.h"
#include "include/spx_defs.h"
#include "include/spx_fixed_vector.h"
#include "include/spx_alloc.h"

struct spx_fixed_vector *spx_fixed_vector_new(SpxLogDelegate *log,\
        size_t size,\
        SpxFixedVectorValueNewDeledate *node_new_handle,
        void *arg,\
        SpxVectorValueFreeDelegate *node_free_handle,\
        err_t *err){
    err_t rc = 0;
    struct spx_fixed_vector *vector = NULL;
    struct spx_vector_node *node = NULL;
    vector = spx_alloc_alone(sizeof(*vector),err);
    if(NULL == vector) {
        SpxLog2(log,SpxLogError,rc,\
                "alloc vector is fail.");
        return NULL;
    }
    size_t i = 0;
    for(i = 0;i < size;i++){
        struct spx_vector_node *n = spx_alloc_alone(sizeof(*n),err);
        if(NULL == n){
            goto r1;
        }
        void *v = node_new_handle(arg,err);
        if(NULL == v){
            SpxFree(n);
            goto r1;
        }
        n->v = v;
        if(NULL == vector->header){
            vector->header = n;
            vector->tail = n;
        }else {
            n->prev = vector->tail;
            vector->tail->next = n;
            vector->tail = n;
        }
    }
    vector->log = log;
    vector->node_free_handle = node_free_handle;
    vector->size = size;
    vector->busy_tail = NULL;
    vector->busy_header = NULL;
    return vector;
r1:
    while(NULL != (node = vector->header)){
        vector->node_free_handle(node->v);
        vector->header = node->next;
        SpxFree(node);
    }
    SpxFree(vector);
    return NULL;
}

err_t spx_fixed_vector_destory(struct spx_fixed_vector **vector){
    if(NULL == *vector){
        return 0;
    }
    err_t rc = 0;
    struct spx_vector_node *node = NULL;
    while(NULL !=(node =  (*vector)->header)){
        (*vector)->header = (*vector)->header->next;
        if(NULL != (*vector)->node_free_handle){
            (*vector)->node_free_handle(&(node->v));
        }
        SpxFree(node);
    }
    while(NULL !=(node =  (*vector)->busy_header)){
        (*vector)->busy_header = (*vector)->busy_header->next;
        SpxFree(node);
    }
    (*vector)->size = 0;
    (*vector)->tail = NULL;
    SpxFree(*vector);
    return rc;
}

err_t spx_fixed_vector_push(struct spx_fixed_vector *vector,void *v){
    err_t rc = 0;
    struct spx_vector_node *node = NULL;
    if(NULL == vector->busy_header){
        SpxLog1(vector->log,SpxLogError,"the busy vector is null.");
        return ENOENT;
    }
    node = vector->busy_header;
    vector->busy_header = node->next;
    node->v = v;
    node->next = NULL;
    if(NULL == vector->header){
        vector->header = node;
        vector->tail = node;
    }else {
        vector->tail->next = node;
        node->prev = vector->tail;
        vector->tail = node;
    }
    vector->busysize --;
    return rc;
}

void *spx_fixed_vector_pop(struct spx_fixed_vector *vector, err_t *err){
    if(NULL == vector || 0 ==  vector->size){
        *err = EINVAL;
        SpxLog1(vector->log,SpxLogError,"the vector argument is fail.");
        return NULL;
    }

    struct spx_vector_node *node = NULL;
    void *v = NULL;
    if(NULL != vector->header){
        node = vector->header;
        vector->header = node->next;
        if(NULL != vector->header){
            vector->header->prev = NULL;
        }
        v = node->v;
        node->prev = NULL;
        node->next = NULL;
        if(NULL == vector->busy_header){
            vector->busy_header = node;
            vector->busy_tail = node;
        }else{
            node->prev = vector->busy_tail;
            vector->busy_tail->next = node;
            vector->busy_tail = node;
        }
        node->v = NULL;
        vector->busysize ++;
    } else {
        v = NULL;
    }
    if(NULL == vector->header){
        vector->tail = NULL;
    }
    return v;
}


struct spx_fixed_vector_iter *spx_fixed_vector_iter_new(struct spx_fixed_vector *vector,\
        err_t *err){
    if(NULL == vector){
        *err = EINVAL;
        return NULL;
    }
    struct spx_fixed_vector_iter *iter = NULL;
    iter = spx_alloc_alone(sizeof(*iter),err);
    if(NULL == iter){
        SpxLog2(vector->log,SpxLogError,*err,\
                "allo the vector iter is fail.");
        return NULL;
    }
    iter->vector = vector;
    iter->curr = NULL;
    return iter;
}

err_t spx_fixed_vector_iter_destroy(struct spx_vector_iter **iter){
    if(NULL == *iter){
        return 0;
    }
    if(NULL == (*iter)->vector){
        return 0;
    }
    SpxFree(*iter);
    return 0;
}

void *spx_fixed_vector_iter_next(struct spx_vector_iter *iter) {
    if(NULL == iter){
        return NULL;
    }
    struct spx_vector *vector = iter->vector;
    if(NULL == vector){
        return NULL;
    }
    if(iter->curr == iter->vector->tail){
        return NULL;
    }
    void *v= NULL;
    iter->curr = NULL == iter->curr \
                 ? vector->header : iter->curr->next;
    v = iter->curr->v;
    return v;
}


