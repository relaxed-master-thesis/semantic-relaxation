#pragma once

#include "bench/Benchmark.h"
#include "bench/ErrorCalculator.h"
#include "bench/Interval.h"

#include <cstdint>
#include <list>
#include <unordered_set>
#include <vector>

namespace bench {
class ParallelBatchImp : public AbstractExecutor, public ErrorCalculator {
  public:
	ParallelBatchImp() = default;
	~ParallelBatchImp() = default;

	Result calcMaxMeanError() override;
	void prepare(InputData data) override;
	long execute() override;


  private:
	struct SubProblem {
		std::vector<Interval> intervals{};
		std::unordered_set<uint64_t> non_counting_puts{};
		std::list<uint64_t> puts{};
		std::vector<uint64_t> getValues{};
		int64_t start_time;
		uint64_t end_time;
	};

	std::vector<Interval> intervals{};
	size_t numGets{0};
	std::vector<SubProblem> subProblems{};

};
} // namespace bench