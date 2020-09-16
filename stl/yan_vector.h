//
// Created by YCNN on 2020-05-29.
//

#ifndef SAKURA_YAN_VECTOR_H
#define SAKURA_YAN_VECTOR_H

#include "yan_config.h"
#include "yan_alloc.h"
#include "yan_iterator.h"
#include "yan_uninitialized.h"

__STL_BEGIN_NAMESPACE

template<class _Tp,class _Alloc>
class _Vector_base{
public:
    typedef _Alloc allocator_type;
    allocator_type get_allocator()const{return allocator_type ();}

    _Vector_base(const _Alloc &):_M_start(0),_M_finish(0),_M_end_of_storage(0){}

    _Vector_base(size_t __n.const_Alloc &):_M_start(0),_M_finish(0),_M_end_of_storage(0){
        _M_start = _M_allocate(__n);
        _M_finish = _M_start;
        _M_end_of_storage = _M_start + __n;
    }
    ~_Vector_base(){
        _M_deallocate(_M_start,_M_end_of_storage - _M_start);

    }

protected:
    _Tp *_M_start;
    _Tp *_M_finish;
    _Tp *_M_end_of_storage;

    typedef simple_alloc<_Tp,_Alloc> _M_data_allocator;
    _Tp *_M_allocate(size_t __n){
        return _M_data_allocator::allocate(__n);
    }
    void _M_deallocate(_Tp *__p,size_t __n){
        _M_data_allocator::deallocate(__p,__n);
    }
};

template<class _Tp,class _Alloc = alloc>
class vector: protected _Vector_base<_Tp,_Alloc>{
private:
    typedef _Vector_base<_Tp,_Alloc>_Base;
public:
    typedef _Tp value_type;
    typedef value_type *pointer;
    typedef const value_type *const_pointer;
    typedef value_type *iterator;
    typedef const value_type *const_iterator;
    typedef value_type &reference;
    typedef const value_type &const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef typename _Base::allocator_type allocate_type;

    allocate_type get_allocate() const{return _Base::get_allocator();}

    typedef reverse_iterator<const_iterator> const_reverse_iterator;
    typedef reverse_iterator<iterator> reverse_iterator;
protected:
    using _Base::_M_allocate;
    using _Base::_M_deallocate;
    using _Base::_M_end_of_storage;
    using _Base::_M_finish;
    using _Base::_M_start;

protected:
    void _M_insert_aux(iterator __position,const _Tp &__x);
    void _M_insert_aux(iterator __position);

public:
    iterator begin()
    {
        return _M_start;
    }
    const_iterator begin()const {
        return _M_start;
    }
    iterator end(){
        return _M_finish;
    }
    const_iterator end()const {
        return _M_finish;
    }
    reverse_iterator rbegin(){
        return reverse_iterator(end());
    }
    const_reverse_iterator rbegin()const{
        return reverse_iterator(end());
    }
    reverse_iterator rend(){
        return reverse_iterator(begin());
    }
    const_reverse_iterator rend()const{
        return reverse_iterator(begin());
    }
    size_type size const{
        return size_type(end() - begin());
    };
    size_type max_size() const {
        return size_type(-1) / sizeof(_Tp);
    }
    size_type capacity() const{
        return size_type (_M_end_of_storage - begin());
    }
    bool  empty() const{
        return begin() == end();
    }
    reference operator[](size_type __n){
        return *(begin() + __n);
    }
    const_reference operator[](size_type __n)const{
        return *(begin() + __n);
    }
    void _M_range_check(size_type __n) const{
        if(__n>=size){
            throw;
        }
    }
    const_reverse_iterator at(size_type __n) const{
        _M_range_check(__n);
        return (*this)[__n];
    }
    explicit vector(const allocate_type &__a = get_allocate()):_Base(__a){

    }
    vector(size_type __n,const _Tp &__value,const allocate_type &__a = allocate_type()):_Base(__n,__a){
        _M_finish = uninitialized_fill_n(_M_start,__n,__value);
    }
    explicit vector(size_type __n):_Base(__n,allocate_type()){
        _M_finish = uninitialized_fill_n(_M_start,__n,_Tp());
    }
    vector(const vector<_Tp,_Alloc> &__x):_Base(__x.size(),__x.get_allocator()){
        _M_finish = uninitialized_copy(__x.begin(),__x.end(),_M_start);
    }

    template<class _InputIterator>
    vector<_InputIterator __first,_InputIterator __last,const allocate_type &__a = allocate_type()):_Base(__a){
    typedef typename _Is_integer<_InputIterator>::_Integral Integral;
    _M_initialize_aux(__first,__last,_Integral());
}

