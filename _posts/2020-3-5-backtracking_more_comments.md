---
layout: post
title: Some notes about Backtracking
categories: algorithms
keywords: algorithms
---
# Backtracking: Some Notes

**The use of yield in the python code:** we may want to process the solution from outside the backtracking algorithm.  We let our algorithm return a generator instead. We use the keyword yield. As an example: 

```python
def all_n_binary(n):
	def _all_n_binary(lst): 
		if  len(lst) == n:
			yield lst
			return
		
		for x in [0,1]: 
			yield  from _all_n_binary(lst + [x])
	return _all_n_binary([])

print([f for f in all_n_binary(3)])
```

**The case of enumerating pairs**:  you have probably been writing algorithms to generate all possible solutions, where each solution is a pair $(x,y)$ for $x \in X$ and $y \in Y$.  We do that all the time, but with a simpler algorithm. It is actually just two for-loops. 

```python
for x in X: 
	for  y in Y: 
		process((x,y))
```

```python 
for x,y in [x,y for x in X for y in Y]: 
	process((x,y)) 
```

Sometimes we want to set $Y$ to $X$. For example, if $X$ is a set of points, we want to generate all possible line segments from $X$, or we want to generate all substrings in a given string. We want in these cases to make sure that we do not repeat the solution $\{x_1, x_2\}$ and $\{x_2, x_1\}$, so we add additional constraint that we never extend a solution $\{x_1}\$ with values greater than the maximum element inside it, as follows. 

```python 
for x,y in [x,y for x in X for y in X if y > x] : 
	process((x,y)) 
```

To enumerate n-sets, instead of 2-sets (i.e., pairs), we will need n for-loops. That's just too much, so we write a recursive backtracking algorithm (or, a for-loop with a stack or queue). 

Excersices like this are common in technical challenges or interviews. Thinking of the solution in term of backtracking will make your job easier.  This approach allows you to write a quick solution very quickly, and then improve it (e.g., by prunning the search tree, or finding a more intelligent to check if a solution is possible). 




> Written with [StackEdit](https://stackedit.io/).
