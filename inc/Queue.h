#include "UnsafeVector.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <stdexcept>
namespace bench {
class Queue {
  public:
	Queue() { vec = UnsafeVector<uint64_t>(); };
	~Queue() {};
	void enq(uint64_t item) { 
        // printf("Adding: %lu\n", item);
        vec.push(item); 
        }
	uint64_t deq(uint64_t item, uint64_t *rank) {
		for (size_t pos = 0; pos < vec.size(); pos++) {
            printf("pos = %lu\n", pos);
            printf("*vec[pos] = %lu\n", *vec[pos]);
			if (*vec[pos] == item) {
				*rank = pos;
				uint64_t ret = *vec[pos];
				vec.pop(pos);

                printf("removing: %lu\n", item);
				return ret;
			}
		}
		fprintf(stderr, "item %lu not found\n", item);
        throw 0;
        return 0;
	}

  private:
	UnsafeVector<uint64_t> vec;
};
} // namespace bench