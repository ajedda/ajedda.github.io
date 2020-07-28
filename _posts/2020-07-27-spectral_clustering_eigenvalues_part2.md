---
layout: post
title: "Spectral Clustering: how it works in practice?"
categories: algorithms
keywords: algorithms; Machine Learning
---


My objective in this post is to explain some theoretical ideas with examples written in Python. 
I will show how spectral clustering behaves in an idle scenario. In this ideal scenario, the underlying graph consists of subgraphs (components) with no links between any pair of nodes in different subgraphs. We will further assume that each subgraph is a clique (i.e., there is a link of weight 1 between all the vertices of a subgraph). 

Let's build this graph using function ``build_blocked_W`` below. The parameter `n` is the number of nodes in $G$, and `n_b` is the number of subgraphs. 


```python
import numpy as np 
import random


def build_blocked_W(n=9, n_b=3, shuffle=True): 
    """
    Build a graph with n nodes, and n_b connected componenets. 
    If shuffle is False, all nodes within a component are indexed 
    next to each other. Otherwise, the nodes are shuffled.
    """
    assert n % n_b == 0 
    
    
    comps_indices = [i for i in range(n)]
    if shuffle == True: 
        random.shuffle(comps_indices)
    
    # building the blocks. E.g. [(0,1,2), (3,4,5), (6,7,8)]
    comps = [comps_indices[i*n_b:(i+1)*n_b] for i in range(n)]


    W_sh = np.zeros((n,n))
    for g in comps: 
        for i,j in [(i,j) for i in g for j in g]: 
            W_sh[i,j] = 1
            
    return W_sh



def is_symmetric(M): 
    """
    Test if a matrix M is symmetric.
    """
    return np.allclose(M, M.T)



```


```python
W = build_blocked_W(9, 3, False)
assert is_symmetric(W)
W
```




    array([[1., 1., 1., 0., 0., 0., 0., 0., 0.],
           [1., 1., 1., 0., 0., 0., 0., 0., 0.],
           [1., 1., 1., 0., 0., 0., 0., 0., 0.],
           [0., 0., 0., 1., 1., 1., 0., 0., 0.],
           [0., 0., 0., 1., 1., 1., 0., 0., 0.],
           [0., 0., 0., 1., 1., 1., 0., 0., 0.],
           [0., 0., 0., 0., 0., 0., 1., 1., 1.],
           [0., 0., 0., 0., 0., 0., 1., 1., 1.],
           [0., 0., 0., 0., 0., 0., 1., 1., 1.]])




```python
def build_laplacian(W): 
    """
    Build the laplacian of the graph represented by W, 
    where the laplacian is: L = D-W
    D is the diagnoal matrix such that D[i,i] is the degree
    of node i in W. 
    
    returns D and L. 
    """
    D = np.diag(np.sum(W, axis=1))
    L = D - W 
    return D, L 


def is_diagonal(M): 
    """
    Test if matrix M is diagonal. Mij = 0 if i!=j
    """
    if not is_symmetric(M): return False 
    _0 = np.zeros(M.shape)
    return np.allclose(_0, M - np.diag(np.diag(M)))


D, L = build_laplacian(W)
assert not is_diagonal(L)
assert is_diagonal(D)

print("Matrix D\n", D, "\n")
print("Matrix L\n", L)
```

    Matrix D
     [[3. 0. 0. 0. 0. 0. 0. 0. 0.]
     [0. 3. 0. 0. 0. 0. 0. 0. 0.]
     [0. 0. 3. 0. 0. 0. 0. 0. 0.]
     [0. 0. 0. 3. 0. 0. 0. 0. 0.]
     [0. 0. 0. 0. 3. 0. 0. 0. 0.]
     [0. 0. 0. 0. 0. 3. 0. 0. 0.]
     [0. 0. 0. 0. 0. 0. 3. 0. 0.]
     [0. 0. 0. 0. 0. 0. 0. 3. 0.]
     [0. 0. 0. 0. 0. 0. 0. 0. 3.]] 
    
    Matrix L
     [[ 2. -1. -1.  0.  0.  0.  0.  0.  0.]
     [-1.  2. -1.  0.  0.  0.  0.  0.  0.]
     [-1. -1.  2.  0.  0.  0.  0.  0.  0.]
     [ 0.  0.  0.  2. -1. -1.  0.  0.  0.]
     [ 0.  0.  0. -1.  2. -1.  0.  0.  0.]
     [ 0.  0.  0. -1. -1.  2.  0.  0.  0.]
     [ 0.  0.  0.  0.  0.  0.  2. -1. -1.]
     [ 0.  0.  0.  0.  0.  0. -1.  2. -1.]
     [ 0.  0.  0.  0.  0.  0. -1. -1.  2.]]
    




