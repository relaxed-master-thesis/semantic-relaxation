#pragma once

#include "bench/Benchmark.h"
#include "bench/util/VectorTree.h"

#include <cmath>
#include <unordered_map>
#include <vector>

namespace bench {
class IVTImp : public AccurateExecutor {
  public:
	IVTImp() = default;
	~IVTImp() = default;
	AbstractExecutor::Measurement calcMaxMeanError() override;
	void prepare(const InputData &data) override;
	AbstractExecutor::Measurement execute() override;
	void reset() override;

  private:
	struct Interval {
	  public:
		Interval() = default;
		Interval(uint64_t start, uint64_t end)
			: start(start), end(end), min(start), max(end), rank(0) {}
		~Interval() = default;

		uint64_t start, end, min, max, rank;
		bool evaluated;
	};

	uint64_t inheritRank(Interval &interval);
	uint64_t evalSubtree(size_t root, Interval &interval);
	uint64_t getRank(size_t root, Interval &interval);
	uint64_t getRank2(size_t root, Interval &interval);
	void updateMinMax();

	std::shared_ptr<std::vector<Operation>> put_stamps;
	std::shared_ptr<std::vector<Operation>> get_stamps;
	size_t put_stamps_size;
	size_t get_stamps_size;

	std::unordered_map<uint64_t, uint64_t> put_map;
	std::vector<Interval> segments;

	void printTreePretty();
	void printTree();
	void fix_dup_timestamps();
	VectorTree<Interval> tree;
};
} // namespace bench