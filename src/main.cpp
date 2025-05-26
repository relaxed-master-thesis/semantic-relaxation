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
#include "bench/impl/stackImpl/FenwickStackImp.hpp"
#include "bench/impl/stackImpl/ReplayTreeStackImp.hpp"
#include "bench/impl/stackImpl/StackReplayImp.hpp"
#include "bench/util/Benchmark.hpp"
#include "bench/util/Executor.hpp"
#include "bench/util/FenwickTree.hpp"

#include <cstdint>
#include <cstdio>
#include <endian.h>
#include <exception>
#include <fstream>
#include <iostream>
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

	// -p for preset?
	using btype = bench::BenchCfg::BenchType;
	btype bType = btype::DEFAULT;
	if (arg_map.find("-p") != arg_map.end()) {
		std::string preset = arg_map["-p"];
		if (preset == "generic") {
			bType = btype::GENERIC;
		} else if (preset == "batch") {
			bType = btype::TEST_BATCH_SIZE;
		} else if (preset == "delay") {
			bType = btype::DELAY;
		} else if (preset == "approx") {
			bType = btype::APPROX;
		} else if (preset == "size") {
			bType = btype::SIZE;
		} else {
			std::cerr << "Invalid preset specified (-p " << preset << ")\n";
			return std::nullopt;
		}

		std::cout << "Preset: " << preset << "\n";
	}

	// -q for queue type
	using qtype = bench::BenchCfg::QueueType;
	qtype qType = qtype::GENERIC;
	if (arg_map.find("-q") != arg_map.end()) {
		std::string queue = arg_map["-q"];
		if (queue == "2ddq") {
			qType = qtype::TWODD_QUEUE;
		} else if (queue == "dcbo") {
			qType = qtype::DCBO_QUEUE;
		} else if (queue == "stack") {
			qType = qtype::STACK;
		} else {
			std::cerr << "Invalid queue specified (-q " << queue << " )\n";
			return std::nullopt;
		}
		std::cout << "Queue: " << queue << "\n";
	}


	if (qType == qtype::TWODD_QUEUE) { 
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
			std::cerr << "No height specified (-l)\n";
			return std::nullopt;
		}
		return std::make_optional<std::pair<bench::BenchCfg, InputInfo>>(
			bench::BenchCfg(numThreads, dir, numRuns, numGets, bType, qType),
			InputInfo{true, width, height});
	}

	if (qType == qtype::DCBO_QUEUE) {
		size_t width = 0;
		size_t wArgPos = dir.find("-w");
		if (wArgPos != std::string::npos) {
			width = std::stoi(dir.c_str() + wArgPos + 2);
		}

		return std::make_optional<std::pair<bench::BenchCfg, InputInfo>>(
			bench::BenchCfg(numThreads, dir, numRuns, numGets, bType, qType),
			InputInfo{false, width, 0});
	}
	
	if (qType == qtype::STACK) {
		return std::make_optional<std::pair<bench::BenchCfg, InputInfo>>(
			bench::BenchCfg(numThreads, dir, numRuns, numGets, bType, qType),
			InputInfo{false, 0, 0});
	}

	std::cerr << "Invalid preset or queue specified (-p or -q)\n";
	return std::nullopt;
}

int testBatchSizes(std::pair<bench::BenchCfg, InputInfo> cfg) {
	// if these are run with all 2dd configs, they are the "perfect" batch sizes
	// for all the rank errors but since the complexity is an approxmation, its
	// not perfect, but the tendency is there
	// k -> best batch
	// 6.56 -> 15.1
	// 27.51 -> 63.34
	// 116.56 -> 268.39
	// 507.86 -> 1169.39
	// 1982.51 -> 4564.9
	// 8057.16 -> 18552.3
	// 33113.71 -> 76247.14
	// std::vector<int> batch_sizes = {15, 63, 268, 1169, 4564, 18552, 76247};
	std::vector<int> batch_sizes = {1, 10, 100, 1000, 10000, 100000, 1000000};
	bench::Benchmark myBench{cfg.first};
	myBench.loadData().verifyData(true).setBaseline<bench::GeijerBatchImp>(
		batch_sizes[0]);

	for (int i = 1; i < batch_sizes.size(); ++i) {
		myBench.addConfig<bench::GeijerBatchImp>(batch_sizes[i]);
	}

	myBench.run();
	myBench.printResults();
	return 0;
}

