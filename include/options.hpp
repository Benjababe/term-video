#ifndef OPTIONS_H
#define OPTIONS_H

#include <iostream>
#include <string>

namespace Vid2ASCII
{
    struct Options
    {
        std::string filename = "";
        std::string char_set = "";
        unsigned char col_threshold = 0;
        int frames_to_skip = 0;
        bool print_colour = false;
        bool force_aspect = false;
        bool use_buffer = false;
        bool force_avg_lumi = false;
    };

    int parse_arguments(Options &, int, char **);
}

#endif