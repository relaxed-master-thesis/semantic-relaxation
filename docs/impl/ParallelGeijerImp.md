# [ParallelGeijerImp](../../src/impl/ParallelGeijer.cpp)

**Note: This solution does not work for obvious reasons**

The idea was to simply parallelize [`bench::GeijerImp`](../../src/impl/GeijerImp.cpp) by using OpenMP directives. For obvious reasons this does not work as the problem is inherently sequential and in parallel the queue is mutated in the wrong order. This implementation merely exist as a guideline on how fast [`bench::GeijerImp`](../../src/impl/GeijerImp.cpp) could run if perfectly parallel.