#pragma once

#include "bench/util/Executor.hpp"

#include <cstdint>
#include <vector>

namespace bench {
class FenwickImp : public AccurateExecutor {
  public:
	FenwickImp() = default;
	~FenwickImp() = default;

	struct PushedItem {
		int64_t pop_time;
	};
	std::vector<PushedItem> pushed_items;

	AbstractExecutor::Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	AbstractExecutor::Measurement execute() override;
	void reset() override;

  private:
};
} // namespace bench