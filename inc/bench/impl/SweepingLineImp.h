#pragma once

#include "bench/Benchmark.h"
#include "bench/Operation.h"

#include <cstdint>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
#include <memory.h>
#include <vector>


#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>

using namespace __gnu_pbds;

// Ordered set that supports order statistics
template <typename T>
using ordered_set = tree<T, null_type, std::greater<T>, rb_tree_tag,
						 tree_order_statistics_node_update>;

//  I wonder why this formatting is so different compared to other files
namespace bench {
class SweepingLineImp : public AccurateExecutor {
  public:
	SweepingLineImp() = default;
	~SweepingLineImp() = default;
	AbstractExecutor::Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	AbstractExecutor::Measurement execute() override;

  private:
	struct item {
		uint64_t value;
		item *next;
	};
	enum class EventType { START, END };
	struct Event {
		EventType type;
		uint64_t time;
		uint64_t start_time;
		uint64_t end_time;
	};
	std::vector<Event> events;
	std::shared_ptr<std::vector<Operation>> get_stamps;
	std::shared_ptr<std::vector<Operation>> put_stamps;
	// std::set<uint64_t> end_tree;
	ordered_set<uint64_t> end_tree;
	uint64_t get_stamps_size;
};
} // namespace bench