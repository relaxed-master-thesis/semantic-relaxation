#pragma once

#include "bench/Benchmark.h"
#include "bench/ErrorCalculator.h"
#include "bench/Operation.h"

#include <memory>
#include <vector>

namespace bench {
class GeijerBatchPopImp : public ErrorCalculator, public AbstractExecutor {
  public:
	GeijerBatchPopImp() = default;
	~GeijerBatchPopImp() = default;
	Result calcMaxMeanError() override;
	void prepare(InputData data) override;
	long execute() override;

  private:
	std::shared_ptr<std::vector<Operation>> put_stamps;
	std::shared_ptr<std::vector<Operation>> get_stamps;
	size_t put_stamps_size;
	size_t get_stamps_size;
	struct item {
		uint64_t value;
		item *next;
	};
	item *put_stamps_head;
};
} // namespace bench
