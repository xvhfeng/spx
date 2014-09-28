#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "spx_types.h"
#include "spx_string.h"
#include "spx_defs.h"
#include "spx_socket.h"
#include "spx_notifier_module.h"
#include "spx_module.h"
#include "spx_job.h"
#include "spx_socket_accept.h"
#include "spx_time.h"

void spx_socket_accept_nb(SpxLogDelegate *log,int fd){
    err_t err = 0;
    while(true){
        struct sockaddr_in client_addr;
        unsigned int socket_len = 0;
        int client_sock = 0;
        socket_len = sizeof(struct sockaddr_in);
        client_sock = accept(fd, (struct sockaddr *) &client_addr,
                &socket_len);
        if (0 > client_sock) {
            if (EWOULDBLOCK == errno || EAGAIN == errno) {
                continue;
            }
            continue;
        }

        if (0 == client_sock) {
            continue;
        }

        size_t idx = client_sock % g_spx_notifier_module->threadpool->size;
        struct spx_job_context *jcontext =  spx_job_pool_pop(g_spx_job_pool,&err);
        if(NULL == jcontext){
            SpxClose(client_sock);
            SpxLog1(log,SpxLogError,\
                    "pop nio context is fail.");
            return;
        }

        SpxLogFmt1(log,SpxLogDebug,"recv socket conntect.wakeup notifier module idx:%d.jc idx:%d."
                ,idx,jcontext->idx);

        jcontext->fd = client_sock;

        struct spx_thread_context *tc = spx_get_thread(g_spx_notifier_module,idx);
        jcontext->tc = tc;
        spx_module_dispatch(tc,spx_notifier_module_wakeup_handler, jcontext);
    }
}

