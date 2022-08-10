---
layout: post
title: "What strtok teaches about std::string"
categories: algorithms
keywords: algorithms; 
---

Let's assume you have this function: 

```cpp
bool match(const std::string& pattern, const std::string& str)
```

You want to match a pattern to a string. Inside this function, you want to use a tokenizer. You decided to go old-fashion and use `strtok`. Then, you found that `strtok` signature is as follow. 

```cpp
char * strtok ( char * str, const char * delimiters )
```

Yes, the input string is passed as `char*` instead of `const char*`. And yes indeed, `strtok` changes the value of the input string. Here is an example: 

```cpp
void use_strtok(char* s) 
{
    char* t_ptr = strtok(s, ","); 
    for (; t_ptr; t_ptr = strtok(NULL, ",") ) 
    {
        std::cout << __FUNCTION__ << ":" << t_ptr << '\n'; 
    }
} 

void strtok_example() 
{
    char str[] = "A,B,C"; 
    std::cout << "str before: " << str << '\n';
    
    use_strtok(str);     
    std::cout << "str after: " << str << '\n';
} 

```

The output of this example is: 
```cpp
str before: A,B,C
use_strtok:A
use_strtok:B
use_strtok:C
str after: A
```

I can't see a case where we want to change the input. So let's implement a dummy `match` function for our testing purposes as follow. 

```cpp
bool match(const std::string& pattern, const std::string& str)
{ 
  // some code here .. 
  
  use_strtok(pattern); 

  // then more code ..
} 
```

Obviously, `use_strtok(pattern)` won't compile since pattern is a `std::string`. Even, `pattern.c_str()` won't work since `std::string::c_str` returns a `const char*`. What should we do then? One solution I found was something as follows.

```cpp
namespace ver1 {

bool match(const std::string& pattern, const std::string& str) 
{ 
    constexpr std::size_t buffer_size = 256;

    static char buffer[buffer_size+1];                      //P1
    strncpy(buffer, pattern.c_str(), buffer_size);          //P2
    buffer[buffer_size] = '\0';                             //P3
    use_strtok(buffer); 
    return true;
} 

}
```

and to test it I use: 

```cpp
int main ()
{ 
    std::string pattern = "A,B,C";
    ver1::match(pattern, ""); 
    std::cout << "pattern after: " << pattern << '\n'; 
}
```

The output is as expected, but let's analyze the behaviour a bit. There are a few reasons for what we see in function `match` above. 

*P1:* we need to copy the `pattern` string into a `char*`. Thus, we create a buffer. The function designer, however, decided to make the buffer static (or a class member variable). The function designer set the static buffer size with constant value that is big enough to store the pattern string.  It is 256 in this example, but it was much larger in the production code I saw (because pattern was much larger). Of course, the larger the size, the slower the function is. The reason for this design decision is not clear to me. It seems it was there to achieve better performance by avoiding memory allocation. It did not. Production code was spending long time in this match function. 

*P2:* we need to copy pattern to buffer. Note that  `strncpy` was used with `n` set to `buffer_size` instead of `pattern.size()`.  

*P3:* we always terminate the buffer with `null` so that `strtok` works correctly. 

Did the designer make good decisions here? Let's do the opposite and optimize from there: 

```cpp
namespace ver2
{

bool match(const std::string& pattern, const std::string& str) 
{ 
    const auto buffer_size = pattern.size()+1;  // the 1 is for the null not counted in size(). 
    char* buffer = new char[buffer_size];
    //auto buffer = std::make_unique<char[]>(buffer_size); // that version of gcc didn't support that yet. 
    strcpy(buffer, pattern.c_str()); 
    use_strtok(buffer); 
    delete[] buffer;
    return true;
}   

}
```

What happen if `pattern` is small (e.g., 1 or 2 chars etc ..)? We want to avoid the dynamic allocation in this case.  This is what we call *Small String Optimization* (SSO). The `std::string` implementation should have this done. So why not using `std::string`? Such as: 

```cpp
namespace ver3
{

bool match(const std::string& pattern, const std::string& str) 
{
  std::string pattern_cpy{pattern};     // using copy-constructor
  use_strtok(const_cast<char*>(pattern_cpy.c_str()));   
  return true;
} 

}
```

With this version, we have dynamic allocation with smal string optimization. That should be a very efficient implementation. it may look strange sine we are using `const_cast`, but 1) we are using `pattern_cpy` only once, and 2) we know what we are doing - **or do we?**

While the output of `ver1::match` and `ver2::match` is: 

```
use_strtok:A
use_strtok:B
use_strtok:C
pattern after: A,B,C
```

 ... we found that the output of `ver3::match` is:
```  
use_strtok:A
use_strtok:B
use_strtok:C
pattern after: ABC
``` 


Did you notice what is  `pattern` is  after `ver3::match`?! It is wrong!  

Well, this happens in gcc 4.8.3 (the compiler used in prod at that time). Newer compilers don't have this problem. Why? 

It is the reference counting in `std::string` implemenation in gcc4.8.3. When we did `pattern_cpy{pattern}`, the std::string implementation assumed they are the same. The `const_cast` - which is a hack after all - is not a sufficient reason for the implementation to assume that `pattern_cpy` is different from `pattern`. And because `strtok` changes `pattern_cpy`, it also changed `pattern`!  Reference counting in gcc4.8.3 was there for performance reasons. It is removed from newer implemenations and we started using Small String Optimzation, for performance reasons as well. We had to use gcc4.8.3. So what should we do? one way is to force the copy using a different constructor. 


```cpp

namespace ver4
{

bool match(const std::string& pattern, const std::string& str) 
{
  std::string pattern_cpy{pattern.begin(), pattern.end()};  // note: we are using a different constructor here. 
  use_strtok(const_cast<char*>(pattern_cpy.c_str()));   
  return true;
} 

}
``` 

Now this works. It led to about 75%  performance in execution time. We are talking minutes here. From 10-13 minutes to 2-3 minutes. This is even if small string optimization is not implemented in our environment/compiler.  Other than its performance, I liked this implemenation because of its portability. 


Anyway, *why would any C++ developer use strtok? Seriously, why?* 