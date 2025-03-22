#include "bench/impl/ReplayImp.hpp"
#include "bench/Operation.hpp"
#include "bench/util/Executor.hpp"
#include "bench/util/VectorQueue.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>

namespace bench {
AbstractExecutor::Measurement ReplayImp::calcMaxMeanError() {

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
			const Operation &insert = (*put_stamps)[ins_ix++];
			q.enq(insert.value);

			if (ins_ix >= put_stamps_size) {
				next_ins_tick = std::numeric_limits<uint64_t>::max();
				break;
			}

			next_ins_tick = (*put_stamps)[ins_ix].time;
		}

		/* Do deletions. */
		while (next_del_tick < next_ins_tick) {

			const Operation &deleted_item = (*get_stamps)[del_ix++];

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

void ReplayImp::prepare(const InputData &data) {
	put_stamps = data.getPuts();
	get_stamps = data.getGets();
	put_stamps_size = put_stamps->size();
	get_stamps_size = get_stamps->size();
}

AbstractExecutor::Measurement ReplayImp::execute() {
	return calcMaxMeanError();
}
void ReplayImp::reset() {
	put_stamps = nullptr;
	get_stamps = nullptr;
	put_stamps_size = 0;
	get_stamps_size = 0;
}
} // namespace bench