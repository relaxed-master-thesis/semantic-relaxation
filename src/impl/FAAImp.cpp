#include "bench/impl/FAAImp.h"
#include "bench/Benchmark.h"

#include <cstdint>
#include <unordered_map>

namespace bench {
	void FAAImp::prepare(const InputData &data) {
        this->puts = data.getPuts();
        this->gets = data.getGets();
    }
    
	AbstractExecutor::Measurement FAAImp::execute() {
        uint64_t queue_size = 0;

        uint64_t rank_max = 0;
        long double rank_sum = 0;

        size_t put_idx = 0;
        size_t get_idx = 0;

        while (put_idx < puts->size() && get_idx < gets->size()) {
            const auto &put = puts->at(put_idx);
            const auto &get = gets->at(get_idx);

            if (put.time < get.time) {
                ++queue_size;
                rank_max = rank_max < queue_size ? queue_size : rank_max;
                ++put_idx;
            } else {
                rank_sum += queue_size--;
                get_idx++;
            }
        }

        if (put_idx == puts->size()) {
            rank_sum += (queue_size * (queue_size + 1)) / 2;
        }

        return {rank_max, (long double)(rank_sum * 0.72772) / puts->size()};
    }
    
	AbstractExecutor::Measurement FAAImp::calcMaxMeanError() {
        return {0,0};
    }
    void FAAImp::reset() {
        puts = nullptr;
        gets = nullptr;
    }
    

    void IMEImp::prepare(const InputData &data) {
        std::unordered_map<uint64_t, size_t> put_time_map{};
        auto puts = data.getPuts();
        auto gets = data.getGets();
        put_count = puts->size();
        get_count = gets->size();

        exec_start = puts->begin()->time;
        exec_end = gets->rbegin()->time;

        intervals.resize(puts->size());
        for (size_t i = 0; i < puts->size(); ++i) {
            auto &put = (*puts)[i];
            auto &interval = intervals[i];
            interval.start = put.time;
            interval.end = exec_end + 1;
            put_time_map[put.value] = i;
        }
        for (size_t i = 0; i < gets->size(); ++i) {
            auto &get = (*gets)[i];
            auto &interval = intervals[put_time_map[get.value]];
            interval.end = get.time;
            interval.was_popped = true;
            interval.pop_idx = i;
            intv_avg_size += (interval.end - interval.start);
        }

        intv_avg_size /= gets->size();
    }

	AbstractExecutor::Measurement IMEImp::execute() {
        uint64_t rank_max = 0;
        long double rank_sum = 0;
        
        for (size_t i = 0; i < intervals.size(); ++i) {
            const auto &interval = intervals[i];
            if (!interval.was_popped)
                continue;

            uint64_t i_size = interval.end - interval.start;
            long double rank = (((long double)intv_avg_size / i_size) * i) - interval.pop_idx;
            rank = rank < 0.0f ? 0.0f : rank;
            rank = i < interval.pop_idx ? interval.pop_idx - i : i - interval.pop_idx;

            // damn de va ganska clean
            // vi kör på de andraaaaaaa
            // okokok aa,ye aware aa sre
            rank_max = rank_max < rank ? rank : rank_max;
            rank_sum += rank;
        }

        return {rank_max, rank_sum / get_count};
    }
    
	AbstractExecutor::Measurement IMEImp::calcMaxMeanError() {
        return {0,0};
    }
    void IMEImp::reset() {
        intervals.clear();
        intv_avg_size = 0;
        put_count = 0;
        get_count = 0;
        exec_start = 0;
        exec_end = 0;
    }


}