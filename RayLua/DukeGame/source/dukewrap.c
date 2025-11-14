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
// mt = map transfer
// populating tags from the end.
#define PI 3.14159265358979323846

#define MT_LAST 15 // index, not count
#define MT_STATNUM (MT_LAST - 2)
#define MT_PICLOW (MT_LAST - 3)
#define MT_CSTAT (MT_LAST - 4)
#define MT_PICOVER (MT_LAST - 5)
#define MT_HNUMLOW (MT_LAST - 6)
#define MT_HNUMHI (MT_LAST - 7)
#define MT_SHADELOW (MT_LAST - 8)
#define MT_SHADEHI (MT_LAST - 9)
#define MT_VIS (MT_LAST - 10)
#define MT_EXTRA (MT_LAST - 11)
#define MT_FIRST_WALL (MT_LAST - 12)

#define FLOOR 1
#define CEIL 0

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
void SetSectorFloorZ(int i, long z) {
    sector[i].floorz = z;
    rayl->SetFloorZ(i, z / 512.0f / 16.0f);
}
void SetSectorCeilZ(int i, long z) {
    sector[i].floorz = z;
    rayl->SetFloorZ(i, z / 512.0f / 16.0f);
}

void SetSprPosXYZ(long i, long x, long y, long z) // not in .h file
{
    // convert to projection laters.
    sprite[i].x = x;
    sprite[i].y = y;
    sprite[i].z = z;
    rayl->SetSpritePos(i, x / 512.0f, y / 512.0f, z / 512.0f / 16.0f);
}
void SetSprPosXY(long i, long x, long y) // not in .h file
{
    // convert to projection laters.
    sprite[i].x = x;
    sprite[i].y = y;
    rayl->SetSpritePos(i, x / 512.0f, y / 512.0f, sprite[i].z / 512.0f / 16.0f);
}

spritetype ReadSprite(long i) {
    spritetype a = {};
    return a;
}

// is it ok to store internal function in pointer?
void InitDukeWrapper(engineapi_t *api) // pass in real api
{
    bbeng.SetSprPos = SetSprPos;
    bbeng.SetSprPosXY = SetSprPosXY;
    rayl = api;
    inputs = (char*)calloc(20 , sizeof(char));
    bbeng.FrameInputs = inputs;
    map = rayl->GetLoadedMap();
}


short forwardToAng(point3d forw) {
    return (int)(atan2(forw.y, forw.x) * 1024.0 / PI);
}
void ConvertSector(int i,sectortype* sect) {
    sect_t b2sec = map->sect[i];
    /* short */      sect->wallptr = b2sec.tags[MT_FIRST_WALL];// and this is count of walls, like idx = wallptr + i for i in (0..n)
    /* long */       sect->ceilingz = b2sec.z[CEIL];
    /* long */       sect->floorz = b2sec.z[FLOOR];
    /* short */      sect->ceilingstat = b2sec.tags[MT_STATNUM];
    /* short */      sect->floorstat = b2sec.tags[MT_STATNUM+1];
    /* short */      sect->ceilingpicnum = b2sec.surf[CEIL].tilnum;
    /* short */      sect->ceilingheinum = b2sec.tags[MT_HNUMHI];
    /* signed char*/ sect->ceilingshade = b2sec.tags[MT_SHADEHI];
    /* char */       sect->ceilingpal = b2sec.surf[CEIL].pal;
    /* char */       sect->ceilingxpanning=1;
    /* char */       sect->ceilingypanning=1;
    /* short */      sect->floorpicnum = b2sec.surf[FLOOR].tilnum;
    /* short */      sect->floorheinum = b2sec.tags[MT_HNUMLOW];
    /* signed char*/ sect->floorshade = b2sec.tags[MT_SHADELOW];
    /* char */       sect->floorpal = b2sec.surf[FLOOR].pal;
    /* char */       sect->floorxpanning = 1 ;
    /* char */       sect->floorypanning = 1 ;
    /* char */       sect->visibility = b2sec.tags[MT_VIS];
    /* char */       sect->filler=0;
    /* short */      sect->lotag = b2sec.surf[FLOOR].lotag;
    /* short */      sect->hitag = b2sec.surf[FLOOR].hitag;
    /* short */      sect->extra = b2sec.tags[MT_EXTRA];
}
void ConvertSprite(int i,spritetype* spr) {
    spri_t b2spr = map->spri[i];
    spr->x = b2spr.p.x * 512;
    spr->y = b2spr.p.y * 512;
    spr->z = b2spr.p.z * (512*16);
    spr->cstat = b2spr.tags[MT_CSTAT],
    spr->picnum = b2spr.tilnum;
    spr->shade = b2spr.tags[MT_SHADELOW];
    spr->pal = b2spr.pal,
    spr->clipdist = b2spr.fat,
    spr->filler = 0;
    spr->xrepeat =1,
    spr->yrepeat=1;
    spr->xoffset=0,
    spr->yoffset=0;
    spr->sectnum = b2spr.sectn,
    spr->statnum = b2spr.tags[MT_STATNUM];
    spr->ang = forwardToAng(b2spr.f),
    spr->owner =0 ,
    spr->xvel=0,
    spr->yvel=0,
    spr->zvel=0;
    spr->lotag = b2spr.lotag;
    spr->hitag = b2spr.hitag;
    spr->extra = b2spr.tags[MT_EXTRA];
}
void ConvertWall(int i,walltype* w, wall_t b2wall) {

        /* long */           w->x = b2wall.x * 512;
        /* long */           w->y = b2wall.y * 512;
        /* short */          w->point2 = b2wall.nw, // we can abolish it by extracting api.
        /* short */          w->nextwall = b2wall.nw,
        /* short */          w->nextsector = b2wall.ns,
        /* short */          w->cstat = b2wall.tags[MT_CSTAT];
        /* short */          w->picnum = b2wall.surf.tilnum;
                            if (b2wall.surfn>2)
        /* short */              w->overpicnum = b2wall.xsurf[1].tilnum;
                             else
                                 w->overpicnum = w->picnum;
        /* signedchar */     w->shade = b2wall.tags[MT_SHADELOW];
        /* char */           w->pal = b2wall.surf.pal;
        /* char */           w->xrepeat = 1;
        /* char */           w->yrepeat = 1;
        /* char */           w->xpanning = 1;
        /* char */           w->ypanning = 1;
        /* short */          w->lotag = b2wall.surf.lotag;
        /* short */          w->hitag = b2wall.surf.hitag;
        /* short */          w->extra = b2wall.tags[MT_EXTRA];
}

// todo sync sprita xy and floorpos at the frame end.
// and do keys.
void ParseMapToDukeFormat() {
    int numsprites;
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
        sector[i].wallptr = walln;
        for (int k = 0; k < map->sect[i].n; ++k) {
            ConvertWall(walln, &wall[walln],map->sect[i].wall[k]);
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

    // should account for findsectorofpoint
    for (int i = 0; i < numsprites; i++)
        insertsprite(sprite[i].sectnum, sprite[i].statnum);

    //Must be after loading sectors, etc!
    //bbeng.FindSectorOfPoint(ps[0].posx, ps[0].posy, &ps[0].cursectnum);
}

void GetInput() {
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
