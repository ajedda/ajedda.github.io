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

I don't like this code. I am redefining the ``get_type`` code for the case ``types_list<Args...>`` and ``Args...``. I would have done the same if I wanted a specialized for ``std::tuple<Args...>``. I should have written one implementation, perhaps for the ``Args...`` case. Then, define multiple interfaces that call the same implementation. We want to do something like this: 

```cpp
template <std::size_t N, typename... Others>
struct get_type<N, types_list<Others...>>
{
    using type = typename get_type<N, Others...>::type; 
}; 
```

Unfortunately, this will not work. It is ambiguous. Which template specialization should we call? The ambiguity is coming from specializing the ``std::size_t N`` argument. Here is what we should do instead; use a ``detail`` namespace. Actually, *always use a ``detail`` namespace to hide your implementation in template programming*. Here is how: 

```cpp
// The implementation. 
namespace detail
{
    template <std::size_t N, typename... Args> 
    struct get_type;
    
    template <typename T, typename... Others>
    struct get_type<0, T, Others...> 
    {
        using type = T;    
    };

    template <std::size_t N, typename T, typename... Others>
    struct get_type<N, T, Others...>
    {
        static_assert(sizeof...(Others) + 1 > N, "Out of range"); 
        using type = typename get_type<N-1, Others...>::type; 
    }; 
}

// The interface
template <std::size_t N, typename... Args> 
struct get_type; 

template <std::size_t N, typename... Others>
struct get_type
{
   using type = typename detail::get_type<N, Others...>::type;  
};

template <std::size_t N, typename... Others>
struct get_type<N, types_list<Others...>>
{
   using type = typename detail::get_type<N, Others...>::type; 
}; 

template <std::size_t N, typename... Others>
struct get_type<N, std::tuple<Others...>>
{
   using type = typename detail::get_type<N, Others...>::type; 
}; 
```

Ok, one last thing. We use sometimes use ``std::conditional_t`` instead of writing ``std::conditional``. That will save us from writing ``type`` as in ``std::conditional<b, T, F>::type``. How can we do this? This is quite simple as well. We just need to create an alias as follows: 

```cpp
template <std::size_t N, typename... Others> 
using get_type_t = typename get_type<N, Others...>::type; 
```

This post was just a simple exercise in template programming. We wanted a generalization of ``std::conditional`` that helps us to get a type in a types list using an index at compile-time. We did not need anything other than variadic template specialization. However, we dealt with few techniques. 1) the use of the ``type`` alias within a struct to get the result, 2) the use of the ``detail`` namespace to hide the interface from the implementation (and we saw how this helped us in solving some ambiguties), and 3) the use of the ``*_t`` alias to access the ``type`` alias.  These techniques are frequently used in the STL type traits. As we get involved in harder exercises, we will note that these techniques (and of course more complex ones) are frequently used. 

