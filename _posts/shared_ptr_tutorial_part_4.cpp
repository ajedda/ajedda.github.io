// In the previous tutorials, we used uint32_t as our reference 
// counter. Why uint32_t and not uint16_t? or uint8_t? 
// We are also assuming a single-thread design. The only part of 
// our program that will be shared by the threads is this reference 
// counter. So how to protect it? mutex? spinlock? atomic? 
// All of these are implementation details. 
// What we will do in this tutorial is to hide the implementation details 
// of the reference counter in another class. We will examine multiple 
// multi-threaded implementations as well.  

// Before that, I will try to improve my test-cases (which were kind-of non-existent) 
// before this tutorial. This is tricky since we are testing 
// for memory leaks as well. As you will see, hiding the details of the ref-counter 
// may help us a bet. 

// Many topics to discuss next, I will certainly divide this into 
// multiple tutorials.
// TODO 
