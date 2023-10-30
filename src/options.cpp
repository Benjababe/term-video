#include "options.hpp"

int TermVideo::parse_arguments(TermVideo::Options &opts, int argc, char **argv)
{
    opts.char_set = "@&%QWNM0gB$#DR8mHXKAUbGOpV4d9h6PkqwSE2]ayjxY5Zoen[ult13If}C{iF|(7J)vTLs?z/*cr!+<>;=^,_:'-.` ";

    std::string ascii_colour_chars = " `.-':_,^=;><+!rc*/z?sLTv)J7(|Fi{C}fI31tlu[neoZ5Yxjya]2ESwqkP6h9d4VpOGbUAKXHm8RD#$Bg0MNWQ%&@";

    // handle arguments
    for (int i = 0; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "-f" || arg == "--file")
        {
            if (i + 1 < argc)
                opts.filename = std::string(argv[++i]);
            else
            {
                std::cerr << "Option \"" << arg << "\" requires one argument";
                return -1;
            }
        }

        else if (arg == "-al" || arg == "--audio-language")
        {
            if (i + 1 < argc)
                opts.audio_language = std::string(argv[++i]);
            else
            {
                std::cerr << "Option \"" << arg << "\" requires one argument";
                return -1;
            }
        }

        else if (arg == "-ct" || arg == "--color-threshold" || arg == "--colour-threshold")
        {
            if (i + 1 < argc)
                opts.col_threshold = std::stoi(argv[++i]);
            else
            {
                std::cerr << "Option \"" << arg << "\" requires one argument";
                return -1;
            }
        }

        else if (arg == "-s" || arg == "--skip-frames")
        {
            if (i + 1 < argc)
                opts.frames_to_skip = std::stoi(argv[++i]);
            else
            {
                std::cerr << "Option \"" << arg << "\" requires one argument";
                return -1;
            }
        }

        else if (arg == "-c" || arg == "color" || arg == "--colour")
        {
            opts.print_colour = true;
            opts.char_set = ascii_colour_chars;
        }

        else if (arg == "-fa" || arg == "--force-aspect")
        {
            opts.force_aspect = true;
        }

        else if (arg == "-b" || arg == "--buffer")
        {
            opts.use_buffer = true;
        }

        else if (arg == "-alumi" || arg == "--avg-lumi")
        {
            opts.force_avg_lumi = true;
        }
    }

    if (opts.filename.length() == 0)
    {
        std::cerr << "No file provided!" << std::endl;
        return -1;
    }

    return 1;
}