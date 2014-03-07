#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "headers/spx_alloc.h"
#include "headers/spx_errno.h"
#include "headers/spx_message.h"
#include "headers/spx_types.h"
#include "headers/spx_string.h"

 union d2i{
    double v;
    uint64_t i;
};

union f2i{
    float v;
    uint32_t i;
};

spx_private void spx_msg_i2b(uchar_t *b,const i32_t n);
spx_private i32_t spx_msg_b2i(uchar_t *b);
spx_private void spx_msg_l2b(uchar_t *b,const i64_t n) ;
spx_private i64_t spx_msg_b2l(uchar_t *b);

err_t spx_msg_init(struct spx_msg_ctx **ctx,const size_t len){/*{{{*/
    if (0 == len){
        return EINVAL;
    }
    err_t err = 0;
    if(0 != (err = spx_alloc_alone(sizeof(struct spx_msg_ctx),(void **) ctx))){
        return err;
    }
    if(0 != (err = spx_alloc_alone(len,(void **) &((*ctx)->b)))){
        goto r1;
    }
    (*ctx)->s = len;
    return err;
r1:
    spx_free(*ctx);
    return err;
}/*}}}*/
err_t spx_msgpack_destory(struct spx_msg_ctx **ctx){/*{{{*/
    if(NULL != (*ctx)->b){
        spx_free((*ctx)->b);
    }
    (*ctx)->e = NULL;
    (*ctx)->s = 0;
    spx_free(*ctx);
    return 0;
}/*}}}*/
err_t spx_msgpack_int( struct spx_msg_ctx *ctx,const int v){/*{{{*/
    return spx_msgpack_i32(ctx,(i32_t) v);
}/*}}}*/
err_t spx_msgpack_i8(struct spx_msg_ctx *ctx,const i8_t v){/*{{{*/
    if(NULL == ctx) return EINVAL;
    *(ctx->e) = (char) v;
    (ctx->e)++;
    return 0;
}/*}}}*/
err_t spx_msgpack_i32( struct spx_msg_ctx *ctx,const i32_t v){/*{{{*/
    if(NULL == ctx) return EINVAL;
    spx_msg_i2b(ctx->e,v);
    return 0;
}/*}}}*/
err_t spx_msgpack_i64( struct spx_msg_ctx *ctx,const i64_t v) {/*{{{*/
    if (NULL == ctx) return EINVAL;
    spx_msg_l2b(ctx->e,v);
    return 0;
}/*}}}*/
err_t spx_msgpack_u8( struct spx_msg_ctx *ctx,const u8_t v){/*{{{*/
    if (NULL == ctx) return EINVAL;
    *(ctx->e) = (uchar_t) v;
    (ctx->e)++;
    return 0;
}/*}}}*/
err_t spx_msgpack_u32( struct spx_msg_ctx *ctx,const u32_t v){/*{{{*/
    if(NULL == ctx) return EINVAL;
    spx_msg_i2b(ctx->e,(i32_t) v);
    return 0;
}/*}}}*/
err_t spx_msgpack_u64( struct spx_msg_ctx *ctx,const u64_t v){/*{{{*/
    if(NULL == ctx) return EINVAL;
    spx_msg_l2b(ctx->e,(i64_t) v);
    return 0;
}/*}}}*/
err_t spx_msgpack_double( struct spx_msg_ctx *ctx,const double v){/*{{{*/
    if (NULL == ctx) return EINVAL;
    union d2i n;
    spx_bzero(&n);
    n.v = v;
    spx_msg_l2b(ctx->e,n.i);
    return 0;
}/*}}}*/
err_t spx_msgpack_float( struct spx_msg_ctx *ctx,const float v){/*{{{*/
    if (NULL == ctx) return EINVAL;
    union f2i n;
    spx_bzero(&n);
    n.v = v;
    spx_msg_i2b(ctx->e,n.i);
    return 0;
}/*}}}*/
err_t spx_msgpack_true( struct spx_msg_ctx *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    *(ctx->e) = (uchar_t) true;
    (ctx->e)++;
    return 0;
}/*}}}*/
err_t spx_msgpack_false( struct spx_msg_ctx *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    *(ctx->e) = (uchar_t) false;
    (ctx->e)++;
    return 0;
}/*}}}*/
err_t spx_msgpack_string( struct spx_msg_ctx *ctx,const uchar_t *s,const size_t len){/*{{{*/
    if (NULL == ctx) return EINVAL;
    spx_memcpy(ctx->e,s,len);
    return 0;
}/*}}}*/
err_t spx_msgpack_ubytes( struct spx_msg_ctx *ctx,const ubyte_t *b,const size_t len){/*{{{*/
    if (NULL == ctx) return EINVAL;
    spx_memcpy(ctx->e,b,len);
    return 0;
}/*}}}*/
err_t spx_msgpack_bytes( struct spx_msg_ctx *ctx,const byte_t *b,const size_t len){/*{{{*/
    if (NULL == ctx) return EINVAL;
    spx_memcpy(ctx->e,b,len);
    return 0;
}/*}}}*/

