#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#elif defined(__linux__)
#include <sys/ioctl.h>
#endif

#ifndef INCL_STD_HEADERS
#define INCL_STD_HEADERS
#include <iostream>
#endif

void set_terminal_title(char *);
void get_terminal_size(int &, int &);
void clear_terminal(char);
std::string get_char_col(unsigned char, unsigned char, unsigned char, char);
