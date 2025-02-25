# [IVTImp](../../src/impl/IVTImp.cpp) (Interval Vector Tree)

**Note: This algorithm is WIP and does not always generate the correct results**

Similar to the Fenwick Tree implementation, this is also implemented using a tree with `std::vector`-based storage. However, this tree is a complete binary heap and the backing vector is sorted as such. More information about the `VectorTree` can be found [here](../util/VectorTree.md). The idea behind this solution was that given a complete binary heap of intervals sorted on interval start time in ascending order, it may be possible to eliminate whole subtrees to check since we know that relaxation errors can only be induced by intervals that were enqueued before any given intervals. In other words, all intervals with enqueue times larger than any given interval's start time are irrelevant. Since the binary heap is sorted these intervals would be placed in the subtree where the current interval's right child is the root of that subtree. In summation, we only need to check a interval's parent, siblings, and grandchildren of the interval's left child.

This solution also revealed an interesting property of the problem of calculating relaxation errors. If we store the data as intervals and not as items being enqueued and dequeued a solution could be made inifitely parallel as an interval-based solution would only require reads from the data set.

## Implementation
