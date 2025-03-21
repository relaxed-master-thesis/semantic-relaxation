#pragma once

#include "bench/util/AbstractParser.h"

#include <memory.h>
#include <stdint.h>

namespace bench {
class TimestampParser : public AbstractParser {
  public:
	TimestampParser() = default;
	~TimestampParser() = default;

	InputData parse(const std::string &get, const std::string &put);
};
} // namespace bench
