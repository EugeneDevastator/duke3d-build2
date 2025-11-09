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

typedef struct {
    float deltaTime;        // Time since last frame in seconds
    float fixedDeltaTime;   // Fixed timestep (1/120.0f for Duke3D compatibility)
    float accumulator;      // For fixed timestep accumulation
    long totalTicks;        // Equivalent to old totalclock
} GameTimer;

GameTimer gameTimer = {0};
// Convert old units to seconds
#define TICS_TO_SECONDS(tics) ((float)(tics) / 120.0f)
#define SECONDS_TO_TICS(seconds) ((long)((seconds) * 120.0f))

// Convert TICSPERFRAME movements to per-second
#define MOVEMENT_TO_UNITS_PER_SEC(movement) ((float)(movement) * 26.0f)
// essentially 26 ticks per frame. so mps = vel/26
/*
void UpdateGameTimer() {
    static uint64_t lastTime = {0};
    static uint64_t frequency = {0};

   // if (frequency.QuadPart == 0) {
   //     QueryPerformanceFrequency(&frequency);
   //     QueryPerformanceCounter(&lastTime);
   //     return;
   // }

    uint64_t currentTime;
    QueryPerformanceCounter(&currentTime);

    gameTimer.deltaTime = (float)(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
    gameTimer.accumulator += gameTimer.deltaTime;
    gameTimer.fixedDeltaTime = 1.0f / 120.0f; // Match TICRATE

    // Update tick counter for compatibility
    gameTimer.totalTicks += (long)(gameTimer.deltaTime * 120.0f);

    lastTime = currentTime;
}

*/