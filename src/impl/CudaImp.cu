#include <memory>
#include <vector>
#include <fstream>
#include <iostream>
#include <unordered_map>

bool InitCUDA() {
    int count;
    cudaGetDeviceCount(&count);
    if (count == 0) {
        fprintf(stderr, "There is no device.\n");
        return false;
    }
 
    int i;
    for (i = 0; i < count; i++) {
        cudaDeviceProp prop;
        cudaGetDeviceProperties(&prop, i);
        if (cudaGetDeviceProperties(&prop, i) == cudaSuccess) {
            if (prop.major >= 1)  {
                break;
            }
        }
    }
 
    if (i == count) {
        fprintf(stderr, "There is no device supporting CUDA 1.x.\n");
        return false;
    }
    cudaSetDevice(i);
    return true;
}

struct operation {  
    uint64_t time{0};
    uint64_t value{0};
}

struct interval {
    uint64_t start{0};
    uint64_t end{0};
}

__global__ static void kernel(uint64_t *cuda_intervals, uint64_t num_intervals, uint64_t *errors) {
    unsigned int my_block = gridDim.x * 16 + gridDim.y; // 15 * 16 + 15 = 255
    unsigned int my_idx = blockDim.x * 8 + blockDim.y; // 7 * 8 + 7 = 63
    unsigned int my_intv_idx = my_block * 64 + my_idx; // 255 * 64 + 63

    uint64_t start = cuda_intervals[my_intv_idx];
    uint64_t end = cuda_intervals[my_intv_idx+1];

    uint64_t num_errs = 0;
    if (my_intv_idx < num_intervals) {
        for (uint64_t i = 0; i < my_intv_idx; ++i) {
            if (cuda_intervals[i] < start && end < cuda_intervals[i+1]) {
                num_errs += 1;
            }
        }
    }

    errors[my_intv_idx] = num_errs;
}

std::pair<uint64_t, long double> execute(uint64_t *intervals, size_t num_intervals, uint64_t num_pops) {
    uint64_t *cuda_intervals;
    uint64_t *cuda_errors;
    cudaMalloc(&cuda_intervals, sizeof(uint64_t) * 2 * num_intervals);
    cudaMemcpy(cuda_intervals, intervals, sizeof(uint64_t) * 2 * num_intervals, cudaMemcpyHostToDevice);
    cudaMalloc(&cuda_errors, sizeof(uint64_t) * num_intervals);

    dim3 dimBlock(8,8,1); // 64 threads per block
    dim3 dimGrid(16,16,1); // 16384 / 64 tpb = 256 blocks

    kernel<<<dimGrid, dimBlock>>>(...);

    cudaDeviceSynchronize();

    uint64_t *errors = (uint64_t*)malloc(sizeof(uint64_t) * num_intervals);
    cudaMemcpy(errors, cuda_errors, sizeof(uint64_t) * num_intervals, cudaMemcpyDeviceToHost);

    // find max and calc mean
    uint64_t max = 0, sum = 0;
    for (size_t i = 0; i < num_intervals; ++i) {
        uint64_t err = errors[i];
        sum += err;
        if (err > max) { max = err; }
    }

    long double mean = (long double)sum / num_pops;
    return {max, mean};
}

int main(int argc, char **argv) {
    if (!argc != 3) {
        std::cerr << "Bad usage\n";
    }

    if (!InitCUDA()) {
        std::cerr << "failed to init cuda\n";
    }

    std::ifstream sgets(argv[1]);
    std::ifstream sputs(argv[2]);
   
    auto pgets = std::make_shared<std::vector<operation>>();
    auto pputs = std::make_shared<std::vector<operation>>();
    uint64_t time, value;
    
    while (sgets >> time >> value) {
        pgets->emplace_back(time, value);
    }
    while (sputs >> time >> value) {
        pputs->emplace_back(time, value);
    }

    uint64_t *pints = (uint64_t*)malloc(sizeof(uint64_t) * pputs->size() * 2u);

    // lets run this shit
    std::unordered_map<uint64_t, uint64_t> putMap{};
    for (size_t i = 0; i < pputs->size(); ++i) {
        uint64_t *interv = &(pints[i*2]);
        auto &put = pputs->at(i);
        interv[0] = put.time;
        interv[1] = ~0;
        putMap[put.value] = i * 2;
    }
    for (size_t i = 0; i < pgets->size(); ++i) {
        auto &get = pgets->at(i);
        auto &interv_end = &(pints[putMap[get.value] + 1]);
        *interv_end = get.time;
    }

    auto [max, mean] = execute(pints, pputs->size(), pgets->size());
    std::cout << "Max: " << max << "\tMean: " << mean "\n";
}