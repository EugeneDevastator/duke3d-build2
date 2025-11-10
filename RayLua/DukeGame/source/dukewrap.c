//
// Created by omnis on 11/6/2025.
//
#include "dukewrap.h"
#include "../../interfaces/engineapi.h"

#include <stdlib.h>

#include "engine.h"
#include "keyboard.h"
dukewrapper bbeng;
engineapi_t *rayl;
char *inputs;
mapstate_t* map;

void SetSprPos(long i, long x, long y, long z) // not in .h file
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

void SetSprPosXY(long i, long x, long y) // not in .h file
{
    // redirect to main api.
    // main api. set pos (i, x-z,y) for ex.
}

spritetype ReadSprite(long i) {
    spritetype a = {};
    return a;
}

// is it ok to store internal function in pointer?
void InitWrapper(engineapi_t *api) // pass in real api
{
    bbeng.SetSprPos = SetSprPos;
    bbeng.SetSprPosXY = SetSprPosXY;
    rayl = api;
    inputs = malloc(20 * sizeof(char));
    bbeng.FrameInputs = inputs;
    map = rayl->GetLoadedMap();
}

short forwardToAng(point3d forw) {
    return 0;
   //       map->startfor.x = cos(((float)s)*PI/1024.0);
    //      map->startfor.y = sin(((float)s)*PI/1024.0);
}
void ConvertSector(int i,sectortype* sect) {

}
void ConvertSprite(int i,spritetype* spr) {

spr->lotag = map->spri[i].lotag;
spr->hitag = map->spri[i].hitag;

}
void ConvertWall(int i,walltype* w) {
// also store map ow n - b2wall
}
void ParseMapToDukeFormat() {
    initspritelists();

    clearbuf((&show2dsector[0]), (long)((MAXSECTORS + 3) >> 5), 0L);
    clearbuf((&show2dsprite[0]), (long)((MAXSPRITES + 3) >> 5), 0L);
    clearbuf((&show2dwall[0]), (long)((MAXWALLS + 3) >> 5), 0L);
    ps[0].posx = map->startpos.x;
    ps[0].posy = map->startpos.y;
    ps[0].posz = map->startpos.z;
    ps[0].ang = forwardToAng(map->startfor);
    ps[0].cursectnum = -1; // do update at th end

    numsectors = (short)map->numsects;
    for (int i = 0; i < numsectors; ++i) {
        ConvertSector(i,&sector[i]);
    }
    // --- walls ---
    int walln = 0;
    for (int i = 0; i < numsectors; ++i) {
        for (int k = 0; k < map->sect[i].n; ++k) {
            ConvertWall(walln, &wall[walln]);
            walln++;
        }
        walln+=map->sect[i].n;
    }
    numwalls = (short)walln;

    // sprites
    numsprites = (short)map->numspris;
    for (int i = 0; i < numsprites; ++i) {
        ConvertSprite(i,&sprite[i]);
    }

    for (int i = 0; i < numsprites; i++)
        insertsprite(sprite[i].sectnum, sprite[i].statnum);

    //Must be after loading sectors, etc!
    bbeng.FindSectorOfPoint(ps[0].posx, ps[0].posy, &ps[0].cursectnum);
}

void GetInput() {
    rayl->GetKeysThisFrame(inputs);
}


/*
*short nextsectorneighborz(short sectnum, long thez, short topbottom, short direction)
{
    walltype* wal;
    long i, testz, nextz;
    short sectortouse;

    if (direction == 1) nextz = 0x7fffffff;
    else nextz = 0x80000000;

    sectortouse = -1;

    wal = &wall[sector[sectnum].wallptr];
    i = sector[sectnum].wallnum;
    do
    {
        if (wal->nextsector >= 0)
        {
            if (topbottom == 1)
            {
                testz = sector[wal->nextsector].floorz;
                if (direction == 1)
                {
                    if ((testz > thez) && (testz < nextz))
                    {
                        nextz = testz;
                        sectortouse = wal->nextsector;
                    }
                }
                else
                {
                    if ((testz < thez) && (testz > nextz))
                    {
                        nextz = testz;
                        sectortouse = wal->nextsector;
                    }
                }
            }
            else
            {
                testz = sector[wal->nextsector].ceilingz;
                if (direction == 1)
                {
                    if ((testz > thez) && (testz < nextz))
                    {
                        nextz = testz;
                        sectortouse = wal->nextsector;
                    }
                }
                else
                {
                    if ((testz < thez) && (testz > nextz))
                    {
                        nextz = testz;
                        sectortouse = wal->nextsector;
                    }
                }
            }
        }
        wal++;
        i--;
    }
    while (i != 0);

    return (sectortouse);
}*/
int FindClosestSectorIdByHeigh(int sectnum, long baseZ, short isOtherFloor, short isDirectionUpward) {
    return 0; // look above for impl.
}

typedef struct {
    float deltaTime; // Time since last frame in seconds
    float fixedDeltaTime; // Fixed timestep (1/120.0f for Duke3D compatibility)
    float accumulator; // For fixed timestep accumulation
    long totalTicks; // Equivalent to old totalclock
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
