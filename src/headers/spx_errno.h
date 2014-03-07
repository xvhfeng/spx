
#ifndef SPX_ERRNO_H
#define SPX_ERRNO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>

#include "spx_types.h"

#define SpxSuccess 512
#define ENODLMT 513

const char *spx_err_info_list[] = {
    "Success.",
    "No delimiter."
};

const char *spx_strerror(err_t err);

#ifdef __cplusplus
}
#endif
#endif
