// Recursive destructor in a singleton. 

#include <iostream> 

struct R 
{
    R() 
    {
      std::cout << "R::R() \n";   
    } 
    
    ~R() 
    {
        std::cout << "R::~R() \n"; 
        if (_instance) 
		{
			delete _instance;
			_instance = nullptr; // this is useless.  
		} 
    } 
    
    static R& instance()
    {
        if (!_instance) 
        {
          _instance = new R();   
        } 
        return *_instance; 
    } 
    
    static R* _instance; 
    
}; 

R* R::_instance{nullptr}; 

int main()
{
    R r;    
    auto& r_instance = R::instance();  
    (void) r_instance; 
}