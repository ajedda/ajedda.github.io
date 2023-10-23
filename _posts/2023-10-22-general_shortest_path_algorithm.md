---
layout: post
title: "All shotest paths lead to this algorithm"
categories: algorithms
keywords: algorithms; 
published: true
---

The previous [post]({{ site.baseurl }}{% link _posts/2022-04-05-fibanocci_dynamic_programming.md %}) examined Fibanocci sequence and how we can generate them using dynamic programming (DP). The main conclusions were: 

1. Naive recursion will often fail. 

2. We solved the problem using a generic Dynamic Programming approach. That is, initializing $F(x)$ for all of x. Then, in iterations, evaluating and updating $F(x)$ using exisiting F(y) until convergence.  

3. The order at which we evaluate $F(x)$ improves the speed of convergence (e.g., it is fastest for Fibanocci when we start evaluating $F(0)$). 
	
Fibanocci sequence is the one of first examples that algorithms courses use to teach DP. However, a high percentage of students and engineers usually get one point from these courses; Memoization. Let me cut it short, DP is not Memoization. 

If I taught DP to undergrads, I would delay it until teaching graphs and shortest path algorithms. Why? 1) DP is all about graphs, 2) one of the major problems in DP is shortest path. The following, and other future posts, should clarify this point in more details. 

Let's formulate the shortest path problem in its simplest form. We will assume positive edges only. If $V(x)$ is the shortest path from $x$ to a destination $t$. Then: 
	
<div class="math">
\begin{alignat*}{4}
	V(x) & {}={} \min \{ d(x, x) + V(x), d(x, u_1) + V(u_1),  d(x, u_2) + V(u_2) \dots \}  \\
    V(t) & {}={} 0 \\
\end{alignat*}
</div>
	
Where $d(x,u)$ is the edge length from $u$ to $v$, and $\{u_1, u_2, \dots }$ are the neighbors of $x$. 


This is a system of equations. The solutions of some equations may depends on others. Cyclic dependency may be found as well, where $V(x)$ depends on $V(y)$ and vice versa. Dynamic programming is suitable for these kind of problems.  As we did previously in the case of Fibanocci, we should solve each of the equations above in iterations starting with some initial values of $V$'s, until we reach convergence. The code should look something similiar to the one below.

```python
"""
This is the main loop. We continue iterating until convergence. 
"""
done = False 
while not done: 
    done = solve_all_equations()
```


```python 
"""
solving all equations. 
equation i solution is found in v[i] for node v_i 
keep iterating until convergence (i.e., none of v's - or shortest paths - have changed). 
"""
def solve_all_equations():
    done = True 
    for i in v.keys(): 
        changed = solve(i)
        if changed: 
            done = False 
    return done 

```

```python
"""
Updating the shortest path for node i. 
We assume that graph_dict is a key-value dict with nodes as keys, and values are lists 
of pairs (x, d): x: neighbor of node, and d for edge weight. 
The value next_step[i] records the neighbor of i that leads to the shortest path. 
We assume no loops and no negative edges. 
"""
def solve(i): 
    
    all_i_distances = [(d + v[x], x) for x, d in graph_dict[i]] + [(v[i], i)]
    new_v_i, new_n_i = min(all_i_distances)
    
    changed =  (new_v_i != v[i])
   
    if changed:
        v[i] = new_v_i 
        next_step[i] = new_n_i

    return changed 

```



Let's come with final questions and answers: 

1. **How long should we iterate?** 
    - The algorithm above iterates until no changes in the solution. This is upper bounded by the number of nodes in the graph, denoted $n$ (that is, there will be no changes after $n$ steps). 

2. **Is this a new algorithm?** 
    - No it is not. This is often called Bellman-Form algorithm, which is taught in undergraduate algorithms courses. The way of writing the code is different and the reason of that is to magnify the use of DP here. Many versions iterates $n$ times.

3. **Can we converge faster?** 
    - Yes. This is the whole idea of many other shortest path algorithms. It all depends on the order of evaluating the DP equations. Recall how generating Fibanocci $F(n)$ was much faster when evaluating from $0$ to $n$ compared to $n$ to 0.  The same idea applies in these algorithms: 
        - Dijkstra: always pick the node closest to the target. 
        - You can pick the nodes in a random order. 
        - You can assign an order for the nodes, evaluating in this order at even iterations, and in the opposite order in the odd iterations. 
            - Note: these algorithms may sound as random heuristics I came out with. They are not. These are published papers. 
        - We have special algorithms if the underlying graph is non-arbitrary (for example, DAG or a tree).  
	- *Note*: shortest path in DAG is a very interesting example. It falls in a family of DP algorithms such as 0-1 Knapsack, Longest common sequence, and many others. I will have to write a post about it. 

4. **Is it always the case that the upper bound of convergence is $n$ (where $n$ is the number of equations)?**
    - No. It all depends on the equations. Assume for example that we change `min` to `mean`. The algorithm will not converge in $n$ steps. The algorithm in this case will solve the expected path length to reach a node (by the way, it has some applications. I will have to write a post about it soon). 


 

