#include "bench/impl/SweepingLineImp.h"
#include "bench/Benchmark.h"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <sys/types.h>
#include <unordered_map>
#include <unordered_set>

namespace bench {

ErrorCalculator::Result SweepingLineImp::calcMaxMeanError() {
	uint64_t rank_sum = 0, rank_max = 0, counted_rank = 0, const_error = 0;
	for (auto &event : events) {
		if (event.end_time == ~0) {
			const_error++;
			continue;
		}
		if (event.type == EventType::START) {
			uint64_t rank = end_tree.order_of_key(event.end_time);
			rank += const_error;
			rank_sum += rank;
			counted_rank++;
			if (rank > rank_max) {
				rank_max = rank;
			}
			end_tree.insert(event.end_time);
		} else {
			end_tree.erase(event.end_time);
		}
	}
	return {rank_max, (double)rank_sum / counted_rank};
}

void SweepingLineImp::prepare(InputData data) {
	// events.resize(data.puts->size() + data.gets->size());
	get_stamps_size = data.gets->size();
	std::unordered_map<uint64_t, size_t> getMap{};
	std::unordered_set<uint64_t> getSet{};
	for (auto get : *data.gets) {
		getMap[get.value] = get.time;
		getSet.insert(get.value);
	}
	for (auto put : *data.puts) {
		uint64_t end_time = 0;
		if (getSet.find(put.value) == getSet.end()) {
			end_time = ~0;
		} else {
			end_time = getMap[put.value];
		}
		Event start_event;
		start_event.type = EventType::START;
		start_event.time = put.time;
		start_event.start_time = put.time;
		start_event.end_time = end_time;
		events.push_back(start_event);

		Event end_event;

		end_event.type = EventType::END;
		end_event.time = end_time;
		end_event.start_time = put.time;
		end_event.end_time = end_time;
		events.push_back(end_event);
	}
	// sort events by time, if equals, end comes first
	std::sort(events.begin(), events.end(),
			  [](const Event &left, const Event &right) -> bool {
				  return left.time < right.time;
			  });
}

long SweepingLineImp::execute() {
	std::cout << "Running SweepingLineImp...\n";
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