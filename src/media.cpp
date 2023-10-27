#include "media.hpp"

namespace Vid2ASCII
{
    MediaLoader::MediaLoader(Options opts)
    {
        this->mediaInfo.file_path = opts.filename;
        this->mediaInfo.format_ctx = avformat_alloc_context();
    }

    std::string MediaLoader::open_file(std::string file_path)
    {
        if (file_path.length() > 0)
            this->mediaInfo.file_path = file_path;

        int ret = avformat_open_input(
            &mediaInfo.format_ctx,
            this->mediaInfo.file_path.c_str(),
            nullptr,
            nullptr);
        if (ret < 0)
            return "Unable to open media file!";

        ret = avformat_find_stream_info(mediaInfo.format_ctx, nullptr);
        if (ret < 0)
            return "Unable to find stream info!";

        return "";
    }
}