The next functions will return the eigenvalues and eigenvectors of a matrix M and sort them according to the order of the eigenvalues. 


```python
def sort_eig(eig_vals, eig_vecs): 
    s_eig_vecs = eig_vecs[:, eig_vals.argsort()]
    s_eig_vals = np.sort(eig_vals)
    return s_eig_vals, s_eig_vecs 


def get_eig(M, ordered_vec=True):
    eig_vals, eig_vecs = [np.round(x.real,2) for x in np.linalg.eig(M)] 
    if ordered_vec==True: 
        eig_vals, eig_vecs = sort_eig(eig_vals, eig_vecs) 
    return eig_vals, eig_vecs

```

First, let's study the eigenvalues of 1 of the connected components. This is a clique of `n_b` nodes. `L[:n_b, :n_b]` is what we are looking for: 


```python
n_b = 3 
Lb = L[:n_b,:n_b] # the first block of the Laplacian. 
wb, vb = get_eig(Lb) # these will be sorted. 

print("Eigen-values (ordered) of one block of L (Lb)\n", wb, '\n')
print("Eigen-vectors (ordered) of one block of L (Lb)\n", vb, '\n')

```

    Eigen-values (ordered) of one block of L (Lb)
     [-0.  3.  3.] 
    
    Eigen-vectors (ordered) of one block of L (Lb)
     [[-0.58  0.82  0.31]
     [-0.58 -0.41 -0.81]
     [-0.58 -0.41  0.5 ]] 
    
    

The first eigenvector is `vb[:,0]`


```python
print(vb[:,0])
```

    [-0.58 -0.58 -0.58]
    

The elements of `vb[:0]`  are all the same. The corresponding eigenvalues is zero. This matches our analysis. 

The elements of this vector have the value $1/\sqrt{n}$. You may notice sometimes that the sign is negative. If yes, the corresponding eigenvalue is negative too (it is -0 in any case). Just to verify the result, we can check that $B_i v_{bi} = w_{bi} v_{bi}$.


```python
i = 1
Lb @ vb[:,i], wb[i] * vb[:,i]    # the @ operator is the matrix multiplier. 
```




    (array([ 2.46, -1.23, -1.23]), array([ 2.46, -1.23, -1.23]))



That was for 1'st block of `L`. Let's examining the eigen properties of `L`. 


```python
w, v = get_eig(L, ordered_vec=false)
print("Eigen-vectors of L\n", v, '\n')
print("Eigen-values of L\n", w)
```

    Eigen-vectors of L
     [[ 0.82 -0.58  0.31  0.    0.    0.    0.    0.    0.  ]
     [-0.41 -0.58 -0.81  0.    0.    0.    0.    0.    0.  ]
     [-0.41 -0.58  0.5   0.    0.    0.    0.    0.    0.  ]
     [ 0.    0.    0.    0.82 -0.58  0.31  0.    0.    0.  ]
     [ 0.    0.    0.   -0.41 -0.58 -0.81  0.    0.    0.  ]
     [ 0.    0.    0.   -0.41 -0.58  0.5   0.    0.    0.  ]
     [ 0.    0.    0.    0.    0.    0.    0.82 -0.58  0.31]
     [ 0.    0.    0.    0.    0.    0.   -0.41 -0.58 -0.81]
     [ 0.    0.    0.    0.    0.    0.   -0.41 -0.58  0.5 ]] 
    
    Eigen-values of L
     [ 3. -0.  3.  3. -0.  3.  3. -0.  3.]
    

