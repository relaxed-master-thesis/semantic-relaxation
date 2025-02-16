#pragma once

#include <cstdint>
#include <memory>

namespace bench {
class Interval {
  public:
	Interval() = default;
	Interval(uint64_t start, uint64_t end) : start(start), end(end), min(start), max(end) {}
	~Interval() = default;

	int compareTo(Interval &other);

	bool operator<(const Interval &other) const
	{
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
  private:
};

}