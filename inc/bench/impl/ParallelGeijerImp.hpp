#pragma once

#include "bench/util/Operation.hpp"
#include "bench/util/Executor.hpp"

#include <memory.h>
#include <vector>

namespace bench {
class ParallelGeijerImp : public AccurateExecutor {
  public:
	ParallelGeijerImp() = default;
	~ParallelGeijerImp() = default;
	AbstractExecutor::Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	AbstractExecutor::Measurement execute() override;
	void reset() override;

  private:
	struct item {
		uint64_t value;
		item *next;
	};
	std::shared_ptr<std::vector<Operation>> get_stamps;
	item *put_stamps_head;
	size_t put_stamps_size;
	size_t get_stamps_size;
};
} // namespace bench