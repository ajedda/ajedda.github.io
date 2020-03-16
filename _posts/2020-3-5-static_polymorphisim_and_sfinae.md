---
layout: post
title: "Static Polymorphisim and SFINAE: an Application"
categories: algorithms
keywords: algorithms
published: false
---



As software developers, we start with an initial set of requirements, we build an architecture that meets all of these requirements using some abstractions. We then keep getting many requirements, and add (rarely remove) code. Our architecture get messy.  In Linus Torvalds words: 

> inefficient abstracted programming models where two years down the road you notice that some abstraction wasn't very efficient, but now all 
   your code depends on all the nice object models around it, and you cannot fix it without rewriting your app.


I don't like Linus's ideas. I don't like re-writing my App. I like refactoring, but I like doing it in stages. I see one easy to fix problem in the code. I fix it. Then, I have another look and repeat until I am happy with what I have. I will share one situation I had in a project I worked on. 

The initial architecture we are dealing with is as follows: 

![enter image description here](https://lh3.googleusercontent.com/kPfp6IbVLPExFiRWDozngL__A_T2DajR2KnN6e9ve9_RDje_u7j7WH8Jub4y_cMpwpfrK0KGz8Yc0k-CTdaJfd7K3Xgub87Td4bz0eA9abAV8H4UpnY1-wuajc2gQnPQKFCIRQTriSrvhxW1mN8PMwNnZpsq0o3SOHD77dXsEhyqEdIAHHx7gEcYGzOBsUWUtuI7Igs2_3efVoXYQNEIdlWuMLv4ZJYDCYw_7ROWiEutHlRrTe1sqUVicK3LXKgch-mT8dslEe0U_RASz4vfILvBGxXoHs3FoIgDXcFxUNrVcf359ozBmU_1W1Wf36Dv5JT2V_yWITctSO7Ribt-dPE4ZyneiHIdb77byv3B0yq6ar_nmv1tQLO7B3jzrDNUVGPa-EUkBRcRuc0kJmlsGXbMcFL-JFvU9g9ccEYCzoMOdK1ohgB7_MJD1CD333SRErDuC6JGHDWZhH7-RECEy73JMpZwvY_szE1skAvZrrPTExPjqh4zELh6FTPlbaEwmLDP61mhNQYS4bho0X57RICOcrshGSRfUJBdXT5e38tTYLhcD5TDKcJ_MYWiA-4qANAqhAIgULqCYqhpj4HHI_wDVLQvGMl4BmpjK3kKjO1z0MqHKa_FbmBiaUmNfMYq9NkAUZ2Zq9CRb0zebuhVo1S2E7aIp6WHV66jG0Fg6K77CSgO35yovMA=w1167-h226-no)


A message of type A is received by function *recv*.  It pass then by a pipeline of SomeBigClass instance, ProcessA instance, and some subprocesses SubprocessA1, SubprocessA2 ... . 


![enter image description here](https://lh3.googleusercontent.com/N4X0LMF725CENSUWzMNVrVqJF5B7PZmFmwb0vyzmKgXB86SjenQcArfqeZESi9Lezx-IkdaGrzObAiPOeg7zqL7NtVq9qofOXbH_j2jNrRbGmJ0oMDNqJw4WdPYeNlm-iogoSDvK_G5KwXNIRNs-DcrSNo4egulB6fIybwzOvYYULhwvQ1NOqSQ9SUZsVugaWXqHTiIqw3oevXCHjfYfN0JuJ1M-dVMDcGdWO2vw_9l6iAbh6R8ota89AfthoDyQ9mJB864vAm4RKeke_CfypB46jCFL3jtoNNq1cpgMXRiWYUe-jNjk1QWZT6h_eoAJYt_yLcYNvYs9Mp7IgpdY0d8Eu8dZsez92BF4uJfQJSXJUcMYjDcFFKDxIllJGW5PEGzHpEY7DFVQQKPJRlTjKmUsPlRK5dbxnl1USVBpsSRMquL5GQahIF2QMQCUr11bNrJQYuhbVmDZqXAhl6MZG5H0ow_il-o87lxH3I2QEx_VqRZV1VSqlkKRpoAu31WaFr9z1RAb0IUE6W4UA892eIex1SJwphmyr1eOdJVz4NPCQYXRVjCUCrZW_FbwG8HvPuzJWhjuwqMZfT0LnLjTJHB0QCZkGAusqwqw9WMgowpdQf7nahMCaRKfPOiOIrq16G_WmX0EZTIjRytfePkJ-0OXVvEFA3ThDmF1bSF-wefah5AdU3BV750=w1296-h462-no)

Then, we have a new message of type MsgB.  We create a concept of AcceptedMsg for MsgA or MsgB.  Function *recv* takes an instance of the concept AcceptedMsg.  SomeBigClass will have two instances; one for ProcessA, and one for ProcessB.  ProcessA will pass it by its subprocesses, whereas ProcessB will pass it by its subprocesses. **However, ProcessA and ProcessB have some common subprocesses!** 

This led to a situation where our code looked something like this (very abstractly of course). 

```cpp
// The messages and concept definition.  
struct MsgA {}; 
struct MsgB {}; 

template <class T>
concept AcceptedMsg = std::is_same<MsgA, T>::value || std::is_same<MsgB, T>::value;
```

Function *recv* is defined as follows:
```cpp
void recv(SomeBigClass& sbc, AcceptedMsg const& msg)
{
    sbc.process(msg); 
} 
```

.. and SomeBigClass is defined as: 
```cpp
struct SomeBigClass
{
    ProcessA pa;
    ProcessB pb;
    void process(MsgA const& m) { pa.process(m); } 
    void process(MsgB const& m) { pb.process(m); }
};
```

The common subprocesses of ProcessA and ProcessB are handled by a ProcessBase class. 
```cpp
struct ProcessBase 
{
    void process(AcceptedMsg const& m) { sp_com1.handle(m); } 
    SP_Com1 sp_com1;
}; 
```

... where *SP_Com1* have a handle function that accepts a concept AcceptedMsg, as below: 

```cpp
struct SP_Com1 { void handle(AcceptedMsg const&) {std::cout << "sp_common::handle \n"; } }; 
```

The rest of ProcessA and ProcessB (and their subprocesses) are defined as follows: 

```cpp
struct SP_A1 { void handle(MsgA const&) { std::cout << "sp_a1::handle \n"; } }; 
struct SP_A2 { void handle(MsgA const&) { std::cout << "sp_a2::handle \n"; } }; 
struct SP_B1 { void handle(MsgB const&) { std::cout << "sp_b1::handle \n"; } }; 
struct SP_B2 { void handle(MsgB const&) { std::cout << "sp_b2::handle \n"; } }; 


struct ProcessA : private ProcessBase
{ 
   void process(MsgA const& m) 
   { 
       std::cout << "ProcessA::procss \n";
       sp_a1.handle(m); 
       sp_a2.handle(m); 
       ProcessBase::process(m); 
   } 
   
   SP_A1 sp_a1; 
   SP_A2 sp_a2; 
 
}; 

struct ProcessB : private ProcessBase
{ 
    void process(MsgB const& m) 
    { 
        std::cout << "ProcessB::process\n"; 
        sp_b1.handle(m); 
        sp_b2.handle(m); 
        ProcessBase::process(m);  
    } 
    SP_B1 sp_b1; 
    SP_B2 sp_b2; 
}; 
```

Putting all these together, I can run the following code: 
```cpp
SomeBigClass sbc;
MsgA msg_a; 
recv(sbc, msg_a); 
  
std::cout << " ---- \n"; 
MsgB msg_b; 
recv(sbc, msg_b);   
```

... which will prints: 

```cpp
processA::procss 
sp_a1::handle 
sp_a2::handle 
sp_common::handle 
 ---- 
ProcessB::process
sp_b1::handle 
sp_b2::handle 
sp_common::handle 
```

Here are **the problems** that I don't like:
- I don't like it that SomeBigClass has an instance of ProcessA and ProcessB. In that example. SomeBigClass ended up with process(MsgA) and process(MsgB). If it originally had functions *foo*, *bar*, and *ham* that took MsgA as input parameter, you may end with a copy of *foo*, *bar*, and *ham* but for MsgB. A bad repeition of code.  

- I also don't like it even more that ProcessA and Process inherits from ProcessBase.  It is just hard to follow. 
-	If I used concepts in one place, then  I should everywhere that follow. 

The **goal** that I want to achieve: remove everything I don't like.  Here is a solution. 

Now, I will replace SomeBigClass with SomeCommonBigClass. 

```cpp
struct SomeCommonBigClass
{
    CommonProcess cp; 
    void process(AcceptedMsg const& m) { cp.process(m); }
}; 
```

Note, SomeCommonBigClass::process deals with the concept AcceptedMsg now.  It also have an instance of newly defined CommonProcess.  This new class combines ProcessA and ProcessB functionality in one class. (Note that ProcessA and ProcessB have no use for any class other SomeBigClass or SomeCommonBigClass. The sole purpose of ProcessB is to handle SomeBigClass's MsgA messages. This why this refactoring makes sense here). 

```cpp
struct CommonProcess
{
   void process(AcceptedMsg const& m) 
   {
      std::cout << "CommonProcess::process\n";
      sp_a1.handle(m); 
      sp_a2.handle(m); 
      sp_b1.handle(m); 
      sp_b2.handle(m); 
      sp_com1.handle(m);
  } 
  SP_A1 sp_a1; 
  SP_A2 sp_a2; 
  SP_B1 sp_b1; 
  SP_B2 sp_b2;
  SP_Com1 sp_com1; 
}; 
```

You see now? SomeCommonBigClass has an instance of all the subprocesses; whether they are for MsgA or MsgB.   That will lead to a compile error!  (sp_b1, sp_b2 do not work wit MsgA, and sp_a1, sp_a2 do not work with MsgB). 

What should we do here? **Borrow some SFINAE techniques**.  

We will use a wrapper for handle. 
```cpp

template <typename P, typename M> 
void handle(P& p, M const& m) 
{
  p.handle(m);   
} 

struct CommonProcess
{
   void process(AcceptedMsg const& m) 
   {
      std::cout << "CommonProcess::process\n";
      handle(sp_a1, m);  // instead of sp_a1.process(m). 
      handle(sp_a2, m); 
      handle(sp_b1, m); 
      handle(sp_b2, m); 
      handle(sp_com1, m);    
   } 
    SP_A1 sp_a1; 
    SP_A2 sp_a2; 
    SP_B1 sp_b1; 
    SP_B2 sp_b2;
    SP_Com1 sp_com1;    
};
```

This will still not work. handle was just a wrapper that does nothing. What we want to achieve is to generate two version of *handle*.  Given a process of type P and a msg of type M, *handle* should invoke P::handle if it exists, otherwise it does nothing.  This is where we use the **C++ Detection Idiom**. We will use *std::experimental::is_detected* (but we can write it ourselves of course).   Now we replace the *handle* wrapper with: 

```cpp
template <typename T, typename M> 
using has_handle_t = decltype( std::declval<T>().handle(std::declval<M const&>())); 

template <typename T, typename M> 
using handle_detected = std::experimental::is_detected<has_handle_t, T, M>; 

// If P has handle for M, then invoke p.handle(m). 
template <typename P, typename M, std::enable_if_t<handle_detected<P, M>::value>* = nullptr> 
void handle(P& p, M const& m) 
{
  p.handle(m);   
} 

// else: 
template <typename P, typename M, std::enable_if_t<! handle_detected<P, M>::value>* = nullptr> 
void handle(P&, M const&) 
{
}
```

**Are there other solutions?** Yes. Just use an interface. It is actually more classical and easier to understand. 1) You will need to modify all of subprocesses to inherits from some interface (this includes changes in all the declaratio of subprocesses - if you can of course). 2) It will be a little bit slower (I don't like this argument). 3) If I use concepts (i.e., static polymorphisim), then let me stick with it and avoid dynamic polymorphisim.  4) They are not that cool anyway. 

**Is it harder to understand?** I don't think so. It is 2020. A lot of changes are happening with C++. We should not be afraid of using the full powers of C++ templates (and all related techniques).  These techniques are not restricted to library writers anymore.  However, even in 2020, be prepared to have your code rejected because of difficulty of maintainance and similar aguments. 
