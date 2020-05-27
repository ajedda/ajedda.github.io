---
layout: post
title: "A Type Erased Deleter for std::unique_ptr"
categories: C++
keywords: programming; C++
---


In the previous post, we gave a comparison between the deleters in ``std::shared_ptr`` and those in ``std::unique_ptr``.  I introduced a stream factory function where I am returning ``std::shared_ptr<std::ostream>>`` instead of ``std::unique_ptr<std::ostream>>`` just because of that interesting property of deleters in ``std::shared_ptr``.  I will show in this post a way to use ``std::unique_ptr`` in that factory function.  I will introduce a *type-erased deleter* class, which is a wrapper that holds a pointer to any deleter type.  This technique, called *Type Erasure with Templates* may be useful in many other scenarios.  I will also show that ``std::shared_ptr`` reference counting uses the exact same technique. Let's start: 

Thes stream factory function we were dealing with was: 
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


First, we want our deleter to be 1) a callable class, and 2) delete a pointer of type T. In other words, we want it to implement ``operator()(T* ptr)``. 

```cpp
template <typename T> 
struct  TypeErasedDeleter
{
	...
	void operator()(T* ptr); 
	...
}; 
```

``TypeErasedDeleter`` should have a pointer to a polymorphic ``DeleterBase`` pointer. The implementation is of that ``DeleterBase`` is what will make the difference later. ```TypeErasedDeleter`` when called, should call the deleter of that pointer. 

```cpp
template <typename T> 
struct DeleterBase
{ 
	virtual operator()(T* ptr); 
	virtual ~DeleterBase() = default; 
}; 

template <typename T> 
struct TypeErasedDeleter
{
	void operator()(T* ptr) { (*_deleter_ptr)(); }
	
	std::unique_ptr<DeleterBase> _deleter_ptr; 
} 
```

Now, how can we set ``_deleter_ptr`` to point to any type of a deleter? Same way ``std::shared_ptr`` did. That is, using the constructor.  That is, we will have a constructor that looks a follows: 

```cpp
template <typename DeleterType> 
TypeErasedDeleter(DeleterType const& d)
 : _deleter_ptr{std::make_unique<D>(d)}
{} 
```

.. But wait a minute. If we do this, then ``DeleterType`` must inherits from ``DeleterBase``.  Although possible, this is just too much headache, and very limiting (for example, you won't be able to create a lambda, or a function, or a third-party deleter).  The solution is quite easy; another level of indirection.  We will wrap it around a class ``DeleterImpl`` defined as follows: 

```cpp
template <typename T, typename DeleterType>
struct DeleterImpl : DeleterBase<T>
{
  DeleterImpl(DeleterType const& d) : _d{d} {} 
  void operator()(T* ptr) override {_d(ptr); } 
  DeleterType _d; 
};   
```

Note, ``DeleterType`` is what the user provides. ``DeleterImpl`` is nothing but a wrapper. Note also, ``DeleterImpl::operator()`` is virtual, but ``DeleterType::operator()`` does not have to be (but it must be implemented). 

Now, what happens if you don't want to provide a deleter? then, a default deleter should be called. This should be implemented in ``DeleterBase::operator()``. 

Overall code looks like this: 

```cpp
namespace ver3
{
  
  template <typename T>
  struct DeleterBase
  {
    virtual void operator()(T* ptr) 
    { 
        // This is the default behaviour. 
        // In practice, we should take care of the case when ptr is 
        // a dynamically allocated array. In other words, calling 
        // delete may cause an issue. The solution is quite easy, 
        // but we will discuss it in future posts. 
        delete ptr; 
    }
    virtual ~DeleterBase() = default;
  }; 

  template <typename T, typename DeleterType>
  struct DeleterImpl : DeleterBase<T>
  {
     DeleterImpl(DeleterType const& d) : _d{d} {} 
     void operator()(T* ptr) override {_d(ptr); } 
     DeleterType _d; 
  }; 
  
  template <typename T>
  struct TypeErasedDeleter
  {
    // When deleter provided: use DeleterBase as a deleter. 
    TypeErasedDeleter() 
        : deleter{std::make_unique<DeleterBase<T>>()}
    {} 
    
    template <typename DeleterType>
    TypeErasedDeleter(DeleterType const& d) 
        : deleter{std::make_unique<DeleterImpl<T, DeleterType>>(d)}
    {} 
    
    void operator()(T* ptr) { (*deleter)(ptr); }
    std::unique_ptr<DeleterBase<T>> deleter; 
  }; 
    
} 
```

Compare these classes with the reference counter classes in ``std::shared_ptr``. You will notice that it is the same pattern, *Type Erasure with Templates*. 

In practice, we want to put everything together. We want to convert ``DelterBase`` and ``DeleterImpl`` to inner classes.  It will simplify the code a little bit. This is what it will look like. 

```cpp
 template <typename T>
  struct TypeErasedDeleter
  {
    
    TypeErasedDeleter() 
        : _deleter_ptr{std::make_unique<DeleterBase>()}
    {} 
    
    template <typename DeleterType>
    TypeErasedDeleter(DeleterType const& d) 
        : _deleter_ptr{std::make_unique<DeleterImpl<DeleterType>>(d)}
    {} 
    
    void operator()(T* ptr) { (*_deleter_ptr)(ptr); }
    
     

    struct DeleterBase
    {
      virtual void operator()(T* ptr) { std::cout << "unimplemented deleter \n";  delete ptr; }
      virtual ~DeleterBase() = default;
    }; 

    template <typename DeleterType>
    struct DeleterImpl : DeleterBase
    {
       DeleterImpl(DeleterType const& d) : _d{d} {} 
       void operator()(T* ptr) override {_d(ptr); } 
       DeleterType _d; 
    };    


    std::unique_ptr<DeleterBase> _deleter_ptr; 
  }; 
```

So how will all of this work now? 
First I want to create a type alias: 

```cpp
template <typename T> 
using unique_ptr_deleter = std::unique_ptr<T, TypeErasedDeleter<T>>; 
```

... the tests will look as follow: 
```cpp
struct DefDeleter 
{
   void operator()(int* ptr) { std::cout << "def deleter \n"; delete ptr;  } 
}; 

struct AnotherDeleter 
{
   AnotherDeleter(int x) : _x{x} {}
   void operator()(int* ptr) { std::cout << "another  deleter " << _x << '\n'; delete ptr;  }
   int _x{}; 
};

void func(int* ptr) { std::cout << "func deleter \n"; delete ptr; }


unique_ptr_deleter<int> up1{new int, DefDeleter{}}; 
unique_ptr_deleter<int> up2{new int, AnotherDeleter{5}}; 
unique_ptr_deleter<int> up3{new int}; 
unique_ptr_deleter<int> up4{new int, [](auto* ptr) { std::cout << "from lambda \n"; delete ptr; }}; 
unique_ptr_deleter<int> up5{new int, &func}; // You were not able to do with std::unique_ptr. 

```

One last thing to check is the size of the new pointer. It is not just a single pointer now. It is two pointers: ``T*`` and ``unique_ptr<DeleterBase>``.   Now, we can write our stream factory function with ``unique_ptr_deleter``. 

```cpp
   auto stream_factory = [](bool use_cout) -> unique_ptr_deleter<std::ostream>
   {     
    if (use_cout) 
    { 
        auto noop_deleter = [](auto* p) {}; 
        return unique_ptr_deleter<std::ostream>{&std::cout, noop_deleter}; 
    }
    else 
    { 
        auto fs = new std::ofstream{"hello.txt"}; 
        return unique_ptr_deleter<std::ostream>{fs};
    }
   };
```