Note how the eigenvectors matrix is divided into blocks in the same way `L` is divided. Note as well how these blocks in `v` are repeated, and are all equal to `v_b` (the eigenvectors matrix of the block `Lb`). 

Another thing to note (which is quite important here) is that we have 3 eigenvalues with value zero. This is equivalent to the number of components. Therefore, we can find the number of connected components within a graph by simply checking how many eigenvalues is equal to zero.

Let's sort the eigenvectors now (according to the eigenvalues). 


```python
w_sorted, v_sorted = sort_eig(w,v)
print("The sorted eigenvectors are: \n", v_sorted, '\n')
print("The sorted eigenvalues are: \n", w_sorted)
```

    The sorted eigenvectors are: 
     [[-0.58  0.    0.    0.82  0.31  0.    0.    0.    0.  ]
     [-0.58  0.    0.   -0.41 -0.81  0.    0.    0.    0.  ]
     [-0.58  0.    0.   -0.41  0.5   0.    0.    0.    0.  ]
     [ 0.   -0.58  0.    0.    0.    0.82  0.31  0.    0.  ]
     [ 0.   -0.58  0.    0.    0.   -0.41 -0.81  0.    0.  ]
     [ 0.   -0.58  0.    0.    0.   -0.41  0.5   0.    0.  ]
     [ 0.    0.   -0.58  0.    0.    0.    0.    0.82  0.31]
     [ 0.    0.   -0.58  0.    0.    0.    0.   -0.41 -0.81]
     [ 0.    0.   -0.58  0.    0.    0.    0.   -0.41  0.5 ]] 
    
    The sorted eigenvalues are: 
     [-0. -0. -0.  3.  3.  3.  3.  3.  3.]
    

Now assume that we wanted to cluster the data into `n_b` clusters. We will have to examine the first `n_b` eigenvectors and run K-Means on top of them. 


```python
print("The first n_b eigenvectors of L:\n\n", v_sorted[:,:n_b])
```

    The first n_b eigenvectors of L:
    
     [[-0.58  0.    0.  ]
     [-0.58  0.    0.  ]
     [-0.58  0.    0.  ]
     [ 0.   -0.58  0.  ]
     [ 0.   -0.58  0.  ]
     [ 0.   -0.58  0.  ]
     [ 0.    0.   -0.58]
     [ 0.    0.   -0.58]
     [ 0.    0.   -0.58]]
    

I think it is easy to imagine what the result will be. 

Will the results change if the matrix `W` was shuffled? Short answer: the eigenvectors will be different according to the shuffle. In principle, everything we learned so far should still work.  Below are some examples. 


