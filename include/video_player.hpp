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

namespace Vid2ASCII
{
    struct VideoInfo
    {
        AVFormatContext *format_ctx;
        AVStream *stream;
    };

    class VideoPlayer
    {
    private:
        VideoInfo video_info;
        Renderer *renderer;
        BufferRenderer *buffered_renderer;
        bool use_buffer;

    public:
        VideoPlayer(MediaInfo, Options);
        void play_file();
    };
}

#endif