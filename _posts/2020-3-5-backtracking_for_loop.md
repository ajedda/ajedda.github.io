---
layout: post
title: Implementing Backtracking with a Stack
categories: algorithms
keywords: algorithms
---

```python
class Stack(object):
	def __init__(self, ):
		self.lst = []
	def push(self, x):
		self.lst = [x] + self.lst
	def pop(self):
		return  self.lst.pop(0)
	def empty(self):
		return  len(self.lst) == 0
```

```python 
def all_n_decimal_loop(n):
	S = Stack()
	for x in  reversed([0,1,None]): S.push(x)
	lst = []
	
	while  not S.empty():
		s = S.pop()
	
		# if we exhausted all nodes in the sub-tree.
		if s is  None:
			if  len(lst) == 0: break
			lst.pop()
		else:
			# extends the solution lst with a new choice s.
			lst.append(s)
			# if solutoin: print.
			if  len(lst) == n:
				print("Solution: {}".format(lst))
			# if base-case pop the last element in the lst (or, go one level up in the tree.) 
				lst.pop(-1)
			else: 
				# These are the choices of lst + a stopping mark in the form of None.
				for x in  reversed([0,1,None]): 
					S.push(x)
``` 

 
> Written with [StackEdit](https://stackedit.io/).
