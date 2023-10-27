#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "audio.hpp"
#include "buffer_renderer.hpp"
#include "export.hpp"
#include "media.hpp"
#include "optimiser.hpp"
#include "options.hpp"
#include "renderer.hpp"
#include "terminal.hpp"

void play_audio(Vid2ASCII::Options opts)
{
    Vid2ASCII::Media::Audio::play_file();
}

void play_video(Vid2ASCII::Options opts)
{
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
}

int main(int argc, char **argv)
{
    Vid2ASCII::Options opts;
    int err_code = Vid2ASCII::parse_arguments(opts, argc, argv);

    if (err_code < 0)
    {
        std::cout << "Error with parsing arguments" << std::endl;
        return 0;
    }

    std::string res = Vid2ASCII::Media::Audio::open_file(opts);
    if (res.length() > 0)
    {
        std::cout << res << std::endl;
        return 0;
    }

    std::thread thread_audio(play_audio, opts);
    std::thread thread_video(play_video, opts);

    thread_audio.join();
    thread_video.join();

    return 0;
}