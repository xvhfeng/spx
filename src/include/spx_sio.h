#ifndef _SPX_SIO_H_
#define _SPX_SIO_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#include "spx_types.h"
#include "spx_sio_context.h"

err_t spx_sio_init(SpxLogDelegate *log,\
        size_t size,size_t stack_size,\
        SpxSioDelegate *sio_reader);

void spx_sio_reader(struct ev_loop *loop,ev_io *watcher,int revents);





#ifdef __cplusplus
}
#endif
#endif
