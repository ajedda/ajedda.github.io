#include <iostream> 
#include <type_traits>



template <typename T, typename... Others>
struct type_exists; 

template <typename T, typename F>
struct type_exists<T, F>
{
    static constexpr bool value = false; 
}; 

template <typename T>
struct type_exists<T, T>
{
    static constexpr bool value = true; 
};

template <typename T, typename... Others>
struct type_exists<T, T, Others...> 
{
    static constexpr bool value = true; 
}; 

template <typename T, typename F, typename... Others>
struct type_exists<T, F, Others...>
{
    static constexpr bool value = type_exists<T, Others...>::value;  
}; 



int main()
{
    
    std::cout << type_exists<int, int>::value; 
    std::cout << type_exists<int, bool>::value; 
    std::cout << type_exists<int, int, bool>::value; 
    std::cout << type_exists<int, bool, char>::value; 
    std::cout << type_exists<int, bool, char, int>::value;  
}