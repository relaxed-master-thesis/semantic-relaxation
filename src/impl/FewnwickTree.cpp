#include "bench/ErrorCalculator.h"
#include "bench/impl/FenwickImp.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <limits>
#include <numeric>
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

ErrorCalculator::Result FenwickImp::calcMaxMeanError() {
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
	auto printArr = [](const std::vector<int64_t> &arr) {
		for (auto &elem : arr) {
			std::cout << elem << ", ";
		}
		std::cout << "\n";
	};

	printArr(BIT.getBit());

	for (int64_t i = 0; i < n; ++i) {
		int64_t endVal = intervals[i].end;
		printArr(BIT.getBit());

		if (endVal == std::numeric_limits<int64_t>::max()) {
			++constError;
			continue;
		}
		++countedElems;
		int64_t comprEnd = endIndices[endVal];

		std::cout << "sum += " << BIT.query(n) << " - " << BIT.query(comprEnd)
				  << " + " << constError << "\n";
		sum += BIT.query(n) - BIT.query(comprEnd) + constError;
		BIT.update(comprEnd, 1);
	}

	printArr(BIT.getBit());

	uint64_t max =
		static_cast<uint64_t>(*std::max_element(result.begin(), result.end()));
	std::cout << "max: " << max << ", sum: " << sum
			  << ", countedElems: " << countedElems << ", totElems: " << n
			  << "\n";
	return {max, (double)sum / countedElems};
}

void FenwickImp::prepare(InputData data) {
	intervals.resize(data.puts->size());
	std::unordered_map<int64_t, size_t> putMap{};
	int64_t time = 0;
	for (size_t i = 0; i < data.puts->size(); ++i) {
		auto &put = data.puts->at(i);
		auto &interval = intervals.at(i);
		interval.start = time;
		interval.end = std::numeric_limits<int64_t>::max();
		interval.value = static_cast<int64_t>(put.value);
		putMap[put.value] = i;
		++time;
	}
	for (size_t i = 0; i < data.gets->size(); ++i) {
		auto &get = data.gets->at(i);
		int64_t getv = static_cast<int64_t>(get.value);
		auto &interval = intervals.at(putMap[getv]);
		interval.end = time;
		++time;
	}
}

long FenwickImp::execute() {
	std::cout << "Running FenwickImp...\n";
	auto start = std::chrono::high_resolution_clock::now();
	auto result = calcMaxMeanError();
	auto end = std::chrono::high_resolution_clock::now();
	auto duration =
		std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	std::cout << "Runtime: " << duration.count() << " us\n";
	std::cout << "Mean: " << result.mean << ", Max: " << result.max << "\n";
	return duration.count();
}
} // namespace bench