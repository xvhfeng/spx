/*
 * =====================================================================================
 *
 *       Filename:  skiplist_test.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/05/26 15时21分35秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>

#include "spx_defs.h"
#include "spx_skiplist.h"
#include "spx_log.h"
#include "spx_types.h"
#include "spx_log.h"
#include "spx_alloc.h"

spx_private void spx_skiplist_printf(struct spx_skiplist *spl);
spx_private err_t skiplist_kfree(SpxLogDelegate *log,void **k);
spx_private err_t skiplist_vfree(SpxLogDelegate *log,void **v);
spx_private int skiplist_range_cmper(void *k1,u32_t l1,\
        void *k2,u32_t l2);

spx_private void spx_skiplist_printf(struct spx_skiplist *spl){
    u32_t l = 0;
    struct spx_skiplist_n *n = spl->header;
    struct spx_skiplist_n *q = NULL;
    for(l = 0; l < spl->level; l++){
        while(NULL != (q = n->next[l])) {
            if(NULL == spl->kprintf){
                printf("%s    ",(char *) q->k);
            } else {
                SpxString(skey,SpxStringRealSize(SpxKeyStringSize));
                spl->kprintf(skey,SpxKeyStringSize,q->k);
                printf("%s    ",skey);
            }
        }
        printf("\n");
    }
}
spx_private err_t skiplist_kfree(SpxLogDelegate *log,void **k){
    i32_t **n = (i32_t **)k;
    SpxFree(*n);
    return 0;
}
spx_private err_t skiplist_vfree(SpxLogDelegate *log,void **v){
    i32_t **n = (i32_t **) v;
    SpxFree(*n);
    return 0;
}

spx_private int skiplist_range_cmper(void *k1,u32_t l1,\
        void *k2,u32_t l2){
        return 0;
}
/*
    err_t spx_skiplist_new(SpxLogDelegate *log,\
            int type,u32_t maxlevel,\
            bool_t allow_conflict,
            SpxSkipListCmperDelegate cmper,\
            SpxSkipListUniqueInspectorDelegate *inspector,\
            SpxSkipListKeySnprintfDelegate *printf,\
            SpxSkipListKeyFreeDelegate *kfree,\
            SpxSkipListValueFreeDelegate *vfree,\
            struct spx_skiplist **spl);
*/
int main(int argc,char **argv){
    err_t rc = 0;
    SpxLogDelegate *log = spx_log;
    struct spx_skiplist *spl = NULL;
    if(0 != (rc = spx_skiplist_new(log,\
                    SPX_SKIPLIST_IDX_I32,8,false,\
                    spx_skiplist_i32_default_cmper,\
                    NULL,\
                    spx_skiplist_i32_default_printf,\
                    skiplist_kfree,\
                    skiplist_vfree,\
                    &spl))){
        SpxLog2(log,SpxLogError,rc,\
                "create skiplist is fail;");
        return 0;
    }

    int i = 0;
    for(i = 0;i< 100;i++){
        i32_t *k = NULL;
        if(0 != (rc = spx_alloc_alone(sizeof(i32_t),(void **)&k))){
            SpxLog2(log,SpxLogError,rc,\
                    "alloc for key is fail.");
            return 0;
        }
        *k = i;
        i32_t *v = NULL;
        if(0 != (rc = spx_alloc_alone(sizeof(i32_t),(void **)&v))){
            SpxLog2(log,SpxLogError,rc,\
                    "alloc for value is fail.");
            return 0;
        }
        *v = *k;
        if(0 != (rc = spx_skiplist_insert(spl,k,sizeof(*k),v,sizeof(*v),-1))){
            SpxLog2(log,SpxLogError,rc,\
                    "insert key to skiplist is fail.");
        }
    }

    spx_skiplist_printf(spl);
    i32_t k = 9;
    i32_t *v = NULL;
    u64_t vl = 0;
    if(0 != (rc = spx_skiplist_get_and_move(spl,(void *)&k,(u32_t) sizeof(k),\
                    (void **) &v,&vl,\
                    skiplist_range_cmper))){
        SpxLog2(log,SpxLogError,rc,\
                "get and move key:9 is fail.");
        return 0;
    }
    spx_skiplist_printf(spl);
    i32_t delk = 50;
    if(0 != (rc = spx_skiplist_delete(spl,&delk,sizeof(delk)))){
      SpxLog2(log,SpxLogError,rc,\
              "delete the key:50 is fail.");
    }
    spx_skiplist_printf(spl);
    return 0;
}
