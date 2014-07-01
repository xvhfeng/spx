#ifndef SPX_MESSAGE_H
#define SPX_MESSAGE_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <sys/types.h>

#include "spx_types.h"


#define SpxMsgSeekSet 0
#define SpxMsgSeekCurrent 1
#define SpxMsgSeekEnd 2

struct spx_msg{
    uchar_t *buf;
    uchar_t *last;
    size_t s;
    err_t err;
};

struct spx_msg *spx_msg_new(const size_t len,err_t *err);
err_t spx_msg_free(struct spx_msg **ctx);
err_t spx_msg_seek(struct spx_msg *ctx,off_t offset,int whence);

err_t spx_msg_pack_int( struct spx_msg *ctx,const int v);
err_t spx_msg_pack_i8(struct spx_msg *ctx,const i8_t v);
err_t spx_msg_pack_i32( struct spx_msg *ctx,const i32_t v);
err_t spx_msg_pack_i64( struct spx_msg *ctx,const i64_t v);
err_t spx_msg_pack_u8( struct spx_msg *ctx,const u8_t v);
err_t spx_msg_pack_u32( struct spx_msg *ctx,const u32_t v);
err_t spx_msg_pack_u64( struct spx_msg *ctx,const u64_t v);
err_t spx_msg_pack_double( struct spx_msg *ctx,const double v);
err_t spx_msg_pack_float( struct spx_msg *ctx,const float v);
err_t spx_msg_pack_true( struct spx_msg *ctx);
err_t spx_msg_pack_false( struct spx_msg *ctx);
err_t spx_msg_pack_string( struct spx_msg *ctx,string_t s);
err_t spx_msg_pack_fixed_string( struct spx_msg *ctx,string_t s,size_t len);
err_t spx_msg_pack_ubytes( struct spx_msg *ctx,const ubyte_t *b,const size_t len);
err_t spx_msg_pack_bytes( struct spx_msg *ctx,const byte_t *b,const size_t len);

int spx_msg_unpack_int( struct spx_msg *ctx);
i8_t spx_msg_unpack_i8( struct spx_msg *ctx);
i32_t spx_msg_unpack_i32( struct spx_msg *ctx);
i64_t spx_msg_unpack_i64( struct spx_msg *ctx);
u8_t spx_msg_unpack_u8( struct spx_msg *ctx);
u32_t spx_msg_unpack_u32( struct spx_msg *ctx);
u64_t spx_msg_unpack_u64( struct spx_msg *ctx);
double spx_msg_unpack_double( struct spx_msg *ctx);
float spx_msg_unpack_float( struct spx_msg *ctx);
bool_t spx_msg_unpack_bool( struct spx_msg *ctx);
string_t spx_msg_unpack_string( struct spx_msg *ctx,const size_t len,err_t *err);
ubyte_t *spx_msg_unpack_ubytes( struct spx_msg *ctx,const size_t len,err_t *err);
byte_t *spx_msg_unpack_bytes( struct spx_msg *ctx,const size_t len,err_t *err);

spx_private spx_inline size_t spx_msg_size(struct spx_msg *ctx){
    return ctx->s;
}

spx_private spx_inline size_t spx_msg_len(struct spx_msg *ctx){
    return ctx->last - ctx->buf;
}
#ifdef __cplusplus
}
#endif
#endif