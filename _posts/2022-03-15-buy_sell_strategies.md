---
layout: post
title: "How does a psychic trade stocks?"
categories: Algorithms
keywords: programming; Python; Algorithms
---

Assume you are getting future prices of a stock. You are asked to trade such that your profits are maximized. You should sell high and buy low. You probably want to do this many times as well. What algorithms (or rules) to follow? 

You have two states. A state when you have no stocks in your portfolio, so you can either buy or do nothing. Another state is when you have stocks in your portfolio, hence you can sell (and realize profits) or do nothing. The stocks you don't sell at the end are considered a loss. 

We can formulate this as a recursive function. Let me write it in as a code. 

```python 
def simple_trade(prices):
    def _simple_trade(p, t):
        
        # Base case: time is out of our scope.  
        if t >= len(prices): 
            return -1 * p
        
        # p == 0: can either buy or do nothing. 
        if p == 0: 
            v1 = _simple_trade(prices[t], t+1)   # buy: p is set to prices[t]
            v2 = _simple_trade(0, t+1)           # do nothing: update t only.  
            
            # We are interested in the action that gives us the highest return. 
            v = max(v1, v2) 
            return v
        else: 
            v1 = (prices[t] - p) + _simple_trade(0, t+1)  # sell: (prices[t] - p) is the profit of this step. 
                                                          #       _simple_trade(0, t+1) is the profit of the next step.
            v2 = _simple_trade(p, t+1)                    # do nothing: update t only. 
            
            # Same as before, we are interested in the action that gives us the highest return.  
            v = max(v1, v2)
            return v
        
    
    # We start with an empty portfolio and time zero. 
    return _simple_trade(0, 0)
    
```


Note that if `_simple_trade(p, t)` will return the same result for every time its input is `p` and `t`. We can therefore make `_simple_trade` more efficient by caching.  Our cache key will be the pair `(p, t)`. We can call the pair `(p, t)` as the *state* of `_simple_trade`. 



```python 
def memoized_rec_trade(prices):
    G = dict() 
    def _memoized_rec_trade(p, t): 
        if i >= len(prices): return -1 * p
        if (p,t) in G:       return G[(p,t)]

        if p == 0: 
            a1 = _memoized_rec_trade(prices[t], t+1)
            a2 = _memoized_rec_trade(0, t+1)
            v = max(a1, a2)  
            G[(p,t)] = v
            return v 
        else: 
            a1 = (prices[t] - p) + _memoized_rec_trade(0, t+1)
            a2 = _memoized_rec_trade(p, t+1)
            v = max(a1, a2)
            G[(p,t)] = v
            # The way to read this: 
            # There are two actions to follow when in state (p, t). a1 to sell, and a2 to hold. 
            # We G[(p, t)] is the profit returned by taking the best action from a1 and a2. 
            # Which action have we taken? Unfortunatly, it is not recorded here. 
            # We can use an dict of actions (call it, A) in order to best action from state (p, t).   
            # This best actions dictionary will be used in the next example. 
            return v
        
        
    return _memoized_rec_trade(0, 0)
```

 

`G` is my cache. This cache is the cause of a significant improvement in execution time. Although it is a dictionary, I am calling it `G` here because it is a graph; a graph of *states* that is built on the fly.  

Let's generalize this solution a bit further. 

```python
def gen_actions_trades(prices):
    G = dict() 
    A = dict() 
    
    def _get_actions(p,t):
        if p == 0: 
            actions = [(0, (prices[t], t+1)),     # (r, next_state)
                       (0, (0, t+1)),
                      ]
        """
        The way to read this: 
        When in state (p=0, t), you can have two actions; a1) buy, a2) do nothing. 
        Each action has a reward and leads to another state. In the case of a1 (buy), 
            the reward is zero because we haven't sold anything. 
            the next state is (prices[t], t+1), because portfolio has a stock with value set to prices[t] (or, current price) and t+1 
            because we want to evaluate the next time price. 
        """
        else: 
            actions = [((prices[t]-p), (0, t+1)),  # (r, next_state)
                       (0, (p, t+1)),
                      ]
        """
        The two actions here are a1) sell, a2) hold. 
        In action a1 (sell), the reward is prices[t] (i.e., current price) minus the price we bought at which is p. 
        The next state is (0, t+1) because the portfolio is empty now. 
        """
        return actions 
        
    def _gen_actions_trades(p, t):
       
        s = (p,t)
        if t >= len(prices): return -1 * p 
        if s in G: return G[s]
        

        actions = _get_actions(*s)  
        v, a = max_evaulated_action(_gen_actions_trades, actions)
        G[s] = v     
        A[s] = a     # to store the best action. 
        return v
        
        
    return _gen_actions_trades(0, 0), G, A


"""
return the action with maximum value. 
The output of this function is a pair (value, action)
"""
def max_evaulated_action(f, rs_list):
    return max([(r + f(*s), s) for r,s in rs_list]) 
        
    


res, G, A = gen_actions_trades(prices)
optimal_A = backtrack_actions(A, (0,0))   # How would you write this? 
```


Many brokers still charge you for each trade you make. How can you include the trading cost in the algorithm above? We just need to change the `_get_actions` function as follows: 

```python 
    def _get_actions(p,t):
        if p == 0: 
            actions = [(-1 * trade_cost, (prices[t], t+1)),   # buy: 
                       (0,               (0, t+1)),           # do nothing.
                      ]

        else: 
            actions = [((prices[t]-p) - trade_cost, (0, t+1)),  # sell
                       (0,                          (p, t+1)),  # hold.
                      ]
            
        return actions 


```

Do you want extra constraints? Ok. We started with the assumption that we have no stocks in the portfolio, but enough cash to buy the stock at any price. In reality, we would have an initial cash budget. How would you change the algorithm above to add this constraint? 


Why all of that? For two reasons: 
1. The idea of caching/memorization. Caching is a technique widely used, especially when teaching *dynamic programming*. Many students (and experienced software developers) wrongfully think that caching used with recursive functions  _is_ dynamic programming. I will explain how this is not true in future posts. This implementation of caching is not taught in undergraduate dynamic programming classes. Instead, we usually estimate the size of the caching graph and fill it in as we run the algorithm. Here, we are constructing the graph on the fly. Nothing new here. It is just that the method I use in this post is more generalizable. 

2. The idea of _actions_ and _rewards_. The reward in the algorithm above was the profit made in cash. The actions were either buy, sell, or do nothing. The terms are very suitable for this specific problem. However, these are common terms in _reinforcment learning_. Dynamic programming is also an approach used in reinforcement learning. I feel that teaching dynamic programming as it is taught in a reinforcement learning class is much more beneficial to the students compared to the method followed in algorithms classes based on Cormen et al.'s classical Introduction to Algorithms class. I am hoping to talk about dynamic programming using these terms in the next posts. My idea: graph algorithms are what should be thought of first. Most algorithms (including dynamic programming algorithms) are nothing but derivatives of graph algorithms. Yet, graph algorithms are taught very late in algorithms classes, and graphs are taught as *data structures* more than *relations*. No need to think about this now. My following posts should be around this topic. 

3. I forgot to mention a third reason for this post. It has a cool application; stock prices trend following. Next post (I hope). 