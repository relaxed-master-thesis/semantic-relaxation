#include "bench/impl/OrderApproxImp.hpp"
#include "bench/util/Operation.hpp"
#include "bench/util/Executor.hpp"
#include <cstddef>
#include <cstdint>
#include <unordered_map>
namespace bench {
    AbstractExecutor::Measurement OrderApproxImp::calcMaxMeanError() {
        uint64_t rank_sum = 0, rank_max = 0;
        for(auto &point : points){
            int r = (int)point.deq_idx - (int)point.enq_idx;
            uint64_t rank = r < 0 ? 0 : r; 
            rank_sum += rank;
            if(rank > rank_max){
                rank_max = rank;
            }
        }
        return {rank_max, (double)rank_sum / points.size()};
    }
    void OrderApproxImp::prepare(const InputData &data) {
        put_stamps = data.getPuts();
        get_stamps = data.getGets();
        std::unordered_map<uint64_t, uint64_t> get_map{};
        for(size_t i = 0; i < get_stamps->size(); ++i){
            const Operation &get = get_stamps->at(i);
            get_map[get.value] = i;
        }
        for(size_t i = 0; i < put_stamps->size(); ++i){
            const Operation &put = put_stamps->at(i);
            uint64_t get_idx = get_map.find(put.value) == get_map.end() ? ~0 : get_map[put.value];
            points.emplace_back(i, get_idx);
        }
    }
    void OrderApproxImp::reset() {
        points.clear();
    }
    AbstractExecutor::Measurement OrderApproxImp::execute() {
       return calcMaxMeanError();
    }

}