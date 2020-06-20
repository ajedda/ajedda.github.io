---
layout: post
title: "Shared Pointers - Part 1: A very simple single-threaded implementation"
categories: C++
keywords: programming; C++
---

Here is a good C++ guideline, use unique and shared pointers to represent objects ownerships because they are good at preventing memory leaks. As a rule of thumb, prefer unique pointers. Shared pointers pose unnecessary performance overhead. They have, however, some applications (when you really want them). More importantly, they are more fun to implement. 

This series of blog posts go through the implementation of shared pointers in gcc. There are a few techniques that I liked and I would like to share with you in this series of posts. I notice that multi-threading occupy most of discussions that I have been in about shared pointers. Some job interviews love this aspect about shared pointers. I don't. You will see in this series of posts that multi-threading will take 1 or 2 posts at most. The rest is about software design techniques. For example, I will talk about custom deleters, type erasure, different techniques for template specialization, some tricks with inheritance, and other techniques that can be useful to C++ developers. 

Let's start with the first implementation. It is a single-threaded implementation. No fancy features, just the main idea.

The two members of the shared pointer are: 1) the pointer of the owned object (of template type T), and 2) a reference counter that keeps track of how many owners own that object. Every time an object acquire (release) the pointer, the reference counter must incremented (decremented).  These are implemented by the constructor, copy constructor, assignment operator, and destructor. 

What are the member variables of our shared pointer class? Well, in this basic implementation, the reference counter is of type unsigned integer. We also have the pointer itself, which is of type T. This leads to a struct as follows: 

```cpp
namespace ver1 {

template <typename T> 
struct shared_ptr
{
	uint64_t* _ref_cntr{nullptr}; 
	T* _ptr{nullptr}; 

}; 

}
```

Why ``_ref_cntr`` is a pointer? because it will be shared by multiple objects.  Why 64 bits? no specific reason. We can replace it with ``uint32_t` as we just don't think that we will need more than $2^{32}$ owners for that shared pointer. 

As a rule of thumb, a member variables declarations order should be from most used to least. In this case, the reference counter is used more than the pointer. 

The constructor is very simple: 

```cpp
template <class T>
shared_ptr<T>::shared_ptr(T* ptr) 
  : _ref_cntr{new uint64_t{1}}, _ptr{ptr}
{ 
}
```

All you need here is to copy  the pointer, allocate a new ``_ref_cntr``, and set it to 1 to indicate that this shared pointer instance is the only owner of ``_ptr``. The fun starts with the copy constructor: 

```cpp
template <class T> 
shared_ptr<T>::shared_ptr(shared_ptr<T> const& rhs) 
{ 
    _ptr = rhs._ptr; 
   _ref_cntr = rhs._ref_cntr; 
   if (_ref_cntr != nullptr)       
   {  ++(*_ref_cntr);  }     
}
```

All the copy constructor does is to *acquire* the ownership of ``rhs._ptr``.  So let's re-write that as follows: 

```cpp
template <class T> 
shared_ptr<T>::shared_ptr(shared_ptr<T> const& rhs) 
{ 
  acquire(rhs._ptr, rhs._ref_cntr);   
} 

template <class T> 
void shared_ptr<T>::acquire(T* ptr, uint64_t* ref_cntr) 
{ 
   _ptr = ptr; 
   _ref_cntr = ref_cntr; 
   if (_ref_cntr != nullptr)       
   {  ++(*_ref_cntr);  }       
}
```
The assignment operator has an extra step; the *release*. When calling the assignment operator (as in the case ``sp1=sp2``). The shared pointer ``sp1`` may already be owning another object. Therefore, it should release it before acquiring sp2's pointer. 

```cpp
template <class T> 
shared_ptr<T>& shared_ptr<T>::operator=(shared_ptr const& rhs)
{ 
  if (this != &rhs && _ptr != rhs._ptr) 
  { 
    release(); 
    acquire(rhs._ptr, rhs._ref_cntr); 
  } 
  return *this; 
} 

template <class T> 
void shared_ptr<T>::release() 
{ 
  if (_ref_cntr != nullptr) 
  { 
    --(*_ref_cntr);     // reduce the number of owners of ptr. 
    if ((*_ref_cntr) == 0)  // i.e. "this" was the last owner. 
    {   
      delete _ptr;          // Now delete the ptr. 
      delete _ref_cntr;     // But also delete the _ref_cntr! 
    }  
  } 

} 
```

You may guess by now that all the destructor does is calling ``release``. 

```cpp
template <class T> 
shared_ptr<T>::~shared_ptr() 
{ 
  release(); 
} 
```

Putting the class declaration together, we get: 

```cpp
namespace ver1 { 
    
template <class T> 
struct shared_ptr
{ 
   
  T* _ptr{nullptr}; 
  uint32_t* _ref_cntr{nullptr};  
 
  shared_ptr(T* ptr = nullptr); 
  shared_ptr(shared_ptr const& rhs); 
  shared_ptr& operator=(shared_ptr const& rhs); 
  ~shared_ptr(); 


  void acquire(T*, uint32_t*); 
  void release();  
};

}
```

Here is some code to test everything. 

```cpp
struct test_struct
{ 
   test_struct() { std::cout << "cstr\n"; }
   ~test_struct() { std::cout << "~dstrct\n"; }
};

int main() 
{
    auto s1 = ver1::shared_ptr<test_struct>{new test_struct}; 
    auto s2 = ver1::shared_ptr<test_struct>{new test_struct};

    auto s3 = s1; 
    s3 = s2; 
    s3 = s3;
    s3 = s2; 
    s3 = s2; 

}
```

The expected output is: 

```cpp
cstr
cstr
~dstrct
~dstrct  
```

That's it for now. The next post will get into some more details. 