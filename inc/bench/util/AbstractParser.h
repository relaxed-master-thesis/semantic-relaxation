#pragma once

#include "bench/util/InputData.h"

namespace bench {

class AbstractParser {
  public:
	virtual InputData parse(const std::string &get, const std::string &put) = 0;
};
} // namespace bench