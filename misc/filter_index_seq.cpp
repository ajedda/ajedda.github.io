// refactored all_unique. 
#include <iostream> 
#include <type_traits>
#include <utility>
#include <variant> 
#include <tuple>

template <typename... Args> 
struct types_list; 





namespace detail
{
    template <std::size_t n, typename... Args> 
    struct get_type;
    
    template <typename T, typename... Others>
    struct get_type<0, T, Others...> 
    {
        using type = T;    
    };

    template <std::size_t n, typename T, typename... Others>
    struct get_type<n, T, Others...>
    {
        using type = typename get_type<n-1, Others...>::type; 
    }; 
}


namespace detail 
{

  constexpr std::size_t get_number(std::size_t n) { return n; } 
  
  template <typename T> 
  constexpr std::size_t get_number(T n) { return static_cast<std::size_t>(n); }
} 


template <auto n, typename... Args>
struct get_type_base 
{
   static constexpr std::size_t sn = detail::get_number(n); 
   using type = typename detail::get_type<sn, Args...>::type; 
};



template <auto  n, typename... Args>
struct get_type : get_type_base<n, Args...>
{ 
}; 


template <auto n, template <typename...> typename TypeContainer, typename... Args> 
struct get_type<n, TypeContainer<Args...>> : get_type_base<n, Args...> {}; 

template <auto n, typename... Args> 
using get_type_t = typename get_type<n, Args...>::type; 


template <typename TypeList, std::size_t... Ns> 
struct raw_sub_list;

template <template <typename ...> typename TypeList, typename... Args, std::size_t... Ns> 
struct raw_sub_list<TypeList<Args...>, Ns...>
{  
 using type = TypeList<typename get_type<Ns, TypeList<Args...>>::type...>; 
};

/*
template <typename TypeList, std::size_t... Ns> 
struct raw_sub_list
{  
 using type = std::tuple<typename get_type<Ns, TypeList>::type...>; 
};
*/ 

template <typename TypeList, typename Indices> 
struct sub_list; 

template <typename TypeList, std::size_t... Ns> 
struct sub_list<TypeList, std::index_sequence<Ns...>> : raw_sub_list<TypeList, Ns...>
{
}; 

template <typename TypeList, template <std::size_t...> typename IndexSeq, std::size_t... Ns>
struct sub_list<TypeList, IndexSeq<Ns...>> : raw_sub_list<TypeList, Ns...>
{
}; 

template <typename TypeList, typename Indices> 
using sub_list_t = typename sub_list<TypeList, Indices>::type; 

// There are some cool things we can do with index_sequences. 
// For example: we may want to create an index sequence from 0 to n, or from m to n (in ascending or desceding order). 
// This has some interesting applications. 


// This is the index seq implementatoin. 
template <std::size_t... Ns> 
struct index_seq {}; 

// How to make a sequence given N? 

// A very important basic operation; prepend. 
template <std::size_t n, typename Indices> 
struct prepend; 

template <std::size_t n, std::size_t... Ns> 
struct prepend<n, index_seq<Ns...>>
{ 
    using type = index_seq<n, Ns...>; 
}; 

template <std::size_t n, typename Indices> 
using prepend_t = typename prepend<n, Indices>::type; 


// append follows the same concept. I am adding it here becuase the syntax may seem incorrect to some. 
template <std::size_t n, typename Indices> 
struct append; 

template <std::size_t n, std::size_t... Ns> 
struct append<n, index_seq<Ns...>>
{ 
    using type = index_seq<Ns..., n>; 
}; 

template <std::size_t n, typename Indices> 
using append_t = typename append<n, Indices>::type; 



// 
template <typename Indices1, typename Indices2> 
struct concat; 

template <std::size_t... Ms, std::size_t... Ns> 
struct concat<index_seq<Ms...>, index_seq<Ns...>>
{ 
    using type = index_seq<Ms..., Ns...>; 
}; 

template <typename Indices1, typename Indices2> 
using concat_t = typename concat<Indices1, Indices2>::type; 



// We implemented a append, let's create a make_index_seq?