int runPresetIfDefined(
	bench::Benchmark &myBench, std::optional<std::pair<bench::BenchCfg, InputInfo>> &optCfg, bench::BenchCfg::BenchType bType, bench::BenchCfg::QueueType qType) {

	auto &info = optCfg.value().second;

	using btype = bench::BenchCfg::BenchType;
	using qtype = bench::BenchCfg::QueueType;

	if (qType == qtype::STACK) {
		myBench.setBaseline<bench::StackReplayImp>()
			.addConfig<bench::ReplayTreeStackImp>()
			.addConfig<bench::FenwickStackImp>();
	} else if (bType == btype::GENERIC) {
		myBench.setBaseline<bench::GeijerImp>()
			.addConfig<bench::GeijerBatchImp>()
			.addConfig<bench::ParallelBatchImp>(false)
			.addConfig<bench::ReplayTreeImp>()
			.addConfig<bench::FenwickImp>()
			.addConfig<bench::ParallelFenwickImp>();
	} else if (bType == btype::TEST_BATCH_SIZE) {
		testBatchSizes(optCfg.value());
		std::exit(0);
	} else if (bType == btype::DELAY) {
		myBench.setBaseline<bench::GeijerDelayImp>()
			.addConfig<bench::FenwickDelayImp>();
	} else if (bType == btype::APPROX) {
		myBench.setBaseline<bench::FenwickImp>()
			.addConfig<bench::FenwickAImp>(0.1f)
			.addConfig<bench::ReplayTreeAImp>(0.1f)
			.addConfig<bench::MonteReplayTree>(0.1f)
			.addConfig<bench::MonteFenwickImp>(0.1f);
		
		if (info.is2Ddqopt) {
			myBench.addConfig<bench::MinMax2DDAImp>(0.1f, info.width, info.height);
		}
	} else if(bType == btype::SIZE){
		std::cout << "Running size benchmark with " << optCfg.value().first.numGets << " gets\n";
		myBench.setBaseline<bench::FenwickImp>()
			.addConfig<bench::ReplayTreeImp>();
	}else {
		return 0;
	}

	myBench.run();
	myBench.printResults();
	return 1;
}

int main(int argc, char *argv[]) {
	auto optCfg = parseArguments(argc, argv);

	if (!optCfg.has_value())
		return -1;

	bench::BenchCfg cfg = optCfg.value().first;
	InputInfo info = optCfg.value().second;
	bench::Benchmark myBench{cfg};
	myBench.loadData().verifyData(true);

	if (runPresetIfDefined(myBench, optCfg, cfg.benchType, cfg.queueType)) {
		return 0;
	}

	// myBench.loadData()
	// 	.verifyData(true)
	// 	.setBaseline<bench::StackReplayImp>()
	// 	.addConfig<bench::FenwickStackImp>()
	// 	.addConfig<bench::ReplayTreeStackImp>();
	// 	.setBaseline<bench::GeijerImp>()
	// 	.addConfig<bench::ReplayTreeImp>()
	// .addConfig<bench::FenwickImp>();
	// .addConfig<bench::GeijerDelayImp>();
	// .addConfig<bench::MonteReplayTree>(.1)
	// .addConfig<bench::MonteFenwickImp>(.1);
	// .addConfig<bench::GeijerBatchImp>()
	// 	.addConfig<bench::ReplayTreeImp>()
	// 	.addConfig<bench::FenwickImp>()
	// .addConfig<bench::ParallelBatchImp>(false);
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
	// myBench.run();
	// myBench.printResults();
}