```python
W_sh = build_blocked_W(9, 3, True)
print("A shuffled W: \n", W_sh, '\n\n')

D_sh, L_sh = build_laplacian(W_sh)
assert is_symmetric(L_sh)
assert is_symmetric(D_sh)

w_sh, v_sh = get_eig(L_sh)
print("Eigenvectors of shuffled L:\n", v_sh, '\n')
print("Eigenvalues of shuffled L:\n", w_sh, '\n')
print(v_sh[:,:n_b])
```

    A shuffled W: 
     [[1. 1. 0. 1. 0. 0. 0. 0. 0.]
     [1. 1. 0. 1. 0. 0. 0. 0. 0.]
     [0. 0. 1. 0. 0. 1. 1. 0. 0.]
     [1. 1. 0. 1. 0. 0. 0. 0. 0.]
     [0. 0. 0. 0. 1. 0. 0. 1. 1.]
     [0. 0. 1. 0. 0. 1. 1. 0. 0.]
     [0. 0. 1. 0. 0. 1. 1. 0. 0.]
     [0. 0. 0. 0. 1. 0. 0. 1. 1.]
     [0. 0. 0. 0. 1. 0. 0. 1. 1.]] 
    
    
    Eigenvectors of shuffled L:
     [[-0.58  0.    0.    0.82  0.31  0.    0.    0.    0.  ]
     [-0.58  0.    0.   -0.41 -0.81  0.    0.    0.    0.  ]
     [ 0.    0.58  0.02  0.    0.    0.82 -0.21 -0.1  -0.  ]
     [-0.58  0.    0.   -0.41  0.5   0.    0.    0.    0.  ]
     [ 0.    0.   -0.58  0.    0.    0.   -0.79 -0.45 -0.  ]
     [ 0.    0.58  0.02  0.    0.   -0.41  0.11  0.63 -0.  ]
     [ 0.    0.58  0.02  0.    0.   -0.41  0.11 -0.53  0.  ]
     [ 0.    0.   -0.58  0.    0.    0.    0.39  0.3   0.71]
     [ 0.    0.   -0.58  0.    0.    0.    0.39  0.15 -0.71]] 
    
    Eigenvalues of shuffled L:
     [-0. -0.  0.  3.  3.  3.  3.  3.  3.] 
    
    [[-0.58  0.    0.  ]
     [-0.58  0.    0.  ]
     [ 0.    0.58  0.02]
     [-0.58  0.    0.  ]
     [ 0.    0.   -0.58]
     [ 0.    0.58  0.02]
     [ 0.    0.58  0.02]
     [ 0.    0.   -0.58]
     [ 0.    0.   -0.58]]
    


```python
# For verification. 
i = 2
print(L_sh @ v_sh[:,i])
print(w_sh[i] * v_sh[:,i]) 
```

    [0. 0. 0. 0. 0. 0. 0. 0. 0.]
    [ 0.  0.  0.  0. -0.  0.  0. -0. -0.]
    

