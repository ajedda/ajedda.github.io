---
layout: post
title: Random thoughts about std::intersection_set, and its relationship to matching. 
categories: algorithms
keywords: algorithms
---



The STL library algorithms should be the basic blocks of other more sophisticated algorithms.  I will talk about ``std::intersection_set``.  My argument is that it can be extended further, and it can also serve as a basis of some basic matching and joining (as in SQL join) algorithms. I will describe how it works. I will provide an implementation. Then, I will provide some extensions.  

As understood from its name, ``std::intersection_set`` finds the intersection of two ranges. It assumes that the two ranges are sorted. And because we are dealing with sets (as in set theory), we should assume that the sets have distinct elements. Let's start with an example of how to use it. 

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

If the ranges are ordered differently (let's say by descending order), then we can simply change the ``cmp`` function such as: 

```cpp
auto cmp = [](const auto& l, const auto& r) { return l < r; };
```

In ``std::intersection``, ``cmp`` is used in two different ways; ``cmp(l,r)`` and ``cmp(r,l)``. That declaration above worked only because ``r`` and ``l`` have the same type.  Let's show this by working on an example where the ranges are of different types. 

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

This will not work (even if you replace ``cmp`` to:  ``auto cmp = [](const auto& l, const auto& r) { return l.a < r; };``. What you should do instead is to replace ``cmp`` with a struct as follows:

```cpp
struct cmp
{
  bool operator()(S s, int a) { return s.a < a; }
  bool operator()(int a, S s) { return a < s.a; } 
};
```

I will show one implementation of ``std::set_intersection`` to understand what is happening (I will be using ``operator<`` instead of ``cmp`` here). 

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

The other thing that I want you to look at is the assignment at the else-statement (i.e., `*out_it++ = *first1++; first2++;`).  I think that this is a weakness in ``std::intersection_set``. Why shouldn't we take the ``*first2`` instead? Why don't the standard let us decide which one to use? Maybe we will want to combine ``*first1`` and ``*first2``?  This design is not wrong, especially given the first two assumptions about sets: 1) unique elements, and 2) same type elements in both sets.  But I think passing a *combiner* function that combines the results of ``*first1`` and ``*first2`` may be useful in some scenarios; something like this: 

```cpp
namespace my 
{
   template <typename Iterator1, typename Iterator2, typename OutIterator, typename Comparator, 
            typename Combiner >
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

This will be another specialization of ``std::set_intersection``,  and you will use it like this: 

```cpp
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

int main() 
{
    
    std::vector<M> A{M{1,1},M{2,2},M{5,5},M{7,7}}; 
    std::vector<S> B{S{2,1},S{4,3},S{5,5}}; 
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

    my::set_intersection(std::cbegin(A), std::cend(A), 
                         std::cbegin(B), std::cend(B), 
                         std::back_inserter(C), 
                         cmp{}, comb); 
  
    std::cout << "C: ";  for (auto c: C) { std::cout << c << ' '; } std::cout << '\n'; 
    // This will print: C: M{2, 2}-S{2, 1} M{5, 5}-S{5, 5} 

}
```

I had two sets in the previous example; `A` and `B`. The elements of these sets are of types `M` and `S`. All elements have identifiers. The intersection is based on that identifier. This is not a *set intersection* in the sense of set theory, but more of a *one-to-one matching* in its simplest form. The combiner returns a string, which is then stored in the result vector ``C``.  A special case in this *one-to-one matching* is when the sets have different elements that have the same identifier.  For example: 

```cpp
std::vector<M> A{M{1,1},M{2,2},M{2,3},M{5,5},M{7,7}}; 
std::vector<S> B{S{2,1},S{2,4},S{4,3},S{5,5}}; 
```

Here, I added `M{2,3}` to the first set `A`, and `S{2,4}` to the second set `B`. According to our implementation, `M{2,3}` is not the same as the existing element `M{2,2}`. The same applies to `S{2,1}` and `S{2,4}`.  The result of `C` in this case is: `C: M{2, 2}-S{2, 1} M{2, 3}-S{2, 4} M{5, 5}-S{5, 5}`. Note that `M{2,3}` is matched with `S{2,4}`.  Therefore, the result of ``std::set_intersection`` when you have redundant elements changes according to the elements order (and hence according to your sort algorithm). 

A minor change to the algorithm can lead to a *one-to-many matching* algorithm (again, in a very simple form). The for loop in the algorithm will be changed to: 

```cpp
    for ( ; (first1 != last1) && (first2 != last2) ; ) 
    {
         if      (cmp(*first1, *first2)) { first1++; }      
         else if (cmp(*first2, *first1)) { first2++; }        
         else                            { *out_it++ = comb(*first1, *first2++); }     // don't increment first1 now. 
    } 
```

The result of our previous example would be: `C: M{2, 2}-S{2, 1} M{2, 2}-S{2, 4} M{5, 5}-S{5, 5}`. 

What about *many-to-many matching* (or, *inner join*)? That is, if I had `A` set to `{M{2,2}, M{2,3}}` and `{S{2,1},S{2,4}}`. I want the result to be: ``(M{2-2}-S(2,1)), (M{2-2}, S{2,4}), (M{2,3}, S{2,1}), (M{2,3},S{2,4})``. This also requires nothing but a small modification. The code is a better explainer here. 


```cpp
  template <typename Iterator1, typename Iterator2, typename OutIterator, typename Comparator, 
            typename Combiner>
  OutIterator join(Iterator1 first1, Iterator1 last1, 
                               Iterator2 first2, Iterator2 last2, 
                               OutIterator out_it, 
                               Comparator cmp,
                               Combiner&& comb)
  { 
    // Note: we introduce a new variable fixed_f2, 
    // we replaced (first2 != last2) with (fixed_f2!=last2). 
    for (auto fixed_f2=first2 ; (first1 != last1) && (fixed_f2 != last2) ; ) 
    {
         if      (cmp(*first1, *first2)) { first1++; first2 = fixed_f2;}      
         else if (cmp(*first2, *first1)) { first2++; fixed_f2 = first2; }        
         else                            { *out_it++ = comb(*first1, *first2++); }   
         
         
         if ((first2 != fixed_f2) && (first2 == last2)) 
         { first1++; first2 = fixed_f2; } 
    } 
    return out_it; 
  }

```

I could have also written the for-loop like this:

```cpp
    for (auto fixed_f2=first2 ; (first1 != last1) && (fixed_f2 != last2) ; ) 
    {
         if      ((first2 == last2) || cmp(*first1, *first2)) { first1++; first2 = fixed_f2;}      
         else if (cmp(*first2, *first1)) { first2++; fixed_f2 = first2; }        
         else                            { *out_it++ = comb(*first1, *first2++); }   
         
         
         //if ((first2 != fixed_f2) && (first2 == last2)) { first1++; first2 = fixed_f2; }
    } 
```


We can do all sort of joins building on top of these algorithms. This was made possible by adding the *combiner* (and the correct comparison operators of course). However, note that this was only possible because we assumed the sets were sorted. 

It seems to me that work on sets in STL algorithms was just to fill some blanks, and it stopped afterward. I think these algorithms can be extended. I included some ideas in this post, but I am pretty sure there are many other related algorithms we can add. *Join* algorithms can be useful specially with the increased popularity of data science algorithms. 



