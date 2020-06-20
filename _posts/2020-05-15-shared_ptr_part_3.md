---
layout: post
title: "Shared Pointers - Part 3: More decoupling"
categories: C++
keywords: programming; C++
---


We will do some more code re-organization in this post. There were some details in the shared_ptr class implementation (e.g., the copy constructor, assignment operator, and destructor) that should be moved to the reference counter class.  As an example, the shared pointer in the previous version directly called the reference counter release and acquire. These two functions should be private to the reference counter. 

How can we hide these details? It is not trivial because the reference counter is shared (i.e., created in the heap). We can hide these details in a proxy class. We will call this the ``shared_ptr_counter`` class.  With the introduction of this class, the ``shared_ptr`` class will delegate all reference counting to the new ``shared_ptr_counter`` class. There will be no acquire, no release, and not even an allocation at class ``shared_ptr``.  Furthermore, the shared pointer class will not have a pointer to the reference counter as it was the case in the previous  two versions. I think this is a nice a pattern that is worth sharing. I am not sure if this pattern (as introduced in this post) has a name. It can be classified under one of the *Handle Body* idioms. As we build on top of that in the next posts, I will show that re-organizaiton will allow us to do some cool stuff, especially when it comes to custom deleters (but that will be in the next posts). 

Let's see how all this work. The implementation of ``ref_counter_ptr_t`` will remain unchanged. 

```cpp
namespace ver3
{ 

template <class T> 
struct ref_counter_ptr_t
{ 
  
  uint32_t _ref_cntr;    
  T* _ptr;    
 
  ref_counter_ptr_t(T* ptr = nullptr); 
  void acquire(); 
  void release();    
 
};



}
```

Our proxy class is called ``shared_ptr_counter``. Our objective it to convert ``shared_ptr`` implementation to as follow: 

```cpp
template <class T> 
struct shared_ptr
{
    shared_ptr(T* ptr=nullptr)
     : _shared_ptr_ctr{ptr} 
     , _ptr{ptr} 
    {} 
    
    
    shared_ptr_counter<T>   _shared_ptr_ctr; 
    T* _ptr{nullptr};  
};
```

In other words, I am only using the default class constructors and operators.  

Class ``shared_ptr`` constructors/assignment operators will implements the behaviour of the construcotrs/assignment operators of the previous ``shared_ptr`` version. This goes as follows: 

```cpp
template <class T> 
struct shared_ptr_counter 
{
   shared_ptr_counter(T* ptr=nullptr); 
   shared_ptr_counter(shared_ptr_counter const&); 
   shared_ptr_counter& operator=(shared_ptr_counter const&); 
   ~shared_ptr_counter(); 
   
   ref_counter_ptr_t<T>* _ref_cntr{nullptr}; 
}; 


template <class T> 
shared_ptr_counter<T>::shared_ptr_counter(T* ptr) 
{   
  try
  { 
    _ref_cntr = new ref_counter_ptr_t<T>(ptr); 
  } 
  catch (...)     
  { 
    delete ptr;  
    throw; 
  }   
} 

template <class T>  
shared_ptr_counter<T>::shared_ptr_counter(shared_ptr_counter<T> const& rhs) 
{ 
    _ref_cntr = rhs._ref_cntr; 
    if (_ref_cntr != nullptr) 
        _ref_cntr->acquire(); 
} 


template <class T> 
shared_ptr_counter<T>& shared_ptr_counter<T>::operator=(shared_ptr_counter<T> const& rhs) 
{  
  if (this != &rhs) 
  {
    if (_ref_cntr != nullptr)  _ref_cntr->release();  // release the current _ref_cntr. 
    _ref_cntr = rhs._ref_cntr;  // change the pointer of the current
                                // ref_cntr to rhs._ref_cntr. 
                                
    if (_ref_cntr != nullptr) _ref_cntr->acquire();     // acquire now. 
  }
  return *this; 
} 

template <class T> 
shared_ptr_counter<T>::~shared_ptr_counter() 
{
    if (_ref_cntr != nullptr) 
        _ref_cntr->release(); 
}  
```

Let's get back to the acquire and release. These should be private in ``ref_counter_ptr_t``. They are not in this version. However, ``shared_ptr_counter`` accesses them. That can be done using a friend relationship. This friend relationship between ``shared_ptr_counter`` and ``ref_counter_ptr_t`` makes more sense (compared to the friend relationship we had between ``shared_ptr`` and ``ref_counter_ptr_t``). This is because both ``ref_counter_ptr_t`` and ``shared_ptr_counter`` does the same thing after all (that is, reference counting, instead of sharing pointers.) 

... and this is it for now. 
