#pragma once

#include "bench/util/Executor.hpp"
#include <cstdint>

namespace bench {
class GeijerDelayImp : public AccurateQueueExecutor {
  public:
    GeijerDelayImp() = default;
    ~GeijerDelayImp() = default;

	void prepare(const InputData &data) override;
	Measurement execute() override;
	Measurement calcMaxMeanError() override;
	void reset() override;

    private:
    struct item {
        uint64_t value;
        uint64_t delay;
        item *next;
    };
    std::shared_ptr<std::vector<Operation>> gett_stamps;
    item *put_stamps_head;
    size_t put_stamps_size;
    size_t get_stamps_size;
};
} // namespace bench