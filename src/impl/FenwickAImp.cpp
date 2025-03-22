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
	int64_t n = static_cast<int64_t>(intervals.size());

	std::ranges::sort(
		intervals, [](const SInterval &left, const SInterval &right) -> bool {
			return left.start < right.start;
		});

	std::vector<int64_t> sortedEnds(n, 0);
	for (size_t i = 0; i < n; ++i) {
		sortedEnds[i] = intervals[i].end;
	}

	std::ranges::sort(sortedEnds);
	std::unordered_map<int64_t, int64_t> endIndices{};
	for (int64_t i = 0; i < n; ++i) {
		endIndices[sortedEnds[i]] = i + 1;
	}

	FenwickTree<int64_t> BIT(n);
	std::vector<int64_t> result(n, 0);
	int64_t constError = 0;
	int64_t countedElems = 0;
	int64_t sum = 0;
	int64_t max = 0;

	for (int64_t i = 0; i < n; ++i) {
		int64_t endVal = intervals[i].end;

		if (endVal == std::numeric_limits<int64_t>::max()) {
			++constError;
			continue;
		}
		++countedElems;
		int64_t comprEnd = endIndices[endVal];

		int64_t res = BIT.query(n) - BIT.query(comprEnd) + constError;
		sum += res;
		max = max < res ? res : max;
		BIT.update(comprEnd, 1);
	}

	return {static_cast<uint64_t>(max), (double)sum / countedElems};
}

void FenwickAImp::prepare(const InputData &data) {
	auto gets = data.getGets();
	auto puts = data.getPuts();

	std::unordered_map<int64_t, size_t> getMap{};
	int64_t time = 0;

	size_t numGets = gets->size() * counting_share;
	if (counting_type == CountingType::AMOUNT) {
		numGets = counting_amount;
		numGets = std::min(numGets, gets->size());
	}
	for (size_t i = 0; i < numGets; ++i) {
		const auto &get = gets->at(i);
		getMap[get.value] = time++;
	}

	size_t gets_found = 0;
	size_t i = 0;

	int64_t min_get_time = puts->size() + 1;
	while (gets_found <= numGets && i < puts->size()) {
		const auto &put = puts->at(i);
		SInterval interval{};
		interval.start = put.time;
		interval.value = put.value;
		if (getMap.contains(put.value)) {
			gets_found++;
			interval.end = getMap[put.value] + min_get_time;
			// snark...... hahahahaha ja
		} else {
			interval.end = std::numeric_limits<int64_t>::max();
		}
		intervals.push_back(interval);
		++i;
	}
}

AbstractExecutor::Measurement FenwickAImp::execute() {
	return calcMaxMeanError();
}
void FenwickAImp::reset() { intervals.clear(); }
} // namespace bench