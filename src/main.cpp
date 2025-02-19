#include "bench/Benchmark.h"
#include "bench/impl/AITImp.h"
#include "bench/impl/BatchPopImp.h"
#include "bench/impl/GeijerBatch.h"
#include "bench/impl/GeijerBatchPopImp.h"
#include "bench/impl/GeijerImp.h"
#include "bench/impl/HeuristicGeijer.h"
#include "bench/impl/IVTImp.h"
#include "bench/impl/ParallelBatchImp.h"
#include "bench/impl/ReplayImp.h"
#include "bench/impl/SweepingLineImp.h"
#include "bench/util/QKParser.h"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <memory>
#include <string>

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
		uint64_t value = rand() % size/2;
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
		if (values.size() == size/2) {
			done = true;
		}

		
	}
	std::ofstream put_file(filename + "_put_stamps.txt");
	std::ofstream get_file(filename + "_get_stamps.txt");
	//sort puts in time order
	std::sort(puts.begin(), puts.end(),
			  [](const bench::Operation &a, const bench::Operation &b) {
				  return a.time < b.time;
			  });
	for (auto &put : puts) {
		put_file << put.time << " " << put.value << std::endl;
	}
	//sort gets in time order
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
	std::string folder_name = "generated";
	createAndSaveData(1000, "./data/timestamps/" + folder_name + "/combined");
	bench::Benchmark<bench::QKParser, bench::GeijerImp> geijerBench{};
	long geijer_duration = geijerBench.run(
		"./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
		"./data/timestamps/" + folder_name + "/combined_put_stamps.txt",
		"./data/timestamps/" + folder_name + "/output.txt");
	bench::Benchmark<bench::QKParser, bench::SweepingLineImp> sweepingImp{};
	long sweeping_duration = sweepingImp.run(
		"./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
		"./data/timestamps/" + folder_name + "/combined_put_stamps.txt",
		"./data/timestamps/" + folder_name + "/output.txt");
	
	return 0;
}

int main(int argc, char *argv[]) {

	// return testWithCreatedData();

	// std::string folder_name = "queue-k-seg-1s-4t";
	// std::string folder_name = "2dd-queue-opt-1ms";
	// std::string folder_name = "generated";
	// std::string folder_name = "2dd-queue-opt-30ms";
	// std::string folder_name = "2dd-queue-opt-1s";
	std::string folder_name = "q-k-1s-8t";
	// std::string folder_name = "q-k-1ms-8t";
	// std::string folder_name = "FAKE";
	// std::string folder_name = "2dd-queue-opt-100ms";
	// std::string folder_name = "2dd-queue-opt-500ms";
	bench::Benchmark<bench::QKParser, bench::GeijerImp> geijerBench{};
	long geijer_duration = geijerBench.run(
		"./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
		"./data/timestamps/" + folder_name + "/combined_put_stamps.txt",
		"./data/timestamps/" + folder_name + "/output.txt");

	bench::Benchmark<bench::QKParser, bench::SweepingLineImp> sweepingImp{};
	long sweeping_duration = sweepingImp.run(
		"./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
		"./data/timestamps/" + folder_name + "/combined_put_stamps.txt",
		"./data/timestamps/" + folder_name + "/output.txt");
	
	bench::Benchmark<bench::QKParser, bench::ParallelBatchImp> parBench{};
	long par_duration = parBench.run(
		"./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
		"./data/timestamps/" + folder_name + "/combined_put_stamps.txt",
		"./data/timestamps/" + folder_name + "/output.txt");



	bench::Benchmark<bench::QKParser, bench::GeijerBatchPopImp> geijerBatchPopBench{};
	long geijerBatchPop_duration = geijerBatchPopBench.run(
		"./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
		"./data/timestamps/" + folder_name + "/combined_put_stamps.txt",
		"./data/timestamps/" + folder_name + "/output.txt");

	bench::Benchmark<bench::QKParser, bench::HeuristicGeijer> heuristicGeijer{};
	heuristicGeijer.executor->setHeuristicSizeAndCutoff(10000, 2000);
	heuristicGeijer.executor->setBatchSize(10000);
	long heu_duration = heuristicGeijer.run(
		"./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
		"./data/timestamps/" + folder_name + "/combined_put_stamps.txt",
		"./data/timestamps/" + folder_name + "/output.txt");
	
	printf("\n\n----------------------------------\n");
	printf("Geijer: %ld\n", geijer_duration);
	printf("SweepingLine: %ld speedup: %.2f\n", sweeping_duration,
		   (double)geijer_duration / sweeping_duration);
	printf("ParallelBatch: %ld speedup: %.2f\n", par_duration,
		   (double)geijer_duration / par_duration);
	printf("GeijerBatchPop: %ld speedup: %.2f\n", geijerBatchPop_duration,
		   (double)geijer_duration / geijerBatchPop_duration);
	printf("HeuristicGeijer: %ld speedup: %.2f\n", heu_duration
		   , (double)geijer_duration / heu_duration);
	printf("----------------------------------\n\n");

	// bench::Benchmark<bench::QKParser, bench::HeuristicGeijer>
	// heuristicGeijer{};
	// heuristicGeijer.executor->setHeuristicSizeAndCutoff(10000, 2000);
	// heuristicGeijer.executor->setBatchSize(10000);
	// long heu_duration = heuristicGeijer.run(
	// 	"./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
	// 	"./data/timestamps/" + folder_name + "/combined_put_stamps.txt",
	// 	"./data/timestamps/" + folder_name + "/output.txt");

	// bench::Benchmark<bench::QKParser, bench::AITImp> AITImp{};
	// long ait_duration = AITImp.run(
	// 	"./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
	// 	"./data/timestamps/" + folder_name + "/combined_put_stamps.txt",
	// 	"./data/timestamps/" + folder_name + "/output.txt");


	// bench::Benchmark<bench::QKParser, bench::ReplayImp> replayImp{};
	// replayImp.run(projDir + "/data/timestamps/combined_get_stamps.txt",
	// 			  projDir + "/data/timestamps/combined_put_stamps.txt",
	// 			  projDir + "/data/timestamps/output.txt");
	// bench::Benchmark<bench::QKParser, bench::BatchPopImp> batchImp{};
	// batchImp.run(projDir + "/data/timestamps/combined_get_stamps.txt",
	// 			  projDir + "/data/timestamps/combined_put_stamps.txt",
	// 			  projDir + "/data/timestamps/output.txt");
	return 0;
}