#include <colour.hpp>

/**
 * @brief Returns an approximate relative luminance of pixel, skips linearisation for performance.
 *        https://en.wikipedia.org/wiki/Relative_luminance
 *
 * @param r Redness value (0-255)
 * @param g Greenness value (0-255)
 * @param b Blueness value (0-255)
 * @return double Approximate luminance value between 0 & 255
 */
uchar Vid2ASCII::get_luminance_approximate(uchar r, uchar g, uchar b)
{
    double luminance = (0.2126 * r) + (0.7152 * g) + (0.0722 * b);
    return (uchar)luminance;
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
WORD Vid2ASCII::get_win32_col(uchar r, uchar g, uchar b)
{
    WORD col = 0;
    uchar luminance = get_luminance_approximate(r, g, b);

    col |= (r >= 0x80) ? FOREGROUND_RED : 0;
    col |= (g >= 0x80) ? FOREGROUND_GREEN : 0;
    col |= (b >= 0x80) ? FOREGROUND_BLUE : 0;
    col |= (luminance >= 0x80) ? FOREGROUND_INTENSITY : 0;

    return col;
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
std::string Vid2ASCII::get_char_ansi_col(uchar r, uchar g, uchar b, char c)
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