---
layout: post
title: "An unconventional way to calculate Fibonacci numbers"
categories: algorithms
keywords: algorithms; 
---

You probably have seen many ways to calculate Fibonacci numbers. Instructors use this problem to explain various concepts (e.g., how recursion can explore exponentially, dynamic programming bottom-up or top-down approaches, caching, matrix multiplication, and others). I will introduce one rather unconventional way to solve this problem. Whatever I am introducing here is to help understand dynamic programming. The solution is general-purpose and I wouldn't recommend it for a well-understood problem such as Fibonacci numbers. 

I recall I was asked a long time ago to solve the Fibonacci number problem in an interview. My solution was quite typical. Start from $i = 0$. Calculate `Fib(i)` until $i = n$. You should always keep track of `Fib(i-1)` and `Fib(i-2)`.  I told the interviewer that I used dynamic programming for that. His reply was quite interesting. "They call this dynamic programming?". Until now, I wish I knew whether he meant that "this is not dynamic programming", or "hmm.. I don't know what dynamic programming is". He was a smart guy. I think he meant the former. If this was the case, then I agree with him because *Caching is not dynamic programming*.  

Let's write `Fib(n)` for $n = 4$ as a system of equations. 

<div class="math">
\begin{alignat*}{4}
   f(4) & {}={} &  f(3) & {}+{} & f(2) \\
   f(3) & {}={} &  f(2) & {}+{} & f(1) \\
   f(2) & {}={} &  f(1) & {}+{} & f(0) \\
   f(1) & {}={} &  1                    \\
   f(0) & {}={} &  0                             
\end{alignat*}
</div>

In order to evaluate $f(i)$, we should have already calculated $f(i-1)$ and $f(i-2)$. I would like to avoid the side effects of recursion. What should I do? The idea is to have an array `V` that stores the Fibonacci values. `V` is initialized to zeros. Then, we evaluate all of the equations above *in any order* such that: 

<div class="math">
\begin{alignat*}{4}
   f(4) & {}={} &  V(3) & {}+{} & V(2) \\
   f(3) & {}={} &  V(2) & {}+{} & V(1) \\
   f(2) & {}={} &  V(1) & {}+{} & V(0) \\
   f(1) & {}={} &  1                    \\
   f(0) & {}={} &  0                             
\end{alignat*}
</div>

Then, we set `V(i)` to $f(i)$. Obviously this won't lead to the right solution. In the first iteration, $f(i)$ is only correct for the base cases $f(1)$ and $f(0)$.  But now we have new values in `V`.  If we repeat the same previous steps with the new `V`, we will start getting more correct values (e.g., $f(i) = 1$ and `V` have more correct values). Repeating these steps will eventually lead to the correct solution (i.e., $f(i)$ is correct for all $i$ and `V[i]` is equal to $f(i)$).  *When should we stop?*, when `V` does not change anymore.  


```python
def fib_v1(n): 
    indices = list(range(n+1))
    V = {k:0 for k in indices} 
    
    while True:
        random.shuffle(indices)
        prev_V = V.copy() 
    
        delta = 0 
        for i in indices: 
            if i > 1: V[i] = prev_V[i-1] + prev_V[i-2]
            else: V[i] = i
            delta = max(delta, abs(V[i]-prev_V[i]))
        
        if delta == 0: break 
    
    return V[n]
```

It is very cool that it will converge (Hint: `V[i]` won't change once it reaches the correct value). It is cooler that it will converge even if we don't use `prev_V`. Instead, we can update `V` on the fly. It will converge faster in most cases. See the code below. 

```python
def fib_v2(n): 
    indices = list(range(n+1))
    V = {k:0 for k in indices} 
    
    while True:
        random.shuffle(indices)
        
        delta = 0 
        for i in indices: 
            prev_v = V[i]
            if i > 1: V[i] = V[i-1] + V[i-2]
            else: V[i] = i
            delta = max(delta, abs(V[i]-prev_v))
        
        if delta == 0: break 
    
    return V[n]
```

You will notice that the ordering at which we evaluate the equations has an impact on the speed of convergence. The best ordering is when indices are set to `[0, 1, ..., n]`. It will take only two iterations in this case; the correct solution is reached in the first iteration whereas the second iteration is used to check that `V` does not change.  Yes, that ordering of indices is what we use in the bottom-up dynamic programming approach we all know about (or, what I have used in the interview). 

So why this unconventional method? because dynamic programming is used to solve more complex problems. For example. Let's assume that we formulate problem $P$ with a system of equations as above. However, we noticed that there are cycles such that $f(i)$ depends on $f(j)$ and $f(j)$ depends on $f(i)$. The approach used in `fib_v1` and `fib_v2` would help.  The *shortest path problem* is an example of where we find these scenarios. More details in the next posts. I hope I show you there how important the shortest path problem is.

