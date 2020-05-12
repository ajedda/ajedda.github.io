---
layout: post
title: A Framework for Backtracking Algorithms
categories: algorithms
keywords: algorithms
comments: true
---


![Just some random tree](https://cdn-images-1.medium.com/fit/t/1600/480/0*bo2IpxNTU1Wrx6zv.jpg)

*Backtracking* is a fundamental approach to design algorithms.  If you want to solve a combinatorial optimization problem that is NP-Hard, your first approach should probably always be backtracking for an exact solution, and greedy for a quick one. A modification to the backtracking solution may lead to a _dynamic programming_ approach, a _heuristic algorithm_,  an _approximation algorithm_.  

This blog post is about Backtracking. We will go through a few examples  (with code in python). We will introduce a general approach to writing a backtracking-based algorithm. Then, we will discuss the techniques to improve the execution time of backtracking-based algorithms.

**Toy example:** We start with a toy example. Let's assume that we want to generate all binary numbers from $n$ bits. (that is, 0000, 0001, $\dots$, 1111 for $n=4$).  The first algorithmic approach that should come to our mind is *backtracking*.  A solution written with Python can look like that: 

```python 
def all_n_binary(n):
  def _all_n_binary(lst):
    # Our helper is an inner function to hide it. 
    if  len(lst) == n:
      print(lst)
      return

    for x in [0,1]:
      _all_n_binary(lst + [x])
  _all_n_binary([])

# You call the function as follows: 
all_n_binary(4) 
```   

Let's re-write the above function as a generator:

```python 
def all_n_binary(n):
  def _all_n_binary(lst):
    if  len(lst) == n:
      yield lst
      return

    for x in [0,1]:
      yield from _all_n_binary(lst + [x])
  _all_n_binary([])
  
# We call it as follows: 
for r in all_n_binary(4): 
    print (r)
```

Can you visualize the search tree? here is how it looks like: 
{:refdef: style="text-align: center;"}
![Search tree for all 3-binary](/assets/images/search_tree_1.jpg)
{:refdef}

What if you want to print all n-decimals? That is, for n=2, we should output 00, 01, ..., 99.  In this case, we will need to change one line only. 

```python
def all_n_decimal(n):
  def _all_n_decimal(lst):
    if  len(lst) == n:
      yield lst
      return

    for x in range(0, 10):
      yield from _all_n_decimal(lst + [x])
  _all_n_decimal([])
```

I want to add more constraints (and hence, more fun) to my problem. I want to output all n-binary without the sequence 11.  There are more than one solution to this problem. One method is to check before yielding whether the sequence has a 11 within it. The code is as follows: 

```python 
def _11_exists(lst): 
  if len(lst) < 2: return False 
  return (1,1) in zip(lst[:,-1], lst[1:])
  
def no_11_binary_v1(n): 
  def _no_11_binary(lst): 
    
    if len(lst) == n: 
      if not _11_exists(lst): yield lst 
      return 
      
    for x in [0, 1]: 
      yield from _no_11_binary(lst + [x])
    
  return _no_11_binary(n)
```

The search tree of the previous code is given in the figure below. Note that there are some branches that we did not have to examine until the end (i.e., until the branch had n nodes). For example, we did not have to examine 110 or 111. We could have stopped at 11. The code no_11_binary_v2 solves this problem. 

{:refdef: style="text-align: center;"}
![Never yielded solutions](/assets/images/search_tree_2.jpg)
{:refdef} 

```python 
def no_11_binary_v2(n): 
  def _no_11_binary(lst): 
    
    if len(lst) == n: 
      yield lst 
      return 
    
    if len(lst) == 0 or lst[-1] == 0: 
        choices = [0, 1]
    else :
        choices = [0]
    for x in choices: 
      yield from _no_11_binary(lst + [x])
    
  return _no_11_binary(n)
```

We are able to reduce the size of the search tree by specifying a better choice of nodes to visit. 

{:refdef: style="text-align: center;"}
![Better choices, smaller tree](/assets/images/search_tree_3.jpg)
{: refdef}

We can further refactor that code. We will remove that lonely return statement and includes in our choices list. 

```python 
def no_11_binary_v3(n): 
  def _no_11_binary(lst): 
    
    if len(lst) == n: 
      yield lst 
   
   
    if len(lst) == n: 
        choices = []
    if len(lst) == 0 or lst[-1] == 0: 
        choices = [0, 1]
    else :
        choices = [0]
        
    for x in choices: 
      yield from _no_11_binary(lst + [x])
    
  return _no_11_binary(n)
```

There are two important parts in the code above. The ```if len(lst) == n```, and ```choices```. The if-statement is to decide whether we reached a solution. The choices controls which paths to follow in the search tree. This allows us to create a generalized framework for recursive backtracking. 

```python
def no_11_binary_v3(n): 
  def is_feasible_solution(lst): 
    return len(lst) == n
  
  def choices(lst): 
    if len(lst) == n: return [] 
    if len(lst) == 0: return [0, 1]
    if lst[-1] == 0: return [0, 1]
    return [0]
    
  def _no_11_binary(lst): 
    
    if is_feasible_solution(lst):
      yield lst 
   
    for x in choices(lst): 
      yield from _no_11_binary(lst + [x])
    
  return _no_11_binary(n)
```

When solving a problem using backtracking you should follow these three steps: 
1. Follow the general backtracking algorithm (or framework) above. 
2. Implement choices()
3. Implement is_feasible_solution()

As you have noticed, the smarter your implementations for the functions the smaller is your search tree is and faster your algorithm is. 

.. and this is how to get a general backtracking algorithm. 