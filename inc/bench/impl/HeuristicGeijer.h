#pragma once

#include "bench/Benchmark.h"
#include "bench/ErrorCalculator.h"
#include "bench/Operation.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace bench {
class HeuristicGeijer : public ErrorCalculator, public AbstractExecutor {
  public:
	HeuristicGeijer() = default;
	~HeuristicGeijer() = default;
	Result calcMaxMeanError() override;
	void prepare(InputData data) override;
	long execute() override;
    void setHeuristicSizeAndCutoff(uint64_t size, uint64_t cutoff) {
        heuristicSize = size;
        mean_cutoff = cutoff;
        heuristicsSet = true;
    }
    void setBatchSize(uint64_t size) {
        batch_size = size;
        batchSizeSet = true;
    }
  private:
    Result getHeuristicResult(size_t gets) {
        return calcMaxMeanErrorGeijer(gets);
    }
	Result calcMaxMeanErrorBatch();
	Result calcMaxMeanErrorGeijer(uint64_t get_stamps_to_use);
	std::shared_ptr<std::vector<Operation>> put_stamps;
	std::shared_ptr<std::vector<Operation>> get_stamps;
	size_t put_stamps_size;
	size_t get_stamps_size;
	struct item {
		uint64_t value;
		item *next;
	};
	item *put_stamps_head;
	item *put_stamps_head_heuristics;
	uint64_t batch_size;
    uint64_t heuristicSize;
    //if heuristic gives higher than this, we will use the batch version
    uint64_t mean_cutoff;
    bool heuristicsSet = false;
    bool batchSizeSet = false;
};
} // namespace bench

