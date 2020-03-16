#include <iostream> 
#include <memory> 

// We were able in Tutorial-2 to remove some implemenation
// details from the shared_ptr class. But it is not enough. 
// E.g., The shared_ptr controlled the deletion of the _ptr in 
// case of exception. 
// I will show that we can hide all the details in our shared_ptr 
// class. Hiding implementation is a good sofware-engineering technique. 
// We, C++ developers, love it. We also love RAII. This is what we 
// will do in this tutorial. 

// How? if most of the work in shared_ptr is in the constructors
// and destructor, then can we use the default of them only. 

// We first hide all the ref counting details inside a class called 
// shared_ptr_counter
template <class T> 
struct shared_ptr_counter 
{
    
    shared_ptr_counter(T* ptr = nullptr); 
    shared_ptr_counter(const shared_ptr_counter& rhs); 
    shared_ptr_counter& operator=(const shared_ptr_counter& rhs); 
    ~shared_ptr_counter();   
    
    uint32_t* _ref_cntr_ptr{nullptr};      
    T*         _ptr{nullptr};  

    // The basic idea: 
    // An instance of this class has a pointer to a ref-counter which 
    // resides in the heap. 
    // The reference counter is identified by its _ptr. 
    // We copy a shared_ptr_counter -> we acquire. 
    // We assign a shared_ptr_cutner -> we release, we acquire. 
    // We destruct a shared_ptr_counter -> we release.
    // These are the operations that the shared_ptr used to do in 
    // our previous tutorials. Therefore, shared_ptr only stores 
    // a "copy" (no pointer) for a shared_ptr_countr .. and initialize
    // it with its pointer.  Boom! simple! 
}; 

template <class T> 
shared_ptr_counter<T>::shared_ptr_counter(T* ptr) 
{
    try
    { 
        _ref_cntr_ptr = new uint32_t{1}; 
        _ptr = ptr; 
    } 
    catch (...) 
    { 
      delete ptr; 
      throw;   
    } 
}
  
template <class T>  
shared_ptr_counter<T>::shared_ptr_counter(const shared_ptr_counter<T>& rhs) 
    : _ptr(rhs._ptr), _ref_cntr_ptr(rhs._ref_cntr_ptr)
{ 
  //  acquire. 
  if (_ref_cntr_ptr != nullptr)  ++(*_ref_cntr_ptr);
} 

template <class T>  
shared_ptr_counter<T>& shared_ptr_counter<T>::operator=(const shared_ptr_counter<T>& rhs) 
{
   if (this != &rhs && _ref_cntr_ptr != rhs._ref_cntr_ptr) 
   {
     // release.  
    if (_ref_cntr_ptr != nullptr)  --(*_ref_cntr_ptr);
    if ((*_ref_cntr_ptr) == 0) 
    { delete _ptr; delete _ref_cntr_ptr; }     
     

      // acquire.
     _ptr = rhs._ptr; 
     _ref_cntr_ptr = rhs._ref_cntr_ptr; 
     if (_ref_cntr_ptr != nullptr)  ++(*_ref_cntr_ptr); 
   } 
   return *this;
} 

template <class T>  
shared_ptr_counter<T>::~shared_ptr_counter() 
{
  if (_ref_cntr_ptr == nullptr) { return; } 
  
  // release. 
  --(*_ref_cntr_ptr); 
  if ((*_ref_cntr_ptr) == 0) 
  {
    delete _ptr; 
    delete _ref_cntr_ptr; 
  } 
     
} 
  
// and that above will allow me to do the following 
// shared_ptr implemenation! only default's! 
template <class T> 
struct light_impl_shared_ptr
{
     light_impl_shared_ptr(T* ptr=nullptr) 
      : _ptr(ptr), _shp_cntr{ptr}
     {} 
     
     light_impl_shared_ptr(light_impl_shared_ptr const&) = default;
     light_impl_shared_ptr& operator=(light_impl_shared_ptr const&) = default ;
     ~light_impl_shared_ptr() = default; 
     
     T* _ptr{nullptr}; 
     shared_ptr_counter<T> _shp_cntr; 
}; 
 

// --------------------------------
struct test_struct
{ 
   test_struct() { std::cout << "cstr\n"; }
   ~test_struct() { std::cout << "~dstrct\n"; }
};

template <class T, class ShPtr=light_impl_shared_ptr<T>> 
ShPtr make_shared_ptr() 
{
    return ShPtr(new T); 
} 

int main() 
{
    auto s1 = make_shared_ptr<test_struct>();  
    { 
      auto s0 = make_shared_ptr<test_struct>(); 
    }
    auto s2 = make_shared_ptr<test_struct>(); 
    auto s3 = s1; 
    s3 = s2; 
    s3 = s3;
    s3 = s2; 
    s3 = s2; 
    // Expected outpt.  
    // cstr
    // cstr 
    // ~dstrct
    // cstr
    // ~dstrct
    // ~dstrct  
}
