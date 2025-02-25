#include "bench/impl/FenwickImp.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
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

	FenwickTree BIT(n);
	std::vector<int64_t> result(n, 0);
	int64_t constError = 0;
	int64_t countedElems = 0;
	int64_t sum = 0;

	for (int64_t i = 0; i < n; ++i) {
		int64_t endVal = intervals[i].end;

		if (endVal == std::numeric_limits<int64_t>::max()) {
			++constError;
			continue;
		}
		++countedElems;
		int64_t comprEnd = endIndices[endVal];

		sum += BIT.query(n) - BIT.query(comprEnd) + constError;
		BIT.update(comprEnd, 1);
	}

	uint64_t max =
		static_cast<uint64_t>(*std::max_element(result.begin(), result.end()));
	return {max, (double)sum / countedElems};
}

void FenwickImp::prepare(const InputData &data) {
	auto gets = data.getGets();
	auto puts = data.getPuts();

	intervals.resize(puts->size());

	std::unordered_map<int64_t, size_t> putMap{};
	int64_t time = 0;

	for (size_t i = 0; i < puts->size(); ++i) {
		auto &put = puts->at(i);
		auto &interval = intervals.at(i);
		interval.start = time;
		// this is a potential bug, multiple intvs have same end
		interval.end = std::numeric_limits<int64_t>::max();
		interval.value = static_cast<int64_t>(put.value);
		putMap[put.value] = i;
		++time;
	}
	for (size_t i = 0; i < gets->size(); ++i) {
		auto &get = gets->at(i);
		int64_t getv = static_cast<int64_t>(get.value);
		auto &interval = intervals.at(putMap[getv]);
		interval.end = time;
		++time;
	}
}

AbstractExecutor::Measurement FenwickImp::execute() {
	return calcMaxMeanError();
}
} // namespace bench