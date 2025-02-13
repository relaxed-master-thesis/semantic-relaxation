#include "bench/impl/IVTImp.h"

#include <chrono>
#include <stack>
#include <unordered_set>

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

uint64_t IVTImp::getRank2(size_t root, Interval &interval) { return 0; }

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

	/*
	example: arr = [{st=3, en=8, min=0, max=10},
					{st=1, en=10, min=0, max=10},
					{st=4, en=6, min=4, max=6},
					{st=0, en=9, min=0, max=9},
					{st=2, en=7, min=2, max=7}]

	tree:
		- (3, 8)
			- (1, 10)
				- (0, 9)
				- (2, 7)
			- (4, 6)


	calc err for (3, 8): (2)

	tovisit: (3, 8)x (1,10)
	rank: 		0

	revelation:
		- only your left childs can contain yourself
		- traverse the list


	*/

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

ErrorCalculator::Result IVTImp::calcMaxMeanError() {
	// printTree();
	printTreePretty();

	uint64_t rank_sum = 0;
	uint64_t rank_max = 0;

	auto &arr = tree.getArr();
	// #pragma omp parallel for shared(arr) reduction(+ : rank_sum)                   \
    // 	reduction(max : rank_max)
	for (size_t i = 0; i < arr.size(); ++i) {
		Interval &interval = arr[i];
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

void IVTImp::prepare(InputData data) {
	put_stamps_size = data.puts->size();
	get_stamps_size = data.gets->size();
	put_stamps = data.puts;
	get_stamps = data.gets;
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

long IVTImp::execute() {
	std::cout << "Running IVTImp...\n";
	auto start = std::chrono::high_resolution_clock::now();
	auto result = calcMaxMeanError();
	auto end = std::chrono::high_resolution_clock::now();
	auto duration =
		std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	std::cout << "Runtime: " << duration.count() << " us\n";
	std::cout << "Mean: " << result.mean << ", Max: " << result.max << "\n";
	return duration.count();
}
} // namespace bench