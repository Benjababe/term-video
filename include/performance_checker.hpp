#ifndef PERFORMANCE_CHECKER_H
#define PERFORMANCE_CHECKER_H

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#if defined(_WIN32)
typedef long long int64;
#elif defined(__linux__)
typedef int64_t int64;
#endif

namespace Vid2ASCII
{
    class PerformanceChecker
    {
    public:
        PerformanceChecker();
        void start_frame_time();
        void end_frame_time();
        void add_wait_time(int64);
        double get_avg_frame_time();
        int64 get_avg_wait_time();

    private:
        int frame_count;
        double frame_time_total;
        int64 wait_time_total;
        std::chrono::_V2::system_clock::time_point start_time;
    };
}

#endif