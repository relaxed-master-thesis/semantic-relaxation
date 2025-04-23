#pragma once

#include "bench/util/Executor.hpp"
#include "bench/util/InputData.hpp"
#include "bench/util/Operation.hpp"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

namespace bench {
class ParallelBoxImp : public AccurateQueueExecutor {
  public:

    struct range {
        size_t from, to;
    };

	ParallelBoxImp(size_t height, size_t width) : height(height), width(width) {}
    ~ParallelBoxImp() = default;

	Measurement execute() override;
	Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	void reset() override;

  private:
    std::pair<uint64_t, uint64_t> calcBox(size_t tid, range r);

    size_t height{0}, width{0};
    std::vector<range> ranges;
    std::shared_ptr<const std::vector<Operation>> puts;
    std::shared_ptr<const std::vector<Operation>> gets;

    std::vector<std::vector<uint64_t>> range_pp_orders{};
};
} // namespace bench