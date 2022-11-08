#ifndef INCL_PERFCHECKER_HEADER
#define INCL_PERFCHECKER_HEADER
#include <performance_checker.hpp>
#endif

Vid2ASCII::PerformanceChecker::PerformanceChecker()
{
    this->frame_count = 0;
    this->frame_time_total = 0;
#if defined(_WIN32)
    this->start_time = std::chrono::high_resolution_clock::now();
#endif
}

void Vid2ASCII::PerformanceChecker::start_frame_time()
{
#if defined(_WIN32)
    this->start_time = std::chrono::high_resolution_clock::now();
#endif
}

void Vid2ASCII::PerformanceChecker::end_frame_time()
{
#if defined(_WIN32)
    auto end_time = std::chrono::high_resolution_clock::now();
    double frame_time = std::chrono::duration<double, std::milli>(end_time - this->start_time).count();
#elif defined(__linux__)
    double frame_time = 0;
#endif

    this->frame_count++;
    this->frame_time_total += frame_time;
}

void Vid2ASCII::PerformanceChecker::add_wait_time(int64 wait_time)
{
    this->wait_time_total += wait_time;
}

double Vid2ASCII::PerformanceChecker::get_avg_frame_time()
{
    return this->frame_time_total / this->frame_count;
}

int64 Vid2ASCII::PerformanceChecker::get_avg_wait_time()
{
    return this->wait_time_total / this->frame_count;
}