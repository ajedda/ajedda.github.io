---
layout: post
title: "Filtering Index Sequences"
categories: C++
keywords: programming; C++
---

Assume you are given an index sequence, you are asked to filter all odd numbers.  Filtering problems like this are not uncommon when dealing with lists, in fact ``filter`` is as common as ``map`` and ``reduce`` in functional programming.  We want to design a filter for index sequences (at compile time of course).  We will start with this simple problem of filtering odd numbers. Then, I will introduce a generalized filter. I will use that to solve the following problem: given a type list ``T``, find a corresponding types list ``U`` with all the unique types in ``T``.  This is not an imaginary problem. It has some interesting applications. For example, in [*], a tuple iterator was introduced. The dereference operator returns a variant of the types within the tuple. That variant should not have redundant types. That's what I can think of now. Let's get into the details. 


*How to filter odd numbers from an index sequence?* The interface to our filter is as all the others we had:

```cpp
template <typename IndexSeq> 
struct filter_evens : detail::filter_evens<IndexSeq>
{
};

template <typename IndexSeq> 
using filter_evens_t = typename filter_evens<IndexSeq>::type; 
```

Now, let's get into the details. We will use ``std::conditional``. If the head of the index sequence is not even, we prepend it to the following result, and we do this recursively with the rest of the sequence. The code is a better explainer. 

```cpp
// in namespace detail.
template <std::size_t N, std::size_t... Ns> 
struct filter_evens<index_seq<N, Ns...>>
{       
	using type = std::conditional_t<(N%2==0),  
                                     typename filter_evens<index_seq<Ns...>>::type,
                                     prepend_t<N, typename filter_evens<index_seq<Ns...>>::type>  >;
                                        
    }; 
```

What about the base case? It is quite easy to implement. 

```cpp
template <> 
struct filter_evens<index_seq<>>
{
	using type = index_seq<>;
}; 
```

We use ``filter_evens`` as follows: 

```cpp
static_assert(std::is_same_v<filter_evens_t<index_seq<2,3,4,5>>, index_seq<3,5>>); 
```

As you can see from ``filter_evens`` that we are filtering based on the result of ``N%2==0``. Can we pass a function instead? and what form should this function be in?  Since ``N`` here (i.e., the index sequence head) is a template parameter, we will want ``N`` to be also a template parameter to our function ``F``. That is, we want to call it as ``F<N>()``, or ``F(N)`` as we usually do.  Another approach is to create ``F`` as a struct. 

```cpp
template <std::size_t N>
struct F 
{
	static constexpr bool value = (N%2 == 0); 
};
```
and hence we call it as ``F<N>::value``.  Which approach is better? If ``F`` is a function, we can use ``if constexpr``. And if ``F`` is a struct we can use partial specialization. Anyway, why do I care about partial specialization here? Well, I was thinking of adding additional optional parameters to  ``F``, something like this:

```cpp
template <std::size_t N, typename... OptPs>
struct F
{
// some logic here. 
};
``` 

These additional parameters are types since we are dealing with types lists here. Therefore, calling ``F`` as ``F(N)`` (or ``F(N, OptPs...)`` is no longer valid), and we are stuck with ``F<N, OptPs...>()``. I chose to implement ``F`` as a struct. It is more coherent with what we have in the previous posts.  Ok, it is time to implement the general filter ``filter_index_seq``.  First, the interface: 

```cpp
template <typename IndexSeq, template <auto, typename...> typename F, typename... FArgs> 
struct filter_index_seq : detail::filter_index_seq<IndexSeq, F, FArgs...>
{
};

template <typename IndexSeq, template <auto, typename...> typename F, typename... FArgs> 
using filter_index_seq_t = typename filter_index_seq<IndexSeq, F, FArgs...>::type;
``` 

So, ``template <auto, typename...> typename F`` means that ``F`` is a template that takes an auto variable (usually it will be ``std::size_t``, and optional parameters ``FArgs...``.   

We will replace ``(N%2==0)`` in the previous example with: 

```cpp
constexpr static bool fn_eval = F<N, FArgs...>::value;
```

.. and now we have everything to implement ``filter_index_seq``. 

```cpp
namespace  detail 
{
    template <typename IndexSeq, template <auto, typename...> typename F, typename... FArgs>
    struct filter_index_seq;     

    template <template <auto, typename...> typename F, typename... FArgs> 
    struct filter_index_seq<index_seq<>, F, FArgs...>
    {
        using type = index_seq<>;
    }; 
    
    template <template <auto, typename...> typename F, typename... FArgs, auto N, auto... Ns> 
    struct filter_index_seq<index_seq<N, Ns...>, F, FArgs...>
    {
        constexpr static bool fn_eval = F<N, FArgs...>::value; 
        using type = std::conditional_t<fn_eval,  
                                        typename filter_index_seq<index_seq<Ns...>, F, FArgs...>::type,
                                        prepend_t<N, typename filter_index_seq<index_seq<Ns...>, F, FArgs...>::type>>;
                                        
    }; 
    
}
```


Let's test this.  First, we will create an ``F`` that filters even numbers. 

```cpp
template <std::size_t i> 
struct IsEven
{
   constexpr static bool value = (i % 2 == 0);  
}; 
```

... and use it as: 

```cpp
static_assert(std::is_same_v<filter_index_seq_t<index_seq<2,3,4,5>, IsEven>, index_seq<3,5>>); 
```

Let's do a more complex example where we use extra parameters. This extra parameter is a variant (or any other types list). We want the indexes of all types that are not of type ``double``.

```cpp
using DList = std::variant<double, int, char, int>; 
    static_assert(std::is_same_v<filter_index_seq_t<index_seq<0, 1, 2, 3>, IsIndexOfDouble, DList>, index_seq<1,2,3>>); 
```
... where ``IsIndexOfDouble`` is: 
```cpp
template <std::size_t i, typename TL> 
struct IsIndexOfDouble
{
   constexpr static bool value = std::is_same_v<get_type_t<i, TL>, double>;  
}; 
```



Nothing stops from doing more complex logic, especially when you can pass more types in addition to ``TL``.


Now, we get into the main application. Removing redundant types. Let's define our ``F`` such that it filters an index if its corresponding type already exists in types list we are treating. 

```cpp
template <std::size_t i, typename TL> 
struct IsPreviouslyOccured
{
    using current_type = get_type_t<i, TL>; 
    using p_types_sublist = sub_list_t<TL, typename ::make_index_seq<i-1>::type>; 
    constexpr static bool value = type_exists_v< current_type, p_types_sublist>;

}; 


template <typename TL> 
struct IsPreviouslyOccured<0, TL> 
{
   constexpr static bool value = false;  
};
```

This is all we need. 

```cpp
using u_index_dlist = filter_index_seq_t<index_seq<0,1,2,3>, IsPreviouslyOccured, DList>; 
// Why <0,1,2,3>? because DList has 4 types. Of course, you can use something prettier than that.  
using list_unique_types = sub_list_t< DList,  u_index_dlist >;
static_assert(std::is_same_v< list_unique_types, std::variant<double, int, char>>);
```

Can we do it differently? Obviously yes. Perhaps, it will be better as well (i.e., less generated code from the templates and faster compilation time). My objective in these posts is to introduce as many ideas and concepts as I can.  I have to admit, I can do a better job in the terms I am using in these posts. I also continuously rename my variables. I hope I reach a point where I have a list of techniques that help library developers.  I hope you enjoyed this post. 