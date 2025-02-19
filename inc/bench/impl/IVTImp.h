#pragma once

#include "bench/Benchmark.h"
#include "bench/ErrorCalculator.h"
#include "bench/Interval.h"
#include "bench/util/VectorTree.h"

#include <cmath>
#include <unordered_map>
#include <vector>

namespace bench {
class IVTImp : public ErrorCalculator, public AbstractExecutor {
  public:
	IVTImp() = default;
	~IVTImp() = default;
	Result calcMaxMeanError() override;
	void prepare(InputData data) override;
	long execute() override;

  private:
	class Entry : public Interval {
	  public:
	  	Entry() = default;
		Entry(uint64_t start, uint64_t end) : Interval(start, end) {}
		~Entry() = default;

		uint64_t rank;
		bool evaluated;
	};

	uint64_t inheritRank(Entry &interval);
	uint64_t evalSubtree(size_t root, Entry &interval);
	uint64_t getRank(size_t root, Interval &interval);
	uint64_t getRank2(size_t root, Entry &interval);
	void updateMinMax();

	std::shared_ptr<std::vector<Operation>> put_stamps;
	std::shared_ptr<std::vector<Operation>> get_stamps;
	size_t put_stamps_size;
	size_t get_stamps_size;

	std::unordered_map<uint64_t, uint64_t> put_map;
	std::vector<Entry> segments;

	void printTreePretty();
	void printTree();
	void fix_dup_timestamps();
	VectorTree<Entry> tree;
};
} // namespace bench