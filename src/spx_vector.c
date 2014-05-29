#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "headers/spx_vector.h"
#include "headers/spx_types.h"
#include "headers/spx_defs.h"
#include "headers/spx_errno.h"
#include "headers/spx_alloc.h"

err_t spx_vector_init(SpxLogDelegate *log,struct spx_vector **vector,\
        SpxVectorNodeFreeDelegate *handle){/*{{{*/
    err_t rc = 0;
    if(0 != (rc = spx_alloc_alone(sizeof(struct spx_vector),(void **)vector))){
        SpxLog2(log,SpxLogError,rc,\
                "alloc vector is fail.");
        return rc;
    }
    (*vector)->log = log;
    (*vector)->handle = handle;
    (*vector)->size = 0;
    (*vector)->header = NULL;
    return rc;
}/*}}}*/

err_t spx_vector_destory(struct spx_vector **vector) {/*{{{*/
    if(NULL == *vector){
        return 0;
    }
    err_t rc = 0;
    struct spx_vector_node *node = NULL;
    while(NULL !=(node =  (*vector)->header)){
        (*vector)->header = (*vector)->header->next;
        if(NULL != (*vector)->handle){
            (*vector)->handle(&(node->v));
        }
        SpxFree(node);
    }
    (*vector)->size = 0;
    (*vector)->tail = NULL;
    SpxFree(*vector);
    return rc;
}/*}}}*/

err_t spx_vector_add(struct spx_vector *vector,void *v){/*{{{*/
    err_t rc = 0;
    struct spx_vector_node *node = NULL;
    if(0 != (rc = spx_alloc_alone(sizeof(struct spx_vector_node),(void **) &node))){
        SpxLog2(vector->log,SpxLogError,rc,\
                "alloc vector node is fail.");
        return rc;
    }
    node->v = v;
    node->next = NULL;
    vector->tail->next = node;
    vector->tail = node;
    vector->size++;
    return rc;
}/*}}}*/

err_t spx_vector_get(struct spx_vector *vector,size_t idx,void **v){/*{{{*/
    err_t rc = 0;
    if(idx > vector->size){
        rc = EINVAL;
        SpxLog1(vector->log,SpxLogError,"the idx overflow the vector size.");
        return rc;
    }
    struct spx_vector_node *node = vector->header;
    size_t i = 0;
    for (i = 0; i < vector->size; i++) {
        if(i == idx){
            *v = node->v;
            break;
        }
        node = node->next;
    }
    return rc;
}/*}}}*/

err_t spx_vector_push(struct spx_vector *vector,void *v){
    err_t rc = 0;
    struct spx_vector_node *node = NULL;
    if(0 != (rc = spx_alloc_alone(sizeof(struct spx_vector_node),(void **) &node))){
        SpxLog2(vector->log,SpxLogError,rc,\
                "alloc vector node is fail.");
        return rc;
    }
    node->v = v;
    node->next = NULL;
    vector->tail->next = node;
    vector->tail = node;
    vector->size++;
    return rc;
}

err_t spx_vector_pop(struct spx_vector *vector, void **v){
    err_t rc = 0;
    struct spx_vector_node *node = NULL;
    if(NULL != vector->header){
        node = vector->header;
        vector->header = node->next;
        *v = node->v;
        vector->size --;
        SpxFree(node);
    } else {
        *v = NULL;
    }
    if(NULL == vector->header){
        vector->tail = NULL;
    }
    return rc;
}

err_t spx_vector_iter_create(struct spx_vector *vector,\
        struct spx_vector_iter **iter){/*{{{*/
    if(NULL == vector){
        return EINVAL;
    }
    err_t rc = 0;
    if(0 != (rc = spx_alloc_alone(sizeof(struct spx_vector_iter),(void **) iter))){
        SpxLog2(vector->log,SpxLogError,rc,\
                "allo the vector iter is fail.");
        return rc;
    }
    (*iter)->vector = vector;
    (*iter)->curr = NULL;
    return rc;
}/*}}}*/

err_t spx_vector_iter_destroy(struct spx_vector_iter **iter){/*{{{*/
    if(NULL == *iter){
        return 0;
    }
    if(NULL == (*iter)->vector){
        return 0;
    }
    SpxFree(*iter);
    return 0;
}/*}}}*/

void *spx_vector_iter_next(struct spx_vector_iter *iter) {/*{{{*/
    if(NULL == iter){
        return NULL;
    }
    struct spx_vector *vector = iter->vector;
    if(NULL == vector){
        return NULL;
    }
    void *v= NULL;
    iter->curr = NULL == iter->curr \
                 ? vector->header : iter->curr->next;
    v = iter->curr->v;
    return v;
}/*}}}*/
