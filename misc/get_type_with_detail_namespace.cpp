#include <iostream> 
#include <type_traits>



template <typename... Args> 
struct types_list; 





namespace detail
{
    template <std::size_t N, typename... Args> 
    struct get_type;
    
    template <typename T, typename... Others>
    struct get_type<0, T, Others...> 
    {
        using type = T;    
    };

    template <std::size_t N, typename T, typename... Others>
    struct get_type<N, T, Others...>
    {
        using type = typename get_type<N-1, Others...>::type; 
    }; 
}


template <std::size_t N, typename... Args> 
struct get_type; 

template <std::size_t N, typename... Others>
struct get_type
{
   using type = typename detail::get_type<N, Others...>::type;  
}; 

template <std::size_t N, typename... Others>
struct get_type<N, types_list<Others...>>
{
   using type = typename detail::get_type<N, Others...>::type; 
}; 




int main()
{

    using types_list = types_list<int, double, short>; 
    static_assert(std::is_same_v<get_type<0, types_list>::type, int>); 
    static_assert(std::is_same_v<get_type<1, types_list>::type, double>);
    static_assert(std::is_same_v<get_type<2, types_list>::type, short>);  
    static_assert(std::is_same_v<get_type<0, int>::type, int>); 
    static_assert(std::is_same_v<get_type<0, int, double>::type, int>); 
    
}