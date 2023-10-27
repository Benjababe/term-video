#include "media.hpp"

namespace Vid2ASCII::Media
{
    MediaInfo mediaInfo;

    std::string open_file(Options opts)
    {
        int ret = avformat_open_input(&mediaInfo.format_ctx, opts.filename.c_str(), nullptr, nullptr);
        if (ret < 0)
            return "Unable to open media file!";

        ret = avformat_find_stream_info(mediaInfo.format_ctx, nullptr);
        if (ret < 0)
            return "Unable to find stream info!";
    }
}