#include <iostream> 
#include <type_traits> 

template <std::size_t... N> 
struct integer_seq{};
 
template <typename T, typename S>
struct prepend_to_integer_seq; 

template <std::size_t F, std::size_t... N> 
struct prepend_to_integer_seq<integer_seq<F>, integer_seq<N...>> 
{ 
  using type = integer_seq<F, N...>;   
};

// Note that we can do more fun using the technique above with SFINAE. 
namespace details
{
template <typename T, typename S>
struct another_prepend_to_integer_seq; 

template <std::size_t F, std::size_t... N> 
struct another_prepend_to_integer_seq<integer_seq<F>, integer_seq<N...>> 
{ 
  using type = integer_seq<F, N...>;   
};



}
template <typename T, typename S> 
struct another_prepend_to_integer_seq : details::another_prepend_to_integer_seq<T, S> {};


// This technique gives us some flexibility to do some fun stuff. 


namespace details
{
template <typename T, typename Enable=void>
struct rm_odds_from_integer_seq; 

template <>
struct rm_odds_from_integer_seq<integer_seq<>, void>
{
    using type = integer_seq<>; 
}; 

template <std::size_t N0, std::size_t... N> 
struct rm_odds_from_integer_seq<integer_seq<N0, N...>, std::enable_if_t<N0%2==1>> 
{ 
  using type = typename rm_odds_from_integer_seq<integer_seq<N...>>::type;  
};

template <std::size_t N0, std::size_t... N> 
struct rm_odds_from_integer_seq<integer_seq<N0, N...>, std::enable_if_t<N0%2==0>> 
{ 
  using I = typename rm_odds_from_integer_seq<integer_seq<N...>>::type; 
  using type = typename prepend_to_integer_seq<integer_seq<N0>, I >::type; 
};


}
template <typename T> 
struct rm_odds_from_integer_seq : details::rm_odds_from_integer_seq<T, void> {};


// This recursion is quite useful. We can do a make_integer_sequence with it. 

namespace details 
{
    template <std::size_t N, typename integer_seqType, typename Enable=void>
    struct make_desc_integer_seq;    
    
    template <std::size_t N, std::size_t... Ns>
    struct make_desc_integer_seq<N, integer_seq<Ns...>, std::enable_if_t<(N <= 0)>>
    {
       using type = integer_seq<N, Ns...>;
    }; 
    
    template <std::size_t N, std::size_t... Ns>
    struct make_desc_integer_seq<N, integer_seq<Ns...>, std::enable_if_t<(N > 0)>>
    {
       using I =  typename make_desc_integer_seq<N-1, integer_seq<Ns...>>::type;
       using type = typename prepend_to_integer_seq<integer_seq<N>, I >::type; 
    };
    
}

template <std::size_t N> 
struct make_desc_integer_seq : details::make_desc_integer_seq<N, integer_seq<>, void> {}; 


namespace details 
{
    template <std::size_t J, std::size_t N, typename integer_seqType, typename Enable=void>
    struct make_integer_seq;    
    
    template <std::size_t J, std::size_t N, std::size_t... Ns>
    struct make_integer_seq<J, N, integer_seq<Ns...>, std::enable_if_t<(J >= N)>>
    {
       using type = integer_seq<N, Ns...>;
    }; 
    
    template <std::size_t J, std::size_t N, std::size_t... Ns>
    struct make_integer_seq<J, N, integer_seq<Ns...>, std::enable_if_t<(J < N)>>
    {
       using I =  typename make_integer_seq<J+1, N, integer_seq<Ns...>>::type;
       using type = typename prepend_to_integer_seq<integer_seq<J>, I >::type; 
    };
    
}

template <std::size_t N> 
struct make_integer_seq : details::make_integer_seq<0, N, integer_seq<>, void> {}; 


int main()
{ 
    static_assert(std::is_same_v<integer_seq<1,2,3>, prepend_to_integer_seq<integer_seq<1>,integer_seq<2,3>>::type>);
    static_assert(std::is_same_v<integer_seq<1,2,3>, another_prepend_to_integer_seq<integer_seq<1>,integer_seq<2,3>>::type>);
    static_assert(std::is_same_v<integer_seq<2,4,6,8,10,4>, rm_odds_from_integer_seq<integer_seq<1,2,3,4,5,6,8,10,4>>::type>);
    
    // You probably have noticed that we don't need to hide the implementation into a detail class.
    static_assert(std::is_same_v<integer_seq<2,4,6,8,10,4>, details::rm_odds_from_integer_seq<integer_seq<1,2,3,4,5,6,8,10,4>>::type>);
    
    
    static_assert(std::is_same_v<make_desc_integer_seq<0>::type, integer_seq<0>>);
    static_assert(std::is_same_v<make_desc_integer_seq<1>::type, integer_seq<1,0>>);
    static_assert(std::is_same_v<make_desc_integer_seq<2>::type, integer_seq<2,1,0>>); 
    
    static_assert(std::is_same_v<make_integer_seq<0>::type, integer_seq<0>>);
    static_assert(std::is_same_v<make_integer_seq<1>::type, integer_seq<0,1>>);
    static_assert(std::is_same_v<make_integer_seq<2>::type, integer_seq<0,1,2>>); 
} 
