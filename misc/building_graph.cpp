#include <iostream>
#include <utility> 
#include <vector>
#include <tuple> 
#include <variant>


struct C1; 
struct B1; 
struct B2; 
struct A1; 

struct A1 
{   
   using depends_on = std::tuple<>; 
   friend std::ostream& operator<<(std::ostream& os, A1 const&) { return std::cout << "A1"; }
};



struct B1 
{
   using depends_on = std::tuple<A1, B2>;
   friend std::ostream& operator<<(std::ostream& os, B1 const&) { return std::cout << "B1"; }
}; 

struct B2 
{
   using depends_on = std::tuple<A1>;
   friend std::ostream& operator<<(std::ostream& os, B2 const&) { return std::cout << "B2"; }
}; 

struct C1
{
   using depends_on = std::tuple<B1, B2>;
   friend std::ostream& operator<<(std::ostream& os, C1 const&) { return std::cout << "C1"; }
}; 

template <typename F, typename... Ts>
void for_each(std::tuple<Ts...>& ts, F f) 
{
  (f(std::get<Ts>(ts)),...);   
} 



namespace detail 
{
    // forward declaration. 
   template <typename T> 
   struct create; 
   
   template <typename Tuple> 
   struct create_dep; 


   template <typename T> 
   struct create
   {
      template <typename V>
      void operator()(V& vec)
      {
        std::cout << "starting the creation of  : " << T{} << '\n'; 
        std::cout << "creating dep ... \n"; 
        using ds = typename T::depends_on; 
        detail::create<ds>{}(vec); 
        std::cout << "creating self .. \n"; 
        detail::create_dep<T>{}(vec); 
      } 
       
   }; 

   //template <template <typename... Ds> typename Tuple> 
   template <typename... Ds>
   struct create<std::tuple<Ds...>>
   {
       template <typename V> 
       void operator()(V& vec) 
       {    
            // find if there an element in the vec of type Ds. 
            // for each type in Ds. 
            (detail::create<Ds>{}(vec),...);       
       } 

   }; 
   
   
  template <typename D>
  struct create_dep 
  {
    
    template <typename V> 
    void operator()(V& vec) 
    {

        bool found = false; 
        for (auto& v: vec) 
        {
          if (auto pvar = std::get_if<D>(&v))
          { 
              found = true;
              std::cout << "found element with log: " << *pvar << '\n'; 
              break; 
          }
        } 
        
        
        
        if (!found) 
        {
          std::cout << "creating " << D{} << '\n';  
          vec.push_back(D{});
        } 
        
 
    } 
   };
  

  

   

 

} 

template <typename... Ts>
struct VarVectorBuilder 
{

   template <typename T> 
   void create() 
   {
        detail::create<T>{}(vec); 
   } 
   
   using var_t = std::variant<Ts...>; 
   std::vector<var_t> vec; 
}; 

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

  /*
  using var_t = std::variant<Ts...>; 
  std::vector<var_t> vec1; 
  (vec1.push_back(Ts{}), ...);
  return vec1; 
  */ 
  
  VarVectorBuilder<Ts...> var_vec_builder; 
  (var_vec_builder.template create<Ts>(), ...); 
  return var_vec_builder.vec; 
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

    auto vec1 = create_variant_vec<B1, C1, A1, B2>(); 
    for (auto& v: vec1) 
    {
      std::visit([](auto&& arg) { std::cout << arg; }, v);  
    } 
    std::cout << '\n'; 
    

    auto vec2 = create_variant_vec<A1, B1, B2, C1>(); 
    for (auto& v: vec2) 
    {
      std::visit([](auto&& arg) { std::cout << arg; }, v);  
    } 
    std::cout << '\n'; 
    
    auto vec3 = create_variant_vec<B1, C1, A1, B2>(); 
    for (auto& v: vec3) 
    {
      std::visit([](auto&& arg) { std::cout << arg; }, v);  
    } 
    std::cout << '\n'; 


    /*
    auto vec2 = create_tuple<A1, B1, B2, C1>(); 
    for_each(vec2, [](auto&& arg) { arg.log(); }); 
    */ 
} 