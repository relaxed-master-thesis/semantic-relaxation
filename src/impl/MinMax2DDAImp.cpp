#include "bench/impl/MinMax2DDAImp.hpp"
#include "bench/util/Executor.hpp"
#include "bench/util/FenwickTree.hpp"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>

// #define VALIDATION

namespace bench {
ApproximateQueueExecutor::Measurement MinMax2DDAImp::execute() {
	size_t n = pushed_items.size();
	FenwickTree<int> BIT(n);
	int64_t constErr = 0;
	int64_t countedElems = 0;
	int64_t sum = 0;
	int64_t max = 0;
	for (int i = 0; i < n; ++i) {
		int pop_time = pushed_items[i].pop_time;
		if (pop_time == std::numeric_limits<int>::max()) {
			++constErr;
			continue;
		}
		++countedElems;
		int64_t res = BIT.query(pop_time) + constErr;
		BIT.update(pop_time, 1);
		sum += res;
		max = max < res ? res : max;
	}
	return {static_cast<uint64_t>(max), (double)sum / countedElems};
}

ApproximateQueueExecutor::Measurement MinMax2DDAImp::calcMaxMeanError() {
	return {0, 0};
}

void MinMax2DDAImp::reset() {
	pushed_items.clear();
}

void MinMax2DDAImp::prepare(const InputData &data) {
	const auto gets = data.getGets();
	const auto puts = data.getPuts();
	size_t nElems = puts->size() * countingShare;

	if (expectedHeight == 0 || expectedWidth == 0) {
		throw std::runtime_error("Expected height and width must be set");
	}
	size_t getsSize = gets->size();
	size_t boxSize = expectedHeight * expectedWidth;
	size_t numBoxes = (nElems / boxSize) + 1;
	size_t elemsToCount = std::min(numBoxes * boxSize, getsSize);
	size_t half = getsSize / 2;
	size_t startIdx = half - (half % boxSize);
	startIdx = 0;

	if (getsSize < boxSize) {
		boxSize = getsSize;
	}

	std::unordered_map<uint64_t, int> getMap{};
	for (size_t i = 0; i < boxSize; ++i) {
		auto &put = gets->at(i + startIdx);
		getMap[put.value] = i+1;
	}

	for (size_t i = 0; i < boxSize; ++i) {
		auto &put = puts->at(i + startIdx);
		if (getMap.find(put.value) != getMap.end()) {
			pushed_items.emplace_back(getMap[put.value]);
		}  else {
			pushed_items.emplace_back(std::numeric_limits<int>::max());
		}
	}
}
} // namespace bench