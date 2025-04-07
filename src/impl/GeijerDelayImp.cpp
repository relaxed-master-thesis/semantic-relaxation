#include "bench/impl/GeijerDelayImp.hpp"
#include "bench/util/Executor.hpp"
#include <cassert>
#include <cstdint>
#include <format>
#include <iostream>

namespace bench {
void GeijerDelayImp::prepare(const InputData &data) {
	put_stamps_size = data.getPuts()->size();
	get_stamps_size = data.getGets()->size();
	gett_stamps = std::make_shared<std::vector<Operation>>(*data.getGets());
	auto puts = data.getPuts();

	struct item *item_list =
		static_cast<struct item *>(malloc(put_stamps_size * (sizeof(item))));
	for (size_t enq_ind = 0; enq_ind < put_stamps_size; enq_ind += 1) {
		item_list[enq_ind].value = puts->at(enq_ind).value;
		item_list[enq_ind].next = &item_list[enq_ind + 1];
		item_list[enq_ind].delay = 0;
	}
	item_list[put_stamps_size - 1].next = nullptr;
	put_stamps_head = &item_list[0];
}

AbstractExecutor::Measurement GeijerDelayImp::execute() {
	// return {0, 0};
	uint64_t delay_sum = 0;
	uint64_t delay_max = 0;
	uint64_t rank_sum = 0;
	uint64_t rank_max = 0;

	item *head = put_stamps_head;

	for (size_t deq_ind = 0; deq_ind < get_stamps_size; ++deq_ind) {
		uint64_t key = (*gett_stamps)[deq_ind].value;

		if (head == nullptr) {
			break;
		}

		uint64_t rank = 0;
		uint64_t delay = 0;
		if (head->value == key) {
			delay = head->delay;
			head = head->next;
		} else {
			rank = 1;
			++head->delay;
			item *current = head;

			while (current->next->value != key) {
				++current->next->delay;
				current = current->next;
				rank += 1;
				if (current->next == nullptr) {
					// if get doesnt exist for put, just ignore it
					throw std::runtime_error(
						std::format("Put for elem {} not found\n", key));
					rank = 0;
					break;
				}
			}
			delay = current->next->delay;
			current->next = current->next->next;
		}

		(*gett_stamps)[deq_ind].value = rank;

		rank_sum += rank;
		if (rank > rank_max) {
			rank_max = rank;
		}
		delay_sum += delay;
		if (delay > delay_max) {
			delay_max = delay;
		}
	}
	//loop over the rest of the puts
	auto current = head;
	while (current != nullptr) {
		delay_sum += current->delay;
		if (current->delay > delay_max) {
			delay_max = current->delay;
		}
		current = current->next;
	}

	std::cout << "delay_sum: " << delay_sum << std::endl;
	std::cout << "rank_sum: " << rank_sum << std::endl;
	// assert(delay_sum == rank_sum && "Sum of ranks and delays should be equal");
	return {delay_max, (long double)delay_sum / (long double)put_stamps_size};
}

AbstractExecutor::Measurement GeijerDelayImp::calcMaxMeanError() {
	return {0, 0};
}

void GeijerDelayImp::reset() {
	free(put_stamps_head);

	gett_stamps = nullptr;
	put_stamps_head = nullptr;

	put_stamps_size = 0;
	get_stamps_size = 0;
}

} // namespace bench