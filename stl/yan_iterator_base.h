//
// Created by YCNN on 2020-05-25.
//

#ifndef SAKURA_YAN_ITERATOR_BASE_H
#define SAKURA_YAN_ITERATOR_BASE_H

#include <stddef.h>
#include "yan_config.h"

__STL_BEGIN_NAMESPACE
struct input_iterator_tag{
};
struct output_iterator_tag{
};
struct forward_iterator_tag: public input_iterator_tag{
};
struct bidirectional_iterator_tag: public forward_iterator_tag{
};
struct random_access_iterator_tag: public bidirectional_iterator_tag{
};
//这里开始都是两个参数
//输入的迭代器
template <class _Tp, class _Distance>
struct input_iterator{
    typedef input_iterator_tag iterator_category;
    typedef _Tp value_type;
    typedef _Distance difference_type;
    typedef _Tp *pointer;
    typedef _Tp &reference;
};
//输出的迭代器
struct output_iterator{
    typedef output_iterator_tag iterator_category;
    typedef void value_type;
    typedef void difference_type;
    typedef void pointer;
    typedef void reference;
};
//单向走的迭代器
template<class _Tp,class _Distance>
struct forward_iterator{
    typedef forward_iterator_tag iterator_category;
    typedef _Tp value_type;
    typedef _Distance difference_type;
    typedef _Tp *pointer;
    typedef _Tp &reference;
};
//双向走的迭代器
template<class _Tp,class _Distance>
struct bidirectional_iterator{
    typedef bidirectional_iterator_tag iterator_category;
    typedef _Tp value_type;
    typedef _Distance difference_type;
    typedef _Tp * pointer;
    typedef _Tp &reference;
};
//随机取值的迭代器
template <class _Tp,class _Distance>
struct ramdom_access_iterator{
    typedef random_access_iterator_tag iterator_category;
    typedef _Tp value_type;
    typedef _Distance difference_type;
    typedef _Tp *pointer;
    typedef _Tp &reference;
};

//传入五个参数
template <class _Category,class _Tp,class _Distance = ptrdiff_t,class _Pointer=_Tp *,class _Reference = Tp &>
struct iterator{
    typedef _Category iterator_category;
    typedef _Tp value_type;
    typedef _Distance difference_type;
    typedef _Pointer pointer;
    typedef _Reference reference;
};
//typedef+原类型名+新类型名
//typename == class
//template<class T> ==> 看起来只支持用户自定义类型
//使用时才声明命名空间 ==> 限定名，否则，非限定名
//只要依赖于模板参数，就是依赖名，否则，非依赖名
//typename告诉它是类型而不是变量，其他的是变量

//传入一个迭代器
template <class _Iterator>
struct iterator_traits{
    typedef typename _Iterator::iterator_category iterator_category;
    typedef typename _Iterator::value_type value_type;
    typedef typename _Iterator::difference_type difference_type;
    typedef typename _Iterator::pointer pointer;
    typedef typename _Iterator::reference reference;
};

//size_t类型指明数组长度，unsigned，必须是正数
//ptrdiff_t可以存放两个指针之间的距离，可能是负数，signed

//对于指针类型的偏特化
template <class _Tp>
struct iterator_traits<_Tp *>{
    typedef random_access_iterator_tag iterator_category;
    typedef _Tp value_type;
    typedef ptrdiff_t difference_type;
    typedef _Tp *pointer;
    typedef _Tp &reference;
};

//常指针
template <class _Tp>
struct iterator_traits<const _Tp *>{
    typedef random_access_iterator_tag iterator_category;
    typedef _Tp value_type;
    typedef ptrdiff_t difference_type;
    typedef const _Tp *pointer;
    typedef const _Tp &reference;
};

//将一个类型通过调用函数的方式改了下名字？
//该的是category
template <class _Iter>
//内置函数          返回值                                     函数名             参数列表
inline typename iterator_traits<_Iter>::iterator_category __iterator_category(const _Iter &){
    typedef typename iterator_traits<_Iter>::iterator_category _Catgory;
    return _Catgory();
}

//有一个参数,shiyige
template <class _Iter>
inline typename iterator_traits<_Iter>::iterator_category iterator_category(const _Iter &__i){
    return __iterator_category(__i);
}

