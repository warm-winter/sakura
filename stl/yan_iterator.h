//
// Created by YCNN on 2020-05-26.
//

#ifndef SAKURA_YAN_ITERATOR_H
#define SAKURA_YAN_ITERATOR_H

#include "yan_config.h"
#include "yan_iterator_base.h"

__STL_BEGIN_NAMESPACE
template <class _Container>
class back_insert_iterator{
protected:
    _Container *container;
public:
    typedef _Container container_type;
    typedef output_iterator_tag iterator_category;
    typedef void value_type;
    typedef void difference_type;
    typedef void pointer;
    typedef void reference;

    //explicit 修饰类构造函数，表示该构造函数是显示的
    //implicit 修饰类构造函数，隐式（默认的）
                                                     //对参数列表赋值
    explicit back_insert_iterator<_Container &__x> : container( &__x){}

    back_insert_iterator<_Container> &operator=(const typename _Container::value_type &__value){
        container->push_back(__value);//调用了容器的push_back函数
        return *this;
    }
    //???这三个重载我有点迷
    //重载操作符，暂时没有实现任何功能，就是把迭代器返回了
    back_insert_iterator<_Container> &operator*(){return *this}
    back_insert_iterator<_Container> &operator++(){return *this}
    back_insert_iterator<_Container> &operator(int){return *this}
};

template <class _Container>
//从无到1
inline back_insert_iterator<_Container> back_inserter(_Container &__x){
    return back_insert_iterator<_Container>(__x);
}

template <class _Container>
class front_insert_iterator{
protected:
        _Container *container;
public:
    typedef _Container container_type;
    typedef output_iterator_tag iterator_category;//五种迭代器的类型标识
    typedef void value_type;//迭代器所指对象的类型
    typedef void difference_type;//表示两个迭代器之间的距离的
    typedef void pointer;//迭代器所致对象的指针类型
    typedef void reference;//迭代器所指对象的引用类型

    explicit front_insert_iterator(_Container &__x):container(&__x){}

    front_insert_iterator<_Container> &operator=(const typename _Container::value_type &__value){
        container->push_front(__value);
        return *this;
    }
    front_insert_iterator<_Container> &operator*(){return *this;}
    front_insert_iterator<_Container> &operator++(){return *this;}
    front_insert_iterator<_Container> &operator++(int){return *this;}
    };

template <class _Container>
inline front_insert_iterator<_Container> front_inserter(_Container &__x){
    return front_insert_iterator<_Container>(__x);
}

 //   _Container是指定的泛型
template <class _Container>
class insert_iterator{
protected:
        _Container *container;
        typename _Container::iterator iter;
public:
    typedef _Container *container;
    typedef output_iterator_tag iterator_category;
    typedef void value_type;
    typedef void difference_type;
    typedef void pointer;
    typedef void reference;
//构造函数
    insert_iterator(_Container &__x,typename _Container::interator __i):container(&__x),iter(__i){}

    insert_iterator<_Container> &operator=(const typename _Container::value_type &__value){
        iter = container->insert(iter,__value);
        ++iter;
        return *this;

    }
    insert_iterator<_Container> &operator*(){return *this;}
    insert_iterator<_Container> &operator++(){return *this;}
    insert_iterator<_Container> &operator++(int){return *this;}
    };
//reverse_iterator的迭代方向和iterator相反。
// 即若r1和r2指向同一个元素，r1+1和r2-1指向同一个元素

template <class _Iterator>
class reverse_iterator{
protected:
    _Iterator current;
public:
    typedef typename iterator_traits<_Iterator>::iterator_category iterator_category;
    typedef typename iterator_traits<_Iterator>::value_type value_type;
    typedef typename iterator_traits<_Iterator>::difference_type difference_type;
    typedef typename iterator_traits<_Iterator>::pointer pointer;
    typedef typename iterator_traits<_Iterator>::reference reference;

    typedef _Iterator iterator_type;
    typedef reverse_iterator<_Iterator> _Self;

public:
    reverse_iterator(){}
    explicit reverse_iterator(iterator_type __x):current(__x){}
    reverse_iterator(const _Self &__x):current(__x.current){}

    template<class _Iter>
    reverse_iterator(const reverse_iterator<_Iter> &__other):current(__other.base())

    iterator_type base() const{
        return current;
    }

    reference operator*() const {
        _Iterator __tmp = current;
        return *--__tmp;
    }

    pointer operator->() const {
        return &(operator*())
    }

    _Self &operator++(){
        --current;//逆反的
        return *this;
    }

    _Self operator++(int){
        _Self __tmp = *this;
        --current;
        return __tmp;
    }
    _Self &operator--(){
        ++current;
        return *this;
    }
    _Self &operator--(int){
        _Self __tmp = *this;
        ++current;
        return __tmp;
    }
    _Self operator+(difference_type __n) const {
        return _Self (current - __n);
    }
    _Self operator-(difference_type __n)const {
        return _Self (current + __n);
    }
    _Self &operator+=(difference_type __n){
        current -= __n;
        return *this;
    }
    _Self &operator-=(difference_type __n){
        current += __n;
        return *this;
    }
    //用下标获取
    reference operator[](difference_type __n) const {
        return *(*this + __n);
    }
};

    template<class _Iterator>
    inline bool operator==(const reverse_iterator<_Iterator> &__lhs,const reverse_iterator<_Iterator> &__rhs){
        return __lhs.base() == __rhs.base();
    }

    template<class _Iterator>
    inline bool operator!=(const reverse_iterator<_Iterator> &__lhs,const reverse_iterator<_Iterator> &__rhs){
        return !(__lhs == __rhs);
    }
    //因为是逆迭代器，所以大小是反的
    template<class _Iterator>
    inline bool operator<(const reverse_iterator<_Iterator> &__lhs,const reverse_iterator<_Iterator> &__rhs){
        return __rhs.base() < __lhs.base();
    }

    template<class _Iterator>
    inline bool operator>(const reverse_iterator<_Iterator> &__lhs,const reverse_iterator<_Iterator> &__rhs){
        return __rhs < __lhs;
    }

    template<class _Iterator>
    inline bool operator<=(const reverse_iterator<_Iterator> &__lhs,const reverse_iterator<_Iterator> &__rhs){
        return !(__rhs < __lhs);
    }
    //上面那个是类里面的函数实现的，
    //这里是外面对操作符重载，所以需要传入reverse_iterator
    template<class _Iterator>
    reverse_iterator<_Iterator> operator+(typename reverse_iterator<_Iterator>::difference_type __n,const reverse_iterator<_Iterator> &__x){
        return reverse_iterator<_Iterator>(__x.base() - __n);
    }

    template<class _Iterator>
    typename reverse_iterator<_Iterator>::difference_type operator-(const reverse_iterator<_Iterator> &__lhs,const reverse_iterator<_Iterator>&__rhs){
        return __rhs.base() - __lhs.base();
    }
__STL_END_NAMESPACE
#endif //SAKURA_YAN_ITERATOR_H
