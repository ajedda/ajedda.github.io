---
layout: post
title: "Shared Pointers - Part 4: Custom deleters"
categories: C++
keywords: programming; C++
---

It is time for a new feature in our shared pointer; custom deleters. Let's say that your pointer was allocated with *malloc* (or any other custom allocator). It should, in this case, be deallocated using *free* (or the corresponding de-allocator).  Unfortunately, this shared pointer implementation will fail to do the job correctly.  The good news is that this problem is easy to fix. We should tell our shared pointer to use a special *deleter* function instead of using the default ``delete``.  One way to do that is to replace ```delete _ptr``` in ```release()``` with a  ```dispose(_ptr)``` as shown in this example. 

```cpp
template <class T> 
void ref_counter_ptr_t<T>::release()
{
  if (--_ref_cntr == 0) 
  { 
    dispose(_ptr);
    delete this;  
  }  
}
```

How to tell this shared pointer implementation to use a custom deleter if provided, or ``delete`` otherwise? what is the type of ``dispose``? should ``dispose`` be a member variable? and how? That's what we will be answering next. 

Before we start, we should note that the shared pointer class definition will not change. That is, we will not add a new template variable to the class. It will remain: 

```cpp
template <typename T>
class shared_ptr; 
```

This forces us to go on the design path that I will discuss next. 


The first thing we should note is that ```shared_ptr_counter<T>::_ref_cntr``` is a pointer of type  ```ref_counter_ptr_t```. This allows us to use a polymorphic reference counter. This reference counter implements ```dispose``` differently. Let's re-organize the ``ref_counter_ptr_t`` as discussed. We will create a base class, and two derived classes, each implementing dispose differently. 

```cpp
template <class T> 
struct ref_counter_ptr_base
{ 
  
  uint64_t _ref_cntr;    
  T* _ptr;    
 
  ref_counter_ptr_base(T* ptr = nullptr); 
  void acquire(); 
  void release();   
  
  // These are the only new functions. Everything 
  // else will have the same implemenation.  
  virtual void dispose(T* ptr) = 0; 
  virtual ~ref_counter_ptr_base() = default; 
 
};
```

This is the version where ```delete`` is used: 

```cpp
template <class T> 
struct ref_counter_ptr_default final : public ref_counter_ptr_base<T>
{
  using ref_counter_ptr_base<T>::ref_counter_ptr_base; 
  
  void dispose(T* ptr) override 
  { 
    delete ptr; 
  }
    
};
```

... now, we will declare the version that uses the custom deleter. 

```cpp
template <class T, typename Deleter> 
struct ref_counter_ptr_deleter final : public ref_counter_ptr_base<T>
{ 
  ref_counter_ptr_deleter(T* ptr, Deleter d)
   : ref_counter_ptr_base<T>{ptr} 
   , deleter{d}
  {} 
   
  void dispose(T* ptr) override 
  { 
    deleter(ptr); 
  }
  
  Deleter deleter; 
};
```

Contrary to ```ref_counter_ptr_default```,  class ```ref_counter_ptr_deleter``` has a member variable of type ``Deleter``, which can be function, a callable object, or lambda. Whatever ``Deleter`` is, it must be Copy Constructible. 

But where  is ``Deleter`` in the ``shared_ptr``? It is in one of the constructors, a template construtor actually.  That is, we should have a shared_pointer constructor as follows: 

```cpp
template <typename Deleter> 
shared_ptr(T* ptr, Deleter d)
  : _shared_ptr_ctr{ptr, d} 
  , _ptr{ptr} 
{} 
```


Similarly, ```shared_ptr_counter<T>``` will only have an additional constructor.  It will also have a member variable of type ```ref_counter_ptr_base<T>```.  

```cpp
template <class T> 
struct shared_ptr_counter 
{
   shared_ptr_counter(T* ptr=nullptr); 
   
   template <typename Deleter> 
   shared_ptr_counter(T* ptr, Deleter d); 
   
   // Same implementation as before. 
   shared_ptr_counter(shared_ptr_counter const&); 
   shared_ptr_counter& operator=(shared_ptr_counter const&); 
   ~shared_ptr_counter(); 
   
   ref_counter_ptr_base<T>* _ref_cntr{nullptr}; 
}; 
```

.. and 
```cpp
template <class T> 
shared_ptr_counter<T>::shared_ptr_counter(T* ptr) 
{ 
  
  try
  { 
    _ref_cntr = new ref_counter_ptr_default<T>(ptr); 
  } 
  catch (...)     
  { 
    delete ptr;  
    throw; 
  }   

} 

template <class T> 
template <typename Deleter>
shared_ptr_counter<T>::shared_ptr_counter(T* ptr, Deleter d)
{ 
  try
  { 
    _ref_cntr = new ref_counter_ptr_deleter<T, Deleter>(ptr, d); 
  } 
  catch (...)     
  { 
    d(ptr);   
    throw; 
  }  
} 
```

Everything else is implemented as before. So, this is it. A pretty nice feature with some two cool techniques: 

 1. Use polymorphism to implement different dispose behavior. 
 2. Use templatized constructors to control which implementation to use. 

As you can see, this had minimum changes on the shared pointer class. 
