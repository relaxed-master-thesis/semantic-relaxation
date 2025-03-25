#pragma once

#include "bench/util/Executor.hpp"

#include <cstdint>
#include <vector>

namespace bench {
class ParallelFenwickImp : public AccurateExecutor {
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

	ParallelFenwickImp() = default;
	~ParallelFenwickImp() = default;

	struct PushedItem {
		uint64_t time;
		uint64_t pop_time;
		int64_t pop_order;
	};
	std::vector<PushedItem> pushed_items;
	struct range {
        uint64_t from, to;
    };
	std::vector<range> ranges;
	uint64_t get_stamps_size;


	AbstractExecutor::Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	AbstractExecutor::Measurement execute() override;
	void reset() override;

	std::pair<uint64_t, uint64_t> calcRange(range r);

  private:
};
} // namespace bench