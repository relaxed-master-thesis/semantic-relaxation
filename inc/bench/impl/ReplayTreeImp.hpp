#pragma once

#include "bench/util/Operation.hpp"
#include "bench/util/Executor.hpp"
#include "bench/util/GNUOrderedSet.hpp"

#include <cstdint>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
#include <functional>
#include <memory.h>
#include <vector>

//  I wonder why this formatting is so different compared to other files
namespace bench {
class ReplayTreeImp : public AccurateExecutor {
  public:
	ReplayTreeImp() = default;
	~ReplayTreeImp() = default;
	AbstractExecutor::Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	AbstractExecutor::Measurement execute() override;
	void reset() override;

  private:
	struct item {
		uint64_t value;
		item *next;
	};
	enum class EventType { PUSH, POP };
	struct Event {
		EventType type;
		uint64_t time;
		uint64_t push_time;
		uint64_t pop_time;
	};
	std::vector<Event> events;
	std::shared_ptr<std::vector<Operation>> get_stamps;
	std::shared_ptr<std::vector<Operation>> put_stamps;
	// std::set<uint64_t> end_tree;
	ordered_set<uint64_t, std::greater<uint64_t>> pop_tree;
	uint64_t get_stamps_size;
};
} // namespace bench