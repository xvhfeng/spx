/*
 * =====================================================================================
 *
 *       Filename:  skiplist.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/05/19 11时23分26秒
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
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "headers/spx_skiplist.h"
#include "headers/spx_defs.h"
#include "headers/spx_errno.h"
#include "headers/spx_alloc.h"
#include "headers/spx_types.h"
#include "headers/spx_vector.h"
#include "headers/spx_string.h"

#define SkipListNodeSize(s) \
    (sizeof(struct spx_skiplist_n) \
     + sizeof(struct spx_skiplist_n *) * (s))

spx_private int get_skiplist_level(u32_t max_level);

spx_private int get_skiplist_level(u32_t max_level){/*{{{*/
    u32_t seedVal = 0;
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv , &tz);
    seedVal=((((unsigned int)tv.tv_sec&0xFFFF)+
                (unsigned int)tv.tv_usec)^
            (unsigned int)tv.tv_usec);
    srand((unsigned int)seedVal +  rand());
    return rand() % max_level + 1;
}/*}}}*/


int spx_skiplist_i32_default_cmper(void *k1,u32_t l1,void *k2,u32_t l2){
    if(NULL == k1 || NULL == k2) errno = EINVAL;
    i32_t *i1 = 0,*i2 = 0;
    i1 = (i32_t *) k1;
    i2 = (i32_t *) k2;
    if(*i1 < *i2) return -1;
    else if(*i1 > *i2) return 1;
    else return 0;
}
int spx_skiplist_i64_default_cmper(void *k1,u32_t l1,void *k2,u32_t l2){
    if(NULL == k1 || NULL == k2) errno = EINVAL;
    i64_t *i1 = 0,*i2 = 0;
    i1 = (i64_t *) k1;
    i2 = (i64_t *) k2;
    if(*i1 < *i2) return -1;
    else if(*i1 > *i2) return 1;
    else return 0;
}
int spx_skiplist_u32_default_cmper(void *k1,u32_t l1,void *k2,u32_t l2){
    if(NULL == k1 || NULL == k2) errno = EINVAL;
    u32_t *i1 = 0,*i2 = 0;
    i1 = (u32_t *) k1;
    i2 = (u32_t *) k2;
    if(*i1 < *i2) return -1;
    else if(*i1 > *i2) return 1;
    else return 0;
}
int spx_skiplist_u64_default_cmper(void *k1,u32_t l1,void *k2,u32_t l2){
    if(NULL == k1 || NULL == k2) errno = EINVAL;
    u64_t *i1 = 0,*i2 = 0;
    i1 = (u64_t *) k1;
    i2 = (u64_t *) k2;
    if(*i1 < *i2) return -1;
    else if(*i1 > *i2) return 1;
    else return 0;
}
int spx_skiplist_string_default_cmper(void *k1,u32_t l1,void *k2,u32_t l2){
    if(NULL == k1 || NULL == k2) errno = EINVAL;
    string_t s1 = (string_t) k1;
    string_t s2 = (string_t) k2;
    if(SpxStringIsEmpty(s1) \
            && !SpxStringIsNullOrEmpty(s2)) return -1;
    if(!SpxStringIsEmpty(s1) \
            && SpxStringIsEmpty(s2)) return 1;
    if(SpxStringIsEmpty(s1) \
            && SpxStringIsEmpty(s2)) return 0;
    return SpxStringCmp(s1,s2,SpxMax(l1,l2));
}
int spx_skiplist_time_default_cmper(void *k1,u32_t l1,void *k2,u32_t l2){
    if(NULL == k1 || NULL == k2) errno = EINVAL;
    time_t *i1 = 0,*i2 = 0;
    i1 = (time_t *) k1;
    i2 = (time_t *) k2;
    if(*i1 < *i2) return -1;
    else if(*i1 > *i2) return 1;
    else return 0;
}


