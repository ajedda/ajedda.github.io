#include <iostream> 
#include <type_traits>
#include <tuple>
#include <optional>

template <typename... Args>
struct types_list {}; 


namespace detail 
{ 
    
template <typename T, typename... Others>
struct type_exists; 

template <typename T, typename F>
struct type_exists<T, F> : std::is_same<T, F> {};  


template <typename T, typename F, typename... Others>
struct type_exists<T, F, Others...>
{
    static constexpr bool value =  std::is_same_v<T, F> || type_exists<T, Others...>::value;  
}; 

}



template <typename T, typename... Others>
struct type_exists
{
   static constexpr bool value = detail::type_exists<T, Others...>::value;  
}; 

template <typename T, typename... Others>
struct type_exists<T, types_list<Others...>>
{
   static constexpr bool value = detail::type_exists<T, Others...>::value;  
}; 


template <typename T, typename... Others>
struct type_exists<T, std::tuple<Others...>>
{
   static constexpr bool value = detail::type_exists<T, Others...>::value;  
};
 

template <typename T, typename... Others>
static constexpr bool type_exists_v = type_exists<T, Others...>::value; 

template <typename T, typename... Args>
std::optional<T> opt_get(std::tuple<Args...> const& t)
{
    if constexpr (type_exists_v<T, Args...>)
    {
        return std::get<T>(t);    
    }
    return std::nullopt; 
} 

int main()
{
    
    std::cout << type_exists<int, int>::value; 
    std::cout << type_exists<int, bool>::value; 
    std::cout << type_exists<int, types_list<int, bool>>::value; 
    std::cout << type_exists<int, bool, char>::value; 
    std::cout << type_exists_v<int, bool, char, int>;    
    std::cout << type_exists_v<int, types_list<int, bool, char, double>>; 
    std::cout << type_exists_v<int, std::tuple<int, bool, char, double>>; 
    
    std::cout << std::endl; 
    std::tuple<int, bool, char, double> t; 
    std::get<double>(t) = 3; 
    std::cout << std::get<double>(t); 
   
    auto print_opt = [](auto const& opt) { if (opt) { std::cout << *opt; } else { std::cout << "N";}}; 
    print_opt(opt_get<double>(t)); 
    print_opt(opt_get<float>(t)); 
}