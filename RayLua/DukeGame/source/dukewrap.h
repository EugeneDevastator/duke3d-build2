//
// Created by omnis on 11/6/2025.
//

#ifndef GAME_DUKEWRAP_H
#define GAME_DUKEWRAP_H
#include "types.h"

typedef struct
{
    //set sprite position with sector correction
     void (*SetSprPos)(long i,long x, long y, long z);
     void (*SetSprPosXY)(long i, long x, long y);

    spritetype (*ReadSprite)(long i);
     void (*WriteSprite)(long i, spritetype s);

    int (*FindClosestSectorIdByHeigh)(int sectnum, long baseZ, short isOtherFloor, short isDirectionUpward);

    sectortype (*ReadSect)(long i);
     void (*WriteSect)(long i, sectortype sect);

     sectortype (*ReadSectP)(long i, sectortype *target);

     walltype (*ReadWall)(long i);
     void (*FindSectorOfPoint)(long x, long y, int *inoutSecNum); // for api use: int func(x,y,sectn)
     void (*WriteSectInfo)(long i, sectortype *s);
     void (*SetCeilHeight)(long sid, long height);
     void (*SetFloorHeight)(long sid, long height);
     long (*GetFloorZSloped)(long sid, long x, long y);
     void (*KeepAway)(long* x, long* y, long w); // see engine.c
     int (*HitScan)(long xs, long ys, long zs,
         short sectnum,
         long vx, long vy, long vz,
         short* hitsect, short* hitwall, short* hitsprite,
         long* hitx, long* hity, long* hitz,
         unsigned long cliptype);

     int (*arrpt)[10];
} dukewrapper;

extern dukewrapper bbeng; // bb= build 2

void InitWrapper();

#endif //GAME_DUKEWRAP_H