template <class _Integer>
void _M_initialize_aux(_Integer __n,_Integer __value,__true_type){
    _M_start = _M_allocate(__n);
    _M_end_of_storage = _M_start + __n;
    _M_finish = uninitialize_fill_n(_M_start,__n,__value);
}
template<class _InputIterator>
void _M_initialize_aux(_InputIterator __first,_InputIterator __last,__false_type){
    _M_range_initialize(__first,__last,__ITERATOR_CATEGORY(__first));
}
~vector(){
    destroy(_M_start,_M_finish);
}
vector<_Tp,_Alloc> &operator=(const vector<_Tp,_Alloc> &__x);
void reverse(size_type __n){
    if(capacity() < __n){
        const size_type __old_size = size();
        iterator __tmp = _M_allocate_and_copy(__n,_M_start,_M_finish);
        destroy(_M_start,_M_finish);
        _M_deallocate(_M_start,_M_end_of_storage - _M_start);
        _M_start = __tmp;
        _M_finish = __tmp + __old_size;
        _M_end_of_storage = _M_start + __n;
    }
}
void assign(size_type __n,const _Tp &__val){
    _M_fill_assign(__n,__val);
}
void _M_fill_assign(size_type __n,const _Tp &__val);

    template<class _InputIterator>
    void assign(_InputIterator __first,_InputIterator __last){
        typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
        _M_assign_dispatch(__first,__last,_Integral());
    }

    template<class _Integer>
    void _M_assign_dispatch(_Integer __n,_Integer __val,__true_type){
        _M_fill_assign((size_type )__n,(_Tp) __val);
    }

    template<class _InputIterator>
    void _M_assign_dispatch(_InputIterator __first,_InputIterator __last,__false_type){
        _M_assign_aux(__first,__last,__ITERATOR_CATEGORY(__first));
    }

    template<class _InputIterator>
    void _M_assign_aux(_InputIterator __first,_InputIterator __last,input_iterator_tag);

    template<class _ForwardIterator>
    void _M_assign_aux(_ForwardIterator __first,_ForwardIterator __last,forward_iterator_tag);

    reference front(){
        return *begin();
    }
    const_reference front() const{
        return *begin();
    }
    reference back(){
        return *(end()-1);
    }
    const_reference back() const{
        return *(end()-1);
    }
    void push_back(const _Tp &__value){
        if(_M_finish != _M_end_of_storage)
        construct(_M_finish,__value);
        ++_M_finish;
    }else{
        _M_insert_aux(end(),__value);
    }


}

void push_back(){
    if(_M_finish != _M_end_of_storage){
        construct(_M_finish);
    }else{
        _M_insert_aux(end());
    }
}
void swap(vector<_Tp,_Alloc> &__x){
    if(this != &__x){
        sakura_stl::swap(_M_start,__x._M_start);
        sakura_stl::swap(_M_finish,__x._M_finish);
        sakura_stl::swap(_M_end_of_storage,__x._M_end_of_storage);
    }
}
iterator insert(iterator __position,const _Tp &__x){
    size_type __n = __position - begin();
    if(_M_finish != _M_end_of_storage && __position == end()){
        construct(_M_finish,__x);
        ++_M_finish;
    }
    else{
        _M_insert_aux(__position,__x);
    }
    return begin( ) + __n;
}

template<class _InputIterator>
void insert(iterator _pos,_InputIterator __first,_InputIterator __last){
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_insert_dispatch(__pos,__first,__last,_Integral());
}

template<class _Integer>
void _M_insert_dispatch(iterator __pos,_Integer __n,_Integer __val,__true_type){
    _M_fill_insert(__pos,(size_type)__n,(_Tp) __val);
}

template<class _InputIterator>
void _M_insert_dispatch(interator __pos,_InputIterator __first,_InputIterator __last,__false_type){
    _M_range_insert(__pos,__first,__last,__ITERATOR_CATEGORY(__first));
}
void insert(iterator __pos,size_type __n,const _Tp &__x){
    _M_fill_insert(__pos,__n,__x);
}
void _M_fill_insert(iterator __pos,size_type __n,const _Tp &__x);

void pop_back(){
    --_M_finish;
    destroy(_M_finish);
}
iterator erase(iterator __position){
    if(__position + 1 != end()){
        copy(__position + 1,_M_finish,__position);
    }
    --_M_finish;
    destroy(_M_finish);
    return __position;
}
iterator erase(iterator __first,iterator __last){
    iterator __i = copy(__last,_M_finish,__first);
    destroy(__i,_M_finish);
    _M_finish = _M_finish -(__last - __first);
    return __first;
}
void resize(sizt_type __new_size,const _Tp &__x){
    if(__new_size < size()){
        erase(begin() + __new_size,end())
    }else{
        insert(end(),__new_size - size(),__x);
    }

}
void resize(size_type __new_size){
    resize(__new_size,_Tp());
}
void clear(){
    erase(begin(),end());
}
protected:
template<class _ForwardIterator>
iterator _M_allocate_and_copy(size_type __n,_ForwardIterator __first,_ForwardIterator __last){
    iterator __result = _M_allocate(__n);
    try{
        uninitialized_copy(__first,__last,__result);
        return __result;
    }catch(...){
        _M_deallocate(__result,__n);
        throw;
    }
}

