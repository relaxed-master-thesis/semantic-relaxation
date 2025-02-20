#include "bench/impl/IVTImp.h"
#include "bench/Benchmark.h"
#include "bench/Interval.h"
#include "bench/Operation.h"

#include <cstddef>
#include <cstdint>
#include <stack>
#include <unordered_set>
#include <vector>

namespace bench {
void IVTImp::printTreePretty() {
	std::cout << "Tree {start, end, min, max}:\n";

	auto &arr = tree.getArr();

	auto prindent = [](size_t lvl) -> void {
		for (size_t i = 0; i < lvl; ++i) {
			std::cout << "  ";
		}
	};

	auto prelem = [](Interval &interval) -> void {
		std::cout << "- {" << interval.start << ", " << interval.end << ", "
				  << interval.min << ", " << interval.max << "}\n";
	};

	std::stack<size_t> toPrint{};
	toPrint.push(0);

	while (!toPrint.empty()) {
		size_t i = toPrint.top();
		toPrint.pop();
		prindent(tree.getLevel(i));
		prelem(arr[i]);
		if (tree.hasRightChild(i))
			toPrint.push(tree.rightChild(i));
		if (tree.hasLeftChild(i))
			toPrint.push(tree.leftChild(i));
	}
}

void IVTImp::printTree() {
	auto &arr = tree.getArr();

	std::cout << "arr = [";
	for (size_t i = 0; i < arr.size() - 1; ++i) {
		auto &e = arr[i];
		std::cout << "\t{st=" << e.start << ", en=" << e.end
				  << ", min=" << e.min << ", max=" << e.max << "},\n";
	}
	auto &e = arr[arr.size() - 1];
	std::cout << "\t{st=" << e.start << ", en=" << e.end << ", min=" << e.min
			  << ", max=" << e.max << "}]\n";
}

uint64_t IVTImp::inheritRank(Entry &entry) {
	size_t i = 0;

	auto &iintv = tree.getNode(i);
	if (iintv.start < entry.start && entry.end < iintv.end) {
		if (!iintv.evaluated) {
			std::cout << "Evaluation order error at root!\n";
		}
		return iintv.rank;
	}

	std::stack<size_t> toVisit{};
	toVisit.push(1); // start evaluating the left half of the tree
	while (!toVisit.empty()) {
		size_t i = toVisit.top();
		Entry &data = tree.getNode(i);
		if (data.start < entry.start && entry.end < data.end) {
			if (!data.evaluated) {
				std::cout << "Evaluation order error at " << i << "!\n";
			}
			return data.rank;
		}
		if (tree.hasLeftChild(i)) {
			Interval &left = tree.getNode(tree.leftChild(i));
			if (left.min < entry.start && entry.end < left.max)
				toVisit.push(tree.leftChild(i));
		}
		// Important here to push this last so that the traversal order is from
		// rightmost to leftmost
		if (tree.hasRightChild(i)) {
			Interval &right = tree.getNode(tree.rightChild(i));
			if (right.min < entry.start && entry.end < right.max)
				toVisit.push(tree.rightChild(i));
		}
		toVisit.pop();
	}

	// Unclear what to do here, if we reach this point then there is no
	// containing node in the left half of the tree, idk if this means that the
	std::cout << "Got to the unclear point for entry {" << entry.start << ", "
			  << entry.end << "}\n";
	return 0;
}

uint64_t IVTImp::evalSubtree(size_t root, Entry &entry) {
	std::queue<size_t> toVisit{};
	uint64_t rank = 0;

	toVisit.push(tree.leftChild(root));
	while (!toVisit.empty()) {
		size_t i = toVisit.front();
		Interval &data = tree.getNode(i);
		if (data.start < entry.start && entry.end < data.end) {
			++rank;
		}
		if (tree.hasLeftChild(i)) {
			Interval &left = tree.getNode(tree.leftChild(i));
			if (left.min < entry.start && entry.end < left.max)
				toVisit.push(tree.leftChild(i));
		}
		if (tree.hasRightChild(i)) {
			Interval &right = tree.getNode(tree.rightChild(i));
			if (right.min < entry.start && entry.end < right.max)
				toVisit.push(tree.rightChild(i));
		}
		toVisit.pop();
	}

	tree.getNode(root).rank = rank;
	return rank;
}

uint64_t IVTImp::getRank2(size_t root, Entry &entry) {
	std::queue<size_t> toVisit{};

	// Evaluate left subtree
	if (tree.hasLeftChild(root)) {
		entry.rank += evalSubtree(tree.leftChild(root), entry);
	}

	// If root is right child then there must exist a left child
	if (tree.isRightChild(root)) {
		auto parentIdx = tree.getParent(root);
		auto &parent = tree.getNode(parentIdx);
		if (parent.start < entry.start && entry.end < parent.end) {
			++entry.rank;
		}
		entry.rank += evalSubtree(tree.leftChild(parentIdx), entry);
	}

	// If root belongs to right half of tree
	if (tree.getHalf(root) == 1) {
		entry.rank += inheritRank(entry);
		// Here we want to find the smallest node in the left half that contains
		// ourselves, so we should search from right to left of every level
	}

	entry.evaluated = true;
	return entry.rank;
}

uint64_t IVTImp::getRank(size_t root, Interval &interval) {
	std::queue<size_t> toVisit{};
	std::unordered_set<size_t> visited{};
	if (tree.hasLeftChild(root))
		toVisit.push(tree.leftChild(root));
	if (!tree.isRoot(root) && tree.isRightChild(root))
		toVisit.push(tree.leftChild(tree.getParent(root)));

	// push all indices of nodes to your left on the same level
	size_t currLevel = tree.getLevel(root);
	size_t i = root - 1;
	while (tree.getLevel(i) == currLevel) {
		toVisit.push(i--);
	}

	uint64_t rank = 0;
	while (!toVisit.empty()) {
		size_t i = toVisit.front();
		if (visited.find(i) != visited.end()) {
			toVisit.pop();
			continue;
		}
		visited.insert(i);
		Interval &data = tree.getNode(i);

		// this should never be <= or >= because the data-node might be the
		// interval we are investigating as timestamps are unique
		if (data.start < interval.start && data.end > interval.end) {
			++rank;
		}
		if (tree.isRightChild(i)) {
			Interval &parent = tree.getNode(tree.getParent(i));
			if (parent.min <= interval.start && parent.max >= interval.end)
				toVisit.push(tree.getParent(i));
		}
		if (tree.hasLeftChild(i)) {
			Interval &left = tree.getNode(tree.leftChild(i));
			if (left.min <= interval.start && left.max >= interval.end)
				toVisit.push(tree.leftChild(i));
		}
		if (tree.hasRightChild(i)) {
			Interval &right = tree.getNode(tree.rightChild(i));
			if (right.min <= interval.start && right.max >= interval.end)
				toVisit.push(tree.rightChild(i));
		}
		toVisit.pop();
	}

	return rank;
}

AbstractExecutor::Measurement IVTImp::calcMaxMeanError() {
	// printTree();
	printTreePretty();

	uint64_t rank_sum = 0;
	uint64_t rank_max = 0;

	auto &arr = tree.getArr();
	// #pragma omp parallel for shared(arr) reduction(+ : rank_sum)                   \
    // 	reduction(max : rank_max)
	for (size_t i = 0; i < arr.size(); ++i) {
		Interval &interval = arr.at(i);
		uint64_t rank = getRank(i, interval);
		std::cout << "arr[" << i << "] = {" << interval.start << ", "
				  << interval.end << ", " << interval.min << ", "
				  << interval.max << "} has error " << rank << "\n";
		if (rank > rank_max)
			rank_max = rank;
		rank_sum += rank;
	}
	const double rank_mean = (double)rank_sum / get_stamps_size;

	return {rank_max, rank_mean};
}

// Fix this why is it so slow???
void IVTImp::fix_dup_timestamps() {
	std::cout << "lens: " << put_stamps_size << ", " << get_stamps_size << "\n";
	bool keep_going = true;
	uint64_t next_ins_tick = put_stamps->at(0).time;
	uint32_t ins_ix = 0;
	uint64_t next_del_tick = get_stamps->at(0).time;
	uint32_t del_ix = 0;
	uint64_t time = 0;
	while (keep_going) {
		// fix insert timestamps
		while (ins_ix < put_stamps_size && next_ins_tick <= next_del_tick) {
			next_ins_tick = (*put_stamps)[ins_ix].time;
			(*put_stamps)[ins_ix++].time = time++;
		}
		// fix deletion timestamps
		while (next_del_tick < next_ins_tick) {
			next_del_tick = (*get_stamps)[del_ix].time;
			(*get_stamps)[del_ix++].time = time++;
			if (del_ix >= get_stamps_size) {
				keep_going = false;
				break;
			}
		}
	}
}

void IVTImp::updateMinMax() {
	auto &arr = tree.getArr();
	for (size_t i = arr.size() - 1; i > 0; --i) {
		Interval &node = arr[i];
		Interval &parent = arr[(i - 1) / 2];
		parent.min = std::min(parent.min, node.min);
		parent.max = std::max(parent.max, node.max);
	}
}

void IVTImp::prepare(const InputData &data) {
	put_stamps = std::make_shared<std::vector<Operation>>(*data.getPuts());
	get_stamps = std::make_shared<std::vector<Operation>>(*data.getGets());
	put_stamps_size = put_stamps->size();
	get_stamps_size = get_stamps->size();
	std::cout << "putting timestamps...\n";
	uint64_t time = put_stamps_size + 1;
	for (auto get : *get_stamps) {
		put_map[get.value] = time++;
	}
	time = 1;
	for (auto put : *put_stamps) {
		uint64_t get_time = put_map[put.value];
		segments.emplace_back(time++, get_time);
	}
	tree.build(segments);
	updateMinMax();
}

AbstractExecutor::Measurement IVTImp::execute() { return calcMaxMeanError(); }
} // namespace bench