#include <iostream> 
#include <type_traits> 
#include <array>


template <std::size_t... N> 
struct integer_seq
{    
};
 
template <typename T, typename S>
struct concat_integer_seq; 

template <std::size_t... F, std::size_t... N> 
struct concat_integer_seq<integer_seq<F...>, integer_seq<N...>> 
{ 
  using type = integer_seq<F..., N...>;   
};

 
template <typename A, std::size_t... N> 
void f(A& arr, integer_seq<N...>)
{
    ((std::cout<< arr[N] << " - "),...);
} 


int main()
{ 
    std::array<int, 4> arr{0,1,2,3}; 
    f(arr, integer_seq<1,3>{}); 
    
    static_assert(std::is_same<concat_integer_seq<integer_seq<1,2>, integer_seq<3,4>>::type, integer_seq<1,2,3,4>>::value, ""); 
} 

