#include "bench/util/Benchmark.hpp"
#include "bench/util/Executor.hpp"
#include "bench/util/InputData.hpp"
#include <exception>
#include <format>
#include <fstream>
#include <memory>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace bench {
static std::string
getTemplateParamTypeName(std::shared_ptr<AbstractExecutor> ptr) {
#ifdef __GNUC__
	int status;
	std::string tname = typeid(*ptr).name();
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

Benchmark::Benchmark(const BenchCfg &cfg) : cfg(cfg) {}

Benchmark &Benchmark::loadData() {
	std::string getOps = cfg.inputDataDir + "/combined_get_stamps.txt";

	uint64_t time, value;
	std::ifstream getFile(getOps);
	auto gets = std::make_shared<std::vector<Operation>>();
	if (!getFile.good()) {
		throw std::runtime_error("File \"" + getOps + "\" not found\n");
		return *this;
	}
	size_t getCount = 0;
	while (getCount++ < cfg.numGets && getFile >> time >> value) {
		gets->emplace_back(time, value);
	}

	std::string putOps = cfg.inputDataDir + "/combined_put_stamps.txt";
	std::ifstream putFile(putOps);
	if (!putFile.good()) {
		throw std::runtime_error("File \"" + putOps + "\" not found\n");
		return *this;
	}

	auto puts = std::make_shared<std::vector<Operation>>();
	uint64_t lastGetTime = time;
	time = 0;
	while (time < lastGetTime && putFile >> time >> value) {
		puts->emplace_back(time, value);
	}

	data = InputData(gets, puts);

	return *this;
}

Benchmark &Benchmark::verifyData(bool cleanup) {
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
				std::cerr << "Put after get detected for value " << put.value
						  << " with time diff: "
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

Result Benchmark::runSingle(std::shared_ptr<AbstractExecutor> executor) {
	using clock_type =
		typename std::conditional<std::chrono::high_resolution_clock::is_steady,
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
		executor.reset();
		return {"Prepare failed: " + std::string(e.what())};
	}

	// If the clock is not monotonic we cannot trust the measurements
	if (prepTime < 0 || prepTime > 1'000'000'000'000) {
		return {std::format("Prepare time invalid: {}", prepTime)};
	} else if (prepTime == 0) {
		prepTime = 1;
	}

	try {
		const auto execStart = clock_type::now();
		measurement = executor->execute();
		const auto execEnd = clock_type::now();
		execTime = std::chrono::duration_cast<std::chrono::microseconds>(
					   execEnd - execStart)
					   .count();
	} catch (const std::exception &e) {
		executor.reset();
		return {"Execute failed: " + std::string(e.what())};
	}

	if (execTime < 0 || execTime > 1'000'000'000'000) {
		return {std::format("Execute time invalid: {}", execTime)};
	} else if (execTime == 0) {
		execTime = 1;
	}

	// Reset the executor for future runs
	executor->reset();

	return {prepTime, execTime, measurement};
}

Result Benchmark::runCumulative(std::shared_ptr<AbstractExecutor> executor,
								Result &initialResult) {
	std::string execName = getTemplateParamTypeName(executor);
	long prepTimeSum = 0;
	long execTimeSum = 0;

	// Use `long` to avoid casting on return
	long successfulRuns = 0;
	long fails = 0;

	// allow for two failed runs
	while (fails < 2 && successfulRuns < cfg.numRuns) {
		auto ires = runSingle(executor);

		if (!ires.isValid) {
			std::cerr << "New result for " << execName
					  << " is invalid, skipping\n";
			++fails;
			continue;
		}

		if (initialResult.measurement != ires.measurement) {
			std::cerr << "Different runs of " << execName
					  << " do not yield same measurements, skipping...\n";
			++fails;
			std::cout << "Initial: " << initialResult.measurement.mean << " "
					  << initialResult.measurement.max
					  << " New: " << ires.measurement.mean << " "
					  << ires.measurement.max << "\n";
			continue;
		}

		prepTimeSum += ires.prepareTime;
		execTimeSum += ires.executeTime;
		++successfulRuns;

		executor->reset();
	}

	if (fails > 2) {
		return {std::format("{} failed {}/{} runs", execName, fails,
							fails + successfulRuns)};
	}

	return {prepTimeSum / successfulRuns, execTimeSum / successfulRuns,
			initialResult.measurement};
}

void Benchmark::run() {
	for (auto &executor : executors) {
		std::string execName = getTemplateParamTypeName(executor);
		std::cout << "Running " << execName << "...\n";

		// Do a dry run first to warm up the cache
		auto dryRunResult = runSingle(executor);

		// If the dry run fails we cannot trust the cache
		// so it would be unfair to compare the results
		if (!dryRunResult.isValid) {
			std::cerr << "Dry run for " << execName << " failed\n";
			results.push_back(dryRunResult);
			continue;
		}

		auto cumulativeResult = runCumulative(executor, dryRunResult);

		if (!cumulativeResult.isValid) {
			std::cerr << execName << " failed to produce result: "
					  << cumulativeResult.errMsg << "\n";
		}

		results.push_back(cumulativeResult);
	}
}

struct TableEntry {
  private:
	TableEntry(const std::string &name, long double mean, uint64_t max,
			   long tot, float totSpeedup, long calc, float calcSpeedup,
			   long prep, float prepSpeedup);

  public:
	TableEntry(const std::string &name, const Result &res,
			   const Result &baselineRes);
	TableEntry(const std::string &name, const std::string &errMsg);

	static std::string timeToNiceStr(long time) {
		if (time < 1'000)
			return std::format("{}us", std::max(1L, time));
		if (time < 1'000'000)
			return std::format("{:.2f}ms", (float)time / 1000.0);
		if (time < 60'000'000)
			return std::format("{:.2f}s", (float)time / 1'000'000.0);

		auto timeInSeconds = time / 1'000'000;
		auto minutes = timeInSeconds / 60;
		auto remSeconds = timeInSeconds % 60;
		return std::format("{}m{}s", minutes, remSeconds);
	}

	const bool isValid;
	const std::string name, errMsg;
	const long double mean;
	const uint64_t max;
	const long tot, calc, prep;
	const float totSpeedup, calcSpeedup, prepSpeedup;

	const std::string smean, smax, stot, scalc, sprep;
};

// rip this ctor
TableEntry::TableEntry(const std::string &name, long double mean, uint64_t max,
					   long tot, float totSpeedup, long calc, float calcSpeedup,
					   long prep, float prepSpeedup)
	: isValid(true), name(name), mean(mean), max(max), tot(tot),
	  totSpeedup(totSpeedup), calc(calc), calcSpeedup(calcSpeedup), prep(prep),
	  prepSpeedup(prepSpeedup), smean(std::format("{:.2f}", mean)) ,
	  smax(std::to_string(max)),
	  stot(std::format("{} ({:.2f})", timeToNiceStr(tot), totSpeedup)),
	  scalc(std::format("{} ({:.2f})", timeToNiceStr(calc), calcSpeedup)),
	  sprep(std::format("{} ({:.2f})", timeToNiceStr(prep), prepSpeedup)) {}

TableEntry::TableEntry(const std::string &name, const Result &res,
					   const Result &baselineRes)
	: TableEntry::TableEntry(
		  name, res.measurement.mean, res.measurement.max,
		  res.prepareTime + res.executeTime,
		  float(baselineRes.prepareTime + baselineRes.executeTime) /
			  float(res.prepareTime + res.executeTime),
		  res.executeTime,
		  float(baselineRes.executeTime) / float(res.executeTime),
		  res.prepareTime,
		  float(baselineRes.prepareTime) / float(res.prepareTime)) {}

TableEntry::TableEntry(const std::string &name, const std::string &errMsg)
	: isValid(false), errMsg(errMsg), name(name), mean(0), max(0), tot(0),
	  totSpeedup(0), calc(0), calcSpeedup(0), prep(0), prepSpeedup(0),
	  smean(""), smax(""), stot(""), scalc(""), sprep("") {}

void Benchmark::printResults() {
	// Minimum column widths
	size_t nameLen{4}, meanLen{4}, maxLen{3}, totLen{16}, calcLen{15},
		prepLen{16};

	std::vector<TableEntry> entries{};
	const Result &baseRes = results.at(0);

	// Construct entries with string representations
	for (size_t i = 0; i < executors.size(); ++i) {
		const auto &res = results.at(i);

		if (!res.isValid) {
			entries.emplace_back(getTemplateParamTypeName(executors.at(i)),
								 res.errMsg);
			continue;
		}

		std::string name = getTemplateParamTypeName(executors.at(i));
		TableEntry entry{name, res, baseRes};
		entries.emplace_back(name, res, baseRes);

		nameLen = std::max(nameLen, entry.name.size());
		meanLen = std::max(meanLen, entry.smean.size());
		maxLen = std::max(maxLen, entry.smax.size());
		totLen = std::max(totLen, entry.stot.size());
		calcLen = std::max(calcLen, entry.scalc.size());
		prepLen = std::max(prepLen, entry.sprep.size());
	}

	// Round up to neareset 2 tab sizes
	nameLen += (nameLen % 4) + 4;
	meanLen += (meanLen % 4) + 4;
	maxLen += (maxLen % 4) + 4;
	totLen += (totLen % 4) + 4;
	calcLen += (calcLen % 4) + 4;
	prepLen += (prepLen % 4) + 4;

	size_t totWidth = nameLen + meanLen + maxLen + totLen + calcLen + prepLen;

	std::string title = cfg.inputDataDir + " gets: " + std::to_string(cfg.numGets);

	size_t titleLen = std::strlen(title.c_str());
	size_t padding = (totWidth - titleLen - 2) / 2;

	std::cout << std::string(padding, '-') << ' ' << title << ' '
			  << std::string(padding, '-') << "\n";

	std::cout << std::left << std::setw(nameLen) << "Name" << std::setw(meanLen)
			  << "Mean" << std::setw(maxLen) << "Max" << std::setw(totLen)
			  << "Total (speedup)" << std::setw(calcLen) << "Calc (speedup)"
			  << std::setw(prepLen) << "Prep (speedup)" << "\n";

	long double correct_mean = baseRes.measurement.mean;
	uint64_t correct_max = baseRes.measurement.max;

	const std::string ANSI_Green = "\033[92m";
	const std::string ANSI_Yellow = "\033[93m";
	const std::string ANSI_Red = "\033[91m";
	const std::string ANSI_White = "\033[37m";

	for (size_t i = 0; i < entries.size(); ++i) {
		const auto &entry = entries.at(i);
		if (!entry.isValid) {
			std::cout << std::left << std::setw(nameLen) << entry.name
					  << ANSI_Red << std::setw(meanLen) << entry.errMsg
					  << ANSI_White << "\n";
			continue;
		}

		std::string meanCol = ANSI_Green;
		std::string maxCol = ANSI_Green;
		if (executors.at(i)->type() == BenchmarkType::Approximate) {
			float meanDiff = std::abs(correct_mean - entry.mean) / correct_mean;
			float maxDiff =
				std::abs((int64_t)correct_max - (int64_t)entry.max) /
				(float)correct_max;
			if (meanDiff > 0.1) {
				meanCol = ANSI_Red;
			} else if (meanDiff > 0.01) {
				meanCol = ANSI_Yellow;
			}
			if (maxDiff > 0.1) {
				maxCol = ANSI_Red;
			} else if (maxDiff > 0.01) {
				maxCol = ANSI_Yellow;
			}
		} else {
			if (std::abs(entry.mean - correct_mean) > 0.0001) {
				meanCol = ANSI_Red;
			}
			if (entry.max != correct_max) {
				maxCol = ANSI_Red;
			}
		}

		std::cout << std::left << std::setw(nameLen) << entry.name << meanCol
				  << std::setw(meanLen) << entry.smean << maxCol
				  << std::setw(maxLen) << entry.smax << "\033[0m"
				  << std::setw(totLen) << entry.stot << std::setw(calcLen)
				  << entry.scalc << std::setw(prepLen) << entry.sprep << "\n";
	}

	std::cout << std::string(totWidth, '-') << "\n";
}

} // namespace bench