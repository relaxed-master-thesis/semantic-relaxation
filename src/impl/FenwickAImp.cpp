#include "bench/impl/FenwickAImp.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <limits>
#include <unordered_map>
#include <unordered_set>

namespace bench {
void FenwickAImp::FenwickTree::update(int64_t idx, int64_t val) {
	while (idx < BIT.size()) {
		BIT[idx] += val;
		idx += idx & -idx;
	}
}

int64_t FenwickAImp::FenwickTree::query(int64_t idx) {
	int64_t sum = 0;
	while (idx > 0) {
		sum += BIT[idx];
		idx -= idx & -idx;
	}
	return sum;
}

AbstractExecutor::Measurement FenwickAImp::calcMaxMeanError() {
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
	int64_t max = 0;

	for (int64_t i = 0; i < n; ++i) {
		int64_t endVal = intervals[i].end;

		if (endVal == std::numeric_limits<int64_t>::max()) {
			++constError;
			continue;
		}
		++countedElems;
		int64_t comprEnd = endIndices[endVal];

		int64_t res = BIT.query(n) - BIT.query(comprEnd) + constError;
		sum += res;
		max = max < res ? res : max;
		BIT.update(comprEnd, 1);
	}

	return {static_cast<uint64_t>(max), (double)sum / countedElems};
}

void FenwickAImp::prepare(const InputData &data) {
	auto gets = data.getGets();
	auto puts = data.getPuts();

    // bara spara minne, exakt, just in case
	intervals.resize(puts->size());

	std::unordered_map<int64_t, size_t> getMap{};
    std::unordered_set<int64_t> getSet{};
	int64_t time = 0;

    size_t numGets = gets->size() * counting_share;
    for (size_t i = 0; i < numGets; ++i) {
        const auto &get = gets->at(i);
        getMap[get.value] = time++;
    }

    size_t gets_found = 0;
    size_t i = 0;
    
    // jag tror jag också e klar men venne om detta funkar
    // tror det borde göra det
    // de makear sense men de blev fortfarande fel
    // wtf det löste det jo
    // vänta va ändra du aha ok broooooooooooor såhär nära
	int64_t min_get_time = puts->size() + 1;
    while (gets_found <= numGets && i < puts->size()) {
        const auto &put = puts->at(i);
        auto &interval = intervals.at(i);
        interval.start = put.time;
        interval.value = put.value;
        if (getMap.contains(put.value)) {
            gets_found++;
            interval.end = getMap[put.value] + min_get_time;
            // snark...... hahahahaha ja
        } else {
            interval.end = std::numeric_limits<int64_t>::max();
        }
        ++i;
    }
    // yep
    // vet du vad jag kan göra såhär istället

    // size_t last_get = gets->size() * counting_share;
    // int64_t max_time = gets->at(last_get).time;
    // time = puts->at(0).time;

    // std::unordered_map<int64_t, int64_t> putMap{};

    // do {
    //     auto &put = puts->at(i);
	// 	auto &interval = intervals.at(i);
	// 	interval.start = time;
	// 	// this is a potential bug, multiple intvs have same end
	// 	interval.end = std::numeric_limits<int64_t>::max();
	// 	interval.value = static_cast<int64_t>(put.value);
	// 	putMap[put.value] = i;
	// 	++time;
    // } while (time < max_time);

    // dehär funkar för vanliga, låt mig koka
	// for (size_t i = 0; i < puts->size(); ++i) {
	// 	auto &put = puts->at(i);
	// 	auto &interval = intervals.at(i);
	// 	interval.start = time;
	// 	// this is a potential bug, multiple intvs have same end
	// 	interval.end = std::numeric_limits<int64_t>::max();
	// 	interval.value = static_cast<int64_t>(put.value);
	// 	putMap[put.value] = i;
	// 	++time;
	// }
	// for (size_t i = 0; i < gets->size(); ++i) {
	// 	auto &get = gets->at(i);
	// 	int64_t getv = static_cast<int64_t>(get.value);
	// 	auto &interval = intervals.at(putMap[getv]);
	// 	interval.end = time;
	// 	++time;
	// }
}

AbstractExecutor::Measurement FenwickAImp::execute() {
	return calcMaxMeanError();
}
void FenwickAImp::reset() {
	intervals.clear();
}
} // namespace bench