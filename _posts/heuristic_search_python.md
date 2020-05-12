---
layout: post
title: Heuristic Search Algorithms
categories: algorithms
keywords: algorithms
---


# Heuristic Search Algorithms

We build on what has been discussed in the Backtracking post. Our focus is still on optimization problems (maximize an objective function). We will study in this post **heuristic search algorithms**. The idea of these algorithms is simple.  If you know one solution $s_1$, can you create from it another solution $s_2$?  For example, The solution 0111 is a solution in the 4-binary numbers enumeration problem. Can you create 0101 from 0111? You just need to flip one bit.  **Can you go from one solution to another until the maximum is found?** The answer is yes. We can do that in many ways. This post will go through some main approaches. 

**Some definitions:** Let's represent our solutions as vertices in a **solutions graph $G = (V,E)$**.  We create an edge between a pair of solutions $(s_1, s_2)$ if it is possible to convert $s_1$ to $s_2$. The relationship is not necessarily undirected. That is, $(s_1,  s_2) \in E$ does not imply $(s_2, s_1) \in E$ .  We define the neighborhood of $s_1$, denoted $N(s_1)$, as all solutions can be obtained from $s_1$.  

But **how to define these neighborhood relationships?** It is best to give an example to explain this. Let's assume that your solution is a 01-string. A solution $s_1$ is linked to another $s_2$ if the Hamming distance between them is $k=1$. That is, it takes you one bit trigger to convert one to the other.  Therefore, the neighbors of 0000 are $\{0001, 0010, 0100, 1000\}$.  You can set $k$ to 2, 3, or $n$ (and this case, the graph $G$ is complete). You can define other kinds of relationships. It is only your imagination (and a bit of logic) that limits you. 

Let's assume we have solution $s_i$ with cost $c_i$ that we found using some algorithm.  We can look for a better solution in $N(s_i)$ (let's call it $s_{i+1}$ if found). From $s_{i+1}$, we repeat the same step. We stop when $s_{i+1}$ is the best solution in its neighborhood.  This is what we call the **Hill-Climbing Search Algorithm**. 

There are some important details we should care about:

 - **Should we look for the best solution in $N(s_i)$? or any solution is fine?** This step can be a bottleneck if $N(s_i)$ is large. If $N(s_i)$ has every other solution in the graph, then we will pass by each solution, and a maximum is found from one iteration (the more fancy name for this in ML is _Grid Search_). If $N(s_i)$ is small, then we have less power reaching the best solution (fewer edges implies fewer possible paths to reach the optimal solution).  

 - **What should we do with infeasible solutions in $N(s_i)$?** For example, some algorithms decide to take a random solution in $N(s_i)$ to save on execution time. This random solution, however, can be infeasible.  What should we do? Some possible choices: 
	 - Stop the search and announce the winner, 
	 - Find another solution, and repeat. 


**(insert code with comments). **

You follow this approach, and probably will not find the optimal solution. You will find a solution that has the maximum score among its neighbors. In more technical terms, we always find a **local maxima** (if run sufficiently many times), but not necessarily find a **global maxima**.   To find the global maxima, what we usually do is to run the algorithms multiples times, but from different starting points (technically, to make the experiments independent). 

**(insert example)**

Ok, let's assume you picked a random solution $s_{i-1}$ from the neighborhood set (i.e., this is the approach you are following).  This solution is not better than $s_i$. What should we do? previously we said we either stop the search or find another solution and repeat.   Let's add another option. What we can do is to **accept $s_{i-1}$ with certain probability $p_a$**.  This can help in finding the global maxima.  You can use this approach as well when you are stuck at a local maxima. That is, you are in a local maxima $s_{i-1}$. Instead of announcing the winner, you pick a less preferrable solution $s_{i}$ randomly and then accept it with probability $p_a$.  This is the idea of **Simulated Annealing**.  Picking $p_a$ is an art (as most of these heuristics). Simulated Annealing sets $p$ to be a function of the current iteration (i.e., $i$), the cost of the current solution $c_i$, and the chosen solution $c_{i+1}$.  For example, if $|c_{i} - c_{i-1}|$ is large, then $p$ is small. If $i$ is large, then $p_a$ is small. 


You move down the hill, what stop you from going back to the same peak you were in? Nothing really (given what we have proposed so far). In fact, we may get stuck in a cycle (a $\rightarrow$ b $\rightarrow$ c $\rightarrow$ d $\rightarrow$ a).  **Tabu Search**, another heuristic algorithm, creates a set of forbidden choices; a Tabu list.  This list can be designed as a sliding window. 


> Written with [StackEdit](https://stackedit.io/).
