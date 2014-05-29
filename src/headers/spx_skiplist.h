/*
 * =====================================================================================
 *
 *       Filename:  spx_skiplist.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/05/21 16时57分57秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef _SPX_SKIPLIST_H_
#define _SPX_SKIPLIST_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "spx_types.h"
#include "spx_vector.h"

#define SPX_SKIPLIST_LEVEL_DEFAULT 16
#define SPX_SKIPLIST_IDX_I32 0
#define SPX_SKIPLIST_IDX_I64 1
#define SPX_SKIPLIST_IDX_U32 2
#define SPX_SKIPLIST_IDX_U64 3
#define SPX_SKIPLIST_IDX_STRING 4
#define SPX_SKIPLIST_IDX_TIME 5
#define SPX_SKIPLIST_IDX_OBJECT 6

    typedef int SpxSkipListCmperDelegate(\
            void *k1,u32_t l1,void *k2,u32_t l2);
    typedef int SpxSkipListRangeCmperDelegate(\
            void *k1,u32_t l1,void *k2,u32_t l2);
    typedef bool_t SpxSkipListUniqueInspectorDelegate(\
            void *v1,size_t l1,void *v2,size_t l2);
    typedef err_t SpxSkipListValueFreeDelegate(\
            SpxLogDelegate *log,void **v);
    typedef err_t SpxSkipListKeyFreeDelegate(\
            SpxLogDelegate *log,void **k);
    typedef void SpxSkipListKeySnprintfDelegate(\
           string_t buf,size_t size,void *k);


    int spx_skiplist_i32_default_cmper(void *k1,u32_t l1,void *k2,u32_t l2);
    int spx_skiplist_i64_default_cmper(void *k1,u32_t l1,void *k2,u32_t l2);
    int spx_skiplist_u32_deault_cmper(void *k1,u32_t l1,void *k2,u32_t l2);
    int spx_skiplist_u64_default_cmper(void *k1,u32_t l1,void *k2,u32_t l2);
    int spx_skiplist_string_default_cmper(void *k1,u32_t l1,void *k2,u32_t l2);
    int spx_skiplist_time_default_cmper(void *k1,u32_t l1,void *k2,u32_t l2);

    void spx_skiplist_i32_default_printf(string_t buf,size_t size,void *k);
    void spx_skiplist_u32_default_printf(string_t buf,size_t size,void *k);
    void spx_skiplist_i64_default_printf(string_t buf,size_t size,void *k);
    void spx_skiplist_u64_default_printf(string_t buf,size_t size,void *k);
    void spx_skiplist_time_default_printf(string_t buf,size_t size,void *k);

    struct spx_skiplist_n{
        u32_t level;
        void *k;
        u32_t kl;
        union{
            struct spx_vector *list;
            struct spx_skiplist_v *obj;
        }v;
        struct spx_skiplist_n *next[];
    };

    struct spx_skiplist_v{
        void *v;
        size_t s;
    };

    struct spx_skiplist {
        u32_t type;
        u32_t level;
        u32_t maxlevel;
        u64_t key_count;
        u64_t val_count;
        bool_t allow_conflict;
        struct spx_skiplist_n *header;
        SpxSkipListCmperDelegate *cmp;
        SpxSkipListUniqueInspectorDelegate *inspector;
        SpxSkipListKeyFreeDelegate *kfree;
        SpxSkipListValueFreeDelegate *vfree;
        SpxSkipListKeySnprintfDelegate *kprintf;
        SpxLogDelegate *log;
    };

    err_t spx_skiplist_new(SpxLogDelegate *log,\
            int type,u32_t maxlevel,\
            bool_t allow_conflict,
            SpxSkipListCmperDelegate cmper,\
            SpxSkipListUniqueInspectorDelegate *inspector,\
            SpxSkipListKeySnprintfDelegate *printf,\
            SpxSkipListKeyFreeDelegate *kfree,\
            SpxSkipListValueFreeDelegate *vfree,\
            struct spx_skiplist **spl);

    err_t spx_skiplist_insert(struct spx_skiplist *spl,\
            void *k,u32_t kl,void *v,u64_t vl,
            int level);

err_t spx_skiplist_get_and_move(struct spx_skiplist *spl,\
        void *k,u32_t kl,void **v,u64_t *vl,
        SpxSkipListRangeCmperDelegate *searcher);

    err_t spx_skiplist_delete(struct spx_skiplist *spl,\
            void *k,u32_t l);
    /*
       err_t spx_skiplist_search(struct spx_skiplist *spl,\
       void *min,u32_t l1,void *max,u32_t l2);

       err_t spx_skiplist_find(struct spx_skiplist *spl,\
       void *k,u32_t l);
       */
    void spx_skiplist_destory(struct spx_skiplist **spl);

#ifdef __cplusplus
}
#endif
#endif
