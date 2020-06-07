---
layout: post
title: "Shared Pointers - Part 6: Multi-threaded locking policies (specializing functionalities)"
categories: C++
keywords: programming; C++
---

Everything we have seen so far about this tutorial's implementation of ``std::shared_ptr`` is single-threaded. My focus was (and still is) on the software design techniques used by gcc to implement this library.  My objective in the next posts is to introduce a multi-threaded implementation of ``std::shared_ptr``. 

First, there are few methods we can use to implement a multi-threaded ``std::shared_ptr``. That will depend on the *lock policy* that we are using. For example, we can use an *atomic reference counter*, *mutexes*, or simply assume that our environment is always single-threaded (and hence we just continue using the single-threaded design). We need to specialize our implementation according to the lock policy used.  Let's introduce an enum class for the ``LockPolicy``. 

```cpp
enum class LockPolicy
{
	single, 
	atomic, 
	mutex
}; 
```

This *LockPolicy* can be an additional template variable to our shared pointer class.  The signature of ``std::shared_ptr`` may end up such as: 

```cpp
template <typename T, LockPolicy P=default_lock_policy()>
class shared_ptr; 
```
where ``default_lock_policy()`` is dependent on the environment. But this is not what we have in the standards.  The implementation of gcc hides this as follows: 

```cpp 
template <typename T, LockPolicy P> 
class __shared_ptr<T, P>; 

template <typename T> 
class shared_ptr : __shared_ptr<T, default_lock_policy()>
{
.. 
} 
```

A side note: you can use ``__shared_ptr`` and specify your own locking policy if you want. This code will not be portable though.  

```cpp
#include <bits/shared_ptr_base.h> 

std::__shared_ptr<int> _sp_in_default;  
std::__shared_ptr<int, std::_Lock_policy::_S_single> _sp_in_single; 
```


Anyway, the only two functions that we would like to specialize are ``acquire`` and ``release``, which are in ``ref_counter_ptr_base``.   This will force us to include the ``LockPolicy`` template variable in pretty much every class we wrote.  Our classes will have the following signatures: 

```cpp
template <class T, LockPolicy P> 
struct shared_ptr_counter;

template <class T, typename Deleter, LockPolicy P> 
struct ref_counter_ptr_deleter; 

template <class T, LockPolicy P> 
struct ref_counter_ptr_default; 
```

and finally: 
```cpp
template <typename T, LockPolicy P> 
struct ref_counter_ptr_base
{
	...
	void acquire(); 
	void release(); 
	...
	T* _ptr; 
	uint32_t _ref_cntr; 
}; 
```

**How to specialize ``acquire`` and ``release`` according to the LockPolicy used?** I can think of at least three ways to do that. Let's start with the **first method: converting a partial specialization to full specialization**: 

I would like to write something like: 

```cpp
template <typename T> 
void ref_counter_ptr_base<T, LockPolicy::single>::acquire() 
{ ... } 

template <typename T> 
void ref_counter_ptr_base<T, LockPolicy::atomic>::acquire() 
{ ... } 

.. etc ... 
```

This will *not* work. You *cannot* partially specialize a function. To avoid *partial template specialization*, we can make ``ref_counter_ptr_base`` be a template of the LockPolicy only. This means, ``ref_counter_ptr_base`` will no more hold a member variable of type ``T*``. The children of ``ref_counter_ptr_base`` will do instead.  This leads to a refactoring of ``ref_counter_ptr_base`` and its children as follows: 

```cpp
template <LockPolicy P>  
struct ref_counter_ptr_base
{
   uint32_t _ref_cntr; // the only member variable here. 
   
   virtual void dispose() = 0; 
   // (compared to: 
   // virtual void dispose(T* ptr) = 0; 
   // )
   // .. and some functions. 
};  

template <class T, LockPolicy P> 
struct ref_counter_ptr_default final : public ref_counter_ptr_base<P>
{
	T* _ptr; 
	void dispose() override; 
}; 

template <class T, typename Deleter, LockPolicy P> 
struct ref_counter_ptr_deleter final : public ref_counter_ptr_base<P>
{
   T* _ptr; 
   Deleter _deleter; 
   void dispose() override; 
} 
```

That first method is pretty nice. This is what gcc is doing now. It is not easy to do. It may result in a lot of refactoring. 

The **second method** use the C++17 feature **if constexpr**. It makes the code much easier. However, not everyone has the luxury of using C++17. With if-constexpr, we will keep the template ``ref_counter_ptr_base`` as a function of ``T`` and ``LockPolicy``. The implementation of ``acquire`` will look like this: 

```cpp
template <typename T, LockPolicy> 
void ref_counter_ptr_base<T, P>::acquire() 
{ 
	if constexpr (P == LockPolicy::atomic) 
	{
		acquire_atomic(...); 
	} 
	else if constexpr (P == LockPolicy::atomic) 
	{
		acquire_mutex(...);
	} 
	else { acquire_single(...); }
} 
```

It may look more modern and cooler, but it has some main issues. The ``acquire_*`` functions will have to be static in some scenarios (e.g., when different specialization use different member variables. We will see a case of that in this tutorial). Therefore, we should call them with references to all the variables that may be used. For example, ``acquire_single`` may be called with ``acquire_single(_ref_cntr, _ptr)``, etc ... . 



The **third method** is to **invoke a fully specialized callable class instead of a function**. Here is an example of how to do this: 

```cpp
template <LockPolicy P> 
struct private_acquire; 

template <typename T, LockPolicy P> 
struct some_other_ref_counter
{
   void acquire(); 
   T v;  
};

template <typename T, LockPolicy P>
void some_other_ref_counter<T, P>::acquire()
{ 
    private_acquire<P>::acquire(v); 
} 

template <LockPolicy P> 
struct private_acquire 
{
    template <typename T> 
    static void acquire(T v);
};

template <>
template <typename T> 
void private_acquire<LockPolicy::single>::acquire(T v) { std::cout << "single private_acquire \n"; }

template <>
template <typename T> 
void private_acquire<LockPolicy::atomic>::acquire(T v) { std::cout << "atomic private_acquire \n"; }

template <>
template <typename T> 
void private_acquire<LockPolicy::mutex>::acquire(T v) { std::cout << "mutex private_acquire \n"; }

``` 
 

This shares the same cons of the if-constexpr method plus the extra verbosity.  It also has a bad limitation. All of the private acquire functions receive the same set of input variables. 

I like the **first method** for this shared pointer we are designing here. The main reason is that I don't like continuously passing the member variables to my specialized functions (e.g., ``acquire_single(_ref_cntr, _ptr)``). 

This is it for this post. The implementation details are in the next post. 