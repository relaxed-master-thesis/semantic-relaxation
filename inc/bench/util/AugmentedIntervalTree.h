#pragma once

#include "bench/Interval.h"

#include <memory>

namespace bench {
class AugmentedIntervalTree {
  public:
    AugmentedIntervalTree() = default;
    ~AugmentedIntervalTree() = default;
    std::shared_ptr<Interval> insertNode(std::shared_ptr<Interval> tmp, std::shared_ptr<Interval> newNode);
    uint64_t getRank(std::shared_ptr<Interval> tmp, std::shared_ptr<Interval> interval, uint64_t rank);
    std::shared_ptr<Interval> root;
  private:
};


} // namespace bench