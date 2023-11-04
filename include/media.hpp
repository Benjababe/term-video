#define __USE_FFMPEG

#ifndef MEDIA_H
#define MEDIA_H

#include "options.hpp"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

namespace TermVideo
{
    struct MediaInfo
    {
        bool locked;
        int seek_step_ms;
        double clock_ms;
        std::string file_path;
        AVFormatContext *format_ctx;

        const AVCodec *v_decoder;
        AVStream *v_stream;
        AVCodecContext *v_codec_ctx;
        SwsContext *v_sws_ctx;

        const AVCodec *a_decoder;
        AVStream *a_stream;
        AVCodecContext *a_codec_ctx;
        SwrContext *a_swr_ctx;
    };
}

#endif