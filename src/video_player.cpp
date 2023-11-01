#include "video_player.hpp"

namespace TermVideo
{
    /**
     * @brief Sets up renderer and FFmpeg read stream
     * @param opts
     * @return std::string Error string
     */
    std::string VideoPlayer::init_player(Options opts)
    {
        if (opts.use_buffer)
            this->renderer = new BufferRenderer(opts);
        else
            this->renderer = new Renderer(opts);

        this->renderer->init_renderer();
        this->renderer->video_info.seek_step_ms = opts.seek_step_ms;

#ifdef __USE_FFMPEG
        std::string res = this->renderer->open_file();
        if (res.length() > 0)
            return res;

        res = this->renderer->get_decoder();
        if (res.length() > 0)
            return res;
#endif

        return "";
    }

    void VideoPlayer::play_file()
    {
        this->renderer->start_renderer();
    }

    /**
     * @brief Seek video either forwards or backwards
     * @param seek_back Flag indicating going forwards or backwards
     */
    void VideoPlayer::seek(bool seek_back)
    {
        this->renderer->seek(seek_back);
    }
}