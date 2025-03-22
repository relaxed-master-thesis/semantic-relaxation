#pragma once

#include "bench/util/Executor.hpp"

#include <cstdint>

namespace bench {
class FenwickAImp : public ApproximateExecutor {
  public:
	struct SInterval {
		SInterval() = default;
		~SInterval() = default;
		int64_t start;
		int64_t end;
		int64_t value;
	};

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
	std::vector<SInterval> intervals{};
	float counting_share;
	CountingType counting_type;
	uint64_t counting_amount{0};
};
} // namespace bench