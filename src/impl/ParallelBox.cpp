#include "bench/impl/ParallelBox.hpp"
#include "bench/util/Executor.hpp"
#include "bench/util/FenwickTree.hpp"

#include <algorithm>
#include <cstdint>
#include <future>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <vector>

namespace bench {
AbstractExecutor::Measurement ParallelBox::execute() {
	auto func = [this](size_t tid,
					   std::pair<uint64_t, uint64_t> *results) -> void {
		*results = calcBox(ranges.at(tid));
	};

	std::vector<std::thread> threads;
	std::vector<std::pair<uint64_t, uint64_t>> results(ranges.size(), {0, 0});

	for (size_t i = 0; i < ranges.size(); ++i) {
		std::thread thread(func, i, &(results[i]));
		threads.push_back(std::move(thread));
	}
	uint64_t rankMax = 0;
	uint64_t rankSum = 0;
	for (size_t i = 0; i < threads.size(); ++i) {
		threads[i].join();
		auto [max, sum] = results.at(i);
		rankMax = std::max(rankMax, max);
		rankSum += sum;
	}
	return {rankMax, (long double)rankSum / gets->size()};
}

AbstractExecutor::Measurement ParallelBox::calcMaxMeanError() { return {0, 0}; }

std::pair<uint64_t, uint64_t> ParallelBox::calcBox(range r) {

	std::vector<std::pair<int, int>> points{};
	for (size_t i = r.from; i < r.to; ++i) {
		auto &put = puts->at(i);
		if (getMap.find(put.value) != getMap.end()) {
			points.emplace_back(put.time, getMap[put.value]);
		} else {
			points.emplace_back(put.time, std::numeric_limits<int>::max());
		}
	}
    std::cout << "Points size: " << points.size() << "\n";

	size_t n = r.to - r.from;
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

	return {max, sum};
}

void ParallelBox::prepare(const InputData &data) {
	if (height == 0 || width == 0) {
		throw std::invalid_argument("Height and width must be set");
	}

	this->gets = data.getGets();
	this->puts = data.getPuts();

	size_t threadsAvailable = std::thread::hardware_concurrency();
	size_t boxSize = height * width;
	size_t numBoxes = (puts->size() + boxSize - 1) / boxSize;
	size_t numRanges = std::min(threadsAvailable, numBoxes);
	size_t boxesPerThread = numBoxes / numRanges;
	// std::unordered_map<uint64_t, int> getMap{};

	for (size_t i = 0; i < gets->size(); ++i) {
		auto &put = gets->at(i);
		getMap[put.value] = put.time;
	}

	for (size_t i = 0; i < numRanges; ++i) {
		size_t from = i * boxesPerThread * boxSize;
		size_t to = (i == numRanges - 1) ? puts->size()
										 : from + boxesPerThread * boxSize;
		// std::cout << "(" << from << ", " << to << ")" << "\n";
		ranges.emplace_back(from, to);
	}
}

void ParallelBox::reset() {
	gets = nullptr;
	puts = nullptr;
	ranges.clear();
	getMap.clear();
}

} // namespace bench