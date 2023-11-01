#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <conio.h>
#include <iostream>

#include "audio_player.hpp"
#include "video_player.hpp"

#if defined(_WIN32)
#include <windows.h>
#endif

namespace TermVideo
{
    void listen_seek_keys(AudioPlayer *, VideoPlayer *);
}

#endif