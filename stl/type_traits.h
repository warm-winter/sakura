//
// Created by YCNN on 2020-05-27.
//

#ifndef SAKURA_TYPE_TRAITS_H
#define SAKURA_TYPE_TRAITS_H

struct __true_type{

};
struct __false_type{

};
template<class _Tp>
        //trivial 琐碎的
struct __type_traits{
    typedef __true_type this_dummy_member_must_be_first;
 /*不要删除此成员。它通知编译器

自动专门研究

__类型特征模板是特殊的。它只是确保

如果一个实现使用一个模板

被称为不相关的特质。*/
    typedef __false_type has_trivial_default_constructor;
    typedef __false_type has_trivial_copy_constructor;
    typedef __false_type has_trivial_assignment_operator;
    typedef __false_type has_trivial_destructor;
    typedef __false_type is_POD_type;
/*为了

自动生成特定类型专门化的编译器

本班学生：

-如果您愿意的话，您可以重新排列下面的成员

-如果您愿意，可以删除下面的任何成员

-如果没有相应的

编译器中的名称更改

-您添加的成员将被视为普通成员，除非

您可以在编译器中添加适当的支持。
 * */
};

template<>
struct __type_traits<bool>{
typedef __true_type has_trivial_default_constructor;
typedef __true_type has_trivial_copy_constructor;
typedef __true_type has_trivial_assignment_operator;
typedef __true_type has_trivial_destructor;
typedef __true_type is_POD_type;

};
template<char>{
    typedef __true_type has_trivial_default_constructor;
    typedef __true_type has_trivial_copy_constructor;
    typedef __true_type has_trivial_assignment_operator;
    typedef __true_type has_trivial_destructor;
    typedef __true_type is_POD_type;
}
//unsigned char:char不是可以表示整数嘛，所以会有有符号和无符号的区别
template<>
struct __type_traits<signed char >{
    typedef __true_type has_trivial_default_constructor;
    typedef __true_type has_trivial_copy_constructor;
    typedef __true_type has_trivial_assignment_operator;
    typedef __true_type has_trivial_destructor;
    typedef __true_type is_POD_type;
};
template<>
struct __type_traits<unsigned char >{
    typedef __true_type has_trivial_default_constructor;
    typedef __true_type has_trivial_copy_constructor;
    typedef __true_type has_trivial_assignment_operator;
    typedef __true_type has_trivial_destructor;
    typedef __true_type is_POD_type;
};
template<>
struct __type_traits<short>{
    typedef __true_type has_trivial_default_constructor;
    typedef __true_type has_trivial_copy_constrcutor;
    typedef __true_type has_trivial_assignment_operator;
    typedef __true_type has_trivial_destrucor;
    typedef __true_type is_POD_type;

};
template<>
    struct __type_traits<unsigned short >{
        typedef __true_type has_trivial_default_constructor;
        typedef __true_type has_trivial_copy_constructor;
        typedef __true_type has_trivial_assignment_operator;
        typedef __true_type has_trivial_destructor;
        typedef __true_type is_POD_type;
    };
//特化
template<>
struct __type_traits<int>{
    typedef __true_type has_trivial_default_constructor;
    typedef __true_type has_trivial_copy_contrucotr;
    typedef __true_type has_trivial_assignment_operator;
    typedef __true_type has_trivial_destructor;
    typedef __true_type is_POD_type;
};
template<>
struct __type_traits<unsigned int >{
    typedef __true_type has_trivial_default_constructor;
    typedef __true_type has_trivial_copy_constructor;
    typedef __true_type has_trivial_assignment_operator;
    typedef __true_type has_trivial_destructor;
    typedef __true_type is_POD_type;
};
template<>
struct __type_traits<long>{
    typedef __true_type has_trivial_default_constructor;
    typedef __true_type has_trivial_copy_construtor;
    typedef __true_type has_trivial_assignment_operator;
    typedef __true_type has_trivial_destructor;
    typedef __true_type is_POD_type;
};
template<>
struct __type_traits<unsigned long>{
    typedef __true_type has_trivial_default_constructor;
    typedef __true_type has_trivial_copy_constructor;
    typedef __true_type has_trivial_assignment_operator;
    typedef __true_type has_trivial_destructor;
    typedef __true_type is_POD_type;
};
template<>
struct __type_traits<long long>{
    typedef __true_type has_trivial_default_constructor;
    typedef __true_type has_trivial_copy_constructor;
    typedef __true_type has_trivial_assignment_operator;
    typedef __true_type has_trivial_destructor;
    typedef __true_type is_POD_type;
};
template<>
struct __type_traits<unsigned long long >{
    typedef __true_type has_trivial_default_constructor;
    typedef __true_type has_trivial_copy_constructor;
    typedef __true_type has_trivial_assignment_operator;
    typedef __true_type has_trivial_destructor;
    typedef __true_type is_POD_type;
};
template<>
struct __type_traits<float>{
    typedef __true_type has_trivial_default_constructor;
    typedef __true_type has_trivial_copy_constructor;
    typedef __true_type has_trivial_assignment_operator;
    typedef __true_type has_trivial_destructor;
    typedef __true_type is_POD_type;
};
template<>
struct __type_traits<double>{
    typedef __true_type has_trivial_default_constructor;
    typedef __true_type has_trivial_copy_constructor;
    typedef __true_type has_trivial_assignment_operator;
    typedef __true_type has_trivial_destructor;
    typedef __true_type is_POD_type;
};
template<>
struct __type_traits<long double>{
    typedef __true_type has_trivial_default_constructor;
    typedef __true_type has_trivial_copy_constructor;
    typedef __true_type has_trivial_assignment_operator;
    typedef __true_type has_trivial_destructor;
    typedef __true_type is_POD_type;
};
template<class _Tp>
    struct _Is_interger{
        typedef __false_type _Interger;
    };

template<>
struct _Is_interger<int>{
    typedef __true_type _Interger;
};
template<>
struct _Is_interger<unsigned int >{
    typedef __true_type _Interger;
};
template<>
struct _Is_interger<bool>{
    typedef __true_type _Integer;
};
template<>
struct _Is_interger<char>{
    typedef __true_type _Integer;
};
template<>
struct _Is_interger<signed char>{
    typedef __true_type _Integer;
};
template<>
struct _Is_interger<unsigned char>{
    typedef __true_type _Integer;
};
template<>
struct _Is_interger<short>{
    typedef __true_type _Integer;
};
template<>
struct _Is_interger<unsigned short >{
    typedef __true_type _Integer;
};
template<>
struct _Is_interger<long>{
    typedef __true_type _Integer;
};
template<>
struct _Is_interger<unsigned long>{
    typedef __true_type _Integer;
};

#endif //SAKURA_TYPE_TRAITS_H
