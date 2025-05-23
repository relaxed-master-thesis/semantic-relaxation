#pragma once

#include "bench/util/Executor.hpp"
#include "bench/util/Operation.hpp"

#include <memory>
#include <vector>

namespace bench {
class GeijerBatchImp : public AccurateQueueExecutor {
  public:
	GeijerBatchImp() = default;
	GeijerBatchImp(uint64_t batch_size) : batch_size(batch_size) {};
	~GeijerBatchImp() = default;
	AbstractExecutor::Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	AbstractExecutor::Measurement execute() override;
	void reset() override;

	bool hasName() override { return true; }
	std::string name() override {
		return "bench::GeijerBatchImp_" + std::to_string(batch_size);
	}

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

	// Default value, found in the paper.
	uint64_t batch_size = 10000;
};
} // namespace bench
