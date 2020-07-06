---
layout: post
title: "Two applications for the comma operator"
categories: C++
keywords: programming; C++
---

Here is the thing. The comma operator is one of those little features in C and C++ that you probably found mostly useless, but you cannot remove it because it has one or two applications. One application is: 

```cpp
for (std::size_t i{2}; (i < 5);  std::cout << i, i++) {} 
``` 

This prints 234. There is, however, interesting use of the comma operator in fold expressions in C++17.  I will use this feature to write a ``for_each_n`` function. Given a function `F` and some arguments, execute `F` with its arguments `n` times.  While implementing this, I will be using the comma operator (on two different occasions), fold expressions, and index sequences.  My main reason for this post is to show an application for all these tools that I've just mentioned. 

I want to use my ``for_each_n`` as follows: 

```cpp
    auto f = []() { std::cout << "f"; };
    for_each_n<4>(f); 
    std::cout << '\n'; 
    
    auto g = [](char c) { std::cout << "g" << c; };
    for_each_n<5>(g, 'a');
    std::cout << '\n';
    
    auto h = [](char c, int n) { std::cout << c << n << ' '; };
    for_each_n<2>(h, 'd', 4);
    std::cout << '\n'; 
```

This should outputs: 

```cpp
ffff
gagagagaga
d4 d4
```

Let's start with the interface as usual. 

```cpp
template <std::size_t n, typename F, typename... Args> 
void for_each_n(F f, Args&&... args) 
{
    detail::for_each_n<std::make_index_sequence<n>, F, Args...>{}(f, std::forward<Args>(args)...); 
}
```

We are passing a function `f` of type `F` with its arguments of types `Args...`.     We will call ``detail::for_each_n``. But we will create an index sequence starting from `0` to `n-1` using `std::make_index_sequence<n>`.  ``detail::for_each_n`` is a struct here which I will call its `operator()`. I prefer using structs instead of functions when dealing with templates.  

```cpp
namespace detail 
{
template <typename IndexSeq, typename F, typename... ArgsT>
struct for_each_n; 
}
```

Next is the implementation of ``detail::for_each_n::operator()`` which is the most important thing in this post. 

```cpp
template <std::size_t... n, typename F, typename... Args> 
struct for_each_n<std::index_sequence<n...>, F, Args...>
{
   void operator()(F gf, Args&&... args) 
   {
	   // fill the blanks. 
   } 
}; 
```

What should we include in blanks knowing that I want to include fold expression? You can't unpack a fold expressions without using the indices `n...`. So we have to do a trick. Aha! let's use the comma operator. 

```cpp
   void operator()(F gf, Args&&... args) 
   {
	  ((gf(std::forward<Args>(args)...)
   } 
```

For the sake of simplicity, let's ignore the args for a moment as follows: 

```cpp
   void operator()(F gf, Args&&... args) 
   {
	  ((gf(std::forward<Args>(args)...)
   } 
```

There are two comma operators here. First, the comma in `(gf(), n)` is a comma operator. The `n` has no effect at all. We will end up calling `gf()`. We need to include that useless statement (i.e., `n`) - but this is a hack to use the fold expression.  There is also another comma operator used here. The fold expression will translate ` ((gf(), n),...)` to: 

```cpp
(gf(),1), (gf(),2), ..., (gf(),n-1).
```

... where that `,` between `(gf(),i)` and `(gf(),i+1)` is a comma operator. 

Ok great! we are not done yet. Writing ``operator()`` in that way will raise warnings. We need to avoid them. One way to do this is: 

```cpp
   void operator()(F gf, Args&&... args) 
   {
        auto f = [&](std::size_t) { gf(std::forward<Args>(args)...); };
        (f(n),...); 
   } 
```

Here the fold expression means `f(1),f(2), ..., f(n-1)`.  You can also use a template function such as `f<1>(gf, args...)`, it is not that important here.  

Another thing you can do is to suppress the warnings. In gcc, I can do something like this: 

```cpp
   void operator()(F gf, Args&&... args) 
   {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wunused-value"
        ((gf(std::forward<Args>(args)...), n),...);
        #pragma GCC diagnostic pop
   }
```

**Are there other possible solutions?** of course yes.  We could have used recursion (template-wise).  ``detail::for_each_n<n, ....>`` will call ``detail::for_each_n<n-1, ....>`` etc .. until `detail::for_each_n<0, ...>`.  We have this technique before, so where is the fun? 

We could have also just used a for-loop: 
```cpp
for (std::size_t i{0}; i < n; i++) 
{
    for (std::size_t i{0}; i < n; i++) 
    {
      gf(std::forward<Args>(args)...);   
    } 
}
```

Not everyone will want to use this solution though. The other solution we provided is based on ``loop unrolling``. It avoids doing the unnecessary `i<n` and `i++` operations. It can be faster.  The compiler generally unrolls such for-loops for you in optimization mode if it thinks it will lead to better performance.  

So, should we use this ``for_each_n``? I don't think we should (unless you think that you are smarter than the compiler).  In summary, I think we just ended up with yet another useless application of the comma operator that will make it less prone to depreciation (that's not including the comma operator in fold expressions which is quite powerful).  