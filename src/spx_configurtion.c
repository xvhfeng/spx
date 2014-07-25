/*
 * =====================================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  spx_configurtion.c
 *        Created:  2014/07/16 16时37分49秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <stdio.h>

#include "include/spx_types.h"
#include "include/spx_defs.h"
#include "include/spx_string.h"
#include "include/spx_configurtion.h"

void *spx_configurtion_parser(SpxLogDelegate *log,\
        SpxConfigurtionParserBeforeDelegate *before,\
        SpxConfigurtionParserAfterDelegate *after,\
        string_t filename,\
            SpxConfigurtionLineParserDelegate *lineparser,\
        err_t *err) {
    SpxErrReset;
    void *config = NULL;
    if(NULL != before){
        config = before(log,err);
        if(0 != *err){
            return NULL;
        }
    }
    FILE *f = fopen(filename,"r");
    if(NULL == f){
        *err = errno;
        return  config;
    }

    string_t line = spx_string_newlen(NULL,SpxStringRealSize(SpxLineSize),err);
    if(NULL == line){
        goto r1;
    }

    while(NULL != fgets(line,SpxLineSize,f)){
        spx_string_updatelen(line);
        spx_string_trim(line," ");
        if(!SpxStringBeginWith(line,'#')){
            lineparser(line,config,err);
            if(0 != err){
                goto r1;
            }
        }
        spx_string_clear(line);
    }
    if(NULL != after){
        *err = after(config);
    }

r1:
    fclose(f);
    return config;
}