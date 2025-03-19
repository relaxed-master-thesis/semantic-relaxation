#pragma once

#include "bench/util/QKParser.h"
#include <cstdint>
#include <format>
#include <string>
#include <sys/types.h>
#include <unordered_map>
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
#include <vector>

namespace bench {

enum BenchmarkType { Accurate = 0, Approximate };

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

		bool operator==(const Measurement &other) {
			return std::abs(mean - other.mean) < eps && max == other.max;
		}

		bool operator!=(const Measurement &other) { return !(*this == other); }

		const double eps = 0.0001;
		long double mean;
		uint64_t max;
	};

	virtual void prepare(const InputData &data) = 0;
	virtual Measurement execute() = 0;
	virtual Measurement calcMaxMeanError() = 0;
	virtual void reset() = 0;
	virtual BenchmarkType type() = 0;
};

class AccurateExecutor : public AbstractExecutor {
  public:
	BenchmarkType type() { return BenchmarkType::Accurate; }
};
class ApproximateExecutor : public AbstractExecutor {
  public:
	BenchmarkType type() { return BenchmarkType::Approximate; }
	enum class CountingType { SHARE, AMOUNT };
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

template <class Baseline> class Benchmark {
  public:
	Benchmark() = delete;

	Benchmark(const BenchCfg &cfg) : cfg(cfg) {
		executors.push_back(std::make_shared<Baseline>());
	}

	template <class U, typename... Args> Benchmark &addConfig(Args &&...args) {
		executors.push_back(std::make_shared<U>(std::forward<Args>(args)...));

		return *this;
	}

	Benchmark &loadData() {
		data = bench::TimestampParser().parse(
			cfg.inputDataDir + "/combined_get_stamps.txt",
			cfg.inputDataDir + "/combined_put_stamps.txt");
		//verify that no puts are after gets
		std::unordered_map<uint64_t, uint64_t> getMap{};
		for (const auto &get : *data.getGets()) {
			getMap[get.value] = get.time;
		}
		bool error = false;
		int fails = 0;
		for(const auto &put : *data.getPuts()) {
			if(getMap.find(put.value) == getMap.end()) {
				continue;
			}
			if(put.time > getMap[put.value]) {
				if(!error) {
					std::cerr << "Put after get detected for value " << put.value << "\n";
				}
				fails++;
				error = true;
			}
		}

		if(error) {
			std::cerr << "Error in data, " << fails << " puts after gets in file "
					  << cfg.inputDataDir << "\n";
			exit(1);
		}
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

  private:
	Result runSingle(std::shared_ptr<AbstractExecutor> &executor,
					 const InputData &data) {
		using timepoint =
			std::chrono::time_point<std::chrono::high_resolution_clock>;

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

		auto prepTime = std::chrono::duration_cast<std::chrono::microseconds>(
							prepEnd - prepStart)
							.count();
		auto execTime = std::chrono::duration_cast<std::chrono::microseconds>(
							execEnd - execStart)
							.count();

		return {prepTime, execTime, measurement};
	}

  public:
	void run() {
		for (auto &executor : executors) {
			std::string execName = getTemplateParamTypeName(executor);
			std::cout << "Running " << execName << "...\n";

			auto cumRes = runSingle(executor, data);
			for (size_t i = 0; i < 3 && !cumRes.isValid; ++i) {
				cumRes = runSingle(executor, data);
			}

			if (!cumRes.isValid) {
				std::cerr << execName
						  << " failed to produce result, skipping...\n";
				results.push_back(cumRes);
				continue;
			}

			size_t succ_runs = 1;
			for (size_t i = 1; i < cfg.numRuns; ++i) {
				executor->reset();
				auto ires = runSingle(executor, data);

				if (!ires.isValid) {
					std::cerr << "New result for " << execName
							  << " is invalid, skipping\n";
					continue;
				}

				if (cumRes.measurement != ires.measurement) {
					std::cerr << "Different runs of same executor do not yield "
								 "same measurements, skipping...\n";
					continue;
				}

				cumRes.executeTime += ires.executeTime;
				cumRes.prepareTime += ires.prepareTime;
				succ_runs++;
			}

			cumRes.executeTime /= succ_runs;
			cumRes.prepareTime /= succ_runs;

			results.push_back(cumRes);
		}
	}

	void printResults() {
		// dont look at this too close...
		struct StrResults {
			StrResults(std::string &name, long double mean, uint64_t max,
					   long tot, float tot_speedup, long calc,
					   float calc_speedup, long prep, float prep_speedup,
					   BenchmarkType type)
				: name(name), mean(std::format("{:.2f}", mean)),
				  max(std::to_string(max)),
				  tot(std::format("{} ({:.2f})", timeToNiceStr(tot), tot_speedup)),
				  calc(std::format("{} ({:.2f})", timeToNiceStr(calc), calc_speedup)),
				  prep(std::format("{} ({:.2f})", timeToNiceStr(prep), prep_speedup)),
				  type(type){};

			static std::string timeToNiceStr(long time) {
				if (time < 1'000)
					return std::format("{}us", time);
				if (time < 1'000'000)
					return std::format("{}ms", time / 1000);
				if (time < 1'000'000'000)
					return std::format("{}s", time / 1'000'000);

				auto timeInSeconds = time / 1'000'000;
				auto minutes = timeInSeconds / 60;
				auto remSeconds = timeInSeconds % 60;
				return std::format("{}m{}s", minutes, remSeconds);
			}

			std::string name;
			std::string mean, max, tot, calc, prep;
			BenchmarkType type;
		};

		std::vector<StrResults> sresults{};
		const Result &baseRes = results.at(0);
		for (size_t i = 0; i < executors.size(); ++i) {
			const auto &res = results.at(i);
			const auto &exec = executors.at(i);
			auto mean = res.measurement.mean;
			auto max = res.measurement.max;
			auto tot = res.prepareTime + res.executeTime;
			auto tot_spedup =
				float(baseRes.executeTime + baseRes.prepareTime) / float(tot);
			auto calc = res.executeTime;
			auto calc_speedup = (float)(baseRes.executeTime) / float(calc);
			auto prep = res.prepareTime;
			auto prep_speedup = float(baseRes.prepareTime) / float(prep);
			std::string name = getTemplateParamTypeName(exec);

			// BenchmarkType type = BenchmarkType::Accurate;
			// if (auto appexecptr =
			// 		std::dynamic_pointer_cast<ApproximateExecutor>(
			// 			executors.at(i))) {
			// 	type = BenchmarkType::Approximate;
			// }
			sresults.emplace_back(name, mean, max, tot, tot_spedup, calc,
								  calc_speedup, prep, prep_speedup,
								  exec->type());
		}

		size_t nameLen{4}, meanLen{4}, maxLen{3}, totLen{16}, calcLen{15},
			prepLen{16};
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

		size_t titleLen = std::strlen(cfg.inputDataDir.c_str());
		size_t padding = (totWidth - titleLen - 2) / 2;

		std::cout << std::string(padding, '-') << ' ' << cfg.inputDataDir << ' '
				  << std::string(padding, '-') << "\n";

		std::cout << std::left << std::setw(nameLen) << "Name"
				  << std::setw(meanLen) << "Mean" << std::setw(maxLen) << "Max"
				  << std::setw(totLen) << "Total (speedup)"
				  << std::setw(calcLen) << "Calc (speedup)"
				  << std::setw(prepLen) << "Prep (speedup)" << "\n";

		long double correct_mean = results.at(0).measurement.mean;
		uint64_t correct_max = results.at(0).measurement.max;
		for (size_t i = 0; i < results.size(); ++i) {
			// Green: 	"\033[92m"
			// Yellow: 	"\033[93m"
			// Red: 	"\033[91m"
			// White: 	"\033[37m"
			const auto &res = sresults.at(i);
			const auto &ares = results.at(i);
			std::string meanCol = "\033[92m";
			std::string maxCol = "\033[92m";
			if (res.type == BenchmarkType::Approximate) {
				float mean_diff =
					std::abs(correct_mean - ares.measurement.mean) /
					correct_mean;
				float max_diff =
					(long double)std::abs((int64_t)correct_max -
										  (int64_t)ares.measurement.max) /
					correct_max;
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
				if (std::abs(ares.measurement.mean - correct_mean) > 0.0001) {
					meanCol = "\033[91m";
				}
				if (correct_max != ares.measurement.max) {
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
	const BenchCfg &cfg;
	bench::InputData data;
};
} // namespace bench