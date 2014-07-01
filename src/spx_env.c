/*
 * =====================================================================================
 *
 *       Filename:  spx_env.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/06/09 14时28分33秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>

#include "include/spx_types.h"
#include "include/spx_string.h"
#include "include/spx_defs.h"


void spx_env_daemon() {
        pid_t pid;

        if ((pid = fork()) != 0) {
            exit(0);
        }

        setsid();

        if ((pid = fork()) != 0) {
            exit(0);
        }
    return;
}

err_t spx_set_group_and_user(SpxLogDelegate *log,string_t gname,string_t uname){
    err_t err = 0;
    SpxErrReset;
    if(!SpxStringIsNullOrEmpty(gname)){
        struct group *g = getgrnam(gname);
        if(NULL == g){
            SpxLogFmt1(log,SpxLogError,\
                    "not found the group:%s.",gname);
            err = 0 == errno ? EACCES : errno;
            return err;
        }
        if(0 != setegid(g->gr_gid)){
            err = 0 == errno ? EPERM : errno;
            SpxLogFmt2(log,SpxLogError,err,\
                    "set the process group id:%d by gname:%s is fail.",
                    g->gr_gid,gname);
            return err;
        }
    }
    if(!SpxStringIsNullOrEmpty(uname)){
        struct passwd *u = getpwnam(uname);
        if(NULL == u){
            SpxLogFmt1(log,SpxLogError,\
                    "not foung the user:%s.",uname);
            err = 0 == errno ? EACCES : errno;
        }
        if (0 != seteuid(u->pw_uid)) {
            err = 0 == errno ? EPERM : errno;
            SpxLogFmt2(log,SpxLogError,err,\
                    "set the process uid:%d by uname:%s is fail.",\
                    u->pw_uid,uname);
            return err;
        }
        return err;
    }
    return err;
}
