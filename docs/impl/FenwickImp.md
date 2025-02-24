# [FenwickImp](../../src/impl/FenwickImp.cpp)

This implementation is based on a Fenwick Tree. The idea of this implementation is to take advantage of the Fenwick tree's property of *O(log n)* search complexity while still being able to keep multiple children per node. This is possible as the querying indexes with bitwise operations. More information about Fenwick trees can be found on [wikipedia](https://en.wikipedia.org/wiki/Fenwick_tree).

## Implementation

Start by sorting the intervals by increasing start values. If two start values are equal then sort by decreasing end times (in this particular case all start values are unique because of extra preprocessing but this doesn't have to be performed). This is done in $O(n\ log(n))$ time[^rangessort].

```cpp
std::ranges::sort(
	intervals, [](const SInterval &left, const SInterval &right) -> bool {
		return left.start < right.start;
});
```

Copy the intervals' end times to a separate list and sort them in ascending order. Then iterate over this separate list and compress their values to being index based starting from `1`. `std::unordered_map` provides an average complexity of $O(1)$ with worst case complexity of $O(n)$. As all start and end times are unique across all intervals it is safe to assume the map will have no collisions and therefore have worst case complexity of $O(1)$[^inthash]. In total the complexity becomes $O(n+n\ log(n)+ n)=O(n\ log(n))$

```cpp
std::vector<int64_t> sortedEnds(n, 0);
for (size_t i = 0; i < n; ++i) {
	sortedEnds[i] = intervals[i].end;
}

std::ranges::sort(sortedEnds);
std::unordered_map<int64_t, int64_t> endIndices{};
for (int64_t i = 0; i < n; ++i) {
	endIndices[sortedEnds[i]] = i + 1;
}
```

Then we iterate over the interval list sorted on increasing start times and query the tree. Since the tree stores prefix sums of the amount of intervals encountered up to a specific time point (or interval end time), we can query for the total number of encountered intervals and the number of encountered intervals upto our current interval's end time. The difference between the query results then represents the amount of intervals that have been enqueued but not dequeued which is our relaxation error. After querying we also update the tree by increasing the affected prefix sums by `1` for subsequent intervals. Similar to previous steps the complexity for this step becomes $O(n\cdot(3\ log(n)))=O(n\ log(n))$

```cpp
for (int64_t i = 0; i < n; ++i) {
	int64_t endVal = intervals[i].end;

	if (endVal == std::numeric_limits<int64_t>::max()) {
		++constError;
		continue;
	}
	++countedElems;
	int64_t comprEnd = endIndices[endVal];

	sum += BIT.query(n) - BIT.query(comprEnd) + constError;
	BIT.update(comprEnd, 1);
}
```

In total, this solution is ran in $O(n\ log(n))$ time.

**Notes:**

[^inthash]: The C++ hash function for integers return the integer itself which guarantees unique hashes as long as the keys to be hashed are unique.
[^rangessort]: https://en.cppreference.com/w/cpp/algorithm/ranges/sort