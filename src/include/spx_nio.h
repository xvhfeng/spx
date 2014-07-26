/*
 * =====================================================================================
 *
 *       Filename:  spx_nio.h
 *
 *    Description:  ,e
 *
 *        Version:  1.0
 *        Created:  2014/06/09 17时43分29秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef _SPX_NIO_H_
#define _SPX_NIO_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <ev.h>

#include "spx_types.h"
#include "spx_job.h"

err_t  spx_nio_regedit_reader(struct ev_loop *loop,int fd,struct spx_job_context *jcontext);
err_t  spx_nio_regedit_writer(struct ev_loop *loop,int fd,struct spx_job_context *jcontext);

void spx_nio_reader(struct ev_loop *loop,ev_io *watcher,int revents);
void spx_nio_writer(struct ev_loop *loop,ev_io *watcher,int revents);

void spx_nio_reader_body_handler(int fd,struct spx_job_context *jcontext);
void spx_nio_writer_body_handler(int fd,struct spx_job_context *jcontext);




#ifdef __cplusplus
}
#endif
#endif
