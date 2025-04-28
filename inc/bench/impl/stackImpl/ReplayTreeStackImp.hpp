#pragma once

#include "bench/util/Executor.hpp"
#include "bench/util/GNUOrderedSet.hpp"
#include <functional>

namespace bench {
    class ReplayTreeStackImp : public AccurateStackExecutor {
      public:
        ReplayTreeStackImp() = default;
        ~ReplayTreeStackImp() = default;

        AbstractExecutor::Measurement calcMaxMeanError() override;
        void prepare(const InputData &data) override;
        AbstractExecutor::Measurement execute() override;
        void reset() override;

      private:
struct event {
    event(bool isPush, bool hasPop, uint64_t pushOrder, uint64_t time)
        : isPush(isPush), hasPop(hasPop), pushOrder(pushOrder), time(time) {}

    bool isPush;
    bool hasPop;
    uint64_t pushOrder;
    uint64_t time;
  };
       uint64_t get_stamps_size;
	      std::vector<event> events;
        ordered_set<uint64_t, std::greater<uint64_t>> push_tree;
    };

}