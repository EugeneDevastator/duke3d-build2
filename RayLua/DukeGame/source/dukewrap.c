//
// Created by omnis on 11/6/2025.
//
#include "dukewrap.h"
#include "../../interfaces/engineapi.h"

#include <stdlib.h>

#include "engine.h"
#include "game.h"
#include "keyboard.h"
dukewrapper bbeng;
engineapi_t *rayl;
char *inputs;
mapstate_t* map;
// mt = map transfer
// populating tags from the end.
#define PI 3.14159265358979323846
long mapToEngine[MAXSPRITES];
long mapToDuke[MAXSPRITES];

void SetSprPos(long i, long x, long y, long z) // not in .h file
{
    SetSprPosXYZ(i,x,y,z);
}

void InsertSprite(int sect, float x, float y, float z) {
    rayl->InsertSprite(sect, x / 512.0f, y / 512.0f, z / (512.f*16.f));
// need to handle b2 id somehow.
}
void InsertSpriteTMP(int sect, float x, float y, float z, int dukeid) {
    rayl->InsertSprite(sect, x / 512.0f, y / 512.0f, z / (512.f*16.f));
    mapToEngine[dukeid] = map->numspris-1;
    mapToDuke[map->numspris-1] = dukeid;
// need to handle b2 id somehow.
}


void DeleteSprite(int dukeid) {
    int movedplace = mapToEngine[dukeid];

    rayl->DeleteSprite(movedplace);
    int movedid = map->numspris;
    int dukelink = mapToDuke[movedid];
    mapToEngine[dukelink]= movedplace;
    mapToDuke[movedplace]=dukelink;
    mapToDuke[movedid]=-1;
    mapToEngine[dukeid]=-1;

}


void SetSectorFloorZ(int i, long z) {
    sector[i].floorz = z;
    rayl->SetFloorZ(i, z / (512.f*16.f));
}
void SetSectorCeilZ(int i, long z) {
    sector[i].ceilingz = z;
    rayl->SetCeilZ(i, z / (512.f*16.f));
}

void SetSprPosXYZ(long i, long x, long y, long z) // not in .h file
{
    int engineid = mapToEngine[i];
    // convert to projection laters.
    sprite[i].x = x;
    sprite[i].y = y;
    sprite[i].z = z;
    rayl->SetSpritePos(engineid, x / 512.0f, y / 512.0f, z / (512.f*16.f));
}
void SetSprPosXY(long i, long x, long y) // not in .h file
{
    int engineid = mapToEngine[i];
    // convert to projection laters.
    sprite[i].x = x;
    sprite[i].y = y;
    rayl->SetSpritePos(engineid, x / 512.0f, y / 512.0f, sprite[engineid].z / (512.f*16.f));
}

spritetype ReadSprite(long i) {
    spritetype a = {};
    return a;
}
void DoDukeUpdate(float dt) {
    DoDukeLoop(dt);
 //  sprite[ps[0].i].x = ps[0].posx;
 //  sprite[ps[0].i].y = ps[0].posy;
 //  sprite[ps[0].i].z = ps[0].posz;
    rayl->SetPlayerPos(
        ps[0].posx / 512.0f,
        ps[0].posy/ 512.0f,
        ps[0].posz/ (512.f*16.f)
        );

SET_SPRITE_XYZ(ps[0].i, ps[0].posx, ps[0].posy,ps[0].posz);
    sprite[ps[0].i].ang=ps[0].ang;

    long h = ps[0].horiz+ps[0].horizoff;
    float yaw = ((float)ps[0].ang) * PI / 1024.0f;
    float pitch = -((float)(h - 100)) * PI / 1024.0f;  // 100 is center

    // Forward vector combining yaw and pitch
    float x1 = cos(pitch) * cos(yaw);
    float y1 = cos(pitch) * sin(yaw);
    float z1 = sin(pitch);

    rayl->SetPlayerForward(x1,y1,z1);
}
// is it ok to store internal function in pointer?
void InitDukeWrapper(engineapi_t *api) // pass in real api
{
    bbeng.SetSprPos = SetSprPos;
    bbeng.SetSprPosXY = SetSprPosXY;
    bbeng.GetFloorZSloped = getceilzofslope;
    bbeng.FindSectorOfPoint = updatesector;
    rayl = api;
    bbeng.FrameInputs = api->Inputs;
    map = rayl->GetLoadedMap();
    rayl->RegisterUpdate(DoDukeUpdate);
}



