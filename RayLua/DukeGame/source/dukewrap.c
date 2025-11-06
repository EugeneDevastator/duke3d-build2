//
// Created by omnis on 11/6/2025.
//
#include "dukewrap.h"

#include <stdlib.h>
dukewrapper bbeng;

void SetSprPos(long i,long x, long y, long z) // not in .h file
{
    // redirect to main api.
// main api. set pos (i, x-z,y) for ex.

     // see int setsprite(short spritenum, long newx, long newy, long newz) in engine .c
    // need to update sprite sector as well.
}
void SetSprPosXY(long i,long x, long y) // not in .h file
{
    // redirect to main api.
    // main api. set pos (i, x-z,y) for ex.
}

// is it ok to store internal function in pointer?
void InitWrapper() // pass in real api
{
    bbeng.SetSprPos = SetSprPos;
    bbeng.SetSprPosXY = SetSprPosXY;
    // save main api
}


