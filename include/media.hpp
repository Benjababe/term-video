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
        int64_t time_pt_ms;
        std::string file_path;

        const AVCodec *decoder;
        AVStream *stream;
        AVFormatContext *format_ctx;
        AVCodecContext *codec_ctx;
    };

    class MediaLoader
    {
    public:
        MediaLoader(Options);

        MediaInfo media_info;
        std::string open_file(std::string);
    };
}

#endif