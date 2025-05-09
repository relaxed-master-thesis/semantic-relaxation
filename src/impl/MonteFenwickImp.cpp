#include "bench/impl/MonteFenwickImp.hpp"
#include "bench/util/FenwickTree.hpp"
#include "bench/util/FastRandom.hpp"

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

	std::unordered_map<int64_t, size_t> putMap{};
	std::vector<PushedItem> all_pushed_items{};
	for (size_t i = 0; i < puts->size(); ++i) {
		auto &put = puts->at(i);
		all_pushed_items.push_back({std::numeric_limits<int64_t>::max()});
		putMap[put.value] = i;
	}
	size_t num_gets = gets->size() * counting_share;
	// num_gets = std::max(num_gets, (size_t)1000);
	std::unordered_set<size_t> get_indices;
	for(size_t i = 0; i < num_gets; ) {
		if(get_indices.insert(xorshf96() % (gets->size() - 1)).second) {
			++i;
		}
	}
	size_t pop_order = 1;
	for (size_t i = 0; i < num_gets; ++i) {
		if(get_indices.find(i) == get_indices.end()) {
			continue;
		}
		auto &get = gets->at(i);
		all_pushed_items[putMap[get.value]].pop_time = pop_order++;
	}
	pop_order = 1;
	for(size_t i = 0; i < all_pushed_items.size(); ++i) {
		if(all_pushed_items[i].pop_time != std::numeric_limits<int64_t>::max()) {
			pushed_items.emplace_back(all_pushed_items[i].pop_time);
		}
	}
	std::cout << "Pushed items size: " << pushed_items.size() << "\n";
	std::cout << "All pushed items size: " << all_pushed_items.size() << "\n";

}

AbstractExecutor::Measurement MonteFenwickImp::execute() {
	return calcMaxMeanError();
}
void MonteFenwickImp::reset() { pushed_items.clear(); reset_random(); }
} // namespace bench