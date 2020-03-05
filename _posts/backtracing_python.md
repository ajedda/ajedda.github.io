---
title: Backtracking and Variants
abstract: 'This is a shot abstract. '
categories: algorithms
keywords: algorithms
weblogName: New Weblog
postDate: 2020-03-05T01:01:45.0503792-05:00
---
# Backtracking and variants 

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
all_n_binary(4) 
```   

Now instead of all n-binary numbers, what about all n-decimal numbers. What about using any other base? All you have to do, besides using more meaningful names, is to change the for-loop: 
```python
		for x in [0,1]:
			_all_n_binary(lst + [x])
```
To: 
```python
		b = 10
		for x in range(0, b): 
			_all_n_decimal(lst + [x])
```

and if you want to hexadecimal. You should use [0, 1, $\dots$, 9, A, B, C, D, E, F] instead of range(0, 10). 

**Another example:** a similar problem is to enumerate all k-sized subsets of the n-sized set $S$. For example, if $S = \{1,2,3\}$ and $k=2$. We want to output:     $\{1,2\}, \{1,3\},$ and $\{2,3\}$.  The following code can be used to solve this problem.  
```python 
def all_k_subsets(n, k):
	def _all_k_subsets(lst):
		if  len(lst) == k: print(lst)
		# The base-case (but what about using k instead of n?)  
		if  len(lst) == n: 	return			

		for x in  range(1, n+1):
			_all_k_subssets(lst + [x])
	
	print ("printing all subsets ({},{})".format(n, k))
	return _all_k_subsets([])
```

This function also has minor differences from the previous two functions; the base-case if-statement, and the range(1, n+1). 

**The search tree:** (continue). 

**Some Definitions:** Every subsets generated in the previous example is called a *solution*. Every subset of size $k$ is called a *feasible solution*. For example, $\{1\}$ and $\{1,2,3\}$ in _all_k_subsets(2, 3)_ are solutions, but $\{1, 2\}$ is a feasible solution (because its size is equal to $k=2$).   

A solution $\{x_1, \dots, x_{l-1}\}$ can be extended with any of the elements in a set $\mathbf{C}(\{x_1, \dots, x_{l-1}\})$.  As an example, in the _all_n_binary_ problem for $n = 4$, the solution $\{1, 0\}$ can be extended with the *choices set* $\mathbf{C}(\{0, 1\})$. That is, it can be extended to 100 or 101. In the case of _all_n_decimal_ for $n=2$, the solution 3 can be extended to $30, 31, \dots, 39$.  This extension process is done in the for-loop in the algorithms above.  

**The general algorithm:** Based on all of that, here is a general backtracking algorithm: 
```python 
def backtrack():	
	def _backtrack(lst):
		if is_solution(lst): process(lst)	
		if base_case(lst): return
		for x in choices(lst):
			_backtrack(lst + [x])
		return _backtrack([])
``` 

**Why do we use backtracking?** we use backtracking to solve three types of problems: 
 1. Generate all solutions for a problem. 
 2. Generate any solution for a problem. 
 3. Find the solution with maximum score (or minimum cost). 

**More examples:** The examples above, despite their simplicity, can be extended to solve more complex optimization problems. For example, we want to generate all spanning trees in a graph, all cliques, all solutions for a boolean satisfiability problem, 0-1 Knapsack problem solution, or just to solve some puzzles.

For the knapsack problem, the items that we are allowed to put in the knapsack can be represented as $\{x_1, \dots, x_n\}$, and the content of the knapsack can be represented with an n-binary number.  Thus, find all binary numbers, check if the corresponding knapsack leads to a feasible solution, and then find the knapsack that maximizes the weight (or, score).  We can also represent the solution as a subset of $\{x_1, \dots, x_n\}$. 

**Quiz**: Can you generate all possible combinations of the sets $S_1, \dots, S_n$. For example, S1 = \{a, b\} and S2 = {1,2,3} leads to {a, 1}, {a, 2}, ... , {b,3}.  


**Q: How to calculate the time-complexity of backtracking algorithms?** It depends on the size of the search tree. This can be known in many problems (e.g., _all_n_binary_). It is not easy in many other problems. Why? because we usually use smart tricks to **prune** the tree from paths we know won't lead to any solution.  Experiments are our performance measuring tools in these cases. 

Pruning helps also in reducing the search tree, and hence speed up the algorithm.  We achieve pruning by carefully defining the choices set $\mathbf{C}$ (i.e., _choices_ in the code).  Here are some examples of some pruning tricks: 

 - Did we need to check solutions with a size greater than $k$   in _all_k_subsets(k, n)_? 
 - If I want to find all un-repeated sets (that is, {1,2}, and {2,1} are the same), do I really need to extends solution {2} with {1}? 
 - If I want to find all cliques in a graph, and my solution is $\{v_1, v_4, v_5\}$, which forms a clique, should I extend this solution with a node $v_6$ that is not neighbor to all of them? 
 - If I have a knapsack with $\{e_1, e_3, e_5\}$ and its cost is 12, which is greater than our constraint, should we extend this knapsack?  

There are some really smart pruning tricks out there that are based on complex mathematical properties. You can publish a paper if you find one. 

```python 
def all_ord_n_sets(k, n):
	def _all_ord_n_sets(lst): 
		if  len(lst) == k: 
			print(lst)
			return
		# example of pruning with the set choices. 	
		b = max(lst)+1  if lst else  0
		for x in  range(b, n):
			_all_ord_n_sets(lst + [x])
	_all_ord_n_sets([])
