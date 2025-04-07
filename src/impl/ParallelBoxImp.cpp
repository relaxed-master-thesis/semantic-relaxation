#include "bench/impl/ParallelBoxImp.hpp"
#include "bench/util/Executor.hpp"
#include "bench/util/FenwickTree.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <vector>

namespace bench {
AbstractExecutor::Measurement ParallelBoxImp::execute() {
	auto func = [this](size_t tid,
					   std::pair<uint64_t, uint64_t> *results) -> void {
		*results = calcBox(tid, ranges.at(tid));
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
	return {rankMax, (double)rankSum / (uint64_t)gets->size()};
}

AbstractExecutor::Measurement ParallelBoxImp::calcMaxMeanError() { return {0, 0}; }

std::pair<uint64_t, uint64_t> ParallelBoxImp::calcBox(size_t tid, range r) {
	size_t n = r.to - r.from;
	FenwickTree<uint64_t> BIT(gets->size());
	uint64_t constErr = 0;
	int countedElems = 0;
	uint64_t sum = 0;
	uint64_t max = 0;
	auto &pporders = range_pp_orders.at(tid);

	for (int i = 0; i < n; ++i) {
		int64_t pop_order = static_cast<int64_t>(pporders[i]);

		if (pop_order == std::numeric_limits<int>::max()) {
			++constErr;
			continue;
		}

		++countedElems;

		uint64_t res = BIT.query(pop_order) + constErr;
		BIT.update(pop_order, 1);

		sum += res;
		max = max < res ? res : max;
	}

	return {max, sum};
}

void ParallelBoxImp::prepare(const InputData &data) {
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
	std::unordered_map<uint64_t, int> getMap{};

	for (size_t i = 0; i < gets->size(); ++i) {
		auto &put = gets->at(i);
		getMap[put.value] = i + 1;
	}

	for (size_t i = 0; i < numRanges; ++i) {
		size_t from = i * boxesPerThread * boxSize;
		size_t to = (i == numRanges - 1) ? puts->size()
										 : from + boxesPerThread * boxSize;
		ranges.emplace_back(from, to);
	}

	for (size_t i = 0; i < ranges.size(); ++i) {
		auto &range = ranges.at(i);
		std::vector<uint64_t> pporders{};
		for (size_t j = range.from; j < range.to; ++j) {
			auto &put = puts->at(j);
			if (getMap.find(put.value) != getMap.end()) {
				pporders.emplace_back(getMap[put.value]);
			} else {
				pporders.emplace_back(std::numeric_limits<int>::max());
			}
		}
		range_pp_orders.push_back(std::move(pporders));
	}
}

void ParallelBoxImp::reset() {
	gets = nullptr;
	puts = nullptr;
	ranges.clear();
	range_pp_orders.clear();
}

} // namespace bench