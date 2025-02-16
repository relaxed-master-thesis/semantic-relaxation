#include "bench/impl/ParallelBatchImp.h"
#include "bench/Benchmark.h"
#include "bench/ErrorCalculator.h"
#include "bench/Interval.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iterator>
#include <limits>
#include <omp.h>
#include <unordered_map>

namespace bench {
ErrorCalculator::Result ParallelBatchImp::calcMaxMeanError() {
	uint64_t rankSum = 0, rankMax = 0;
	size_t start = 0, end = 0;

#pragma omp parallel reduction(+ : rankSum)                   \
	reduction(max : rankMax)
	{
		int tid = omp_get_thread_num();
		SubProblem &problem = subProblems.at(tid);
		auto head = problem.puts.begin();
		for (size_t deq_ind = 0; deq_ind < problem.getValues.size();
			 deq_ind++) {

			//(*get_stamps)[deq_ind].value;
			uint64_t key = problem.getValues[deq_ind];

			uint64_t rank_error = 0;
			if (*head == key) {
				std::advance(head, 1);
				rank_error = 0;
			} else {
				rank_error = 1;
				auto current = head;
				while (*std::next(current, 1) != key) {
					std::advance(current, 1); // current = current->next;
					rank_error += 1;
					if (current ==
						problem.puts.end()) { //(current->next == NULL) {
						perror("Out of bounds on finding matching relaxation "
							   "enqueue\n");
						printf("%zu\n", deq_ind);
						exit(-1);
					}
				}
				// current->next has the removed item, so just unlink it from
				// the data structure
				problem.puts.erase(std::next(
					current, 1)); // current->next = current->next->next;
			}

			// Store rank error in get_stamps for variance calculation
			// (*get_stamps)[deq_ind].value = rank_error;

			rankSum += rank_error;
			if (rank_error > rankMax) {
				rankMax = rank_error;
			}
		}
	}

	const double rankMean = (double)rankSum / numGets;

	return {rankMax, rankMean};
}

void ParallelBatchImp::prepare(InputData data) {
	intervals.resize(data.puts->size());
	std::unordered_map<uint64_t, size_t> putMap{};
	uint64_t max_time = 0;
	for (size_t i = 0; i < data.puts->size(); ++i) {
		auto &put = data.puts->at(i);
		auto &intv = intervals.at(i);
		intv.start = put.time;
		intv.end = ~0;
		intv.value = put.value;
		putMap[put.value] = i;
	}
	numGets = data.gets->size();
	for (size_t i = 0; i < numGets; ++i) {
		auto &get = data.gets->at(i);
		auto &interval = intervals.at(putMap[get.value]);
		interval.end = get.time;
		if (interval.end > max_time) {
			max_time = interval.end;
		}
	}
	size_t numThreads = 16;
	subProblems.resize(numThreads);
	uint64_t min_time = intervals.at(0).start;
	uint64_t time_range = max_time - min_time;
	uint64_t time_window = time_range / numThreads;
	uint64_t idx = 0;
	for (int i = 0; i < numThreads; i++) {
		uint64_t curr_start = min_time + time_window * i;
		uint64_t curr_end =
			std::min(curr_start + time_window * (i + 1), max_time);
		// interval to add to thread i
		subProblems[i].start_time = curr_start;
		subProblems[i].end_time = curr_end;
		while (idx < intervals.size() && intervals.at(idx).start < curr_end) {
			auto &interv = intervals.at(idx);
			subProblems[i].puts.push_back(interv.value);
			subProblems[i].intervals.push_back(interv);
			if (interv.end > curr_end && i != numThreads - 1) {
				// dup interval
				subProblems[i + 1].intervals.push_back(interv);
				subProblems[i + 1].puts.push_back(interv.value);
			}
			idx++;
		}

		// this sorting is correct
		std::ranges::sort(
			subProblems[i].intervals,
			[](const Interval &left, const Interval &right) -> bool {
				return left.end < right.end;
			});

		for (auto &intv : subProblems[i].intervals) {
			subProblems[i].getValues.push_back(intv.value);
		}
	}
}

long ParallelBatchImp::execute() {
	std::cout << "Running ParallelBatchImp...\n";
	auto start = std::chrono::high_resolution_clock::now();
	auto result = calcMaxMeanError();
	auto end = std::chrono::high_resolution_clock::now();
	auto duration =
		std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	std::cout << "Runtime: " << duration.count() << " us\n";
	std::cout << "Mean: " << result.mean << ", Max: " << result.max << "\n";
	return duration.count();
}
} // namespace bench