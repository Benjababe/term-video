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
#include "media_player.hpp"
#include "optimiser.hpp"
#include "options.hpp"
#include "renderer.hpp"
#include "terminal.hpp"

void play_media(TermVideo::MediaPlayer *media_player)
{
    media_player->play_file();
}

void listen_keys(TermVideo::MediaPlayer *media_player)
{
    TermVideo::listen_seek_keys(media_player);
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

    TermVideo::MediaPlayer media_player;
    res = media_player.init_player(opts);
    if (res.length() > 0)
    {
        std::cerr << res << std::endl;
        std::cin.ignore();
        return 0;
    }

    std::thread thread_media(play_media, &media_player);
    std::thread thread_keyboard(listen_keys, &media_player);

    thread_media.join();
    thread_keyboard.join();

    std::cin.ignore();
    return 0;
}