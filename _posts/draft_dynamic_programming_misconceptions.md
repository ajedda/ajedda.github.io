

Structure:
1. Subset sum: given a set S of integers and an integer k, find a subset of S that sum to k. 
2. Approach 1: enumeration. 

```python 
"""
A = [6,5,4,2,1,]
k = 8 
Find the minimum subset that sums to k. 
"""


def bf_subset_sum_v1(A, k): 
    n_nodes = 0
    def _bf_subset_sum(index, c_size, c_sum): 
        nonlocal n_nodes 
        n_nodes += 1 
        # print(n_nodes, c_sum)
        
        n_sizes = []
        if sum(c_sum) == k: 
            print(f"This is a solution: {c_sum},{c_size}")
            n_sizes.append(c_size) 
            # return c_size
        
        for j in range(index+1, len(A)): 
            s = _bf_subset_sum(j, c_size+1, c_sum + [A[j]])
            if s is not None: n_sizes.append(s)
         
        return None if len(n_sizes) == 0 else min(n_sizes)

    res =  _bf_subset_sum(-1, 0, [])
    print("Nodes:", n_nodes)
    return res


# ---------------------
def bf_subset_sum_v2(A, k): 
    n_nodes = 0 
    def _bf_subset_sum(index, c_size, c_sum): 
        
        nonlocal n_nodes 
        n_nodes += 1
        # print(n_nodes, index, c_sum)
        if index >= len(A): return None 
        
        ms = []
        s = sum([x for x in c_sum if x is not None])
        if s == k: 
            print(f"This is a solution: {c_sum},{c_size}")
            ms = [c_size]
        
       
        
         
        
        v1 = _bf_subset_sum(index+1, c_size+1, c_sum + [A[index]])
        v2 = _bf_subset_sum(index+1, c_size, c_sum + [None] )
        
        ms = ms + [v1, v2]
        ms = [v for v in ms if v is not None]
        return None if len(ms) == 0 else min(ms)

    
    res =  _bf_subset_sum(0, 0, [])
    print("Nodes: ", n_nodes)
    return res 

A = [6,5,4,1,1,]
k = 9  
print(bf_subset_sum_v1(A,k))
print("--" * 5)
print(bf_subset_sum_v2(A,k))
```

This returns the size of the solution. But what if you wanted the best solution? 

```python 
def bf_subset_sum(A, k): 
    best_S = []
    
    def _bf_subset_sum(index, S): 
        nonlocal best_S 
        if sum(S) == k: 
            if best_S == [] or len(S) < len(best_S): 
                best_S = S
            return 
        
        
        for j in range(index+1, len(A)): 
            _bf_subset_sum(j, S+[A[j]])

            
        return 

    _bf_subset_sum(-1, [])
    return best_S 
```


4. Approach 2: memorization.
    - People call this dynamic programming. It's not. 
    - Compare how the structure of the solution changes. 
        - Enumeration: exponentially large tree. 
        - Memoization: quasi-polynmial graph. 

5. Explain why this is not dynamic programming. 
    - Fibannocci as an example. 
        - The quick solution. 
        - Formulate as a system of equations. 

6. hmmm .. that doesn't seem so obvious to me. 
    - Why should I visit the nodes in Fiabnnoci in unspecified order?
    - Let's take another example then. 
        - shortest path of a graph. 
        - average path of a graph. 
        - Side note: can we reach shortest path from average path? Yes. 

7. So why Dynamic-SS was called dynamic programming? 
    - In reality, we are doing two steps: 
        - 1. build the graph. 
        - 2. calculate a shortest path. 

8. Still not clear: why shortest path took many iterations, while Dynamic-SS took only 1? 
    - This is caused by the structure of the graph. 
        - DAG vs. arbitraty graph. 
        - Example: find s-t shortest path in a DAG.

9. How these concepts were used in Dijkstra shortest path algorithm?
    - minimize the number of iterations until convergence. 