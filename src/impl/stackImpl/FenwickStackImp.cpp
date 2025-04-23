#include "bench/impl/stackImpl/FenwickStackImp.hpp"
#include "bench/util/FenwickTree.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <unordered_map>

namespace bench {
AbstractExecutor::Measurement FenwickStackImp::calcMaxMeanError() {
	return AbstractExecutor::Measurement();
}

void FenwickStackImp::prepare(const InputData &data) {
	auto puts = data.getPuts();
	auto gets = data.getGets();

	std::unordered_map<int64_t, size_t> getMap{};
	for (size_t i = 0; i < gets->size(); ++i) {
		auto &get = gets->at(i);
		elems.emplace_back(false, true, 0, get.time);
		getMap[get.value] = i;
	}
	for (size_t i = 0; i < puts->size(); ++i) {
		auto &put = puts->at(i);
		if (getMap.find(put.value) == getMap.end()) {
            elems.emplace_back(true, false, i + 1, put.time);
		} else {
            elems.emplace_back(true, true, i + 1, put.time);
			elems[getMap[put.value]].pushOrder = i + 1;
        }
	}

	std::sort(elems.begin(), elems.end(),
			  [](const event &a, const event &b) { return a.time < b.time; });

	pushes = puts->size();
	pops = gets->size();
}

AbstractExecutor::Measurement FenwickStackImp::execute() {
	ReverseFenwickTree<int64_t> BIT(pushes);
	int64_t constError = 0, sum = 0, max = 0;

	for (const auto &elem : elems) {
		if (elem.isPush) {
			BIT.update(elem.pushOrder, 1);
		} else if (elem.hasPop) {
			BIT.update(elem.pushOrder, -1);
			int64_t rank = BIT.query(elem.pushOrder);
			sum += rank;
			max = std::max(max, rank);
		}
	}

	return {static_cast<uint64_t>(max), static_cast<double>(sum) / pops};
}

void FenwickStackImp::reset() {
	elems.clear();
	pushes = 0;
}
} // namespace bench