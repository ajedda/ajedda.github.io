---
layout: post
title: "Fallback in static polymorphisim: an application of Concepts and  the Detection Idiom"
categories: algorithms
keywords: algorithms
---

One of the nicest features in Polymorphisim is that you do not need to override the behavior of a default virtual function. This saves you from repeating code. 

Let's start with an example. Let's assume we have a collection of workers of type WorkerA. At one point in the project, these workers handled messages of type MsgA. Later on, we introduced a new kind of workers of type WorkerB that handle messages of type MsgB. Using dynamic polymorphisim, we end up with this structure of code. 

```cpp
// Messages definitions
struct MsgA {}; 
struct MsgB {}; 
```

```cpp
struct WorkerA : public WorkerInterface
{
    void handle(MsgA const&) override { std::cout << "handle msgA \n"; }  
};

struct WorkerA : public WorkerInterface
{
    void handle(MsgB const&) override { std::cout << "handle msgB \n"; }  
};
```

At this point, WorkerA does not do anything with MsgB, so it will not implement a handler for MsgB. Similarly, WorkerB does not do anything with MsgA. Therefore, we define a default behaviour for these handlers in the interface WorkerInterface. 

```cpp
struct WorkerInterface 
{
    virtual void handle(MsgA const&) { std::cout << "default handle msgA\n"; }
    virtual void handle(MsgB const&) { std::cout << "default handle msgB\n"; }
}; 
```

But this does not come freely. First, we should use the instances of WorkerA and WorkerB as references, which can be annoying (you will have to add ```using WorkerInterface::process``` if not). Second, it may be annoying to inherit from the interface with an extra work ranging from adding few lines to creating wrapper classes. Third, some reviewers may just hate that extra indirection for performance reasons (even though this does not have that big of an impact, and can be zero in many cases).

The answer to these problems is usually static polymorphisim. How do we implement this nice default handlers in static polymorphisim. The answer is the detection idiom. Concepts will help us even more. Let me walk you through an example. 

Let's create our first concept for MsgA and MsgB classes. 

```
template <class T>
concept AcceptedMsg = std::is_same<MsgA, T>::value || std::is_same<MsgB, T>::value;
```

Now, we have some big class that will invoke the workers handler upon the reception of an accepted message. 

```
struct SomeClass
{
  WorkerA a1, a2; 
  WorkerB b1, b2; 
  
  // upon reception of an accepted message, call the handler 
  // each worker. 
  void upon_recv(AcceptedMsg const& msg); 
};
```

**The problem is** how should upon_recv be implemented? The first (incorrect) way to do that is: 

```
void SomeClass::upon_recv(AcceptedMsg const& msg)
{
    a1.handle(msg); 
    a2.handle(msg); 
    b1.handle(msg); 
    b2.handle(msg);
}
```

Unfortunately, this will not work. AcceptedMsg is either MsgA or MsgB. When upon_recv receives MsgA message, it fail compiling ```b1.handle(msg)``` and ```b2.handle(msg)```. If it received MsgB message, it fails at ```a1.handle(msg)``` and ```a2.handle(msg)```. 

We need to create a default behavior like the virtual functions in the first section of this blog. We are looking for something as below: 

```
void handle(WorkerA& a, MsgA const& msg) { a.handle(msg); }
void handle(WorkerA& a, MsgB const& msg) {}  // default behavior. 

void handle(WorkerB& b, MsgA const& msg) {}  // default behavior. 
void handle(WorkerB& b, MsgB const& msg) { b.handle(msg); }

void SomeClass::upon_recv(AcceptedMsg const& msg)
{
    handle(a1, msg); 
    handle(a2, msg); 
    handle(b1, msg); 
    handle(b2, msg);
}
```

Perfect. This works! Now imagine that you have WorkerC, WorkerD, WorkerE, and others. Some of the handle MsgA messages, some handle MsgB messages, and some handle none, or both. You will end up declaring two functions for each one of the new workers. Fortunately, we don't have to do this with templates. 

```
template <typename W> 
void handle(W& w, AcceptedMsg const& msg)
{
    // if W::handle(AcceptedMsg const&) exists: 
    //      w.handle(msg)
    // else:
    //      do nothing. 
}
```

**How to detect that a class has a function with a given signature?** We use the SFINAE magic. We can use std::experimental::is_detected. 

```cpp
template <typename W, typename M> 
using has_handle_t = decltype(std::declval<W>().handle(std::declval<M const&>())); 
```

You can use static_assert to test the behavior. 
```cpp
static_assert(std::experimental::is_detected<has_handle_t, WorkerA, MsgA>::value, 
             "Worker A has handler for MsgA");


static_assert(!std::experimental::is_detected<has_handle_t, WorkerA, MsgB>::value, 
             "Worker A has handler for MsgB");
```

Then we use the C++17 if-constexpr as:

```cpp
template <typename W, AcceptedMsg M> 
void handle(W& w, M const& msg)
{
    if constexpr (std::experimental::is_detected_v<has_handle_t, W, M>) 
    {
        w.handle(msg);
    }
    else 
    { 
        // default behaviour: 
        // do nothing (obviously, we don't need 
        // the else-statment here).    
    }
}
```

The if-constexpr is a C++17 feature. Without this feature, we would use std::enable_if as follows: 

```cpp
namespace std {
    // Just to remove the experimental. 
    using std::experimental::is_detected_v;
    
    // Easier reading. 
    template <bool B, typename T = void>
    using enable_if_not_t = std::enable_if_t<!B,T>; 
}
```

```cpp
template <typename W, 
          AcceptedMsg M, 
          std::enable_if_t<std::is_detected_v<has_handle_t, W, M>>* = nullptr>
void handle(W& w, M const& msg)
{
    w.handle(msg);    
}

template <typename W, 
          AcceptedMsg M, 
          std::enable_if_not_t<std::is_detected_v<has_handle_t, W, M>>* = nullptr>
void handle(W&, M const&)
{
    // default behavior. Do nothing. 
}
```

**Does that sell static polymorphisim to you?** If not, then I did not a good job. The C++ standard committee can do a better job. They have been trying to convince users to use this technique and similar ones by: 
* Advocating the use of concepts. This is true even before Concepts are standardized. For example, the STL use the terms FwdIterator and RandomIterator and others. 

* Introducing features that facilitates writing compile-time code such as: constexpr, if-constexpr, std::declval, std::enable_if, std::experimental::is_detected, type deduction, and others. 


Yet, these features are still not widely popular in the day-to-day C++ programmer life. There is a high chance that a code that uses these features will be rejected because it is "harder to read", or because it is not easy to find developers who can maintain it. As a result, dynamic polymorphisim can be preferred to static polymorphisim, or macros  to templates (at least in my experience). 



Personally, I will go with static polymorphisim whenever it makes more sense (e.g., when dealing with tuples, or when the number of objects we are dealing is known in advance and can be manually controlled).  And with concepts becoming more popular, it will make sense more often. 












