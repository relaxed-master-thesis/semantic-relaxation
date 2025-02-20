#pragma once

#include "bench/Benchmark.h"

#include <cstdint>

namespace bench {
class FenwickImp : public AbstractExecutor {
  public:
	class FenwickTree {
	  private:
		std::vector<int64_t> BIT;

	  public:
		FenwickTree(size_t n) : BIT(n + 1, 0){};
		~FenwickTree() = default;

		void update(int64_t idx, int64_t val);
		int64_t query(int64_t idx);
		const std::vector<int64_t> &getBit() const { return BIT; }
	};

	class SInterval {
	  public:
		SInterval() = default;
		~SInterval() = default;
		int64_t start;
		int64_t end;
		int64_t value;
	};

	FenwickImp() = default;
	~FenwickImp() = default;

	AbstractExecutor::Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	AbstractExecutor::Measurement execute() override;

  private:
	std::vector<SInterval> intervals{};
};
} // namespace bench