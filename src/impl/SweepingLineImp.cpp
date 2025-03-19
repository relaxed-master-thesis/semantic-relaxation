#include "bench/impl/SweepingLineImp.h"
#include "bench/Benchmark.h"
#include "bench/Operation.h"

#include <cstdint>
#include <cstdio>
#include <sys/types.h>
#include <unordered_map>
#include <unordered_set>

namespace bench {

AbstractExecutor::Measurement SweepingLineImp::calcMaxMeanError() {
	uint64_t rank_sum = 0, rank_max = 0, counted_rank = 0, const_error = 0;
	// int pops_to_calc = events.size() *0.01;

	for (auto &event : events) {
		if (event.end_time == ~0) {
			const_error++;
		} else if (event.type == EventType::START) {
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
		// if(counted_rank >= pops_to_calc) {
		// 	break;
		// }
	}
	return {rank_max, (double)rank_sum / counted_rank};
}

void SweepingLineImp::prepare(const InputData &data) {
	auto gets = data.getGets();
	get_stamps_size = gets->size();
	std::unordered_map<uint64_t, size_t> getMap{};
	for (const Operation &get : *gets) {
		getMap[get.value] = get.time;
	}
	auto puts = data.getPuts();
	for (const Operation &put : *puts) {
		uint64_t end_time = 0;
		if (getMap.find(put.value) == getMap.end()) {
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

AbstractExecutor::Measurement SweepingLineImp::execute() {
	return calcMaxMeanError();
}
void SweepingLineImp::reset(){
	events.clear();
	get_stamps = nullptr;
	get_stamps_size = 0;
	put_stamps = nullptr;
	end_tree.clear();
}
} // namespace bench