#include "media.hpp"

namespace TermVideo
{
    MediaLoader::MediaLoader(Options opts)
    {
        this->media_info.file_path = opts.filename;
        this->media_info.format_ctx = avformat_alloc_context();
    }

    std::string MediaLoader::open_file(std::string file_path)
    {
        if (file_path.length() > 0)
            this->media_info.file_path = file_path;

        int ret = avformat_open_input(
            &media_info.format_ctx,
            this->media_info.file_path.c_str(),
            nullptr,
            nullptr);
        if (ret < 0)
            return "Unable to open media file!";

        ret = avformat_find_stream_info(media_info.format_ctx, nullptr);
        if (ret < 0)
            return "Unable to find stream info!";

        return "";
    }
}