#include "bench/impl/stackImpl/StackReplayImp.hpp"
#include "bench/util/InputData.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
#include <sys/types.h>

namespace bench {  
    AbstractExecutor::Measurement StackReplayImp::calcMaxMeanError() {
        return AbstractExecutor::Measurement();
    }


    AbstractExecutor::Measurement StackReplayImp::execute() {
        uint64_t max = 0;
        uint64_t sum = 0;
        size_t stackItemIdx = 0;
        for(const auto &event : events) {
            uint64_t rank = 0;
            uint64_t key = event.value;
            if (event.isPut) {
                stackItems[stackItemIdx] = {event.value, stackHead};
                stackHead = &stackItems[stackItemIdx];
                stackItemIdx++;
            } else {
                if(stackHead->value == key) {
                    rank = 0;
                    stackHead = stackHead->next;
                }
                else{
                    rank = 1;
                    StackItem *current = stackHead;
                    while (current->next->value != key) {
                        current = current->next;
                        rank += 1;
                        if (current->next == nullptr) {
                            throw std::runtime_error(
                                std::format("Put for elem {} not found\n", key));
                        }
                    }
                    // current->next has the removed item, so just unlink it from the
                    // data structure
                    current->next = current->next->next;
                }
                sum += rank;
                if (rank > max)
                    max = rank;
            }
        }

        return {max, (long double)sum / (long double)get_stamps_size};
    }

    void StackReplayImp::prepare(const InputData &data) {
        auto puts = data.getPuts();
        auto gets = data.getGets();
        get_stamps_size = gets->size();
        stackItems.reserve(puts->size());
        events.reserve(puts->size() + gets->size());
        for (const auto &put : *puts) {
            events.push_back({put.time, put.value, true});
        }
        for (const auto &get : *gets) {
            events.push_back({get.time, get.value, false});
        }
        // Sort events by time 
        std::sort(events.begin(), events.end(),
                  [](const Event &a, const Event &b) { return a.time < b.time; });
        stackHead = nullptr;
    }

    void StackReplayImp::reset() {
        events.clear();
        stackItems.clear();
        get_stamps_size = 0;
    }
}
