#include "colour.hpp"

/**
 * @brief Returns an approximate relative luminance of pixel, skips linearisation for performance.
 *        https://en.wikipedia.org/wiki/Relative_luminance
 *
 * @param r Redness value (0-255)
 * @param g Greenness value (0-255)
 * @param b Blueness value (0-255)
 * @param force_avg_luminance Use the average of the RGB values instead of the relative luminance
 * @return double Approximate luminance value between 0 & 255
 */
uchar TermVideo::get_luminance_approximate(uchar r, uchar g, uchar b, bool force_avg_luminance)
{
    double r_mult = (force_avg_luminance) ? 0.3333 : 0.2126,
           g_mult = (force_avg_luminance) ? 0.3333 : 0.7152,
           b_mult = (force_avg_luminance) ? 0.3333 : 0.0722;

    double luminance = (r_mult * r) + (g_mult * g) + (b_mult * b);
    return static_cast<uchar>(luminance);
}

#if defined(_WIN32)
/**
 * @brief Returns a foreground colour closest to the RGB values given to be used with CHAR_INFO
 *
 * @param r Redness value (0-255)
 * @param g Greenness value (0-255)
 * @param b Blueness value (0-255)
 * @return WORD
 */
WORD TermVideo::get_win32_col(uchar r, uchar g, uchar b)
{
    WORD col = 0;
    uchar luminance = get_luminance_approximate(r, g, b, true);

    col |= (r >= 0x80) ? FOREGROUND_RED : 0;
    col |= (g >= 0x80) ? FOREGROUND_GREEN : 0;
    col |= (b >= 0x80) ? FOREGROUND_BLUE : 0;
    col |= (luminance >= 0x80) ? FOREGROUND_INTENSITY : 0;

    return col;
}
#endif

#if defined(__linux__)
/**
 * @brief Returns an index closest to the one set automatically
 *
 * @param r Redness value (0-255)
 * @param g Greenness value (0-255)
 * @param b Blueness value (0-255)
 * @return int Index of the colour pair (0-255)
 */
int TermVideo::get_ncurses_col_index(uchar r, uchar g, uchar b, short step)
{
    int divisions = step + 1;

    int r_int = r * divisions,
        g_int = g * divisions,
        b_int = b * divisions;

    int r_index = nearbyint(r_int / 255),
        g_index = nearbyint(g_int / 255),
        b_index = nearbyint(b_int / 255);

    int ncurses_col_index = (r_index * divisions * divisions) + (g_index * divisions) + b_index;
    return ncurses_col_index;
}
#endif

/**
 * @brief Returns a char encoded with ANSI colour
 *
 * @param r Redness value (0-255)
 * @param g Greenness value (0-255)
 * @param b Blueness value (0-255)
 * @param c Character to be encoded
 * @return std::string ANSI colour encoded character
 */
std::string TermVideo::get_char_ansi_col(uchar r, uchar g, uchar b, char c)
{
    if (c != ' ')
    {
        // max length is 24 for single character & RGB encoding
        char str_out[24];

#if _WINDLL
        sprintf_s(str_out, "\033[38;2;%d;%d;%dm%c", r, g, b, c);
#else
        snprintf(str_out, sizeof(str_out), "\033[38;2;%d;%d;%dm%c", r, g, b, c);
#endif

        return std::string(str_out);
    }

    else
    {
        return std::string(c, 1);
    }
}