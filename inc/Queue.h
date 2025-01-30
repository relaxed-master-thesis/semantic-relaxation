#pragma once

#include <cstdint>
#include <cstdio>
#include <vector>

namespace bench {
template <class T> class Queue {
  public:
	Queue() : vec({}) {}
	~Queue(){};
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

  private:
	std::vector<T> vec;
};
} // namespace bench