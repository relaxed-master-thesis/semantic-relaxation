#pragma once
#include "bench/util/Operation.hpp"
#include "bench/util/Executor.hpp"
#include <cstdint>
#include <memory>
#include <vector>
namespace bench {
    class OrderApproxImp : public ApproximateExecutor {
        public:
        OrderApproxImp() = default;
        void prepare(const InputData &data) override;
        Measurement execute() override;
        Measurement calcMaxMeanError() override;
        void reset() override;
        private:
        struct Point{
            Point() = default;
            Point(uint64_t enq_idx, uint64_t deq_idx) : enq_idx(enq_idx), deq_idx(deq_idx) {}
            Point(uint64_t enq_idx) : enq_idx(enq_idx), deq_idx(~0) {}
            public:
                uint64_t enq_idx, deq_idx;
        };
        std::shared_ptr<const std::vector<Operation>> put_stamps;
        std::shared_ptr<const std::vector<Operation>> get_stamps;
        std::vector<Point> points;


    };
}