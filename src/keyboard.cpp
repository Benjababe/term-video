#include "keyboard.hpp"

namespace TermVideo
{
    void listen_seek_keys(AudioPlayer *audio_player, VideoPlayer *video_player)
    {
        HWND console = GetForegroundWindow();
        bool ctrl_pressed = false;

        while (1)
        {
            if (console != GetForegroundWindow())
                continue;

            if (GetKeyState(VK_CONTROL) & 0x8000)
                ctrl_pressed = true;
            else
                ctrl_pressed = false;

            if (!ctrl_pressed)
                continue;

            if (GetKeyState(VK_LEFT) & 0x8000)
            {
                video_player->seek(true);
                audio_player->seek(true);
                while (GetKeyState(VK_LEFT) & 0x8000)
                    ;
            }

            if (GetKeyState(VK_RIGHT) & 0x8000)
            {
                video_player->seek(false);
                audio_player->seek(false);
                while (GetKeyState(VK_RIGHT) & 0x8000)
                    ;
            }
        }
    }
} // namespace TermVideo