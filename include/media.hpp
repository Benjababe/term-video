#define __USE_FFMPEG

#ifndef MEDIA_H
#define MEDIA_H

#include "options.hpp"

#if defined(__USE_FFMPEG)
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}
#endif

namespace TermVideo
{
    struct MediaInfo
    {
        bool locked;
        int seek_step_ms;
        int64_t time_pt_ms;
        std::string file_path;

#if defined(__USE_FFMPEG)
        const AVCodec *decoder;
        AVStream *stream;
        AVFormatContext *format_ctx;
        AVCodecContext *codec_ctx;
#endif
    };
}

#endif