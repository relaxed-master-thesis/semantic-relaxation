#pragma once

#include "bench/util/Executor.hpp"
#include "bench/util/InputData.hpp"
#include "bench/util/Operation.hpp"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

namespace bench {
class ParallelBox : public AccurateExecutor {
  public:

    struct range {
        size_t from, to;
    };

	ParallelBox(size_t height, size_t width) : height(height), width(width) {}
    ~ParallelBox() = default;

	Measurement execute() override;
	Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	void reset() override;

  private:
    std::pair<uint64_t, uint64_t> calcBox(range r);

    size_t height{0}, width{0};
    std::vector<range> ranges;
    std::unordered_map<uint64_t, uint64_t> getMap{};
	// std::vector<std::pair<int, int>> points{};
    std::shared_ptr<const std::vector<Operation>> puts;
    std::shared_ptr<const std::vector<Operation>> gets;
};
} // namespace bench