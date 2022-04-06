---
layout: post
title: "Notes about Teaching Dynamic Programming - Part 1"
categories: algorithms
keywords: algorithms; 
published: false
---

When teaching dynamic programming, we talk a lot about top-down approaches, bottom-up approaches, and sub-problems. We don't talk enough about graphs and enumeration. (NB: I think algorithm courses focus on graphs as data structures instead of relationships. We are not doing much good to graphs with this approach). Anyway, if you talk about graphs an enumeration more, we would be able to teach students better approach to designing the dynamic programming recursive functions. My main point, dynamic programming is all about graphs.

Perhaps, the hardest part of designing a dynamic programming algorithm is the solution recursive function. The input of this function $V$ is a _state_ $s$, denoted $V(s)$, which is a function of other states $s_{i_1}, \dots, s_{i_k}$, denoted $V(s_i) = f(s_{i_1}, \dots, s_{i_k})$. We usually call s_{i_1}, \dots, s_{i_k} as the _neighbors_ of $s_i$. Your first main job is to define what a _state_ means to your algorithm, and to define what are the neighbors of each state. 

As an example, there are many states in the psychic trading algorithm from the previous posts. Each of them is defined by two factors; 1) the value of the stocks of the portfolio, and 2) the time step. We denoted $s$ as `(p, t)`.  The neighbors of $s$ are the actions we are allowed to do (If `p > 0`, sell or do nothing. If `p > 0`, buy or do nothing). There are some states that didn't have any neighbors; those where `t` is greater than our horizon. We typically call these states the base cases of a recursion. In some cases (e.g., fiabonicci number), the states are straightforward. 

Many combinatorial problems (specially, optimization problems) can be solved with DP. The states of the DP algorithm of these problems usually come from enumerating  all possible solutions of the problem (i.e., brute force). Let's take a very common example; **0-1 Knapsack** problem. In every step in the knapsack problem, we have the choice of either adding an item to the kanpsack or not. This is nothing but an enumeration of all possible solutions. What is hardly taught however is that this is not the only method of enumerating all possible solutions. I will show this with a code. Let's take a simple variant of the 0-1 Knapsack problem; the **Decision Subset Sum** problem. Given a set of integers $A$ and an integer $k$, is there a subset $A'$ of $A$ that its elements sums to $k$. There are two ways to enumerate all solutions. We implement  this by overriding the `_get_actions` 

```python
def exact_subset_sum(A, k): 
    
    ..
    ..

    
    def _get_actions_01(index, s_sum):
        if index+1 < len(A): 
            # add A[index+1]  to the solution.     
            # Why tuple? because lists are not hashable.              
            a1 = (index+1, s_sum + tuple([A[index+1]]))

            # don't add A[index+1] to the solution. 
            a2 = (index+1, s_sum )

            # Why zero? It is related to the reward of each action. 
            # More about this in the next posts. 
            actions = [(0,a1), (0,a2)]
        else: 
            # This state has no neighbors. 
            actions = []
        return actions 
    
    
    
    def _exact_subset_sum(index, s_sum):
        # s_sum is a set of the elements of the solution. 
        # You can replace this with the sum of the elements, 
        # I am just using for debugging. 

        # index: is the index of the current element in A to be processed. 

        # Yes, some of the elements in s_sum are None - just for debugging for 
        # those interested. 
        if sum(filter(None, s_sum)) == k: return k 
        
        s = (index, s_sum)
        if s in Values: return Values[s]
        
        actions = _get_actions(index, s_sum) 
        v, a = max_evaulated_action(_exact_subset_sum, actions, s)
        Values[s] = v     
        Actions[s] = a     # to store the best action. 
            
        return v 
    
    
    initial_state = (-1, tuple([]))
    _get_actions = _get_actions_01
    return _exact_subset_sum(*initial_state), Values, Actions  


```


The solution space tree here looks as follow: 



It is not the most space-efficient tree. A better tree would be the tree generated from the following `_get_actions` function. 

```python 
def _get_actions_n(index, s_sum):
    actions = [] 
    for i in range(index+1, len(A)):
        next_state = (i, s_sum + tuple([A[i]]))
        actions.append((0, next_state))
    return actions 
```


The solution space tree here looks as follow: 


Both `_get_actions` implementations lead to the same optimal solution. (Actually, not really. The way the symmetry is broken in `max_evaulated_action` may lead to different solutions all with the same sum). 


*There is something common* in all of these implementations, and all DP algorithms. The actions taken from a given state depends only on that state. It doesn't depend on previous state, future states, or any other variables.  This the *markovian* property, which is extremly powerful despite the simplicity. Many real-world systems can be modelled with systems having the markovian property (or, a Markov Decision Process - more about this in future).  An algorithm designer should take this into consideration always. If you feel that the actions taken from state $s$ depends on another state from the past, then you are doing something wrong. Here is an example of what I mean. 

Our friend the psychic trader can have only two states: 1) Portfolio empty - ready to buy, 2) Portfolio has stocks - ready to sell. But we would like to know the profit of each trade when selling.  One way to model this is by defining a variable to store the value of the stocks we own. We would like also have another variable to keep track of the time.  But having variables like these will prevent our model from being markovian. This is because the actions taken from a state $s$ does not depend only on the state $s$. Instead it depends on two additional variables. We overcame this problem by incorporating these two variables in our states definition, and hence the state become `(p, t)`. 

Two main ideas to conclude from this post. 

1. The relationship between enumeration and dynamic programming is very strong. Yet, enumeration algorithms are rarely taught in undergradurate algorithms courses. 

2. That recursive DP function should have the markovian property. Each state has a set of neighbors. An action takes from one state to another. That action depends only on the current state. This forms a graph. This raises many interesting points that I will discuss in future posts. 