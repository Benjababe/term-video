#ifndef INCL_STD_HEADERS
#define INCL_STD_HEADERS
#include <algorithm>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#endif

#if defined(_WIN32)
typedef long long int64;
#elif defined(__linux__)
typedef int64_t int64;
#endif

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
    std::chrono::steady_clock::time_point start_time;
};