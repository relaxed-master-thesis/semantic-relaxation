#include "bench/impl/AITImp.h"
#include "bench/Benchmark.h"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>
#include <omp.h>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

namespace bench {

AbstractExecutor::Measurement AITImp::calcMaxMeanError() {

	uint64_t rank_sum = 0;
	uint64_t rank_max = 0;

#pragma omp parallel for
	for (auto i : segments) {
		uint64_t rank = ait.getRank(ait.root, i, 0);
		if (rank > rank_max)
			rank_max = rank;
		rank_sum += rank;
	}

	const double rank_mean = (double)rank_sum / get_stamps_size;

	return {rank_max, rank_mean};
}

void AITImp::fix_dup_timestamps() {
	bool keep_going = true;
	uint64_t next_ins_tick = put_stamps->at(0).time;
	uint32_t ins_ix = 0;
	uint64_t next_del_tick = get_stamps->at(0).time;
	uint32_t del_ix = 0;
	uint64_t time = 0;
	while (keep_going) {
		// fix insert timestamps
		while (ins_ix < put_stamps_size && next_ins_tick <= next_del_tick) {
			next_ins_tick = (*put_stamps)[ins_ix].time;
			(*put_stamps)[ins_ix++].time = time++;
		}
		// fix deletion timestamps
		while (next_del_tick < next_ins_tick) {
			next_del_tick = (*get_stamps)[del_ix].time;
			(*get_stamps)[del_ix++].time = time++;
			if (del_ix >= get_stamps_size) {
				keep_going = false;
				break;
			}
		}
	}
}

void AITImp::prepare(const InputData &data) {
	get_stamps_size = data.getGets()->size();
	get_stamps = std::make_shared<std::vector<Operation>>(*data.getGets());
	put_stamps_size = data.getPuts()->size();
	put_stamps = std::make_shared<std::vector<Operation>>(*data.getPuts());
	fix_dup_timestamps();
	for (auto put : *put_stamps) {
		put_map[put.value] = put.time;
	}
	for (auto get : *get_stamps) {
		uint64_t put_time = put_map[get.value];
		segments.emplace_back(std::make_shared<Interval>(put_time, get.time));
	}
	for (auto i : segments) {
		ait.root = ait.insertNode(ait.root, i);
	}
}

AbstractExecutor::Measurement AITImp::execute() { return calcMaxMeanError(); }

} // namespace bench