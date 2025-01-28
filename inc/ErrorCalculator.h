#pragma once

#include <cstdint>

namespace bench {
class ErrorCalculator {
  public:
	struct Result {
		Result(uint64_t max, long double mean) : max(max), mean(mean) {}

		uint64_t max;
		long double mean;
	};
	ErrorCalculator() = default;
	~ErrorCalculator() = default;
	virtual Result calcMaxMeanError() = 0;
};
} // namespace bench