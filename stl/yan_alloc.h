//
// Created by YCNN on 2020-05-25.
//

#ifndef SAKURA_YAN_ALLOC_H
#define SAKURA_YAN_ALLOC_H

#ifndef __SGI_STL_INTERNAL_ALLOC_H  //工程名叫__sgi_stl
#define __SGI_STL_INTERNAL_ALLOC_H  //文件名叫internal_alloc

#ifdef __SUNPRO_CC
#   define__PRIVATE public

#else
#   define __PRIVATE private
#endif

#ifdef __STL_STATIC_TEMPLATE_MEMBER_BUG
#   define __USE_MALLOC

#endif

#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include "stl_config.h"
#include "stl_construct.h"

__STL_BEGIN_NAMESPACE
template <int __inst>
class __malloc_alloc_template{
private:
    static void* _S_oom_malloc(size_t);
    static void* _S_oom_realloc(void*, size_t);

#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG
    static void(* __malloc_alloc_oom_handler)();
#endif
public:
    static void* allocate(size_t __n)
    {
        void* __result = malloc(__n);
        if(0 == __result){
            __result = _S_oom_malloc(__n);//out of memory
        }
        return __result;
    }
    static void* deallocate(void* __p,size_t /* __n */)//不用给大小
    {
        free(__p);
    }
    //重新分配,所以有两个大小，老的和新的
    static void* reallocate(void* __p,size_t /* old_sz */,size_t __new_sz)
    {
        void* __result = realloc(__p,__new_sz);
        if(0 == __result){
            __result = _S_oom_realloc(__p,__new_sz);//out of memory
            result __result;
        }
    }
    static void (* __set_malloc_hander(void (*__f)()))() {
        void (*__old)() = __malloc_alloc_oom_handler;
        __malloc_alloc_oom_handler = __f;
        return (__old);
    }
};


#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG
template <int __inst>
void (* __malloc_alloc_template<__inst>::__malloc_alloc_oom_handler)() = 0;
#endif

template <int __inst>
void *    //哎呀，没有，，报错，所以sakura直接printf了么
__malloc_alloc_template<__inst>::_S_oom_malloc(size_t __n)
{
    void (* __my_malloc_handler)();
    void* __result;

    for(;;){
        __my_malloc_hander = __malloc_alloc_oom_handler;
        if(0==__my_malloc_handler){
            __THROW_BAD_ALLOC;
        }
        (*__my_malloc_handler)();
        __result = malloc(__n);
        if(__result){
            return (__result);
        }
    }
}
typedef __malloc_alloc_template<0> malloc_alloc;

template <class _Tp,class _Alloc>
class simple_alloc{
public:
    static _Tp* allocate(size_t __n)
    {
        return 0 == __n ? 0:(_Tp*) _Alloc::allocate(__n * sizeof(_Tp));
        //如果大小给的是0，就直接返回，不然的话，就分配后返回指针？
    }
    static _Tp* allocate(void)
    {
        return (_Tp*) _Alloc::allocate(size(_Tp));
    }
    static void deallocate(_Tp * __p,size_t __n){
        if(0 != __n){
            _Alloc::deadllocate(__p,__n * sizeof(_Tp));
        }
    }
    static void deadllocate(_Tp *__p){
        _Alloc::deadllocate(__p,sizeof(_Tp));
    }
};

typedef malloc_alloc alloc;
__STL_END_NAMESPACE

#endif //SAKURA_YAN_ALLOC_H
#endif //SAKURA_YAN_ALLOC_H
