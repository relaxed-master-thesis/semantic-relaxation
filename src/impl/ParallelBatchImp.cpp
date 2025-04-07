#include "bench/impl/ParallelBatchImp.hpp"
#include "bench/util/Executor.hpp"
#include "bench/util/GNUOrderedSet.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <omp.h>
#include <sys/types.h>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
namespace bench {
std::pair<uint64_t, uint64_t>
ParallelBatchImp::calcMaxSumErrorGeijer(SubProblem problem, size_t tid) {
	uint64_t rankSum = 0, rankMax = 0, wasted_work = 0, actual_work = 0;
	// SubProblem &problem = subProblems.at(tid);
	auto head = problem.puts2;
	for (size_t deq_ind = 0; deq_ind < problem.getValues.size(); deq_ind++) {

		uint64_t key = problem.getValues[deq_ind];

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
					throw std::runtime_error("Out of bounds on finding "
											 "matching relaxation enqueue\n");
				}
			}
			current->next = current->next->next;
		}

		if (problem.non_counting_puts.find(key) !=
			problem.non_counting_puts.end()) {
			wasted_work++;
			continue;
		}
		actual_work++;
		rank_error += problem.const_error;
		rankSum += rank_error;
		if (rank_error > rankMax) {
			rankMax = rank_error;
		}
	}

	// printf("Thread %zu wasted work: %lu actual work: %lu rankMax: %lu\n",
	// tid, 	   wasted_work, actual_work, rankMax); printf("percent wasted
	// work:
	// %.2f%%\n\n", 	   (double)(wasted_work * 100) / (wasted_work +
	// actual_work));

	return {rankMax, rankSum};
}
std::pair<uint64_t, uint64_t>
ParallelBatchImp::calcMaxSumErrorBatch(SubProblem problem, size_t tid) {

	uint64_t get_stamps_size = problem.getValues.size();
	uint64_t batch_size = 10000;
	uint32_t ins_ix = 0;
	uint32_t del_ix = 0;

	uint64_t rank_sum = 0;
	uint64_t rank_max = 0;
	// set rank_min to the maximum value of an unsigned int
	uint64_t rank_min = UINT64_MAX;

	bool keep_running = true;
	size_t batches_done = 0;

	size_t total_batches_needed = get_stamps_size / batch_size;

	auto head = problem.puts2;
	while (keep_running) {

		// map from value to insert order 0...n
		std::unordered_map<uint64_t, uint64_t> pops;
		/* Do deletions. */
		uint64_t ins = 0;
		while (ins < batch_size) {
			uint64_t val = problem.getValues[del_ix++];
			pops.insert({val, ins++});
			if (del_ix >= get_stamps_size) {
				keep_running = false;
				break;
			}
		}

		int dels = pops.size();
		// map of all pops and where they were found
		ordered_set<int, std::less<int>> found_pops;

		uint64_t found = 0;
		while (found < dels && pops.contains(head->value)) {
			uint64_t pop_order = pops.at(head->value);
			uint64_t fi = found_pops.order_of_key(pop_order);
			found_pops.insert(pop_order);
			uint64_t rank_error = found - fi;

			bool isNonCounting = problem.non_counting_puts.find(head->value) !=
								 problem.non_counting_puts.end();

			if (!isNonCounting) {
				rank_error += problem.const_error;
				rank_sum += rank_error;
				if (rank_error > rank_max)
					rank_max = rank_error;
				if (rank_error < rank_min)
					rank_min = rank_error;
			}

			head = head->next;
			found++;
		}
		uint64_t idx = found;
		auto current = head;
		while (found < dels) {
			idx++;
			if (pops.contains(current->next->value)) {

				uint64_t pop_order = pops.at(current->next->value);
				uint64_t rank = idx;
				// fi is the ammount of pops that are in fron of me in the q and
				// in time.
				uint64_t fi = found_pops.order_of_key(pop_order);
				found_pops.insert(pop_order);
				assert(rank >= fi);
				uint64_t rank_error = rank - fi;

				bool isNonCounting =
					problem.non_counting_puts.find(current->next->value) !=
					problem.non_counting_puts.end();

				if (!isNonCounting) {
					rank_error += problem.const_error;
					rank_sum += rank_error;
					if (rank_error > rank_max)
						rank_max = rank_error;
					if (rank_error < rank_min)
						rank_min = rank_error;
				}

				current->next = current->next->next;
				found++;
			} else {
				current = current->next;
			}
		}
	}
	const double rank_mean = (double)rank_sum / get_stamps_size;

	return {rank_max, rank_sum};
}

