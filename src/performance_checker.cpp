#include "performance_checker.hpp"

Vid2ASCII::PerformanceChecker::PerformanceChecker()
{
    this->frame_count = 0;
    this->frame_time_total = 0;
    // this->start_time = std::chrono::high_resolution_clock::now();
}

void Vid2ASCII::PerformanceChecker::start_frame_time()
{
    this->start_time = std::chrono::high_resolution_clock::now();
}

void Vid2ASCII::PerformanceChecker::end_frame_time()
{
    auto end_time = std::chrono::high_resolution_clock::now();
    auto frame_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - this->start_time);

    this->frame_count++;
    this->frame_time_total += frame_time.count();
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