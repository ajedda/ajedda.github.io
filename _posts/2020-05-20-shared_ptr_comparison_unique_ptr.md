---
layout: post
title: "Shared Pointers - Part 6: Comparison with unique_ptr custom deleters"
categories: C++
keywords: programming; C++
---


There is a big difference between the custom deleter in ``std::shared_ptr`` and its counterpart in ``std::unique_ptr``. It is an interesting difference that I will talk about in this post. 

If you remember from the previous post, adding a custom deleter to the shared pointer was done using the constructor. The class definition did not change. This was made possible because we moved the deleter into reference counter. In code words: 

```cpp
auto sp = std::shared_ptr<int>{new int, [](auto* ptr) {delete ptr; }}; 
static_assert(std::is_same_v<std::shared_ptr<int>, decltype(sp)>); 
```

This is not the case in ``std::unique_ptr``. The type of the deleter is part of ``std::unique_ptr`` class definition. This means that two unique pointers with different deleters do not have the same type, even if the underlying pointer type is the same.  Let me give you some examples of how to give ``std::unique_ptr`` custom deleters. 

```cpp
// default deleter. 
std::unique_ptr<int> p1{new int{10}};  


// With a default constructible deleter. 
struct DefConstDeleter 
{ 
   void operator()(int*) 
   { 
   std::cout << "Calling Def Cons Deleter \n"; 
   } 
}; 
int x{10}; 
std::unique_ptr<int, DefConstDeleter> p2{&x};
std::unique_ptr<int, DefConstDeleter> p2a{&x, DefConstDeleter{}}; 
//std::unique_ptr<int> p2b{&x, DefConstDeleter{}};  // This will not compile. 


// With copy constructible deleter: 
struct CopyConstDeleter 
{
   CopyConstDeleter(std::string const& msg) : _msg{msg} {} 
   void operator()(int*) 
   { 
    std::cout << "Calling copy const deleter with msg: " << _msg << '\n'; 
   }
   std::string _msg; 
};

std::unique_ptr<int, CopyConstDeleter> p3{&x, CopyConstDeleter{"from main"}};  


// It works with lambdas, but not with function pointers. 
void func_deleter(int*) 
{ 
std::cout << "calling func deleter \n"; 
}
//std::unique_ptr<int, decltype(func_deleter)> p5{&x, func_deleter};

auto lambda_deleter = [](int*) { std::cout << "lambda deleter \n"; }; 
std::unique_ptr<int, decltype(lambda_deleter)> p4{&x, lambda_deleter};
```

In summary, 1) the type of the unique pointer depends on the deleter's type, and 2) you can't have a function as a deleter for an ``std::unique_ptr``. Both of these are not true for ``shared_ptr``.   

```cpp
std::shared_ptr<int> sp1{new int{10}}; 
std::shared_ptr<int> sp2{&x, DefConstDeleter{}}; 
std::shared_ptr<int> sp3{&x, CopyConstDeleter{"from main"}}; 
std::shared_ptr<int> sp4{&x, lambda_deleter}; 
std::shared_ptr<int> sp5{&x, func_deleter};        
    
// They are all the same type. It doesn't hurt to check. 
static_assert(std::is_same_v<decltype(sp1), decltype(sp2)>);
static_assert(std::is_same_v<decltype(sp2), decltype(sp3)>);
static_assert(std::is_same_v<decltype(sp3), decltype(sp4)>);
```

So, what is the source of these differences? The design objective of a unique_ptr is to make it as close as possible to a raw pointer (except that the resources are freed at the constructor). This includes similarities in the unique_ptr size and its corresponding raw pointer. How does a unique_ptr have the same size of the underlying pointer, yet have different deleters? The answer is empty base optimization.  The deleter in unique_ptr is not a member variable. It is a class that ``unique_ptr`` inherits from. The class definition of a ``unique_ptr`` is something like this: 


```cpp
template <typename T, typename Deleter> 
class unique_ptr : Deleter<T>
{
// ... 
	T* _ptr; 
}
```

instead of this: 
```cpp
template <typename T, typename Deleter> 
class unique_ptr 
{
	T* _ptr;
	Deleter<T> _deleter; 
}; 
```

The size of the second class must be greater than sizeof(T*), even if ``Deleter<T>`` has no member variables. In the first case, the class size can be equal to sizeof(T*) if ``Deleter<T>`` has no member variables. This is called empty member optimization. C++20 introduces a way to define ``Deleter<T>`` as a member variable while keeping the benefits of empty base optimization.  

**An application:** I found this beneficial while dealing once with a *stream factory*. The objective is to design a factory function that returns either an file ostream, or the standard output stream ``std::cout``.   Both streams are of the same base type ``std::ostream``. One way to do this is to wrap the returned stream inside a shared_ptr. For example: 

```cpp
auto stream_factory = [](bool use_cout) -> std::shared_ptr<std::ostream>
{     
    if (use_cout) 
    { 
        auto noop_deleter = [](auto* p) {}; 
        return std::shared_ptr<std::ostream>{&std::cout, noop_deleter}; 
    }
    else 
    { 
        auto fs = new std::ofstream{"hello.txt"}; 
        return std::shared_ptr<std::ostream>{fs};
    }
};
```
That, in the way I wrote it, will not work with unique_ptr. As an exercise, try to find a different way to do this.  

 I have to admit as well that you can do it (in the case of ``std::ostreams``) with unique pointers only. The solution is provided in the code below.  **However**, keep in mind that you may still end up with the scenario above. 

```cpp
auto stream_factory = [](bool use_cout) -> std::unique_ptr<std::ostream>
{     
    if (use_cout) 
    { 
        return std::make_unique<std::ostream>(std::cout.rdbuf()); 
    }
    else 
    { 
        return std::make_unique<std::ofstream>("hello.txt"); 
    }
};
```