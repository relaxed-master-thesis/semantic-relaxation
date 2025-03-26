#pragma once

#include "bench/util/Executor.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <list>
#include <sys/types.h>
#include <thread>
#include <unordered_set>
#include <vector>

namespace bench {
class ParallelBatchImp : public AccurateExecutor {
  public:
	ParallelBatchImp() = default;
	ParallelBatchImp(bool useParSplit)
		: numThreads(std::thread::hardware_concurrency()),
		  useParSplit(useParSplit) {}
	~ParallelBatchImp() = default;

	AbstractExecutor::Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	AbstractExecutor::Measurement execute() override;
	void reset() override;
	void setNumThreads(uint64_t numThreads) { this->numThreads = numThreads; }
	void setUseParSplit(bool useParSplit) { this->useParSplit = useParSplit; }

  private:
	struct Item {
		uint64_t value;
		Item *next;
	};

	struct Interval {
		Interval() = default;
		Interval(uint64_t start, uint64_t end) : start(start), end(end) {}
		~Interval() = default;

		uint64_t start;
		uint64_t end;
		uint64_t value;
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
	bool useParSplit{false};
	std::vector<SubProblem> subProblems{};
	void splitOnTime(const std::vector<Interval> &intervals, uint64_t max_time);

	void splitOnWork(const std::vector<Interval> &intervals, uint64_t max_time);
	void splitOnWorkPar(const std::vector<Interval> &intervals,
						uint64_t max_time);

	std::pair<uint64_t, uint64_t> calcMaxSumErrorBatch(SubProblem problem,
													   size_t tid);
	std::pair<uint64_t, uint64_t> calcMaxSumErrorGeijer(SubProblem problem,
														size_t tid);
};
} // namespace bench