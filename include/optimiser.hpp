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
    uchar prev_r, prev_g, prev_b, threshold;

public:
    void set_prev_colours(uchar, uchar, uchar);
    void set_colour_threshold(uchar);
    bool apply_ansi_col(uchar, uchar, uchar);
};

#endif