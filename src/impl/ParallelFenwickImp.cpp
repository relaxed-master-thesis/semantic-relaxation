#include "bench/impl/ParallelFenwickImp.hpp"
#include "bench/util/FenwickTree.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>
#include <thread>
#include <unordered_map>

namespace bench {
AbstractExecutor::Measurement ParallelFenwickImp::calcMaxMeanError() {
	return {0, 0};
}
std::pair<uint64_t, uint64_t> ParallelFenwickImp::calcRange(range r) {
	uint64_t from = r.from;
	uint64_t to = r.to;
	size_t n = pushed_items.size();
	ReverseFenwickTree<int64_t> BIT(n);

	int64_t sum = 0;
	int64_t max = 0;
	size_t constErr = 0;
	for (int64_t i = 0; i < n; ++i) {
		PushedItem item = pushed_items[i];
		if (item.time > to) {
			break;
		}
		if (item.pop_time < from) {
			continue;
		}
		int64_t pop_order = pushed_items[i].pop_order;
		if (pop_order == std::numeric_limits<int64_t>::max()) {
			constErr++;
			continue;
		}
		if (from <= item.pop_time && item.pop_time < to) {
			int64_t res = BIT.query(pop_order) + constErr;
			sum += res;
			max = max < res ? res : max;
		}
		BIT.update(pop_order, 1);
	}
	return {static_cast<uint64_t>(max), sum};
}


void ParallelFenwickImp::prepare(const InputData &data) {
	auto gets = data.getGets();
	get_stamps_size = gets->size();
	auto puts = data.getPuts();

	std::unordered_map<int64_t, size_t> putMap{};
	std::unordered_map<int64_t, size_t> getMap{};

	for (size_t i = 0; i < puts->size(); ++i) {
		auto &put = puts->at(i);
		pushed_items.push_back(
			{put.time, ~0UL, std::numeric_limits<int64_t>::max()});
		putMap[put.value] = i;
	}
	for (size_t i = 0; i < gets->size(); ++i) {
		auto &get = gets->at(i);
		size_t pop_idx = putMap[get.value];
		pushed_items[pop_idx].pop_time = get.time;
		pushed_items[pop_idx].pop_order = i + 1;
	}
	// prep threads

	size_t threadsAvailable = std::thread::hardware_concurrency();

	size_t gets_per_thread = gets->size() / threadsAvailable;
	for (size_t i = 0; i < threadsAvailable; ++i) {
		if (i == threadsAvailable - 1) {
			auto start_get = gets->at(i * gets_per_thread);
			ranges.push_back({start_get.time, std::numeric_limits<int64_t>::max()});
			break;
		}
		auto start_get = gets->at(i * gets_per_thread);
		auto end_get = gets->at((i + 1) * gets_per_thread);
		ranges.push_back({start_get.time, end_get.time});
	}
}

AbstractExecutor::Measurement ParallelFenwickImp::execute() {
	auto func = [this](size_t tid,
					   std::pair<uint64_t, uint64_t> *results) -> void {
		*results = calcRange(ranges.at(tid));
	};

	size_t threadsAvailable = std::thread::hardware_concurrency();
	std::vector<std::thread> threads;
	std::vector<std::pair<uint64_t, uint64_t>> results(threadsAvailable);
	for (size_t i = 0; i < threadsAvailable; ++i) {
		threads.push_back(std::thread(func, i, &results[i]));
	}
	uint64_t rankMax = 0;
	uint64_t rankSum = 0;
	for (size_t i = 0; i < threadsAvailable; ++i) {
		threads[i].join();
		auto [max, sum] = results.at(i);
		rankMax = std::max(rankMax, max);
		rankSum += sum;
	}
	return {rankMax, (double)rankSum / get_stamps_size};

	return calcMaxMeanError();
}
void ParallelFenwickImp::reset() { pushed_items.clear(); }
} // namespace bench