#include <algorithm>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include <export.hpp>
#include <options.hpp>
#include <optimiser.hpp>
#include <renderer.hpp>
#include <buffer_renderer.hpp>
#include <terminal.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

int main(int argc, char **argv)
{
    Vid2ASCII::Options opts;
    int err_code = Vid2ASCII::parse_arguments(opts, argc, argv);

    if (err_code < 0)
        return 0;

    if (opts.use_buffer)
    {
        Vid2ASCII::BufferRenderer bRenderer(opts);
        bRenderer.init_renderer();
        bRenderer.start_renderer();
    }
    else
    {
        Vid2ASCII::Renderer renderer(opts);
        renderer.init_renderer();
        renderer.start_renderer();
    }

    return 0;
}