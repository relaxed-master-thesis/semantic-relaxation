#pragma once

#include "Benchmark.h"
#include "ErrorCalculator.h"

#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_set>

namespace bench {
template <typename T> class VectorTree {
  private:
	struct Reorder {
		size_t start;
		size_t end;
		size_t index;
	};

  public:
	VectorTree() = default;
	~VectorTree() = default;

	void build(std::vector<T> &nodes) {
		size_t n = nodes.size();
		arr.resize(n, T{}); // initialize with default values
		std::queue<Reorder> q{};
		q.push({0, n - 1, 0});
		while (!q.empty()) {
			Reorder &r = q.front();
			if (r.start > r.end || r.index >= n) {
				q.pop();
				continue;
			}
			size_t num_elements = r.end - r.start + 1;
			size_t h = static_cast<size_t>(std::floor(std::log2(num_elements)));
			size_t num_elements_last_level = static_cast<size_t>(
				std::min(std::pow(2, h), num_elements - std::pow(2, h) + 1));
			size_t mid = r.start + static_cast<size_t>(std::pow(2, h - 1) - 1) +
						 std::min(num_elements_last_level,
								  static_cast<size_t>(std::pow(2, h - 1)));
			arr[r.index] = std::move(nodes[mid]);
			q.push({r.start, mid - 1, 2 * r.index + 1});
			q.push({mid + 1, r.end, 2 * r.index + 2});
			q.pop();
		}
	}

	bool hasLeftChild(size_t index) {
		return 2 * index + 1 < arr.size();
	}

	bool hasRightChild(size_t index) {
		return 2 * index + 2 < arr.size();
	}

	size_t leftChild(size_t index) { return 2 * index + 1; }

	size_t rightChild(size_t index) { return 2 * index + 2; }

	T &getNode(size_t index) {
		assert(index < arr.size());
		return arr[index];
	}

	const std::vector<T> &getArr() { return arr; }

  private:
	std::vector<T> arr{};
};

class IVTImp : public ErrorCalculator, public AbstractExecutor {
  public:
	IVTImp() = default;
	~IVTImp() = default;
	Result calcMaxMeanError() override;
	void prepare(InputData data) override;
	long execute() override;
};
} // namespace bench