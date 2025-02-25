# [GeijerBatchImp](../../src/impl/GeijerBatch.cpp)
GeijerBatchImp builds on [GeijerImp](GeijerImp.md) but makes an effort to reduce the amount of list traversals. Starting with an example to explain the idea, lets say we have pushed 1, 2, 3, 4, 5, 6 and 7 to the queue, making it look like this: 
```cpp
{1, 2, 3, 4, 5, 6, 7}
```
And now we want to pop 2, 7 and 5, in that order. Instead of traversing the list three times, we can traverse it only once. 

We step into the list untill we find an element we want to pop. First we find 2, at index 1, so its rank error is 1. Then we keep going and then we find 5 at index 4, then we know that we already popped the 2, making the rank error 4 - 1 = 3. When we finally find the 7, we know that its only the 2 that was actually popped before the 7 so the rank error is 6 - 1 = 5. 

If we want to implement this idea we somehow need to keep track of the pop orders of all elements in a batch, and we also need to know how many of the popped elements in front of an element in the list has a lower pop-order. 

# Implementation
The idea above can be implemented by using a `std::unordered_map` to keep track of all elements in a bach and their pop-order. The map maps element values to its pop-order. in the example above it would look like this: 
```cpp
pops = {{2, 0}, {7, 1}, {5, 2}}
```
Then when we find an element in the list, we need an efficient way to find how many pops affect our rank error. This can be done by using an `orderd_set` with a red black tree as backend. 
The example above can be split up into three steps, one for each pop. 

$i$ is the index of the found element.

$f$ is the number of pops found that will happen before the current element.

$r$ is the rank error.
#### Before list traversal
```cpp
pops = {{2, 0}, {7, 1}, {5, 2}}
found_pops = {};
```
#### Finding 2
```cpp
i = 1
f = found_pops.order_of_key(pops[2]) = 0
r = 1
pops = {{2, 0}, {7, 1}, {5, 2}}
found_pops = {0};

```
#### Finding 5
```cpp
i = 4
f = found_pops.order_of_key(pops[5]) = 1
r = 3
pops = {{2, 0}, {7, 1}, {5, 2}}
found_pops = {0, 2};
```
#### Finding 7
```cpp
i = 6
f = found_pops.order_of_key(pops[7]) = 1
r = 5
pops = {{2, 0}, {7, 1}, {5, 2}}
found_pops = {0, 1, 2};
```

Giving us the rank errors: 
```
2: 1
7: 5
5: 3
```

Finding the element in the unordered map can be done in $O(1)$ time. Finding the ammount of pops before an element in the ordered set can be done in $O(log(b))$ where $b$ is the batch size. This gives us a total time complexity of $O(n*log(b))$
