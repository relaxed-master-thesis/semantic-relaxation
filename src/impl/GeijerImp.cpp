#include "bench/impl/GeijerImp.hpp"
#include "bench/util/Executor.hpp"
#include "bench/util/Operation.hpp"

#include <format>
#include <memory>

namespace bench {

AbstractExecutor::Measurement GeijerImp::calcMaxMeanError() {
	uint64_t rank_error_sum = 0;
	uint64_t rank_error_max = 0;

	item *head = put_stamps_head;

	for (size_t deq_ind = 0; deq_ind < get_stamps_size; deq_ind++) {

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
					throw std::runtime_error(
						std::format("Put for elem {} not found\n", key));
				}
			}
			// current->next has the removed item, so just unlink it from the
			// data structure
			current->next = current->next->next;
		}

		// Store rank error in get_stamps for variance calculation
		(*get_stamps)[deq_ind].value = rank_error;

		rank_error_sum += rank_error;
		if (rank_error > rank_error_max)
			rank_error_max = rank_error;
	}
	long double rank_error_mean =
		(long double)rank_error_sum / (long double)get_stamps_size;
	if (get_stamps_size == 0)
		rank_error_mean = 0.0;

	// Find variance
	long double rank_error_variance = 0;
	for (size_t deq_ind; deq_ind < get_stamps_size; deq_ind += 1) {
		long double off =
			(long double)(*get_stamps)[deq_ind].value - rank_error_mean;
		rank_error_variance += off * off;
	}
	rank_error_variance /= get_stamps_size - 1;

	// printf("variance_relaxation , %.4Lf\n", rank_error_variance);

	return {rank_error_max, rank_error_mean};
}

void GeijerImp::prepare(const InputData &data) {

	put_stamps_size = data.getPuts()->size();
	get_stamps_size = data.getGets()->size();
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

AbstractExecutor::Measurement GeijerImp::execute() {
	return calcMaxMeanError();
}
void GeijerImp::reset() {
	get_stamps = nullptr;
	free(put_stamps_head);
	put_stamps_head = nullptr;
	put_stamps_size = 0;
	get_stamps_size = 0;
}

} // namespace bench