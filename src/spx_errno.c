
#include <string.h>
#include <errno.h>
#include <math.h>

#include "spx_types.h"
#include "spx_errno.h"

const char *c_strerror(err_t err){
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
