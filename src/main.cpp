#include "Benchmark.h"
#include "QKParser.h"
#include "GeijerImp.h"

int main(int argc, char * argv[]) {
    bench::Benchmark<bench::QKParser, bench::GeijerImp> geijerBench{}; 

    geijerBench.run(
        "/home/virre/thesis/semantic-relaxation/data/timestamps/combined_get_stamps.txt", 
        "/home/virre/thesis/semantic-relaxation/data/timestamps/combined_put_stamps.txt",
        "/home/virre/thesis/semantic-relaxation/data/timestamps/output.txt");  

    return 0;
}