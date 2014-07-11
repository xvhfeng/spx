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

#include <stdlib.h>
#include <stdio.h>

#include "spx_types.h"
#include "spx_message.h"

#define SpxWriteOpen(filename,is_clear) open(filename,is_clear ? O_RDWR|O_CREAT|O_TRUNC : O_RDWR|O_CREAT|O_APPEND,0744)
#define SpxReadOpen(filename) open(filename,O_RDONLY)
#define SpxFWriteOpen(filename,is_clear) fopen(filename,is_clear ? "w" : "a")
#define SpxFReadOpen(filename) fopen(filename,"r")

#define SpxFileExist(filename) ( 0 == access(filename,F_OK))
#define SpxFileReadable(filename) (0 == access(filename,R_OK))
#define SpxFileWritable(filename) (0 == access(filename,W_OK))
#define SpxFileExecutable(filename) (0 == access(filename,X_OK))


err_t spx_read(int fd,byte_t *buf,const size_t size,size_t *len);
err_t spx_write(int fd,byte_t *buf,const size_t size,size_t *len);
err_t spx_read_nb(int fd,byte_t *buf,const size_t size,size_t *len);
err_t spx_write_nb(int fd,byte_t *buf,const size_t size,size_t *len);
err_t spx_read_to_msg(int fd,struct spx_msg *ctx,const size_t size,size_t *len);
err_t spx_read_to_msg_nb(int fd,struct spx_msg *ctx,const size_t size,size_t *len);
err_t spx_write_from_msg(int fd,struct spx_msg *ctx,const size_t size,size_t *len);
err_t spx_write_from_msg_nb(int fd,struct spx_msg *ctx,const size_t size,size_t *len);
err_t spx_fwrite_string(FILE *fp,string_t s,size_t size,size_t *len);

err_t spx_set_nb(int fd);
#ifdef __cplusplus
}
#endif
#endif
