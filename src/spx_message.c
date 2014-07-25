#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "include/spx_alloc.h"
#include "include/spx_errno.h"
#include "include/spx_message.h"
#include "include/spx_types.h"
#include "include/spx_string.h"

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

struct spx_msg *spx_msg_new(const size_t len,err_t *err){/*{{{*/
    if (0 == len){
        *err = EINVAL;
        return NULL;
    }
    struct spx_msg *ctx = spx_alloc_alone(sizeof(*ctx),err);
    if(NULL == ctx){
        return NULL;
    }
    ctx->buf = spx_alloc_alone(len,err);
    if(NULL != ctx){
        goto r1;
    }
    ctx->last = ctx->buf;
    ctx->s = len;
    return ctx;
r1:
    SpxFree(ctx);
    return NULL;
}/*}}}*/

err_t spx_msg_free(struct spx_msg **ctx){/*{{{*/
    if(NULL != (*ctx)->buf){
        SpxFree((*ctx)->buf);
    }
    (*ctx)->last = NULL;
    (*ctx)->s = 0;
    SpxFree(*ctx);
    return 0;
}/*}}}*/

err_t spx_msg_seek(struct spx_msg *ctx,off_t offset,int whence){
    if(NULL == ctx || NULL == ctx->buf){
        return EINVAL;
    }
    switch(whence){
        case SpxMsgSeekSet :{
                                ctx->last = ctx->buf + offset;
                                break;
                            }
        case SpxMsgSeekCurrent:{
                                   ctx->last += offset;
                                   break;
                               }
        case SpxMsgSeekEnd:{
                               ctx->last = ctx->buf + (spx_msg_size(ctx) + offset);
                               break;
                           }
    }
    return 0;
}

void spx_msg_clear(struct spx_msg *ctx){
    SpxZeroLen(ctx->buf,ctx->s);
    ctx->last = ctx->buf;
    ctx->err = 0;
}


err_t spx_msg_pack_int( struct spx_msg *ctx,const int v){/*{{{*/
    return spx_msg_pack_i32(ctx,(i32_t) v);
}/*}}}*/

err_t spx_msg_pack_i8(struct spx_msg *ctx,const i8_t v){/*{{{*/
    if(NULL == ctx) return EINVAL;
    *(ctx->last) = (char) v;
    (ctx->last)++;
    return 0;
}/*}}}*/

err_t spx_msg_pack_i32( struct spx_msg *ctx,const i32_t v){/*{{{*/
    if(NULL == ctx) return EINVAL;
    spx_msg_i2b(ctx->last,v);
    return 0;
}/*}}}*/

