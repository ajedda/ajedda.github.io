---
layout: post
title: "Shared Pointers - Part 3: more organization"
categories: C++
keywords: programming; C++
---


My objective in this post is to do some more organization of the code. There were some details in the shared_ptr class implementation (e.g., the copy constructor, assignment operator, and destructor) that should be moved to the reference counter class.  For example, the shared pointer in the previous version directly called the reference counter release and acquire. These two functions should be private to the reference counter. 

How can we hide these details? It is not trivial because the reference counter is shared (i.e., created in the heap). The GCC implementation hides these details in a proxy class. We will call this the shared_ptr_counter class.  With the introduction of this class, the shared_ptr class will delegate all reference counting to the new shared_ptr_counter class. It is a nice pattern in my opinion that is worth sharing.  What is the name of this technique/pattern? I am not really sure. However, the STL library in gcc uses different techniques/patterns to hide implementation details. One of them is inheriting from an implementation class (e.g., shared_ptr inherits from __shared_ptr). 


Let's see how all this work. The implementation of ref_counter_ptr_t will remain unchanged. 

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

template <class T>
ref_counter_ptr_t<T>::ref_counter_ptr_t(T* ptr) 
  : _ref_cntr(1), _ptr(ptr)
{ }
 
template <class T> 
void ref_counter_ptr_t<T>::acquire() 
{ 
  ++_ref_cntr;
} 

template <class T> 
void ref_counter_ptr_t<T>::release()
{
  if (--_ref_cntr == 0) 
  { 
    delete _ptr; 
    delete this;  
  }  
}

}
```

Our proxy class is called shared_ptr_counter. Our objective it to convert shared_ptr implementation to as follow: 

```cpp
template <class T> 
struct shared_ptr
{
    shared_ptr(T* ptr=nullptr)
     : _shared_ptr_ctr{ptr} 
     , _ptr{ptr} 
    {} 
    
    
    shared_ptr_counter<T>   _shared_ptr_ctr{nullptr}; 
    T* _ptr{nullptr};  
};
```

In other words, I am only using the default class constructors and operators.  

Class shared_ptr constructors/assignment operators will implements the behaviour of the construcotrs/assignment operators of the previous shared_ptr version. This goes as follows: 

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

Let's get back to the acquire and release. These should be private in ref_counter_ptr_t. They are not in this version. However, shared_ptr_counter accesses them. That can be done using a friend relationship. This friend relationship between shared_ptr_counter and ref_counter_ptr_t makes more sense (compared to the friend relationship we had between shared_ptr and ref_counter_ptr_t). This is because both ref_counter_ptr_t and shared_ptr_counter does the same thing after all (that is, reference counting, instead of sharing pointers.) 

... and this is it for now. 