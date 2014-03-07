#ifndef SPX_PATH_H
#define SPX_PATH_H
#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>

#include "spx_types.h"

err_t spx_is_dir(const string_t path,bool_t *isdir);
err_t spx_mkdir(const log_t log,const string_t path,const mode_t mode);
err_t spx_fullname(const string_t path,const string_t filename,\
        string_t fullname);

#ifdef __splusplus
}
#endif
#endif