void spx_skiplist_i32_default_printf(string_t buf,size_t size,void *k){
    i32_t *n = (i32_t *)k;
    spx_numb_tostring(buf,size,*n);
}
void spx_skiplist_u32_default_printf(string_t buf,size_t size,void *k){
    u32_t *n = (u32_t *)k;
    spx_numb_tostring(buf,size,*n);
}
void spx_skiplist_i64_default_printf(string_t buf,size_t size,void *k){
    i64_t *n = (i64_t *)k;
    spx_numb_tostring(buf,size,*n);
}
void spx_skiplist_u64_default_printf(string_t buf,size_t size,void *k){
    u64_t *n = (u64_t *)k;
    spx_numb_tostring(buf,size,*n);
}
void spx_skiplist_time_default_printf(string_t buf,size_t size,void *k){
    u64_t *n = (u64_t *)k;
    spx_numb_tostring(buf,size,*n);
}

err_t spx_skiplist_new(SpxLogDelegate *log,\
        int type,u32_t maxlevel,\
        bool_t allow_conflict,
        SpxSkipListCmperDelegate cmper,\
        SpxSkipListUniqueInspectorDelegate *inspector,\
        SpxSkipListKeySnprintfDelegate *kprintf,\
        SpxSkipListKeyFreeDelegate *kfree,\
        SpxSkipListValueFreeDelegate *vfree,\
        struct spx_skiplist **spl){/*{{{*/
    int err = 0;
    if(0 > type || type == SPX_SKIPLIST_IDX_OBJECT){
        SpxLogFmt1(log,SpxLogDebug,\
                "the argument is fail.type is %d,but compers count is %d.",\
                type,SPX_SKIPLIST_IDX_OBJECT);
        return EINVAL;
    }

    u32_t level = 0 == maxlevel ? SPX_SKIPLIST_LEVEL_DEFAULT : maxlevel;
    if(0 != (err = spx_alloc_alone(sizeof(struct spx_skiplist),(void **) spl))){
        SpxLog2(log,SpxLogError,err,"alloc skiplist is fail.");
        goto r1;
    }
    if(0 != (err = spx_alloc_alone(SkipListNodeSize(level),\
                    ((void **) &((*spl)->header))))){
        SpxLog2(log,SpxLogError,err,"alloc skiplist header is fail.");
        goto r1;
    }
    (*spl)->level = 0;
    (*spl)->cmp = cmper;
    (*spl)->inspector = inspector;
    (*spl)->allow_conflict = allow_conflict;
    (*spl)->maxlevel =level;
    (*spl)->log = log;
    (*spl)->type =type;
    (*spl)->vfree = vfree;
    (*spl)->kfree = kfree;
    (*spl)->kprintf = kprintf;
    SpxLogFmt1(log,SpxLogDebug,\
            "create skiplist.max level:%d,idx:%d allow_conflict:%d,",\
            level,type,allow_conflict);
    return 0;
r1:
    if(NULL != (*spl)->header){
        SpxFree((*spl)->header);
    }
    if(NULL != (*spl)){
        SpxFree(*spl);
    }
    return err;
}/*}}}*/

