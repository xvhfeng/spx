#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "headers/spx_types.h"
#include "headers/spx_defs.h"
#include "headers/spx_alloc.h"

err_t spx_malloc(const size_t s,void **p){
    err_t rc = 0;
    if(0 == s){
        return EINVAL;
    }
    *p = (void *) malloc(s);
    if(NULL == *p){
        rc = 0 == errno ? ENOMEM : errno;
    }
    return rc;
}

err_t spx_alloc(const size_t numbs,const size_t s,void **p){
    err_t rc = 0;
    if(0 == s || 0 == numbs){
        return EINVAL;
    }
    *p = (void *) calloc(numbs,s);
    if(NULL == *p){
        rc = 0 == errno ? ENOMEM : errno;
    }
    return rc;
}
err_t spx_alloc_string(const size_t s,string_t *p){
    if(0 == s){
        return EINVAL;
    }
    return spx_alloc(s + 1,sizeof(char),(void **)p);

}
err_t spx_alloc_alone(const size_t s,void **p){
    if(0 == s){
        return EINVAL;
    }
    return spx_alloc(1,s,p);
}

err_t spx_alloc_bstring(const size_t s,bstring_t **p){
    err_t rc = 0;
    if(0 != (rc = spx_alloc_alone(sizeof(bstring_t) + s + 1 , (void **)p))){
        return rc;
    }
    (*p)->s = s;
    return rc;
}

err_t spx_memalign_alloc(const size_t size,void **p) {
    err_t rc = 0;
    if(0 != (rc = posix_memalign(p, SPX_ALIGN_SIZE, size))){
        p = NULL;
        return rc;
    }
    return rc;
}
