/*************************************************************
 *                     _ooOoo_
 *                    o8888888o
 *                    88" . "88
 *                    (| -_- |)
 *                    O\  =  /O
 *                 ____/`---'\____
 *               .'  \\|     |//  `.
 *              /  \\|||  :  |||//  \
 *             /  _||||| -:- |||||-  \
 *             |   | \\\  -  /// |   |
 *             | \_|  ''\---/''  |   |
 *             \  .-\__  `-`  ___/-. /
 *           ___`. .'  /--.--\  `. . __
 *        ."" '<  `.___\_<|>_/___.'  >'"".
 *       | | :  `- \`.;`\ _ /`;.`/ - ` : | |
 *       \  \ `-.   \_ __\ /__ _/   .-` /  /
 *  ======`-.____`-.___\_____/___.-`____.-'======
 *                     `=---='
 *  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *           佛祖保佑       永无BUG
 *
 * ==========================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  SpxAlloc.c
 *        Created:  2015年01月09日 22时11分19秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "SpxTypes.h"
#include "SpxAlloc.h"
#include "SpxError.h"
#include "SpxLimits.h"

var spxAlloc(const size_t s,err_t *err){
    if(0 == s){
        *err = EINVAL;
        return NULL;
    }
    var p = calloc(s,sizeof(char));
    if(NULL == p){
        *err = 0 == errno ? SpxEAlloc : errno;
    }
    return p;
}

var spxAllocNumbs(const size_t numbs,const size_t s,err_t *err){
    if(0 == s || 0 == numbs){
        *err = EINVAL;
        return NULL;
    }
    var p = calloc(numbs,s);
    if(NULL == p){
        *err = 0 == errno ? SpxEAlloc : errno;
        return NULL;
    }
    return p;
}

var spxAllocAlone(const size_t s,err_t *err){
    if(0 == s){
        *err = EINVAL;
        return NULL;
    }
    return spxAlloc(s,err);
}

var spxAllocAlign(const size_t size,err_t *err) {
    var p = NULL;
    if(0 != (*err = posix_memalign(&p, SpxAlignSize, size))){
        return NULL;
    }
    if(NULL == p){
        *err = 0 == errno ? SpxEAlloc : errno;
    }
    return p;
}

var spxAllocPointer(const size_t numbs,err_t *err){
    var p = calloc(numbs,SpxPSize);
    if(NULL == p){
        *err = 0 == errno ? SpxEAlloc : errno;
    }
    return p;
}

var spxRealloc(var p,const size_t size,err_t *err){
    var ptr = realloc(p,size);
    if(NULL == ptr){
        *err = 0 == errno ? SpxEAlloc : errno;
        return NULL;
    }
    return ptr;
}
