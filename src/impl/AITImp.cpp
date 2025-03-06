#include "bench/impl/AITImp.h"
#include "bench/Benchmark.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <memory>
#include <omp.h>
#include <queue>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

namespace bench {

AbstractExecutor::Measurement AITImp::calcMaxMeanError() {

	uint64_t rank_sum = 0;
	uint64_t rank_max = 0;

	auto getRank = [this](const Interval &intv) -> uint64_t {
		uint64_t errs = 0;

		std::queue<size_t> toVisit{};
		toVisit.push(0);
		while (!toVisit.empty()) {
			size_t idx = toVisit.front();
			toVisit.pop();

			if (idx >= intervals.size() || idx < 0)
				continue;

			const Interval &curr = ivt.getNode(idx);

			if (curr.start < intv.start && intv.end < curr.end) {
				errs += 1;
			}

			if (ivt.hasLeftChild(idx)) {
				size_t lc = ivt.leftChild(idx);
				const auto &currc = ivt.getNode(lc);
				if (currc.max >= intv.end) {
					toVisit.push(lc);
				}
			}

			if (ivt.hasRightChild(idx)) {
				size_t rc = ivt.rightChild(idx);
				const auto &currc = ivt.getNode(rc);
				if (currc.min <= intv.start) {
					toVisit.push(rc);
				}
			}
		}

		return errs;
	};

#pragma omp parallel for reduction(+ : rank_sum) reduction(max: rank_max)
	for (auto &intv : intervals) {
		uint64_t rank = getRank(intv);
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
	put_stamps = std::make_shared<std::vector<Operation>>(*data.getPuts());
	get_stamps = std::make_shared<std::vector<Operation>>(*data.getGets());
	put_stamps_size = put_stamps->size();
	get_stamps_size = get_stamps->size();
	intervals.resize(data.getPuts()->size(), Interval{0, 0, 0, 0});
	for (size_t i = 0; i < put_stamps_size; ++i) {
		auto &intv = intervals.at(i);
		auto &put = put_stamps->at(i);
		intv.start = i + 1;
		intv.end = std::numeric_limits<uint64_t>::max();
		intv.max = std::numeric_limits<uint64_t>::max();
		intv.min = i + 1;
		put_map[put.value] = i;
	}
	uint64_t time = put_stamps_size + 1;
	for (size_t i = 0; i < get_stamps_size; ++i) {
		auto &get = get_stamps->at(i);
		auto &intv = intervals.at(put_map[get.value]);
		intv.end = time + i;
		intv.max = time + i;
	}
	ivt.cbuild(intervals);

	// fix max vals
	/*
		0
		1 2
		3 4 5 6
		...
	*/
	auto &tree = ivt.getArr();
	for (size_t i = tree.size() - 1; i > 0; --i) {
		auto &intv = tree.at(i);
		auto &parent = tree.at(ivt.getParent(i));
		parent.max = std::max(parent.max, intv.max);
		parent.min = std::min(parent.min, intv.min);
	}
}

AbstractExecutor::Measurement AITImp::execute() { return calcMaxMeanError(); }

} // namespace bench