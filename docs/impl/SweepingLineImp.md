# [SweepingLineImp](../../src/impl/SweepingLineImp.cpp)

The sweeping line algorithm is as simple as the name suggests. It is a single loop over all timestamps (referred to as events) and accumulates the total rank error (used for mean) and max.

## Implementation
The two vectors of [`bench::Operation`](../../inc/bench/Operation.h) are merged into one vector of `bench::SweepingLineImp::Event` where every event is either a `EventType::START` or `EventType::END`. Each event keeps track of the interval that it represents in the way that for every *interval* there will exist two events, one start and one end, that both hold information about their originating interval. This vector of events are then sorted by the timestamps that each event represent. For instance, for any interval the start-event will always precede the end-event for the same interval. For further reference these are also referred to as *linearization points*. In total this preparation is done in $O(n\log(n))$

The solution itself is rather simple as the name suggests. We iterate over the sorted events vector and cover three cases. 
1) If the event does not have a valid end-time (the value that was enqueued was never dequeued) it will give all subsequent intervals a constant error of $1$.
2) If the event is a start-event we need to perform some additional processing. The solution uses an ordered set which is an ordered tree of interval end-times. We query the tree for the order of our current event's end-time (remember that all events keep track of both start and end times). The order is the amount of items in the set that are *strictly* smaller than our key. Which means that for every query we know how many elements are dequeued before us, which is our rank error. The querying is also done in $O(\log n)$. To ensure correctness for all subsequent calculations we also insert our events end-time to the ordered set.
3) If the event is an end-event then we simply remove the end-time from the ordered set as it will no longer affect future calculations.

All-in-all this solution is run in $O(n\log(n))$. In practice this is closer to $O(n\log(k))$ where $k$ is the size of the FIFO queue at any given time. Normally $k\ll n$ but this is not guaranteed as its possible for all enqueues to happen before any dequeue, hence $O(n\log(n))$.

```cpp
for (auto &event : events) {
	if (event.end_time == ~0) {
		const_error++;
	} else if (event.type == EventType::START) {
		uint64_t rank = end_tree.order_of_key(event.end_time);
		rank += const_error;
		rank_sum += rank;
		counted_rank++;
		if (rank > rank_max) {
			rank_max = rank;
		}
		end_tree.insert(event.end_time);
	} else {
		end_tree.erase(event.end_time);
	}
}
```