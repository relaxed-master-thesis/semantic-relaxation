#pragma once

#include "bench/Operation.h"
#include "bench/Interval.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <type_traits>
#include <vector>

namespace bench {

struct InputData {
  public:
	InputData(std::shared_ptr<std::vector<Operation>> gets,
			  std::shared_ptr<std::vector<Operation>> puts)
		: gets(gets), puts(puts) {}

	std::shared_ptr<std::vector<Operation>> gets;
	std::shared_ptr<std::vector<Operation>> puts;
};

class AbstractParser {
  public:
	virtual InputData parse(const std::string &get, const std::string &put) = 0;
};

class AbstractExecutor {
  public:
	virtual void prepare(InputData data) = 0;
	virtual long execute() = 0;
};

// TODO: enforce shared data format between parser and executor
// or create shared format
template <class T, class V>
class Benchmark {
  public:
	Benchmark() {
		static_assert(std::is_base_of<AbstractParser, T>::value,
					  "parser type does not derive AbstractParser");
		static_assert(std::is_base_of<AbstractExecutor, V>::value,
					  "executor type does not derive AbstractExecutor");

		parser = std::make_shared<T>();
		executor = std::make_shared<V>();
	}

	long run(const std::string &getOps, const std::string &putOps,
			 const std::string &output) {
		std::shared_ptr<AbstractParser> aparser =
			std::dynamic_pointer_cast<AbstractParser>(this->parser);
		std::shared_ptr<AbstractExecutor> aexecutor =
			std::dynamic_pointer_cast<AbstractExecutor>(this->executor);

		InputData data = aparser->parse(getOps, putOps);
		std::cout << "parse successful\n";
		aexecutor->prepare(data);
		std::cout << "prepare successful\n";
		return aexecutor->execute();
	}

	//   private:
	std::shared_ptr<T> parser;
	std::shared_ptr<V> executor;
};
} // namespace bench