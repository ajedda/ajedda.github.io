---
layout: post
title: std::intersection_set is not general enough
categories: algorithms
keywords: algorithms
---



I think that the STL library algorithms should be as generic as possible. In other words, these algorithms should be the basic blocks of other more sophisticated algorithms. I will talk about std::intersection_set.  My argument is that it is not general enough. To make it more general, however, we may end up with an interface that is hard to use.  

As understood from its name, std::intersection_set finds the intersection of two ranges. It assumes that the two ranges are sorted. Let's start with an example of how to use std::intersection_set. 

```cpp
std::vector<int> A{1,2,5,7}; 
std::vector<int> B{2,4,5}; 
std::vector<int> C; 

auto cmp = [](const auto& l, const auto& r) { return l < r; }; 
std::set_intersection(std::cbegin(A), std::cend(A), 
                      std::cbegin(B), std::cend(B), 
                      std::back_inserter(C), 
                      cmp);  
  
std::cout << "C: ";  for (auto c: C) { std::cout << c << ' '; } std::cout << '\n'; 

// This will print:  C: 2 5. 
```
If the ranges are ordered differently (let's say descneding order), then we can simply change the cmp function such as: 
```cpp
auto cmp = [](const auto& l, const auto& r) { return l < r; };
```

In reality, *cmp* is used in two different ways; *cmp(l,r)* and *cmp(r,l)*. That declaration above worked only because *r* and *l* have the same type.  Let's show this by working on an example where the ranges are of different types. 

```cpp
struct S 
{
   explicit S(int a_) : a{a_} {} 
   int a; 
}; 
```

```cpp
std::vector<int> A{1,2,5,7}; 
std::vector<S>   B{S{2},S{4},S{5}}; 
std::vector<int> C;
  
auto cmp = [](const auto& l, const auto& r) { return l < r.a; };
std::set_intersection(std::cbegin(A), std::cend(A), 
                      std::cbegin(B), std::cend(B), 
                      std::back_inserter(C), 
                      cmp); 
  
std::cout << "C: ";  for (auto c: C) { std::cout << c << ' '; } std::cout << '\n'; 
```
This will not work (even if you replace cmp to:  `auto cmp = [](const auto& l, const auto& r) { return l.a < r; };`What you should do instead is to replace *cmp* with a struct as follows: 
```cpp
struct cmp
{
  bool operator()(S s, int a) { return s.a < a; }
  bool operator()(int a, S s) { return a < s.a; } 
};
```

I will show you how std::set_intersection is implemented (one way of doing that of course) to understand what is happening (I will be using operator< instead of cmp here). 

```cpp
namespace my 
{ 
  template <typename Iterator1, typename Iterator2, typename OutIterator> 
  OutIterator set_intersection(Iterator1 first1, Iterator1 last1, 
                               Iterator2 first2, Iterator2 last2, 
                               OutIterator out_it)
  { 
    for ( ; (first1 != last1) && (first2 != last2) ; ) 
    {
         if      (*first1 < *first2) { first1++; }      
         else if (*first2 < *first1) { first2++; }       
         else                        { *out_it++ = *first1++; first2++; }    
    } 
    return out_it; 
  }
                               
} 
``` 

Note that there are `*first1 < *first2`, and then `*first2 < *first1`.  We are dealing with two different operator< functions here.  (*Side note:* We intentionally don't use `*first1 == *first2`. Why? if the data comes from one type, then we will need to define operator< only, instead of operator< and the equality operator.)   

The other thing that I want you to look at is the assignment at the else-statement (i.e., `*out_it++ = *first1++; first2++;`).  I think that this is a weakness in *std::intersection_set*. Why shouldn't we take the *first2 instead? Why don't the standard let us decide which one to use? Maybe we will want to combine *first1 and *first2.   Something like this: 

```cpp
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
}
```

.. and you will use it like this: 

```cpp
std::vector<int> A{1,2,2,5,7}; 
std::vector<S>   B{S{2},S{4},S{5}}; 
std::vector<std::string> C;

struct cmp
{
  bool operator()(S s, int a) { return s.a < a; }
  bool operator()(int a, S s) { return a < s.a; } 
}; 
  
auto comb = [](int a, S s) 
{ 
    std::ostringstream os; 
    os <<  "int(" << a << ")-S(" << s.a << ")"; 
    return os.str(); 
};

my::set_intersection(std::cbegin(A), std::cend(A), 
                     std::cbegin(B), std::cend(B), 
                     std::back_inserter(C), 
                     cmp{}, comb); 
  
std::cout << "C: ";  for (auto c: C) { std::cout << c << ' '; } std::cout << '\n'; 
// This prints: C: int(2)-S(2) int(5)-S(5)
```

I see some applications for this combinator as in matching. Like you have two sets of S's and M's each with a unique identifier. We would like to match the elements with the same ID in the output. We may also want to run some function on these output pairs.  I am not sure how serious these applications are, so I will just skip them for now. 

(Test changes). 