//Spaghetti algorithm
//if you modify the code ,please make sure you understand this code.
err_t spx_skiplist_insert(struct spx_skiplist *spl,\
        void *k,u32_t kl,void *v,u64_t vl,
        int level){/*{{{*/
    if(NULL == spl){
        return EINVAL;
    }
    if(NULL == k || NULL == v){
        SpxLog1(spl->log,SpxLogDebug, "the argument is fail.");
        return EINVAL;
    }

    int err = 0;
    struct spx_skiplist_n **up = NULL;
    struct spx_skiplist_v *value = NULL;
    struct spx_skiplist_n *node = NULL;

    up = spx_alloc_mptr(spl->maxlevel,&err);
    if(0 != err){
        SpxLog2(spl->log,SpxLogError,err,"alloc skiplist node is fail.");
        return err;
    }
    //if you understand the code behand this line very hard,please donot modify it
    //警告：如果你理解以下的语句比较吃力，不要修改它。

    struct spx_skiplist_n *p = spl->header;
    struct spx_skiplist_n *q = NULL;
    i32_t l = 0;
    int r = 0;
    for(l = spl->level - 1; l >= 0; l--){
        while(NULL != (q = p->next[l])){
            r = spl->cmp(k,kl,q->k,q->kl);
            if( 0 < r){
                up[l] = q;
                p = q;
                continue;
            }
            break;
        }
        up[l] = p;
    }

    if(0 == r && 0 != spl->key_count) {
        //if have same key,the r must == 0 except first insert
        if(!spl->allow_conflict){
            if(NULL == spl->kprintf){
                SpxLogFmt1(spl->log,SpxLogError,\
                        "the key:%s is exist and skiplist is not allow conflict.",
                        SpxString2Char2(k));
            } else {
                SpxString(skey,SpxStringRealSize(SpxKeyStringSize));
                spl->kprintf(skey,SpxKeyStringSize,k);
                SpxLogFmt1(spl->log,SpxLogError,\
                        "the key:%s is exist and skiplist is not allow conflict.",
                        skey);
            }
            err = EEXIST;
            goto r2;
        }
        //add the same key,so not change the skiplist structure
        if(0 != (err = spx_alloc_alone(sizeof(struct spx_skiplist_v),\
                        ((void **) &value)))){
            SpxLog2(spl->log,SpxLogError,err,\
                    "alloc skiplist value is fail.");
            goto r2;
        }
        value->s = vl;
        value->v = v;
        if(0 != (err = spx_vector_push(q->v.list,value))){
            SpxLog2(spl->log,SpxLogError,err,\
                    "push the value to the vector of skiplist node is fail.")
                goto r3;
        }
        spl->val_count ++;
        goto r2;
    }

    //是不是准备要放弃看这个算法了？这才刚刚开始
    //is not giving up understanding this function? this is begin,just so so.
    l = 0 > level ? get_skiplist_level(spl->maxlevel) : level;
    int i = 0;
    if(l >(i32_t) spl->level){
        for(i = l -1; i >= (int) spl->level; i--){
            up[i] = spl->header;
        }
    }
    if(0 != (err = spx_alloc_alone(SkipListNodeSize(l),(void **) &node))){
        SpxLog2(spl->log,SpxLogError,err,\
                "alloc node for skiplist is fail.");
        goto r3;
    }
    node->k = k;
    node->kl = kl;
    node->level = l;
    //add the same key,so not change the skiplist structure
    if(0 != (err = spx_alloc_alone(sizeof(struct spx_skiplist_v),\
                    ((void **) &value)))){
        SpxLog2(spl->log,SpxLogError,err,\
                "alloc skiplist value is fail.");
        goto r3;
    }
    value->s = vl;
    value->v = v;

    if(spl->allow_conflict){
        if(0 != (err = spx_vector_init(spl->log,&(node->v.list),spl->vfree))){
            SpxLog2(spl->log,SpxLogError,err,\
                    "alloc vector for skiplist node is fail.");
            goto r3;
        }
        if(0 != (err = spx_vector_push(node->v.list,value))){
            SpxLog2(spl->log,SpxLogError,err,\
                    "push value to vector is fail.");
            goto r3;
        }
    }else {
        node->v.obj = value;
    }
    for(i = l - 1; i >= 0; i--){
        node->next[i] = up[i]->next[i];
        up[i]->next[i] =node;
    }
    if(l > (int) spl->level){
        spl->level = l;
    }
    spl->key_count ++;
    spl->val_count ++;
    goto r2;
r3:
    if(NULL != value){
        SpxFree(value);
    }
    if(NULL != node){
        SpxFree(node);
    }
r2:
    if(NULL != up){
        SpxFree(up);
    }
    return err;
}/*}}}*/

