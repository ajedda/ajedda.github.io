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
struct raw_sub_list
{  
 using type = std::tuple<typename get_type<Ns, TypeList>::type...>; 
}; 

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


namespace detail 
{
     
template <typename T, typename... Args> 
struct type_exists
{
   static constexpr bool value = (std::is_same_v<T, Args> || ...);  
};

} 

template <typename T, typename TypeList> 
struct type_exists; 

template <typename T, template <typename...> typename TypeList, typename... Args>
struct type_exists<T, TypeList<Args...>> : detail::type_exists<T, Args...> 
{};

template <typename T, typename TypeList>
static constexpr bool type_exists_v = type_exists<T, TypeList>::value; 


// are all types in a type list unique?
namespace detail 
{
    template <typename TypeList, std::size_t first, std::size_t last>
    struct is_all_unique ; 
    
    // the base case. 
    template <typename TypeList, std::size_t last> 
    struct is_all_unique<TypeList, last, last> 
    {
        using head_type = get_type_t<last, TypeList>;
        using index_seq_type = typename ::make_index_seq<last-1>::type;
        using sublist_type = sub_list_t<TypeList,  index_seq_type >;     
        static constexpr bool value = ! type_exists_v<head_type, sublist_type>; 
    }; 
    
    // The head type should not exist in the sublist prior to head (i.e. from 0 to first-1). 
    template <typename TypeList, std::size_t first, std::size_t last> 
    struct is_all_unique
    {
         using head_type = get_type_t<first, TypeList>;
         // We are in namespace detail, so call the global namespace.
         using index_seq_type = typename ::make_index_seq<first-1>::type;  
         using sublist_type = sub_list_t<TypeList,  index_seq_type >; 
          
         static constexpr bool value = ! type_exists_v<head_type, sublist_type> && is_all_unique<TypeList, first+1, last>::value;
    };
    
    // some special cases. 
    // This is the case where there is only one type in the TypeList. (i.e., sizeof...(Args) = 1); 
    template <typename TypeList> 
    struct is_all_unique<TypeList, 1, 0> 
    {
        static constexpr bool value = true; 
    };
    
    // This is the case where there is no type in the TypeList. (i.e., sizeof...(Args) = 0); 
    template <typename TypeList> 
    struct is_all_unique<TypeList, 1, std::size_t(-1)> 
    {
        static constexpr bool value = true; 
    };
}

template <typename TypeList> 
struct is_all_unique; 

template <template <typename...> typename TypeList, typename... Args>
struct is_all_unique<TypeList<Args...>> : detail::is_all_unique< TypeList<Args...>, 
                                                            1, 
                                                            sizeof...(Args)-1>                                                        
{     
}; 

template <typename TypeList> 
constexpr static bool is_all_unique_v = is_all_unique<TypeList>::value;  


template <std::size_t n> 
struct make_even_index_seq;

template <std::size_t n> 
struct make_even_index_seq : detail::make_even_index_seq<0, n, index_seq<>> {}; 

// You can always replace SFINAE with concepts: Let's re-write make_even_index_seq with a concept. 
// The class is make_even_index_seq_cncp


namespace  detail 
{
    template <typename F, typename IndexSeq>
    struct filter_index_seq;     

    template <typename F> 
    struct filter_index_seq<F, index_seq<>>
    {
        using type = index_seq<>;
    }; 
    
    template <typename F, std::size_t N, std::size_t... Ns> 
    struct filter_index_seq<F, index_seq<N, Ns...>>
    {
        using type = std::conditional_t<F{}(N),   // TODO: is there a better way to call F{}()? 
                                        typename filter_index_seq<F, index_seq<Ns...>>::type,
                                        prepend_t<N, typename filter_index_seq<F, index_seq<Ns...>>::type>>;
                                        
    }; 
    
}

// This is the index seq implementatoin. 
template <typename F, typename IndexSeq> 
struct filter_index_seq;  

template <typename F, std::size_t... Ns> 
struct filter_index_seq<F, index_seq<Ns...>> : detail::filter_index_seq<F, index_seq<Ns...>> 
{
}; 

template <typename F, typename IndexSeq> 
using filter_index_seq_t = typename filter_index_seq<F, IndexSeq>::type; 


int main()
{

    using types_list = std::variant<int, double, short>; 
    using types_pair = std::pair<int, double>; 
    static_assert(std::is_same_v<get_type<0, types_pair>::type, int>); 
    static_assert(std::is_same_v<get_type<0, types_list>::type, int>); 
    static_assert(std::is_same_v<get_type<1, types_list>::type, double>);
    static_assert(std::is_same_v<get_type<2, types_list>::type, short>);  
    static_assert(std::is_same_v<get_type<0, int>::type, int>); 
    static_assert(std::is_same_v<get_type<0, int, double>::type, int>);
    
	enum class Type { Integer = 0, Float, Double}; 
    static_assert(std::is_same_v<get_type<Type::Integer, int, double>::type, int>);
    
    static_assert(std::is_same_v< sub_list<types_list, std::index_sequence<0, 2>>::type, std::tuple<int, short>>);
    static_assert(std::is_same_v< raw_sub_list<types_list, 0, 2>::type, std::tuple<int, short>>);
    
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
    static_assert(  type_exists_v<short, sub_list_t<  std::tuple<int, double, char, short>, make_index_seq<3>::type >>  );
    static_assert(  ! type_exists_v<short, sub_list_t<  std::tuple<int, double, char, short>, make_index_seq<2>::type >>  );
    
    struct FilterEvenFunc 
    {
       constexpr bool operator()(std::size_t i) { return i % 2 == 0; }
    }; 
    static_assert(std::is_same_v<filter_index_seq_t<FilterEvenFunc, index_seq<2,3,4,5>>, index_seq<3,5>>); 
    
    /*
    struct IntegerFilterFunc
    {
        constexpr bool operator()(std::size_t i) 
        {
            if constexpr (std::is_same_v<get_type_t<i, Tuple>, int>) { return true; } 
            return false; 
        } 
        using Tuple = std::tuple<int, double, int, char>;     
    }; 
    static_assert(std::is_same_v<filter_index_seq_t<IntegerFilterFunc, index_seq<1,2,3,4>>, index_seq<2,4>>); 
    */ 

    static_assert( is_all_unique_v<std::tuple<short>>); 
    static_assert( is_all_unique_v<std::tuple<short, int>>);
    static_assert(!is_all_unique_v<std::tuple<short, short>>);
    static_assert( is_all_unique_v<std::tuple<short, int, float, double>>);
    static_assert(!is_all_unique_v<std::tuple<short, int, float, short>>);
    static_assert(!is_all_unique_v<std::tuple<short, int, int, float>>);
    static_assert(!is_all_unique<std::tuple<short, int, char, float, float>>::value);
    static_assert( is_all_unique<std::tuple<>>::value); 
    
    std::variant<int, int> v;
    v=10; 
}