short forwardToAng(point3d forw) {
    return (int)(atan2(forw.y, forw.x) * 1024.0 / PI);
}
void ConvertSector(int i,sectortype* sect) {
    sect_t b2sec = map->sect[i];
    /* short */      sect->wallptr = b2sec.tags[MT_SEC_FWALL];// and this is count of walls, like idx = wallptr + i for i in (0..n)
    /* short */      sect->wallnum = b2sec.n;// and this is count of walls, like idx = wallptr + i for i in (0..n)
    /* long */       sect->ceilingz = b2sec.z[CEIL] * (512*16);
    /* long */       sect->floorz = b2sec.z[FLOOR] * (512*16);
    /* short */      sect->ceilingstat = b2sec.tags[MT_STATCEIL];
    /* short */      sect->floorstat = b2sec.tags[MT_STATFLOOR];
    /* short */      sect->ceilingpicnum = b2sec.surf[CEIL].tilnum;
    /* short */      sect->ceilingheinum = b2sec.tags[MT_SEC_HNHI];
    /* signed char*/ sect->ceilingshade = b2sec.tags[MT_SHADEHI];
    /* char */       sect->ceilingpal = b2sec.surf[CEIL].pal;
    /* char */       sect->ceilingxpanning=1;
    /* char */       sect->ceilingypanning=1;
    /* short */      sect->floorpicnum = b2sec.surf[FLOOR].tilnum;
    /* short */      sect->floorheinum = b2sec.tags[MT_SEC_HNLOW];
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
    mapToEngine[i]=i;
    mapToDuke[i]=i;
    spri_t b2spr = map->spri[i];
    spr->x = b2spr.p.x * 512;
    spr->y = b2spr.p.y * 512;
    spr->z = b2spr.p.z * (512*16);
    spr->cstat = b2spr.tags[MT_CSTAT];
    spr->sectnum = b2spr.sect;
    spr->picnum = b2spr.tilnum;
    spr->shade = b2spr.tags[MT_SHADELOW];
    spr->pal = b2spr.pal;
    spr->clipdist = b2spr.tags[MT_SPR_CLIPDIST];
    spr->filler = 0;
    spr->xrepeat = b2spr.tags[MT_SPR_XREP];
    spr->yrepeat = b2spr.tags[MT_SPR_YREP];
    spr->xoffset = b2spr.tags[MT_SPR_XOFF];
    spr->yoffset = b2spr.tags[MT_SPR_YOFF];
    spr->statnum = b2spr.tags[MT_STATNUM];
    spr->ang = forwardToAng(b2spr.f);
    spr->owner = -1;//b2spr.sect;
    spr->xvel=0;
    spr->yvel=0;
    spr->zvel=0;
    spr->lotag = b2spr.lotag;
    spr->hitag = b2spr.hitag;
    spr->extra = b2spr.tags[MT_EXTRA];
}

void ConvertWall(wall_t b2wall) {
    long idx = b2wall.tags[MT_WAL_WALLIDX];
    walltype *w = &wall[idx];

    w->x = b2wall.x * 512;
    w->y = b2wall.y * 512;
    w->point2 = b2wall.tags[MT_WALLPT2]; // we can abolish it by extracting api.
    w->nextwall = b2wall.tags[MT_NEXTWALL],
    w->nextsector = b2wall.ns,
    w->cstat = b2wall.tags[MT_CSTAT];
    w->picnum = b2wall.surf.tilnum;
    if (b2wall.surfn > 2)
        w->overpicnum = b2wall.xsurf[1].tilnum;
    else
        w->overpicnum = w->picnum;
    /* signedchar */
    w->shade = b2wall.tags[MT_SHADELOW];
    w->pal = b2wall.surf.pal;
    w->xrepeat = 1;
    w->yrepeat = 1;
    w->xpanning = 1;
    w->ypanning = 1;
    w->lotag = b2wall.surf.lotag;
    w->hitag = b2wall.surf.hitag;
    w->extra = b2wall.tags[MT_EXTRA];
    w->nextwall = b2wall.tags[MT_NEXTWALL];
    w->nextsector = b2wall.tags[MT_WAL_NEXTSEC];
}
void setPcursectnum(int pid, int sectn) {
    ps[pid].cursectnum = sectn;
    if (sectn < 0) {
        int a = 1; // breakp.
    }
}
// todo sync sprita xy and floorpos at the frame end.
// and do keys.
void ParseMapToDukeFormat() {
    int numsprites;
    initspritelists();

    clearbuf((&show2dsector[0]), (long)((MAXSECTORS + 3) >> 5), 0L);
    clearbuf((&show2dsprite[0]), (long)((MAXSPRITES + 3) >> 5), 0L);
    clearbuf((&show2dwall[0]), (long)((MAXWALLS + 3) >> 5), 0L);
    ps[0].posx = map->startpos.x * 512;
    ps[0].posy = map->startpos.y* 512;
    ps[0].posz = map->startpos.z* 512*16;
    ps[0].poszv = 0;
    ps[0].ang = forwardToAng(map->startfor);
    ps[0].cursectnum = map->startsectn; // do update at th end

    numsectors = (short)map->numsects;
    for (int i = 0; i < numsectors; ++i) {
        ConvertSector(i,&sector[i]);
    }

    // --- walls ---
    int walln = 0;
    for (int i = 0; i < numsectors;i++) {
       //sector[i].wallptr = walln;
        for (int k = 0; k < map->sect[i].n; k++) {
            ConvertWall(map->sect[i].wall[k]);
            walln++;
        }
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
    updatesector(ps[0].posx, ps[0].posy, &ps[0].cursectnum);
    int a =1;
}

void GetInput() {
}

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
