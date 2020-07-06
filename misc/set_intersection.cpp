#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>


namespace my 
{

    
   template <typename Iterator1, typename Iterator2, typename OutIterator, typename Comparator, 
            typename Combiner
            >
  OutIterator set_intersection(Iterator1 first1, Iterator1 last1, 
                               Iterator2 first2, Iterator2 last2, 
                               OutIterator out_it, 
                               Comparator cmp,
                               Combiner&& comb)
  { 

    for ( ; (first1 != last1) && (first2 != last2) ; ) 
    {
         if      (cmp(*first1, *first2)) { first1++; }      
         else if (cmp(*first2, *first1)) { first2++; }        
         else                            { *out_it++ = comb(*first1++, *first2++); }    
    } 
    return out_it; 
  }
  
  
  
  template <typename Iterator1, typename Iterator2, typename OutIterator, typename Comparator, 
            typename Combiner
            >
  OutIterator join(Iterator1 first1, Iterator1 last1, 
                               Iterator2 first2, Iterator2 last2, 
                               OutIterator out_it, 
                               Comparator cmp,
                               Combiner&& comb)
  { 

    for (auto fixed_f2=first2 ; (first1 != last1) && (fixed_f2 != last2) ; ) 
    {
         if      ((first2 == last2) || cmp(*first1, *first2)) { first1++; first2 = fixed_f2;}      
         else if (cmp(*first2, *first1)) { first2++; fixed_f2 = first2; }        
         else                            { *out_it++ = comb(*first1, *first2++); }   
         
         
         //if ((first2 != fixed_f2) && (first2 == last2)) { first1++; first2 = fixed_f2; }
    } 
    return out_it; 
  }
}





struct S 
{
   explicit S(int id_, int v_) : id{id_}, v{v_} {} 
   int id; 
   int v; 
}; 


struct M 
{
   explicit M(int id_, int v_) : id{id_}, v{v_} {} 
   int id; 
   int v; 
}; 

std::ostream& operator<<(std::ostream& os, S s) { return os << "S{" << s.id << ", " << s.v << "}"; }
std::ostream& operator<<(std::ostream& os, M m) { return os << "M{" << m.id << ", " << m.v << "}"; }



int main() 
{
    
    std::vector<M> A{M{2,2},M{2,3}, M{5,5}}; 
    std::vector<S> B{S{2,1},S{2,4}}; 
    std::vector<std::string> C;

    struct cmp
    {
        bool operator()(S s, M m) { return s.id < m.id; }
        bool operator()(M m, S s) { return m.id < s.id; }
    }; 
  
    auto comb = [](M m, S s) 
    { 
        std::ostringstream os; 
        os <<  m << "-" << s; 
        return os.str(); 
    };

    my::join(std::cbegin(A), std::cend(A), 
                         std::cbegin(B), std::cend(B), 
                         std::back_inserter(C), 
                         cmp{}, comb); 
  
    std::cout << "C: ";  for (auto c: C) { std::cout << c << ' '; } std::cout << '\n'; 
    // This prints: C: int(2)-S(2) int(5)-S(5)

}