namespace detail 
{
    template <std::size_t I, std::size_t N, typename Indices> 
    struct make_index_seq;
    
    template <std::size_t N, typename Indices>
    struct make_index_seq<N, N, Indices>
    {
       using type = prepend_t<N, Indices>;  
    }; 
    
    
    template <std::size_t I, std::size_t N, typename Indices>
    struct make_index_seq
    {
       using sub_seq_type = typename make_index_seq<I+1, N, Indices>::type; 
       using type = prepend_t<I, sub_seq_type>; 
    }; 

}

template <std::size_t from, std::size_t to=0>
struct make_index_seq; 

// If you only provide one parameter (such as: make_index_seq<4>), we assume it starts from zero.
// see the default to is set to zero. 
template <std::size_t to> 
struct make_index_seq<to, 0> : detail::make_index_seq<0, to, index_seq<>> {};  

template <std::size_t from, std::size_t to> 
struct make_index_seq : detail::make_index_seq<from, to, index_seq<>> {}; 

// Note: until now, we didn't use any thing other than template specialization. 

// but what if for example, you wanted only even numbers in the index sequence? 
// this is where we will start dealing with SFINAE (or more modernly concepts and the require keyword). 




namespace detail 
{
     
template <typename T, typename... Args> 
struct has_type
{
   static constexpr bool value = (std::is_same_v<T, Args> || ...);  
};

} 

template <typename T, typename TypeList> 
struct has_type; 

template <typename T, template <typename...> typename TypeList, typename... Args>
struct has_type<T, TypeList<Args...>> : detail::has_type<T, Args...> 
{};

template <typename T, typename TypeList>
static constexpr bool has_type_v = has_type<T, TypeList>::value; 


// are all types in a type list unique?
namespace detail 
{
    
    template <typename TypeList, typename IndexSeq>
    struct is_all_unique; 
    
    template <typename TypeList, 
              template <std::size_t...> typename IndexSeq, 
              std::size_t... Ns>
    struct is_all_unique<TypeList, IndexSeq<Ns...>>
    {
        static constexpr bool value = (! has_type_v< get_type_t<Ns, TypeList>, 
                                                        sub_list_t<TypeList, typename ::make_index_seq<Ns-1>::type>> && ...); 
    }; 
    
 

}

template <typename TypeList> 
struct is_all_unique; 


template <template <typename...> typename TypeList, typename... Args> 
struct is_all_unique<TypeList<Args...>> : detail::is_all_unique< TypeList<Args...>, 
                                                                 typename make_index_seq<1, sizeof...(Args)-1>::type> 
{}; 

// Some special cases: sizeof list 1 or zero. 
template <template <typename...> typename TypeList, typename... Args> requires (sizeof...(Args) <= 1)
struct is_all_unique<TypeList<Args...>> 
{
   static constexpr bool value = true;  
}; 



template <typename TypeList> 
constexpr static bool is_all_unique_v = is_all_unique<TypeList>::value;  



namespace detail 
{ 
    template <std::size_t i, std::size_t n, typename Indices, typename Enable=void> 
    struct make_even_index_seq;
    
    template <std::size_t i, std::size_t n, typename Indices>
    struct make_even_index_seq<i, n, Indices, std::enable_if_t<(i > n)>>
    {
       using type = Indices; 
    }; 
    
    
    template <std::size_t i, std::size_t n, typename Indices>
    struct make_even_index_seq<i, n, Indices, std::enable_if_t<!(i > n)>>
    {
       using sub_seq_type = typename make_even_index_seq<i+2, n, Indices>::type; 
       using type = prepend_t<i, sub_seq_type>; 
    };
} 



template <std::size_t n> 
struct make_even_index_seq;

template <std::size_t n> 
struct make_even_index_seq : detail::make_even_index_seq<0, n, index_seq<>> {}; 

// You can always replace SFINAE with concepts: Let's re-write make_even_index_seq with a concept. 
// The class is make_even_index_seq_cncp

namespace detail 
{
    template <typename IndexSeq>
    struct filter_evens;     

    template <> 
    struct filter_evens<index_seq<>>
    {
        using type = index_seq<>;
    }; 
    
