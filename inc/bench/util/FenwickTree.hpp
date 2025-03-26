#pragma once

#include <cstdint>
#include <type_traits>
#include <vector>

namespace bench {
template <typename T> class FenwickTree {
	static_assert(std::is_integral<T>::value, "T must be an integral type");

  public:
	FenwickTree(int n) : BIT(n + 1, 0){};
	~FenwickTree() = default;

	void update(int64_t idx, T val) {
		while (idx > 0) {
			BIT[idx] += val;
			idx -= idx & -idx;
		}
	}

	T query(int64_t idx) {
		T sum = 0;
		while (idx < BIT.size()) {
			sum += BIT[idx];
			idx += idx & -idx;
		}
		return sum;
	}

	const std::vector<T> &getBit() const { return BIT; }

  private:
	std::vector<T> BIT;
};
} // namespace bench