#include "bench/impl/GeijerBatchImp.hpp"
#include "bench/util/Executor.hpp"
#include "bench/util/GNUOrderedSet.hpp"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <unordered_map>

namespace bench {
AbstractExecutor::Measurement GeijerBatchImp::calcMaxMeanError() {

	if (get_stamps_size == 0)
		return {0, 0};

	uint32_t ins_ix = 0;
	uint32_t del_ix = 0;

	uint64_t rank_sum = 0;
	uint64_t rank_max = 0;

	bool keep_running = true;
	size_t batches_done = 0;
	// add heuristic?
	uint64_t batch_size = 10000;
	size_t total_batches_needed = get_stamps_size / batch_size;
	// printf("Batches of size %lu needed: %lu\n", batch_size,
	//    total_batches_needed);

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
		ordered_set<uint64_t, std::less<uint64_t>> found_pops;
		uint64_t found = 0;
		while (found < dels && pops.contains(head->value)) {
			uint64_t pop_order = pops.at(head->value);
			uint64_t fi = found_pops.order_of_key(pop_order);
			found_pops.insert(pop_order);
			uint64_t rank_error = found - fi;
			rank_sum += rank_error;
			if (rank_error > rank_max)
				rank_max = rank_error;

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

void GeijerBatchImp::prepare(const InputData &data) {
	put_stamps_size = data.getPuts()->size();
	get_stamps_size = data.getGets()->size();
	put_stamps = std::make_shared<std::vector<Operation>>(*data.getPuts());
	get_stamps = std::make_shared<std::vector<Operation>>(*data.getGets());
	auto puts = data.getPuts();
	struct item *item_list =
		static_cast<struct item *>(malloc(put_stamps_size * (sizeof(item))));
	for (size_t enq_ind = 0; enq_ind < put_stamps_size; enq_ind += 1) {
		item_list[enq_ind].value = puts->at(enq_ind).value;
		item_list[enq_ind].next = &item_list[enq_ind + 1];
	}
	item_list[put_stamps_size - 1].next = NULL;
	put_stamps_head = &item_list[0];
}

AbstractExecutor::Measurement GeijerBatchImp::execute() {
	return calcMaxMeanError();
}
void GeijerBatchImp::reset() {
	free(put_stamps_head);
	put_stamps_head = NULL;
	put_stamps_size = 0;
	get_stamps_size = 0;
	get_stamps = nullptr;
	put_stamps = nullptr;
}

} // namespace bench