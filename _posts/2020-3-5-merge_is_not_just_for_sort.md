---
layout: post
title: std::merge is not just for sorting
categories: algorithms
keywords: algorithms
---


![enter image description here](https://miro.medium.com/max/4400/1*ONrJdGmOClFIa-Jdlr5UjA.jpeg)

You have probably learned about **MergeSort** in your first algorithms/data structure course. It is the first go-to example when explaining recursion, divide-and-conquer algorithms, or achieving the sorting lower bound O(n logn). 

A crucial part of that algorithm is the *merge* operation and its linear time complexity.  We can write **MergeSort** as follows.
```cpp
template <typename Container, typename Iter> 
Container merge_sort(Iter first, Iter last) 
{ 
    if (std::distance(first, last) <= 1) 
    { 
        return Container{first, last};
    } 
    
    
    auto midpoint = std::next(first, std::distance(first, last) / 2);
    // Just a fancy C++ way of saying: 
    // midpoint = first + (last - first)/2; 
    auto c1 = merge_sort<Container>(first, midpoint); 
    auto c2 = merge_sort<Container>(midpoint, last); 
    
    // Too much copying, but that's not the point. 
    Container c; 
    std::merge(std::cbegin(c1), 
               std::cend(c1), 
               std::cbegin(c2), 
               std::cend(c2),
               std::back_inserter(c)); 
    return c;
} 
```



What is **merge()**? It is a linear algorithm. It takes two sorted ranges. It merges them into one bigger sorted range. It achieves this in O($n+m$) steps, where $n$ and $m$ are the sizes of the two ranges respectively. It achieves this linearity because the two containers are sorted.  

```cpp
template <typename Iter, typename OutIter> 
void merge(Iter f1, Iter l1, 
           Iter f2, Iter l2, 
           OutIter out)
{ 
  for ( ; (f1 != l1) && (f2 != l2); )
  {
    if (*f1 < *f2) { *out++ = *f1++; }
    else           { *out++ = *f2++; }
  } 
  // Only one of these can run. 
  for ( ; f1 != l1; *out++ = *f1++); 
  for ( ; f2 != l2; *out++ = *f2++); 
} 

```

Forget about this now, and let's deal with an interesting application. 

**Application:** we want to design a simulator for a trading engine. The engine receives messages from two sources (market data and signals).  These are processed one after the other. At the end of each day, we want to revise how our engine performed based on the same messages that we received that day. Fortunately, we store our messages with their arrival timestamps. We store the messages in two different data sources.  We want to *process* each message in the right order of arrival.  Here is a code of how to do this. 

```cpp
Simulator sim{}; 

// Already sorted containers.  
// std::vector<M> mkt_src{ .... }; 
// std::vector<S> sig_src{ .....}; 

// because we love iterators. 
auto mf = std::cbegin(mkt_src); 
auto me = std::cend(mkt_src);
auto sf = std::cbegin(sig_src);
auto se = std::cend(sig_src);

for ( ; (mf != me) && (sf != se); ) 
{ 
    // ts is the timestamp of the message. 
    if (ts(*mf) < ts(*sf)) { sim.process(*mf++); }
    else                   { sim.process(*sf++); }
} 

for ( ; mf != me; sim.process(*mf++) ); 
for ( ; sf != se; sim.process(*sf++) );
```

Do you see the similarities between std::merge and the code above? Well, yes. There is a similarity. The question now is how to use std::merge code? Why? Because:  

 1. We re-use existing code. 
 2. std::merge (and all the STL library functions) may be optimized by the compiler. 

The problems that we are facing are: 

 1. The use of *ts* (as in,  ts(*mf) < ts(*sf)). 
 2. The use of *sim.process*.  

The solution is  *Iterators* (or better, *operator overloading*).  

**Problem 1:** The first thing we want to remove is that *ts* in this if-statement: 
```cpp
if (ts(*mf) < ts(*sf))
```

Fortunately, the STL provides us with a version of std::merge that can use a function Op(*mf, *sf) instead of *mf<*sf.  You can also overload the operator<(const M&, const S&), but I don't recommend that.  The signature of that std::merge version is: 

```cpp
template <typename Iter1, typename Iter2, 
          typename OutIter, typename CompOp> 
void merge(Iter1 ff, Iter1 fe, Iter2 sf, Iter2 se, 
           OutIter out, CompOp&& op);  
```

.. and we can call it as follows. 
```cpp
std::vector<int> s{1,3,5}; 
std::vector<int> m{2,4,6};
std::vector<int> res{};

// Assuming that ts(int) is already defined.   
auto comp = [](const auto& rhs, const auto& lhs) 
            { return ts(rhs) < ts(lhs); }; 
            
std::merge(s.cbegin(), s.cend(), 
           m.cbegin(), m.cend(), 
           std::back_inserter(res), 
           comp);
// res is {1,2,3,4,5,6}.  
```

**Problem 2**: How to use sim.process() in std::merge?  Again, the solution is *operator overloading*.  We need to have an iterator (out) such that: 

```cpp 
S s;
*out = s; 
```
is equivalent to: 
```cpp
S s; 
sim.process(s); 
```

and that: *out++* or *++out* have no impact. 

The idea is not different than what we have in std::back_insert_iterator; the type of the object that we get from std::back_inserter.  But, if you want to create such an iterator, and use it with STL functions, then you should respect the STL specifications of iterators. Boost helps us here and provides us with an iterator class that does just this for us. It is called *Function Output Iterator*. 

```cpp
#include <boost/function_output_iterator.hpp>

struct S 
{ 
  S(int s_) : s{s_} {}
  int s; 
}; 

struct M 
{ 
  M(int m_) : m{m_} {} 
  int m; 
};

struct Simulator 
{ 
   void operator()(const S& s) const 
   { std::cout << s.s; } 
   void operator()(const M& m) const 
   { std::cout << m.m; }
}; 

Simulator sim; 
auto f_iter = boost::make_function_output_iterator(sim); 
*f_iter++ = S{1}; // this will print 1
```

**Ok, Finally -  we  can put everything together.** 
```cpp
std::merge(std::cbegin(mkt_src), 
           std::cend(mkt_src), 
           std::cbegin(sig_src), 
           std::cend(sig_src), 
           f_iter, 
           comp);
```

I appreciate the STL library and C++ more when finding yet another application to concepts such as operator overloading, or a function such as std::merge. To be fair, *merge* is one of these fundamental algorithms. Using it as a block of code to build more complex algorithms and write clearer code - now that's really cool!  I may sound over-excited about something that can be seen as trivial to many. I am sure, however, that many developers do not see the STL library algorithms in this way. One thing to know, If you like this, then the STL algorithms were mainly designed with way of thinking in mind. There are many other examples. I hope I can write more about them. I just love it!

    

