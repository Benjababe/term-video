#include "terminal.hpp"

void Vid2ASCII::set_terminal_title(const char *title)
{
    std::cout << "\033]0;" << title << "\007";
}

void Vid2ASCII::hide_terminal_cursor()
{
#if defined(_WIN32)
    HANDLE write_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo(write_handle, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(write_handle, &cursorInfo);
#elif defined(__linux__)
    std::cout << "\033[?25l";
#endif
}

// https://stackoverflow.com/questions/23369503/get-size-of-terminal-window-rows-columns/62485211#62485211
void Vid2ASCII::get_terminal_size(int &width, int &height)
{
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    width = abs((int)(csbi.srWindow.Right - csbi.srWindow.Left + 1));
    height = abs((int)(csbi.srWindow.Bottom - csbi.srWindow.Top + 1));
#elif defined(__linux__)
    struct winsize w;
    ioctl(fileno(stdout), TIOCGWINSZ, &w);
    width = (int)(w.ws_col);
    height = (int)(w.ws_row);
#endif
}

void Vid2ASCII::init_terminal_col(bool print_colour)
{
    // white bg / black fg for grayscale, inverse for colour printing
    std::cout << ((print_colour) ? "\033[38;2;255;255;255m" : "\033[38;2;0;0;0m");
    std::cout << ((print_colour) ? "\033[48;2;0;0;0m" : "\033[48;2;255;255;255m");
}