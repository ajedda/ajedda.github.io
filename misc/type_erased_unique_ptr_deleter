#include <iostream> 
#include <memory> 


namespace ver3
{
  
  template <typename T>
  struct TypeErasedDeleter
  {
    
    TypeErasedDeleter() 
        : deleter{std::make_unique<Deleter>()}
    {} 
    
    template <typename DeleterType>
    TypeErasedDeleter(DeleterType const& d) 
        : deleter{std::make_unique<DeleterModel<DeleterType>>(d)}
    {} 
    
    void operator()(T* ptr) { (*deleter)(ptr); }
    
     
    struct Deleter 
    {
       virtual void operator()(T* ptr) { std::cout << "unimplemented deleter \n";  delete ptr; }
       virtual ~Deleter() = default;
    }; 
    
    template <typename DeleterType>
    struct DeleterModel : Deleter 
    {
       DeleterModel(DeleterType const& d) : _d{d} {} 
       void operator()(T* ptr) override {_d(ptr); } 
       DeleterType _d; 
    }; 

    std::unique_ptr<Deleter> deleter; 
  }; 
    
} 

struct DefDeleter 
{
   void operator()(int* ptr) { std::cout << "def deleter \n"; delete ptr;  } 
}; 

struct AnotherDeleter 
{
   AnotherDeleter(int x) : _x{x} {}
   void operator()(int* ptr) { std::cout << "another  deleter " << _x << '\n'; delete ptr;  }
   int _x{}; 
};

template <typename T> 
using unique_ptr_deleter = std::unique_ptr<T, ver3::TypeErasedDeleter<T>>; 

void func(int* ptr) { std::cout << "func deleter \n"; delete ptr; }

int main() 
{
    ver3::TypeErasedDeleter<int> td1{DefDeleter{}};
    td1(new int);
    
    ver3::TypeErasedDeleter<int> td2{AnotherDeleter{10}}; 
    td2(new int); 
    
    unique_ptr_deleter<int> up1{new int, DefDeleter{}}; 
    unique_ptr_deleter<int> up2{new int, AnotherDeleter{5}}; 
    unique_ptr_deleter<int> up3{new int}; 
    unique_ptr_deleter<int> up4{new int, [](auto* ptr) { std::cout << "from lambda \n"; delete ptr; }}; 
    unique_ptr_deleter<int> up5{new int, &func}; // You were not able to do with std::unique_ptr. 
    
    std::cout << "size of up1: " << sizeof(up1) 
              << " size of up2: " << sizeof(up2) 
              << " size of up3: " << sizeof(up3) << '\n';  
    
   
} 