/***********************************************************************
 *                              _ooOoo_
 *                             o8888888o
 *                             88" . "88
 *                             (| -_- |)
 *                              O\ = /O
 *                          ____/`---'\____
 *                        .   ' \\| |// `.
 *                         / \\||| : |||// \
 *                       / _||||| -:- |||||- \
 *                         | | \\\ - /// | |
 *                       | \_| ''\---/'' | |
 *                        \ .-\__ `-` ___/-. /
 *                     ___`. .' /--.--\ `. . __
 *                  ."" '< `.___\_<|>_/___.' >'"".
 *                 | | : `- \`.;`\ _ /`;.`/ - ` : | |
 *                   \ \ `-. \_ __\ /__ _/ .-` / /
 *           ======`-.____`-.___\_____/___.-`____.-'======
 *                              `=---='
 *           .............................................
 *                    佛祖镇楼                  BUG辟易
 *            佛曰:
 *                    写字楼里写字间，写字间里程序员；
 *                    程序人员写程序，又拿程序换酒钱。
 *                    酒醒只在网上坐，酒醉还来网下眠；
 *                    酒醉酒醒日复日，网上网下年复年。
 *                    但愿老死电脑间，不愿鞠躬老板前；
 *                    奔驰宝马贵者趣，公交自行程序员。
 *                    别人笑我忒疯癫，我笑自己命太贱；
 *                    不见满街漂亮妹，哪个归得程序员？
 * ==========================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  SpxAtomic.h
 *        Created:  2015年01月12日 11时38分04秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/
#ifndef _SPXATOMIC_H_
#define _SPXATOMIC_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>

#define __SpxAtomicAdd(ptr,v) __sync_add_and_fetch(ptr,v)
#define __SpxAtomicSub(ptr,v) __sync_sub_and_fetch(ptr,v)
#define __SpxAtomicIncr(ptr) __sync_add_and_fetch(ptr,1)
#define __SpxAtomicDecr(ptr) __sync_sub_and_fetch(ptr,1)
#define __SpxAtomicLazyAdd(ptr,v) __sync_fetch_and_add(ptr,v)
#define __SpxAtomicLazySub(ptr,v) __sync_fetch_and_sub(ptr,v)
#define __SpxAtomicLazyIncr(ptr) __sync_fetch_and_add(ptr,1)
#define __SpxAtomicLazyDecr(ptr) __sync_fetch_and_sub(ptr,1)
#define __SpxAtomicRelease(ptr) __sync_lock_release(ptr)
#define __SpxAtomicSet(ptr,v) __sync_lock_test_and_set(ptr,v)
#define __SpxAtomicIsCas(ptr,o,v) __sync_bool_compare_and_swap(ptr,o,v)
#define __SpxAtomicCas(ptr,o,v) __sync_val_compare_and_swap(ptr,o,v)



#define __SpxAtomicVAdd(v,a) __sync_add_and_fetch(&(v),a)
#define __SpxAtomicVSub(v,s) __sync_sub_and_fetch(&(v),s)
#define __SpxAtomicVIncr(v) __sync_add_and_fetch(&(v),1)
#define __SpxAtomicVDecr(v) __sync_sub_and_fetch(&(v),1)
#define __SpxAtomicLazyVAdd(v,a) __sync_fetch_and_add(&(v),a)
#define __SpxAtomicLazyVSub(v,s) __sync_fetch_and_sub(&(v),s)
#define __SpxAtomicLazyVIncr(v) __sync_fetch_and_add(&(v),1)
#define __SpxAtomicLazyVDecr(v) __sync_fetch_and_sub(&(v),1)
#define __SpxAtomicVRelease(v) __sync_lock_release(&(v))
#define __SpxAtomicVSet(v,n) __sync_lock_test_and_set(&(v),n)
#define __SpxAtomicVIsCas(v,o,n) __sync_bool_compare_and_swap(&(v),o,n)
#define __SpxAtomicVCas(v,o,n) __sync_val_compare_and_swap(&(v),o,n)



#define __SpxAtomicMB() __sync_synchronize()
#define __SpxAtomicRMB() __SpxAtomicMB()
#define __SpxAtomicWMB() __SpxAtomicMB()

#ifdef __cplusplus
}
#endif
#endif
