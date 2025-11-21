#ifndef OPTIONS_H
#define OPTIONS_H

#include <iostream>
#include <string>

namespace TermVideo
{
    struct Options
    {
        std::string filename = "";
        std::string char_set = "";
        std::string audio_language = "";
        unsigned char col_threshold = 0;
        int frames_to_skip = 0;
        int seek_step_ms = 5000;
        bool print_colour = false;
        bool force_aspect = false;
        bool use_buffer = false;
        bool force_avg_lumi = false;
        bool use_audio = true;
        bool display_frametime = false;
        bool use_ascii = false;
    };

    int parse_arguments(Options &, int, char **);
    int return_arg_missing_value(std::string);
}

#endif