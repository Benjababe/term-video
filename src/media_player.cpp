#include "media_player.hpp"

namespace TermVideo
{
    void MediaPlayer::time_check()
    {
        while (1)
        {
            std::cout << "Video: " << this->renderer->info.clock_ms
                      << " Audio: " << this->audio_player->info.clock_ms
                      << " Diff: " << this->renderer->info.clock_ms - this->audio_player->info.clock_ms
                      << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }
    }

    /**
     * @brief Sets up audio player, renderer and FFmpeg read stream
     * @param opts
     * @return std::string Error string
     */
    std::string MediaPlayer::init_player(Options opts)
    {
        this->seek_step_ms = opts.seek_step_ms;

        if (opts.use_buffer)
            this->renderer = new BufferRenderer(opts);
        else
            this->renderer = new Renderer(opts);

        this->renderer->init_renderer();

#ifdef __USE_FFMPEG
        std::string res = this->renderer->open_file();
        if (res.length() > 0)
            return res;

        res = this->renderer->get_decoder();
        if (res.length() > 0)
            return res;

        this->audio_player = new AudioPlayer();
        this->audio_player->init_player(opts);
#endif

        return "";
    }

    void MediaPlayer::play_file()
    {
        std::thread video_thread(&Renderer::start_renderer, this->renderer);
        std::thread audio_thread(&AudioPlayer::play_file, this->audio_player);
        // std::thread time_thread(&MediaPlayer::time_check, this);

        video_thread.join();
        audio_thread.join();
        // time_thread.join();
    }

    /**
     * @brief Seek video either forwards or backwards
     * @param seek_back Flag indicating going forwards or backwards
     */
    void MediaPlayer::seek(bool seek_back)
    {
        while (this->renderer->info.locked)
            ;
        this->renderer->info.locked = true;
        while (this->audio_player->info.locked)
            ;
        this->audio_player->info.locked = true;

        int64_t rel_time = this->seek_step_ms;
        int64_t v_time_pt = this->renderer->info.clock_ms;
        int64_t a_time_pt = this->audio_player->info.clock_ms;
        int flags = AVSEEK_FLAG_FRAME;

        if (seek_back)
        {
            rel_time *= -1;
            flags |= AVSEEK_FLAG_BACKWARD;
        }

        this->renderer->seek(v_time_pt + rel_time, flags);
        this->audio_player->seek(a_time_pt + rel_time, flags);

        this->renderer->info.locked = false;
        this->audio_player->info.locked = false;
    }
}