err_t spx_msg_pack_i64( struct spx_msg *ctx,const i64_t v) {/*{{{*/
    if (NULL == ctx) return EINVAL;
    spx_msg_l2b(ctx->last,v);
    return 0;
}/*}}}*/
err_t spx_msg_pack_u8( struct spx_msg *ctx,const u8_t v){/*{{{*/
    if (NULL == ctx) return EINVAL;
    *(ctx->last) = (uchar_t) v;
    (ctx->last)++;
    return 0;
}/*}}}*/
err_t spx_msg_pack_u32( struct spx_msg *ctx,const u32_t v){/*{{{*/
    if(NULL == ctx) return EINVAL;
    spx_msg_i2b(ctx->last,(i32_t) v);
    return 0;
}/*}}}*/
err_t spx_msg_pack_u64( struct spx_msg *ctx,const u64_t v){/*{{{*/
    if(NULL == ctx) return EINVAL;
    spx_msg_l2b(ctx->last,(i64_t) v);
    return 0;
}/*}}}*/
err_t spx_msg_pack_double( struct spx_msg *ctx,const double v){/*{{{*/
    if (NULL == ctx) return EINVAL;
    union d2i n;
    SpxZero(n);
    n.v = v;
    spx_msg_l2b(ctx->last,n.i);
    return 0;
}/*}}}*/
err_t spx_msg_pack_float( struct spx_msg *ctx,const float v){/*{{{*/
    if (NULL == ctx) return EINVAL;
    union f2i n;
    SpxZero(n);
    n.v = v;
    spx_msg_i2b(ctx->last,n.i);
    return 0;
}/*}}}*/
err_t spx_msg_pack_true( struct spx_msg *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    *(ctx->last) = (uchar_t) true;
    (ctx->last)++;
    return 0;
}/*}}}*/
err_t spx_msg_pack_false( struct spx_msg *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    *(ctx->last) = (uchar_t) false;
    (ctx->last)++;
    return 0;
}/*}}}*/
err_t spx_msg_pack_string( struct spx_msg *ctx,string_t s){/*{{{*/
    if (NULL == ctx) return EINVAL;
    ctx->last = SpxMemcpy(ctx->last,s,spx_string_len(s));
    return 0;
}/*}}}*/
err_t spx_msg_pack_fixed_string( struct spx_msg *ctx,string_t s,size_t len){/*{{{*/
    if (NULL == ctx) return EINVAL;
    ctx->last = SpxMemcpy(ctx->last,s,spx_string_len(s));
    ctx->last += len - spx_string_len(s);
    return 0;
}
err_t spx_msg_pack_ubytes( struct spx_msg *ctx,const ubyte_t *b,const size_t len){/*{{{*/
    if (NULL == ctx) return EINVAL;
    ctx->last = SpxMemcpy(ctx->last,b,len);
    return 0;
}/*}}}*/
err_t spx_msg_pack_bytes( struct spx_msg *ctx,const byte_t *b,const size_t len){/*{{{*/
    if (NULL == ctx) return EINVAL;
    ctx->last =  SpxMemcpy(ctx->last,b,len);
    return 0;
}/*}}}*/

int spx_msg_unpack_int( struct spx_msg *ctx){/*{{{*/
    return spx_msg_unpack_i32(ctx);
}/*}}}*/
i8_t spx_msg_unpack_i8( struct spx_msg *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    i8_t n = 0;
    n = (i8_t) *(ctx->last);
    ctx->last++;
    return n;
}/*}}}*/
i32_t spx_msg_unpack_i32( struct spx_msg *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    return  (i32_t) spx_msg_b2i(ctx->last);
}/*}}}*/
i64_t spx_msg_unpack_i64( struct spx_msg *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    return (i64_t) spx_msg_b2l(ctx->last);
}/*}}}*/
u8_t spx_msg_unpack_u8( struct spx_msg *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    i8_t n = 0;
    n = (i8_t) *(ctx->last);
    ctx->last++;
    return n;
}/*}}}*/
u32_t spx_msg_unpack_u32( struct spx_msg *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    return (u32_t) spx_msg_b2i(ctx->last);
}/*}}}*/
u64_t spx_msg_unpack_u64( struct spx_msg *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    return (u64_t) spx_msg_b2l(ctx->last);
}/*}}}*/
double spx_msg_unpack_double( struct spx_msg *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    union d2i n;
    SpxZero(n);
    n.i = (u64_t) spx_msg_b2l(ctx->last);
    return n.v;
}/*}}}*/
float spx_msg_unpack_float( struct spx_msg *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    union f2i n;
    SpxZero(n);
    n.i = (u32_t) spx_msg_b2i(ctx->last);
    return n.v;
}/*}}}*/
bool_t spx_msg_unpack_bool( struct spx_msg *ctx){/*{{{*/
    if (NULL == ctx) return EINVAL;
    u8_t n = spx_msg_unpack_u8(ctx);
    return (bool_t) n;
}/*}}}*/
string_t spx_msg_unpack_string( struct spx_msg *ctx,\
        const size_t len,err_t *err){/*{{{*/
    string_t p = NULL;
    p = spx_string_newlen(ctx->last,len,err);
    ctx->last += len;
    return p;
}/*}}}*/
ubyte_t *spx_msg_unpack_ubytes( struct spx_msg *ctx,const size_t len,err_t *err){/*{{{*/
    return (ubyte_t *)spx_msg_unpack_string(ctx,len, err);
}/*}}}*/
byte_t *spx_msg_unpack_bytes( struct spx_msg *ctx,const size_t len,err_t *err){/*{{{*/
    return (byte_t *) spx_msg_unpack_string(ctx,len, err);
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

