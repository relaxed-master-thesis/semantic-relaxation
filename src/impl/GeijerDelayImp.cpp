#include "bench/impl/GeijerDelayImp.hpp"
#include "bench/util/Executor.hpp"
#include <cstdint>

namespace bench {
void GeijerDelayImp::prepare(const InputData &data) {
	put_stamps_size = data.getPuts()->size();
	get_stamps_size = data.getGets()->size();
	put_stamps = std::make_shared<std::vector<Operation>>(*data.getPuts());
	auto gets = data.getGets();

	struct item *item_list =
		static_cast<struct item *>(malloc(get_stamps_size * (sizeof(item))));
	for (size_t deq_ind = 0; deq_ind < get_stamps_size; deq_ind += 1) {
		item_list[deq_ind].value = gets->at(deq_ind).value;
		item_list[deq_ind].next = &item_list[deq_ind + 1];
	}
	item_list[get_stamps_size - 1].next = nullptr;
	get_stamps_head = &item_list[0];
}

AbstractExecutor::Measurement GeijerDelayImp::execute() {
	// return {0, 0};
	uint64_t delay_sum = 0;
	uint64_t delay_max = 0;
	uint64_t rank_sum = 0;
	uint64_t rank_max = 0;

	item *head = get_stamps_head;

	for (size_t enq_ind = 0; enq_ind < put_stamps_size; ++enq_ind) {
		uint64_t key = (*put_stamps)[enq_ind].value;

		if (head == nullptr) {
			break;
		}

		uint64_t rank = 0;
		if (head->value == key) {
			head = head->next;
		} else {
			rank = 1;
			head->delay++;
			item *current = head;
			if (current->next == nullptr) {
				// if get doesnt exist for put, just ignore it
				rank = 0;
			} else {
				while (current->next->value != key) {
					current->next->delay++;
					current = current->next;
					rank += 1;
					if (current->next == nullptr) {
						// if get doesnt exist for put, just ignore it
						rank = 0;
						break;
					}
				}
			}

			current->next = current->next->next;
			
		}

		(*put_stamps)[enq_ind].value = rank;

		rank_sum += rank;
		if (rank > rank_max) {
			rank_max = rank;
		}
	}

	return {delay_max, (long double)delay_sum / (long double)get_stamps_size};
}

AbstractExecutor::Measurement GeijerDelayImp::calcMaxMeanError() {
	return {0, 0};
}

void GeijerDelayImp::reset() {
	free(get_stamps_head);

	put_stamps = nullptr;
	get_stamps_head = nullptr;

	put_stamps_size = 0;
	get_stamps_size = 0;
}

} // namespace bench