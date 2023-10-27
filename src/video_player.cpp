#include "video_player.hpp"

namespace Vid2ASCII
{
    VideoPlayer::VideoPlayer(MediaInfo media_info, Options opts)
    {
        this->video_info.format_ctx = media_info.format_ctx;
        this->use_buffer = opts.use_buffer;

        if (opts.use_buffer)
        {
            this->buffered_renderer = new BufferRenderer(media_info, opts);
            this->buffered_renderer->init_renderer();
        }
        else
        {
            this->renderer = new Renderer(media_info, opts);
            this->renderer->init_renderer();
        }
    }

    void VideoPlayer::play_file()
    {
        if (this->use_buffer)
            this->buffered_renderer->start_renderer();
        else
            this->renderer->start_renderer();
    }
}