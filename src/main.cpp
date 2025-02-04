#include "BatchPopImp.h"
#include "Benchmark.h"
#include "GeijerImp.h"
#include "QKParser.h"
#include "ReplayImp.h"

#include <string>

int main(int argc, char *argv[]) {
	bench::Benchmark<bench::QKParser, bench::GeijerImp> geijerBench{};

	std::string projDir =
	"/mnt/c/Chalmers/MPALG2/MasterThesis/semantic-relaxation";
	// std::string projDir = "/home/virre/thesis/semantic-relaxation";

	geijerBench.run(projDir + "/data/timestamps/combined_get_stamps.txt",
					projDir + "/data/timestamps/combined_put_stamps.txt",
					projDir + "/data/timestamps/output.txt");

	// bench::Benchmark<bench::QKParser, bench::ReplayImp> replayImp{};
	// replayImp.run(projDir + "/data/timestamps/combined_get_stamps.txt",
	// 			  projDir + "/data/timestamps/combined_put_stamps.txt",
	// 			  projDir + "/data/timestamps/output.txt");
	bench::Benchmark<bench::QKParser, bench::BatchPopImp> batchImp{};
	batchImp.run(projDir + "/data/timestamps/combined_get_stamps.txt",
				  projDir + "/data/timestamps/combined_put_stamps.txt",
				  projDir + "/data/timestamps/output.txt");



	return 0;
}