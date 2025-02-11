#pragma once

#include "Benchmark.h"
#include "ErrorCalculator.h"
#include "Operation.h"
#include "AugmentedIntervalTree.h"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>
namespace bench {
class AITImp : public ErrorCalculator, public AbstractExecutor {
  public:
	AITImp() = default;
	~AITImp() = default;
	Result calcMaxMeanError() override;
	void prepare(InputData data) override;
	long execute() override;


  private:
	std::shared_ptr<std::vector<Operation>> put_stamps;
	std::shared_ptr<std::vector<Operation>> get_stamps;
	size_t put_stamps_size;
	size_t get_stamps_size;

    std::unordered_map<uint64_t, uint64_t> put_map;
    std::vector<std::shared_ptr<Interval>> segments;

	AugmentedIntervalTree ait;

	void fix_dup_timestamps();


};
} // namespace bench

