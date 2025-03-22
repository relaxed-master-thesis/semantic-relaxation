#pragma once

#include "bench/util/InputData.hpp"

#include <cmath>
#include <cstdint>

namespace bench {

enum BenchmarkType { Accurate = 0, Approximate };

class AbstractExecutor {
  public:
	struct Measurement {
		Measurement() = default;
		Measurement(uint64_t max, long double mean) : max(max), mean(mean) {}
		Measurement(Measurement &&) = default;
		Measurement &operator=(const Measurement &other) {
			if (this != &other) {
				max = other.max;
				mean = other.mean;
			}
			return *this;
		}
		Measurement(const Measurement &other)
			: max(other.max), mean(other.mean) {}

		bool operator==(const Measurement &other) {
			return std::abs(mean - other.mean) < eps && max == other.max;
		}

		bool operator!=(const Measurement &other) { 
			return std::abs(mean - other.mean) > eps || max != other.max; 
		}

		const double eps = 0.0001;
		long double mean;
		uint64_t max;
	};

	virtual void prepare(const InputData &data) = 0;
	virtual Measurement execute() = 0;
	virtual Measurement calcMaxMeanError() = 0;
	virtual void reset() = 0;
	virtual BenchmarkType type() = 0;
};

class AccurateExecutor : public AbstractExecutor {
  public:
	BenchmarkType type() final { return BenchmarkType::Accurate; }
};
class ApproximateExecutor : public AbstractExecutor {
  public:
	BenchmarkType type() final { return BenchmarkType::Approximate; }
	enum class CountingType { SHARE, AMOUNT };
};
} // namespace bench