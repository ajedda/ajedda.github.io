#include <iostream>
#include <utility> 

namespace detail 
{
   
   
// Why struct? I don't really like functions when dealing with templates. 
template <typename IndexSeq, typename F, typename... ArgsT>
struct repeat; 

template <std::size_t... n, typename F, typename... Args> 
struct repeat<std::index_sequence<n...>, F, Args...>
{
   void operator()(F gf, Args&&... args) 
   {

        
        // This generats a warning (e.g., in gcc, warning: right operand of comma operator has no effect [-Wunused-value]). 
        //((gf(), n),...);
        
        // How to get rid of this warning? 
        // use indirection as follows. 
        # if 0
        auto f = [&](std::size_t) { gf(std::forward<Args>(args)...); };
        (f(n),...);
        #endif 
        
        // or .. you may use some template wrapper like f<n>(gf); 
        
        // but I want the minimum templates written. 
        
        // so, I will temporarily supress warnings. 
        // it is probably the same amount of code. 
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wunused-value"
        ((gf(std::forward<Args>(args)...), n),...);
        #pragma GCC diagnostic pop
   } 
   
}; 



}


template <std::size_t n, typename F, typename... Args> 
void repeat(F gf, Args&&... args) 
{
    detail::repeat<std::make_index_sequence<n>, F, Args...>{}(gf, std::forward<Args>(args)...); 
}

int main() 
{
    auto f = []() { std::cout << "f"; };
    repeat<4>(f); 
    std::cout << '\n'; 
    
    auto g = [](char c) { std::cout << "g" << c << std::endl; };
    repeat<5>(g, 'a');
    
    auto h = [](char c, int n) { std::cout << c << n << ' '; };
    repeat<2>(h, 'd', 4); 
    

    for (std::size_t i{2}; (i < 5);  std::cout << i, i++) 
    {
    } 
} 