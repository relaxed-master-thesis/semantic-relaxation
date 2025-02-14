#pragma once

#include "bench/Benchmark.h"

#include <memory.h>
#include <stdint.h>

namespace bench {

class QKParser : public AbstractParser {
  public:
	QKParser() = default;
	~QKParser() = default;

	// std::shared_ptr<std::vector<Operation>> parseFile(std::string &filename);
	InputData parse(const std::string &get, const std::string &put);
};
} // namespace bench
