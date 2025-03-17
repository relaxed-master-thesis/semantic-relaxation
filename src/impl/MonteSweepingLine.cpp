#include "bench/impl/MonteSweepingLine.h"
#include "bench/Benchmark.h"
#include "bench/Operation.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <sys/types.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace bench {

AbstractExecutor::Measurement MonteSweepingLine::calcMaxMeanError() {
	uint64_t rank_sum = 0, rank_max = 0, counted_rank = 0, const_error = 0;

	for (auto &event : events) {
		if (event.end_time == ~0) {
			const_error++;
		} else if (event.type == EventType::START) {
			uint64_t rank = end_tree.order_of_key(event.end_time);
			rank += const_error;
			rank /= counting_share;
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

void MonteSweepingLine::prepare(const InputData &data) {
	auto gets = data.getGets();
	get_stamps_size = gets->size();
	std::unordered_map<uint64_t, size_t> getMap{};
	// for (const Operation &get : *gets) {
	size_t num_gets = gets->size() * counting_share * 100;
	for (size_t i = 0; i < gets->size(); ++i) {
		const Operation &get = gets->at(i);
		getMap[get.value] = get.time;
	}
	auto puts = data.getPuts();
	size_t num_puts = puts->size() * counting_share;
	events.resize(num_puts * 2);
	for (size_t i = 0; i < num_puts; ++i) {
		size_t idx = xorshf96() % (puts->size() - i);
		const Operation &put = puts->at(idx);
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
		events.at(+i * 2) = start_event;

		Event end_event;

		end_event.type = EventType::END;
		end_event.time = end_time;
		end_event.start_time = put.time;
		end_event.end_time = end_time;
		// events.push_back(end_event);
		events.at(i * 2 + 1) = end_event;
	}

	// sort events by time, if equals, end comes first
	std::sort(events.begin(), events.end(),
			  [](const Event &left, const Event &right) -> bool {
				  return left.time < right.time;
			  });
}

AbstractExecutor::Measurement MonteSweepingLine::execute() {
	return calcMaxMeanError();
}

} // namespace bench