#pragma once

#include <cstdint>
#include <cstdio>
#include <set>
#include <vector>

namespace bench {
template <class T> class VectorQueue {
  public:
	VectorQueue() : vec({}) {}
	~VectorQueue(){};

	void enq(T item) { vec.push_back(item); };

	T deq(T item, uint64_t *rank) {
		for (size_t pos = 0; pos < vec.size(); pos++) {
			if (vec[pos] == item) {
				*rank = pos;
				T &ret = vec[pos];
				vec.erase(vec.begin() + pos);
				return ret;
			}
		}
		fprintf(stderr, "item %lu not found\n", item);
		return 0;
	}

	void batch_deq(std::unordered_map<T, uint64_t> m,
				   std::vector<uint64_t> *ranks) {
		// map of all pops and where they were found
		std::set<int> found_pops;

		int dels = m.size();
		std::set<int> erase_idxs;

		uint64_t found = 0;
		uint64_t idx = 0;
		while (found < dels) {
			uint64_t curr = vec[idx];
			if (m.contains(curr)) {

				uint64_t pop_order = m.at(curr);
				uint64_t rank = idx;
				// fi is the ammount of pops that are in fron of me in the q and
				// in time.
				// uint64_t fi = found_pops.order_of_key(pop_order);
				auto it = found_pops.lower_bound(pop_order);
				uint64_t fi = std::distance(found_pops.cbegin(), it);
				found_pops.insert(pop_order);

				// the ranks is the found rank minus the ammount of pops in
				// front in the q and in the
				ranks->at(pop_order) = rank - fi;
				m.erase(curr);

				erase_idxs.insert(idx);
				found++;
			}
			idx++;
		}
		// actually remove elements from the queue
		for (auto it = erase_idxs.rbegin(); it != erase_idxs.rend(); ++it)
			vec.erase(vec.begin() + *it);
	}

  private:
	std::vector<T> vec;
};
} // namespace bench