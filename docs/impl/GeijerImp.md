# [GeijerImp](../../src/impl/GeijerImp.cpp)
GeijerImp builds on the realization that for calculating the rank errors for all pops we can just do all pushes at once, without caring about interleaving the pops inbetween the pushes. GeijerImp also utilizes that deletion from a linked list is very quick. 

First a linked list is bulit where all elements contains a next pointer and a value. The linked list is built as if it was the queue wher all pushes has been applied. 
Then for each pop, it looks through the list to find the element while keeping track of the index of the found item. Then it simply removes the element from the list by changing the next pointer of the predecessor to the found item to the found items next pointer. It does this for all pops. 

GeijerImp is increadibly fast at low relaxation errors with an approximate time complexity of $O(n * k)$ where $n$ is the isze of the list and $k$ is the mean relaxation error.  