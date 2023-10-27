#include "optimiser.hpp"

TermVideo::Optimiser::Optimiser() {}
TermVideo::Optimiser::Optimiser(uchar col_threshold)
{
    this->set_colour_threshold(col_threshold);
    this->set_prev_colours(0, 0, 0);
}

/**
 * @brief Sets the previously used pixel colours
 *
 * @param r Redness value (0-255)
 * @param g Greenness value (0-255)
 * @param b Blueness value (0-255)
 */
void TermVideo::Optimiser::set_prev_colours(uchar r, uchar g, uchar b)
{
    this->prev_r = r;
    this->prev_g = g;
    this->prev_b = b;
}

/**
 * @brief Sets the threshold to determine whether a pixel's colour is considered different from another
 *
 * @param col_threshold Threshold value for each colour to check
 */
void TermVideo::Optimiser::set_colour_threshold(uchar col_threshold)
{
    this->col_threshold = col_threshold;
}

/**
 * @brief Checks whether to apply ANSI colour coding.
 *        Used to optimise frame printing performance
 *
 * @param r Redness value (0-255)
 * @param g Greenness value (0-255)
 * @param b Blueness value (0-255)
 * @param c ASCII character to be printed
 * @return bool Whether to use ANSI colour coding
 */
bool TermVideo::Optimiser::should_apply_ansi_col(uchar r, uchar g, uchar b, uchar c)
{
    uchar diff_r = abs(this->prev_r - r);
    uchar diff_g = abs(this->prev_g - g);
    uchar diff_b = abs(this->prev_b - b);

    // apply ansi colour if pixel is outside of threshold range of previous pixel
    // and character is not a blank
    bool apply_ansi = (diff_r > this->col_threshold && diff_g > this->col_threshold && diff_b > this->col_threshold) && (c != ' ');
    return apply_ansi;
}