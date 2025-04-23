#pragma once

#include "bench/util/Executor.hpp"

#include <cstdint>
#include <vector>

namespace bench {
class FenwickAImp : public ApproximateQueueExecutor {
  public:
	FenwickAImp(float counting_share)
		: counting_share(counting_share), counting_type(CountingType::SHARE){};
	FenwickAImp(uint64_t counting_ammount)
		: counting_amount(counting_ammount),
		  counting_type(CountingType::AMOUNT){};
	~FenwickAImp() = default;

	AbstractExecutor::Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	AbstractExecutor::Measurement execute() override;
	void reset() override;

  private:
	struct PushedItem {
		int64_t pop_time;
	};
	std::vector<PushedItem> pushed_items;
	float counting_share;
	CountingType counting_type;
	uint64_t counting_amount{0};
};
} // namespace bench