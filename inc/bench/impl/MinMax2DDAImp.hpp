#pragma once

#include "bench/util/Executor.hpp"

namespace bench {
class MinMax2DDAImp : public ApproximateQueueExecutor {

  public:
	MinMax2DDAImp() = default;
	MinMax2DDAImp(float share, size_t expectedHeight, size_t expectedWidth)
		: countingShare(share), expectedHeight(expectedHeight),
		  expectedWidth(expectedWidth) {}

	void prepare(const InputData &data) override;
	Measurement execute() override;
	Measurement calcMaxMeanError() override;
	void reset() override;

  private:
	struct PushedItem {
		int64_t pop_time;
	};
	std::vector<PushedItem> pushed_items;
	float countingShare{1.0};
	size_t expectedHeight{0};
	size_t expectedWidth{0};
};
} // namespace bench