#ifndef COLOR_H
#define COLOR_H

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <math.h>
#endif

typedef unsigned char uchar;
typedef unsigned short WORD;

namespace TermVideo
{
    uchar get_luminance_approximate(uchar, uchar, uchar, bool);
    WORD get_win32_col(uchar, uchar, uchar);
    int get_ncurses_col_index(uchar, uchar, uchar, short);
    std::string get_char_ansi_col(uchar, uchar, uchar, std::string);
}

#endif