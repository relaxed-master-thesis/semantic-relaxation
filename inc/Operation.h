#pragma once

#include <cstdint>

namespace bench {
struct Operation {
	Operation() = default;
	Operation(uint64_t time, uint64_t val) : time(time), value(val) {}

	uint64_t time;
	uint64_t value;
};
} // namespace bench