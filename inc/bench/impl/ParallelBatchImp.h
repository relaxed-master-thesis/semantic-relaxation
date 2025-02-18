#pragma once

#include "bench/Benchmark.h"
#include "bench/ErrorCalculator.h"
#include "bench/Interval.h"

#include <compare>
#include <cstddef>
#include <cstdint>
#include <list>
#include <sys/types.h>
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
	struct Item {
		uint64_t value;
		Item *next;
	};

	struct SubProblem {
		std::vector<Interval> intervals{};
		std::unordered_set<uint64_t> non_counting_puts{};
		uint64_t const_error{0};
		std::list<uint64_t> puts{};
		Item *puts2;
		std::vector<uint64_t> getValues{};
		int64_t start_time;
		uint64_t end_time;
	};

	std::vector<Interval> intervals{};
	size_t numGets{0};
	uint64_t numThreads{2};
	std::vector<SubProblem> subProblems{};
	void splitOnTime(const std::vector<Interval> &intervals, uint64_t max_time);

	void splitOnWork(const std::vector<Interval> &intervals, uint64_t max_time);

	
	std::pair<uint64_t, uint64_t> calcMaxSumErrorBatch(SubProblem problem, size_t tid);
	std::pair<uint64_t, uint64_t> calcMaxSumErrorGeijer(SubProblem problem, size_t tid);
};
} // namespace bench