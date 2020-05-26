// shared_ptr custom deleter vs. unique_ptr custom deleter

#include <iostream> 
#include <memory> 



void func_deleter(int*) { std::cout << "calling func deleter \n"; }



template <typename CallableParent> 
struct Child : CallableParent 
{
   Child() = default; 
   
   Child(CallableParent const& cp) 
    : CallableParent{cp}
   {} 
   
   ~Child() 
   { 
       std::cout << "~Child -> "; 
       CallableParent::operator()(&_x); 
   }
   int _x{}; 
}; 

int main() 
{
    struct DefConstDeleter 
    { 
        void operator()(int*) { std::cout << "Calling Def Cons Deleter \n"; } 
    }; 
    struct CopyConstDeleter 
    {
       CopyConstDeleter(std::string const& msg) : _msg{msg} {} 
       void operator()(int*) { std::cout << "Calling copy const deleter with msg: " << _msg << '\n'; }
       std::string _msg; 
    }; 
    
    auto lambda_deleter = [](int*) { std::cout << "lambda deleter \n"; }; 
    
    int x{5}; 
 
    std::unique_ptr<int> p1{new int{10}}; 
    std::unique_ptr<int, DefConstDeleter> p2{&x}; 
    //std::unique_ptr<int> p2c{&x, DefConstDeleter{}};  // This will not compile. 
    std::unique_ptr<int, CopyConstDeleter> p3{&x, CopyConstDeleter{"from main"}}; 
    
    
    //std::unique_ptr<int, decltype(func_deleter)> p5{&x, func_deleter};  // that doesn't work. 
    std::unique_ptr<int, decltype(lambda_deleter)> p4{&x, lambda_deleter};  // but this works (lambda is a callable struct if you go deep). 
                                                                            // You can inherit from lambda.
    
    static_assert(! std::is_same_v<decltype(p1), decltype(p2)>);
    static_assert(! std::is_same_v<decltype(p1), decltype(p3)>);
    static_assert(! std::is_same_v<decltype(p1), decltype(p4)>);
    static_assert(! std::is_same_v<decltype(p2), decltype(p3)>);
    static_assert(! std::is_same_v<decltype(p2), decltype(p4)>); 
    static_assert(! std::is_same_v<decltype(p3), decltype(p4)>); 
    
    
    // It is different in shared_ptr. 
    std::shared_ptr<int> sp1{new int{10}}; 
    std::shared_ptr<int> sp2{&x, DefConstDeleter{}}; 
    std::shared_ptr<int> sp3{&x, CopyConstDeleter{"from main"}}; 
    std::shared_ptr<int> sp4{&x, lambda_deleter}; 
    std::shared_ptr<int> sp5{&x, func_deleter};        
    
    // They are all the same type. It doesn't hurt to check. 
    static_assert(std::is_same_v<decltype(sp1), decltype(sp2)>);
    static_assert(std::is_same_v<decltype(sp2), decltype(sp3)>);
    static_assert(std::is_same_v<decltype(sp3), decltype(sp4)>);
    
    
    Child<DefConstDeleter> c1; 
    Child<CopyConstDeleter> c2{CopyConstDeleter{"from another main"}}; 
    std::cout << sizeof(c1) << " - " << sizeof(c2) << " --- " << sizeof(int) << " - " << sizeof(std::string) << std::endl; 
    
}