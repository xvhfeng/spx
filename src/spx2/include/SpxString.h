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
 *       Filename:  SpxString.h
 *        Created:  2015年01月19日 12时43分13秒
 *         Author:  Seapeak.Xu (www.94geek.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ****************************************************************************/
#ifndef _SPXSTRING_H_
#define _SPXSTRING_H_
#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>

#include "SpxTypes.h"
#include "SpxObject.h"
#include "SpxMFunc.h"

    struct SpxString{
        SpxObjectBase;
        size_t _freeSize;
        char _buf[0];
    };

#define __SpxStringConvert(n,s) __SpxTypeConvert(struct SpxString,n, \
        __SpxDecr(s,sizeof(struct SpxString)))

#define __SpxStringConvertToObject(n,s) __SpxTypeConvert(struct SpxObject,n, \
        __SpxDecr(s,sizeof(struct SpxString)))

#define __SpxStringFree(s) \
    do { \
        if(NULL != s && _spxStringFree(s)) { \
            s = NULL; \
        } \
    }while(false)

#define __SpxStringFreeForce(s) \
    do { \
        if(NULL != s) {\
            _spxStringFreeForce(s);\
            s = NULL; \
        } \
    }while(false)



    private u32_t __spxStringRefCount(string_t s){
        if(NULL == s){
            return 0;
        }
         __SpxStringConvert(ss,s);
        return ss->_spxObjectRefs;
    }

    private size_t __spxStringLength(string_t s){
        if(NULL == s){
            return 0;
        }
        __SpxStringConvert(ss,s);
        return ss->_spxObjectSize - ss->_freeSize;
    }

    private size_t __spxStringSize(string_t s){
        if(NULL == s){
            return 0;
        }
        __SpxStringConvert(ss,s);
        return ss->_spxObjectSize;
    }

    private size_t __spxStringFreeSize(string_t s){
        if(NULL == s){
            return 0;
        }
        __SpxStringConvert(ss,s);
        return ss->_freeSize;
    }



#define __SpxStringNewLength(len,err) spxStringNew(NULL,len,err)
#define __SpxStringNewEmpty(err) spxStringNew(NULL,0,err)
#define __SpxStringClone(s,err) spxStringNew(s,__spxStringLength(s),err)

#define __SpxStringAppendChars(s,t,err) spxStringAppend(s,t,strlen(t),err)
#define __SpxStringAppendString(s,t,err) spxStringAppend(s,t,__spxStringLength(t),err)

#define __SpxStringLeftTrim(s) spxStringLeftTrimSet(s, " ");
#define __SpxStringRightTrim(s) spxStringRightTrimSet(s, " ");
#define __SpxStringTrim(s) spxStringTrimSet(s, " ");
#define __SpxStringLineTrim(s) __SpxStringRightTrim(s);spxStringRightTrimSet(s,SpxLineEndDlmtString)
#define __SpxStringCopyChars(s,begin,t,err)  spxStringCopy(s,begin,t,strlen(t),err)
#define __SpxStringSplit(s,sep,seplen,count,err)  spxStringSplitChars(s,__spxStringLength(s),sep,seplen,count,err);
#define __SpxStringSplitString(s,sep,count,err) spxStringSplitChars(s,__spxStringLength(s),sep,__spxStringLength(sep),count,err);

#define __SpxStringIsNullOrEmpty(s) (NULL == s \
        || (0 == __spxStringLength(s)))
#define __SpxStringIsEmpty(s) (NULL != s \
        && (0 == __spxStringLength(s)))

#define __SpxStringEndWith(s,c) \
        (!__SpxStringIsNullOrEmpty(s) \
        && (c == *(s +  __spxStringLength(s) - 1)))
#define __SpxStringBeginWith(s,c) ( c == *s)

string_t spxStringNew(const var init,size_t len,err_t *err);
string_t spxStringConvert(const char *init,err_t *err);
var spxStringRef(string_t s);
bool_t _spxStringFree(string_t s);
bool_t _spxStringFreeForce(string_t s);

string_t spxStringAppend(string_t s,const var t,size_t len,err_t *err);
string_t spxStringAlignedAppend(string_t s,const var t,const size_t len,
        const size_t align,err_t *err);
string_t spxStringInsert(string_t s,const i32_t offset,
        const var t,const size_t len,err_t *err);
string_t spxStringAlignedInsert(string_t s,const i32_t offset,
        const var t,const size_t len,
        size_t align,err_t *err);
string_t spxStringCopy(string_t s,int begin,const var t,size_t len,err_t *err);
err_t spxStringAppendPrint(string_t *dest,string_t s,const char *fmt,va_list ap);
err_t spxStringAppendFormat(string_t *dest,string_t s,const char *fmt,...);
void spxStringRange(string_t s,int start,int end);
string_t spxStringRangeNew(string_t s,int start,int end,err_t *err);
void spxStringTrimSet(string_t s,const char *cset);
void spxStringLeftTrimSet(string_t s,const char *cset);
void spxStringRightTrimSet(string_t s,const char *cset);
void spxStringUpdate(string_t s);
void spxStringClear(string_t s);
string_t spxStringFromNumber(i64_t value,err_t *err);
string_t *spxStringSplitChars(const char *s,int len,
        const char *sep,int seplen,int *count,err_t *err);
void spxStringSplitFree(string_t *tokens, int count);
void spxStringToLower(string_t s);
void spxStringToUpper(string_t s);

string_t spxStringJoin(char **argv,int argc,char *sep,size_t seplen,err_t *err);
string_t spxStringJoinString(string_t *argv,int argc,const char *sep,size_t seplen,err_t *err);

int spxStringCmp(const string_t s1, const string_t s2);
int spxStringCaseCmp(const string_t s1, const char *s2);
int spxStringCaseCmpString(const string_t s1, const string_t s2);

bool_t spxStringBeginWithString(const string_t s1,const string_t s2);
bool_t spxStringBeginWith(const string_t s1,const char *s2);
bool_t spxStringBeginCaseWithString(const string_t s1,const string_t s2);
bool_t spxStringBeginCaseWith(const string_t s1,const char *s2);
bool_t spxStringEndWithString(const string_t s1,const string_t s2);
bool_t spxStringEndCaseWithString(const string_t s1,const string_t s2);
bool_t spxStringExist(string_t s,char c);

#ifdef __cplusplus
}
#endif
#endif
