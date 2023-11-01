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
#include "keyboard.hpp"
#include "optimiser.hpp"
#include "options.hpp"
#include "renderer.hpp"
#include "terminal.hpp"
#include "video_player.hpp"

void play_audio(TermVideo::AudioPlayer *audio_player)
{
    audio_player->play_file();
}

void play_video(TermVideo::VideoPlayer *video_player)
{
    video_player->play_file();
}

void listen_keys(TermVideo::AudioPlayer *audio_player, TermVideo::VideoPlayer *video_player)
{
    TermVideo::listen_seek_keys(audio_player, video_player);
}

int main(int argc, char **argv)
{
    std::string res;

    TermVideo::Options opts;
    int err_code = TermVideo::parse_arguments(opts, argc, argv);

    if (err_code < 0)
    {
        std::cerr << "Error with parsing arguments" << std::endl;
        std::cin.ignore();
        return 0;
    }

    TermVideo::AudioPlayer audio_player;
    res = audio_player.init_player(opts);
    if (res.length() > 0)
    {
        std::cerr << res << std::endl;
        std::cin.ignore();
        return 0;
    }

    TermVideo::VideoPlayer video_player;
    res = video_player.init_player(opts);
    if (res.length() > 0)
    {
        std::cerr << res << std::endl;
        std::cin.ignore();
        return 0;
    }

    std::thread thread_audio(play_audio, &audio_player);
    std::thread thread_video(play_video, &video_player);
    std::thread thread_keyboard(listen_keys, &audio_player, &video_player);

    thread_audio.join();
    thread_video.join();
    thread_keyboard.join();

    std::cin.ignore();
    return 0;
}