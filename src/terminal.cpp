#include <iostream>
#include <terminal.hpp>

void set_terminal_title(char *title)
{
    std::cout << "\033]0;" << title << "\007";
}

// https://stackoverflow.com/questions/23369503/get-size-of-terminal-window-rows-columns/62485211#62485211
void get_terminal_size(int &width, int &height)
{
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    width = (int)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
    height = (int)(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
#elif defined(__linux__)
    struct winsize w;
    ioctl(fileno(stdout), TIOCGWINSZ, &w);
    width = (int)(w.ws_col);
    height = (int)(w.ws_row);
#endif
}

// https://stackoverflow.com/questions/5866529/how-do-we-clear-the-console-in-assembly/5866648#5866648
void clear_terminal(char fill = ' ')
{
    COORD tl = {0, 0};
    CONSOLE_SCREEN_BUFFER_INFO s;
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(console, &s);
    DWORD written, cells = s.dwSize.X * s.dwSize.Y;
    FillConsoleOutputCharacter(console, fill, cells, tl, &written);
    FillConsoleOutputAttribute(console, s.wAttributes, cells, tl, &written);
    SetConsoleCursorPosition(console, tl);
}

std::string get_char_col(unsigned char r, unsigned char g, unsigned char b, char c)
{
    if (c != ' ')
    {
        char str_out[24];
        sprintf_s(str_out, "\033[38;2;%d;%d;%dm%c", r, g, b, c);
        return std::string(str_out);
    }
    else
    {
        std::string str_out = "";
        str_out.push_back(c);
        return str_out;
    }
}