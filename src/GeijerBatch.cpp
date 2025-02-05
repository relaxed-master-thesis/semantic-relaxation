#include "GeijerBatch.h"
#include "Benchmark.h"
#include "ErrorCalculator.h"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <omp.h>

namespace bench {
ErrorCalculator::Result GeijerBatch::calcMaxMeanError() {
	uint64_t rank_error_sum = 0;
	uint64_t rank_error_max = 0;

	item *global = put_stamps_head;
	item *head;

#pragma omp parallel shared(global, rank_error_sum, rank_error_max) private(head)
	{
		int n_threads = omp_get_num_threads();
		int thread_id = omp_get_thread_num();

		const size_t num_local_items = put_stamps_size / n_threads;
		head = static_cast<item *>(
			malloc((put_stamps_size / n_threads) * sizeof(item)));
		size_t start = num_local_items * thread_id;
		size_t end = std::min(start + num_local_items, put_stamps_size);
		(void)memcpy(head, &global[start], (end - start) * sizeof(item));

#pragma parallel for private(num_local_items, head, deq_ind) shared(get_stamps)
		{
			for (size_t deq_ind = start; deq_ind < end; deq_ind++) {

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
							perror(
								"Out of bounds on finding matching relaxation "
								"enqueue\n");
							printf("%zu\n", deq_ind);
                            std::exit(-1);
						}
					}
					// current->next has the removed item, so just unlink it
					// from the data structure
					current->next = current->next->next;
				}

				// Store rank error in get_stamps for variance calculation
				(*get_stamps)[deq_ind].value = rank_error;

				rank_error_sum += rank_error;
				if (rank_error > rank_error_max)
					rank_error_max = rank_error;
			}
		}
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

void GeijerBatch::prepare(InputData data) {
	put_stamps_size = data.puts->size();
	get_stamps_size = data.gets->size();
	get_stamps = data.gets;

	struct item *item_list =
		static_cast<struct item *>(malloc(put_stamps_size * (sizeof(item))));
	for (size_t enq_ind = 0; enq_ind < put_stamps_size; enq_ind += 1) {
		item_list[enq_ind].value = (*data.puts)[enq_ind].value;
		item_list[enq_ind].next = &item_list[enq_ind + 1];
	}
	item_list[put_stamps_size - 1].next = NULL;
	put_stamps_head = &item_list[0];
}

void GeijerBatch::execute() {
	std::cout << "Running GeijerBatch...\n";
	auto start = std::chrono::high_resolution_clock::now();
	auto result = calcMaxMeanError();
	auto end = std::chrono::high_resolution_clock::now();
	auto duration =
		std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	std::cout << "Runtime: " << duration.count() << " us\n";
	std::cout << "Mean: " << result.mean << ", Max: " << result.max << "\n";
}

} // namespace bench