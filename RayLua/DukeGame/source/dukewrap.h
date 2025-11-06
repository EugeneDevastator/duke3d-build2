//
// Created by omnis on 11/6/2025.
//

#ifndef GAME_DUKEWRAP_H
#define GAME_DUKEWRAP_H
typedef struct
{
     void (*SetSprPos)(long i,long x, long y, long z);
     void (*SetSprPosXY)(long i, long x, long y);
     int (*arrpt)[10];
} dukewrapper;

extern dukewrapper bbeng; // bb= build 2

void InitWrapper();

#endif //GAME_DUKEWRAP_H