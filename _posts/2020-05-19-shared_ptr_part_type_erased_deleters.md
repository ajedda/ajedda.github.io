---
layout: post
title: "Shared Pointers - Part 5: Type erased deleters "
categories: C++
keywords: programming; C++
---

Let's have a look at this interesting behaviour in ``std::shared_ptr``. First, what is the behavior of this program: 

```cpp
void* ptr = new test_struct; 
delete ptr; 
```

The output is: 
```cpp
cstr
```

which is what the ``test_struct`` constructor prints. The destructor is not called even though we expicitly called ``delete v_ptr``.  The memory is deallocated,  but the destructor is not called. This can be dangerous in some scenarios.  Your compiler may as well print you a warning message to say that  deleting ``void*`` is undefined. 

Now, let's study the behavior of this program. 

```cpp
int main() 
{
    auto ptr = std::shared_ptr<void>(new test_struct); 
}
```
The output is: 

```cpp
cstr
~dstrct
```

The constructor and destructor of ``test_struct`` will be called here. Yup, that was right.  Unfortunately, the version we have of ``shared_ptr`` does not have the same behavior. How can we enrich our shared pointer to do that? 

Here is the trick. The ``shared_ptr`` will store a pointer of type ``void``, while the reference counter class ``shared_ptr_counter`` will store a pointer of type ``test_struct`` (or any other type convertible to ``void``). This means we will be dealing with two types. We are currently defining ``shared_ptr_counter`` as: 

```cpp
template <typename T> 
struct shared_ptr_counter;
```

With this definition, we won't achieve our objective. However, do we really need to restrict ourselves to this class definition. No, we don't. This will seems clearer to you when you notice the relationship between ``ref_counter_ptr_base``, ``ref_counter_ptr_deleter``, and ``ref_counter_ptr_default``, which is as follows: 

```cpp
template <typename T> 
struct ref_counter_ptr_base; 

template <typename T> 
ref_counter_ptr_deleter::ref_counter_ptr_base<T> { ... }; 

template <typename T> 
ref_counter_ptr_default::ref_counter_ptr_base<T> { ... }; 
```

The only reason we have ``ref_counter_ptr_base`` as a template of typename T is that it store a pointer of type ``T*``. It doesn't have to. Instead, we can store that pointer in the two subclasses. Therefore, we end up with: 

```cpp
struct ref_counter_ptr_base; 

template <typename T> 
ref_counter_ptr_deleter::ref_counter_ptr_base { ... }; 

template <typename T> 
ref_counter_ptr_default::ref_counter_ptr_base { ... }; 
```


If ``ref_counter_ptr_base`` is not a a template, then the same applies to ``shared_ptr_counter``. So now we have: 

```cpp
struct shared_ptr_counter; 
```

instead of: 

```cpp
template <typename T> 
struct shared_ptr_counter; 
```

This is very useful. We still have a problem though. The two classes ``ref_counter_ptr_deleter`` and ``ref_counter_ptr_default`` are still templates. Fortunatley, we store these pointers polymorphically. That is, using a pointer of type ``ref_counter_ptr_base`` in ``shared_ptr_counter``. We can decide what to store in that ``ref_counter_ptr_base`` using the constructo of ``shared_ptr`` and that of ``shared_ptr_counter``, as we did when we decided what deleter to use. We will add extra template parameters to the constructors of ``shared_ptr`` and ``shared_ptr_counter`` as follows: 

```cpp
template <typename T> 
struct shared_ptr
{
    // Note that we are using template parameter Y instead of T. 
    template <typename Y>
    shared_ptr(Y* ptr=nullptr)
     : _shared_ptr_ctr{ptr} 
     , _ptr{ptr} 
    {} 
    
    // Note that we are using template parameter Y instead of T. 
    template <typename Y, typename Deleter> 
    shared_ptr(Y* ptr, Deleter d)
      : _shared_ptr_ctr{ptr, d} 
      , _ptr{ptr} 
    {} 
    
    shared_ptr_counter   _shared_ptr_ctr{nullptr}; 
    T* _ptr{nullptr};  
};
```

.. and now: 

```cpp
struct shared_ptr_counter 
{
   // Note: the class itself is not a template, 
   // only the constructors are. 
   template <typename T>
   shared_ptr_counter(T* ptr=nullptr); 
   
   template <typename T, typename Deleter> 
   shared_ptr_counter(T* ptr, Deleter d); 
   
   shared_ptr_counter(shared_ptr_counter const&); 
   shared_ptr_counter& operator=(shared_ptr_counter const&); 
   ~shared_ptr_counter(); 
   
   ref_counter_ptr_base* _ref_cntr{nullptr}; 
}; 
```

Here is how the ``ref_counter_ptr_base`` class is defined now. 

```cpp
struct ref_counter_ptr_base
{ 
  // Note: there are no pointer member variable here, and hence 
  // the class can be non-template. 
  uint64_t _ref_cntr{1};    
  
 
  ref_counter_ptr_base() = default;  
  void acquire(); 
  void release();   
  
  // Also note the dispose does not take a pointer 
  // as an input parameter.
  virtual void dispose() = 0; 
  virtual ~ref_counter_ptr_base() = default; 
 
};
```

