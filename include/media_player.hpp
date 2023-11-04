#ifndef MEDIA_PLAYER_H
#define MEDIA_PLAYER_H

#include <algorithm>

#include "audio_player.hpp"
#include "buffer_renderer.hpp"
#include "media.hpp"
#include "renderer.hpp"

namespace TermVideo
{
    class MediaPlayer
    {
    private:
        int seek_step_ms;
        MediaInfo *info;
        AudioPlayer *audio_player;
        Renderer *renderer;

        void time_check();

    public:
        MediaPlayer();
        std::string init_player(Options);
        void play_file();
        void seek(bool);
    };
}

#endif