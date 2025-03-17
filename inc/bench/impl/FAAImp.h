#pragma once

#include "bench/Benchmark.h"

#include <cstdint>
#include <vector>

namespace bench {
class FAAImp : public ApproximateExecutor {
  public:
	FAAImp() = default;

	void prepare(const InputData &data);
	Measurement execute();
	Measurement calcMaxMeanError();

  private:
	std::shared_ptr<const std::vector<Operation>> puts;
	std::shared_ptr<const std::vector<Operation>> gets;
};

class IMEImp : public ApproximateExecutor {
  public:
	IMEImp() = default;

	void prepare(const InputData &data);
	Measurement execute();
	Measurement calcMaxMeanError();
private:
    struct Interval {
        Interval() = default;
        Interval(uint64_t start, uint64_t end) : start(start), end(end) {}

        uint64_t start, end, pop_idx;
        bool was_popped{false};
    };

    uint64_t intv_avg_size{0};
    uint64_t exec_start{0}, exec_end{0};
    uint64_t put_count{0}, get_count{0};
    std::vector<Interval> intervals{};
};

} // namespace bench