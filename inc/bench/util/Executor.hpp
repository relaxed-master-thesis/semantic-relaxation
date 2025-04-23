#pragma once

#include "bench/util/InputData.hpp"

#include <cmath>
#include <cstdint>

namespace bench {

enum BenchmarkType { Accurate = 0, Approximate };
enum DataStructureType { Queue = 0, Stack};

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
	virtual DataStructureType dataStructureType() = 0;
};

class AccurateQueueExecutor : public AbstractExecutor {
  public:
	BenchmarkType type() final { return BenchmarkType::Accurate; }
	DataStructureType dataStructureType() final {
		return DataStructureType::Queue;
	}
  private:
};
class AccurateStackExecutor : public AbstractExecutor {
  public:
	BenchmarkType type() final { return BenchmarkType::Accurate; }
	DataStructureType dataStructureType() final {
		return DataStructureType::Stack;
	}
  private:
};
class ApproximateQueueExecutor : public AbstractExecutor {
  public:
	BenchmarkType type() final { return BenchmarkType::Approximate; }
	DataStructureType dataStructureType() final {
		return DataStructureType::Queue;
	}
	enum class CountingType { SHARE, AMOUNT };
};
class ApproximateStackExecutor : public AbstractExecutor {
  public:
	BenchmarkType type() final { return BenchmarkType::Approximate; }
	DataStructureType dataStructureType() final {
		return DataStructureType::Stack;
	}
	enum class CountingType { SHARE, AMOUNT };
};
} // namespace bench