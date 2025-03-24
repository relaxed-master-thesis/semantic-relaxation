#include "bench/impl/SweepingLineAImp.hpp"
#include "bench/util/Operation.hpp"
#include "bench/util/Executor.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <sys/types.h>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace bench {

AbstractExecutor::Measurement SweepingLineAImp::calcMaxMeanError() {
	uint64_t rank_sum = 0, rank_max = 0, counted_rank = 0, const_error = 0;

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
	}
	return {rank_max, (double)rank_sum / counted_rank};
}

void SweepingLineAImp::prep_box(const InputData &data) {
	struct Point {
		uint64_t push_time;
		uint64_t pop_time;
	};
	auto gets = data.getGets();
	uint64_t min_pop_time = gets->front().time;
	std::unordered_map<uint64_t, uint64_t> getMap{};
	for (const Operation &get : *gets) {
		getMap[get.value] = get.time - min_pop_time;
	}
	auto puts = data.getPuts();
	std::vector<Point> points{};
	for (auto put : *puts) {
		if (getMap.find(put.value) != getMap.end()) {
			points.push_back({put.time, getMap[put.value]});
		} else {
			points.push_back({put.time, ~0UL});
		}
	}
	bool keep_going = true;
	size_t idx = 0;
	bool stable = false;
	size_t stable_count = 0;
	uint64_t pop_sum = 0;
	float avg = 0;
	uint64_t limit_pop_time = 0;
	uint64_t max_pop_time = 0;
	std::vector<Point> box{};
	while (keep_going && idx < points.size()) {
		auto point = points[idx];
		pop_sum += point.pop_time;
		avg = (float)pop_sum / (idx + 1);
		//if point.pop_time is more than a 10 percent higher than the max pop time
		max_pop_time = std::max(max_pop_time, point.pop_time);
		
		if (stable && point.pop_time > limit_pop_time * 1.9) {
			std::cout << "breaking at pop time: " << point.pop_time << std::endl;
			keep_going = false;
			break;
		}
		if(point.pop_time > limit_pop_time * 1.5){
			std::cout << "pop time: " << point.pop_time << "(x, y): (" << point.push_time << ", " << point.pop_time << ")\n";
			limit_pop_time = std::max(limit_pop_time, point.pop_time);
		}
		if (point.pop_time < limit_pop_time * 1.1) {
			stable_count++;
		} else if (!stable) {
			stable_count = 0;
		}
		stable = stable_count > 10000;
		if (stable && point.pop_time != limit_pop_time) {
			// std::cout << "stable at: " << idx  << "with max pop time: " << max_pop_time << std::endl;

		}
		box.push_back(point);
		idx++;
	}
	// // print pop time of last point in box
	std::cout << "last pop time: " << box.back().pop_time << std::endl;
	// // print size of box
	std::cout << "box size: " << box.size() << std::endl;
	// //print put stamps size
	std::cout << "put stamps size: " << puts->size() << std::endl;
	// // print avg pop time
	std::cout << "avg pop time: " << avg << std::endl;
	// //print max pop time	
	std::cout << "max pop time: " << max_pop_time << std::endl;
	// //print limit pop time
	std::cout << "limit pop time: " << limit_pop_time << std::endl;
	keep_going = true;
	idx = 0;
	for (auto point : box) {
		uint64_t start_time = point.push_time;
		uint64_t end_time = point.pop_time;

		Event start_event;
		start_event.type = EventType::START;
		start_event.time = start_time;
		start_event.start_time = start_time;
		start_event.end_time = end_time;
		events.push_back(start_event);

		Event end_event;

		end_event.type = EventType::END;
		end_event.time = end_time;
		end_event.start_time = start_time;
		end_event.end_time = end_time;
		events.push_back(end_event);
		idx++;
	}
	// sort events by time, if equals, end comes first
	std::sort(events.begin(), events.end(),
			  [](const Event &left, const Event &right) -> bool {
				  return left.time < right.time;
			  });
}
void SweepingLineAImp::prepare(const InputData &data) {
	auto gets = data.getGets();
	get_stamps_size = gets->size();
	std::unordered_map<uint64_t, size_t> getMap{};
	std::unordered_set<uint64_t> getSet{};
	// get n% of gets
	int gets_to_count = gets->size() * counting_share;
	if (counting_type == CountingType::AMOUNT) {
		gets_to_count = counting_amount;
		gets_to_count = std::min(gets_to_count, (int)gets->size());
	}

	int counted_gets = 0;
	for (const Operation &get : *gets) {
		getMap[get.value] = get.time;
		getSet.insert(get.value);
		counted_gets++;
		if (counted_gets >= gets_to_count) {
			break;
		}
	}
	auto puts = data.getPuts();
	int counted_puts = 0;
	for (const Operation &put : *puts) {
		uint64_t end_time = 0;
		if (getSet.find(put.value) == getSet.end()) {
			end_time = ~0;
		} else {
			end_time = getMap[put.value];
			counted_puts++;
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
		if (counted_puts >= gets_to_count) {
			break;
		}
	}
	// sort events by time, if equals, end comes first
	std::sort(events.begin(), events.end(),
			  [](const Event &left, const Event &right) -> bool {
				  return left.time < right.time;
			  });
}

AbstractExecutor::Measurement SweepingLineAImp::execute() {
	return calcMaxMeanError();
}
void SweepingLineAImp::reset() {
	events.clear();
	get_stamps = nullptr;
	get_stamps_size = 0;
	put_stamps = nullptr;
	end_tree.clear();
}
} // namespace bench