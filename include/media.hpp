#define __USE_FFMPEG

#ifndef MEDIA_H
#define MEDIA_H

#include "options.hpp"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

namespace TermVideo
{
    struct MediaInfo
    {
        bool locked;
        int seek_step_ms;
        double clock_ms;
        std::string file_path;

        const AVCodec *decoder;
        AVStream *stream;
        AVFormatContext *format_ctx;
        AVCodecContext *codec_ctx;
    };
}

#endif