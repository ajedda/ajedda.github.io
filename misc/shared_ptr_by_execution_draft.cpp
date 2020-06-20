#include <iostream> 
#include <type_traits>
#include <atomic> 
#include <tuple>

enum class Mode 
{
    S,
    M
}; 

namespace ver1
{
    
template <Mode M> 
struct ref_counter
{
    void acquire();      
}; 

template <>
void ref_counter<Mode::M>::acquire() { std::cout << "inside mode M \n"; }

template <>
void ref_counter<Mode::S>::acquire() { std::cout << "inside mode S \n"; }


}// ver1

namespace ver2
{

template <Mode M> 
struct counter_type;

template <>
struct counter_type<Mode::M>
{
   std::atomic<int> v; 
}; 

template <>
struct counter_type<Mode::S>
{
   int v; 
}; 

template <Mode M>
struct ref_counter
{
    counter_type<M> _counter;
}; 
    
}

namespace ver3
{
    
template <Mode M> 
struct counter_type;

template <>
struct counter_type<Mode::M>
{
   using type = std::atomic<int>; 

}; 

template <>
struct counter_type<Mode::S>
{
   using type = int; 
}; 

template <Mode M>
struct ref_counter
{
    typename counter_type<M>::type _counter;
    
    // and to remove that typename we can do: 
    using counter_type = typename counter_type<M>::type; 
    counter_type _c; 
};  
}


// namespace ver4: 
// What about we make that more general. 
namespace ver4
{
    template <bool, typename F, typename S> 
    struct conditional; 
    
    template <typename F, typename S> 
    struct conditional<true, F, S> 
    {
        using type = F; 
    }; 

    template <typename F, typename S> 
    struct conditional<false, F, S> 
    {
        using type = S; 
    }; 
    
    template <Mode M>
    struct ref_counter
    {
          
        // and to remove that typename we can do: 
        using counter_type = typename conditional<M==Mode::M, std::atomic<int>, int>::type; 
        counter_type _c; 
    };
} 

namespace ver5
{

    template <Mode M>
    struct ref_counter
    {
          
        // and to remove that typename we can do: 
        using counter_type = typename std::conditional<M==Mode::M, std::atomic<int>, int>::type; 
        counter_type _c; 
    };
}

int main() 
{
    ver1::ref_counter<Mode::M> rf1; rf1.acquire(); 
    ver1::ref_counter<Mode::S> rf2; rf2.acquire(); 
    
    ver2::ref_counter<Mode::M> rf3; 
    static_assert(std::is_same_v<decltype(rf3._counter.v), std::atomic<int>>); 
    ver2::ref_counter<Mode::S> rf4; 
    static_assert(std::is_same_v<decltype(rf4._counter.v), int>); 
    
    ver3::ref_counter<Mode::M> rf5; 
    static_assert(std::is_same_v<decltype(rf5._counter), std::atomic<int>>); 
    ver3::ref_counter<Mode::S> rf6; 
    static_assert(std::is_same_v<decltype(rf6._counter), int>); 
    
    ver3::ref_counter<Mode::M> rf7; 
    static_assert(std::is_same_v<decltype(rf7._c), std::atomic<int>>); 
    ver3::ref_counter<Mode::S> rf8; 
    static_assert(std::is_same_v<decltype(rf8._c), int>); 
    
    ver4::ref_counter<Mode::M> rf9; 
    static_assert(std::is_same_v<decltype(rf9._c), std::atomic<int>>); 
    ver4::ref_counter<Mode::S> rf10; 
    static_assert(std::is_same_v<decltype(rf10._c), int>); 
    

} 