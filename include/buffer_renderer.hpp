#ifndef BUFFER_RENDERER_H
#define BUFFER_RENDERER_H

#include <algorithm>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include "optimiser.hpp"
#include "options.hpp"
#include "performance_checker.hpp"
#include "renderer.hpp"
#include "terminal.hpp"

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

namespace TermVideo
{
    class BufferRenderer : public Renderer
    {
    public:
        BufferRenderer(MediaInfo *, Options);
        void init_renderer() override;
        void start_renderer() override;

    private:
        void frame_to_ascii(uchar *, const int, const int, const int);
        void write_to_buffer(const int, const int, uchar, WORD);
        void check_resize();

#if defined(__USE_OPENCV)
        void process_video_opencv();
#elif defined(__USE_FFMPEG)
        void process_video_ffmpeg();
#endif

#if defined(_WIN32)
        CHAR_INFO *buffer;
        HANDLE write_handle;
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