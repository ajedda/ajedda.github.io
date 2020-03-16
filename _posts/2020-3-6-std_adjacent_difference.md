---
layout: post
title: "We should have std::adjacent_pairs instead of std::adjacent_difference"
categories: C++
keywords: programming; C++
---


Yet another post about another STL function. This one does not belong to *algorithm*; it belongs to *numeric*. It is the std::adjacent_difference as you can see from the title. 

If you are given a range $\{x_1, x_2, \dots, x_n\}$, you should expect the $i^{th}$ result to be $x_{i} - x_{i-1}$. There are only $n-1$ possible results. What should we do when $i = 0$? STL decided that the first result should be set to $x_i$. Why? I don't know. It is just contrary to the idea of giving the caller more choices. 

I will give my own implementation of std::adjacent_difference. I will follow it with my vision of how it should be. The code is below: 

```cpp
namespace ver1
{
    template <typename InpIterator, typename OutIterator>
    OutIterator adjacent_difference(InpIterator first, InpIterator last, OutIterator out) 
    {
      // Rule of thumb: always check the input 
      // ranges if they are equal. 
      if (first == last) { return out; } 
      
      
      // Step 1: Set r[0] to f[0]. 
      // I am commenting this out, but std::adjacent_difference 
      // does not. 
      // *out++ = *first;
      
      // Step 2: r[i] = f[i] - f[i-1]
      for (  ; ;  ) 
      {
        auto prev_f = *first++; 
        if (first == last) { return out; }
        *out++ = *first - prev_f; 
      }
      return out; 
    } 
}
```


The code above sets the $r_0$ to $x_0$ (see Step 1), then set $r_i$ to $x_{i} - x_{i-1}$.  I don't like Step 1, and this is why I commented out. I suggest we call the function as follows: 

```cpp
std::vector<int> v{3,5,2,7,3,4,5,4};

// The STL version and how we call it. 
std::vector<int> std_res; 
std::adjacent_difference(std::begin(v), std::end(v), std::back_inserter(std_res));
    
    
// ver1::adjacent_difference and how we call it. 
std::vector<int> ver1_res;
if (!v.empty()) { ver1_res.push_back(v.front()); } 
ver1::adjacent_difference(std::begin(v), std::end(v), std::back_inserter(ver1_res));
```

Both versions will print: 
```cpp
3 2 -3 5 -4 1 1 -1
```

One version of std::adjacent_difference accepts a binary operation. Instead of $x_i - x_{i-1}$, we can execution a function op($x_i$, $x_{i-1}$). I like this, and it is much more meaningful in ver1::adjacent_difference. We can call it now ver1::adjacent_pairs instead. 

```cpp
namespace ver1
{
    template <typename InpIterator, 
              typename OutIterator, 
              typename BinaryOperation>
    OutIterator adjacent_pairs(InpIterator first, 
                               InpIterator last, 
                               OutIterator out, 
                               BinaryOperation op) 
    {
      if (first == last) { return out; } 

      for (  ; ;  ) 
      {
        auto prev_f = *first++; 
        if (first == last) { return out; }
        *out++ = op(*first, prev_f); 
      }
      return out; 
    } 
}
```
The worst part of that version above is that the user will always need to provide an output iterator even if it is not needed.  I'd rather combine the binary operator and output iterator together. This is not very STL-like programming (perhaps it is time to standardize a function output iterator). The last adjacent_pairs version I am suggesting is: 

```cpp
namespace ver2
{   
    template <typename InpIterator,  typename F>
    void adjacent_pairs(InpIterator first, InpIterator last,  F f) 
    {
      if (first == last) { return ; } 

      for (  ; ;  ) 
      {
        auto prev_f = *first++; 
        if (first == last) { return ; }
        f(*first, prev_f); 
      }
 
    } 
}
```

Note that I am not using an output iterator at all. I am asking function f to take care of that (in other words, I am giving the users more freedom). Function ver2::adjacent_pairs can be used as follows: 

```cpp
std::vector<int> ver2_res; 
if (!v.empty()) ver2_res.push_back(v.front());
auto f = [&ver2_res](auto x, auto px) { ver2_res.push_back(x-px); };
ver2::adjacent_pairs(std::begin(v), std::end(v), f);
```

**Let's have an application.** We are given a list of cities such as Paris, London, New York, Montreal. This list constitutes a trajectory of a passenger. Our job is to find the cheapest airfare tickets for this trajectory.  We will look for the cheapest ticket for each adjacent pair. We can use our new tool as follow. 

```cpp
auto cheapest_ticket = [](const auto& x, const auto& xp) 
{
   std::cout << "cheapest_ticket(" << xp << ", " << x << ")\n"; 
}; 
    
std::vector<std::string> cities{"Paris", "London", "New York", "Montreal"}; 
ver2::adjacent_pairs(std::cbegin(cities), std::cend(cities), cheapest_ticket);  
```

.. which will print: 

```cpp
cheapest_ticket(Paris, London)
cheapest_ticket(London, New York)
cheapest_ticket(New York, Montreal)
```