    template <std::size_t N, std::size_t... Ns> 
    struct filter_evens<index_seq<N, Ns...>>
    {       
        using type = std::conditional_t<(N%2==0),  
                                        typename filter_evens<index_seq<Ns...>>::type,
                                        prepend_t<N, typename filter_evens<index_seq<Ns...>>::type>  >;
                                        
    }; 
    
}


// This is the index seq implementatoin. 
template <typename IndexSeq> 
struct filter_evens : detail::filter_evens<IndexSeq>
{
};

template <typename IndexSeq> 
using filter_evens_t = typename filter_evens<IndexSeq>::type; 




namespace  detail 
{
    template <typename IndexSeq, template <auto, typename...> typename F, typename... FArgs>
    struct filter_index_seq;     

    template <template <auto, typename...> typename F, typename... FArgs> 
    struct filter_index_seq<index_seq<>, F, FArgs...>
    {
        using type = index_seq<>;
    }; 
    
    template <template <auto, typename...> typename F, typename... FArgs, auto N, auto... Ns> 
    struct filter_index_seq<index_seq<N, Ns...>, F, FArgs...>
    {
        constexpr static bool fn_eval = F<N, FArgs...>::value; 
        using type = std::conditional_t<fn_eval,  
                                        typename filter_index_seq<index_seq<Ns...>, F, FArgs...>::type,
                                        prepend_t<N, typename filter_index_seq<index_seq<Ns...>, F, FArgs...>::type>>;
                                        
    }; 
    
}

 
template <typename IndexSeq, template <auto, typename...> typename F, typename... FArgs> 
struct filter_index_seq : detail::filter_index_seq<IndexSeq, F, FArgs...>
{
};

template <typename IndexSeq, template <auto, typename...> typename F, typename... FArgs> 
using filter_index_seq_t = typename filter_index_seq<IndexSeq, F, FArgs...>::type; 


template <template <typename...> typename F, typename I>
struct Eval
{
    constexpr static bool value = F<I>::value;    
    //constexpr static bool value = true; 
};

template <std::size_t i> 
struct IsEven
{
   constexpr static bool value = (i % 2 == 0);  
}; 

template <typename T> 
struct IsEvenType
{
   constexpr static bool value = (T::value % 2 == 0);  
}; 

template <std::size_t i> 
struct Number { static constexpr std::size_t value = i; };


template <std::size_t i>
struct IsIndexOfInteger
{
    using Tuple = std::tuple<double, int, char, int>;
    constexpr static bool value = std::is_same_v<get_type_t<i, Tuple>, int>; 
         
}; 

template <std::size_t i, typename TL> 
struct IsIndexOfDouble
{
   constexpr static bool value = std::is_same_v<get_type_t<i, TL>, double>; 
   // obviously nothing stops from doing more complex logic, especially when you 
   // have the capability to pass more types in addition to TL. 
}; 

template <std::size_t i, typename TL> 
struct IsPreviouslyOccured
{

    // How to implement this such that it can be used to get the 
    // unique types in a types list. 
    //constexpr static bool value = true; 
    using current_type = get_type_t<i, TL>; 
    using p_types_sublist = sub_list_t<TL, typename ::make_index_seq<i-1>::type>; 
    constexpr static bool value = has_type_v< current_type, p_types_sublist>;

}; 


template <typename TL> 
struct IsPreviouslyOccured<0, TL> 
{
   constexpr static bool value = false;  
};


