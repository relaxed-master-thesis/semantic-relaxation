#pragma once

#include "bench/Operation.h"
#include <cstdint>
#include <format>
#include <initializer_list>
#include <string>
#include <utility>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iomanip> // Include this for std::setw
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
	virtual void reset() = 0;
};

class AccurateExecutor : public AbstractExecutor {};
class ApproximateExecutor : public AbstractExecutor {};

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

enum BenchmarkType { Accurate = 0, Approximate };

template <class Baseline> class Benchmark {
  public:
	Benchmark() { executors.push_back(std::make_shared<Baseline>()); }

	template <class U, typename... Args> Benchmark &addConfig(Args &&...args) {
		executors.push_back(std::make_shared<U>(std::forward<Args>(args)...));

		return *this;
	}

	template <class U>
	static std::string getTemplateParamTypeName(std::shared_ptr<U> u) {
#ifdef __GNUC__
		int status;
		std::string tname = typeid(*u).name();
		char *demangledName = abi::__cxa_demangle(tname.c_str(), 0, 0, &status);
		if (status == 0) {
			tname = demangledName;
			std::free(demangledName);
		}
		return tname;
#elif defined(_MSC_VER)
		return typeid(T).name();
#else
		return typeid(T).name();
#endif
	}

	void run(const InputData &data) {
		using timepoint =
			std::chrono::time_point<std::chrono::high_resolution_clock>;

		for (auto &executor : executors) {
			std::cout << "Running " << getTemplateParamTypeName(executor)
					  << "...\n";
			timepoint prepStart, prepEnd, execStart, execEnd;
			AbstractExecutor::Measurement measurement;

			try {
				prepStart = std::chrono::high_resolution_clock::now();
				executor->prepare(data);
				prepEnd = std::chrono::high_resolution_clock::now();
			} catch (const std::exception &e) {
				results.emplace_back(std::string(e.what()));
			}

			try {
				execStart = std::chrono::high_resolution_clock::now();
				measurement = executor->execute();
				execEnd = std::chrono::high_resolution_clock::now();
			} catch (const std::exception &e) {
				results.emplace_back(std::string(e.what()));
			}

			auto prepTime =
				std::chrono::duration_cast<std::chrono::microseconds>(prepEnd -
																	  prepStart)
					.count();
			auto execTime =
				std::chrono::duration_cast<std::chrono::microseconds>(execEnd -
																	  execStart)
					.count();

			results.emplace_back(prepTime, execTime, measurement);
		}
	}

	void printResults() {
		// dont look at this too close...
		struct StrResults {
			StrResults(std::string &name, long double mean, uint64_t max,
					   uint64_t tot, float tot_speedup, uint64_t calc,
					   float calc_speedup, uint64_t prep, float prep_speedup,
					   BenchmarkType type)
				: name(name), mean(std::to_string(mean)),
				  max(std::to_string(max)),
				  tot(std::format("{} ({:.2f})", tot, tot_speedup)),
				  calc(std::format("{} ({:.2f})", calc, calc_speedup)),
				  prep(std::format("{} ({:.2f})", prep, prep_speedup)),
				  type(type){};

			std::string name;
			std::string mean, max, tot, calc, prep;
			BenchmarkType type;
		};

		std::vector<StrResults> sresults{};
		const Result &baseRes = results.at(0);
		for (size_t i = 0; i < executors.size(); ++i) {
			const auto &res = results.at(i);
			auto mean = res.measurement.mean;
			auto max = res.measurement.max;
			auto tot = res.prepareTime + res.executeTime;
			auto tot_spedup =
				float(baseRes.executeTime + baseRes.prepareTime) / float(tot);
			auto calc = res.executeTime;
			auto calc_speedup = (float)(baseRes.executeTime) / float(calc);
			auto prep = res.prepareTime;
			auto prep_speedup = float(baseRes.prepareTime) / float(prep);
			std::string name = getTemplateParamTypeName(executors.at(i));

			BenchmarkType type = BenchmarkType::Accurate;
			if (auto appexecptr =
					std::dynamic_pointer_cast<ApproximateExecutor>(
						executors.at(i))) {
				type = BenchmarkType::Approximate;
			}
			sresults.emplace_back(name, mean, max, tot, tot_spedup, calc,
								  calc_speedup, prep, prep_speedup, type);
		}

		size_t nameLen{4}, meanLen{4}, maxLen{3}, totLen{14}, calcLen{13},
			prepLen{14};
		for (auto &res : sresults) {

			nameLen = std::max(nameLen, std::strlen(res.name.c_str()));
			meanLen = std::max(meanLen, std::strlen(res.mean.c_str()));
			maxLen = std::max(maxLen, std::strlen(res.max.c_str()));
			totLen = std::max(totLen, std::strlen(res.tot.c_str()));
			calcLen = std::max(calcLen, std::strlen(res.calc.c_str()));
			prepLen = std::max(prepLen, std::strlen(res.prep.c_str()));
		}

		// round up to nearest tab size
		// nameLen +%= 4;
		nameLen += (nameLen % 4) + 4;
		meanLen += (meanLen % 4) + 4;
		maxLen += (maxLen % 4) + 4;
		totLen += (totLen % 4) + 4;
		calcLen += (calcLen % 4) + 4;
		prepLen += (prepLen % 4) + 4;

		size_t totWidth =
			nameLen + meanLen + maxLen + totLen + calcLen + prepLen;
		std::cout << std::string(totWidth, '-') << "\n";

		std::cout << std::left << std::setw(nameLen) << "Name"
				  << std::setw(meanLen) << "Mean" << std::setw(maxLen) << "Max"
				  << std::setw(totLen) << "Total speedup" << std::setw(calcLen)
				  << "Calc speedup" << std::setw(prepLen) << "Prep speedup"
				  << "\n";

		float correct_mean = results.at(0).measurement.mean;
		float correct_max = results.at(0).measurement.max;
		for (auto &res : sresults) {
			// Green: 	"\033[92m"
			// Yellow: 	"\033[93m"
			// Red: 	"\033[91m"
			// White: 	"\033[37m"
			std::string meanCol = "\033[92m";
			std::string maxCol = "\033[92m";
			if (res.type == BenchmarkType::Approximate) {
				float mean_diff =
					std::abs(correct_mean - std::stof(res.mean)) / correct_mean;
				float max_diff =
					std::abs(correct_max - std::stof(res.max)) / correct_max;
				if (mean_diff > 0.1) {
					meanCol = "\033[91m";
				} else if (mean_diff > 0.0001) {
					meanCol = "\033[93m";
				}
				if (max_diff > 0.1) {
					maxCol = "\033[91m";
				} else if (max_diff > 0.0001) {
					maxCol = "\033[93m";
				}
			} else {
				if (std::abs(std::stof(res.mean) - correct_mean) > 0.0001) {
					meanCol = "\033[91m";
				}
				if (std::abs(std::stof(res.max) - correct_max) > 0.0001) {
					maxCol = "\033[91m";
				}
			}

			std::cout << std::left << std::setw(nameLen) << res.name << meanCol
					  << std::setw(meanLen) << res.mean << maxCol
					  << std::setw(maxLen) << res.max << "\033[0m"
					  << std::setw(totLen) << res.tot << std::setw(calcLen)
					  << res.calc << std::setw(prepLen) << res.prep << "\n";
		}

		std::cout << std::string(totWidth, '-') << "\n";
	}

	// std::shared_ptr<GeijerImp> baselineExecutor;

  private:
	std::vector<std::shared_ptr<AbstractExecutor>> executors{};
	std::vector<Result> results{};
};
} // namespace bench