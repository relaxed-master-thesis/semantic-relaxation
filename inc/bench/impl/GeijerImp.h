#pragma once

#include "bench/Benchmark.h"
#include "bench/ErrorCalculator.h"
#include "bench/Operation.h"

#include <memory>
#include <vector>

namespace bench {
class GeijerImp : public ErrorCalculator, public AbstractExecutor {
  public:
	GeijerImp() = default;
	~GeijerImp() = default;
	Result calcMaxMeanError() override;
	void prepare(InputData data) override;
	long execute() override;

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
