#include "bench/impl/BatchPopImp.hpp"
#include "bench/Operation.hpp"
#include "bench/util/Executor.hpp"
#include "bench/util/VectorQueue.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <numeric>
#include <unordered_map>
#include <vector>

namespace bench {
AbstractExecutor::Measurement BatchPopImp::calcMaxMeanError() {

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
	int s = 0;
	VectorQueue<uint64_t> q;
	uint64_t max_pop_seq = 0;
	while (keep_running) {
		assert(ins_ix < put_stamps_size);
		assert(next_ins_tick <= next_del_tick);

		/* Do insertions. */
		while (ins_ix < put_stamps_size && next_ins_tick <= next_del_tick) {
			const Operation &insert = (*put_stamps)[ins_ix++];
			q.enq(insert.value);

			if (ins_ix >= put_stamps_size) {
				next_ins_tick = std::numeric_limits<uint64_t>::max();
				break;
			}

			next_ins_tick = (*put_stamps)[ins_ix].time;
		}

		// map from value to insert order 0...n
		std::unordered_map<uint64_t, uint64_t> pops;
		/* Do deletions. */
		uint64_t ins = 0;
		while (next_del_tick < next_ins_tick && del_ix < get_stamps_size) {
			uint64_t val = (*get_stamps)[del_ix++].value;
			// if(pops.contains(val)){
			//     std::cout << "value already in map " << val << ", MAYDAY
			//     MAYDAY WE ARE GOING TO CRASH\n";
			// 	throw std::invalid_argument("We cant have duplicate elements in
			// one continous pop sequence");
			// }
			pops.insert({val, ins++});
			if (del_ix >= get_stamps_size) {
				keep_running = false;
				break;
			}

			next_del_tick = (*get_stamps)[del_ix].time;
		}
		max_pop_seq = std::max(max_pop_seq, ins);

		std::vector<uint64_t> rs;
		rs.resize(pops.size());
		q.batch_deq(pops, &rs);
		rank_sum += std::reduce(rs.begin(), rs.end());
		rank_max = std::max(rank_max, *std::max_element(rs.begin(), rs.end()));
		ranks.insert(ranks.end(), rs.begin(), rs.end());
	}

	const double rank_mean = (double)rank_sum / ranks.size();

	double rank_squared_difference = 0;
	for (size_t idx = 0; idx < ranks.size(); idx++) {
		uint64_t rank = ranks[idx];
		rank_squared_difference += std::pow(rank - rank_mean, 2);
	}

	const double rank_stddev =
		std::sqrt(rank_squared_difference / ranks.size());
	printf("Max pop sequence: %lu\n", max_pop_seq);

	return {rank_max, rank_mean};
}

void BatchPopImp::prepare(const InputData &data) {
	put_stamps_size = data.getPuts()->size();
	get_stamps_size = data.getGets()->size();
	put_stamps = data.getPuts();
	get_stamps = data.getGets();
}

AbstractExecutor::Measurement BatchPopImp::execute() {
	return calcMaxMeanError();
}
void BatchPopImp::reset() {
	put_stamps = nullptr;
	get_stamps = nullptr;
	put_stamps_size = 0;
	get_stamps_size = 0;
}
} // namespace bench