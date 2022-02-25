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


namespace detail 
{

  constexpr std::size_t get_number(std::size_t n) { return n; } 
  
  template <typename T> 
  constexpr std::size_t get_number(T n) { return static_cast<std::size_t>(n); }
} 

template <auto N, typename... Args>
struct get_type_base 
{
   static constexpr std::size_t n = detail::get_number(N); 
   using type = typename detail::get_type<n, Args...>::type; 
};



template <auto  N, typename... Args>
struct get_type : get_type_base<N, Args...>
{ 
}; 

template <auto  N, typename... Args>
struct get_type<N, types_list<Args...>> : get_type_base<N, Args...>
{
}; 






int main()
{

    using types_list = types_list<int, double, short>; 
    static_assert(std::is_same_v<get_type<0, types_list>::type, int>); 
    static_assert(std::is_same_v<get_type<1, types_list>::type, double>);
    static_assert(std::is_same_v<get_type<2, types_list>::type, short>);  
    static_assert(std::is_same_v<get_type<0, int>::type, int>); 
    static_assert(std::is_same_v<get_type<0, int, double>::type, int>);
    
	enum class Type { Integer = 0, Float, Double}; 
    static_assert(std::is_same_v<get_type<Type::Integer, int, double>::type, int>);
    
    
}