#pragma once

#include "bench/util/Operation.hpp"
#include "bench/util/Executor.hpp"
#include "bench/util/GNUOrderedSet.hpp"

#include <cstdint>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
#include <memory.h>
#include <vector>

//  I wonder why this formatting is so different compared to other files
namespace bench {
static unsigned long x = 123456789, y = 362436069, z = 521288629;
static void reset_random() {
	x = 123456789;
	y = 362436069;
	z = 521288629;
}

inline unsigned long xorshf96(void) { // period 2^96-1
	unsigned long t;
	x ^= x << 16;
	x ^= x >> 5;
	x ^= x << 1;

	t = x;
	x = y;
	y = z;
	z = t ^ x ^ y;

	return z;
}
class MonteReplayTree : public ApproximateExecutor {
  public:
	MonteReplayTree() = default;
	MonteReplayTree(float counting_share) : counting_share(counting_share){};
	~MonteReplayTree() = default;
	AbstractExecutor::Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	AbstractExecutor::Measurement execute() override;
	void reset() override;

  private:
	void par(size_t start_idx, size_t num_puts,
			 std::shared_ptr<const std::vector<Operation>> puts,
			 std::unordered_map<uint64_t, size_t> &getMap);

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
	ordered_set<uint64_t, std::greater<uint64_t>> end_tree;
	uint64_t get_stamps_size;
	float counting_share{1.f};
};
} // namespace bench