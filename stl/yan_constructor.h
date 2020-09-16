//
// Created by YCNN on 2020-05-27.
//

#ifndef SAKURA_YAN_CONSTRUCTOR_H
#define SAKURA_YAN_CONSTRUCTOR_H

#include <new>//new和delete
#include "yan_config.h"
#include "type_traits.h"
#include "yan_iterator_base.h"

__STL_BEGIN_NAMESPACE

    template<class _T1,class _T2>
    inline void _Construtor(_T1 * __p,const _T2 & __value){
        // 定位new操作，对__p所指的对象初始化
    new((void *)__p) _T1(__value);
}

    template<class _T1>
    inline void _Construct(_T1 * __p){
    new((void *) __p) _T1();
}

    template<class _Tp>
    void destroy_one(_Tp *,__true_type){}

    template<class _Tp>
    void destroy_one(_Tp *pointer,__false_type){
if (pointer != nullptr){
    pointer->~Tp();
}
}

    template<class _Tp>
    inline void _Destroy(_Tp *__pointer){
    destroy_one(__pointer,typename __type_traits<_Tp>::has_trivial_destructor());
}

//析构了一堆，从first到last
    template<class _ForwardIterator>
    void __destroy_aux(_ForwardIterator __first,_ForwardIterator __last,__false_type){
    for(;__first != __last;++__first)
        _Destroy(&*__first);
}

    template<class _ForwardIterator>
    inline void __destory_aux(_ForwardIterator,_ForwardIterator,__true_type){}


    // 销毁一段元素，参3指定元素的类型，根据元素类型来判断调用哪种辅助销毁函数
    template<class _ForwardIterator,class _Tp>
    inline void __destory(_ForwardIterator __first,_ForwardIterator __last,_Tp *){
        typedef typename __type_traits<_Tp>::has_trivial_destructor _Trivial_destructor;
        __destory_aux(__first,__last,_Trivial_destructor());
}
// 销毁一段元素，根据元素获取它的值类型，具体销毁交给别人去做
    template<class _ForwardIterator>
    inline void _Destory(_ForwardIterator __first,_ForwardIterator __last){
    __destory(__first,__last,__VALUE_TYPE(__first));
}
// 普通内置类型的销毁函数
inline void _Destroy(char *,char *){}
inline void _Destory(int *,int *){}
inline void _Destory(float *,float *){}
inline void _Destory(long *,long *){}
inline void _Destory(double *,double *){}

// 真正的构造，从此处开始发布任务，具体实现交给他人去做。需要分的这么细吗？
// 给定指针，给定要赋给的值，用这个指去构造指针所指的对象
    template<class _T1,class _T2>
    inline void construct(_T1 *__p,const &__value){
    _Construct(__p,__value);
}
// 给定指针，默认构造指针所指的对象
template<class _T1>
inline void construct(_T1 *_p){
    _Construct(_p);
}
//析构一个
template<class _Tp>
inline void destroy(_Tp *__pointer){
    _Destroy(__pointer);
}

//层层调用，最后析构掉了一堆
template<class _ForwardIterator>
inline void destroy(_ForwardIterator __first,_ForwardIterator __last){
    _Destroy(__first,__last);
}

__STL_END_NAMESPACE


#endif //SAKURA_YAN_CONSTRUCTOR_H
