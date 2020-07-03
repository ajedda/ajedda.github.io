---
layout: post
title: "Does your type exist?"
categories: C++
keywords: programming; C++
comments: true
---

We implemented ``get_type`` in a previous post. Sometimes, we want to know if a type exists in a list of types. Simple techniques can solve this problem. We will still use template specialization to solve our problem. We will, however, introduce a new technique in this post. 


Let's start with a motivating example. We have a ``std::tuple``. We can get an element from this tuple using its type. That is, something like this: 

```cpp
std::tuple<int, bool, char, double> t; 
std::get<double>(t) = 3; 
std::cout << std::get<double>(t); 
```

The output here is 3. However, if you try to get a ``float`` (or any type that does not exist in the tuple arguments), you will get a compile error. Try ``std::get<float>(t)`` for example. We want to avoid this compilation error. We may want to return a default value, a pointer, or an optional. For example: 

```cpp
template <typename T, typename... Args>
std::optional<T> opt_get(std::tuple<Args...> const& t)
{
    if constexpr (has_type_v<T, Args...>)
    {
        return std::get<T>(t);    
    }
    return std::nullopt; 
} 
```

We can use it as: 
```cpp
auto print_opt = [](auto const& opt) { if (opt) { std::cout << *opt; } else { std::cout << "N";}}; 
print_opt(opt_get<double>(t)); 
print_opt(opt_get<float>(t)); 
```

This will output 3N. Believe me, this has some interesting applications. The problem of this post is how to design ``has_type_v``.  Let's start with the interface. 

```cpp
template <typename T, typename... Others>
struct has_type
{
   static constexpr bool value = detail::has_type<T, Others...>::value;  
}; 

template <typename T, typename... Others>
struct has_type<T, types_list<Others...>>
{
   static constexpr bool value = detail::has_type<T, Others...>::value;  
}; 


template <typename T, typename... Others>
struct has_type<T, std::tuple<Others...>>
{
   static constexpr bool value = detail::has_type<T, Others...>::value;  
};
```

.. or just have one interface: 
```cpp
template <typename T, template <typename...> typename TypeList, typename... Args>
struct has_type<T, TypeList<Args...>> : detail::has_type<T, Args...> 
{};

 
```

We are using a static constexpr boolean variable here instead of a type alias.  We named this boolean variable ``value``. In general, a type trait has a ``value``, a ``type``, or both. Now, let's get into the details. 

```cpp
namespace detail { 
template <typename T, typename... Others>
struct has_type; 

template <typename T, typename F>
struct has_type<T, F>
{
    static constexpr bool value = false; 
}; 

template <typename T>
struct has_type<T, T>
{
    static constexpr bool value = true; 
};
} 
```

The *base case* is when the arguments has only type. If the searched type ``T`` is the same as the type in ``Args``, then ``value`` is true, otherwise it is false. Occasionally we use type traits ``std::true_type`` and ``false_type`` instead of explicitly declarting ``value``. For example: 

```cpp
template <typename T, typename F>
struct has_type<T, F> : std::false_type {};  

template <typename T>
struct has_type<T, T> : std::true_type {}; 
```

When ``Args`` has more than one type, then we recursively remove one type from ``Args`` until the searched type is in the head of ``Args``, or we reach the base case. 

```cpp
template <typename T, typename... Others>
struct has_type<T, T, Others...> 
{
    static constexpr bool value = true; 
}; 

template <typename T, typename F, typename... Others>
struct has_type<T, F, Others...>
{
    static constexpr bool value = has_type<T, Others...>::value;  
}; 
```

This is it. It works. One more thing. What about ``has_type_v``? Well, here we will use a different type of aliases that appeared only in C++17, or better called *variable template*. This is one of those very powerful features that many C++ developers do not immediately appreciate.

```cpp
template <typename T, typename... Others>
static constexpr bool has_type_v = has_type<T, Others...>::value; 
```


Now, time for testing: 

```cpp
std::cout << has_type<int, int>::value; 
std::cout << has_type<int, bool>::value; 
std::cout << has_type<int, types_list<int, bool>>::value; 
std::cout << has_type<int, bool, char>::value; 
std::cout << has_type_v<int, bool, char, int>;    
std::cout << has_type_v<int, types_list<int, bool, char, double>>; 
std::cout << has_type_v<int, std::tuple<int, bool, char, double>>; 
```

The output should be 1010111. 