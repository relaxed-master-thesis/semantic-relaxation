#include "bench/impl/MonteSweepingLine.h"
#include "bench/Benchmark.h"
#include "bench/Operation.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

namespace bench {

AbstractExecutor::Measurement MonteSweepingLine::calcMaxMeanError() {
	uint64_t rank_sum = 0, rank_max = 0, counted_rank = 0, const_error = 0;

	for (auto &event : events) {
		if (event.end_time == ~0) {
			const_error++;
		} else if (event.type == EventType::START) {
			uint64_t rank = end_tree.order_of_key(event.end_time);
			rank += const_error;
			rank /= counting_share;
			rank_sum += rank;
			counted_rank++;
			if (rank > rank_max) {
				rank_max = rank;
			}
			end_tree.insert(event.end_time);
		} else {
			end_tree.erase(event.end_time);
		}
	}
	return {rank_max, (double)rank_sum / counted_rank};
}

void MonteSweepingLine::prepare(const InputData &data) {
	auto gets = data.getGets();
	get_stamps_size = gets->size();
	std::unordered_map<uint64_t, size_t> getMap{};
	for (size_t i = 0; i < gets->size(); ++i) {
		const Operation &get = gets->at(i);
		getMap[get.value] = get.time;
	}
	auto puts = data.getPuts();
	size_t num_puts = puts->size() * counting_share;
	if(num_puts < 1000){
		num_puts = 1000;
		counting_share = (float)1000 / puts->size();
	}

	events.resize(num_puts * 2);
	for (size_t i = 0; i < num_puts; ++i) {
		size_t idx = xorshf96() % (puts->size() - i);
		const Operation &put = puts->at(idx);
		uint64_t end_time = 0;
		end_time = getMap.find(put.value) == getMap.end() ? ~0 : getMap[put.value];

		events[i * 2] = {EventType::START, put.time, put.time, end_time};
		events[i * 2 + 1] = {EventType::END, end_time, put.time, end_time};
	}

	std::sort(events.begin(), events.end(),
			  [](const Event &left, const Event &right) -> bool {
				  return left.time < right.time;
			  });
}

AbstractExecutor::Measurement MonteSweepingLine::execute() {
	return calcMaxMeanError();
}
void MonteSweepingLine::reset(){
	events.clear();
	end_tree.clear();
	get_stamps = nullptr;
	get_stamps_size = 0;
	put_stamps = nullptr;
	reset_random();
}

} // namespace bench