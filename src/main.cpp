#include "AITImp.h"
#include "BatchPopImp.h"
#include "Benchmark.h"
#include "GeijerBatch.h"
#include "GeijerBatchPopImp.h"
#include "GeijerImp.h"
#include "QKParser.h"
#include "ReplayImp.h"
#include "IVTImp.h"
#include "GeijerBatch.h"

#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>

int main(int argc, char *argv[]) {

	bench::Benchmark<bench::QKParser, bench::GeijerImp> geiTest{};
	geiTest.run("./data/timestamps/queue-k-seg-1ms-6t/combined_get_stamps.txt",
				"./data/timestamps/queue-k-seg-1ms-6t/combined_put_stamps.txt",
				"./data/timestamps/queue-k-seg-1ms-6t/output.txt");

	bench::Benchmark<bench::QKParser, bench::GeijerBatch> errTest{};
	errTest.run("./data/timestamps/queue-k-seg-1ms-6t/combined_get_stamps.txt",
				"./data/timestamps/queue-k-seg-1ms-6t/combined_put_stamps.txt",
				"./data/timestamps/queue-k-seg-1ms-6t/output.txt");
	return 0;

	// std::string folder_name = "queue-k-seg-1s-4t";
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

	// bench::Benchmark<bench::QKParser, bench::GeijerBatchPopImp> geijerBatchPopImp{};
	// long geijer_batch_duration = geijerBatchPopImp.run("./data/timestamps/" + folder_name + "/combined_get_stamps.txt",
	// 			  "./data/timestamps/" + folder_name + "/combined_put_stamps.txt",
	// 			  "./data/timestamps/" + folder_name + "/output.txt");




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