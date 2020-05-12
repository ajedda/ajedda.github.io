---
layout: post
title: "Shared Pointers - Part 1: a very simple single-threaded implementation"
categories: C++
keywords: programming; C++
---

Here is a good C++ guideline, use unique and shared pointers to represent objects ownerships because they are good at preventing memory leaks. As a rule of thumb, prefer unique pointers. Why? they pose unnecessary performance overhead. Shared pointers, however, have some applications (when you really want them), and the truth is that they are more fun to implement.  Side note: this does not necessarily mean that we should smart pointers for everything (e.g., observer_ptr, which is just too much in my opinion. I was happy to know that Bjarne shares the same opinion). 

This series of blog posts go through the implementation of shared pointers in GCC. There are a few techniques that I liked and I would like to focus on. 

Let's start with the first implementation. It is a single-threaded implementation. No fancy features, just the main idea (that is, the constructor, the copy construction, the assignment operator, and the destructor). 

The two members of the shared pointer are: 1) the pointer of the owned object (of template type T), and 2) a reference counter that keeps track of how many owners own that object. In this basic implementation, the reference counter is of type unsigned integer. 

```cpp
namespace ver1 {

template <typename T> 
struct shared_ptr
{
	T* _ptr{nullptr}; 
	uint32_t* _ref_cntr{nullptr}; 
}; 

}
```

Why _ref_cntr is a pointer? because it will be shared by multiple objects.  Why 32 bits? No specific reasons, we don't think that we will need more than $2^{32}$ owners for that shared pointer. 

The constructor is very simple: 

```cpp
template <class T>
shared_ptr<T>::shared_ptr(T* ptr) 
  : _ptr{ptr}, _ref_cntr{new uint32_t{1}}
{ 
  // All you need here is to have a copy of the pointer. 
  // allocate a new _ref_cntr. 
  // Set it to 1 to indicate that we are the only owners of this.

  // Big caution: this is not thread-safe! 
  // We will show how to handle in the future tutorials.  
}
```
The fun starts with the copy constructor: 

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

All the copy constructor does is to _acquire_ the ownership of rhs._ptr (or, the object pointed to by rhs._ptr).  So let's show this in the code as follows: 

```cpp
template <class T> 
shared_ptr<T>::shared_ptr(shared_ptr<T> const& rhs) 
{ 
  acquire(rhs._ptr, rhs._ref_cntr);   
} 

template <class T> 
void shared_ptr<T>::acquire(T* ptr, uint32_t* ref_cntr) 
{ 
   _ptr = ptr; 
   _ref_cntr = ref_cntr; 
   if (_ref_cntr != nullptr)       
   {  ++(*_ref_cntr);  }       
}
```
The assignment operator has an extra step; the _release_. When calling the assignment operator (as in the case sp1=sp2). The shared pointer sp1 may already be owning another object. Therefore, it should release it before acquiring sp2's pointer. 

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

You can guess by now that all the destructor does is calling release. 

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

template <class T, class ShPtr> 
ShPtr make_shared_ptr() 
{
    return ShPtr(new T); 
} 

int main() 
{
    auto s1 = make_shared_ptr<test_struct, ver1::shared_ptr<test_struct>>();  
    auto s2 = make_shared_ptr<test_struct, ver1::shared_ptr<test_struct>>(); 
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