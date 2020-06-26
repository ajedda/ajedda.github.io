---
layout: post
title: "Types sub-lists, index sequences, and checking for redundant types in a type list"
categories: C++
keywords: programming; C++
comments: true
---

A direct application of ``get_type`` and types sets is *types subsets*. Given a set of types $T1$, ..., $Tn$, and a set of indices $i_1, \dots, i_m$, return the set containing $T{i_1}, \dots, T_{i_m}$ - all in C++ of course.  The code is simple (it requires some knoweldge of variadic templates syntax).

```cpp
template <typename TypeList, std::size_t... Ns> 
struct raw_sub_list
{
 using type = std::tuple<typename get_type<Ns, TypeList>::type...>;   
}; 

template <typename TypeList, typename Indices> 
struct sub_list; 

template <typename TypeList, std::size_t... Ns> 
struct sub_list<TypeList, std::index_sequence<Ns...>> : raw_sub_list<TypeList, Ns...>
{
}; 
```

.. and I should add these two interfaces: 

```cpp
template <typename TypeList, template <std::size_t...> typename IndexSeq, std::size_t... Ns>
struct sub_list<TypeList, IndexSeq<Ns...>> : raw_sub_list<TypeList, Ns...>
{
}; 

template <typename TypeList, typename Indices> 
using sub_list_t = typename sub_list<TypeList, Indices>::type; 
```

We call these as follows: 

```cpp
static_assert(std::is_same_v< sub_list<types_list, std::index_sequence<0, 2>>::type, std::tuple<int, short>>);
static_assert(std::is_same_v< raw_sub_list<types_list, 0, 2>::type, std::tuple<int, short>>);
```

What is ``std::index_sequence`` there? Here is an implementation of it: 

```cpp
template <std::size_t... Ns> 
struct index_seq {}; 
```

Index sequence is nothing but a wrapper for variadic parameters of type ``std::size_t...``. This allows you to have multiple index sequences and treat them differently. For example,  your template can have ``index_seq<1,2,3>``, ``index_seq<4,5,6>``, and ``index<7,8,9>`` and return ``index_seq<12, 15, 18>``. You can't do that without the wrapper. 

One important utility when dealing with ``index_seq`` is ``make_index_seq``. Given $n$, $m$, return the sequence ``index_seq<n, n+1, ...., m>``.  Designing this utility template is an interesting exercise. Interestingly, it only requires template specialization. But it also requires some utilities such as ``prepend`` and ``append``. Given an index $n$ and index sequence ``index<Ns...>``: ``prepend`` returns ``index_seq<N, Ns...>`` whereas ``append`` returns ``index_seq<Ns..., N>``. The implementation is straightforward. 

```cpp
template <std::size_t n, typename Indices> 
struct prepend; 

template <std::size_t n, std::size_t... Ns> 
struct prepend<n, index_seq<Ns...>>
{ 
    using type = index_seq<n, Ns...>; 
}; 

template <std::size_t n, typename Indices> 
using prepend_t = typename prepend<n, Indices>::type; 



template <std::size_t n, typename Indices> 
struct append; 

template <std::size_t n, std::size_t... Ns> 
struct append<n, index_seq<Ns...>>
{ 
    using type = index_seq<Ns..., n>; 
}; 

template <std::size_t n, typename Indices> 
using append_t = typename append<n, Indices>::type; 
```

We use it as follows: 

```cpp
static_assert(std::is_same_v< prepend_t<4, index_seq<1,2,3>>, index_seq<4, 1,2,3> >);
static_assert(std::is_same_v< append_t<4, index_seq<1,2,3>>, index_seq<1,2,3, 4> >);
```

These tools are all we need to implement ``make_index_seq```. 

```cpp

namespace detail 
{
    template <std::size_t I, std::size_t N, typename Indices> 
    struct make_index_seq;
    
    // When I == N. 
    template <std::size_t N, typename Indices>
    struct make_index_seq<N, N, Indices>
    {
       using type = prepend_t<N, Indices>;  
    }; 
    
    // When I != N. 
    template <std::size_t I, std::size_t N, typename Indices>
    struct make_index_seq
    {
       using sub_seq_type = typename make_index_seq<I+1, N, Indices>::type; 
       using type = prepend_t<I, sub_seq_type>; 
    }; 

}

template <std::size_t from, std::size_t to=0>
struct make_index_seq; 

// If you only provide one parameter (such as: make_index_seq<4>), we assume it starts from zero.
// see the default to is set to zero. 
template <std::size_t to> 
struct make_index_seq<to, 0> : detail::make_index_seq<0, to, index_seq<>> {};  

