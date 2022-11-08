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

#ifndef INCL_EXPORT_HEADER
#define INCL_EXPORT_HEADER
#include <export.hpp>
#endif

#ifndef INCL_OPTIMISER_HEADER
#define INCL_OPTIMISER_HEADER
#include <optimiser.hpp>
#endif

#ifndef INCL_RENDERER_HEADER
#define INCL_RENDERER_HEADER
#include <renderer.hpp>
#endif

#ifndef INCL_B_RENDERER_HEADER
#define INCL_B_RENDERER_HEADER
#include <buffer_renderer.hpp>
#endif

#ifndef INCL_TERMINAL_HEADER
#define INCL_TERMINAL_HEADER
#include <terminal.hpp>
#endif

#ifndef INCL_OPENCV_HEADER
#define INCL_OPENCV_HEADER
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>
#endif

int frames_to_skip = 0;
unsigned char col_threshold = 0;
bool print_colour = false, force_aspect = false, use_buffer = false;
std::string filename, char_set, ascii_grayscale_chars = "@Saeosx=+:.` ", ascii_colour_chars = " `.*+xse@";

int main(int argc, char **argv)
{
    // handle arguments
    for (int i = 0; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "-f" || arg == "--file")
        {
            if (i + 1 < argc)
            {
                filename = std::string(argv[++i]);
            }
            else
            {
                std::cerr << "Option \"" << arg << "\" requires one argument";
                return 1;
            }
        }

        else if (arg == "-t" || arg == "--threshold")
        {
            if (i + 1 < argc)
                col_threshold = std::stoi(argv[++i]);
            else
            {
                std::cerr << "Option \"" << arg << "\" requires one argument";
                return 1;
            }
        }

        else if (arg == "-s" || arg == "--skip-frames")
        {
            if (i + 1 < argc)
                frames_to_skip = std::stoi(argv[++i]);
            else
            {
                std::cerr << "Option \"" << arg << "\" requires one argument";
                return 1;
            }
        }

        else if (arg == "-c" || arg == "--colour")
        {
            print_colour = true;
        }

        else if (arg == "-a" || arg == "--force-aspect")
        {
            force_aspect = true;
        }

        else if (arg == "-b" || arg == "--buffer")
        {
            use_buffer = true;
        }
    }

    if (use_buffer)
    {
        Vid2ASCII::BufferRenderer bRenderer(
            frames_to_skip,
            print_colour,
            force_aspect,
            filename,
            (print_colour) ? ascii_colour_chars : ascii_grayscale_chars);
        bRenderer.init_renderer();
        bRenderer.start_renderer();
    }
    else
    {
        Vid2ASCII::Renderer renderer(
            frames_to_skip,
            print_colour,
            force_aspect,
            col_threshold,
            filename,
            (print_colour) ? ascii_colour_chars : ascii_grayscale_chars);
        renderer.init_renderer();
        renderer.start_renderer();
    }

    return 0;
}