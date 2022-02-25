#include <iostream>
#include <utility> 
#include <vector>
#include <tuple> 
#include <variant>




struct A1 
{   
    using depends_on = std::tuple<>; 
    void log() { std::cout << "A1"; } 
};



struct B1 
{
   using depends_on = std::tuple<A1>;
   void log() { std::cout << "B1"; } 
}; 

struct B2 
{
   using depends_on = std::tuple<A1>;
   void log() { std::cout << "B2"; } 
}; 

struct C1
{
   using depends_on = std::tuple<B1, B2>;
   void log() { std::cout << "C1"; } 
}; 

template <typename F, typename... Ts>
void for_each(std::tuple<Ts...>& ts, F f) 
{
  (f(std::get<Ts>(ts)),...);   
} 

template <typename... Ts> 
auto create_variant_vec() 
{
    /*
    using var_t = std::variant<A1, B1, B2, C1>; 
    std::vector<var_t> vec1; 
    vec1.push_back(A1{}); 
    vec1.push_back(B1{}); 
    vec1.push_back(B2{}); 
    vec1.push_back(C1{}); 
    */ 

  using var_t = std::variant<Ts...>; 
  std::vector<var_t> vec1; 
  (vec1.push_back(Ts{}), ...);
  return vec1; 
} 

template <typename... Ts> 
auto create_tuple() 
{ 
 
  /*
  using tup_t = std::tuple<A1, B1, B2, C1>; 
  tup_t vec2; 
  std::get<A1>(vec2) = A1{}; 
  std::get<B1>(vec2) = B1{}; 
  std::get<B2>(vec2) = B2{}; 
  std::get<C1>(vec2) = C1{};
  */ 
    
  using tup_t = std::tuple<Ts...>; 
  tup_t vec2; 
  ((std::get<Ts>(vec2) = Ts{}), ...); 
  return vec2; 
} 

int main() 
{

    auto vec1 = create_variant_vec<A1, B1, B2, C1>(); 
    for (auto& v: vec1) 
    {
      std::visit([](auto&& arg) { arg.log(); }, v);  
    } 
    std::cout << '\n'; 
    

    
    auto vec2 = create_tuple<A1, B1, B2, C1>(); 
    for_each(vec2, [](auto&& arg) { arg.log(); }); 
    
} 