---
layout: post
title: "All shotest paths lead to this algorithm"
categories: algorithms
keywords: algorithms; 
published: false
---

These posts are rants about the way dynamic programming is taught. I don't understand why we don't emphasize enough on graphs when we teach dynamic programming. The power of dynamic programming is clearer when we use it to solve shortest path problems. It is even clearer if we show the applications of shortest path problems (other than finding shortest path from city A to city B). In summary, it is all about graphs. 

Let's get into more details. Remember that  generalized algorithm we used to solve Fibanocci numbers in the previous post? we will use to to solve the shortest path from a source $s$ to a target $t$ in a graph $G$. If $V(s)$ is the shortest path from node $s$ to node $t$, then we have: 

1. $V(t) = 0$  
2. $V(u) = \min { w(u, x) + V(x)  : x \in N(u) }$ 

... where $w(u,x)$ is the weight of the edge from $u$ to $x$.   

The algorithm below follows the same generalized dynamic programming approach we discussed previously.

```python  
while True:
    
    
    delta = 0 
    
    for v in nodes: 
        prev_v = _get_Value(v)
        Values[v], Actions[v] = _update_shortest_path(v)
        delta = max(delta, abs(prev_v - Values[v]))
    
    if delta == 0: 
        break 
    
return Values, Actions

```

.. where `_update_shortest_path` is something like this: 

```python
def _update_shortest_path(v): 
    
    if v == t: return 0,None
    
    edges = [(w, neighbor) for neighbor,w in G[v].items() ]
    v, e = min_evaulated_action(_get_Value, edges)

    return v, e 
```
