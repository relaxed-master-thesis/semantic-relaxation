#include "AITImp.h"
#include "Benchmark.h"
#include "Queue.h"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>
#include <numeric>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
#include <omp.h>
#include <queue>

namespace bench {
uint64_t AITImp::getRank(Interval &interval) {
	std::queue<size_t> toVisit{};
	toVisit.push(0);

	uint64_t rank = 0;
	while (!toVisit.empty()) {
		size_t i = toVisit.front();
		toVisit.pop();
		Interval &data = tree.getNode(i);

		if (data.start < interval.start && data.end > interval.end)
			++rank;
		if (tree.hasLeftChild(i)) {
			Interval &left = tree.getNode(tree.leftChild(i));
			if (left.max >= interval.start)
				toVisit.push(tree.leftChild(i));
		}
		if (tree.hasRightChild(i)) {
			Interval &right = tree.getNode(tree.rightChild(i));
			if (right.min <= interval.end)
				toVisit.push(tree.rightChild(i));
		}
	}
	
	return rank;
}

ErrorCalculator::Result AITImp::calcMaxMeanError() {

	uint64_t rank_sum = 0;
	uint64_t rank_max = 0;

// #pragma omp parallel for
	for (auto i : segments) {
		uint64_t rank = getRank(i);
		if (rank > rank_max)
			rank_max = rank;
		rank_sum += rank;
	}
	std::cout << "max: " << rank_max << ", stamps: " << get_stamps_size << "\n";
	const double rank_mean = (double)rank_sum / get_stamps_size;

	return {rank_max, rank_mean};
}

// Fix this why is it so slow???
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

void AITImp::prepare(InputData data) {
	put_stamps_size = data.puts->size();
	get_stamps_size = data.gets->size();
	put_stamps = data.puts;
	get_stamps = data.gets;
	fix_dup_timestamps();
	for (auto put : *put_stamps) {
		put_map[put.value] = put.time;
	}
	for (auto get : *get_stamps) {
		uint64_t put_time = put_map[get.value];
		segments.emplace_back(put_time, get.time);
	}
	tree.build(segments);
}

long AITImp::execute() {
	std::cout << "Running AITImp...\n";
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