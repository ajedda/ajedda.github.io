---
layout: post
title: "Fun with step functions: grouping by the system state (in Pandas and SQL)"
categories: algorithms
keywords: algorithms
---


Our system may be moving between many states. It may be in state $S_1$ at time $t_i$, then goes to to state $S_2$ at time $t_j$, then back to state $S_1$ at $t_k$, etc... .  I  frequently find myself dealing with the following question: how did my system behaves while being in a given state.  There are two ways to look at this question: 

 1. How did the system behave during state $S_i$ in total? For example, how much time was spent in state $S_i$ in total. The answer is easy; $(t_j - t_i)$ + $(t_k - t_j)$.  The implementation in Pandas and SQL is quite simple as well. If you have a column called *state* (and we should assume that this is true throughout this post), then we should just use ```group by state``` in our query. 
 
 2. How did the system behave during the $m^th$ time it entered state $S_i$? For example, the system in the example above spent the period $[t_i, t_j]$ in $S_1$, then the period $[t_j, t_k]$ also in the same state. In this version of the problem, we consider every time the system enters the $S_i$ as different state  $S_{i1}$, $S_{i2}$, $\dots$. 


**Let's focus on problem 2.** We can look at the system transition between the states as a *step function*. The state $S_i$ is represented by $i$ in that step function. For example, the time series in the following example illustrates the system's state in each second. 

```python
states = [0,0,1,1,0,1,0,0]
df = pd.DataFrame({'ts'   : np.arange(len(states))
                   'state': states})
```

What we are looking for is: 

```python
df['exp'] = [0,0,1,1,2,3,4,4] 
```

This is useful because we can group by each state the system got in by using ```df.groupby('exp')``` or ```df.groupby(['exp','state'])```. This solves our problem. 

**Reaching the expected solution** requires one essential step; to know when the system entered a *different* state. This is a simple step: 

```python 
def is_different(ds): 
    return ds.diff().fillna(0).apply(lambda x: x if x == 0 else 1))
```

Function *is_different_* applied to the a data series with values ```[0,0,1,1,0,1,0,0]``` will return ```[0,0,1,0,1,1,1,0]```. That is, the system state changed at seconds 3,5,6, and 7. 

The cumulative sum of the column ```[0,0,1,0,1,1,1,0]``` (i.e., the result of *is_different*) is ```[0,0,1,1,2,3,4,4]```.  **This is our expected result!** Note now that this will work even if our system has many states $0,1,\dots,k$ because we only care about the the time at which the system changed its state.

**What if our system was represented as a union of states?** That is, we can enter state $s_{1j}$ and $s_{2j}$ at the same time. The union of both of these states can be represented as some state $s$.  

```python
states_a = [0,0,1,1,0,1,0,0]
states_b = [0,1,1,1,1,2,2,2]
states_c = [0,2,0,2,2,2,2,0]

df = pd.DataFrame({'ts'      : np.arange(len(states_a)),
                   'states_a': states_a, 
                   'states_b': states_b, 
                   'states_c': states_c})
```

We can solve this problem with the previous solution after converting *states_a*, *states_b*, and *states_c* to one single state *states*. One way to achieve this is: 

```python
df['states'] = 1 * df.states_a + 2 * df.states_b + 4 * df.states_c
# You don't need to restrict yourself with the powers of 2. 
```

We just need to apply the previous solution on *df.states*. 

The final solution looks something like this: 

```python
def gen_multi_states_test_df():
    df3 = pd.DataFrame ({'a':[0,0,0,0,0,0,1],
                     'b':[0,0,1,1,0,0,2], 
                     'c':[1,1,1,1,1,1,1]})

    df3 = (df3
           .assign(v = np.random.uniform(0, 100, size=df3.shape[0]))
           .assign(p = np.arange(df3.shape[0]))
          )
    return df3 

def get_hash(r):
    return r['a'] + 2*r['b'] + 4*r['c']

def find_sub_id_diff(df, field):
    df.loc[:, field] = (df
                .apply(get_hash, axis=1)
                .diff()
                .fillna(0)
                .apply(lambda x: 1 if x != 0 else 0)
                .cumsum())
    return df 

df3_inp = gen_multi_states_test_df()
df3 = find_sub_id_diff(df=df3_inp, field='state')
```

