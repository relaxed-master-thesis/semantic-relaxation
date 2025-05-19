#include "bench/impl/MonteFenwickImp.hpp"
#include "bench/util/FastRandom.hpp"
#include "bench/util/FenwickTree.hpp"
#include "bench/util/GNUOrderedSet.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace bench {
AbstractExecutor::Measurement MonteFenwickImp::calcMaxMeanError() {

	size_t n = pushed_items.size();
	ReverseFenwickTree<int64_t> BIT(n);

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
		int64_t rank = BIT.query(pop_time) + constError;
		rank /= counting_share;
		sum += rank;
		maxIdx = max < rank ? i : maxIdx;
		max = max < rank ? rank : max;
		BIT.update(pop_time, 1);
	}

	std::cout << "Counted elems: " << countedElems << "\n";
	return {static_cast<uint64_t>(max), (double)sum / countedElems};
}

void MonteFenwickImp::prepare(const InputData &data) {
	auto gets = data.getGets();
	auto puts = data.getPuts();

	std::unordered_map<int64_t, int> getMap{};
	uint64_t i = 0;
	for (auto &get : *gets) {
		getMap[get.value] = ++i;
	}
	std::unordered_set<size_t> put_indices{};
	ordered_set<uint64_t, std::less<uint64_t>> pop_order_tree;
	int64_t max_pop_order = std::numeric_limits<int64_t>::max();
	uint64_t num_puts = puts->size() * counting_share;
	for (size_t i = 0; i < num_puts;) {
		size_t idx = xorshf96() % (puts->size() - i);
		if (put_indices.insert(idx).second) {
			++i;
		}
	}
	for (size_t idx = 0; idx < puts->size(); ++idx) {
		if (put_indices.find(idx) == put_indices.end()) {
			continue;
		}
		const Operation &put = puts->at(idx);
		if (getMap.contains(put.value)) {
			pushed_items.push_back({getMap[put.value]});
			pop_order_tree.insert(getMap[put.value]);
			continue;
		} else {
			pushed_items.push_back({max_pop_order});
		}
	}
	std::cout << "Pushed items size: " << pushed_items.size() << "\n";
	for (size_t i = 0; i < pushed_items.size(); ++i) {
		int64_t pop_order = pushed_items[i].pop_time;
		if (pop_order == max_pop_order) {
			continue;
		}
		pushed_items[i].pop_time = pop_order_tree.order_of_key(pop_order) + 1;
	}
}

AbstractExecutor::Measurement MonteFenwickImp::execute() {
	return calcMaxMeanError();
}
void MonteFenwickImp::reset() {
	pushed_items.clear();
	reset_random();
}
} // namespace bench