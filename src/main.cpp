#include "bench/impl/AITImp.hpp"
#include "bench/impl/BatchPopImp.hpp"
#include "bench/impl/FAAImp.hpp"
#include "bench/impl/FenwickAImp.hpp"
#include "bench/impl/FenwickDelayImp.hpp"
#include "bench/impl/FenwickImp.hpp"
#include "bench/impl/GeijerBatchImp.hpp"
#include "bench/impl/GeijerDelayImp.hpp"
#include "bench/impl/GeijerImp.hpp"
#include "bench/impl/HeuristicGeijer.hpp"
#include "bench/impl/IVTImp.hpp"
#include "bench/impl/MinMax2DDAImp.hpp"
#include "bench/impl/MonteFenwickImp.hpp"
#include "bench/impl/MonteReplayTree.hpp"
#include "bench/impl/ParallelBatchImp.hpp"
#include "bench/impl/ParallelBoxImp.hpp"
#include "bench/impl/ParallelFenwickImp.hpp"
#include "bench/impl/ParallelGeijerImp.hpp"
#include "bench/impl/ReplayImp.hpp"
#include "bench/impl/ReplayTreeAImp.hpp"
#include "bench/impl/ReplayTreeImp.hpp"
#include "bench/util/Benchmark.hpp"
#include "bench/util/FenwickTree.hpp"

#include <cstdint>
#include <cstdio>
#include <endian.h>
#include <exception>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

static void createAndSaveData(size_t size, const std::string &filename) {
	// we need to create a random set of sequences
	// they will have to have unique timestamps and values.
	// some will end before the others start
	// we will have to save them to a file
	// we will have to return them as a vector

	bool done = false;
	std::vector<bench::Operation> puts;
	std::vector<bench::Operation> gets;
	std::unordered_set<uint64_t> values;
	std::unordered_set<uint64_t> timestamps;
	while (!done) {
		uint64_t value = rand() % size / 2;
		uint64_t start_time = rand() % size;
		uint64_t end_time = start_time + rand() % size;
		if (values.find(value) == values.end() &&
			timestamps.find(start_time) == timestamps.end() &&
			timestamps.find(end_time) == timestamps.end()) {
			values.insert(value);
			timestamps.insert(start_time);
			timestamps.insert(end_time);
			puts.emplace_back(start_time, value);
			gets.emplace_back(end_time, value);
		}
		if (values.size() == size / 2) {
			done = true;
		}
	}
	std::ofstream put_file(filename + "_put_stamps.txt");
	std::ofstream get_file(filename + "_get_stamps.txt");
	// sort puts in time order
	std::sort(puts.begin(), puts.end(),
			  [](const bench::Operation &a, const bench::Operation &b) {
				  return a.time < b.time;
			  });
	for (auto &put : puts) {
		put_file << put.time << " " << put.value << std::endl;
	}
	// sort gets in time order
	std::sort(gets.begin(), gets.end(),
			  [](const bench::Operation &a, const bench::Operation &b) {
				  return a.time < b.time;
			  });
	for (auto &get : gets) {
		get_file << get.time << " " << get.value << std::endl;
	}
	put_file.close();
	get_file.close();
}
static int testWithCreatedData() {
	// std::string folder_name = "generated";
	// createAndSaveData(1000, "./data/timestamps/" + folder_name +
	// "/combined");

	// bench::InputData data = bench::TimestampParser().parse(
	// 	"./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
	// 	"./data/timestamps/" + folder_name + "/combined_put_stamps.txt");

	// bench::Benchmark<bench::GeijerImp> geijerBench{};
	// auto geijer_res = geijerBench.run(data);

	// bench::Benchmark<bench::ReplayTreeImp> sweepingImp{};
	// auto sweeping_res = sweepingImp.run(data);

	return 0;
}

struct InputInfo {
	bool is2Ddqopt;
	size_t width;
	size_t height;
};