AbstractExecutor::Measurement ParallelBatchImp::calcMaxMeanError() {
	auto func = [this](size_t tid,
					   std::pair<uint64_t, uint64_t> *results) -> void {
		*results = calcMaxSumErrorBatch(subProblems.at(tid), tid);
	};

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

void ParallelBatchImp::splitOnTime(const std::vector<Interval> &intervals,
								   uint64_t max_time) {
	printf("Max time: %lu\n", max_time);
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
			subProblems[i].intervals.push_back(idx);
			if (interv.end >= curr_end && i != numThreads - 1) {
				int next_i = i + 1;
				int64_t next_start = start_times[next_i];
				while (next_i < numThreads && next_start < interv.end) {
					if (interv.end == ~0) {
						subProblems[next_i].const_error++;
					} else {
						subProblems[next_i].non_counting_puts.insert(
							interv.value);
						subProblems[next_i].intervals.push_back(idx);
						subProblems[next_i].puts.push_back(interv.value);
					}

					next_i++;
					next_start = start_times[next_i];
				}
			}
			idx++;
		}
		std::ranges::sort(
			subProblems[i].intervals,
			[intervals](const size_t &left, const size_t &right) -> bool {
				return intervals[left].end < intervals[right].end;
			});

		for (auto &idx : subProblems[i].intervals) {
			auto &intv = intervals.at(idx);
			if (intv.end < max_time + 1)
				subProblems[i].getValues.push_back(intv.value);
		}

		size_t nItems = subProblems[i].puts.size();
		subProblems[i].puts2 =
			static_cast<Item *>(calloc(nItems, sizeof(Item)));
		auto putsiter = subProblems[i].puts.begin();
		for (size_t j = 0; j < nItems; ++j) {
			Item *item = &subProblems[i].puts2[j];
			item->value = *putsiter;
			item->next = j < nItems - 1 ? &subProblems[i].puts2[j + 1] : NULL;
			std::advance(putsiter, 1);
		}
	}
}

