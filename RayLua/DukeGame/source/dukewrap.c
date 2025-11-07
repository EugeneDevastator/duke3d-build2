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
    /*
    *    // move entirely into new engine
    short bad, j, tempsectnum;

    sprite[spritenum].x = newx;
    sprite[spritenum].y = newy;
    sprite[spritenum].z = newz;

    tempsectnum = sprite[spritenum].sectnum;
    updatesector(newx, newy, &tempsectnum);
    if (tempsectnum < 0)
        return (-1);
    if (tempsectnum != sprite[spritenum].sectnum)
        changespritesect(spritenum, tempsectnum);

    return (0);
     **/
}
void SetSprPosXY(long i,long x, long y) // not in .h file
{
    // redirect to main api.
    // main api. set pos (i, x-z,y) for ex.
}
spritetype ReadSprite(long i){
 spritetype a = {};
    return a;
}
// is it ok to store internal function in pointer?
void InitWrapper() // pass in real api
{
    bbeng.SetSprPos = SetSprPos;
    bbeng.SetSprPosXY = SetSprPosXY;
    // save main api
}


