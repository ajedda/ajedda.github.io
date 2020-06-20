---
layout: post
title: "Generalization of std::conditional"
categories: C++
keywords: programming; C++
---

In the previous post, we have a seen a code like this: 

```cpp
template <LockPolicy P> 
struct ref_counter_ptr_base 
{
  using counter_type = typename std::conditional<P==LockPolicy::atomic, std::atomic<uint32_t>, uint32_t>::type;
  counter_type _ref_cntr; 
};
```

.. but what if I wanted to select a type from a list of types? That is, ``counter_type`` is different for each lock policy? We want a generalization of ``std::conditional``. We want a *type getter*.

We want to call our type getter as follows: 

```cpp
using T1 = typename get_type<(std::size_t) e_type::Int, int, double, float>::type; 
static_assert(std::is_same_v<T1, int>); 
    
using T2 = typename get_type<2, int, double, float>::type; 
static_assert(std::is_same_v<T2, float>);
```

We may want to pack the types in a packer (or, a type list) as follows: 
```cpp
using types_list = types_list<int, double, short>; 
static_assert(std::is_same_v<get_type<0, types_list>::type, int>); 
static_assert(std::is_same_v<get_type<1, types_list>::type, double>);
static_assert(std::is_same_v<get_type<2, types_list>::type, short>);
```

This is the introduction of a series of posts on template programming in C++. We will build on the concepts introduced here. The first concept, and the easiest, is ``type_list``. It is nothing but an empty variadic template struct defined as follows: 

```cpp
template <typename... Args> 
struct types_list{};
```

What about ``get_type``? It is also a template struct with some specialization. 

```cpp
template <std::size_t N, typename... Args> 
struct get_type; 
```

The first specialization (or, the base case) is when N is equal to zero. 

```cpp
template <typename T, typename... Others>
struct get_type<0, types_list<T, Others...>> 
{
    using type = T;    
};
```

When N is greater than 1 (note that N is ``std::size_t`` and hence unsigned), we recursively call ``get_type<N-1 .... `` as follows: 

```cpp
template <std::size_t N, typename T, typename... Others>
struct get_type<N, types_list<T, Others...>>
{
   using type = typename get_type<N-1, types_list<Others...>>::type; 
}; 
```

What happens if N is greater than the size of the list? A very bad error will occur. We can add a static_assert message to improves the compiler error message as follows: 


```cpp
template <std::size_t N, typename T, typename... Others>
struct get_type<N, types_list<T, Others...>>
{
   static_assert(N < 1 + sizeof...(Others), "Out of range" ); 
   using type = typename get_type<N-1, types_list<Others...>>::type; 
}; 
```


What if we don't want to use types_list? We can further specialize as follows: 

```cpp
template <typename T,  typename... Others>
struct get_type<0, T, Others...> 
{
    using type = T;    
};


template <std::size_t N, typename T, typename... Others>
struct get_type<N, T, Others...>
{
   static_assert(N < 1 + sizeof...(Others), "Out of range" );
   using type = typename get_type<N-1, Others...>::type; 
}; 
```

Trying to specialize for the cases ``get_type<N, T1, T2, T3>`` and ``get_type<T, types_list<T1, T2, T3>>`` may lead to some undesired results in some extreme cases. For example, what if T1 is of type ``types_list<>``, while the others are int and double? For this reason, I prefer to have different ``get_type`` classes, or just document very well how ``get_type`` should be used. 

There is a lot of interesting tricks we can do by building on top of this. This post is the first of a series that tackles template programming in C++. 

