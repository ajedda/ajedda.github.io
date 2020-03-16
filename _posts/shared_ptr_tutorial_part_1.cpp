#include <iostream> 
#include <memory>

// The problem: design a shared_ptr. 
// It may be seen as an easy problem. The concepts are quite known. But there 
// are a lot to do to  achieve a shared_ptr similar to std::shared_ptr.
// I will try to design std::shared_ptr step-by-step. 
// Many interesting (hidden) details, software engineering techniques, and 
// challenging problems will be tackled in this tutorial. 
// I will start with the simple case. 

// What is a shared_ptr? multiple ownwers, the last owner to 
// be destroyed should free the pointer. 

// We will start with the simplest case. The shared_ptr 
// is not shared between threads. 
// In other words, the reference count is not thread-safe. 
template <class T> 
struct st_simple_shared_ptr
{ 
   
  T* _ptr{nullptr}; 
  uint32_t* _ref_cntr{nullptr};  
  // Why pointer here? The reference counter is 
  // shared by multiple objects. The best place to 
  // store this counter is in the heap. The shared_ptr 
  // owners (holders) all have a copy of this pointer.  
   
                                 
  
  // We should define the constructors, copy constructor, 
  // assignment operator, and destructor. 
  // Move cstr and operator are kept for another lesson. 
  st_simple_shared_ptr(T* ptr = nullptr); 
  st_simple_shared_ptr(st_simple_shared_ptr const& rhs); 
  st_simple_shared_ptr& operator=(st_simple_shared_ptr const& rhs); 
  ~st_simple_shared_ptr(); 

  // We will use two functions: acquire and release. 
  // These are explained in the code below. 
  void acquire(T*, uint32_t*); 
  void release();  
};

template <class T>
st_simple_shared_ptr<T>::st_simple_shared_ptr(T* ptr) 
  : _ptr{ptr}, _ref_cntr{new uint32_t{1}}
{ 
  // All you need here is to have a copy of the pointer. 
  // allocate a new _ref_cntr. 
  // Set it to 1 to indicate that we are the only owners of this.

  // Big caution: this is not thread-safe! 
  // We will show how to handle in the future tutorials.  
} 
 
template <class T> 
st_simple_shared_ptr<T>::st_simple_shared_ptr(st_simple_shared_ptr<T> const& rhs) 
{ 
  // The idea: both st_simple_shared_ptr instances point to the same 
  // pointer and the reference counter.
  // Add 1 (i.e., an additional owner) to the ref counter. 
  /*
  _ptr = rhs._ptr;    
  _ref_cntr = rhs._ref_cntr; 
  if (*_ref_cntr)  ++(*_ref_cntr);
  */ 
  // I will replace the steps above with acquire. 
  // This will be used later. 
  acquire(rhs._ptr, rhs._ref_cntr);   

} 

template <class T> 
void st_simple_shared_ptr<T>::acquire(T* ptr, uint32_t* ref_cntr) 
{ 
   _ptr = ptr; 
   _ref_cntr = ref_cntr; 
   if (_ref_cntr != nullptr)       // make sure _ref_cntr is not nullptr. 
   {  ++(*_ref_cntr);  }       
   
} 


template <class T> 
void st_simple_shared_ptr<T>::release() 
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

template <class T> 
st_simple_shared_ptr<T>::~st_simple_shared_ptr() 
{ 
  // I am starting with the deconstructor. I want to 
  // introduce the idea of "release" (i.e., releasing the resources). 
  // NB: I will start with commented lines, and replace them 
  // with a function called release(). 
  
  /*
  if (_ref_cntr != nullptr) 
  { 
    --(*_ref_cntr);     // reduce the number of owners of ptr. 
    if ((*_ref_cntr) == 0)  // i.e. "this" was the last owner. 
    {   
      delete _ptr;          // Now delete the ptr. 
      delete _ref_cntr;     // But also delete the _ref_cntr! 
    }  
  } 
  */ 
  release(); 
  // We can definitly replace these with a function called release(). 
} 

template <class T> 
st_simple_shared_ptr<T>& st_simple_shared_ptr<T>::operator=(st_simple_shared_ptr const& rhs)
{ 
  // This is a bit tricky. You may think it is similar to 
  // the copy-constructor. But in reality, this is more general. 
  // "this" may be ownining a new pointer (rhs._ptr) now. 
  // so "this" should first release its pointer.  
  
  // Before that, make sure this is not rhs. 
  // and also the pointers are not the same. Why? see comments
  // in the end of this function.  
  
  if (this != &rhs && _ptr != rhs._ptr) 
  { 
    release(); 
    // Now this should own rhs pointer using the following steps. 
    /*
    _ptr = rhs._ptr; 
    _ref_cntr = rhs._ref_cntr; 
    if (_ref_cntr != nullptr) 
    { ++(*_ref_cntr); } 
    */ 
    // I commented the lines above because they are used exactly 
    // in the copy-construct. 
    // I replace them with: 
    acquire(rhs._ptr, rhs._ref_cntr); 

    // Essentially, do the same thing as we did in the copy constructor    // but release the resource. 
  } 
  return *this; 

  // Why the if-statement?
  // If we this == &rhs -> the assignment should have no effect. 
  // if this != &rhs - but the pointers _ptr and rhs._ptr 
  // are the same (which is prefectly fine), then 
  // it means that "this" will release and then acquire the pointer 
  // again - which has not effect as well. Therefore, do nothing!  
} 

// --------------------------------
struct test_struct
{ 
   test_struct() { std::cout << "cstr\n"; }
   ~test_struct() { std::cout << "~dstrct\n"; }
};

template <class T, class ShPtr=st_simple_shared_ptr<T>> 
ShPtr make_shared_ptr() 
{
    return ShPtr(new T); 
} 

int main() 
{
    auto s1 = make_shared_ptr<test_struct>();  
    auto s2 = make_shared_ptr<test_struct>(); 
    auto s3 = s1; 
    s3 = s2; 
    s3 = s3;
    s3 = s2; 
    s3 = s2; 
    // Expected outpt. (but this  
    // cstr
    // cstr
    // ~dstrct
    // ~dstrct  
}
