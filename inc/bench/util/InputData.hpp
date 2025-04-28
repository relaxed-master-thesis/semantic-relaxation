#pragma once

#include "bench/util/Operation.hpp"
#include <cstddef>
#include <memory>
#include <vector>

namespace bench {
struct InputData {
	friend class Benchmark;

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
	void removeItem(size_t get_idx, size_t put_idx) {
		gets->erase(gets->begin() + get_idx);
		puts->erase(puts->begin() + put_idx);
	}

  private:
	std::shared_ptr<std::vector<Operation>> gets;
	std::shared_ptr<std::vector<Operation>> puts;
};
} // namespace bench