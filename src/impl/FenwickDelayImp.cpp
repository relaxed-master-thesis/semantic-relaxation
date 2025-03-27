#include "bench/impl/FenwickDelayImp.hpp"
#include "bench/util/Executor.hpp"

#include <cstddef>
#include <cstdint>
#include <unordered_map>

namespace bench {
void FenwickDelayImp::prepare(const InputData &data) {
	auto gets = data.getGets();
	auto puts = data.getPuts();

	std::unordered_map<int64_t, size_t> putMap{};

	for (size_t i = 0; i < puts->size(); ++i) {
		auto &put = puts->at(i);
		popTimes.push_back(std::numeric_limits<int64_t>::max());
		putMap[put.value] = i;
	}
	for (size_t i = 0; i < gets->size(); ++i) {
		auto &get = gets->at(i);
		popTimes[putMap[get.value]] = i + 1;
	}
}

void FenwickDelayImp::reset() { popTimes.clear(); }

AbstractExecutor::Measurement FenwickDelayImp::execute() {
	size_t n = popTimes.size();
	ReverseFenwickTree<int64_t> BIT(n);

	int64_t constDelay = 0;
	int64_t countedElems = 0;
	int64_t sum = 0;
	int64_t max = 0;

	for (size_t i = 0; i < n; ++i) {
		int64_t popTime = popTimes[i];

		if (popTime == std::numeric_limits<int64_t>::max()) {
			++constDelay;
			continue;
		}

		++countedElems;
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