**How to solve this in SQL?** We will use SQL windows. I will use SQLite here. I tried a similar solution with PostrgreSQL and GreenPlum. We can just re-write all the steps above using SQL, or we can be a little bit smarter and using the power of SQL. 

```sql 
-- ----------------------------------------------------
-- Method 1: Just re-do all the steps we did in Pandas. 
-- ---------------------------------------------------
with t1 as
( 
    select *, (a + 2*b + 3*c) as h_state
    from df5
), 
t2 as
( 
    select *,  
        ifnull(h_state != lag(h_state, 1) over (order by p), 0) as is_different
        -- We are using window here. 
        -- You can use partition in the windows if needed.
        -- The ifnull is similar to fillna(0) in our pandas.
    from t1
),
t3 as 
(
    select *, sum(is_different) over (order by p) as sub_id
    from t2
)
select * from t3
```

We can use less code in SQL as we show in the other method below: 
```sql
-- ----------------------------------------------------
-- Method 2: Use less code. 
-- --------------------------------------------------- 
with t1 as
( 
    select *, 
        ifnull((a != lag(a,1) over  w) | (b != lag(b, 1) over  w) | (c != lag(c, 1) over w), 0) as is_different
    from df5
    window w as (order by p)
), 
t2 as 
( 
    select *, sum(is_different) over (order by p) as sub_id 
    from t1 
)
select * from t2
```

**Side note:** Why we don't have an equivalent to Method 2 above in Pandas? Shouldn't this lead to less code as well? Yes, but the performance will be worse. Actually, one of the main reason that made me look into different solutions for this problem is the terrible performance of Pandas when applying a function on columns (or all values within a row). It is much faster to convert multiple columns into a single column, and apply a function on that new column.  

Here is a very simple experiment to show this performance difference. This definitely is not a scientific performance comparison, but it should give you an idea. We will compare ```find_sub_id_diff``` above with function ```find_sub_id_iterrows``` below. 

```python 
class IsDifferentSubseqRow(object): 
    """ This checks if two subsequent rows are different """
    
    def __init__(self): 
        self.prev_x = None

    def __call__(self, x): 
        if (self.prev_x is None) or (x != self.prev_x).any(): 
            self.prev_x = x 
            # if you want to keep track of how many times 
            # the rows changed, just add a new member variable 
            # initialized to 0 and increment it every time 
            # the state changes. 
            # self.new_state_id += 1  
        return  self.sum

    
def find_sub_id_iterrows(df, field):
    is_diff = IsDifferentSubseqRow()
    for i,row in df.iterrows(): 
        df.at[i, field] = is_diff(row[['a', 'b', 'c']])
    return df
 
      
df7 = gen_multi_states_test_df() 
res1_df7 = find_sub_id_iterrows(df=df7, field='state')

```
**Side note:** why not using rolling/apply here? unfortunately, rolling/apply accepts only data series (or numpy array). For example, the statement: 

```python
df8_inp.rolling(2,1).apply(lambda ds: ds.iloc[0]['a'] , raw=False)
```

will generate this error: 

```python
<ipython-input-31-5185e49637e7> in <lambda>(ds)
----> 1 df8_inp.rolling(2,1).apply(lambda ds: ds.iloc[0]['a'] , raw=False)

IndexError: invalid index to scalar variable.
```

because _ds_ is a data series for each column. 


**Back to the problem:** Comparing the two functions on the same input, I find that ```find_sub_id_diff``` is nearly 5 times faster than ```find_sub_id_iterrows```. 


Anyway, this was just some fun with step functions. The next post will have more fun things we can do with these functions. Mastering these techniques showed up quite handy for me. Queries that took hours now take just few seconds using a powerful database engine. **This was just a random idea that I thought it may be cool to share.**

