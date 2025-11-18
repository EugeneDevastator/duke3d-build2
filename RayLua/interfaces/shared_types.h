//
// Created by omnis on 11/10/2025.
//

#ifndef RAYLIB_LUA_IMGUI_SHARED_TYPES_H
#define RAYLIB_LUA_IMGUI_SHARED_TYPES_H
#include <stdint.h>
// duke tags defs.
#define MT_LAST 15 // index, not count

#define MT_STATCEIL (MT_LAST - 1)

#define MT_STATNUM (MT_LAST - 2)
#define MT_STATFLOOR (MT_LAST - 2)


#define MT_PICLOW (MT_LAST - 3)

#define MT_CSTAT (MT_LAST - 4)

#define MT_PICOVER (MT_LAST - 5)

#define MT_SEC_HNLOW (MT_LAST - 6)
#define MT_WALLPT2 (MT_LAST - 6)
#define MT_SPR_XREP (MT_LAST - 6)

#define MT_SEC_HNHI (MT_LAST - 7)
#define MT_NEXTWALL (MT_LAST - 7)
#define MT_SPR_YREP (MT_LAST - 7)

#define MT_SHADELOW (MT_LAST - 8)
#define MT_SPR_CLIPDIST (MT_LAST - 8)

#define MT_SHADEHI (MT_LAST - 9)

#define MT_VIS (MT_LAST - 10)
#define MT_EXTRA (MT_LAST - 11)

#define MT_SEC_FWALL (MT_LAST - 12)
#define MT_WAL_WALLIDX (MT_LAST - 12)
#define MT_SPR_XOFF (MT_LAST - 12)

#define MT_SEC_WALLNUM (MT_LAST - 13)
#define MT_WAL_NEXTSEC (MT_LAST - 13)
#define MT_SPR_YOFF (MT_LAST - 12)

#define FLOOR 1
#define CEIL 0


typedef struct { float x, y, z; } point3d;
typedef struct { double x, y, z; } dpoint3d; 	//Note: pol doesn't support loops as dpoint3d's!
typedef struct { float x, y; } point2d;
	//Map format:
typedef struct
{
	long tilnum, tilanm/*???*/;

	//Bit0:Blocking, Bit2:RelativeAlignment, Bit5:1Way, Bit16:IsParallax, Bit17:IsSkybox
	union { long flags; struct { char _f1, _f2, _f3, pal; }; }; // temporary pal storage
	union { long tag; struct { short lotag, hitag; }; };
	point2d uv[3];
	unsigned short asc, rsc, gsc, bsc; //4096 is no change
	short renderflags; // new flags;
	short uvVertexAnchor; // 0= lower left, CW order
} surf_t;

typedef struct
{
	float x, y;
	long n, ns, nw; //n:rel. wall ind.; ns & nw : nextsect & nextwall_of_sect
	long owner; //for dragging while editing, other effects during game
	long surfn;
	// maybe make portal innate?

	surf_t surf, *xsurf; //additional malloced surfs when (surfn > 1)
	int32_t tags[16];
} wall_t;

typedef struct
{
	point3d p, r, d, f;      //Position, orientation (right, down, forward)
	point3d v, av;           //Position velocity, Angular velocity (direction=axis, magnitude=vel)
	float fat, mas, moi;     //Physics (moi=moment of inertia)
	long tilnum;             //Model file. Ex:"TILES000.ART|64","CARDBOARD.PNG","CACO.KV6","HAND.KCM","IMP.MD3"
	unsigned short asc, rsc, gsc, bsc; //Color scales. 4096 is no change
	long owner;
	union { long tag; struct { short lotag, hitag; }; };
	long tim, otim;          //Time (in milliseconds) for animation

	//Bit0:Blocking, Bit2:1WayOtherSide, Bit5,Bit4:Face/Wall/Floor/.., Bit6:1side, Bit16:IsLight, Bit17-19:SpotAx(1-6), Bit20-29:SpotWid, Bit31:Invisible
	union { long flags; struct { char _f1, _f2, _f3, pal; }; }; // temporary pal storage

	long sect, sectn, sectp; //Current sector / doubly-linked list of indices
	int32_t tags[16];
} spri_t;

typedef struct
{
	float minx, miny, maxx, maxy; //bounding box
	float z[2];      //ceil&flor height
	point2d grad[2]; //ceil&flor grad. grad.x = norm.x/norm.z, grad.y = norm.y/norm.z
	surf_t surf[2];  //ceil&flor texture info
	wall_t *wall;
	long n, nmax;    //n:numwalls, nmax:walls malloced (nmax >= n)
	long headspri;   //head sprite index (-1 if none)
	long foglev;
	long owner;      //for dragging while editing, other effects during game
	int32_t tags[16];

	// int nwperim - perimeter walls, would be first in sequence
	// int nwnested - nested walls for fully inner sectors
	// could be purely runtime info.
	// but also inverted sector should be much easier to do.


} sect_t;

typedef struct {
	long nsect;
	sect_t* sects;
	long spriorig; // sprite that denotes origin
	point3d transform[3]; // pos, rot, scale or fdr
	uint8_t kind; // 0 - normal. 1 - inverted. 2- procedural subtract, 3- overlay.
	// rotation and position transforms.
	// use some wall as origin, or even sprite.
} chunk_t;

//--------------------------------------------------------------------------------------------------
typedef struct
{
	point3d startpos, startrig, startdow, startfor;
	long startsectn;
	int numsects, malsects; sect_t *sect;
	int numspris, malspris; spri_t *spri;
	int blankheadspri;

#define MAXLIGHTS 256
	int light_spri[MAXLIGHTS], light_sprinum;
} mapstate_t;
#endif //RAYLIB_LUA_IMGUI_SHARED_TYPES_H