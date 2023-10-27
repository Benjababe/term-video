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
        BufferRenderer *buffered_renderer;
        bool use_buffer;

    public:
        VideoPlayer(MediaInfo, Options);
        void play_file();
    };
}

#endif