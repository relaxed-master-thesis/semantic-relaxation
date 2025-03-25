#include "bench/impl/FenwickImp.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>
#include <unordered_map>

namespace bench {
void FenwickImp::FenwickTree::update(int64_t idx, int64_t val) {
	while (idx < BIT.size()) {
		BIT[idx] += val;
		idx += idx & -idx;
	}
}

int64_t FenwickImp::FenwickTree::query(int64_t idx) {
	int64_t sum = 0;
	while (idx > 0) {
		sum += BIT[idx];
		idx -= idx & -idx;
	}
	return sum;
}

AbstractExecutor::Measurement FenwickImp::calcMaxMeanError() {
	
	size_t n = pushed_items.size();
	FenwickTree BIT(n);

	std::vector<int64_t> result(n, 0);
	int64_t constError = 0;
	int64_t countedElems = 0;
	int64_t sum = 0;
	int64_t max = 0;
	size_t maxIdx = 0;
	for (int64_t i = 0; i < n; ++i) {
		int64_t pop_time = pushed_items[i].pop_time;

		if (pop_time == std::numeric_limits<int64_t>::max()) {
			++constError;
			continue;
		}
		++countedElems;
		int64_t res = i - BIT.query(pop_time);
		sum += res;
		maxIdx = max < res ? i : maxIdx;
		max = max < res ? res : max;
		BIT.update(pop_time, 1);
	}
	return {static_cast<uint64_t>(max), (double)sum / countedElems};
}

void FenwickImp::prepare(const InputData &data) {
	auto gets = data.getGets();
	auto puts = data.getPuts();

	std::unordered_map<int64_t, size_t> putMap{};

	for (size_t i = 0; i < puts->size(); ++i) {
		auto &put = puts->at(i);
		pushed_items.push_back({std::numeric_limits<int64_t>::max()});
		putMap[put.value] = i;
	}
	for (size_t i = 0; i < gets->size(); ++i) {
		auto &get = gets->at(i);
		if(putMap.find(get.value) != putMap.end()) {
			pushed_items[putMap[get.value]].pop_time = i + 1;
		}
	}
}

AbstractExecutor::Measurement FenwickImp::execute() {
	return calcMaxMeanError();
}
void FenwickImp::reset() { pushed_items.clear(); }
} // namespace bench