err_t spx_skiplist_delete(struct spx_skiplist *spl,\
        void *k,u32_t kl){/*{{{*/
    if(NULL == spl){
        return EINVAL;
    }
    if(NULL == k){
        SpxLog1(spl->log,SpxLogDebug,\
                "argument is fail.");
        return EINVAL;
    }
    if(0 == spl->key_count) return 0;
    int err = 0;
    struct spx_skiplist_n **up = NULL;

    up = spx_alloc_mptr(spl->maxlevel,&err);
    if(0 != err){
        SpxLog2(spl->log,SpxLogError,err,"alloc skiplist node is fail.");
        return err;
    }
    //if you understand the code behand this line very hard,please donot modify it
    //警告：如果你理解以下的语句比较吃力，不要修改它。

    struct spx_skiplist_n *p = spl->header;
    struct spx_skiplist_n *q = NULL;
    int l = 0;
    int i = 0;
    int r = 0;
    for(l = spl->level - 1; l >= 0; l--){
        while(NULL != (q = p->next[l])){
            r = spl->cmp(k,kl,q->k,q->kl);
            if( 0 < r){
                up[l] = q;
                p = q;
                continue;
            }
            break;
        }
        up[l] = p;
    }

    if(0 != r && 0 != spl->key_count){
        if(NULL == spl->kprintf){
            SpxLogFmt1(spl->log,SpxLogWarn,\
                    "the key:%s is not exist.",
                    SpxString2Char2(k));
        } else {
            SpxString(skey,SpxStringRealSize(SpxKeyStringSize));
            spl->kprintf(skey,SpxKeyStringSize,k);
            SpxLogFmt1(spl->log,SpxLogWarn,\
                    "the key:%s is not exist.",
                    skey);
        }
        goto r1;
    }
    if(spl->allow_conflict){
        struct spx_vector *vector = q->v.list;
        if(NULL == vector){
            if(NULL == spl->kprintf){
                SpxLogFmt1(spl->log,SpxLogWarn,\
                        "the value of vector for key %s is exist and value is null.",\
                        SpxString2Char2(k));
            } else {
                SpxString(skey,SpxStringRealSize(SpxKeyStringSize));
                spl->kprintf(skey,SpxKeyStringSize,k);
                SpxLogFmt1(spl->log,SpxLogWarn,\
                        "the value of vector for key %s is exist and value is null.",\
                        skey);
            }
            goto r1;
        }
        size_t numbs = vector->size;
        if(0 != (err = spx_vector_destory(&vector))){
            if(NULL == spl->kprintf){
                SpxLogFmt1(spl->log,SpxLogError,\
                        "delete the key %s from skiplist is fail.",\
                        SpxString2Char2(k));
            } else {
                SpxString(skey,SpxStringRealSize(SpxKeyStringSize));
                spl->kprintf(skey,SpxKeyStringSize,k);
                SpxLogFmt1(spl->log,SpxLogError,\
                        "delete the key %s from skiplist is fail.",\
                        skey);
            }
            goto r1;
        }

        for(i = q->level - 1; i >= 0; i--){
            up[i]->next[i] = q->next[i];
        }
        if(NULL != spl->kfree) {
            spl->kfree(&(q->k));
        }
        SpxFree(q);
        spl->val_count -= numbs;
        spl->key_count --;
    }else{
        for(i = q->level - 1; i >= 0; i--){
            up[i]->next[i] = q->next[i];
        }

        if(NULL != spl->vfree){
            spl->vfree((void **) &(q->v.obj));
        }
        if(NULL != spl->kfree){
            spl->kfree(&(q->k));
        }
        SpxFree(q);
        spl->val_count --;
        spl->key_count --;
    }
r1:
    if(NULL != up){
        SpxFree(up);
    }
    return err;
}/*}}}*/

