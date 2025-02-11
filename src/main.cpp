#include "AITImp.h"
#include "BatchPopImp.h"
#include "Benchmark.h"
#include "GeijerBatchPopImp.h"
#include "GeijerImp.h"
#include "QKParser.h"
#include "ReplayImp.h"

#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>

int main(int argc, char *argv[]) {

	// std::shared_ptr<bench::Interval> i1 = std::make_shared<bench::Interval>(1, 4);
	// std::shared_ptr<bench::Interval> i2 = std::make_shared<bench::Interval>(2, 6);
	// std::shared_ptr<bench::Interval> i3 = std::make_shared<bench::Interval>(3, 5);
	// 	bench::AugmentedIntervalTree ait;
	// ait.root = ait.insertNode(ait.root, i1);
	// ait.root = ait.insertNode(ait.root, i2);
	// ait.root = ait.insertNode(ait.root, i3);


	// uint64_t r1 = ait.getRank(ait.root, i1, 0);
	// uint64_t r2 = ait.getRank(ait.root, i2, 0);
	// uint64_t r3 = ait.getRank(ait.root, i3, 0);
	// printf("r1: %lu r2: %lu r3: %lu", r1, r2, r3);

	std::string folder_name = "2dd-queue-opt-1ms";
	// std::string folder_name = "SHORT_REAL";

	bench::Benchmark<bench::QKParser, bench::GeijerImp> geijerBench{};
	long geijer_duration = geijerBench.run( "./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
				  "./data/timestamps/" + folder_name + "/combined_put_stamps.txt",
				  "./data/timestamps/" + folder_name + "/output.txt");

	bench::Benchmark<bench::QKParser, bench::AITImp> AITImp{};
	long ait_duration = AITImp.run("./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
				  "./data/timestamps/" + folder_name + "/combined_put_stamps.txt",
				  "./data/timestamps/" + folder_name + "/output.txt");

	bench::Benchmark<bench::QKParser, bench::GeijerBatchPopImp> geijerBatchPopImp{};
	long geijer_batch_duration = geijerBatchPopImp.run("./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
				  "./data/timestamps/" + folder_name + "/combined_put_stamps.txt",
				  "./data/timestamps/" + folder_name + "/output.txt");




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