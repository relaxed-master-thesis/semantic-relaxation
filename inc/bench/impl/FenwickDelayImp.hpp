#pragma once

#include "bench/util/Executor.hpp"

#include <vector>

namespace bench {
class FenwickDelayImp : public AccurateExecutor {
  public:
	FenwickDelayImp() = default;
	~FenwickDelayImp() = default;

	void prepare(const InputData &data) override;
	Measurement execute() override;
	Measurement calcMaxMeanError() override;
	void reset() override;

  private:
	template <typename T> class ReverseFenwickTree {
		static_assert(std::is_integral<T>::value, "T must be an integral type");

	  public:
		ReverseFenwickTree(int n) : BIT(n + 1, 0){};
		~ReverseFenwickTree() = default;

		void update(int64_t idx, T val) {
			while (idx < BIT.size()) {
				BIT[idx] += val;
				idx += idx & -idx;
			}
		}

		T query(int64_t idx) {
			T sum = 0;
			while (idx > 0) {
				sum += BIT[idx];
				idx -= idx & -idx;
			}
			return sum;
		}

		const std::vector<T> &getBit() const { return BIT; }

	  private:
		std::vector<T> BIT;
	};

	std::vector<int64_t> popTimes{};
};
} // namespace bench