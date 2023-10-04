#ifndef TERMINAL_H
#define TERMINAL_H

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#elif defined(__linux__)
#include <sys/ioctl.h>
#endif

#include <algorithm>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

namespace Vid2ASCII
{
    void set_terminal_title(const char *);
    void hide_terminal_cursor();
    void get_terminal_size(int &, int &);
    void init_terminal_col(bool);
}

#endif