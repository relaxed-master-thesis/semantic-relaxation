#include "bench/impl/GeijerBatch.h"
#include "bench/Benchmark.h"
#include "bench/ErrorCalculator.h"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <omp.h>

namespace bench {
AbstractExecutor::Measurement GeijerBatch::calcMaxMeanError() {
	uint64_t rank_error_sum = 0;
	uint64_t rank_error_max = 0;

	item *head = put_stamps_head;

#pragma omp parallel for shared(head, rank_error_sum, rank_error_max,          \
									get_stamps, get_stamps_size)
	for (size_t deq_ind = 0; deq_ind < get_stamps_size; deq_ind++) {
		if (deq_ind == 0) [[unlikely]] {
			std::cout << omp_get_num_threads() << " OMP threads\n";
		}

		uint64_t key = (*get_stamps)[deq_ind].value;

		uint64_t rank_error;
		if (head->value == key) {
#pragma omp critical
			{ head = head->next; }
			rank_error = 0;
		} else {
			rank_error = 1;
			item *current = head;
			while (current->next->value != key) {
				current = current->next;
				rank_error += 1;
				if (current->next == NULL) {
					throw std::runtime_error("Out of bounds on finding "
											 "matching relaxation enqueue\n");
				}
			}
// current->next has the removed item, so just unlink it from
// the data structure
#pragma omp critical
			{ current->next = current->next->next; }
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

	printf("variance_relaxation , %.4Lf\n", rank_error_variance);

	return {rank_error_max, rank_error_mean};
}

void GeijerBatch::prepare(const InputData &data) {
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

AbstractExecutor::Measurement GeijerBatch::execute() {
	return calcMaxMeanError();
}

} // namespace bench