//difference_type
//static_cast <type-id>( expression ):把expression转换为type-id类型
//返回了一个空的distance_type
//distance是一个类型，定义difference的
template <class _Iter>
inline typename iterator_traits<_Iter>::difference_type *__distance_type(const _Iter &){
    return static_cast<typename iterator_traits<_Iter>::difference_type *>(0);
}

//有一个参数,different
template <class _Iter>
inline typename iterator_traits<_Iter>::difference_type *distance_type(const _Iter &__i){
    return __distance_type(__i);
}

//返回一个value，空的
template <class _Iter>
inline typename iterator_traits<_Iter>::value_type *__value_type(const _Iter &){
    return static_cast<typename iterator_traits<_Iter>::value_type *>(0);
}

//返回一个value，有一个参数
template <class _Iter>
inline typename iterator_traits<_Iter>::value_type *value_type(const _Iter &__i){
    return __value_type(__i);
}

#define __ITERATOR_CATEGORY(__i) __iterator_category(__i)
#define __DISTANCE_TYPE(__i) __distance_type(__i)
#define __VALUE_TYPE(__i) __value_type(__i)

//当first走到last的时候，n里面的存的也就是开始到结束的距离了，一步一步走过去的，因为可能不连续
template <class _InputIterator,class _Distance>
inline void __distance(_InputIterator __first,_InputIterator __last,_Distance &__n,input_iterator_tag){
    while (__first != __last){
        ++__first;
        ++__n;
    }
}

//开头和结束的距离
template <class _RandomAccessIterator,class _Distance>
inline void __distance(_RandomAccessIterator __first,_RandomAccessIterator __last,_Distance &__n,random_access_iterator_tag){
    __n += __last - __first;
}

//iterator_category迭代器的五个分类
//用distance
template <class _InputIterator,class _Distance>
inline void distance(_InputIterator __first,_InputIterator __last,_Distance &__n){
    __distance(__first,__last,__n,iterator_category(__first));
}
            //    partial  specializtion
#ifdef __STL_CLASS_PARTIAL_SPECIALIZATION
//没有传入distance

template<class _InputIterator>
inline typename iterator_traits<_InputIterator>::difference_type
__distance(_InputIterator __first,_InputIterator __last,input_interator_tag){
    typename iterator_traits<_InputIterator>::difference_type __n = 0;
    while(__first != __last){
        ++__first;
        ++__n;
    }
    return __n;
}
//开始randomaccessiterator
template<class _RandomAccessIterator>
inline typename iterator_traits<_RandomAccessIterator>::difference_type
        __distance(_RandomAccessIterator __first,_RandomAccessIterator __last,random_access_iterator_tag){
            return __last - __first;
        }
template<class _InputIterator>
inline typename iterator_traits<_InputIterator>::difference_type
distance(_InputIterator __first,_InputIterator __last){
    typedef typename iterator_traits<_InputIterator>::iterator_category_Category;
    __distance(__first,__last,_Category());
}
#endif/* __STL_CLASS_PARTIAL_SPECIALIZATION */

//在迭代器范围内，往后走一格
template <class _InputIterator,class _Distance>
inline void __advance(_InputIterator &__i,_Distance __n,input_iterator_tag){
    while (__n--){
        ++__i;
    }
}

//将迭代器前进或后退n个位置
template <class _BidirectionalIterator,class _Distance>
inline void __advance(_BidirectionalIterator &__i,_Distance __n,bidirectional_iterator_tag){
    if(__n  > 0){//往后走，
        while (__n--){//一部一步走呗，等到0就走完了
            ++__i;
        }
    }else{//往前走
        while (__n++){
            --__i;
        }
    }
}

template <class _RandomAccessIterator,class _Distance>
inline void __advance(_RandomAccessIterator &__i,_Distance __n,random_access_iterator_tag){
    __i+=__n; //连续的，直接加
}

//根据给定的泛型,调用对对应的移动函数
template <class _InputAccessIterator,class _Distance>
inline void distance(_InputAccessIterator &__i,_Distance __n){
    __advance(__i,__n,__iterator_category(__i));
}
//advance是 将iterator移动，而distance是计算两个iterator直接的距离。
//__distance在调用的时候,要把迭代器的对象传过来,但是distance在调用的时候,不需要,指定泛型
__STL_END_NAMESPACE
#endif //SAKURA_YAN_ITERATOR_BASE_H