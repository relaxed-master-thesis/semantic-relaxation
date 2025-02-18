#include "bench/impl/ReplayImp.h"
#include "bench/Benchmark.h"
#include "bench/Operation.h"
#include "bench/util/VectorQueue.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>

namespace bench {
ErrorCalculator::Result ReplayImp::calcMaxMeanError() {

	if (get_stamps_size == 0)
		return {0, 0};

	uint64_t next_ins_tick = put_stamps->at(0).time;
	uint32_t ins_ix = 0;
	uint64_t next_del_tick = get_stamps->at(0).time;
	uint32_t del_ix = 0;
	assert(next_ins_tick < next_del_tick);

	uint64_t rank_sum = 0;
	uint64_t rank_max = 0;
	std::vector<uint64_t> ranks;

	bool keep_running = true;
	VectorQueue<uint64_t> q;
	while (keep_running) {
		assert(ins_ix < put_stamps_size);
		assert(next_ins_tick <= next_del_tick);

		/* Do insertions. */
		while (ins_ix < put_stamps_size && next_ins_tick <= next_del_tick) {
			Operation &insert = (*put_stamps)[ins_ix++];
			q.enq(insert.value);

			if (ins_ix >= put_stamps_size) {
				next_ins_tick = std::numeric_limits<uint64_t>::max();
				break;
			}

			next_ins_tick = (*put_stamps)[ins_ix].time;
		}

		/* Do deletions. */
		while (next_del_tick < next_ins_tick) {

			Operation &deleted_item = (*get_stamps)[del_ix++];

			/* Look up the key. */
			uint64_t rank;
			q.deq(deleted_item.value, &rank);

			ranks.push_back(rank);

			rank_sum += rank;
			rank_max = std::max(rank_max, rank);

			if (del_ix >= get_stamps_size) {
				keep_running = false;
				break;
			}

			next_del_tick = (*get_stamps)[del_ix].time;
		}
	}

	const double rank_mean = (double)rank_sum / ranks.size();

	double rank_squared_difference = 0;
	for (size_t idx = 0; idx < ranks.size(); idx++) {
		uint64_t rank = ranks[idx];
		rank_squared_difference += std::pow(rank - rank_mean, 2);
	}

	const double rank_stddev =
		std::sqrt(rank_squared_difference / ranks.size());

	return {rank_max, rank_mean};
}

void ReplayImp::prepare(InputData data) {
	put_stamps_size = data.puts->size();
	get_stamps_size = data.gets->size();
	put_stamps = data.puts;
	get_stamps = data.gets;
}
long ReplayImp::execute() {
	std::cout << "Running ReplayImp...\n";
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