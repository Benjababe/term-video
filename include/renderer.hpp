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

#ifndef INCL_OPTIMISER_HEADER
#define INCL_OPTIMISER_HEADER
#include <optimiser.hpp>
#endif

#ifndef INCL_RENDERER_HEADER
#define INCL_RENDERER_HEADER
#include <renderer.hpp>
#endif

#ifndef INCL_COLOUR_HEADER
#define INCL_COLOUR_HEADER
#include <colour.hpp>
#endif

#ifndef INCL_TERMINAL_HEADER
#define INCL_TERMINAL_HEADER
#include <terminal.hpp>
#endif

#ifndef INCL_PERFCHECKER_HEADER
#define INCL_PERFCHECKER_HEADER
#include <performance_checker.hpp>
#endif

#ifndef INCL_OPENCV_HEADER
#define INCL_OPENCV_HEADER
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>
#endif

typedef unsigned long ULONG;
typedef unsigned char uchar;

namespace Vid2ASCII
{
    class Renderer
    {
    public:
        Renderer();
        Renderer(int, bool, bool, uchar, std::string, std::string);
        void init_renderer();
        void start_renderer();

        Optimiser optimiser;
        PerformanceChecker perfChecker;

    protected:
        char pixel_to_ascii(uchar, uchar, uchar);
        void frame_downscale(cv::Mat &);
        void wait_for_frame(int64);

        int frames_to_skip;
        int width, height;
        int padding_x, padding_y;
        bool print_colour;
        bool force_aspect;
        uchar col_threshold;
        uchar prev_r, prev_g, prev_b;
        std::string filename, char_set;
        std::chrono::steady_clock::time_point next_frame;

    private:
        void frame_to_ascii(std::string &, uchar *, const int, const int, const int);
        void video_to_ascii(cv::VideoCapture);
    };
}