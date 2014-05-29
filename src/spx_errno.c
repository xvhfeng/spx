
#include <string.h>
#include <errno.h>
#include <math.h>

#include "headers/spx_types.h"
#include "headers/spx_errno.h"

const char *spx_err_info_list[] = {
    "Success.",
    "No delimiter."
};

const char *spx_strerror(err_t err){
    int e = 0;
    e = 0 > err ? (0 - err) : err;
    if(err >= SpxSuccess) {
        e = e - SpxSuccess;
        return spx_err_info_list[e];
    } else {
        return strerror(e);
    }
    return NULL;
}
