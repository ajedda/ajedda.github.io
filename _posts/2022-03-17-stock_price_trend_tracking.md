---
layout: post
title: "Stocks prices trend tracking"
categories: Algorithms
keywords: programming; Python; Algorithms
---

 
The psychic trading algorithm I introduced in the previous post has a cute application.it can serves as a trend tracking algorithm for stock prices. What does that mean? 

When studying a stock price time series, we may want to split that time series into a set of connected trend lines. It is not an easy problem. Common algorithms for this problem usually require human intervention in different phases of the algorithm execution. I am not claiming I am proposing a solution. I just want to share a nice observation; an application of the psychic trading algorithm for drawing trend lines. Let's have a look at some examples of what I mean here. 



<style>
.row {
  display: flex;
  flex-wrap: wrap;
  padding: 0 4px;
}

/* Create two equal columns that sits next to each other */
.column {
  flex: 30%;
  padding: 0 4px;

}


.column img {
  margin-top: 8px;
  vertical-align: middle;
}

</style>



<div class="row">
  <div class="column">
    <img src="/assets/images/trend_tracking/example_5.png">
    <img src="/assets/images/trend_tracking/example_6.png">
  </div>
  <div class="column">
    <img src="/assets/images/trend_tracking/example_7.png">
    <img src="/assets/images/trend_tracking/example_8.png">
  </div>
 <div class="column">
    <img src="/assets/images/trend_tracking/example_9.png">
    <img src="/assets/images/trend_tracking/example_10.png">
  </div>
</div>



The blue line is a price time series generated as a random walk. The red line, or lines, are the trend lines. The trend lines we generate are controlled by a parameter that I will talk about later. Here is an example of the impact of this paramter. This parameter is nothing but the _trading fee_. 

<div class="row">
  <div class="column">
    <img src="/assets/images/trend_tracking/small_example_1.png">
    <img src="/assets/images/trend_tracking/small_example_2.png">
  </div>
</div>



Here is how it goes. Let's add some more constraints to the psychic trading algorithm; like the trading fees. Many brokers still charge you for each trade you make. How can you include the trading fees in the algorithm? We just need to change the `_get_actions` function as follows: 

```python 
def _get_actions(p,t):
    if p == 0: 
        actions = [(-1 * trade_fee, (prices[t], t+1)),   # buy: 
                    (0,             (0, t+1))]           # do nothing.
               
    else: 
        actions = [((prices[t]-p) - trade_fee, (0, t+1)),  # sell
                    (0,                        (p, t+1))]  # hold.
                
        
    return actions 
```

 That `trade_free` is a parameter that will control how trend line reacts  to price movements. The lower the trade fee, the more it will react. The higher the trading fee, the less it will react (i.e., if the price movement is not big, the algorithm will not buy or sell because the benefit of that transaction is lost in the fees). 

 


**How do we get the trend lines from the algorithm actions?**

First, we need to backtrack the actions that led to the best solution. 

```python 
res, G, A = trade_with_fees(prices, trade_fee)
A = backtrack_actions(A, (0,0))
```

`trade_with_fees` is the algorithm with the trading fees constraint. `res` is the value of the best solution, `G` is the  graph of best value, and `A` is the graph of the best actions. That is, `G[s]` is the best value if we start from state `s`, and `A[s]` is the best action taken from that state. `res` therefore is `G[(0,0)]` because initial state is `(0,0)`.  We can use `A` to know the exact actions taken by the algorithm. This is what `backtrack_actions` does. 

```python
def backtrack_actions(A, s):
    res = [s]
    next_s = s 
    while next_s in A: 
        res.append(A[next_s])
        next_s = A[next_s]
    return res 
```

The result of `backtrack_actions` can be something like this: 

```python
[(0, 0), 
 (100, 1), (100, 2), (100, 3), (100, 4), (100, 5), (100, 6), 
 (0, 7), (0, 8), (0, 9), (0, 10), (0, 11), (0, 12), (0, 13), (0, 14), (0, 15), ...
```

We need to find the times we are in a selling or buying state. Let's take a simpler example. 

```python
# (p, t): t is time, p is zero if portfolio empty, and 1 otherwise.
[(0, 0), (1, 1), (1, 2), (1, 3), (0, 4), (0, 5), (0, 6), (1, 7), (8, 0)]
```

We start with an empty portfolio. We are ready to buy. We bought at t=1 because of the state (1, 1). Therefore, from t=0 to t=0, we were in ready-to-buy state. 

We entered the ready-to-sell state at t=0 because we bougt at that time. We exited this state at state (1, 3). Why? because the state that comes after is (p=0, t=4). Hence, we sold at t=3. In summary, from t=0 to t=0 (or simply [0,0]) is ready-to-buy state. [0,3] is ready-to-sell, [3,6] ready-to-buy, [6,7] ready-to-sell, and [7,9] is ready-to-buy.  Therefore, we need to find the sequence of indices [0,0,3,6,7,9]. The algorithm to do this is given below.


```python    
def split_states(states): 
    s = 0 
    index = 0 
    
    res = [s]
    while True: 
        new_index = find_last_in_first_seq(states, index, lambda x: (x[0]>0)==s)
        res.append(new_index)
        if new_index >= len(states): 
            break 
        s = 1 if s == 0 else 0 
        # s = (s+1) % 2
        index = new_index 
    
    return res

```

.. where `find_last_in_first_seq` is defined as follows: 

```python
def find_first(A, i, predicate):
    for j in range(i, len(A)): 
        if predicate(A[j]):
            return j 
    return len(A)
        
        
        
def find_last_in_first_seq(A, i, predicate):
    new_i = find_first(A, i, predicate)
    
    j = find_first(A, new_i, lambda x: not predicate(x))
    if j!=len(A): return j-1
    else:         return j

```


**Bonus point:** _Do you want extra constraints?_ Ok. We started with the assumption that we have no stocks in the portfolio, but enough cash to buy the stock at any price. In reality, we would have an initial cash budget. How would you change the algorithm above to add this constraint?