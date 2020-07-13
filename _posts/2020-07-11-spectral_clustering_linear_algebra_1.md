---
layout: post
title: "Spectral Clustering: the optimization problem"
categories: algorithms
keywords: algorithms; Machine Learning
---

Let's speak about *spectral clustering*, or the optimization problem of spectral clustering.  

We have a graph $G=(V,E)$.  The edge linking any two nodes $v_i$ and $v_j$ has a non-negative weight $w_{ij}$ that reprsents the *similiarity* between the two nodes. A $w_{ij} = 0$ means that $v_i$ and $v_j$ are infinitley disimiliar. 

We want to cluster the nodes of the graph into $m$ clusters.  We will say that $v_i$ is in $m^{th}$ cluster if $x_i = c_m$ for some constant $c_m$. The vector, therefore, $\mathbf{x}$ is a representation of our clusters. 

Let's assume we have a function $g$ such that $g(x_i, x_j)$ is 0 if $v_i$ and $v_j$ are in the same cluster (or, $x_i = x_j$), and 1 otherwise. 

Let's consider the simple case where $k=2$. One way to cluster the nodes of $G$ is by putting similar nodes in the same cluster. As a result, we want dissimilar nodes to be in different clusters. Dissimilar nodes are connected by edges with relatively low weights. The optimization problem: 

$$\min_{\mathbf{x}} \sum _{i,j} w_{ij} g(x_i,x_j)$$  

gives clustering solutions (i.e., different values of $\mathbf{x}$). This clustering is a *cut* in the graph $G$ that have a minimum weight (or simply, *minimum cut* problem). This objective function works fine even for the case $k > 2$. 

