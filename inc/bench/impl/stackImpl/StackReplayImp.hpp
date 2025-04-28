#pragma once
#include "bench/util/Executor.hpp"
#include "bench/util/InputData.hpp"
#include <cstddef>

namespace bench {
    class StackReplayImp : public AccurateStackExecutor {
      public:
        StackReplayImp() = default;
        ~StackReplayImp() = default;

        AbstractExecutor::Measurement calcMaxMeanError() override;
        void prepare(const InputData &data) override;
        AbstractExecutor::Measurement execute() override;
        void reset() override;

      private:
        struct Event {
            uint64_t time;
            uint64_t value;
            bool isPut;
        };
        struct StackItem {
            uint64_t value;
            StackItem *next;
        };
        std::vector<Event> events; 
        StackItem *stackHead;
        std::vector<StackItem> stackItems;
        size_t get_stamps_size;
    };
}