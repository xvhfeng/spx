/*
 * */
#ifndef SPX_ALLOC_H
#define SPX_ALLOC_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#include "spx_types.h"


#if SYSBIT == 32
    #define SPX_ALIGN_SIZE 4
#elif SYSBIT == 64
    #define SPX_ALIGN_SIZE 8
#else
    #define SPX_ALIGN_SIZE 8
#endif

#define SpxFree(ptr) do\
    {\
        if(NULL != ptr)\
        {\
            free(ptr);\
            ptr = NULL; \
        }\
    }while(false)

    err_t spx_malloc(const size_t s,void **p);
    err_t spx_alloc(const size_t numbs,const size_t s,void **p);
    err_t spx_alloc_string(const size_t s,string_t *p);
    err_t spx_alloc_bstring(const size_t s,bstring_t **p);
    err_t spx_alloc_alone(const size_t s,void **p);
    err_t spx_memalign_alloc(const size_t size,void **p);


#ifdef __cplusplus
}
#endif
#endif

