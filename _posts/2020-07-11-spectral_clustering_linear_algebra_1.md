---
layout: post
title: "Spectral Clustering using simple Linear Algebra concepts"
categories: algorithms
keywords: algorithms; Machine Learning
published: false
---

Let's speak about *spectral clustering*. 

We have a graph $G=(V,E)$.  The edge linking any two nodes $v_i$ and $v_j$ has a non-negative weight $w_{ij}$ (where $w_{ij} = 0$ only if $v_i=v_j$. 

We want to cluster the nodes of the graph into $m$ clusters.  We will say that $v_i$ is in $k^{th}$ cluster if $x_i = k$, Therefore, the vector $\mathbf{x}$ is a representation of our clusters. 

Let's assume we have a function $g$ such that $g(x_i, x_j)$ is 0 if $v_i$ and $v_j$ are in the same cluster (or, $x_i = x_j$), and 1 otherwise. 

The optimization problem: 

$$\min_{\mathbf{x}} \sum _{i,j} w_{ij} \mathbf{g}(x_i,x_j)$$  

gives clustering solutions (i.e., different values of $\mathbf{x}$). Obviously, if all nodes are in the same cluster, then the result of that objective function is minimized to zero (it can't be less), so we are looking for solutions other than this trivial one. 

One way to write $\mathbf{g}$ is 

$$\mathbf{g}(x_i, x_j) = \max (1, \mid x_i - x_j \mid)$$

This is just too complex to deal with mathematically. So, *how to solve this optimization problem?* Let's do some relaxation on this function. 

Let's take a simpler clustering problem. We would like to cluster our data into 2 clusters only. This allows us to restrict $x_i$ to be either 0 or 1 (in the first cluster or second cluster).   Now, $\mid x_i - x_j\mid$ is always either 0 or 1. This means that: $$\mathbf{g}(x_i, x_j) = \max(1, \mid x_i - x_j \mid)=\mid x_i - x_j \mid$$. 

One more step. Who likes dealing with absolute values when solving such optimization problems? Let's relax $\mid x_i - x_j \mid$ to something easier to deal with such as $(x_i - x_j)^2$.  They lead to exactly the same results since $x_i$ is either 0 or 1.  So now, let's optimize this function: 

$$ \min_{\mathbf{x}} \sum _{i,j} w_{ij} (x_i - x_j)^2$$

Let's write this in the matrix format. 

$$\sum_{ij} w_{ij} (x_i - x_j)^{2} \\= \sum_{ij} w_{ij} (x_{i}^2 - 2x_i x_j + x_{j}^2)\\=\sum_{ij} w_{ij} x_{i}^2 + \sum_{ij} w_{ij}x_j^2 - 2\sum_{ij}  w_{ij} x_i^2 x_j^2$$

There are three parts in that equation (or actually, only two parts): 

 1. *Part 1:* $\sum_{ij} w_{ij} x_i^2 = \sum_i x_i^2 \sum_j w_{ij}$
 2.  *Part 2:* $\sum_{ij} w_{ij} x_j^2 = \sum_j x_j^2 \sum_i w_{ij}$ 
 3. *Part 3:* $2\sum_{ij}  w_{ij} x_i^2 x_j^2$. 

Part 1 and Part 2 are essentially the same. There is an interesting term there; $\sum_j w_{ij}$. We will denote it $d_i$. This is the sum of all weights node $v_i$, or its *degree*.  

We can define the diagonal matrix $\mathbf{D}$ where $\mathbf{D}_{ij} = 0$ for $i \neq j$ and $d_i$ otherwise (or also denoted as $\mathbf{D} = \mathrm{diag}(d_i, ..., d_n)$) .  This allows to write the term $$ \sum_i x_i^2 \sum_j w_{ij} = \sum_i x_i ^2 d_i = \mathbf{x^TDx}$$

Regarding the third part, we can re-write it in matrix format as $\mathbf{x^TWx}$.  We end up with $$2 (\mathbf{xDx} - \mathbf{x^TWx}) = \ 2 (\mathbf{x^T(D-W)x}) $$

The matrix $\mathbf{D-W}$ is a known matrix that we call the *Laplacian* of the graph $G$, denoted $\mathbf{L}$.  

Great, our optimization problem is: 

$$\min_{x} 2 \mathbf{x^TLx}$$ 

or simply, 

$$\min_{x} \mathbf{x^TLx}$$ 

.. since that constant 2 will not change the optimal solution. 

There is an obvious solution to this problem, $\mathbf{x}=\mathbf{0}$.  Another solution is $\mathbf{x}=\mathbf{1}$, or any vector with all equal elements. These all have the same direction, but different values. Let's add a constraint to our problem. We want $\mid \mid \mathbf{x} \mid \mid = 1$.  So now we have 

$$\min_{x} \mathbf{x^TLx} \\  \mathrm{s.t.} \quad \mid \mid \mathbf{x} \mid \mid = 1$$ 

This is a *constrained quadratic optimization* problem. The matrix $\mathbf{L}$ is a positive semi-definite matrix (that is, $\mathbf{x^TLx} >= 0$ for any $\mathbf{x}$. You can prove this from the non-matrix formula). The solutions to this problem are the eigenvectors of $\mathbf{L}$, denoted $\mathbf{u_1}, ..., \mathbf{u_n}$ ordered by their eigenvalues $\lambda_1 < ... < \lambda_n$.  Since $\mathbf{u_1}$ is $c\mathbf{1}$, then $\lambda_1=0$. 

Note, we are still talking about the problem of clustering the nodes of $G$ into 2 clusters.  The first eigenvector (or the first solution to our problem) does not give us any information because it has only value in the vector. The second, however, *in the ideal case* should have two values only.  That is, $u_{2i} \in \{h_1, h_2\}$ for some $h_1$ and $h_2$.  We never deal with ideal cases.  If you examine the values of $\mathbf{u_2}$, you will find that its elements are clustered into 2 clusters. What to do then? Use a clustering algorithm to find these clusters. Which clustering algorithm? the easiest probably; any *k-Means* algorithm ($k=2$ here). Now, we have a spectral clustering algorithm for $k=2$   

What should we do in the case of $k > 2$? This is quite interesting. So we said $\mathbf{u_1}$ have all its elements the same, and then we used $\mathbf{u_2}$ to obtain the 2 clusters solution.  You can create a $n \times 2$ matrix $[\mathbf{u_1}, \mathbf{u_2}]$ and then run the 2-Means algorithm on it.  (note: the $i^{th}$ row vector in the matrix $[\mathbf{u_1}, \mathbf{u_2}]$ is ${\mathbf{u_1}_i, \mathbf{u_2}_i}$).  You can generalize this for the case k = 3. Simply build the matrix $[\mathbf{u_1}, \mathbf{u_2}, \mathbf{u_3}]$ and run k-Means for $k=3$. Do the same for any $k$. It works. There are a few explanations to why it works. See [1] and [2] for references. 

We used the Laplacian of the graph $G$.  We could have used other known matrixes as well. For example, we can use the *normalized Laplacian* defined as: $$\mathbf{L}_{sym} = \mathbf{D}^{-1/2} \mathbf{L} \mathbf{D}^{-1/2}$$.   

Interestingly, $\mathbf{x^T}\mathbf{L_{sym}}\mathbf{x}$ is equal to $\frac{1}{2} \sum_{ij} w_{ij}(\frac{f_i}{\sqrt{d_i}} - \frac{f_j}{\sqrt{d_j}})^2$.  

This optimization problem was treated by different research communities. Ulrike von Luxburg has an interesting tutorial on this topic.  I recommend reading for more details.  I will try to prepare a toy example with python code soon. I hope you liked this post. 

Note: I learned this view of spectral clustering from a video of some brilliant researcher on YouTube a long time ago. I will add the link if I ever find it. It is definitely better than Andrew Ng's or Ulrike von Luxburg's paper. 

