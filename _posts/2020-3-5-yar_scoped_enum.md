---
layout: post
title: Yet another reason to use scoped enumerations
categories: C++
keywords: programming; C++
---


I was dealing with an annoying bug. It is night, not feeling good, not much patience. GCC (gcc 9.2 to be more specific) is telling me that I cannot use a constructor of a class I've written. Let's call it _A_. 

```cpp
struct A  : public Interface
{
    A() = default; 
    A(int a1_, int a2_) : a1{a1_}, a2{a2_} {}
    int a1;  
    int a2; 
};
``` 

.. and I am calling it with _std::make_unique_

```cpp
auto a_ptr = std::make_unique<A>(1,2);
```

The error message is just annoying as you can imagine. It is too long, I won't copy it. 

I tried to call the constructor as: 
```cpp
auto a = A{1,2}; 
```

The error message, still annoying, but shorter. 

```cpp
main.cpp:28:15: error: expected ',' or ';' before '{' token
   28 |     auto a = A{1,2};
```

After many tries here and there, and losing confidence in my memory, and having doubts about my career as a developer, I realized that the problem was: 

```cpp 
enum Type { A = 0, B = 1 }; 
```

I fixed it by adding _class_ to my enum declaration (i.e., convert it from unscoped to scoped enumeration).

```cpp 
enum class Type { A = 0, B = 1 }; 
```

There is no bug in the compiler. Everything follows the standards. The part of the specifications that talks about this behavior can be found in Section 9.1: 

> A class declaration introduces the class name into the scope where it is declared and hides any class, variable,
function, or other declaration of that name in an enclosing scope (3.3). *If a class name is declared in a scope
where a variable, function, or enumerator of the same name is also declared, then when both declarations
are in scope, the class can be referred to only using an elaborated-type-specifier (3.4.4)*


This means that we can also solve this problem if we declared the A instance with the prefix *class* or *struct*.  For example: 

```cpp
auto a_ptr = std::make_unique<class A>(1,2);
auto a = struct A{1,2}; 
```

I don't think C++ developers prefer to use this C-like syntax. A scoped enum is definitely a better (and more modern) choice. 


The standards gives an example of this behavior. It is something similar to this: 

```cpp
struct stat {}; 
void stat(struct stat*) {} 
// stat* p; // compiler error.
struct stat* p; 
stat(p); 
```

Most naming guidelines in C++ would have solved this problem as well. For example, structs names usually start with an upper case letter in some guidelines, or terminate with "_t" suffix. That would have prevented us from some confusion. 

Compilers errors like the ones we say above can happen all the time. They are worse if you are under pressure (e.g., deadline, outage resolution, interview, etc ...) and specifically when you are dealing with large source code. You may save quite a lot of time (and maybe a bad impression about you) if you just add the keyword **class** in your enums. And that is *yet another reason to use scoped instead of unscoped enumerations*. 



![I just like this picture. No meaning at all.](https://media.treehugger.com/assets/images/2019/07/angeltree.jpg.1200x0_q70_crop-smart.jpg)