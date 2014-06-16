/*
 * =====================================================================================
 *
 *       Filename:  spx_io.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/06/12 11时47分07秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef _SPX_IO_H_
#define _SPX_IO_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "spx_types.h"
#include "spx_message.h"

err_t spx_read(int fd,byte_t *buf,const size_t size,size_t *len);
err_t spx_write(int fd,byte_t *buf,const size_t size,size_t *len);
err_t spx_read_to_msg(int fd,struct spx_msg *ctx,const size_t size,size_t *len);
err_t spx_read_to_msg_nb(int fd,struct spx_msg *ctx,const size_t size,size_t *len);
err_t spx_write_from_msg(int fd,struct spx_msg *ctx,const size_t size,size_t *len);
err_t spx_write_from_msg_nb(int fd,struct spx_msg *ctx,const size_t size,size_t *len);

#ifdef __cplusplus
}
#endif
#endif
