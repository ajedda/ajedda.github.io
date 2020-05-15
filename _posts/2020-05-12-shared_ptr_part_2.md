---
layout: post
title: "Shared Pointers - Part 2: Will you ever delete this?"
categories: C++
keywords: programming; C++
---

The previous post implemented the reference counter as a pointer to an unsigned integer. I don't like this. It is too simplistic. Any features we want to add to the reference counter will have to be done by the shared pointer, even if it is not the job of the shared pointer.  Therefore, let's move the integer reference counter, acquire, and release to a new class.  We will end up with something interesting: we will "delete this".  This is perhaps one of the few cases where we call "delete this".  This is, however, not the reason we are moving the ref counter to a special class. The real reason will be shown in the next tutorial. 

Let's see how our new shared pointer will look like.   

```cpp
template <class T> 
struct ref_counter_ptr_t
{ 
  uint32_t _ref_cntr;    
  T* _ptr;    
 
  ref_counter_ptr_t(T* ptr = nullptr) ; 
  void acquire(); 
  void release();    
};
```

The constructor is pretty easy to define. 

```cpp
template <class T>
ref_counter_ptr_t<T>::ref_counter_ptr_t(T* ptr) 
  : _ref_cntr(1), _ptr(ptr)
{ }
```

The acquire is also simple. However, note that we don't have input arguments for acquiring since the counter belongs to the same class. 

```cpp
template <class T> 
void ref_counter_ptr_t<T>::acquire() 
{ 
  ++_ref_cntr;
} 
```

Now, the release: 

```cpp
template <class T> 
void ref_counter_ptr_t<T>::release()
{
  if (--_ref_cntr == 0) 
  { 
    delete _ptr; 
    delete this;  
  }  
}
```

Yup! delete this. 

That was only the ref counter class. The shared pointer class will looks like this now: 

```cpp
template <class T> 
struct shared_ptr
{
  shared_ptr(T* ptr=nullptr); 
  shared_ptr(shared_ptr const& rhs);
  shared_ptr& operator=(shared_ptr const& rhs);
  ~shared_ptr(); 


  ref_counter_ptr_t<T>* _ref_cntr{nullptr};  
  T* _ptr{nullptr};  
};
```

The constructor definition is as follows: 

```cpp
template <class T> 
shared_ptr<T>::shared_ptr(T* ptr) 
{ 
  try
  { 
    _ref_cntr = new ref_counter_ptr_t<T>(ptr); 
    _ptr = ptr;
  } 
  catch (...)     
  { 
    delete ptr;  
    throw; 
  }   

} 
```

There should be no difference between this definition and the definition we had in the previous post. I am just adding a try-catch block to show that the new may throw. It may throw whether it was for an integer or a ref_counter_ptr_t type.  However, what we should do if it throws is to throw the exception.  The standard shared pointer will also delete the passed pointer. This makes sense since the job of the shared pointer to take ownership of the pointer. If it throws, then it does not exists, and hence we should also release all resources. 

Everything else is pretty much the same. 

```cpp
template <class T>  
shared_ptr<T>::shared_ptr(shared_ptr<T> const& rhs) 
{ 
    _ref_cntr = rhs._ref_cntr; 
    if (_ref_cntr != nullptr) 
        _ref_cntr->acquire(); 
} 

template <class T> 
shared_ptr<T>& shared_ptr<T>::operator=(shared_ptr<T> const& rhs) 
{  
  if (this != &rhs && _ptr != rhs._ptr) 
  {
    if (_ref_cntr != nullptr)  _ref_cntr->release();   
    _ref_cntr = rhs._ref_cntr;  
    if (_ref_cntr != nullptr) _ref_cntr->acquire();     
  }
  return *this; 
} 


template <class T> 
shared_ptr<T>::~shared_ptr() 
{
    if (_ref_cntr != nullptr) 
        _ref_cntr->release(); 
} 
```

There are many more things to do. We will continue in other posts. 
