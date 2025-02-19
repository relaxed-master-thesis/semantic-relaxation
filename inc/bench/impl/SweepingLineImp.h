#pragma once

#include "bench/Benchmark.h"
#include "bench/ErrorCalculator.h"
#include "bench/Operation.h"

#include <cstdint>
#include <functional>
#include <memory.h>
#include <vector>

#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>

using namespace __gnu_pbds;

// Ordered set that supports order statistics
template <typename T>
using ordered_set = tree<T, null_type, std::greater<T>, rb_tree_tag,
						 tree_order_statistics_node_update>;

namespace bench {
class SweepingLineImp : public ErrorCalculator, public AbstractExecutor {
  public:
	SweepingLineImp() = default;
	~SweepingLineImp() = default;
	Result calcMaxMeanError() override;
	void prepare(InputData data) override;
	long execute() override;

  private:
	struct item {
		uint64_t value;
		item *next;
	};
    enum class EventType {
        START,
        END
    };
    struct Event {
        EventType type;
        uint64_t time;
        uint64_t start_time;
        uint64_t end_time;
    };
    std::vector<Event> events;
	std::shared_ptr<std::vector<Operation>> get_stamps;
	std::shared_ptr<std::vector<Operation>> put_stamps;
    ordered_set<uint64_t> end_tree;
    uint64_t get_stamps_size;
};
} // namespace bench