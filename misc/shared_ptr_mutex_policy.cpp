#include <iostream> 
#include <mutex>

enum class Policy { single, atomic, mutex}; 

template <Policy P>
struct mutex_lock_t {}; 

template <> 
struct mutex_lock_t<Policy::mutex>
{
    std::mutex _mutex; 
}; 

template <Policy P = Policy::single> 
struct S : mutex_lock_t<P>
{
  void acquire(); 
  void release(); 
  int a{};   
};



template <Policy P> 
void S<P>::acquire() 
{
  std::cout << "inside general acquire " << ++a << std::endl;   
} 

template <Policy P> 
void S<P>::release() 
{
  std::cout << "inside general release " << --a << std::endl;   
} 


template <> 
void S<Policy::mutex>::acquire() 
{
  std::lock_guard<std::mutex> lock{_mutex}; 
  std::cout << "inside mutex acquire " << ++a << std::endl;   
} 

template <> 
void S<Policy::mutex>::release() 
{
  std::lock_guard<std::mutex> lock{_mutex}; 
  std::cout << "inside mutex release " << --a << std::endl;   
} 

int main() 
{
     S<Policy::mutex> s_mutex; 
     s_mutex.acquire(); 
     s_mutex.release(); 
     
     S s_single; 
     s_single.acquire(); 
     s_single.release(); 
     
     std::cout << "sizeof: mutex  " << sizeof(s_mutex) << ", single: " << sizeof(s_single) << '\n'; 
} 