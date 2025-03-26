#include "bench/impl/FenwickAImp.hpp"
#include "bench/util/FenwickTree.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <limits>
#include <unordered_map>
#include <unordered_set>

namespace bench {
AbstractExecutor::Measurement FenwickAImp::calcMaxMeanError() {
	int64_t n = static_cast<int64_t>(pushed_items.size());
	FenwickTree<int64_t> BIT(n);
	std::vector<int64_t> result(n, 0);
	int64_t constError = 0;
	int64_t countedElems = 0;
	int64_t sum = 0;
	int64_t max = 0;

	for (int64_t i = 0; i < n; ++i) {
		int64_t pop_time = pushed_items[i].pop_time;

		if (pop_time == std::numeric_limits<int64_t>::max()) {
			++constError;
			continue;
		}
		++countedElems;
		int64_t res = BIT.query(pop_time) + constError;
		sum += res;
		max = max < res ? res : max;
		BIT.update(pop_time, 1);
	}

	return {static_cast<uint64_t>(max), (double)sum / countedElems};
}

void FenwickAImp::prepare(const InputData &data) {
	auto gets = data.getGets();
	auto puts = data.getPuts();

	std::unordered_map<int64_t, int> getMap{};
	int64_t time = 0;

	size_t numGets = gets->size() * counting_share;
	if (counting_type == CountingType::AMOUNT) {
		numGets = counting_amount;
		numGets = std::min(numGets, gets->size());
	}
	for (size_t i = 0; i < numGets; ++i) {
		const auto &get = gets->at(i);
		getMap[get.value] = i + 1;
	}

	size_t gets_found = 0;
	size_t i = 0;

	int64_t min_get_time = puts->size() + 1;
	while (gets_found <= numGets && i < puts->size()) {
		const auto &put = puts->at(i);
		if (getMap.contains(put.value)) {
			gets_found++;
			pushed_items.push_back({getMap[put.value]});
		} else {
			pushed_items.push_back({std::numeric_limits<int64_t>::max()});
		}
		++i;
	}
}

AbstractExecutor::Measurement FenwickAImp::execute() {
	return calcMaxMeanError();
}
void FenwickAImp::reset() { pushed_items.clear(); }
} // namespace bench