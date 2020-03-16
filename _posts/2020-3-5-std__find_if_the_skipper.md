---
layout: post
title: std::find_if as a skipper
categories: algorithms
keywords: algorithms
---



One of the algorithms/functions that I find myself using a lot is std::find_if.  It is just a linear search in a range. It can be used however in other applications. One possible implementation of this function is as follows: 

```cpp
template <typename Iterator, typename Op> 
Iterator find_if(Iterator first, Iterator last, Op&& op) 
{
  for (; (first != last) && !op(*first); first++); 
  return first;
} 
```
The magic is in what this function returns; an iterator. Not a boolean, not a number, an iterator!  This function is just a loop that returns the iterator of the first element v in the range such that op(v) to true. If nothing is found (and in this case, *first* will equal to *last*), then last is returned.  

The user will need to test the returned iterator against *last* to check if it exists in our range.  Returning a boolean, instead of an iterator, is just an extra work that this function designer decided to leave to the caller. 

```cpp
// the classical application of std::find_if is something like this. 
auto it = std::find_if(std::cbegin(v), std::cend(v), 
      [] (auto e) { return e % 2; });
// This extra check may sound boring, but it just add some power to this std::find_if function (as we will see next..). 
if (it != std::cend(v)) 
{ 
  std::cout << "There is an odd number there\n"; 
}  
```

The std::find_if is just one simple for-loop that we often see when we just want to skip some elements in a given range.  For example, if we want to find the sum of all positive integers in a sorted std::vector<int>, we can do something like this. 

```cpp
int sum = 0; 
for (auto f = std::cbegin(d); f != std::cend(d); f++) 
{
    if (*f < 0) { continue; } 
    sum += *f;
} 
```

.. or something that looks like this. 

```cpp
// All this means is: skip all negative numbers.
auto f = std::find_if(std::cbegin(v), std::cend(v), [](const auto& v) { return !(v <= 0); }); 
auto sum = std::accumulate(f, std::cend(v), 0); 
```
We will work on some more fun problems next.
 
**The first problem:** we have a vector of numbers.  Let's assume it is sorted.  We want to find the minimum positive number that does not exist in this vector. 

```cpp
int min_positive_not_found_number(std::vector<int> const& vec)
{
  // The first step is to skip any negative number. 
  auto f = std::find_if(std::cbegin(vec), std::cend(vec), [](const auto& v) { return v > 0; }); 
  int sol{1}; 
  for ( ; fw != std::cend(vec); f++)
  {
    if (*f <= sol) { sol = *f + 1; }
    // Note 1: Because the vector is sorted, we don't want to get beyond this point.   
  } 
  return sol; 
} 
```

Because of Note 1, we can rewrite this code as follow: 
```cpp
int min_positive_non_found_number(std::vector<int> const& vec)
{
  // the first step is to skip any negative number. 
  auto f = std::find_if(std::cbegin(vec), std::cend(vec), [](const auto& v) { return v > 0; }); 
  int sol{1}; 
  for ( ; f != std::cend(vec) && (*f <= sol); f++)
  {
    sol = *f + 1; 
    // You can also put this statement in the loop, and turn it into a one-liner.  
    // Not a big deal, and totally unrelated to the topic of this blog :). 
  } 
  return sol; 
} 
```



**Problem 2:** I found std::find_if useful when dealing with strings. Let's assume we want to write a simple csv parser. Let's assume for the sake of clarity that each line in your file has only numbers that are separated by a comma. We want to convert each token in the CSV line to a number. In other words, we want to write a string *tokenizer*. 

```cpp
template <typename Iter, typename D, typename P> 
void tokenize(Iter first, Iter last, D d, P process) 
{ 
  for ( ; ; first++)  
  {
    // std::find is just a special case of std::find_if. This is equivalent to.
    // std::find_if(first, last, [d](const auto v) { return v == d; }); 
    // std::find here means iterate until d is found.  
    auto nf = std::find(first, last, d); 
    process(first, nf); 
    // This is the "skip" statement (i.e., let first jumps directly to nf). 
    first = nf;  
    if (first == last) { return; } 
  }
} 
``` 


We call the function *tokenize* with something similar to this: 

```cpp
  std::string s{"12,234,234"}; 
  auto range_printer = [](auto f, auto l) 
  {
     for ( ; f != l; f++) 
     { std::cout << *f; }
     std::cout << '\n'; 
  }; 
  tokenize(std::begin(s), std::end(s), ',', range_printer);  
```

and the output will be: 
```
12
234
234
```

Of course, we can generalize the _process_ function to any other function we want (e.g., convert the string to integers). 

