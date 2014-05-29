#ifndef SPX_MESSAGE_H
#define SPX_MESSAGE_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <sys/types.h>

#include "spx_types.h"


struct spx_msg_ctx{
    uchar_t *buf;
    uchar_t *last;
    size_t s;
    err_t err;
};

err_t spx_msg_init(struct spx_msg_ctx **ctx,const size_t len);
err_t spx_msg_destory(struct spx_msg_ctx **ctx);

err_t spx_msgpack_int( struct spx_msg_ctx *ctx,const int v);
err_t spx_msgpack_i8(struct spx_msg_ctx *ctx,const i8_t v);
err_t spx_msgpack_i32( struct spx_msg_ctx *ctx,const i32_t v);
err_t spx_msgpack_i64( struct spx_msg_ctx *ctx,const i64_t v);
err_t spx_msgpack_u8( struct spx_msg_ctx *ctx,const u8_t v);
err_t spx_msgpack_u32( struct spx_msg_ctx *ctx,const u32_t v);
err_t spx_msgpack_u64( struct spx_msg_ctx *ctx,const u64_t v);
err_t spx_msgpack_double( struct spx_msg_ctx *ctx,const double v);
err_t spx_msgpack_float( struct spx_msg_ctx *ctx,const float v);
err_t spx_msgpack_true( struct spx_msg_ctx *ctx);
err_t spx_msgpack_false( struct spx_msg_ctx *ctx);
err_t spx_msgpack_string( struct spx_msg_ctx *ctx,const uchar_t *s,const size_t len);
err_t spx_msgpack_ubytes( struct spx_msg_ctx *ctx,const ubyte_t *b,const size_t len);
err_t spx_msgpack_bytes( struct spx_msg_ctx *ctx,const byte_t *b,const size_t len);

int spx_msgunpack_int( struct spx_msg_ctx *ctx);
i8_t spx_msgunpack_i8( struct spx_msg_ctx *ctx);
i32_t spx_msgunpack_i32( struct spx_msg_ctx *ctx);
i64_t spx_msgunpack_i64( struct spx_msg_ctx *ctx);
u8_t spx_msgunpack_u8( struct spx_msg_ctx *ctx);
u32_t spx_msgunpack_u32( struct spx_msg_ctx *ctx);
u64_t spx_msgunpack_u64( struct spx_msg_ctx *ctx);
double spx_msgunpack_double( struct spx_msg_ctx *ctx);
float spx_msgunpack_float( struct spx_msg_ctx *ctx);
bool_t spx_msgunpack_bool( struct spx_msg_ctx *ctx);
uchar_t *spx_msgunpack_string( struct spx_msg_ctx *ctx,const size_t len,const bool_t iscp);
ubyte_t *spx_msgunpack_ubytes( struct spx_msg_ctx *ctx,const size_t len,const bool_t iscp);
byte_t *spx_msgunpack_bytes( struct spx_msg_ctx *ctx,const size_t len,const bool_t iscp);

#ifdef __cplusplus
}
#endif
#endif
