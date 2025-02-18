#include "bench/impl/ParallelBatchImp.h"
#include "bench/Benchmark.h"
#include "bench/ErrorCalculator.h"
#include "bench/Interval.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
#include <future>
#include <iterator>
#include <numeric>
#include <omp.h>
#include <sys/types.h>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

namespace bench {
ErrorCalculator::Result ParallelBatchImp::calcMaxMeanError() {
	auto func = [this](size_t tid, std::pair<uint64_t, uint64_t> *results) -> void {
		uint64_t rankSum = 0, rankMax = 0, wasted_work = 0;
		SubProblem &problem = subProblems.at(tid);
		auto head = problem.puts2;
		for (size_t deq_ind = 0; deq_ind < problem.getValues.size();
			 deq_ind++) {

			uint64_t key = problem.getValues[deq_ind];
			bool isNonCounting = problem.non_counting_puts.find(key) !=
								 problem.non_counting_puts.end();

			uint64_t rank_error = 0;
			if (head->value == key) {
				head = head->next;
				rank_error = 0;
			} else {
				rank_error = 1;
				auto current = head;
				while (current->next->value != key) {
					current = current->next; 
					rank_error += 1;
					if (current->next == NULL) {
						perror("Out of bounds on finding matching relaxation "
							   "enqueue\n");
						printf("%zu\n", deq_ind);
						exit(-1);
					}
				}
				current->next = current->next->next;
			}

			if (problem.non_counting_puts.find(key) !=
				problem.non_counting_puts.end()) {
				wasted_work++;
				continue;
			}
			rankSum += rank_error;
			if (rank_error > rankMax) {
				rankMax = rank_error;
			}
		}
		printf("Thread %zu wasted work: %lu\n", tid, wasted_work);
		*results = {rankMax, rankSum};
	};

	const size_t numThreads = 12;
	std::pair<uint64_t, uint64_t> results[numThreads];
	std::vector<std::thread> threads{};
	for (size_t i = 0; i < numThreads; ++i) {
		std::thread thread(func, i, &(results[i]));
		threads.push_back(std::move(thread));
	}

	uint64_t rankSum = 0, rankMax = 0;
	for (size_t i = 0; i < numThreads; ++i) {
		threads.at(i).join();
		auto [max, sum] = results[i];
		rankSum += sum;
		if (max > rankMax) {
			rankMax = max;
		}
	}
	return {rankMax, (double)rankSum / numGets};
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
	uint64_t time_window = (time_range / numThreads) + 1;
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
		while (idx < intervals.size() && intervals.at(idx).start < curr_end) {
			auto &interv = intervals.at(idx);
			subProblems[i].puts.push_back(interv.value);
			subProblems[i].intervals.push_back(interv);
			if (interv.end >= curr_end && i != numThreads - 1) {
				int next_i = i + 1;
				int64_t next_start = start_times[next_i];
				while (next_i < numThreads && next_start < interv.end) {
					subProblems[next_i].non_counting_puts.insert(interv.value);
					subProblems[next_i].intervals.push_back(interv);
					subProblems[next_i].puts.push_back(interv.value);
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
			if (intv.end < max_time + 1)
				subProblems[i].getValues.push_back(intv.value);
		}

		size_t nItems = subProblems[i].puts.size();
		subProblems[i].puts2 = static_cast<Item*>(calloc(nItems, sizeof(Item)));
		auto putsiter = subProblems[i].puts.begin();
		for (size_t j = 0; j < nItems; ++j) {
			Item *item = &subProblems[i].puts2[j];
			item->value = *putsiter;
			item->next = j < nItems-1 ? &subProblems[i].puts2[j+1] : NULL;
			std::advance(putsiter, 1);
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