int main()
{
    std::cout << Eval<IsEvenType, Number<1>>::value << std::endl;
    std::cout << Eval<IsEvenType, Number<2>>::value << std::endl; 
     

    using tl = types_list<int, double, short>; 
    using types_pair = std::pair<int, double>; 
    static_assert(std::is_same_v<get_type<0, tl>::type, int>); 
    static_assert(std::is_same_v<get_type<0, tl>::type, int>); 
    static_assert(std::is_same_v<get_type<1, tl>::type, double>);
    static_assert(std::is_same_v<get_type<2, tl>::type, short>);  
    static_assert(std::is_same_v<get_type<0, int>::type, int>); 
    static_assert(std::is_same_v<get_type<0, int, double>::type, int>);
    static_assert(std::is_same_v<get_type<0, types_pair>::type, int>); 
    static_assert(std::is_same_v<get_type<1, types_pair>::type, double>); 
    
    
	enum class Type { Integer = 0, Float, Double}; 
    static_assert(std::is_same_v<get_type<Type::Integer, int, double>::type, int>);
    
    static_assert(std::is_same_v< sub_list<tl, std::index_sequence<0, 2>>::type, types_list<int, short>>);
    static_assert(std::is_same_v< raw_sub_list<tl, 0, 2>::type, types_list<int, short>>);
    
    using index_seq_type = index_seq<1,2,3,6>; 
    index_seq_type seq_1; 
    (void) seq_1; 
    
    static_assert(std::is_same_v< prepend_t<4, index_seq<1,2,3>>, index_seq<4, 1,2,3> >);
    static_assert(std::is_same_v< append_t<4, index_seq<1,2,3>>, index_seq<1,2,3, 4> >);
    static_assert(std::is_same_v< concat_t<index_seq<5,6,7>, index_seq<1,2,3>>, index_seq<5,6,7,1,2,3> >);
    
    static_assert(std::is_same_v<make_index_seq<3>::type, index_seq<0,1,2,3>>);  
    static_assert(std::is_same_v<make_even_index_seq<5>::type, index_seq<0,2,4>>);
    static_assert(std::is_same_v<make_index_seq<4,8>::type, index_seq<4,5,6,7,8>>); 
    
    
    static_assert(std::is_same_v<std::tuple<int, double, char>, raw_sub_list<std::tuple<int, double, char, short int>, 0, 1, 2>::type>); 
    static_assert(  has_type_v<short, sub_list_t<  std::tuple<int, double, char, short>, make_index_seq<3>::type >>  );
    static_assert(  ! has_type_v<short, sub_list_t<  std::tuple<int, double, char, short>, make_index_seq<2>::type >>  );
    



    static_assert( is_all_unique_v<std::tuple<short>>); 
    static_assert( is_all_unique_v<std::tuple<short, int>>);
    static_assert(! is_all_unique_v<std::tuple<short, short>>);
    static_assert( is_all_unique_v<std::tuple<short, int, float, double>>);
    static_assert(!is_all_unique_v<std::tuple<short, int, float, short>>);
    static_assert(!is_all_unique_v<std::tuple<short, int, int, float>>);
    static_assert(!is_all_unique<std::tuple<short, int, char, float, float>>::value);
    static_assert( is_all_unique<std::tuple<>>::value); 
    
    
    static_assert(std::is_same_v<filter_index_seq_t<index_seq<2,3,4,5>, IsEven>, index_seq<3,5>>); 
    static_assert(std::is_same_v<filter_evens_t<index_seq<2,3,4,5>>, index_seq<3,5>>); 
    // Let's do something more complex. 
    static_assert(std::is_same_v<filter_index_seq_t<index_seq<0, 1, 2>, IsIndexOfInteger>, index_seq<0, 2>>); 
    using DList = std::variant<double, int, char, int>; 
    static_assert(std::is_same_v<filter_index_seq_t<index_seq<0, 1, 2, 3>, IsIndexOfDouble, DList>, index_seq<1,2,3>>); 
    
    using f_index_dlist = filter_index_seq_t<index_seq<0,1,2,3>, IsIndexOfDouble, DList>; 
    using dlist_no_doubles = sub_list_t< DList,  f_index_dlist >;
    static_assert(std::is_same_v< dlist_no_doubles, std::variant<int, char, int>>);

    
    static_assert(std::is_same_v<filter_index_seq_t<index_seq<0, 1, 2, 3>, IsPreviouslyOccured, DList>, index_seq<0, 1, 2>>);
    using u_index_dlist = filter_index_seq_t<index_seq<0,1,2,3>, IsPreviouslyOccured, DList>; 
    using list_unique_types = sub_list_t< DList,  u_index_dlist >;
    static_assert(std::is_same_v< list_unique_types, std::variant<double, int, char>>);
}
