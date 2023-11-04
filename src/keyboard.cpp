#include "keyboard.hpp"

namespace TermVideo
{
    void listen_seek_keys(MediaPlayer *media_player)
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
                media_player->seek(true);
                while (GetKeyState(VK_LEFT) & 0x8000)
                    ;
            }

            if (GetKeyState(VK_RIGHT) & 0x8000)
            {
                media_player->seek(false);
                while (GetKeyState(VK_RIGHT) & 0x8000)
                    ;
            }
        }
    }
} // namespace TermVideo