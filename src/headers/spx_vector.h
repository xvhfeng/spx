#ifndef SPX_VECTOR_H
#define SPX_VECTOR_H
#ifdef __cplusplus
extern "C" {
#endif

#include "spx_types.h"

    struct spx_vector_node{
        struct spx_vector_node *next;
        void *v;
    };

    typedef err_t (SpxVectorNodeFreeDelegate)(void **v);

    struct spx_vector{
        struct spx_vector_node *header;
        struct spx_vector_node *tail;
        SpxVectorNodeFreeDelegate *handle;
        size_t size;
        SpxLogDelegate *log;
    };

    struct spx_vector_iter{
        struct spx_vector *vector;
        struct spx_vector_node *curr;
    };

    err_t spx_vector_init(SpxLogDelegate *log,struct spx_vector **vector,\
            SpxVectorNodeFreeDelegate *handle);
    err_t spx_vector_destory(struct spx_vector **vector);
    err_t spx_vector_add(struct spx_vector *vector,void *v);
    err_t spx_vector_get(struct spx_vector *vector,size_t idx,void **v);

    err_t spx_vector_push(struct spx_vector *vector,void *v);
    err_t spx_vector_pop(struct spx_vector *vector, void **v);
    err_t spx_vector_iter_init(struct spx_vector *vector,\
            struct spx_vector_iter **iter);
    err_t spx_vector_iter_destroy(struct spx_vector_iter **iter);
    void *spx_vector_iter_next(struct spx_vector_iter *iter) ;


#ifdef __cplusplus
}
#endif
#endif
