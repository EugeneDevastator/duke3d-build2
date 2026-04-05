//
// Created by omnis on 11/6/2025.
//

#ifndef GAME_DUKEWRAP_H
#define GAME_DUKEWRAP_H
#include "build.h"
#include "types.h"
#include "../../interfaces/engineapi.h"

#define SET_SPRITE_XY(a, b, c) (SetSprPosXY(a,b,c))
#define SET_SPRITE_XYZ(a, b, c,d) (SetSprPosXYZ(a,b,c,d))
#define SET_SPRITE_I(a) (SetSprPosXYZ(a,sprite[a].x,sprite[a].y,sprite[a].z))
#define SET_SECTOR_FLORZ(a, b) (SetSectorFloorZ(a,b))
#define SET_SECTOR_CEILZ(a, b) (SetSectorCeilZ(a,b))
#define DEL_SPRITE(a) (DeleteSprite(a))
#define SET_SPR_PIC(sid,pid) (rayl->SetSpritePicNum(mapToEngine[sid],pid))
#define SET_SPR_PIC_I(sid) (rayl->SetSpritePicNum(mapToEngine[sid],sprite[mapToEngine[sid]].picnum))
extern long mapToEngine[MAXSPRITES];
extern long mapToDuke[MAXSPRITES*100];
extern engineapi_t *rayl;
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
     void (*FindSectorOfPoint)(long x, long y, short *inoutSecNum); // for api use: int func(x,y,sectn)
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
     char* FrameInputs;
} dukewrapper;

extern dukewrapper bbeng; // bb= build 2
void SetSprPosXY(long i, long x, long y);
void SetSprPosXYZ(long i, long x, long y, long z);

void SetSectorFloorZ(int i, long z);
void SetSectorCeilZ(int i, long z);
void InitDukeWrapper(engineapi_t* api);
void ParseMapToDukeFormat();
void setPcursectnum(int pid, int sectn);
void InsertSprite(int sect, float x, float y, float z);
void InsertSpriteTMP(int sect, float x, float y, float z, int dukeid);
void DeleteSprite(int dukeid);
#endif //GAME_DUKEWRAP_H