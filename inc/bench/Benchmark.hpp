#pragma once

#include "bench/util/Executor.hpp"
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
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace bench {

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

		return *this;
	}

	Benchmark &verifyData(bool cleanup = false) {
		// verify that no puts are after gets
		std::unordered_map<uint64_t, uint64_t> get_val_to_time{};
		std::unordered_map<uint64_t, uint64_t> get_val_to_idx{};
		for (size_t i = 0; i < data.getGets()->size(); ++i) {
			const auto &get = data.getGets()->at(i);
			get_val_to_time[get.value] = get.time;
			get_val_to_idx[get.value] = i;
		}
		bool error = false;
		int fails = 0;
		std::vector<std::pair<size_t, size_t>> to_remove{};
		for (size_t i = 0; i < data.getPuts()->size(); ++i) {
			const auto &put = data.getPuts()->at(i);
			if (get_val_to_time.find(put.value) == get_val_to_time.end()) {
				continue;
			}
			if (put.time > get_val_to_time[put.value]) {
				if (!error && !cleanup) {
					std::cerr << "Put after get detected for value "
							  << put.value << " with time diff: "
							  << put.time - get_val_to_time[put.value] << "\n";
				}
				fails++;
				if (cleanup) {

					size_t get_idx = get_val_to_idx[put.value];
					to_remove.emplace_back(get_idx, i);
				}
				error = true;
			}
		}

		if (error && !cleanup) {
			std::cerr << "\033[91mError in data: " << fails
					  << " put(s) after get(s) in file " << cfg.inputDataDir
					  << "\n\033[0m";
			exit(1);
		}
		if (error && cleanup) {
			std::cout << "Removing " << to_remove.size()
					  << " put(s) after get(s)\n";
			for (size_t i = to_remove.size(); i > 0; --i) {
				auto [get_idx, put_idx] = to_remove.at(i - 1);
				data.removeItem(get_idx, put_idx);
			}
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
	// template <class T> inline void PreventReordering(const T &value) {
	// 	asm volatile("" : : "m"(value) : "memory");
	// }
	Result runSingle(std::shared_ptr<AbstractExecutor> &executor,
					 const InputData &data) {

		using clock_type = typename std::conditional<
			std::chrono::high_resolution_clock::is_steady,
			std::chrono::high_resolution_clock,
			std::chrono::steady_clock>::type;

		AbstractExecutor::Measurement measurement;
		long prepTime{0}, execTime{0};

		try {
			const auto prepStart = clock_type::now();
			executor->prepare(data);
			const auto prepEnd = clock_type::now();
			prepTime = std::chrono::duration_cast<std::chrono::microseconds>(
						   prepEnd - prepStart)
						   .count();
		} catch (const std::exception &e) {
			results.emplace_back(std::string(e.what()));
		}

		try {
			const auto execStart = clock_type::now();
			measurement = executor->execute();
			const auto execEnd = clock_type::now();
			execTime = std::chrono::duration_cast<std::chrono::microseconds>(
						   execEnd - execStart)
						   .count();
		} catch (const std::exception &e) {
			results.emplace_back(std::string(e.what()));
		}

		if (prepTime < 0 || prepTime > 1'000'000'000) {
			std::cerr << "(prep) Start > End: " << prepTime << "\n";
		}

		if (execTime < 0 || execTime > 1'000'000'000) {
			std::cerr << "(prep) Start > End: " << prepTime << "\n";
		}

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
				  tot(std::format("{} ({:.2f})", timeToNiceStr(tot),
								  tot_speedup)),
				  calc(std::format("{} ({:.2f})", timeToNiceStr(calc),
								   calc_speedup)),
				  prep(std::format("{} ({:.2f})", timeToNiceStr(prep),
								   prep_speedup)),
				  type(type){};

			static std::string timeToNiceStr(long time) {
				if (time < 1'000)
					return std::format("{}us", time);
				if (time < 1'000'000)
					return std::format("{:.2f}ms", (float)time / 1000.0);
				if (time < 1'000'000'000)
					return std::format("{:.2f}s", (float)time / 1'000'000.0);

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