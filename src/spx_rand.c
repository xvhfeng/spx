/*
 * =====================================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  spx_rand.c
 *        Created:  2014/07/15 11时50分25秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "spx_types.h"
#include "spx_time.h"

#define PHI 161803398
u32_t spx_random(u32_t idx){
    pthread_t tid = pthread_self();
    pid_t pid = getpid();
    time_t  time = spx_now();
    u32_t seed = (u32_t) tid & pid & idx & time & PHI;
     srand(seed);
    u32_t r = rand();
    return r;
}

u32_t spx_srandom(){
    return spx_random(1);
}