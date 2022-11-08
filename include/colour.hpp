#ifndef INCL_STD_HEADERS
#define INCL_STD_HEADERS
#include <algorithm>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#endif

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <math.h>
#endif

typedef unsigned char uchar;
typedef unsigned short WORD;

namespace Vid2ASCII
{
    uchar get_luminance_approximate(uchar, uchar, uchar);
    WORD get_win32_col(uchar, uchar, uchar);
    int get_ncurses_col_index(uchar, uchar, uchar, short);
    std::string get_char_ansi_col(uchar, uchar, uchar, char);
}