#pragma once

#include "bench/Benchmark.h"
#include "bench/Operation.h"

#include <memory>
#include <vector>

namespace bench {
class GeijerImp : public AbstractExecutor {
  public:
	GeijerImp() = default;
	~GeijerImp() = default;
	AbstractExecutor::Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	AbstractExecutor::Measurement execute() override;

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
