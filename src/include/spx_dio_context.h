/*
 * =====================================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  spx_dio_context.h
 *        Created:  2014/07/24 17时53分07秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 * =====================================================================================
 */
#ifndef _SPX_DIO_CONTEXT_H_
#define _SPX_DIO_CONTEXT_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>
#include <ev.h>

#include "spx_types.h"
#include "spx_defs.h"
#include "spx_message.h"
#include "spx_properties.h"


    //play a trick
    //and look at me how to implemant sub-calss
    //it must be putted into sub-class with begin.
    //if you do not understand,please no modifition.
#define SpxDioMembers \
    int fd

    struct spx_dio_file{
        SpxDioMembers;
    };

    typedef void (SpxDioDelegate)(struct ev_loop *loop,ev_io *watcher,int revents);
    typedef void (SpxDioProcessDelegate)(int fd,void *arg);

    struct spx_dio_context{
        ev_io watcher;
        size_t idx;
        err_t err;
        bool_t noblacking;
        SpxDioDelegate *dio_handler;
        SpxDioProcessDelegate *dio_process_handler;
        SpxLogDelegate *log;
        struct spx_nio_context *nio_context;
        int events;
        void *arg;//this member must be sub-class of spx_dio_file
    };

    struct spx_dio_context_node{
        bool_t noblacking;
        SpxDioDelegate *dio_handler;
        SpxDioProcessDelegate *dio_process_handler;
        SpxLogDelegate *log;
        int events;
        void *arg;//this member must be sub-class of spx_dio_file
    };

    struct spx_dio_context_pool {
        SpxLogDelegate *log;
        struct spx_fixed_vector *pool;
    };

    extern struct spx_dio_context_pool *g_spx_dio_context_pool;

    void *spx_dio_context_new(size_t idx,void *arg,err_t *err);
    err_t spx_dio_context_free(void **arg);
    void spx_dio_context_clear(struct spx_dio_context *dio_context);

struct spx_dio_context_pool *spx_dio_context_pool_new(\
        SpxLogDelegate *log,\
        size_t size,\
        bool_t noblacking,\
        SpxDioDelegate *dio_handler,\
        SpxDioProcessDelegate *dio_process_handler,\
        int events,\
        void *arg,\
        err_t *err);

    struct spx_dio_context *spx_dio_context_pool_pop(\
            struct spx_dio_context_pool *pool,\
            err_t *err);

    err_t spx_dio_context_pool_push(\
            struct spx_dio_context_pool *pool,\
            struct spx_dio_context *dio_context);

    err_t spx_dio_context_pool_free(\
            struct spx_dio_context_pool **pool);




#ifdef __cplusplus
}
#endif
#endif
