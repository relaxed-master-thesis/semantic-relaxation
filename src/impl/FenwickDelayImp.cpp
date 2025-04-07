#include "bench/impl/FenwickDelayImp.hpp"
#include "bench/util/Executor.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <unordered_map>

namespace bench {
void FenwickDelayImp::prepare(const InputData &data) {
	auto gets = data.getGets();
	num_gets = gets->size();
	auto puts = data.getPuts();

	std::unordered_map<int64_t, size_t> getMap{};
	for (size_t i = 0; i < gets->size(); ++i) {
		auto &get = gets->at(i);
		getMap[get.value] = i + 1;
	}

	for (size_t i = 0; i < puts->size(); ++i) {
		auto &put = puts->at(i);
		if (getMap.contains(put.value)) {
			popTimes.push_back(getMap[put.value]);
		} else {
			popTimes.push_back(std::numeric_limits<int64_t>::max());
		}
	}

	// pushTimes.resize(puts->size(), std::numeric_limits<int64_t>::max());

	// size_t non_get_idx = puts->size() - 1;
	// for (size_t i = 0; i < puts->size(); ++i) {
	// 	auto &put = puts->at(i);
	// 	if(getMap.contains(put.value)) {
	// 		pushTimes[getMap[put.value]] = i + 1;
	// 	} else {
	// 		pushTimes[non_get_idx] = i + 1;
	// 		--non_get_idx;
	// 	}
	// }
}

void FenwickDelayImp::reset() {
	popTimes.clear();
	num_gets = 0;
}

AbstractExecutor::Measurement FenwickDelayImp::execute() {
	size_t n = popTimes.size();
	ReverseFenwickTree<int64_t> BIT(num_gets);

	int64_t constDelay = 0;
	int64_t countedElems = 0;
	int64_t sum = 0;
	int64_t max = 0;

	// delay: pushed after popped before
	for (size_t i = 0; i < n; ++i) {
		int64_t popTime = popTimes[n - i - 1];

		++countedElems;
		if (popTime == 0) {
			std::cout << "popTime: " << popTime << " for i " << i
					  << " in vec of " << popTimes.size() << "elems\n";
		}
		if (popTime == std::numeric_limits<int64_t>::max()) {
			int64_t res = BIT.query(num_gets);
			sum += res;
			max = std::max(max, res);
			continue;
		}

		int64_t res = BIT.query(popTime);
		sum += res;
		max = std::max(max, res);
		BIT.update(popTime, 1);
	}

	return {static_cast<uint64_t>(max), (double)sum / countedElems};
}

AbstractExecutor::Measurement FenwickDelayImp::calcMaxMeanError() {
	return {0, 0};
}

} // namespace bench