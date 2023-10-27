#ifndef OPTIMISER_H
#define OPTIMISER_H

#include <iostream>
#include <stdlib.h>

typedef unsigned char uchar;

namespace TermVideo
{
    class Optimiser
    {
    private:
        uchar prev_r, prev_g, prev_b, col_threshold;

    public:
        Optimiser();
        Optimiser(uchar);
        void set_prev_colours(uchar r, uchar g, uchar b);
        void set_colour_threshold(uchar col_threshold);
        bool should_apply_ansi_col(uchar r, uchar g, uchar b, uchar c);
    };
}

#endif