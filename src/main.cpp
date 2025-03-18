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
#include "bench/impl/MonteSweepingLine.h"
#include "bench/impl/ParallelBatchImp.h"
#include "bench/impl/ParallelGeijerImp.h"
#include "bench/impl/ReplayImp.h"
#include "bench/impl/SweepingLineAImp.h"
#include "bench/impl/SweepingLineImp.h"
#include "bench/util/QKParser.h"

#include <cstdint>
#include <cstdio>
#include <endian.h>
#include <fstream>
#include <memory>
#include <string>
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

struct run_config {
	run_config() : numAvailableThreads(0), inputDataDir("") {}
	run_config(size_t threads, std::string dir) : numAvailableThreads(threads), inputDataDir(dir) {}

	size_t numAvailableThreads{1}; // -n 16 e.g.
	std::string inputDataDir{""};
	size_t numRuns{1};
	
	bool isValid() {
		return !inputDataDir.empty();
	}
};

run_config parseArguments(int argc, char *argv[]) {
	if (argc <= 1) {
		return {};
	}

	// ./build/src/SemanticRelaxation -t 16 -i data/timestamps/2ddqopt-4t-i10k/ -r 3

	std::cout << "argc: " << argc << "\nargv:\n";
	for (int i = 0; i < argc; ++i) {
		std::cout << "argv[" << i << "] = " << argv[i] << "\n"; 
	}
	return {1, ""};
}

int main(int argc, char *argv[]) {

	auto cfg = parseArguments(argc, argv);

	if (!cfg.isValid()) {
		std::cout << "invalid arguments\n";
		return -1;
	}
	return 0;

	// return testWithCreatedData();

	// std::string folder_name = "queue-k-seg-1s-4t";
	// std::string folder_name = "2dd-queue-opt-1ms";
	// std::string folder_name = "generated";
	// std::string folder_name = "FAKE";
	std::string folder_name = "2ddqopt-4t-i10k";
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

	bench::InputData data = bench::TimestampParser().parse(
		"./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
		"./data/timestamps/" + folder_name + "/combined_put_stamps.txt");

	bench::Benchmark<bench::GeijerImp> myBench{};

	myBench.addConfig<bench::FenwickImp>()
		.addConfig<bench::SweepingLineImp>()
		.addConfig<bench::ParallelBatchImp>(cfg.numAvailableThreads, false)
		// .addConfig<bench::SweepingLineAImp>(0.1)
		// .addConfig<bench::MonteSweepingLine>(0.001)
		// .addConfig<bench::FenwickAImp>(0.1)
		.run(data);
	myBench.printResults();
}