#ifndef VIDEO_H
#define VIDEO_H

#include <iostream>

#include "buffer_renderer.hpp"
#include "media.hpp"
#include "options.hpp"
#include "renderer.hpp"
#include "terminal.hpp"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
}

namespace TermVideo
{
    class VideoPlayer
    {
    private:
        Renderer *renderer;

    public:
        std::string init_player(Options);
        void play_file();
        void seek(bool);
    };
}

#endif