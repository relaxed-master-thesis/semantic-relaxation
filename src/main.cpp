#include "Benchmark.h"
#include "QKParser.h"
#include "GeijerImp.h"
#include "ReplayImp.h"
#include "UnsafeVector.h"
#include "Queue.h"
#include <string>

int main(int argc, char * argv[]) {

    bench::Queue q;
    uint64_t a = 1;
    uint64_t b = 2;
    q.enq(a);
    q.enq(b);
    q.deq(a, nullptr);

return 0;

    bench::Benchmark<bench::QKParser, bench::GeijerImp> geijerBench{}; 

    std::string projDir = "/mnt/c/Chalmers/MPALG2/MasterThesis/semantic-relaxation";
    // std::string projDir = "/home/virre/thesis/semantic-relaxation";

    geijerBench.run(
        projDir + "/data/timestamps/combined_get_stamps.txt", 
        projDir + "/data/timestamps/combined_put_stamps.txt",
        projDir + "/data/timestamps/output.txt");  

    bench::Benchmark<bench::QKParser, bench::ReplayImp> replayImp{};
    replayImp.run(
         projDir + "/data/timestamps/combined_get_stamps.txt", 
        projDir + "/data/timestamps/combined_put_stamps.txt",
        projDir + "/data/timestamps/output.txt");  

   return 0;
}