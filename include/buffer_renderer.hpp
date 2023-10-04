#ifndef BUFFER_RENDERER_H
#define BUFFER_RENDERER_H

#include <algorithm>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include <optimiser.hpp>
#include <renderer.hpp>
#include <terminal.hpp>
#include <performance_checker.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#elif defined(__linux__)
#include <sys/ioctl.h>
#include <ncurses.h>
#endif

typedef unsigned long ULONG;
typedef unsigned char uchar;
typedef unsigned short WORD;

namespace Vid2ASCII
{
    class BufferRenderer : public Renderer
    {
    public:
        BufferRenderer(int, bool, bool, bool, std::string, std::string);
        void init_renderer();
        void start_renderer();

    private:
        void frame_to_ascii(uchar *, const int, const int, const int);
        void video_to_ascii(cv::VideoCapture);
        void write_to_buffer(const int, const int, uchar, WORD);

#if defined(_WIN32)
        CHAR_INFO *buffer;
        HANDLE writeHandle;
        COORD buffer_size;
        SMALL_RECT console_write_area;
#elif defined(__linux__)
        void set_curses_colors();

        // how many steps does each colour take in init_color
        short color_step_no;
#endif
    };
}

#endif