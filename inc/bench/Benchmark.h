#pragma once

#include "bench/Operation.h"

#ifdef __GNUC__
#include <cxxabi.h>
#endif

#include <cassert>
#include <chrono>
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

class AbstractParser {
  public:
	virtual InputData parse(const std::string &get, const std::string &put) = 0;
};

class AbstractExecutor {
  public:
	struct Measurement {
		Measurement() = default;
		Measurement(uint64_t max, long double mean) : max(max), mean(mean) {}
		Measurement(Measurement &&) = default;
		Measurement &operator=(const Measurement &other) {
			if (this != &other) {
				max = other.max;
				mean = other.mean;
			}
			return *this;
		}
		Measurement(const Measurement &other)
			: max(other.max), mean(other.mean) {}

		long double mean;
		uint64_t max;
	};

	virtual void prepare(const InputData &data) = 0;
	virtual Measurement execute() = 0;
	virtual Measurement calcMaxMeanError() = 0;
};

// TODO: enforce shared data format between parser and executor
// or create shared format
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

	const bool isValid;
	const std::string errMsg;
	const long prepareTime;
	const long executeTime;
	const AbstractExecutor::Measurement measurement;
};

template <class T> class Benchmark {
  public:
	Benchmark() {
		static_assert(std::is_base_of<AbstractExecutor, T>::value,
					  "typename does not derive AbstractExecutor");

		executor = std::make_shared<T>();
	}

	template <typename... Args> Benchmark(Args &&...args) {
		static_assert(std::is_base_of<AbstractExecutor, T>::value,
					  "typename does not derive AbstractExecutor");

		executor = std::make_shared<T>(std::forward<Args>(args)...);
	}

	std::string getTemplateParamTypeName() {
#ifdef __GNUC__
		int status;
		std::string tname = typeid(T).name();
		char *demangledName = abi::__cxa_demangle(tname.c_str(), 0, 0, &status);
		if (status == 0) {
			tname = demangledName;
			std::free(demangledName);
		}
		return tname;
#else
		// idk how to do this on compilers other than gcc
		// this works for MSVC also :)
		return typeid(T).name(); 
#endif
	}

	Result run(const InputData &data) {
		using timepoint =
			std::chrono::time_point<std::chrono::high_resolution_clock>;

		std::shared_ptr<AbstractExecutor> aexecutor =
			std::dynamic_pointer_cast<AbstractExecutor>(this->executor);

		std::cout << "Running " << getTemplateParamTypeName() << "...\n";
		timepoint prepStart, prepEnd, execStart, execEnd;
		AbstractExecutor::Measurement measurement;

		try {
			prepStart = std::chrono::high_resolution_clock::now();
			aexecutor->prepare(data);
			prepEnd = std::chrono::high_resolution_clock::now();
		} catch (const std::exception &e) {
			return {std::string(e.what())};
		}

		try {
			execStart = std::chrono::high_resolution_clock::now();
			measurement = aexecutor->execute();
			execEnd = std::chrono::high_resolution_clock::now();
		} catch (const std::exception &e) {
			return {std::string(e.what())};
		}

		auto prepTime = std::chrono::duration_cast<std::chrono::microseconds>(
							prepEnd - prepStart)
							.count();
		auto execTime = std::chrono::duration_cast<std::chrono::microseconds>(
							execEnd - execStart)
							.count();

		return {prepTime, execTime, measurement};
	}

	//   private:
	std::shared_ptr<T> executor;
};
} // namespace bench