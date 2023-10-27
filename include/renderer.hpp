#ifndef RENDERER_H
#define RENDERER_H

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "colour.hpp"
#include "media.hpp"
#include "optimiser.hpp"
#include "options.hpp"
#include "performance_checker.hpp"
#include "terminal.hpp"

#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

typedef unsigned long ULONG;
typedef unsigned char uchar;

namespace Vid2ASCII
{
    struct VideoInfo
    {
        AVFormatContext *format_ctx;
        AVStream *stream;
    };

    class Renderer
    {
    public:
        Renderer();
        Renderer(MediaInfo, Options);
        void init_renderer();
        void start_renderer();

        Optimiser optimiser;
        PerformanceChecker perf_checker;

    protected:
        char pixel_to_ascii(uchar, uchar, uchar);
        void frame_downscale(cv::Mat &);
        void wait_for_frame(int64);

        VideoInfo video_info;
        int frames_to_skip;
        int width, height;
        int padding_x, padding_y;
        bool print_colour;
        bool force_aspect;
        bool ready;
        bool force_avg_luminance;
        uchar col_threshold;
        uchar prev_r, prev_g, prev_b;
        std::string filename, char_set;
        std::chrono::steady_clock::time_point next_frame;

    private:
        void frame_to_ascii(std::string &, uchar *, const int, const int, const int);
        void video_to_ascii(cv::VideoCapture);
    };
}

#endif