err_t spx_skiplist_get_and_move(struct spx_skiplist *spl,\
        void *k,u32_t kl,void **v,u64_t *vl,
        SpxSkipListRangeCmperDelegate *searcher){/*{{{*/
    if(NULL == spl){
        return EINVAL;
    }
    if(NULL == k){
        SpxLog1(spl->log,SpxLogDebug,\
                "argument is fail.");
        return EINVAL;
    }
    if(0 == spl->key_count) return 0;
    int err = 0;
    struct spx_skiplist_n **up = NULL;
    up = spx_alloc_mptr(spl->maxlevel,&err);
    if(0 != err){
        SpxLog2(spl->log,SpxLogError,err,"alloc skiplist node is fail.");
        return err;
    }

    //if you understand the code behand this line very hard,please donot modify it
    //警告：如果你理解以下的语句比较吃力，不要修改它。

    struct spx_skiplist_n *p = spl->header;
    struct spx_skiplist_n *q = NULL;
    int l = 0;
    int r = 0;
    for(l = spl->level - 1; l >= 0; l--){
        while(NULL != (q = p->next[l])){
            if(NULL == searcher) {
                r = spl->cmp(k,kl,q->k,q->kl);
            } else {
                r = searcher(k,kl,q->k,q->kl);
            }
            if( 0 < r){
                up[l] = q;
                p = q;
                continue;
            }
            break;
        }
        up[l] = p;
    }

    if(0 != r && 0 != spl->key_count){
        if(NULL == spl->kprintf){
            SpxLogFmt1(spl->log,SpxLogError,\
                    "not found the key:%s in the skiplist.",\
                    SpxString2Char2(k));
        } else {
            SpxString(skey,SpxStringRealSize(SpxKeyStringSize));
            spl->kprintf(skey,SpxKeyStringSize,k);
            SpxLogFmt1(spl->log,SpxLogError,\
                    "not found the key:%s in the skiplist.",\
                    skey);
        }
        goto r1;
    }

    int i = 0;
    struct spx_skiplist_v *nv = NULL;
    if(spl->allow_conflict){
        struct spx_vector *vector = q->v.list;
        if(0 < vector->size){
            if(0 != (err = spx_vector_pop(vector,(void **) &nv))){

                if(NULL == spl->kprintf){
                    SpxLogFmt1(spl->log,SpxLogError,\
                            "pop the key:%s form node of skiplist is fai.",\
                            SpxString2Char2(k));
                } else {
                    SpxString(skey,SpxStringRealSize(SpxKeyStringSize));
                    spl->kprintf(skey,SpxKeyStringSize,k);
                    SpxLogFmt1(spl->log,SpxLogError,\
                            "pop the key:%s form node of skiplist is fai.",\
                            skey);
                }
                goto r1;
            }
            *v = nv->v;
            *vl = nv->s;
        }
        if(0 == vector->size){
            if(0 != (err = spx_vector_destory(&vector))){

                if(NULL == spl->kprintf){
                    SpxLogFmt1(spl->log,SpxLogError,\
                            "remove the key:%s from skiplist is fail.",\
                            SpxString2Char2(k));
                } else {
                    SpxString(skey,SpxStringRealSize(SpxLineSize));
                    spl->kprintf(skey,SpxLineSize,k);
                    SpxLogFmt1(spl->log,SpxLogError,\
                            "remove the key:%s from skiplist is fail.",\
                            skey);
                }
                goto r1;
            }
            for(i = q->level - 1; i >= 0; i--){
                up[i]->next[i] = q->next[i];
            }
            if(NULL != spl->kfree) {
                spl->kfree(&(q->k));
            }
            SpxFree(q);
            spl->key_count --;
        }
        spl->val_count --;
        goto r1;
    }else{
        for(i = q->level - 1; i >= 0; i--){
            up[i]->next[i] = q->next[i];
        }
        *v = q->v.obj->v;
        *vl = q->v.obj->s;
        if(NULL != spl->kfree) {
            spl->kfree(&(q->k));
        }
        SpxFree(q);
        spl->key_count --;
        spl->val_count --;
    }
r1:
    if(NULL != up){
        SpxFree(up);
    }
    return err;
}/*}}}*/

/*
   err_t spx_skiplist_search(struct spx_skiplist *spl,\
   void *min,u32_t l1,void *max,u32_t l2,
   struct spx_vector **rc);

   err_t spx_skiplist_find(struct spx_skiplist *spl,\
   void *k,u32_t l,void **rc);
   */
void spx_skiplist_destory(struct spx_skiplist **spl){/*{{{*/
    if(NULL == *spl){
        return;
    }
    struct spx_skiplist_n *node = (*spl)->header;
    struct spx_skiplist_n *q = NULL;
    if((*spl)->allow_conflict) {
        q = node->next[0];
        while(NULL != q){
            struct spx_vector *list =  q->v.list;
            spx_vector_destory(&list);
            node = q->next[0];
            (*spl)->kfree(&(q->k));
            SpxFree(q);
        }
    } else {
        q = node->next[0];
        while(NULL != q){
            (*spl)->vfree((void **) &(q->v.obj));
            node = q->next[0];
            (*spl)->kfree(&(q->k));
            SpxFree(q);
        }
    }
    SpxFree((*spl)->header);
    SpxFree(*spl);
    return;
}/*}}}*/



