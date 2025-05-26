#pragma once

#include "bench/util/Executor.hpp"
#include "bench/util/InputData.hpp"

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
	enum class BenchType {
		DEFAULT = 0,
		GENERIC,
		TEST_BATCH_SIZE,
		DELAY,
		APPROX,
		SIZE
	};

	enum class QueueType {
		GENERIC = 0,
		TWODD_QUEUE,
		DCBO_QUEUE,
		STACK
	};

	BenchCfg() : numAvailableThreads(0), inputDataDir(""), isStack(false) {}
	BenchCfg(size_t threads, std::string dir, size_t runs, size_t numGets, BenchType bType, QueueType qType)
		: numAvailableThreads(threads), inputDataDir(dir), numRuns(runs),
		  numGets(numGets), benchType(bType), queueType(qType), isStack(qType == QueueType::STACK) {}

	size_t numAvailableThreads{1}; // -n 16 e.g.
	std::string inputDataDir{""};
	size_t numRuns;
	size_t numGets; // -g 100 e.g.
	BenchType benchType{BenchType::DEFAULT};
	QueueType queueType{QueueType::GENERIC};
	bool isStack;
};

class Benchmark {
	friend struct InputData;
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
		dataStructureType = executors.back()->dataStructureType();
		bool validStack = cfg.isStack &&
						  dataStructureType == DataStructureType::Stack;
		bool validQueue = !cfg.isStack &&
						  dataStructureType == DataStructureType::Queue;
		assert(validStack || validQueue && 
			   "Data structure type must be the same as baseline");
		return *this;
	}

	template <class T, typename... Args> Benchmark &addConfig(Args &&...args) {
		static_assert(std::is_base_of<AbstractExecutor, T>::value,
					  "typename must be a subclass of AbstractExecutor");
		assert(!executors.empty() && "Baseline must be set first");
		assert(dataStructureType == T().dataStructureType() &&
			   "Data structure type must be the same as baseline");
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

	void removePutsAfterGets(bool cleanup);
	void fixDuplicateValues();

	std::vector<std::shared_ptr<AbstractExecutor>> executors{};
	std::vector<Result> results{};
	const BenchCfg cfg;
	InputData data;
	DataStructureType dataStructureType{DataStructureType::Queue};
};
} // namespace bench