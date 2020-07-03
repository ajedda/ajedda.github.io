---
layout: post
title: "Rewriting has_type with std::is_same"
categories: C++
keywords: programming; C++
comments: true
---


Let's re-examine the code of ``has_type``. First, the base case: 

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

All this two specialization do is to check if ``T`` is the same as ``F``. There is a nice type trait in STL that does exactly this; ``std::is_same``. Great. Now, how can we use this with the other two specializations for ``has_type``?

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

Well, note that we are doing is an Or statement. We can replace all of that with: 

```cpp
template <typename T, typename F>
struct has_type<T, F> : std::is_same<T, F> {};  

template <typename T, typename F, typename... Others>
struct has_type<T, F, Others...>
{
    static constexpr bool value =  std::is_same_v<T, F> || has_type<T, Others...>::value;  
};
```

This works. C++17 introduced a feature that will further shorten this code; *fold expression*. It is, again, one of these features that you will only appreciate if you do template programming.  Our code now will work with no specializations at all. 

```cpp
template <typename T, typename... Args> 
struct has_type
{
   static constexpr bool value = (std::is_same_v<T, Args> || ...);  
};
```

Here is something to go out with from this post; never underestimate the new features in C++ and never estimate what is available in STL.