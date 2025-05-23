#pragma once

#include "bench/util/Executor.hpp"

#include <cstdint>
#include <vector>

namespace bench {
class FAAImp : public ApproximateQueueExecutor {
  public:
	FAAImp() = default;

	void prepare(const InputData &data) override;
	Measurement execute() override;
	Measurement calcMaxMeanError() override;
	void reset() override;

  private:
	std::shared_ptr<const std::vector<Operation>> puts;
	std::shared_ptr<const std::vector<Operation>> gets;
};

class IMEImp : public ApproximateQueueExecutor {
  public:
	IMEImp() = default;

	void prepare(const InputData &data) override;
	Measurement execute() override;
	Measurement calcMaxMeanError() override;
	void reset() override;

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