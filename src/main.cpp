#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <buffer_renderer.hpp>
#include <export.hpp>
#include <optimiser.hpp>
#include <options.hpp>
#include <renderer.hpp>
#include <terminal.hpp>

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