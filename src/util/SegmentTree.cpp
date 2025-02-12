#include "SegmentTree.h"

namespace bench {
void IntervalMapTree::build(std::vector<Interval> &intervals) {
	this->intervals = intervals;
	for (uint64_t i = 0; i < intervals.size(); i++) {
		timePositions[intervals[i].in] = i;
		timePositions[intervals[i].out] = i;
	}
}
} // namespace bench