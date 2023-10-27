#ifndef MEDIA_H
#define MEDIA_H

#include "options.hpp"

extern "C"
{
#include <libavformat/avformat.h>
}

namespace Vid2ASCII
{
    struct MediaInfo
    {
        std::string file_path;
        AVFormatContext *format_ctx;
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