That was the ideal case. What about a more realistic example? In the most realistic example, we assume that `L` = `L_ideal` + `H` (or let's denote it, $L_n = L + H$). Matrix $L_n$ is the noisy version of the ideal $L$. Matrix $H$ is a *perturbation* that must be symmetric. This perturbation changes the properties of $L$. However, the change can be bounded. There are at least a couple of bounds. For example: $\lambda_i(L_n)$ is between $\lambda_i(L) + \lambda_1(H)$ and  $\lambda_i(L) + \lambda_n(H)$. In certain circumstances, the difference between $L_n$ and $L$ eigenvalues can be a factor of the norm of $H$, and $\mid\lambda_{k+1}(L) - \lambda_{k}(L)\mid$. The larger $H$'s norm the larger is the perturbation effect, and the larger  $\mid\lambda_{k+1}(L) - \lambda_{k}(L)\mid$ the smaller is the perturbation. 

These are all, unfortunately, depends on $L$ and $H$. We don't know these matrices in practice. Can this be useful however? Heuristically, we can assume in practice however that the quality of our clusters is better if $\mid \lambda_{k+1}(L_n) - \lambda_{k}(L_n)\mid$ is large. 

Let's play around with some examples. 


```python
def some_noise(shape, a=0, b=0.1):
    """
    Build symmetric non-negative noise with a given shape.
    Use a uniform distribution in [a,b]
    """
    assert a >= 0
    assert b >= 0
    Nz = np.random.uniform(a, b, shape)
    Nz_up = np.triu(Nz, k=1)
    Nz = Nz_up.T + Nz_up 
    assert is_symmetric(Nz)
    return Nz
```


```python
Nz = some_noise(W.shape, 0, 0.1)
W_nz = W + Nz
assert is_symmetric(W_nz)
```


```python
D_nz, L_nz = build_laplacian(W_nz)
w_nz, v_nz = get_eig(L_nz, True)
print(w_nz)
print()
print(v_nz)
print('\n\n')
print('compare this to eigenvectors/values of Nz')

D_noise, L_noise = build_laplacian(Nz)
w_noise, v_noise = get_eig(L_noise, True)

print(w_noise)
print()
print(v_noise)
```

    [-0.    0.37  0.39  3.24  3.33  3.38  3.4   3.49  3.49]
    
    [[-0.33  0.13 -0.45 -0.3  -0.13 -0.61 -0.41 -0.13 -0.07]
     [-0.33  0.13 -0.46 -0.18  0.1   0.46  0.01  0.4   0.49]
     [-0.33  0.13 -0.45  0.49  0.06  0.14  0.38 -0.28 -0.43]
     [-0.33  0.32  0.34 -0.07  0.2  -0.13  0.26 -0.54  0.5 ]
     [-0.33  0.33  0.33  0.26 -0.62 -0.15  0.12  0.43 -0.  ]
     [-0.33  0.33  0.35 -0.2   0.39  0.29 -0.37  0.1  -0.48]
     [-0.33 -0.46  0.1  -0.58 -0.32  0.18  0.36 -0.13 -0.23]
     [-0.33 -0.45  0.12  0.18  0.51 -0.42  0.19  0.4   0.03]
     [-0.33 -0.46  0.12  0.4  -0.19  0.24 -0.55 -0.26  0.19]]
    
    
    
    compare this to eigenvectors/values of Nz
    [0.   0.23 0.29 0.36 0.38 0.39 0.45 0.49 0.5 ]
    
    [[ 0.33  0.34 -0.11  0.47  0.32  0.31  0.5  -0.2  -0.21]
     [ 0.33  0.26 -0.18  0.03 -0.47 -0.41  0.39  0.37  0.33]
     [ 0.33 -0.39 -0.35  0.03 -0.53  0.03 -0.07 -0.45 -0.35]
     [ 0.33 -0.03  0.34  0.12 -0.13  0.18 -0.24  0.62 -0.52]
     [ 0.33 -0.27 -0.31  0.45  0.3  -0.02 -0.46  0.15  0.43]
     [ 0.33  0.03  0.66  0.14  0.06 -0.49 -0.11 -0.42  0.05]
     [ 0.33  0.64 -0.22 -0.44  0.06  0.08 -0.45 -0.14 -0.03]
     [ 0.33 -0.21  0.33 -0.31 -0.13  0.61  0.16 -0.05  0.48]
     [ 0.33 -0.37 -0.16 -0.5   0.52 -0.29  0.28  0.13 -0.18]]
    

This is mainly it for this post. I want to add some final comments and notes: 

**One of the nice properties of spectral clustering is that it depends on the weights/similarities** between the vertices. This is contrary to K-Means which depends on the position of these points (assuming that they lay on an $R^d$ space). This makes spectral clustering more general. For example, non-geometric graphs (e.g., trees, cliques, or most of the the graphs we learn in a beginner graph theory courses) do no depends on the actual position of the vertices. These graphs are regularly found in social networks for example. Of course, you can always set the similarity between two vertices to be based on the distance between them. For example, you can set $w_{ij}$ to be $\frac{1}{d_{ij}}$, where $d_{ij}$ is the distance between the two nodes.

**Should you accept whatever $w_{ij}$ is given to you?** No. We know already that the weights should be non-negative (i.e., $w_{ij} \ge 0$) and symmetric (i.e., $w_{ij} = w_{ji}$). But, we also want to penalize non-similar nodes making them equal to zero if they are not very similiar. For example, we can use $w^{'}_{ij}$ = $p(w_{ij})$: 
 
1. For $p(w_{ij})$ equal to zero if $v_i$ is not in the $k$-neighborhood of $v_j$ for some $k$.  ($k$-neighborhood graph)
2. For $p(w_{ij})$ equal to $e^{-1/w_{ij}}$, or even more $e^{-(1/w_{ij})^2}$ (or, a more general $e^{\frac{-(1/w_{ij})^2}{\sigma}}$). (radial-basis graph). 


**Know your algorithm**. Without what is going behind the scenes we won't really understand the results of this algorithm. But although theory is quite useful, someone should know that there is no bound to how good the cluster is. That is, the solution of spectral clustering, which is the solution of a relaxed integer optimization problem, has no bounded guarantee of of the original non-relaxed optimization problem.  Spectral clustering is a cool combination of theory and practice.  




