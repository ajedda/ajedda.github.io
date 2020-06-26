---
layout: post
title: "Improving get_type interface to accept enums"
categories: C++
keywords: programming; C++
---

I introduced in the previous post ``get_type``. Given a list of types, return the type with index ``N``, which is of type ``std::size_t``.  I get an error when I use ``get_type`` with an enum, as in the following example: 

```cpp
enum class Type { Integer = 0, Float, Double }; 
using T = typename get_type<Type::Integer, Integer, Float, Double>::type;
```

Therefore, I need to change my code to something like this: 
```cpp
enum class Type { Integer = 0, Float, Double }; 
using T = typename get_type<(std::size_t)Type::Integer, Integer, Float, Double>::type;
```

A solution to this is to define an interface for ``get_type`` that handles the ``Type`` enum. Then, we may need to use another enum and repeat the same steps. 

I don't like this. The main issue is that ``N`` is a non-type parameter. I could have created a concept if this was a type parameter and specialize for the case of numeric types and enums. This does not work, because we want the value of N, not its type.  I tried many solutions until my colleague at work told me about this new feature in C++17; *auto non-type template parameter*. Instead of defining ``N`` as ``std::size_t``, we will just use ``auto``. The compiler will decide. We should do more work. It will only change the interface. 

```cpp
template <auto N, typename... Args>
struct get_type : get_type_base<N, Args...>
{ 
}; 

template <auto n, template <typename...> typename TypeContainer, typename... Args> 
struct get_type<n, TypeContainer<Args...>> : get_type_base<n, Args...> {}; 
```

First, let's use a common base class ``get_type_base`` to avoid redundant code. You can hide it in the ``detail`` namespace.  As you can see, I am using ``auto N`` instead of ``std::size_t N``.  

```cpp
template <auto N, typename... Args>
struct get_type_base 
{
   static constexpr std::size_t n = detail::get_number(N); 
   using type = typename detail::get_type<n, Args...>::type; 
};
```

The new addition here is ``detail::get_number(N)``. All it does is to convert ``N``, whatever its type is, to ``std::size_t``. 

```cpp
namespace detail 
{

  constexpr std::size_t get_number(std::size_t n) { return n; } 
  
  template <typename T> 
  constexpr std::size_t get_number(T n) { return static_cast<std::size_t>(n); }
}
```

We reached our objective. 

```cpp
enum class Type { Integer = 0, Float, Double}; 
static_assert(std::is_same_v<get_type<Type::Integer, int, double>::type, int>);
```

This C++17 feature was not given the attention it deserves. 



