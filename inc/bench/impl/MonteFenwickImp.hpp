#pragma once

#include "bench/util/Executor.hpp"

#include <cstdint>
#include <vector>

namespace bench {
class MonteFenwickImp : public ApproximateExecutor {
  public:
	MonteFenwickImp() = default;
	MonteFenwickImp(float counting_share) : counting_share(counting_share){};
	~MonteFenwickImp() = default;

	struct PushedItem {
		int64_t pop_time;
	};
	std::vector<PushedItem> pushed_items;

	AbstractExecutor::Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	AbstractExecutor::Measurement execute() override;
	void reset() override;

  private:
	float counting_share{1.f};
};
} // namespace bench