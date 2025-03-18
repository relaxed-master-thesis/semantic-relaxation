#pragma once

#include "bench/Operation.h"
#include <memory>
#include <vector>

namespace bench {
struct InputData {
  public:
	InputData(std::shared_ptr<std::vector<Operation>> gets,
			  std::shared_ptr<std::vector<Operation>> puts)
		: gets(gets), puts(puts) {}
	InputData() = default;
	InputData &operator=(const InputData &other) {
		if (this != &other) {
			gets = other.gets;
			puts = other.puts;
		}
		return *this;
	}

	std::shared_ptr<const std::vector<Operation>> getGets() const {
		return gets;
	}
	std::shared_ptr<const std::vector<Operation>> getPuts() const {
		return puts;
	}

  private:
	std::shared_ptr<std::vector<Operation>> gets;
	std::shared_ptr<std::vector<Operation>> puts;
};
} // namespace bench