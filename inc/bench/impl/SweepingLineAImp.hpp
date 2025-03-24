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
class SweepingLineAImp : public ApproximateExecutor {
  public:
	SweepingLineAImp() = default;
	SweepingLineAImp(float counting_share)
		: counting_share(counting_share), counting_type(CountingType::SHARE){};
	SweepingLineAImp(uint64_t counting_ammount)
		: counting_amount(counting_ammount),
		  counting_type(CountingType::AMOUNT){};
	~SweepingLineAImp() = default;
	AbstractExecutor::Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	void prep_box(const InputData &data);
	AbstractExecutor::Measurement execute() override;
	void reset() override;

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
	ordered_set<uint64_t, std::greater<uint64_t>> end_tree;
	uint64_t get_stamps_size;
	float counting_share{1.f};
	CountingType counting_type;
	uint64_t counting_amount{0};
};
} // namespace bench