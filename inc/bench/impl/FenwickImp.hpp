#pragma once

#include "bench/util/Executor.hpp"

#include <cstdint>
#include <vector>

namespace bench {
class FenwickImp : public AccurateExecutor {
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