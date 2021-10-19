---
layout: post
title: "Contracts: Pre and Post Conditions"
categories: C++
keywords: programming; C++
---

You are working on some new code. You read the following comment. 

```cpp
void foo(std::string const& x) 
{   
    // We assume that x is in {"ABC", "XYZ", "IJK"}
    ... 
}
```

This is assumption is a contract bewteen `foo` and its caller.  This is just a comment though. What if the caller doesn't know about this contract. To be safe, we decide to rewrite `foo` as follows. 


```cpp
void foo(std::string const& x)
{
    assert(x == "ABC" || x == "XYZ" || x == "IJK"); 
    ...
}
```

You may think that an extra work that will make `foo` unnecessarily slower. This is true only in debug mode, as `assert` does nothing when not in debug mode (or, if `NDEBUG` is defined).  Definition of `assert` is as follows: 

```cpp
#ifdef NDEBUG
#define assert(condition) ((void)0)
#else
#define assert(condition) /*implementation defined*/
#endif
```

You may not like the limitations of `assert`: 
1. You may not want to stop the program when the contract is broken. 
2. The function where `assert` is written may not be covered by your tests, so you want to have some behaviour in release mode. 
3. You may want a completly different behaviour (e.g., logging a mesage, throwing an excpetion, etc ...). 

Macros are nice in these situations. Let's define `Expects`.

```cpp
#define Expects(b) {\
if (!(b)) \
{\
    std::ostringstream os;\
    os << "Fatal " << #b << " at line: " << __LINE__ << " at function: " << __FUNCTION__ << '\n';   \
    throw std::runtime_error{os.str()};\
}\
}
```

Obviously you can do whatever you want. Similiar to `assert`, we will want to have two behaviours; one for debug and another for release. 

```cpp
#if DEBUG_MODE
#define Expects(b) assert(b)
#else 
.. 
.. 
#endif 
```

Now, your function `foo` can be rerwitten as follow. 

```cpp
void foo(std::string const& x)
{
    Expects(x == "ABC" || x == "XYZ" || x == "IJK"); 
    ...
}
```

That was a _pre-condition_. What about _post-conditions_? In a post-conidition, we want to _ensure_ that the result after some statments meet some condition. 

```cpp
void foo(std::string const& x)
{
    Expects(x == "ABC" || x == "XYZ" || x == "IJK"); 
    .. 
    ..
    int res = bar(y)
    Ensures(res >= 0); 
}
```

In my experience, `Ensures` has the same definition of `Expects`. Thus: 

```cpp
#define Ensures(b) Expects(b)
```

That's it. I take these tools wherever I go. There is no need for unnecessary comments or if-statements. Specially, there is no need for this terrible ubiquitous if-statment below.

```cpp
void foo(Widget* ptr)
{
    if (!ptr) { Log << "ptr is null"; }
}
```

I hope you find this useful. 