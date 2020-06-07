---
layout: post
title: "Shared Pointers - Part 7: Multi-threaded locking policies (atomic and mutex based design)"
categories: C++
keywords: programming; C++
---


We organized our code to provide three different implementations of ``shared_ptr``, according to the lock policy.  I will get into the implementation details in this post. Note, that I won't be using the exact gcc implementation here. First, they use non-standard concurrency tools. Second, their implementation provides support to weak pointers, which I don't.   

The most important scenario we would like to avoid when implementing a multithreaded shared pointer is having two threads deleting a pointer twice.  This is why we need a locking policy.  You will notice that the tools provided by C++ will make solving this problem quite easy. 

**Atomic Lock Policy**: In this lock policy, the reference counter is atomic. That is, ``ref_counter_ptr_base::_ref_cntr`` is of type ``std::atomic<int32_t>`` instead of type ``int32_t``.  This introduces an interesting problem. How can we implement this? I will introduce three ways to do this in the following, all based on template specialization.

The **first method** changes the type of ``_ref_cntr``  to a  template struct, as follows: 
```cpp
template <LockPolicy P> 
struct ref_counter_ptr_base 
{
  ref_counter_t<LockPolicy> _ref_cntr; 
 // .. 
}; 
```

.. and ``ref_counter_t`` is implemented as follows: 

```cpp
template <LockPolicy P> 
struct ref_counter_t
{
	uint32_t _value; 
}; 

template <> 
ref_counter_t<LockPolicy::atomic> 
{
	std::atomic<uint32_t> _value;
}; 
```

Instead of having a struct that has member variables, we can have a struct that has member types. This is what the **second method** does. 

```cpp
template <LockPolicy P> 
struct ref_counter_t
{
	using type = uint32_t;
}; 

template <> 
struct ref_counter_t<LockPolicy::atomic> 
{
	using type = std::atomic<uint32_t>; 
}; 
```
Class ``ref_counter_ptr_base`` will use it as follow: 

```cpp
template <LockPolicy P> 
struct ref_counter_ptr_base 
{
	using counter_type = typename ref_counter_t<P>::type;
	counter_type _ref_cntr; 
}; 
```


We can come out with something more general. This is the **third method**.  We want to select among two types ``T1`` or ``T2`` according to some boolean value. In this case the bool value will be ``P  == LockPolicy::atomic``,  ``T1`` is  ``uint32_t``, and ``T2`` is  ``uint32_t``.   This is not very difficult to do. 

```cpp
typename <bool, typename T1, typename T2>
struct this_or_that; 

typename <typename T1, typename T2>
struct this_or_that<true, T1, T2> 
{
	using type = T1; 
}; 

typename <typename T1, typename T2>
struct this_or_that<false, T1, T2> 
{
	using type = T2; 
}; 
```

We use this class as follows: 

```cpp
template <LockPolicy P> 
struct ref_counter_ptr_base 
{
	using counter_type = typename this_or_that<P==LockPolicy::atomic, std::atomic<uint32_t>, uint32_t>::type;
	counter_type _ref_cntr; 
}; 
```

The standard has a type trait that does exactly that. It is called ```std::conditional``. We can use it as follows: 

```cpp
template <LockPolicy P> 
struct ref_counter_ptr_base 
{
	using counter_type = typename std::conditional<P==LockPolicy::atomic, std::atomic<uint32_t>, uint32_t>::type;
	counter_type _ref_cntr; 
}; 
```

The good thing about the atomic implementation of the shared pointer is that that was the only thing we needed to change.  The increment and decrement of the counter will work just fine now. 

**What about the mutex implementation?** The problem here is that we have a mutex member variable in ``ref_counter_ptr_base``.  That member variable does not exist in the other implementations.  This means the three (or four) methods I showed above will not work.  The solution, fortunately, is easy; inheritance of a specialized template class.  C++'s empty base optimization makes this solution even more efficient. 

```cpp
template <LockPolicy P>
struct lock_policy_mutex {}; 

template <>
struct lock_policy_mutex<LockPolicy::mutex> 
{
   std::mutex _ref_cntr_mutex;  
}; 


template <LockPolicy P> 
struct ref_counter_ptr_base : lock_policy_mutex<P>
{ 
  using counter_type = typename std::conditional<P == LockPolicy::atomic, std::atomic<int>, int>::type; 
  counter_type _ref_cntr{1};
  
  ...
};  
```

I think the code explains itself here. All that is left now is to implement the acquire and release functions in the mutex mode. 

```cpp
template <> 
void ref_counter_ptr_base<LockPolicy::mutex>::acquire() 
{ 
  std::lock_guard<std::mutex> lock{_ref_cntr_mutex};
  ++_ref_cntr;
} 

template <> 
void ref_counter_ptr_base<LockPolicy::mutex>::release()
{
  std::lock_guard<std::mutex> lock{_ref_cntr_mutex};
  if (--_ref_cntr == 0) 
  { 
    dispose(); 
    delete this;  
  }

}
```

Note that because you only use the mutex in one implementation, you can have ``lock_policy_mutex`` as a member variable in ``ref_counter_ptr_base``. That will work with no issue, except that it will unnecessarily increase the size of ``ref_counter_ptr_base`` in the non-mutex implementations. Recall that empty struct takes at least one byte unless we inherit from it (i.e., empty base optimization). 

This is it for the multi-threading shared pointer. As you can see the multi-threading part of ``shared_ptr`` is quite simple. It is the other features that took most of our time and effort. There are still some cool techniques in ``shared_ptr`` that I would like to talk about in the next post. 
 

