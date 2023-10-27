#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "audio_player.hpp"
#include "buffer_renderer.hpp"
#include "export.hpp"
#include "media.hpp"
#include "optimiser.hpp"
#include "options.hpp"
#include "renderer.hpp"
#include "terminal.hpp"
#include "video_player.hpp"

void play_audio(TermVideo::AudioPlayer audio_player)
{
    audio_player.play_file();
}

void play_video(TermVideo::VideoPlayer video_player)
{
    video_player.play_file();
}

int main(int argc, char **argv)
{
    TermVideo::Options opts;
    int err_code = TermVideo::parse_arguments(opts, argc, argv);

    if (err_code < 0)
    {
        std::cout << "Error with parsing arguments" << std::endl;
        return 0;
    }

    TermVideo::MediaLoader media_loader(opts);
    std::string res = media_loader.open_file(opts.filename);
    if (res.length() > 0)
    {
        std::cout << res << std::endl;
        return 0;
    }

    TermVideo::AudioPlayer audio_player(media_loader.media_info);
    res = audio_player.decode_file(opts);
    if (res.length() > 0)
    {
        std::cout << res << std::endl;
        return 0;
    }

    TermVideo::VideoPlayer video_player(media_loader.media_info, opts);

    std::thread thread_audio(play_audio, audio_player);
    std::thread thread_video(play_video, video_player);

    thread_audio.join();
    thread_video.join();

    return 0;
}