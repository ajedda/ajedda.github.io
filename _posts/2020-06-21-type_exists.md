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
    if constexpr (type_exists_v<T, Args...>)
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

This will output 3N. Believe me, this has some interesting applications. The problem of this post is how to design ``type_exists_v``.  Let's start with the interface. 

```cpp
template <typename T, typename... Others>
struct type_exists
{
   static constexpr bool value = detail::type_exists<T, Others...>::value;  
}; 

template <typename T, typename... Others>
struct type_exists<T, types_list<Others...>>
{
   static constexpr bool value = detail::type_exists<T, Others...>::value;  
}; 


template <typename T, typename... Others>
struct type_exists<T, std::tuple<Others...>>
{
   static constexpr bool value = detail::type_exists<T, Others...>::value;  
};
```

We are using a static constexpr boolean variable here instead of a type alias.  We named this boolean variable ``value``. In general, a type trait has a ``value``, a ``type``, or both. Now, let's get into the details. 

```cpp
namespace detail { 
template <typename T, typename... Others>
struct type_exists; 

template <typename T, typename F>
struct type_exists<T, F>
{
    static constexpr bool value = false; 
}; 

template <typename T>
struct type_exists<T, T>
{
    static constexpr bool value = true; 
};
} 
```

The *base case* is when the arguments has only type. If the searched type ``T`` is the same as the type in ``Args``, then ``value`` is true, otherwise it is false. Occasionally we use type traits ``std::true_type`` and ``false_type`` instead of explicitly declarting ``value``. For example: 

```cpp
template <typename T, typename F>
struct type_exists<T, F> : std::false_type {};  

template <typename T>
struct type_exists<T, T> : std::true_type {}; 
```

When ``Args`` has more than one type, then we recursively remove one type from ``Args`` until the searched type is in the head of ``Args``, or we reach the base case. 

```cpp
template <typename T, typename... Others>
struct type_exists<T, T, Others...> 
{
    static constexpr bool value = true; 
}; 

template <typename T, typename F, typename... Others>
struct type_exists<T, F, Others...>
{
    static constexpr bool value = type_exists<T, Others...>::value;  
}; 
```

This is it. It works. One more thing. What about ``type_exists_v``? Well, here we will use a different type of aliases that appeared only in C++17, or better called *variable template*. This is one of those very powerful features that many C++ developers do not immediately appreciate.

```cpp
template <typename T, typename... Others>
static constexpr bool type_exists_v = type_exists<T, Others...>::value; 
```


Now, time for testing: 

```cpp
std::cout << type_exists<int, int>::value; 
std::cout << type_exists<int, bool>::value; 
std::cout << type_exists<int, types_list<int, bool>>::value; 
std::cout << type_exists<int, bool, char>::value; 
std::cout << type_exists_v<int, bool, char, int>;    
std::cout << type_exists_v<int, types_list<int, bool, char, double>>; 
std::cout << type_exists_v<int, std::tuple<int, bool, char, double>>; 
```

The output should be 1010111. 