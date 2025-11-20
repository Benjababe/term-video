#include "performance_checker.hpp"

TermVideo::PerformanceChecker::PerformanceChecker()
{
    this->frame_count = 0;
    this->last_frame_time = 0;
    this->frame_time_total = 0;
}

void TermVideo::PerformanceChecker::start_frame_time()
{
    this->start_time = std::chrono::high_resolution_clock::now();
}

void TermVideo::PerformanceChecker::end_frame_time()
{
    auto end_time = std::chrono::high_resolution_clock::now();
    auto frame_time_micro = std::chrono::duration_cast<std::chrono::microseconds>(end_time - this->start_time);
    auto frame_time_milli = static_cast<double>(frame_time_micro.count()) / 1000.0;

    this->frame_count++;
    this->last_frame_time = frame_time_milli;
    this->frame_time_total += frame_time_milli;
}

void TermVideo::PerformanceChecker::add_wait_time(int64 wait_time)
{
    this->wait_time_total += wait_time;
}

double TermVideo::PerformanceChecker::get_avg_frame_time()
{
    return this->frame_time_total / this->frame_count;
}

int64 TermVideo::PerformanceChecker::get_avg_wait_time()
{
    return this->wait_time_total / this->frame_count;
}