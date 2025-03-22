#pragma once

#include "bench/util/Executor.hpp"
#include "bench/util/InputData.hpp"
#include "bench/util/TimestampParser.hpp"

#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <sys/types.h>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace bench {

struct Result {
  public:
	Result() = delete;
	Result(const std::string &errMsg)
		: isValid(false), errMsg(errMsg), prepareTime(0), executeTime(0),
		  measurement() {}
	Result(long prepTime, long execTime,
		   AbstractExecutor::Measurement measurement)
		: isValid(true), errMsg(""), prepareTime(prepTime),
		  executeTime(execTime), measurement(measurement) {}

	Result &operator=(const Result &other) {
		if (this != &other) {
			isValid = other.isValid;
			errMsg = other.errMsg;
			prepareTime = other.prepareTime;
			executeTime = other.executeTime;
			measurement = other.measurement;
		}
		return *this;
	}

	bool isValid;
	std::string errMsg;
	long prepareTime;
	long executeTime;
	AbstractExecutor::Measurement measurement;
};

struct BenchCfg {
	BenchCfg() : numAvailableThreads(0), inputDataDir("") {}
	BenchCfg(size_t threads, std::string dir, size_t runs)
		: numAvailableThreads(threads), inputDataDir(dir), numRuns(runs) {}

	size_t numAvailableThreads{1}; // -n 16 e.g.
	std::string inputDataDir{""};
	size_t numRuns{1};
};

class Benchmark {
  public:
	Benchmark() = delete;

	Benchmark(const BenchCfg &cfg);

	template <class Base, typename... Args>
	Benchmark &setBaseline(Args &&...args) {
		static_assert(std::is_base_of<AbstractExecutor, Base>::value,
					  "typename must be a subclass of AbstractExecutor");
		assert(executors.empty() && "Baseline must be set first");
		executors.push_back(
			std::make_shared<Base>(std::forward<Args>(args)...));
		return *this;
	}

	template <class T, typename... Args> Benchmark &addConfig(Args &&...args) {
		static_assert(std::is_base_of<AbstractExecutor, T>::value,
					  "typename must be a subclass of AbstractExecutor");
		assert(!executors.empty() && "Baseline must be set first");
		executors.push_back(std::make_shared<T>(std::forward<Args>(args)...));
		return *this;
	}

	Benchmark &loadData();
	Benchmark &verifyData(bool cleanup = false);

	void run();
	void printResults();

  private:

	Result runSingle(std::shared_ptr<AbstractExecutor> executor);
	Result runCumulative(std::shared_ptr<AbstractExecutor> executor,
						 Result &initialResult);

	std::vector<std::shared_ptr<AbstractExecutor>> executors{};
	std::vector<Result> results{};
	const BenchCfg cfg;
	InputData data;
};
} // namespace bench