template<class _InputIterator>
void _M_range_initialize(_InputIterator __first,_InputIterator __last,input_iterator_tag){
    for(;__first != __last;++__first){
        push_back(*__first);
    }
}

template<class _ForwardIterator>
void _M_range_initialize(_ForwardIterator __first,_ForwardIterator __last,forward_iterator_tag){
    size_type __n = 0;
    distance(__first,__last,__n);
    _M_start = _M_allocate(__n);
    _M_end_of_storage = _M_start + __n;
    _M_finish = uninitialize_copy(__first,__last,_M_start);

}

template<class _InputIterator>
void _M_range_insert(iterator __pos,_InputIterator __first,_InputIterator __last,input_iterator_tag);

template<class _ForwardIterator>
void _M_range_insert(iterator __pos,_ForwardIterator __first,_ForwardIterator __last,forward_iterator_tag);
};
template<class _Tp,class _Alloc>
inline bool operator==(const vector<_Tp,_Alloc >&__x,const vector<_Tp,_Alloc> &__y){
    return __x.size() == __y.size() && equal(__x.begin(),__x.end(),__y.begin());
}

template<class _Tp,class _Alloc>
inline bool operator<(const vector<_Tp,_Alloc> &__x,const vector<_Tp,_Alloc>&__y){
    return lexicographical_compare(__x.begin(),__x.end(),__y.begin(),__y.end());
}

template<class _Tp,class _Alloc>
vector<_Tp,_Alloc >&vector<_Tp,_Alloc>::operator=(const vector<_Tp,_Alloc> &__x ){
    if(this != &__x){
        const size_type __xlen = __x.size();
        if(__xlen > capacity()){
            iterator __tmp = _M_allocate_and_copy(__xlen,__x.begin(),__x.end());
            destroy(_M_start,_M_finish);
            destroy(_M_start,_M_finish);
            _M_deallocate(_M_start,_M_end_of_storage - _M_start);
            _M_start = __tmp;
            _M_end_of_storage = _M_start + __xlen;
        }else if(__xlen <= size()){
            iterator __i = copy(__x.begin(),__x.end(),begin());
            destroy(__i,_M_finish);
        }else{
            copy(__x.begin(),__x.begin()+size(),_M_start);
            uninitialize_copy(__x.begin()+size(),__x.end(),_M_finish);
        }
        _M_finish = _M_start + __xlen;
    }
    return *this;

}

template<class _Tp,class _Alloc>
void vector<_Tp,_Alloc>::_M_fill_assign(size_type __n, const _Tp &__val) {
    if(__n > capacity()){
        vector<_Tp,_Alloc> __tmp (__n,__val,get_allocate())；
        __tmp.swap(*this);
    }else if(__n > size()){
        fill(begin(),end(),__val);
        _M_finish = uninitialized_fill_n(_M_finish,__n - size(),__val);
    }else{
        erase(fill_n(begin(),__n,__val),end());
    }
}

template<class _Tp,class _Alloc>
template<class _InputIterator>
void vector<_Tp,_Alloc>::_M_assign_aux(_InputIterator __first, _InputIterator __last, input_iterator_tag) {
    iterator __cur = begin();
    for(;__first != __last && __cur != end();++__cur,++__first)
        *__cur = *__first;
    if(__first == __last)
        erase(__cur,end());
    else
        insert(end(),__first,__last);

}

template<class _Tp,class _Alloc>
template<class _ForwardIter>
void vector<_Tp,_Alloc>::_M_assign_aux(_ForwardIter __first, _ForwardIter __last, forward_iterator_tag) {
    size_type __len = 0;
    distance_type(__first,__last,__len);
    if(__len > capacity()){
        iterator __tmp = _M_allocate_and_copy(__len,__first,__last);
        destroy(_M_start,_M_finish);
        _M_deallocate(_M_start,_M_end_of_storage - _M_start);
        _M_start = __tmp;
        _M_end_of_storage = _M_finish = _M_start + __len;
    }else if(size() >= __len){
        iterator __new_finish = copy(__first,__last,_M_start);
        destroy(__new_finish,_M_finish);
        _M_finish = __new_finish;
    }else{
        _ForwardIter __mid = __first;
        __advance(__mid,size());
        copy(__first,__mid,_M_start);
        _M_finish = uninitialize_copy(__mid,__last,_M_finish);
    }
}

template<class _Tp,class _Alloc>
void vector<_Tp,_Alloc>::_M_insert_aux(iterator __position,const _Tp &__x) {
    if(_M_finish != _M_end_of_storage){
        construct(_M_finish,*(_))
    }
}





__STL_END_NAMESPACE


#endif //SAKURA_YAN_VECTOR_H
