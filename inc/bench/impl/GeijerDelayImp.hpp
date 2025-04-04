#pragma once

#include "bench/util/Executor.hpp"

namespace bench {
class GeijerDelayImp : public AccurateExecutor {
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
        item *next;
    };
    std::shared_ptr<std::vector<Operation>> put_stamps;
    item *get_stamps_head;
    size_t put_stamps_size;
    size_t get_stamps_size;
};
} // namespace bench