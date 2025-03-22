#pragma once

#include <cmath>
#include <queue>
#include <stdint.h>
#include <vector>

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

	// Uses move semantics which leaves `nodes` in an undefined state
	void build(std::vector<T> &nodes) {
		size_t n = nodes.size();
		size = n;
		arr.clear();
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

	// Copies every element which leaves `nodes` in the same state
	void cbuild(const std::vector<T> &nodes) {
		size_t n = nodes.size();
		size = n;
		arr.clear();
		arr.resize(n, T{});
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
			arr[r.index] = nodes[mid];
			q.push({r.start, mid - 1, 2 * r.index + 1});
			q.push({mid + 1, r.end, 2 * r.index + 2});
			q.pop();
		}
	}

	inline size_t getLevel(size_t index) const noexcept {
		return static_cast<size_t>(std::floor(std::log2(index + 1)));
	}

	inline bool isRoot(size_t index) const noexcept { return index == 0; }

	inline size_t getParent(size_t index) const noexcept {
		return (index - 1) / 2;
	}

	// Returns -1 if the node is in the left half of the tree, 1 if it is in the
	// right half of the tree. Returns 0 if the node is the root.
	inline int getHalf(size_t index) const noexcept {
		// Root does not belong to any half
		if (index == 0) {
			return 0;
		}

		auto currLevel = getLevel(index);
		auto levelStartIndex = static_cast<size_t>(std::pow(2, currLevel) - 1);
		auto kidsInLevel = levelStartIndex + 1;
		auto indexInLevel = index - levelStartIndex;
		return indexInLevel <= kidsInLevel / 2 ? -1 : 1;
	}

	inline bool isLeftChild(size_t index) const noexcept {
		return (double(index - 1) / 2.f) - std::floor((index - 1) / 2) >
			   std::numeric_limits<double>::epsilon();
	}

	inline bool isRightChild(size_t index) const noexcept {
		// (double(index - 2) / 2.f) - std::floor((index - 2) / 2) >
		// 	   std::numeric_limits<double>::epsilon();
		return index > 0 && index % 2 == 0;
	}

	inline bool isLeaf(size_t index) const noexcept {
		return !hasLeftChild(index) && !hasRightChild(index);
	}

	inline bool hasLeftChild(size_t index) const noexcept {
		return (2 * index + 1) < arr.size();
	}

	inline bool hasRightChild(size_t index) const noexcept {
		return (2 * index + 2) < arr.size();
	}

	inline size_t leftChild(size_t index) const noexcept {
		return 2 * index + 1;
	}

	inline size_t rightChild(size_t index) const noexcept {
		return 2 * index + 2;
	}

	T &getNode(size_t index) { return arr[index]; }

	std::vector<T> &getArr() { return arr; }

  private:
	size_t size{0};
	std::vector<T> arr{};
};