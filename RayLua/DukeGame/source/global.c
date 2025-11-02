//
// Created by omnis on 11/2/2025.
//
#include "global.h"

long FindDistance2D(int ix, int iy)
{
    int   t;

    ix= abs(ix);        /* absolute values */
    iy= abs(iy);

    if (ix<iy)
    {
        int tmp = ix;
        ix = iy;
        iy = tmp;
    }

    t = iy + (iy>>1);

    return (ix - (ix>>5) - (ix>>7)  + (t>>2) + (t>>6));
}
