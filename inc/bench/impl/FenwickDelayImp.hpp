#pragma once

#include "bench/util/Executor.hpp"

#include <vector>

namespace bench {
class FenwickDelayImp : public AccurateQueueExecutor {
  public:
	FenwickDelayImp() = default;
	~FenwickDelayImp() = default;

	void prepare(const InputData &data) override;
	Measurement execute() override;
	Measurement calcMaxMeanError() override;
	void reset() override;

  private:
	// std::vector<int64_t> popTimes{};
	std::vector<int64_t> popTimes{};
	size_t num_gets{0};
};
} // namespace bench