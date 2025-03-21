#pragma once

#include "bench/Benchmark.h"

namespace bench {
class MinMax2DDAImp : public ApproximateExecutor {
	using point = std::pair<uint64_t, uint64_t>;
	using range = std::pair<size_t, size_t>;

  public:
	MinMax2DDAImp() = default;
	MinMax2DDAImp(float share, size_t expectedHeight,size_t expectedWidth)
		: countingShare(share), expectedHeight(expectedHeight), expectedWidth(expectedWidth) {}

	void prepare(const InputData &data) override;
	Measurement execute() override;
	Measurement calcMaxMeanError() override;
	void reset() override;

  private:
	float countingShare{1.0};
	size_t expectedHeight{0};
	size_t expectedWidth{0};
	std::vector<point> points{};
	std::vector<range> ranges{};
};
} // namespace bench