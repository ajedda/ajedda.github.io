#include <iostream> 
#include <memory> 

// The simple solution of Tutorial-1 shows us that 
// the reference counter and its pointer are used together 
// in the acquire and release. release() took two paramters; 
// the pointer and the ref-counter.  
// So, why not put them together in one struct: 
// call it, ref_counter_ptr_t. (why ref_counter_*ptr*_t? 
// becuase the shard_ptr will only use it as a ptr. 
 
// This will have a nice pros: hide the details of reference counting 
// from the shared_ptr class. Its cons, a slightly more complex design.
// We will go with hiding the details because it is very important 
// to have our design extensible. 


template <class T> 
struct ref_counter_ptr_t
{ 

  
  uint32_t _ref_cntr;    // Note: not a pointer!  
  // Trick: always define the most accessible member of your struct 
  // (in this case _ref_cntr) on top. This is more cache-friendly. 



  T* _ptr;    // Wait! what? _ptr in _ref_counter.  
              // Yes. You will know why soon. 
              // Why not in the shared_ptr? Well, we do also 
              // havea copy of _ptr there as well. 

  // There are two main jobs for the ref_counter. 
  // 1. Keep track of the number of owners of _ptr. 
  // 2. Free _ptr when no owner is alive. 
  // But why 2? Why free _ptr from here. 
  // If I don't, freeing _ptr will be done at the shared_ptr. 
  // and In order to known when the _ptr's owners are dead, 
  // we should always check an output from ref_counter_ptr_t. 
  // In addition to that, shared_ptr will also control when 
  // its reference counter should be freed. This will only 
  // make the design more complex. So, let's stick to this 
  // approach. But hey, there are good benifits of this 
  // approach and we will see them soon. 
  
  // As a result: we need the following functions. 
  ref_counter_ptr_t(T* ptr = nullptr) ; 
  void acquire(); 
  void release();   // note: release has no paramters now.  
 
};

template <class T>
ref_counter_ptr_t<T>::ref_counter_ptr_t(T* ptr) 
  : _ptr(ptr), _ref_cntr(1)
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
    // In the previos version, we had delete _ref_cntr. 
    // _ref_cntr is not a pointer here. 
    // But we have a serious problem. _ref_counter_ptr_t is 
    // allocated in the heap (and that's why we called it: 
    // _ref_counter_*ptr*_t.  
    //  Who should free it? (or who 
    // should free "this"?) [will the last to leave please 
    // turn off the light] --> and we get: 
    delete this; 
    // Yes, delete this. It is not that bad as some think. 
    // We are guaranteed not to use this object anymore. 
    // The Standard library uses this approach. 
  }  
}


//  This was the ref_counter_ptr_t .. how does the 
// the shared_ptr looks like now. 
template <class T> 
struct rfc_shared_ptr
{

    rfc_shared_ptr(T* ptr=nullptr); 
    rfc_shared_ptr(rfc_shared_ptr const& rhs);
    rfc_shared_ptr& operator=(rfc_shared_ptr const& rhs);
    ~rfc_shared_ptr(); 


    ref_counter_ptr_t<T>* _ref_cntr{nullptr};  
    T* _ptr{nullptr}; // Why _ptr here? for simplicity. 
                      // it may sound like we are using 
                      // more memory - but hey, it is 2018 here. 
                      // Does it have a performance benifit? 
                      // I don't know, but it is worth the 
                      // investigation.  
};

template <class T> 
rfc_shared_ptr<T>::rfc_shared_ptr(T* ptr) 
{ 
  // Two taks. _ref_cntr is allocated. 
  // and storing _ptr. 
  // But allocation may throw! We ignored this problem before. 
  // Now we want to handle it. 
  // Solution 1: catch the exception and re-throw it. 
  // Solution 2: catch the exception and re-throw it - but before that
  // delete _ptr (since we are supposed to be the only owner). 
  // the standard library uses this solution. (so Solution 2 we follow). 
  try
  { 
    _ref_cntr = new ref_counter_ptr_t<T>(ptr); 
    _ptr = ptr;
  } 
  catch (...)     // catch anything. We will use different allocators
                  // in the future. Ok. I lied (I am lazy!).  
  { 
    delete ptr;  // But wait. Wasn't the whole reason  of this word 
                  // to avoid deleting the ptr? We will 
                  // talk about this in Tutorial-3.  
    throw; 
  }   

} 

template <class T>  
rfc_shared_ptr<T>::rfc_shared_ptr(rfc_shared_ptr<T> const& rhs) 
{ 
    _ref_cntr = rhs._ref_cntr; 
    if (_ref_cntr != nullptr) 
        _ref_cntr->acquire(); 
} 

template <class T> 
rfc_shared_ptr<T>& rfc_shared_ptr<T>::operator=(rfc_shared_ptr<T> const& rhs) 
{  
  if (this != &rhs && _ptr != rhs._ptr) 
  {
    if (_ref_cntr != nullptr)  _ref_cntr->release();  // release the current _ref_cntr. 
    _ref_cntr = rhs._ref_cntr;  // change the pointer of the current
                                // ref_cntr to rhs._ref_cntr. 
                                
    if (_ref_cntr != nullptr) _ref_cntr->acquire();     // acquire now. 
  }
  return *this; 
} 


template <class T> 
rfc_shared_ptr<T>::~rfc_shared_ptr() 
{
    if (_ref_cntr != nullptr) 
        _ref_cntr->release(); 
} 

struct test_struct
{ 
   test_struct() { std::cout << "cstr\n"; }
   ~test_struct() { std::cout << "~dstrct\n"; }
};


template <class T, class ShPtr=rfc_shared_ptr<T>> 
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
