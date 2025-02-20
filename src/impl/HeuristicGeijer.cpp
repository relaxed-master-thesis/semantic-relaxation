#include "bench/impl/HeuristicGeijer.h"
#include "bench/Benchmark.h"
#include "bench/Operation.h"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>
#include <unordered_map>

#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
#include <vector>

using namespace __gnu_pbds;

// Ordered set that supports order statistics
template <typename T>
using ordered_set = tree<T, null_type, std::less<T>, rb_tree_tag,
						 tree_order_statistics_node_update>;

namespace bench {

AbstractExecutor::Measurement HeuristicGeijer::calcMaxMeanError() {
	if (!heuristicsSet) {
		throw std::runtime_error("Heuristic size and cutoff not set\n");
	}
	if (!batchSizeSet) {
		throw std::runtime_error("Batch size not set\n");
	}
	auto heuristicResult = getHeuristicResult(heuristicSize);
	printf("Heuristic values: size %lu, cutoff %lu, mean %LF, max %lu\n",
		   heuristicSize, mean_cutoff, heuristicResult.mean,
		   heuristicResult.max);

	if (heuristicResult.mean > mean_cutoff) {
		// here we could use the mean and max from the heuristic to set the
		// batch size
		return calcMaxMeanErrorBatch();
	} else {
		return calcMaxMeanErrorGeijer(get_stamps_size);
	}
}

AbstractExecutor::Measurement HeuristicGeijer::calcMaxMeanErrorBatch() {

	if (get_stamps_size == 0)
		return {0, 0};

	uint32_t ins_ix = 0;
	uint32_t del_ix = 0;

	uint64_t rank_sum = 0;
	uint64_t rank_max = 0;
	// set rank_min to the maximum value of an unsigned int
	uint64_t rank_min = UINT64_MAX;

	bool keep_running = true;
	size_t batches_done = 0;

	size_t total_batches_needed = get_stamps_size / batch_size;
	printf("Batches of size %lu needed: %lu\n", batch_size,
		   total_batches_needed);

	item *head = put_stamps_head;
	while (keep_running) {

		// map from value to insert order 0...n
		std::unordered_map<uint64_t, uint64_t> pops;
		/* Do deletions. */
		uint64_t ins = 0;
		while (ins < batch_size) {
			uint64_t val = (*get_stamps)[del_ix++].value;
			pops.insert({val, ins++});
			if (del_ix >= get_stamps_size) {
				keep_running = false;
				break;
			}
		}

		int dels = pops.size();
		// map of all pops and where they were found
		ordered_set<int> found_pops;

		uint64_t found = 0;
		while (found < dels && pops.contains(head->value)) {
			uint64_t pop_order = pops.at(head->value);
			uint64_t fi = found_pops.order_of_key(pop_order);
			found_pops.insert(pop_order);
			uint64_t rank_error = found - fi;
			rank_sum += rank_error;
			if (rank_error > rank_max)
				rank_max = rank_error;
			if (rank_error < rank_min)
				rank_min = rank_error;

			head = head->next;
			found++;
		}
		uint64_t idx = found;
		item *current = head;
		while (found < dels) {
			idx++;
			if (pops.contains(current->next->value)) {

				uint64_t pop_order = pops.at(current->next->value);
				uint64_t rank = idx;
				// fi is the ammount of pops that are in fron of me in the q and
				// in time.
				uint64_t fi = found_pops.order_of_key(pop_order);
				found_pops.insert(pop_order);
				assert(rank >= fi);
				uint64_t rank_error = rank - fi;

				rank_sum += rank_error;
				if (rank_error > rank_max)
					rank_max = rank_error;
				if (rank_error < rank_min)
					rank_min = rank_error;

				current->next = current->next->next;
				found++;
			} else {
				current = current->next;
			}
		}
	}
	const double rank_mean = (double)rank_sum / get_stamps_size;

	return {rank_max, rank_mean};
}
AbstractExecutor::Measurement
HeuristicGeijer::calcMaxMeanErrorGeijer(uint64_t get_stamps_to_use) {
	bool is_heuristic = get_stamps_to_use == heuristicSize;

	uint64_t rank_error_sum = 0;
	uint64_t rank_error_max = 0;

	item *head = is_heuristic ? put_stamps_head_heuristics : put_stamps_head;

	for (size_t deq_ind = 0; deq_ind < get_stamps_to_use; deq_ind++) {

		uint64_t key = (*get_stamps)[deq_ind].value;

		uint64_t rank_error;
		if (head->value == key) {
			head = head->next;
			rank_error = 0;
		} else {
			rank_error = 1;
			item *current = head;
			while (current->next->value != key) {
				current = current->next;
				rank_error += 1;
				if (current->next == NULL) {
					throw std::runtime_error("Out of bounds on finding matching relaxation enqueue\n");
				}
			}
			// current->next has the removed item, so just unlink it from the
			// data structure
			current->next = current->next->next;
		}

		// Store rank error in get_stamps for variance calculation
		if (!is_heuristic)
			(*get_stamps)[deq_ind].value = rank_error;

		rank_error_sum += rank_error;
		if (rank_error > rank_error_max)
			rank_error_max = rank_error;
	}
	long double rank_error_mean =
		(long double)rank_error_sum / (long double)get_stamps_to_use;
	if (get_stamps_to_use == 0)
		rank_error_mean = 0.0;
	if (is_heuristic)
		return {rank_error_max, rank_error_mean};

	// Find variance
	long double rank_error_variance = 0;
	for (size_t deq_ind; deq_ind < get_stamps_to_use; deq_ind += 1) {
		long double off =
			(long double)(*get_stamps)[deq_ind].value - rank_error_mean;
		rank_error_variance += off * off;
	}
	rank_error_variance /= get_stamps_to_use - 1;

	printf("variance_relaxation , %.4Lf\n", rank_error_variance);

	return {rank_error_max, rank_error_mean};
}

void HeuristicGeijer::prepare(const InputData &data) {
	put_stamps_size = data.getPuts()->size();
	get_stamps_size = data.getGets()->size();
	put_stamps = std::make_shared<std::vector<Operation>>(*data.getPuts());
	get_stamps = std::make_shared<std::vector<Operation>>(*data.getGets());

	struct item *item_list =
		static_cast<struct item *>(malloc(put_stamps_size * (sizeof(item))));
	struct item *heuristics_item_list =
		static_cast<struct item *>(malloc(put_stamps_size * (sizeof(item))));

	for (size_t enq_ind = 0; enq_ind < put_stamps_size; enq_ind += 1) {
		item_list[enq_ind].value = put_stamps->at(enq_ind).value;
		item_list[enq_ind].next = &item_list[enq_ind + 1];
		heuristics_item_list[enq_ind].value = put_stamps->at(enq_ind).value;
		heuristics_item_list[enq_ind].next = &heuristics_item_list[enq_ind + 1];
	}

	item_list[put_stamps_size - 1].next = NULL;
	heuristics_item_list[put_stamps_size - 1].next = NULL;
	put_stamps_head = &item_list[0];
	put_stamps_head_heuristics = &heuristics_item_list[0];
}

AbstractExecutor::Measurement HeuristicGeijer::execute() {
	return calcMaxMeanError();
}

} // namespace bench