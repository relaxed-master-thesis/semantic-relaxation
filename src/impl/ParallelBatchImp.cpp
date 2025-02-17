#include "bench/impl/ParallelBatchImp.h"
#include "bench/Benchmark.h"
#include "bench/ErrorCalculator.h"
#include "bench/Interval.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
#include <iterator>
#include <numeric>
#include <omp.h>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

namespace bench {
ErrorCalculator::Result ParallelBatchImp::calcMaxMeanError() {
	uint64_t rankSum = 0, rankMax = 0;
	size_t start = 0, end = 0;
	std::vector<uint64_t> rankSums(subProblems.size(), 0);
	std::vector<uint64_t> rankMaxs(subProblems.size(), 0);

#pragma omp parallel // reduction(+ : rankSum) reduction(max : rankMax)
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
					// if (current->next == NULL) {
					if (current == problem.puts.end()) {
						perror("Out of bounds on finding matching relaxation "
							   "enqueue\n");
						printf("%zu\n", deq_ind);
						exit(-1);
					}
				}
				// current->next has the removed item, so just unlink it
				// from the data structure
				problem.puts.erase(std::next(current));
				// current->next = current->next->next;
			}

			if (problem.non_counting_puts.find(key) !=
				problem.non_counting_puts.end()) {
				// printf("Thread %d: Skipping key: %lu\n", tid, key);
				continue;
			}
			rankSums[tid] += rank_error;
			if (rank_error > rankMaxs[tid]) {
				rankMaxs[tid] = rank_error;
			}
		}
	}
	rankSum = std::accumulate(rankSums.begin(), rankSums.end(), 0);
	rankMax = *std::max_element(rankMaxs.begin(), rankMaxs.end());

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
	size_t numThreads = 12;
	subProblems.resize(numThreads);
	uint64_t min_time = intervals.at(0).start;
	uint64_t time_range = max_time - min_time;
	uint64_t time_window = time_range / numThreads;
	printf("min_time: %lu, max_time: %lu, time_range: %lu, time_window:%lu\n",
		   min_time, max_time, time_range, time_window);
	std::vector<int64_t> start_times(numThreads);
	std::vector<int64_t> end_times(numThreads);
	for (int i = 0; i < numThreads; i++) {
		start_times[i] = min_time + time_window * i;
		end_times[i] = std::min(start_times[i] + time_window, max_time);
		if (i == 0) {
			start_times[i] = min_time - 1;
		}
		if (i == numThreads - 1) {
			end_times[i] = max_time + 1;
		}
	}

	uint64_t idx = 0;
	for (int i = 0; i < numThreads; i++) {

		int64_t curr_start = start_times[i];
		int64_t curr_end = end_times[i];

		subProblems[i].start_time = curr_start;
		subProblems[i].end_time = curr_end;
		while (idx < intervals.size() && intervals.at(idx).start <= curr_end) {
			auto &interv = intervals.at(idx);
			subProblems[i].puts.push_back(interv.value);
			subProblems[i].intervals.push_back(interv);
			if (interv.end >= curr_end && i != numThreads - 1) {
				int next_i = i + 1;
				int64_t next_start = start_times[next_i];
				while (next_i < numThreads && interv.end > next_start) {
					subProblems[next_i].intervals.push_back(interv);
					subProblems[next_i].puts.push_back(interv.value);
					subProblems[next_i].non_counting_puts.insert(interv.value);
					next_i++;
					next_start = start_times[next_i];
				}
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
			// bruh, we added values that were only put but never get
			if (intv.end < max_time)
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