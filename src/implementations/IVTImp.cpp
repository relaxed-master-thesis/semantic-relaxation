#include "IVTImp.h"

#include <chrono>

namespace bench {
    ErrorCalculator::Result IVTImp::calcMaxMeanError() {
        return {0, 0};
    }

    void IVTImp::prepare(InputData data) {
        
    }
    
    long IVTImp::execute() {
        std::cout << "Running IVTImp...\n";
        auto start = std::chrono::high_resolution_clock::now();
        auto result = calcMaxMeanError();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
        std::cout << "Runtime: " << duration.count() << " us\n";
        std::cout << "Mean: " << result.mean << ", Max: " << result.max << "\n";
        return duration.count();
    }
}