std::optional<std::pair<bench::BenchCfg, InputInfo>>
parseArguments(int argc, char *argv[]) {
	if (argc <= 1) {
		return std::nullopt;
	}

	std::unordered_map<std::string, std::string> arg_map;
	for (int i = 0; i < argc; ++i) {
		if (i + 1 < argc) {
			arg_map[argv[i]] = argv[i + 1];
		}
	}

	size_t numThreads = std::thread::hardware_concurrency();
	if (arg_map.find("-t") != arg_map.end()) {
		numThreads = std::stoi(arg_map["-t"]);
	}

	size_t numRuns = 3;
	if (arg_map.find("-r") != arg_map.end()) {
		numRuns = std::stoi(arg_map["-r"]);
	}

	size_t numGets = 0;
	if (arg_map.find("-g") != arg_map.end()) {
		numGets = std::stoi(arg_map["-g"]);
	}

	std::string dir{""};
	if (arg_map.find("-i") != arg_map.end()) {
		dir = arg_map["-i"];
	} else {
		std::cerr << "No directory specified (-i)\n";
		return std::nullopt;
	}

	// if string contains "2Ddqopt" then we are using 2Ddqopt
	if (dir.find("2ddqopt") != std::string::npos) {
		size_t width = 0;
		size_t height = 0;
		size_t wArgPos = dir.find("-w");
		size_t hArgPos = dir.find("-l");
		if (wArgPos != std::string::npos) {
			width = std::stoi(dir.c_str() + wArgPos + 2);
		} else {
			std::cerr << "No width specified (-w)\n";
			return std::nullopt;
		}
		if (hArgPos != std::string::npos) {
			height = std::stoi(dir.c_str() + hArgPos + 2);
		} else {
			std::cerr << "No height specified (-h)\n";
			return std::nullopt;
		}
		return std::make_optional<std::pair<bench::BenchCfg, InputInfo>>(
			bench::BenchCfg(numThreads, dir, numRuns, numGets),
			InputInfo{true, width, height});
	}

	if (dir.find("dcbo") != std::string::npos) {
		size_t width = 0;
		size_t wArgPos = dir.find("-w");
		if (wArgPos != std::string::npos) {
			width = std::stoi(dir.c_str() + wArgPos + 2);
		}

		return std::make_optional<std::pair<bench::BenchCfg, InputInfo>>(
			bench::BenchCfg(numThreads, dir, numRuns, numGets),
			InputInfo{false, width, 0});
	}

	std::cerr << "Invalid data directory (\"" << dir
			  << "\"), must be 2Ddqopt or dcbo\n";
	return std::nullopt;
}

int main(int argc, char *argv[]) {

	auto optCfg = parseArguments(argc, argv);

	if (!optCfg.has_value())
		return -1;

	bench::BenchCfg cfg = optCfg.value().first;
	InputInfo info = optCfg.value().second;
	// bench::Benchmark<bench::GeijerImp> myBench{cfg};
	bench::Benchmark myBench{cfg};

	myBench.loadData()
		.verifyData(true)
		.setBaseline<bench::GeijerImp>()
		.addConfig<bench::ReplayTreeImp>()
	.addConfig<bench::FenwickImp>()
	// .addConfig<bench::GeijerDelayImp>();
	// .addConfig<bench::MonteReplayTree>(.1)
	// .addConfig<bench::MonteFenwickImp>(.1);
		.addConfig<bench::GeijerBatchImp>()
	// 	.addConfig<bench::ReplayTreeImp>()
	// 	.addConfig<bench::FenwickImp>()
		.addConfig<bench::ParallelBatchImp>(false);
	// 	.addConfig<bench::ParallelFenwickImp>();
	// if (info.is2Ddqopt) {
	// 	myBench.addConfig<bench::ParallelBoxImp>(info.width, info.height);
	// }
	// myBench.addConfig<bench::FenwickAImp>(0.1f)
	// 	.addConfig<bench::MonteReplayTree>(0.1)
	// 	.addConfig<bench::ReplayTreeAImp>(0.1f);
	// if (info.is2Ddqopt) {
	// 	myBench.addConfig<bench::MinMax2DDAImp>(0.1f, info.width, info.height);
	// }
	// .addConfig<bench::FenwickImp>()
	// .addConfig<bench::FenwickImp>()
	// .addConfig<bench::ReplayTreeImp>()
	// .addConfig<bench::ParallelBatchImp>(cfg.numAvailableThreads, false)
	// .addConfig<bench::GeijerBatchImp>()
	myBench.run();
	myBench.printResults();
}