#pragma once

#include "Benchmark.h"
#include "ErrorCalculator.h"

namespace bench {
class ReplayImp : public ErrorCalculator, public AbstractExecutor {
  public:
	ReplayImp() = default;
	~ReplayImp(){};
	Result calcMaxMeanError() override;
	void prepare(InputData data) override;
	long execute() override;

  private:
	std::shared_ptr<std::vector<Operation>> put_stamps;
	std::shared_ptr<std::vector<Operation>> get_stamps;
	size_t put_stamps_size;
	size_t get_stamps_size;
};
} // namespace bench