**Problem 3:**. Let's assume we have a pattern matcher. If the pattern is "abc", we expect to match the exact pattern (i.e., anything other than "abc" is false).  A pattern can have the symbol '+', which indicates any character can be matched.  We can make it a little bit more fun and we say that abc*z will match any string that replaces the * char with any other string.  Here is the code of *match*. 

```cpp

bool match(std::string const& pattern, std::string const& word) 
{
  auto pf = std::cbegin(pattern); 
  auto pe = std::cend(pattern); 
  
  auto wf = std::cbegin(word); 
  auto we = std::cend(word); 
  
  for ( ; (pf != pe) && (wf != we) ;  ) 
  {
    if (*pf == *wf || *pf == '+') { pf ++; wf ++; } 
    else if (*pf == '*')
    {
        // We should look for what comes after '*' in the pattern. 
        // special case: that can be pe (i.e., the end). We cant de-reference 
        // that. In this case, anything in the word match, so just quit by setting wf equal to we. 
        wf =  (++pf == pe) ? we : std::find(wf, we, *pf); 
    }
    else { return false; } 
  } 
  
  // special case. "ab*" against "ab". 
  // Here, we will quit the loop because wf == we.
  if ( pf != pe && *pf == '*') { ++ pf; }
  
  return (pf == pe) && (wf == we); 
} 
```

**Problem 4**: You are asked to remove an element from a vector. Great. You look into the STL algorithm library and find a function called *std::remove*. It takes a range of iterators (first and last), and the value to be removed.  You run *std::remove*  on your vector (std::vector<int> vec{1,2,3,4}), The reprint the vector. What do you get? [1,3,4,4]. Fine. 2 is removed, but why do we have two 4's? 

You probably know the answer already. *std::remove* (and its sister *std::remove_if*) do not erase the element. It remove th element from the range, but return a new end of the input range. For example: 

```cpp
std::vector<int> vec{1,2,3,4}; 
print(std::cbegin(vec), std::cend(vec)); // 1,2,3,4
auto ne = std::remove(std::begin(vec), std::end(vec), 2); 
print(std::cbegin(vec), std::cend(vec)); // 1,3,4,4
print(std::cbegin(vec), ne); // 1,3,4
```

In general, you should call std::vector::erase after calling std::remove to erase the elements from the vector. We call this the remove-erase idiom (Side note: Why (until C++20) we don't have std::erase? because the implementation depends on the containers, and not on the iterators). So, based on that, how does std::remove works? 

Let's assume that our vector elements are {0,1,2,3,4,5} and we want to remove 2.  In other words, we want to consider for now the case where we remove only one element. First step is to find where 2 is located. It is located at vec[2]. What we want to do here is to copy vec[3,4,5] into the range vec[2,3,4].  If we had more than one 2, let's say our vector elements are {0,1,2,3,2,5}. In this case, we want to shift vec[3,5] (or, all elements that are not equal to 2).  Lastly, we want to return the new end iterator of the vector. 

```cpp
  template <typename FwdIterator, typename Value>
  FwdIterator remove(FwdIterator f, FwdIterator l, Value value)
  {
    // find first occurence of value.
    auto nf = std::find(f, l, value);
    if (nf == l) { return nf; }

  
    for (auto b_nf = nf; ; )
    {
      if (!(*nf == value)) { *b_nf++ = *nf;}
      // A trick to return b_nf and not define it outside the loop. 
      if (++nf == l) { return b_nf; }
    }
  }
```

We can write this function in three lines. We will use a not very common STL algorithm called std::copy_if. This algorithm copies all elements in a range to another range if these elements meet a given condition. 

```cpp
template <typename FwdIterator, typename Value> 
FwdIterator remove(FwdIterator f, FwdIterator l, Value value) 
{
   auto nf = std::find(f, l, value); 
   if (nf == l) { return l; } // for the case where value is not found. 
   return std::copy_if(nf+1, l, nf, [=](const auto& v) { return !(v == value); }); 
} 
```

Before I conclude this post, note that std::copy_if (and similiarly std::copy) returned an iterator. This returned iterator helped in std::remove. There are certainly more applications for this returned iterator. Nothing come to my mind right now. It does, however, gives the caller more power. 

**Conclusion:** This std::find_if (and its sister std::find) are just one of the coolest algorithms there.  Most of this power comes from returning an iterator. Although it seems like it is giving the caller more checks to worry about, it gave the caller more power. It is a genius idea from a genius algorithm designer. Alex Stepanov mastered this way of thinking when designed the STL library. His books, articles, and lectures have many similar and more powerful examples. This falls under what he termed as *Generic Programming*. I put him at the same level as Knuth. I advise every developer to read more about Alex's programming approach.  It is just genius! 