int spx_msgunpack_int( struct spx_msg_ctx *ctx){/*{{{*/
    return spx_msgunpack_i32(ctx);
}/*}}}*/
i8_t spx_msgunpack_i8( struct spx_msg_ctx *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    i8_t n = 0;
    n = (i8_t) *(ctx->e);
    ctx->e++;
    return n;
}/*}}}*/
i32_t spx_msgunpack_i32( struct spx_msg_ctx *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    return  (i32_t) spx_msg_b2i(ctx->e);
}/*}}}*/
i64_t spx_msgunpack_i64( struct spx_msg_ctx *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    return (i64_t) spx_msg_b2l(ctx->e);
}/*}}}*/
u8_t spx_msgunpack_u8( struct spx_msg_ctx *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    i8_t n = 0;
    n = (i8_t) *(ctx->e);
    ctx->e++;
    return n;
}/*}}}*/
u32_t spx_msgunpack_u32( struct spx_msg_ctx *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    return (u32_t) spx_msg_b2i(ctx->e);
}/*}}}*/
u64_t spx_msgunpack_u64( struct spx_msg_ctx *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    return (u64_t) spx_msg_b2l(ctx->e);
}/*}}}*/
double spx_msgunpack_double( struct spx_msg_ctx *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    union d2i n;
    spx_bzero(&n);
    n.i = (u64_t) spx_msg_b2l(ctx->e);
    return n.v;
}/*}}}*/
float spx_msgunpack_float( struct spx_msg_ctx *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    union f2i n;
    spx_bzero(&n);
    n.i = (u32_t) spx_msg_b2i(ctx->e);
    return n.v;
}/*}}}*/
bool_t spx_msgunpack_bool( struct spx_msg_ctx *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    u8_t n = spx_msgunpack_u8(ctx);
    return (bool_t) n;
}/*}}}*/
uchar_t *spx_msgunpack_string( struct spx_msg_ctx *ctx,const size_t len,const bool_t iscp){/*{{{*/
    uchar_t *p = NULL;
    if(iscp){
        p =ctx->e;
    } else {
        spx_alloc_string(len,&p);
        memcpy(p,ctx->e,len);
    }
    ctx->e += len;
    return (uchar_t *)p;
}/*}}}*/
ubyte_t *spx_msgunpack_ubytes( struct spx_msg_ctx *ctx,const size_t len,const bool_t iscp){/*{{{*/
    return (ubyte_t *)spx_msgunpack_string(ctx,len, iscp);
}/*}}}*/
byte_t *spx_msgunpack_bytes( struct spx_msg_ctx *ctx,const size_t len,const bool_t iscp){/*{{{*/
    return (byte_t *) spx_msgunpack_string(ctx,len, iscp);
}/*}}}*/

spx_private void spx_msg_i2b(uchar_t *b,const i32_t n){/*{{{*/
	*b++ = (n >> 24) & 0xFF;
	*b++ = (n >> 16) & 0xFF;
	*b++ = (n >> 8) & 0xFF;
	*b++ = n & 0xFF;
}/*}}}*/
spx_private i32_t spx_msg_b2i(uchar_t *b){/*{{{*/
    i32_t n =  (i32_t ) ((((i32_t) (*b)) << 24)
            | (((i32_t) (*(b + 1))) << 16)
            | (((i32_t) (*(b+2))) << 8)
            | ((i32_t) (*(b+3))));
    b += sizeof(i32_t);
    return n;
}/*}}}*/
spx_private void spx_msg_l2b(uchar_t *b,const i64_t n){/*{{{*/
	*b++ = (n >> 56) & 0xFF;
	*b++ = (n >> 48) & 0xFF;
	*b++ = (n >> 40) & 0xFF;
	*b++ = (n >> 32) & 0xFF;
	*b++ = (n >> 24) & 0xFF;
	*b++ = (n >> 16) & 0xFF;
	*b++ = (n >> 8) & 0xFF;
	*b++ = n & 0xFF;
}/*}}}*/
spx_private i64_t spx_msg_b2l(uchar_t *b){/*{{{*/
    i64_t n =  (((i64_t) (*b)) << 56) 
        | (((i64_t) (*(b+1))) << 48)
        | (((i64_t) (*(b + 2))) << 40) 
        | (((i64_t) (*(b + 3))) << 32)
        | (((i64_t) (*(b + 4))) << 24)
        | (((i64_t) (*(b + 5))) << 16)
        | (((i64_t) (*(b + 6))) << 8) 
        | ((i64_t) (*(b + 7)));
    b += sizeof(i64_t);
    return n;
}/*}}}*/

