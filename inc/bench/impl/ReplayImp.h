#pragma once

#include "bench/Benchmark.h"
#include "bench/ErrorCalculator.h"

namespace bench {
class ReplayImp : public AbstractExecutor {
  public:
	ReplayImp() = default;
	~ReplayImp(){};
	AbstractExecutor::Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	AbstractExecutor::Measurement execute() override;

  private:
	std::shared_ptr<const std::vector<Operation>> put_stamps;
	std::shared_ptr<const std::vector<Operation>> get_stamps;
	size_t put_stamps_size;
	size_t get_stamps_size;
};
} // namespace bench