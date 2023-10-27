#ifndef MEDIA_H
#define MEDIA_H

#include "options.hpp"

extern "C"
{
#include <libavformat/avformat.h>
}

namespace Vid2ASCII
{
    namespace Media
    {
        struct MediaInfo
        {
            std::string file_path;
            AVFormatContext *format_ctx;
        };

        extern MediaInfo mediaInfo;

        std::string open_file(Options);
    }
}

#endif