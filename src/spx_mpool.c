#include <stdlib.h>=C_JumpCtrlJ()
#include <errno.h>
#include <string.h>

#include "headers/spx_mpool.h"
#include "headers/spx_alloc.h"
#include "headers/spx_types.h"
#include "headers/spx_errno.h"

#define mem_align(d, a)     (((d) + (a - 1)) & ~(a - 1))
#define mem_align_ptr(p, a)                                                   \
    (ubyte_t *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))


struct spx_mpool_buff{
    struct spx_mpool_buff *last;
    struct spx_mpool_buff *end;
    struct spx_mpool_buff *n;
    ubyte_t b[0];
};

struct spx_mpool_alone{
    struct spx_mpool_alone *p;
    struct spx_mpool_alone *n;
    ubyte_t e[0];
};

struct spx_mpool{
    struct spx_mpool_buff *cbuf;//the pointer to current buffer
    struct spx_mpool_buff *hbuf;//the pointer to header buffer
    struct spx_mpool_alone *as;
    struct spx_mpool_cleanup *cs;
    size_t limit;//the object size in the buf
    size_t size;//the buf size
};

struct spx_mpool_cleanup {
    spx_mpool_cleanup_handle *f;
    struct spx_mpool_cleanup *n;
    struct spx_mpool_cleanup *p;
    ubyte_t e[0];
};

err_t spx_mpool_init(const size_t size,
        const size_t limit,struct spx_mpool **p){/*{{{*/
    if(0 == size || 0 == limit || (size < limit)){
        return EINVAL;
    }
    err_t rc = 0;
    if(0 !=(rc = spx_alloc_alone(sizeof(struct spx_mpool),(void **) p))){
        return rc;
    }
    (*p)->size = size;
    (*p)->limit = limit;
    return rc;
}/*}}}*/

err_t spx_mpool_alloc(struct spx_mpool * const p,
        const size_t s,void **e){/*{{{*/
    if(0 == s || NULL == p){
        return EINVAL;
    }
    err_t rc = 0;
    struct spx_mpool_alone *a = NULL;
    if(s > p->limit){//alone
        if(0 != (rc = spx_alloc_alone(sizeof(struct spx_mpool_alone) + s,(void **) &a))){
            return rc;
        }
        if(NULL == p->as){
            p->as = a;
        }else {
            p->as->p = a;
            a->n = p->as;
            p->as = a;
        }
        *e = a + sizeof(struct spx_mpool_alone);
    } else {
        struct spx_mpool_buff *b = p->cbuf;
        if(NULL == b) {
            if(0 != (rc = spx_memalign_alloc(sizeof(struct spx_mpool_buff) + p->size,(void **) &b))){
                return rc;
            }
            b->last = b + sizeof(struct spx_mpool_buff);;
            b->end = b + p->size;
            p->cbuf = b;
            p->hbuf = b;
        }
        while(true) {
            ubyte_t *o = NULL;
            o = mem_align_ptr(b->last,s);
            if(((size_t) ((char *) b->end - (char *) o)) < s){ // ptr diff
                b = p->cbuf->n;
                if(NULL == b){
                    if(0 != (rc = spx_memalign_alloc(\
                                    sizeof(struct spx_mpool_buff) + p->size,\
                                    (void **) &b))){
                        return rc;
                    }
                    b->last = b + sizeof(struct spx_mpool_buff);
                    b->end = b + p->size;
                    p->cbuf->n = b;
                    p->cbuf = b;
                }
                continue;
            }
            b->last += s;
            *e = (void *) o;
            break;
        }
    }
    return rc;
}/*}}}*/

err_t spx_mpool_string(struct spx_mpool * const p,
        const size_t l,char **s){/*{{{*/
    if(NULL == p || 0 == l){
        return EINVAL;
    }
    return  spx_mpool_alloc(p,l + 1,(void **)s);
}/*}}}*/

err_t spx_mpool_cleanup_alloc(struct spx_mpool * const p,
        const size_t size,spx_mpool_cleanup_handle *f,
        void **e){/*{{{*/
    //if the f is null,you must use spx_mpool_alloc
    if(0 == size || NULL == f){
        return EINVAL;
    }
    err_t rc = 0;
    struct spx_mpool_cleanup *ptr = NULL;
        if(0 != (rc = spx_alloc_alone(size + sizeof( struct spx_mpool_cleanup),(void **) &ptr))){
            return rc;
        }
    ptr->f = f;
    if(NULL == p->cs){
        p->cs = ptr;
    } else {
        ptr->n = p->cs;
        p->cs->p = ptr;
        p->cs = ptr;
    }
    *e = ptr + sizeof(struct spx_mpool_cleanup);
    return rc;

}/*}}}*/

err_t spx_mpool_cleanup_free(const struct spx_mpool * const p,
        void **e){/*{{{*/
    if(NULL == e || NULL == p){
        return EINVAL;
    }
    struct spx_mpool_cleanup *ptr = (struct spx_mpool_cleanup *) (*e) - sizeof(struct spx_mpool_cleanup);
    if(NULL == ptr){
        return ENXIO;
    }
    ptr->f(&ptr);
    spx_free(ptr);
    return 0;
}/*}}}*/

err_t spx_mpool_reset(struct spx_mpool * const p){/*{{{*/
    //cleanup
    if(NULL == p){
        return EINVAL;
    }
    struct spx_mpool_cleanup *pup = NULL;
    while((NULL != (pup = p->cs))){
        pup->f(pup + sizeof(struct spx_mpool_cleanup));
        p->cs = pup->n;
        spx_free(pup);
    }

    //lager object
    struct spx_mpool_alone *pa = NULL;
    while((NULL != (pa = p->as))){
        p->as = pa->n;
        spx_free(pa);
    }

    //buf
    struct spx_mpool_buff *pb = p->hbuf;
    while(NULL != pb){
        memset((char *)(pb + sizeof(struct spx_mpool_buff)), 0,p->size);
        pb->last = pb + sizeof(struct spx_mpool_buff);
        pb = pb->n;
    }
    p->cbuf = p->hbuf;
    return 0;
}/*}}}*/

err_t spx_mpool_free(struct spx_mpool *const p,void **e){/*{{{*/
    size_t s = sizeof(**e);
    err_t rc = 0;
    if(s > p->limit){
        struct spx_mpool_alone *ptr =(struct spx_mpool_alone *)\
                                   *e - sizeof(struct spx_mpool_alone);
        if(NULL == ptr){
            rc = ENXIO;
            return rc;
        }
        if(NULL != ptr->p){
            ptr->p->n = ptr->n;
        } else {
            p->as = ptr->n;
        }
        spx_free(ptr);
    }
    return rc;
}/*}}}*/

err_t spx_mpool_destroy(struct spx_mpool **p){/*{{{*/
    //cleanup
    if(NULL == p){
        return EINVAL;
    }
    if(NULL == *p) {
        return 0;
    }
    struct spx_mpool_cleanup *pup = NULL;
    while((NULL != (pup = (*p)->cs))){
        pup->f(pup + sizeof(struct spx_mpool_cleanup));
        (*p)->cs = pup->n;
        spx_free(pup);
    }

    //lager object
    struct spx_mpool_alone *pa = NULL;
    while((NULL != (pa = (*p)->as))){
        (*p)->as = pa->n;
        spx_free(pa);
    }

    //buf
    struct spx_mpool_buff *pb = NULL;
    while(NULL != (pb = (*p)->hbuf)){
        (*p)->hbuf = pb->n;
        spx_free(pb);
    }

    spx_free(*p);
    return 0;
}/*}}}*/

