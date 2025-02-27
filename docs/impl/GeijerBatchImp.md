# [GeijerBatchImp](../../src/impl/GeijerBatch.cpp)
GeijerBatchImp builds on [GeijerImp](GeijerImp.md) but makes an effort to reduce the amount of list traversals. Starting with an example to explain the idea, lets say we have pushed 1, 2, 3, 4, 5, 6 and 7 to the all_pops, making it look like this: 
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

$f$ is the number of pops found that with a lower pop-order value than the current element.

$r$ is the rank error and can be calculated by taking $i - f$.

`found_pops` is our `orderd_set` containing the `pop-orders` of untill now found elements in the batch.


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

Finding the element in the unordered map can be done in $O(1)$ time. Finding the amount of pops before an element in the ordered set can be done in $O(log(b))$ where $b$ is the batch size. This gives us a total time complexity of $O(n*log(b))$

### Simple pseudo code for one batch
`all_pops` is a list of all pops, in the order they happened in the monitored execution. 

`batch_size` is the size of one batch.

`all_gets` is a list of all gets, in the order they happened in the monitored execution.

`get_index` is the index used to traverse `all_gets` in order.
```cpp
//first we create the batch
std::unordered_map<uint64_t, uint64_t> pops;
int added_pops = 0;

while(added_pops < batch_size){
    //map next get value to the pop-order
    pops.insert(all_gets[get_index], added_pops++);
}
ordered_set<int> found_pops;
int index = 0;
int found = 0;
//search untill we find all added pops
while(found < added_pops){
    if(pops.contains(all_pops[index])){
        int pop_order = pops.at(all_pops[index])
        int f = found_pops.order_of_key(pop_order);
        int r = index - f;
        //use r to update rank_sum and rank_max
        rank_sum += r;
        rank_max = max(r, rank_max)
        //update found_pops and 
        found_pops.insert(pop_order);
        all_pops.remove_index(index);
        found++;
    }
    index++;
}
```


