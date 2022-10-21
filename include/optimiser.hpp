#ifndef INCL_STD_H
#define INCL_STD_H
#include <iostream>
#include <stdlib.h>
#endif

#ifndef OPTIMISER_H
#define OPTIMISER_H

typedef unsigned char uchar;

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

#endif