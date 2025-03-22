#pragma once

#include "bench/util/Executor.hpp"

namespace bench {
class BatchPopImp : public AccurateExecutor {
  public:
	BatchPopImp() = default;
	~BatchPopImp(){};
	AbstractExecutor::Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	AbstractExecutor::Measurement execute() override;
	void reset() override;

  private:
	std::shared_ptr<const std::vector<Operation>> put_stamps;
	std::shared_ptr<const std::vector<Operation>> get_stamps;
	size_t put_stamps_size;
	size_t get_stamps_size;
};
} // namespace bench