
#include "bench/impl/stackImpl/ReplayTreeStackImp.hpp"
#include "bench/util/Executor.hpp"
#include "bench/util/FenwickTree.hpp"
#include "bench/util/InputData.hpp"
#include "bench/util/Operation.hpp"
#include <algorithm>
#include <iostream>
#include <unordered_map>

namespace bench {

AbstractExecutor::Measurement ReplayTreeStackImp::calcMaxMeanError() {
	uint64_t rank_max = 0, rank_sum = 0;
	for (auto &event : events) {
		if (event.isPush) {
			push_tree.insert(event.pushOrder);
		} else if (event.hasPop) {
			push_tree.erase(event.pushOrder);
			// how many elements in the stack are pushed after me
			uint64_t rank = push_tree.order_of_key(event.pushOrder);

			rank_sum += rank;
			if (rank > rank_max) {
				rank_max = rank;
			}
		}
	}

	return {rank_max, (double)rank_sum / get_stamps_size};
}
AbstractExecutor::Measurement ReplayTreeStackImp::execute() {
	return calcMaxMeanError();
}

void ReplayTreeStackImp::prepare(const InputData &data) {
	auto puts = data.getPuts();
	auto gets = data.getGets();
	get_stamps_size = gets->size();

	std::unordered_map<int64_t, size_t> getMap{};
	for (size_t i = 0; i < gets->size(); ++i) {
		auto &get = gets->at(i);
		events.emplace_back(false, true, 0, get.time);
		getMap[get.value] = i;
	}
	for (size_t i = 0; i < puts->size(); ++i) {
		auto &put = puts->at(i);
		if (getMap.find(put.value) == getMap.end()) {
			events.emplace_back(true, false, i + 1, put.time);
		} else {
			events.emplace_back(true, true, i + 1, put.time);
			events[getMap[put.value]].pushOrder = i + 1;
		}
	}

	std::sort(events.begin(), events.end(),
			  [](const event &a, const event &b) { return a.time < b.time; });
}
void ReplayTreeStackImp::reset() {
	events.clear();
	get_stamps_size = 0;
	push_tree.clear();
}
} // namespace bench