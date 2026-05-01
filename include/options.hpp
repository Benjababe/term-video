#ifndef OPTIONS_H
#define OPTIONS_H

#include <iostream>
#include <string>

namespace TermVideo
{
    struct Options
    {
        Options();

        std::string filename;
        std::string char_set;
        std::string audio_language;
        unsigned char col_threshold;
        int frames_to_skip;
        int seek_step_ms;
        bool print_colour;
        bool force_aspect;
        bool use_buffer;
        bool force_avg_lumi;
        bool use_audio;
        bool display_frametime;
        bool use_ascii;
        bool disable_frame_sync;
    };

    int parse_arguments(Options &, int, char **);
    int return_arg_missing_value(std::string);
}

#endif