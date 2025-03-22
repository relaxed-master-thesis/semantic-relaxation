#include "bench/impl/MinMax2DDAImp.hpp"
#include "bench/util/Executor.hpp"
#include "bench/util/FenwickTree.hpp"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>

// #define VALIDATION

namespace bench {
ApproximateExecutor::Measurement MinMax2DDAImp::execute() {
	// Use a Fenwick tree to calculate the mean and max errors for all points
	size_t n = points.size();

	std::vector<int> sortedEnds(n, 0);
	for (size_t i = 0; i < n; ++i) {
		sortedEnds[i] = points[i].second;
	}

	std::ranges::sort(sortedEnds);
	std::unordered_map<int, int> endIndices{};
	for (int i = 0; i < n; ++i) {
		endIndices[sortedEnds[i]] = i + 1;
	}

	FenwickTree<int> BIT(n);
	int constErr = 0;
	int countedElems = 0;
	int sum = 0;
	int max = 0;

	for (int i = 0; i < n; ++i) {
		int endVal = points[i].second;

		if (endVal == std::numeric_limits<int>::max()) {
			++constErr;
			continue;
		}

		++countedElems;
		int comprEnd = endIndices[endVal];

		int res = BIT.query(n) - BIT.query(comprEnd) + constErr;
		BIT.update(comprEnd, 1);

		sum += res;
		max = max < res ? res : max;
	}

	return {static_cast<uint64_t>(max), (double)sum / countedElems};
}

ApproximateExecutor::Measurement MinMax2DDAImp::calcMaxMeanError() {
	return {0, 0};
}

void MinMax2DDAImp::reset() {
	points.clear();
	ranges.clear();
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

	std::unordered_map<uint64_t, int> getMap{};
	for (size_t i = 0; i < elemsToCount; ++i) {
		auto &put = gets->at(i);
		getMap[put.value] = put.time;
	}

	for (size_t i = 0; i < elemsToCount; ++i) {
		auto &put = puts->at(i);
		if (getMap.find(put.value) != getMap.end()) {
			points.emplace_back(put.time, getMap[put.value]);
		} else {
			points.emplace_back(put.time, std::numeric_limits<int>::max());
		}
	}
}
} // namespace bench