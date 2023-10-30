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

#ifdef __USE_OPENCV
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>
#endif

#ifdef __USE_FFMPEG
extern "C"
{
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/parseutils.h>
#include <libavutil/pixdesc.h>
#include <libavutil/rational.h>
#include <libavformat/avformat.h>
}
#endif

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <ncurses.h>
#endif

typedef unsigned long ULONG;
typedef unsigned char uchar;

#define FETCH_TERMINAL_INTERVAL 1

namespace TermVideo
{
    struct VideoInfo
    {
#ifdef __USE_FFMPEG
        const AVCodec *decoder;
        AVStream *stream;
        AVFormatContext *format_ctx;
        AVCodecContext *codec_ctx;
        SwsContext *sws_ctx;
#endif

        int64 frametime_ns;
        int colour_channels;
        int new_width;
        int new_height;
    };

    class Renderer
    {
    public:
        Renderer();
        Renderer(Options);
        virtual void init_renderer();
        virtual void start_renderer();
        std::string open_file();
        std::string get_decoder();

        Optimiser optimiser;
        PerformanceChecker perf_checker;

    protected:
        char pixel_to_ascii(uchar, uchar, uchar);
        void wait_for_frame();

#if defined(__USE_OPENCV)
        void frame_downscale_opencv(cv::Mat &);
#elif defined(__USE_FFMPEG)
        void frame_downscale_ffmpeg(AVFrame *);
#endif

        VideoInfo video_info;
        int frames_to_skip;
        int width, height;
        int padding_x, padding_y;
        bool print_colour;
        bool force_aspect;
        bool ready;
        bool term_resized;
        bool force_avg_luminance;
        uchar col_threshold;
        uchar prev_r, prev_g, prev_b;
        std::string filename, char_set;
        std::chrono::steady_clock::time_point next_frame;

    private:
        void frame_to_ascii(std::string &, uchar *, const int, const int, const int);
        void print(std::string ascii_frame);

#if defined(__USE_OPENCV)
        void process_video_opencv(cv::VideoCapture);
#elif defined(__USE_FFMPEG)
        void process_video_ffmpeg();
#endif
    };
}

#endif