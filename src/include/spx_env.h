/*
 * =====================================================================================
 *
 *       Filename:  spx_env.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/06/30 14时34分03秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef _SPX_ENV_H_
#define _SPX_ENV_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "spx_types.h"

void spx_env_daemon();
err_t spx_set_group_and_user(SpxLogDelegate *log,string_t gname,string_t uname);

#ifdef __cplusplus
}
#endif
#endif
