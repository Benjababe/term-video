#include "video_player.hpp"

namespace TermVideo
{
    std::string VideoPlayer::init_player(Options opts)
    {
        if (opts.use_buffer)
            this->renderer = new BufferRenderer(opts);
        else
            this->renderer = new Renderer(opts);

        this->renderer->init_renderer();

        std::string res = this->renderer->open_file();
        if (res.length() > 0)
            return res;

        res = this->renderer->get_decoder();
        if (res.length() > 0)
            return res;

        return "";
    }

    void VideoPlayer::play_file()
    {
        this->renderer->start_renderer();
    }
}