If all nodes are in the same cluster, then the result of that objective function is minimized to zero (it can't be less), so we are looking for solutions other than this trivial one. 

One way to write $g$ is 

$$g(x_i, x_j) = \max (1, \mid x_i - x_j \mid)$$

This is not easy to deal with mathematically. So, *how to solve this optimization problem?* Let's do some relaxation on this function. 

Again, let's take a simpler clustering problem. We would like to cluster our data into 2 clusters only. This allows us to restrict $x_i$ to be either 0 or 1 (in the first cluster or second cluster).   Now, $\mid x_i - x_j\mid$ is always either 0 or 1. This means that: $$g(x_i, x_j) = \max(1, \mid x_i - x_j \mid)=\mid x_i - x_j \mid$$. 

Who likes dealing with absolute values when solving such optimization problems? Let's relax $\mid x_i - x_j \mid$ to something easier to deal with such as $(x_i - x_j)^2$.  They lead to exactly the same results since $x_i$ is either 0 or 1.  So now, let's optimize this function: 

$$ \min_{\mathbf{x}} \sum _{i,j} w_{ij} (x_i - x_j)^2$$

Let's write this in the matrix format. 

$$\sum_{ij} w_{ij} (x_i - x_j)^{2} \\= \sum_{ij} w_{ij} (x_{i}^2 - 2x_i x_j + x_{j}^2)\\=\sum_{ij} w_{ij} x_{i}^2 + \sum_{ij} w_{ij}x_j^2 - 2\sum_{ij}  w_{ij} x_i^2 x_j^2$$

There are three parts in that equation (or actually, only two parts): 

 1. *Part 1:* $\sum_{ij} w_{ij} x_i^2 = \sum_i x_i^2 \sum_j w_{ij}$
 2. *Part 2:* $\sum_{ij} w_{ij} x_j^2 = \sum_j x_j^2 \sum_i w_{ij}$ 
 3. *Part 3:* $2\sum_{ij}  w_{ij} x_i^2 x_j^2$. 

Part 1 and Part 2 are essentially the same. There is an interesting term there; $\sum_j w_{ij}$. We will denote it $d_i$. This is the sum of all weights node $v_i$, or its *degree*.  

We can define the diagonal matrix $\mathbf{D}$ where $\mathbf{D}_{ij} = 0$ for $i \neq j$ and $d_i$ otherwise (or also denoted as $\mathbf{D} = \mathrm{diag}(d_i, ..., d_n)$) .  This allows us to write the term $$ \sum_i x_i^2 \sum_j w_{ij} = \sum_i x_i ^2 d_i = \mathbf{x}^T\mathbf{Dx}$$

Regarding the third part, we can re-write it in matrix format as $\mathbf{x}^T\mathbf{Wx}$.  We end up with 

$$2 (\mathbf{x}^T\mathbf{Dx} - \mathbf{x}^T\mathbf{Wx}) = \ 2(\mathbf{x}^T(\mathbf{D}-\mathbf{W})\mathbf{x}) $$

The matrix $\mathbf{D-W}$ is a known matrix that we call the *Laplacian* of the graph $G$, denoted $\mathbf{L}$.  

Great, our optimization problem is: 

$$\min_{x} 2 \mathbf{x}^T\mathbf{Lx}$$ 

or simply, 

$$\min_{x} \mathbf{x}^T\mathbf{Lx}$$ 

.. since that constant 2 will not change the optimal solution. 

This is an *integer programming* problem. We want to relax this restriction and allow $\mathbf{x}$ to be a vector of real numbers. 

There is an obvious solution to this problem, $\mathbf{x}=\mathbf{0}$.  Another solution is $\mathbf{x}=\mathbf{1}$, or any vector with all equal elements. These all represent the trivial solution that we wanted to avoid at the beginning of this problem description. These all have the same direction, but different values. Therefore, let's add a constraint to our problem. We want $\mid \mid \mathbf{x} \mid \mid = 1$.  So now we have 

$$\min_{x} \mathbf{x}^T\mathbf{Lx} \\  \mathrm{s.t.} \quad \mid \mid \mathbf{x} \mid \mid = 1$$ 

Great. We were able to relax our problem to a *constrained quadratic optimization* problem. The matrix $\mathbf{L}$ is a positive semi-definite matrix (that is, $\mathbf{x}^T\mathbf{Lx} >= 0$ for any $\mathbf{x}$. You can prove this from the non-matrix formula). Therefore, the solutions to this problem are the eigenvectors of $\mathbf{L}$, denoted $\mathbf{u_1}, ..., \mathbf{u_n}$ with eigenvalues $\lambda_1 < ... < \lambda_n$.  Since $\mathbf{u_1}$ is $\sqrt{n} \mathbf{1}$, then $\lambda_1=0$. 

Note, we are still talking about the problem of clustering the nodes of $G$ into 2 clusters.  The first eigenvector (or the first solution to our problem) does not give us any information because it has only value in the vector. The second, however, *in the ideal case* should have two values only.  That is, $u_{2i} \in \\{h_1, h_2\\}$ for some $h_1$ and $h_2$.  We never deal with ideal cases.  If you examine the values of $\mathbf{u_2}$, you will find that its elements are clustered into 2 clusters. What to do then? Use a clustering algorithm to find these clusters. Which clustering algorithm? the easiest probably; any *k-Means* algorithm ($k=2$ here). Now, we have a spectral clustering algorithm for $k=2$   

*What should we do in the case of $k > 2$?* This is quite interesting. So we said $\mathbf{u_1}$ have all its elements the same, and then we used $\mathbf{u_2}$ to obtain the 2 clusters solution.  You can create a $n \times 2$ matrix $[\mathbf{u_1}, \mathbf{u_2}]$ and then run the 2-Means algorithm on it.  (note: the $i^{th}$ row vector in the matrix $[\mathbf{u_1}, \mathbf{u_2}]$ is ${\mathbf{u_1}_i, \mathbf{u_2}_i}$).  You can generalize this for the case k = 3. Simply build the matrix $[\mathbf{u_1}, \mathbf{u_2}, \mathbf{u_3}]$ and run k-Means for $k=3$. Do the same for any $k$. It works. There are a few explanations to why it works. See [1] and [2] for references. 

We used the Laplacian of the graph $G$.  We could have used other known matrixes as well. For example, we can use the *normalized Laplacian* defined as: $$\mathbf{L}_{sym} = \mathbf{D}^{-1/2} \mathbf{L} \mathbf{D}^{-1/2}$$.   You will note that $\mathbf{x^T}\mathbf{L_{sym}}\mathbf{x}$ is equal to $\frac{1}{2} \sum_{ij} w_{ij}(\frac{f_i}{\sqrt{d_i}} - \frac{f_j}{\sqrt{d_j}})^2$.  


This optimization problem was treated by different research communities. Ulrike von Luxburg has an interesting tutorial on this topic. The literature, however, may confuse you. Some researchers do not use the weight or similarity between the edges, but instead the distance between them. This can be defined as $1/w_{ij}$. This will change the optimization problem to a maximization instead of minimization. Will it still work? Yes, it will and that's because of the eigenvector properties of the Laplacian. 

This was just a simple explanation of spectral clustering. I find some tutorials asking you to imagine the eigenvectors of the graph $G$. This is just too much to ask for.  I hope you liked this post. I will try to prepare a toy example with python code soon. 




