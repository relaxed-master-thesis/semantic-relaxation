#include "bench/Benchmark.h"
#include "bench/impl/AITImp.h"
#include "bench/impl/BatchPopImp.h"
#include "bench/impl/GeijerBatch.h"
#include "bench/impl/GeijerBatchPopImp.h"
#include "bench/impl/GeijerImp.h"
#include "bench/impl/HeuristicGeijer.h"
#include "bench/impl/IVTImp.h"
#include "bench/util/QKParser.h"
#include "bench/impl/ReplayImp.h"

#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>

int main(int argc, char *argv[]) {


	// std::string folder_name = "queue-k-seg-1s-4t";
	std::string folder_name = "2dd-queue-opt-1ms";
	// std::string folder_name = "REAL";

	bench::Benchmark<bench::QKParser, bench::GeijerImp> geijerBench{};
	long geijer_duration = geijerBench.run(
		"./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
		"./data/timestamps/" + folder_name + "/combined_put_stamps.txt",
		"./data/timestamps/" + folder_name + "/output.txt");
	// bench::Benchmark<bench::QKParser, bench::GeijerBatchPopImp>
	// geijerBatchPopImp{}; long geijer_batch_duration =
	// geijerBatchPopImp.run("./data/timestamps/" + folder_name +
	// "/combined_get_stamps.txt",
	// 			  "./data/timestamps/" + folder_name +
	// "/combined_put_stamps.txt",
	// 			  "./data/timestamps/" + folder_name + "/output.txt");


	bench::Benchmark<bench::QKParser, bench::HeuristicGeijer> heuristicGeijer{};
	heuristicGeijer.executor->setHeuristicSizeAndCutoff(10000, 2000);
	heuristicGeijer.executor->setBatchSize(10000);
	long heu_duration = heuristicGeijer.run(
		"./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
		"./data/timestamps/" + folder_name + "/combined_put_stamps.txt",
		"./data/timestamps/" + folder_name + "/output.txt");


	// bench::Benchmark<bench::QKParser, bench::AITImp> AITImp{};
	// long ait_duration = AITImp.run(
	// 	"./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
	// 	"./data/timestamps/" + folder_name + "/combined_put_stamps.txt",
	// 	"./data/timestamps/" + folder_name + "/output.txt");

	// double speedup = (double)geijer_duration / (double)geijer_batch_duration;
	// printf("\n\nSpeedup using batch in Geijer: %f\n", speedup);

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