``` 

**Optimization problems:** From now on, we will focus on optimization problems only.  That is, we want to find the solution with the maximum score or minimum cost. The problem is nothing but a _search_. Given a set of numbers, find the minimum number in it. Unfortunately, solving this deterministically in the general case requires $\Omega(n)$ steps. If $S$ is huge, as it is the case in our backtracking problems, then we may spend years running our algorithm. 

A technique widely used to reduce the size of the search space, and hence the algorithm execution time, is called **bounding**.  We usually use the term *Branch-and-Bound* when dealing with bounding. However, the term is not well defined (at least in my opinion).  The idea is simple though. 

Let's say we are looking for the element with the maximum score. The maximum score we found so far was $s_m = 40$.  Let's say that the current solution is $s = \{v_2, v_4\}$. This can be extended with the elements $\{v_5, \dots. v_n\}$.   What if I actually can find an upper bound on the score that I will find if I follow this path in the search tree. That is, what if I can correctly count that the maximum score I can get from following this path is 30 (that is, it cannot exceed $s_m$). In this case, I should just ignore this path and follow some other path in the tree. This will prune the rooted  subtree at $\{v_2, v_4\}$.  This can be a lot of pruning. 

**How do we do bounding?** Bounding is not guessing. It is based on sound mathematical observations. Here is a nice example iMax-Clique problem.  Let's assume that the current solution is $S$. It can be extended with the nodes in $\mathbf{C}(S)$.  Observe that: 

> In the Max-Clique problem $S$ must be a clique too, otherwise no solution will be obtained from extending $S$.  This means that any subset of $S$ is also a clique. 

Also note that any extension of $S$ in the MaxClique must also be a clique.  That is, the maximum clique that  I will be able to obtain if I followed this path is less or equal to $|S|$ + $|\mathbf{C}(S)|$.  This is an _upper bound_.  In other words, you can simply prune this sub-tree if $s_m \ge |S|$ + $|\mathbf{C}(S)|$.

**Another example of bounding** in the MaxClique problem is based on _coloring_.  Note that if a graph can be colored with $\chi$ colors, then its maximum clique size is less or equal to $\chi$. Why? If there is a clique of size $k$, then each node will have a different color. Therefore, $\chi$ is at least $k$.  This is an upper bound!  How to use it? Take the graph induced  from the set  $|\mathbf{C}(S)|$. Color it using any known coloring algorithm. Let the number of colors obtained plus the size of your solution be your upper bound.  

**Bounding Bonus Note 1:** Did you note that each subtree has a different bound? Which bound would you prefer? I answer here. The smallest upper bound.  So, should we traverse our tree in a Depth-First traversal?  We traverse the search tree in depth-first because we are using the _function calls stack_. We can use a stack of our design if you want and we would get the same results.  If we use a _queue_, we would traverse the tree in _breadth-first traversal_. We can use other forms of traversals if we use a min-queue using different ordering heuristics.  Smart tree traversal heuristics can have a great impact on the time of finding the solution.


**Bounding Bonus Note 2:** The bounding example usually taught in Algorithm courses use the Zero-One Programming problem as an example. Let's assume that you have an IP problem such that you want to maximize  $\mathbf{c^Tx}$ subject to $\mathbf{Ax \le b}$  and $\mathbf{x} \in \{0, 1\}$. The upper bound we use here is the solution of the relaxed linear programming problem (i.e., remove the constraint $\mathbf{x} \in \{0, 1\}$.  The relaxed $\mathbf{c^Tx}$ is greater than or equal to that of the constrained version.