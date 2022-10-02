#include <optimiser.hpp>

/**
 * @brief Sets the previously used pixel colours
 *
 * @param r Redness value
 * @param g Greenness value
 * @param b Blueness value
 */
void Optimiser::set_prev_colours(uchar r, uchar g, uchar b)
{
    this->prev_r = r;
    this->prev_g = g;
    this->prev_b = b;
}

/**
 * @brief Sets the threshold to determine whether a pixel's colour is considered different from another
 *
 * @param threshold Threshold value for each colour to check
 */
void Optimiser::set_colour_threshold(uchar threshold)
{
    this->threshold = threshold;
}

/**
 * @brief Checks whether to apply ANSI colour coding.
 *        Used to optimise frame printing performance
 *
 * @param r Redness value
 * @param g Greenness value
 * @param b Blueness value
 * @return bool Whether to use ANSI colour coding
 */
bool Optimiser::apply_ansi_col(uchar r, uchar g, uchar b)
{
    uchar diff_r = abs(this->prev_r - r);
    uchar diff_g = abs(this->prev_g - g);
    uchar diff_b = abs(this->prev_b - b);

    bool apply_ansi = (diff_r > this->threshold && diff_g > this->threshold && diff_b > this->threshold);
    return apply_ansi;
}