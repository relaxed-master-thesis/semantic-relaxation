#pragma once

#include <cstdint>

namespace bench {
struct Operation {
	Operation() : time(~0), value(~0) {}
	Operation(uint64_t time, uint64_t val) : time(time), value(val) {}

	uint64_t time;
	uint64_t value;
};
} // namespace bench