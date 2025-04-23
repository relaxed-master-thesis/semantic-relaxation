#pragma once

#include "bench/util/Executor.hpp"
#include <vector>

namespace bench {
class FenwickStackImp : public AccurateStackExecutor {
  public:
	FenwickStackImp() = default;
	~FenwickStackImp() = default;

	AbstractExecutor::Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	AbstractExecutor::Measurement execute() override;
	void reset() override;

  private:
  struct event {
    event(bool isPush, bool hasPop, int64_t pushOrder, int64_t time)
        : isPush(isPush), hasPop(hasPop), pushOrder(pushOrder), time(time) {}

    bool isPush;
    bool hasPop;
    int64_t pushOrder;
    int64_t time;
  };

	std::vector<event> elems{};
  size_t pushes = 0;
  size_t pops = 0;
};
} // namespace bench