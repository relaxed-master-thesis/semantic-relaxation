#include "bench/Benchmark.h"
#include "bench/impl/AITImp.h"
#include "bench/impl/BatchPopImp.h"
#include "bench/impl/FAAImp.h"
#include "bench/impl/FenwickAImp.h"
#include "bench/impl/FenwickImp.h"
#include "bench/impl/GeijerBatchImp.h"
#include "bench/impl/GeijerImp.h"
#include "bench/impl/HeuristicGeijer.h"
#include "bench/impl/IVTImp.h"
#include "bench/impl/MinMax2DDAImp.h"
#include "bench/impl/MonteSweepingLine.h"
#include "bench/impl/ParallelBatchImp.h"
#include "bench/impl/ParallelGeijerImp.h"
#include "bench/impl/ReplayImp.h"
#include "bench/impl/SweepingLineAImp.h"
#include "bench/impl/SweepingLineImp.h"
#include "bench/util/TimestampParser.h"

#include <cstdint>
#include <cstdio>
#include <endian.h>
#include <exception>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
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

	// bench::Benchmark<bench::SweepingLineImp> sweepingImp{};
	// auto sweeping_res = sweepingImp.run(data);

	return 0;
}

std::optional<bench::BenchCfg> parseArguments(int argc, char *argv[]) {
	if (argc <= 1) {
		return std::nullopt;
	}

	std::unordered_map<std::string, std::string> arg_map;
	for (int i = 0; i < argc; ++i) {
		if (i + 1 < argc) {
			arg_map[argv[i]] = argv[i + 1];
		}
	}
	// maybe add checks for invalid arguments

	try {
		return std::make_optional<bench::BenchCfg>(
			{static_cast<size_t>(std::stoi(arg_map["-t"])), arg_map["-i"],
			 static_cast<size_t>(std::stoi(arg_map["-r"]))});
	} catch (std::exception &e) {
		std::cerr << "Invalid arguments, RTM\n";
		return std::nullopt;
	}
}

int main(int argc, char *argv[]) {

	auto optCfg = parseArguments(argc, argv);

	if (!optCfg.has_value())
		return -1;

	// return testWithCreatedData();

	// std::string folder_name = "queue-k-seg-1s-4t";
	// std::string folder_name = "2dd-queue-opt-1ms";
	// std::string folder_name = "generated";
	// std::string folder_name = "FAKE";
	// std::string folder_name = "2ddqopt-4t-i10k";
	// std::string folder_name = "2dd-queue-opt-1s";
	// std::string folder_name = "q-k-1s-8t";
	// std::string folder_name = "q-k-1ms-8t";
	// std::string folder_name = "2dd-queue-opt-1ms-4t-i10k";
	// std::string folder_name = "2dd-queue-opt-100ms";

	// std::string folder_name = "2ddqopt-4t-i10k";
	// std::string folder_name = "2ddqopt-8t-i1M-10ms";
	// std::string folder_name = "2ddqopt-8t-i10k";
	// std::string folder_name = "qkseg-4t-10ms";
	// std::string folder_name = "dcbo-4t-i1M-1ms";

	// std::string folder_name = "2dd-q-opt-w50-l10-i1000-8t-30ms";

	// bench::InputData data = bench::TimestampParser().parse(
	// 	"./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
	// 	"./data/timestamps/" + folder_name + "/combined_put_stamps.txt");

	bench::BenchCfg cfg = optCfg.value();
	// bench::Benchmark<bench::GeijerImp> myBench{cfg};
	bench::Benchmark<bench::FenwickImp> myBench{cfg};

	myBench.loadData()
		.verifyData(true)
		// .addConfig<bench::FenwickImp>()
		// .addConfig<bench::SweepingLineImp>()
		// .addConfig<bench::ParallelBatchImp>(cfg.numAvailableThreads, false)
		// .addConfig<bench::GeijerBatchImp>()
		.addConfig<bench::SweepingLineAImp>(0.1f)
		.addConfig<bench::FenwickAImp>(0.1f)
		.addConfig<bench::MonteSweepingLine>(0.1)
		.addConfig<bench::MinMax2DDAImp>(0.1f, 512, 256)
		.run();
	myBench.printResults();
}