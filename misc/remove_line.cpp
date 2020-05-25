#include <iostream> 
#include <fstream>
#include <sstream> 


int main() 
{
  std::stringstream is{"1 2 3 4 5 6 7"}; 
  std::stringstream os{}; 
  std::size_t which_line{3}; 
  
  for (std::size_t i{1}; ; i++)
  { 
   for ( ; (is) && (is.peek() == ' ');   os.put(is.get()) ) {}   
   
   is.get(*os.rdbuf(), ' '); 
   if (!is) { break; }
   
   
   if (i == which_line-1) 
   {  
       for ( ; (is) && (is.peek() == ' ');   is.get() ) {}
       for ( ; (is) && (is.peek() != ' ');   is.get() ) {}
       is.get(*os.rdbuf()); 
	   break; 
   } 
  } 
  std::cout << os.str() << '\n'; 
} 