void ParallelBatchImp::splitOnWorkPar(const std::vector<Interval> &intervals,
									  uint64_t max_time) {
	subProblems.resize(numThreads);
	uint64_t intervals_per_thread = numGets / numThreads;
	uint64_t intervals_for_last_thread =
		numGets - intervals_per_thread * (numThreads - 1);

	uint unpoped_intervals = intervals.size() - numGets;

	std::vector<int64_t> start_times(numThreads);
	std::vector<int64_t> end_times(numThreads);
	std::vector<uint64_t> start_idx(numThreads);
	std::vector<uint64_t> end_idx(numThreads);
	uint64_t idx = 0;
	for (int i = 0; i < numThreads; i++) {

		uint64_t added_intervals = 0;
		uint64_t intervals_to_add = i != numThreads - 1
										? intervals_per_thread
										: intervals_for_last_thread;
		int64_t curr_start =
			i == 0 ? intervals.at(0).start : end_times[i - 1] + 1;
		start_idx[i] = idx;
		int64_t curr_end = intervals.at(idx).end;

		while (added_intervals < intervals_to_add && idx < intervals.size()) {

			auto &interv = intervals.at(idx);
			if (interv.end != ~0) {
				added_intervals++;
				curr_end = interv.start + 1;
			}
			idx++;
		}
		start_times[i] = curr_start;
		end_times[i] = curr_end;
		if (i == 0) {
			start_times[i] = curr_start - 1;
		}
		if (i == numThreads - 1) {
			end_times[i] = curr_end + 1;
		}
	}
	idx = 0;
	// this can be parallelized
	auto func = [this, intervals, start_times, end_times](
					size_t tid, uint64_t start_idx,
					std::vector<std::mutex> &subProblemMutexes) -> void {
		uint64_t idx = start_idx;
		int64_t curr_start = start_times[tid];
		int64_t curr_end = end_times[tid];

		subProblems[tid].start_time = curr_start;
		subProblems[tid].end_time = curr_end;
		while (idx < intervals.size() && intervals.at(idx).start < curr_end) {
			auto &interv = intervals.at(idx);
			// subProblems[i].puts.push_back(interv.value);
			{
				std::lock_guard<std::mutex> lock(subProblemMutexes[tid]);
				subProblems[tid].intervals.push_back(idx);
			}
			if (interv.end >= curr_end && tid != numThreads - 1) {
				int next_i = tid + 1;
				int64_t next_start = start_times[next_i];
				while (next_i < numThreads && next_start < interv.end) {

					if (interv.end == ~0) {
						std::lock_guard<std::mutex> lock(
							subProblemMutexes[next_i]);
						subProblems[next_i].const_error++;
					} else {
						std::lock_guard<std::mutex> lock(
							subProblemMutexes[next_i]);
						subProblems[next_i].non_counting_puts.insert(
							interv.value);
						subProblems[next_i].intervals.push_back(idx);
						// subProblems[next_i].non_counting_intervals.push_back(interv);
						// subProblems[next_i].puts.push_back(interv.value);
					}

					next_i++;
					next_start = start_times[next_i];
				}
			}
			idx++;
		}
	};

	std::vector<std::thread> threads{};
	std::vector<std::mutex> subProblemMutexes(numThreads);
	for (size_t i = 0; i < numThreads; ++i) {
		std::thread thread(func, i, start_idx[i], std::ref(subProblemMutexes));
		threads.push_back(std::move(thread));
	}
	for (size_t i = 0; i < numThreads; ++i) {
		threads.at(i).join();
	}

	for (int i = 0; i < numThreads; i++) {
		// sort on start times to make sure that the puts are in order
		std::ranges::sort(
			subProblems[i].intervals,
			[intervals](const size_t &left, const size_t &right) -> bool {
				return intervals[left].start < intervals[right].start;
			});
		for (auto &idx : subProblems[i].intervals) {
			auto &intv = intervals.at(idx);
			subProblems[i].puts.push_back(intv.value);
		}
		std::ranges::sort(
			subProblems[i].intervals,
			[intervals](const size_t &left, const size_t &right) -> bool {
				return intervals[left].end < intervals[right].end;
			});

		for (auto &idx : subProblems[i].intervals) {
			auto &intv = intervals.at(idx);
			if (intv.end < max_time + 1)
				subProblems[i].getValues.push_back(intv.value);
		}

		size_t nItems = subProblems[i].puts.size();
		subProblems[i].puts2 =
			static_cast<Item *>(calloc(nItems, sizeof(Item)));
		auto putsiter = subProblems[i].puts.begin();
		for (size_t j = 0; j < nItems; ++j) {
			Item *item = &subProblems[i].puts2[j];
			item->value = *putsiter;
			item->next = j < nItems - 1 ? &subProblems[i].puts2[j + 1] : NULL;
			std::advance(putsiter, 1);
		}
	}
}
void ParallelBatchImp::splitOnWork(const std::vector<Interval> &intervals,
								   uint64_t max_time) {
	subProblems.resize(numThreads);
	uint64_t intervals_per_thread = numGets / numThreads;
	uint64_t intervals_for_last_thread =
		numGets - intervals_per_thread * (numThreads - 1);

	uint unpoped_intervals = intervals.size() - numGets;

	std::vector<int64_t> start_times(numThreads);
	std::vector<int64_t> end_times(numThreads);
	uint64_t idx = 0;
	for (int i = 0; i < numThreads; i++) {

		uint64_t added_intervals = 0;
		uint64_t intervals_to_add = i != numThreads - 1
										? intervals_per_thread
										: intervals_for_last_thread;
		int64_t curr_start =
			i == 0 ? intervals.at(0).start : end_times[i - 1] + 1;
		int64_t curr_end = intervals.at(idx).end;

		while (added_intervals < intervals_to_add && idx < intervals.size()) {
			auto &interv = intervals.at(idx);
			if (interv.end != ~0) {
				added_intervals++;
				curr_end = interv.start + 1;
			}
			idx++;
		}
		start_times[i] = curr_start;
		end_times[i] = curr_end;
		if (i == 0) {
			start_times[i] = curr_start - 1;
		}
		if (i == numThreads - 1) {
			end_times[i] = curr_end + 1;
		}
	}
	idx = 0;
	for (int i = 0; i < numThreads; i++) {

		int64_t curr_start = start_times[i];
		int64_t curr_end = end_times[i];

		subProblems[i].start_time = curr_start;
		subProblems[i].end_time = curr_end;
		while (idx < intervals.size() && intervals.at(idx).start < curr_end) {
			auto &interv = intervals.at(idx);
			subProblems[i].puts.push_back(interv.value);
			subProblems[i].intervals.push_back(idx);
			if (interv.end >= curr_end && i != numThreads - 1) {
				int next_i = i + 1;
				int64_t next_start = start_times[next_i];
				while (next_i < numThreads && next_start < interv.end) {
					if (interv.end == ~0) {
						subProblems[next_i].const_error++;
					} else {
						subProblems[next_i].non_counting_puts.insert(
							interv.value);
						subProblems[next_i].intervals.push_back(idx);
						subProblems[next_i].puts.push_back(interv.value);
					}

					next_i++;
					next_start = start_times[next_i];
				}
			}
			idx++;
		}
		std::ranges::sort(
			subProblems[i].intervals,
			[intervals](const size_t &left, const size_t &right) -> bool {
				return intervals[left].end < intervals[right].end;
			});

		for (auto &idx : subProblems[i].intervals) {
			auto &intv = intervals.at(idx);
			if (intv.end < max_time + 1)
				subProblems[i].getValues.push_back(intv.value);
		}

		size_t nItems = subProblems[i].puts.size();
		subProblems[i].puts2 =
			static_cast<Item *>(calloc(nItems, sizeof(Item)));
		auto putsiter = subProblems[i].puts.begin();
		size_t k = 0;
		for (size_t j = 0; j < nItems; ++j) {
			Item *item = &subProblems[i].puts2[j];
			item->value = *putsiter;
			item->next = j < nItems - 1 ? &subProblems[i].puts2[j + 1] : NULL;
			std::advance(putsiter, 1);
		}
	}
}

void ParallelBatchImp::prepare(const InputData &data) {
	auto puts = data.getPuts();
	intervals.resize(puts->size());
	std::unordered_map<uint64_t, size_t> putMap{};
	uint64_t max_time = 0;
	for (size_t i = 0; i < puts->size(); ++i) {
		auto &put = puts->at(i);
		auto &intv = intervals.at(i);
		intv.start = put.time;
		intv.end = ~0;
		intv.value = put.value;
		putMap[put.value] = i;
	}
	auto gets = data.getGets();
	numGets = gets->size();
	for (size_t i = 0; i < numGets; ++i) {
		auto &get = gets->at(i);
		auto &interval = intervals.at(putMap[get.value]);
		interval.end = get.time;
		if (interval.end > max_time && interval.end != ~0) {
			max_time = interval.end;
		}
	}
	numThreads = 12;
	if (useParSplit) {
		splitOnWorkPar(intervals, max_time);
	} else {
		splitOnWork(intervals, max_time);
	}
}

AbstractExecutor::Measurement ParallelBatchImp::execute() {
	return calcMaxMeanError();
}
void ParallelBatchImp::reset() {
	intervals.clear();
	numGets = 0;
	subProblems.clear();
}
} // namespace bench