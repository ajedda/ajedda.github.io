import functools

@functools.lru_cache
def execute_big_func(): 
  print("Executing a very big func")
  return 42 

class S(object): 
  def __init__(self): 
    pass 

  def is_valid(self, d): 
    s = execute_big_func()
    return d != s 

  def calc(self, d): 
    if not self.is_valid(d): 
      print("not valid")

    print("Other business logic")

s = S() 
s.calc(30)
s.calc(42)
s.calc(52)
s.calc(60)
s2 = S() 
s2.calc(30)
s2.calc(52)

print(" ------------ ")

# Can we do the same example with cached_property
def get_the_s(): 
  print("Running a long function")
  return 42 

class D(object): 
  def __init__(self, i): 
    self.i = i

  @functools.cached_property
  #@property
  def the_s(self): 
    return get_the_s() 

  def is_valid(self, d): 
    return d != self.the_s

  def calc(self, d): 
    if not self.is_valid(d): 
      print("not valid")

    print("Other business logic")
    # print(f"calc({d})") 

d1 = D(10)
d1.the_s
d2 = D(20)
d2.the_s 
d1.the_s

"""
d = D(10) 
d.calc(30)
d.calc(42)
d.calc(52)
d.calc(60)
d1 = D(20) 
d.calc(30)
d.calc(52)
""" 


