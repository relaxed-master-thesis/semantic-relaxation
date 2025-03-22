#pragma once

// #include "bench/Interval.h"

#include <memory>

namespace bench {
struct Interval {
	Interval() = default;
	Interval(uint64_t start, uint64_t end)
		: start(start), end(end), min(start), max(end) {}
	~Interval() = default;

	int compareTo(Interval &other) {
		if (this->start < other.start)
			return -1;
		else if (this->start == other.start)
			return this->end <= other.end ? -1 : 1;
		else
			return 1;
	}

	bool operator<(const Interval &other) const {
		if (start < other.start) {
			return true;
		} else if (start == other.start) {
			return end <= other.end;
		} else {
			return false;
		}
	}

	uint64_t start;
	uint64_t end;
	uint64_t min;
	uint64_t max;
	uint64_t value;
	std::shared_ptr<Interval> left;
	std::shared_ptr<Interval> right;
};

class AugmentedIntervalTree {
  public:
	AugmentedIntervalTree() = default;
	~AugmentedIntervalTree() = default;
	std::shared_ptr<Interval> insertNode(std::shared_ptr<Interval> tmp,
										 std::shared_ptr<Interval> newNode);
	uint64_t getRank(std::shared_ptr<Interval> tmp,
					 std::shared_ptr<Interval> interval, uint64_t rank);
	std::shared_ptr<Interval> root;

  private:
};

} // namespace bench