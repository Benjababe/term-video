#ifndef AUDIO_H
#define AUDIO_H

#include <iostream>

#include "options.hpp"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
}

namespace Vid2ASCII
{
    namespace AudioPlayer
    {
        std::string play_file(std::string);
    }
}

#endif