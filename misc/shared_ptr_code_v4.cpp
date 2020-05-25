#include <iostream> 
#include <memory>


namespace ver4
{ 

template <class T> 
struct ref_counter_ptr_base
{ 
  
  uint32_t _ref_cntr;    
  T* _ptr;    
 
  ref_counter_ptr_base(T* ptr = nullptr); 
  void acquire(); 
  void release();   
  
  virtual void dispose(T* ptr) = 0; 
  virtual ~ref_counter_ptr_base() = default; 
 
};

template <class T>
ref_counter_ptr_base<T>::ref_counter_ptr_base(T* ptr) 
  : _ref_cntr(1), _ptr(ptr)
{ }
 
template <class T> 
void ref_counter_ptr_base<T>::acquire() 
{ 
  ++_ref_cntr;
} 

template <class T> 
void ref_counter_ptr_base<T>::release()
{
  if (--_ref_cntr == 0) 
  { 
    dispose(_ptr); 
    delete this;  
  }  
}

template <class T> 
struct ref_counter_ptr_default final : public ref_counter_ptr_base<T>
{
  using ref_counter_ptr_base<T>::ref_counter_ptr_base; 
  void dispose(T* ptr) override 
  { 
    std::cout << "Deleting with default \n"; 
    delete ptr; 
  }
    
};

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

template <class T> 
struct shared_ptr_counter 
{
   shared_ptr_counter(T* ptr=nullptr); 
   
   template <typename Deleter> 
   shared_ptr_counter(T* ptr, Deleter d); 
   
   shared_ptr_counter(shared_ptr_counter const&); 
   shared_ptr_counter& operator=(shared_ptr_counter const&); 
   ~shared_ptr_counter(); 
   
   ref_counter_ptr_base<T>* _ref_cntr{nullptr}; 
}; 


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

//  This was the ref_counter_ptr_t .. how does the 
// the shared_ptr looks like now. 
template <class T> 
struct shared_ptr
{
    shared_ptr(T* ptr=nullptr)
     : _shared_ptr_ctr{ptr} 
     , _ptr{ptr} 
    {} 
    
    template <typename Deleter> 
    shared_ptr(T* ptr, Deleter d)
      : _shared_ptr_ctr{ptr, d} 
      , _ptr{ptr} 
    {} 
    
    shared_ptr_counter<T>   _shared_ptr_ctr{nullptr}; 
    T* _ptr{nullptr};  
};






 
    
} // namespace ver3 

 

// --------------------------------
struct test_struct
{ 
   test_struct() { std::cout << "cstr\n"; }
   ~test_struct() { std::cout << "~dstrct\n"; }
};

template <typename T>
struct default_deleter
{
   void operator()(T* ptr) 
   { 
     std::cout << "Deleting with the customized default_deleter \n"; 
     delete ptr; 
   } 
}; 

template <class T, class ShPtr> 
ShPtr make_shared_ptr() 
{
    return ShPtr(new T); 
} 

int main() 
{
    
    auto s1 = ver4::shared_ptr<test_struct>(new test_struct);
    auto s2 = ver4::shared_ptr<test_struct>(new test_struct); 
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