template <std::size_t from, std::size_t to> 
struct make_index_seq : detail::make_index_seq<from, to, index_seq<>> {}; 
```

We could have used concepts (or the ``require`` keyword), or SFINAE. The basic template specialization was just enough. 

In addition to ``prepend`` and ``append``, we can create ``concat``, which concatenate two index sequences. This can reduce the complexity of ``make_index_seq`` from linear to logarithmic. 

```cpp
template <typename Indices1, typename Indices2> 
struct concat; 

template <std::size_t... Ms, std::size_t... Ns> 
struct concat<index_seq<Ms...>, index_seq<Ns...>>
{ 
    using type = index_seq<Ms..., Ns...>; 
}; 

template <typename Indices1, typename Indices2> 
using concat_t = typename concat<Indices1, Indices2>::type;

// Use it as: 
// concat_t<index_seq<1,2,3>, index_seq<4,5,6>> 
// and expet index_seq<1,2,3,4,5,6>.  
``` 

With all of these tools, we can ask the following question: is there an integer (or any other type ``T``) in the first $m$ types within a tuple (or any other types list)? 

```cpp
static_assert(  type_exists_v<short, sub_list_t<  std::tuple<int, double, char, short>, make_index_seq<3>::type >>  );
static_assert(! type_exists_v<short, sub_list_t<  std::tuple<int, double, char, short>, make_index_seq<2>::type >>  );
```


That question allows us to answer another more interesting question: **are all the types in a types list unique?**

```cpp
// are all types in a type list unique?
namespace detail 
{
    template <typename TypeList, std::size_t first, std::size_t last>
    struct is_all_unique ; 
    
    // the base case. 
    template <typename TypeList, std::size_t last> 
    struct is_all_unique<TypeList, last, last> 
    {
        using head_type = get_type_t<last, TypeList>;
        using index_seq_type = typename ::make_index_seq<last-1>::type;
        using sublist_type = sub_list_t<TypeList,  index_seq_type >;     
        static constexpr bool value = ! type_exists_v<head_type, sublist_type>; 
    }; 
    
    // The head type should not exist in the sublist prior to head (i.e. from 0 to first-1). 
    template <typename TypeList, std::size_t first, std::size_t last> 
    struct is_all_unique
    {
         using head_type = get_type_t<first, TypeList>;
         // We are in namespace detail, so call the global namespace.
         using index_seq_type = typename ::make_index_seq<first-1>::type;  
         using sublist_type = sub_list_t<TypeList,  index_seq_type >; 
          
         static constexpr bool value = ! type_exists_v<head_type, sublist_type> && is_all_unique<TypeList, first+1, last>::value;
    };
    
    // some special cases. 
    // This is the case where there is only one type in the TypeList. (i.e., sizeof...(Args) = 1); 
    template <typename TypeList> 
    struct is_all_unique<TypeList, 1, 0> 
    {
        static constexpr bool value = true; 
    };
    
    // This is the case where there is no type in the TypeList. (i.e., sizeof...(Args) = 0); 
    template <typename TypeList> 
    struct is_all_unique<TypeList, 1, std::size_t(-1)> 
    {
        static constexpr bool value = true; 
    };
}

template <typename TypeList> 
struct is_all_unique; 

template <template <typename...> typename TypeList, typename... Args>
struct is_all_unique<TypeList<Args...>> : detail::is_all_unique< TypeList<Args...>, 
                                                            1, 
                                                            sizeof...(Args)-1>                                                        
{     
}; 

template <typename TypeList> 
constexpr static bool is_all_unique_v = is_all_unique<TypeList>::value;  
```


... and we can test them: 

```cpp
static_assert( is_all_unique_v<std::tuple<short>>); 
static_assert( is_all_unique_v<std::tuple<short, int>>);
static_assert(!is_all_unique_v<std::tuple<short, short>>);
static_assert( is_all_unique_v<std::tuple<short, int, float, double>>);
static_assert(!is_all_unique_v<std::tuple<short, int, float, short>>);
static_assert(!is_all_unique_v<std::tuple<short, int, int, float>>);
static_assert(!is_all_unique<std::tuple<short, int, char, float, float>>::value);
static_assert( is_all_unique<std::tuple<>>::value); 
```

Does ``is_all_unique_v`` have any application? Yes. One application I can think of is to check at compile time for ``std::variant`` that has redundant types because this redundancy cause compile errors. 

```cpp
std::variant<int, int> v;  v=10;  // error. 
std::variant<int, double> v; v=10; // Ok.  
```

What may have more applications is a template (or, function) that removes redundant types from a types list (i.e., convert it to a set). I will have to implement this. The next posts propose techniques and exercises that will take there. 