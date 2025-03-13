#pragma once

#include "bench/Benchmark.h"
// #include "bench/Interval.h"
#include "bench/Operation.h"
// #include "bench/util/AugmentedIntervalTree.h"
#include "bench/util/VectorTree.h"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

namespace bench {
class AITImp : public AbstractExecutor {
  public:
	AITImp() = default;
	~AITImp() = default;
	Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	Measurement execute() override;

  private:
	struct Interval {
		Interval() : start(0), end(0), max(0), min(0) {}
		Interval(uint64_t start, uint64_t end, uint64_t max, uint64_t min)
			: start(start), end(end), max(max), min(min) {}
		Interval(const Interval &other)
			: start(other.start), end(other.end), max(other.max),
			  min(other.min) {}

		uint64_t start{0}, end{0}, max{0}, min{0};

		Interval &operator=(const Interval &other) {
			if (this != &other) {
				start = other.start;
				end = other.end;
				max = other.max;
				min = other.min;
			}
			return *this;
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
	};

	std::shared_ptr<std::vector<Operation>> put_stamps;
	std::shared_ptr<std::vector<Operation>> get_stamps;
	size_t put_stamps_size;
	size_t get_stamps_size;

	std::unordered_map<uint64_t, uint64_t> put_map;
	// std::vector<std::shared_ptr<Interval>> segments;
	std::vector<Interval> intervals{};

	// AugmentedIntervalTree ait;
	VectorTree<Interval> ivt;

	void fix_dup_timestamps();
	uint64_t getRank(const Interval &intv);
	void calcErrThread(size_t start, size_t end,
					   std::pair<uint64_t, uint64_t> *result);
};
} // namespace bench
