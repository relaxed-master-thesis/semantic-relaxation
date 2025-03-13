#include "bench/Benchmark.h"
#include "bench/impl/AITImp.h"
#include "bench/impl/BatchPopImp.h"
#include "bench/impl/FenwickImp.h"
#include "bench/impl/GeijerBatchImp.h"
#include "bench/impl/GeijerImp.h"
#include "bench/impl/HeuristicGeijer.h"
#include "bench/impl/IVTImp.h"
#include "bench/impl/ParallelBatchImp.h"
#include "bench/impl/ParallelGeijerImp.h"
#include "bench/impl/ReplayImp.h"
#include "bench/impl/SweepingLineImp.h"
#include "bench/util/QKParser.h"

#include <cstdint>
#include <cstdio>
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
	std::string folder_name = "generated";
	createAndSaveData(1000, "./data/timestamps/" + folder_name + "/combined");

	bench::InputData data = bench::TimestampParser().parse(
		"./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
		"./data/timestamps/" + folder_name + "/combined_put_stamps.txt");

	bench::Benchmark<bench::GeijerImp> geijerBench{};
	auto geijer_res = geijerBench.run(data);

	bench::Benchmark<bench::SweepingLineImp> sweepingImp{};
	auto sweeping_res = sweepingImp.run(data);

	return 0;
}

void prettyPrint(std::string folderName, std::vector<std::string> names,
				 std::vector<bench::Result> results) {
	printf("\n----------------- Results for %s -------------\n",
		   folderName.c_str());
	for (size_t i = 0; i < names.size(); i++) {
		printf("%-30s Mean: %.3Lf, Max: %lu \n", names[i].c_str(),
			   results[i].measurement.mean, results[i].measurement.max);
	}
	printf("\n----------------- Times ------------\n");
	for (size_t i = 0; i < names.size(); i++) {
		printf("%-30s %-10ld speedup: %.2f (+prep: %.2f)\n", names[i].c_str(),
			   results[i].executeTime,
			   (double)results[0].executeTime / results[i].executeTime,
			   (double)(results[0].executeTime + results[0].prepareTime) /
				   (results[i].executeTime + results[i].prepareTime));
	}
	printf("-----------------------------------------------\n\n");
}

static std::vector<std::string> names{};
static std::vector<bench::Result> results{};
static bench::InputData data{};
template <class T>
static void run_bench(bench::Benchmark<T> bencher) {
	names.push_back(bencher.getTemplateParamTypeName());
	auto res = bencher.run(data);
	results.push_back(res);
}

int main(int argc, char *argv[]) {

	// return testWithCreatedData();

	// std::string folder_name = "queue-k-seg-1s-4t";
	// std::string folder_name = "2dd-queue-opt-1ms";
	// std::string folder_name = "generated";
	// std::string folder_name = "FAKE";
	// std::string folder_name = "2dd-queue-opt-30ms";
	// std::string folder_name = "2dd-queue-opt-1s";
	// std::string folder_name = "q-k-1s-8t";
	// std::string folder_name = "q-k-1ms-8t";
	// std::string folder_name = "2dd-queue-opt-1ms-4t-i10k";
	// std::string folder_name = "2dd-queue-opt-100ms";
	std::string folder_name = "2dd-queue-opt-1ms-4t-i10k";
	// std::string folder_name = "2dd-q-opt-w50-l10-i1000-8t-30ms";

	data = bench::TimestampParser().parse(
		"./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
		"./data/timestamps/" + folder_name + "/combined_put_stamps.txt");

	//ALLWAYS RUN GEIJER FIRST, IT IS THE BASELINE
	run_bench(bench::Benchmark<bench::GeijerImp>{});
	// run_bench(bench::Benchmark<bench::AITImp>{});
	// run_bench(bench::Benchmark<bench::GeijerBatchImp>{});
	// run_bench(bench::Benchmark<bench::ParallelGeijerImp>{});
	run_bench(bench::Benchmark<bench::ParallelBatchImp>{16, false});
	// run_bench(bench::Benchmark<bench::ReplayImp>{});
	// run_bench(bench::Benchmark<bench::HeuristicGeijer>{});
	// run_bench(bench::Benchmark<bench::SweepingLineImp>{});
	// run_bench(bench::Benchmark<bench::IVTImp>{});
	// run_bench(bench::Benchmark<bench::BatchPopImp>{});
	// run_bench(bench::Benchmark<bench::FenwickImp>{});




	prettyPrint(folder_name, names, results);
	return 0;
}