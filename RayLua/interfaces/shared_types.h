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

 // Convenience macros for flag operations
#define HAS_FLAG(flags, flag)    ((flags) & (flag))
 #define FLAG_ISOFF(flags, flag)    !((flags) & (flag))
#define SET_FLAG(flags, flag)    ((flags) |= (flag))
#define CLEAR_FLAG(flags, flag)  ((flags) &= ~(flag))
#define TOGGLE_FLAG(flags, flag) ((flags) ^= (flag))


#define SPRITE_BLOCKING         (1 << 0)   // 1
#define SPRITE_SEMI_TRANSPARENT (1 << 1)   // 2
#define SPRITE_FLIP_X           (1 << 2)   // 4
#define SPRITE_FLIP_Y           (1 << 3)   // 8
#define SPRITE_WALL_ALIGNED     (1 << 4)   // 16
#define SPRITE_FLOOR_ALIGNED    (1 << 5)   // 32
#define SPRITE_ONE_SIDED        (1 << 6)   // 64
#define SPRITE_TRUE_CENTERED    (1 << 7)   // 128
#define SPRITE_HITSCAN          (1 << 8)   // 256
#define SPRITE_TRANSPARENT      (1 << 9)   // 512
#define SPRITE_IGNORE_SHADE     (1 << 11)  // 2048
#define SPRITE_INVISIBLE        (1 << 15)  // 32768

#define WALL_BLOCKING           (1 << 0)   // 1
#define WALL_BOTTOM_SWAP        (1 << 1)   // 2
#define WALL_ALIGN_FLOOR        (1 << 2)   // 4 // default is align ceil.
#define WALL_FLIP_X             (1 << 3)   // 8
#define WALL_MASKED             (1 << 4)   // 16
#define WALL_SOLID_MASKED       (1 << 5)   // 32
#define WALL_HITSCAN            (1 << 6)   // 64
#define WALL_SEMI_TRANSPARENT   (1 << 7)   // 128
#define WALL_FLIP_Y             (1 << 8)   // 256
#define WALL_TRANSPARENT        (1 << 9)   // 512  // usualy always combines with semitransp.

#define SECTOR_PARALLAX         (1 << 0)   // 1
#define SECTOR_SLOPED           (1 << 1)   // 2
#define SECTOR_SWAP_XY          (1 << 2)   // 4
#define SECTOR_EXPAND_TEXTURE   (1 << 3)   // 8
#define SECTOR_FLIP_X           (1 << 4)   // 16
#define SECTOR_FLIP_Y           (1 << 5)   // 32
#define SECTOR_TEXWALL_ALIGN    (1 << 6)   // 64
#define SECTOR_MASKED           (1 << 7)   // 128
#define SECTOR_TRANSLUCENT      (1 << 8)   // 256
#define SECTOR_REVERSE_TRANS    (SECTOR_MASKED | SECTOR_TRANSLUCENT) // 384


#define SPRITE_B2_BLOCKING         (1 << 0)   // 1
#define SPRITE_B2_1         (1 << 1)   // 1
#define SPRITE_B2_FLIP_X           (1 << 2)   // 4
#define SPRITE_B2_FLIP_Y           (1 << 3)   // 8
#define SPRITE_B2_FACING        (1 << 4)   // 16
#define SPRITE_B2_FLAT_POLY    (1 << 5)   // 32
#define SPRITE_B2_ONE_SIDED        (1 << 6)   // 64
#define SPRITE_B2_IS_LIGHT     (1 << 16)   // 64

#define UV_TEXELRATE 		0 // pixel-rated = duke default.
#define UV_NORMRATE 		1 // tile-rated
#define UV_TEXELFIT 		2 // fit preserving texelrate
#define UV_NORMFIT 		3 // fit to entire square
#define UV_PARALLAX_LIN 	4
#define UV_PARALLAX_CYL 	5
#define UV_PARALLAX_SPH 	6
#define UV_SKYBOX 			7
#define UV_WORLDXY 			7

#define TILING_SQUARE	(1<<0)
#define TILING_HEXSQ	(1<<1)
#define TILING_HEXFULL	(1<<2)
#define TILING_XMIRR	(1<<3)
#define TILING_YMIRR	(1<<4)

// can probably encode in bits:
// this or next wal;
// this or next sec;
// flor or ceil;
// raw z or slope z;

// placeholders for readability
// Dont use in main parser!
#define TEZ_OS 0
#define TEZ_RAWZ 0
#define TEZ_CEIL 0

#define TEZ_FLOR 1<<0  // use floor or ceil
#define TEZ_NS 1<<1 // this or next sect
#define TEZ_SLOPE 1<<2 // slope or rawz;
#define TEZ_INVZ 1<<3 // use next continious wall
#define TEZ_CLOSEST 1<<4 // closest height point instead of arbitrary.
#define TEZ_FURTHEST 1<<5 // closest height point instead of arbitrary.
#define TEZ_WORLDZ1 1<<6 // closest height point instead of arbitrary.

// auto resolution optioons, written in ouv wal
#define TEW_WORLDF -1
#define TEW_WORLDR -2
#define TEW_WORLDD -3
#define TEW_ORTHO -4
//Bit0:Blocking, Bit2:1WayOtherSide, Bit5,Bit4:Face/Wall/Floor/.., Bit6:1side, Bit16:IsLight, Bit17-19:SpotAx(1-6), Bit20-29:SpotWid, Bit31:Invisible
// ------ Duke Nukem tiling and coords.
// 1024 build units(x,y) correspond to 64 pixels. at 8,8 repeat.
// 1024 build units(x,y) correspond to 32 pixels. at 4,4 repeat
// 8192 z build units correspond to 32 pixels at 8x8 repeat.


#define PAN_TO_UV (1.0/8.0) // uvscale = pan * p2uv


// assume one unit is one uv, given scale. so units*unitstouv*scale.
// pan of 16 is 16 pixels. befre scaling.
// duke3d:
// x repeat 1 for wall means 8 pixels per entire wall length
// y repeat 1 for wall means 4 pixels per 8192 z units.
// x pan of 1 equals one pixel move before scaling. (so always 1 pixel)
// y pan of 8 = 1 pixel of 32x32 texture
// y pan of 2 = 1 pixel for 128x128  texture
// also pans are limited by 256. so large textures wont work.

static inline float GetPxOffsetVertical(int ysize, int ypan) {
	int pansPerPx = 256.0/ysize;  // ex 32 size, pans per px = 8
	float pxoffset = ypan / pansPerPx;  // ypan is 16; 16/8 = 2px offset.
}
static inline float GetPxOffsetHorizontal(int ypan) {
	return ypan;
}

// in vert shader: uv = orig world pos X uv transform.
// i need all original wpos, because mono polys.
// and i do need transform per poly.
// but good news - no need for wccw on uv, because we can just use original world information
// ow pos can be also baked, and updated only when walls move. but for now ill update always

// above is regardless of texture size.
// build seems to automatically change pan when you change texture
// kens conversion is kinda ok. so
// 512 build units will fit 32 px at 8,8 repeat
// 32 pixels per unit. that will be scale of 1.
// at scale 2 it becomes 64 and at 0.5 16 pxpu
//				spr->p.x = ((float)b7spr.x)*(1.f/512.f);
//				spr->p.y = ((float)b7spr.y)*(1.f/512.f);
//				spr->p.z = ((float)b7spr.z)*(1.f/(512.f*16.f));
// 					sur->uv[0].x = ((float)b7sec.surf[j].xpanning)/256.0;
//					sur->uv[0].y = ((float)b7sec.surf[j].ypanning)/256.0;
//

typedef struct { float x, y, z; } point3d;
typedef struct { double x, y, z; } dpoint3d; 	//Note: pol doesn't support loops as dpoint3d's!
typedef struct { float x, y; } point2d;
static void vscalar(point3d *p, float s) {
	p->x*=s;
	p->y*=s;
	p->z*=s;
}
typedef struct {
	point3d p, r, d, f;
} transform;

	//Map format:
typedef struct {
// view.xpos += xsize*scale * normal_offset * global_ppi.
	point3d scaling;

	// 0,0,0 = sprite is to the right and above pivot
	// y for flat sprites is disregarded, as we scale in world along x - right, and z - up.
	// 1,0,1 - will align upper right corner to pivot.
	point3d noffset;
} viewanchor; // maybe for billboards or do uvs union. it looks like this shouldbe full 3d transform matrix

typedef struct
{
	long tilnum, tilanm/*???*/;

	//Bit0:Blocking, Bit2:RelativeAlignment, Bit5:1Way, Bit16:IsParallax, Bit17:IsSkybox
	union { uint32_t flags; struct { char _f1, _f2, _f3, pal; }; }; // temporary pal storage
	union { long tag; struct { short lotag, hitag; }; };
	point2d uv[3]; // legacy.
	unsigned short asc, rsc, gsc, bsc; //4096 is no change
	float alpha;
//-------- uvs
	union {
		// 0,1 - origin offset on the plane 2,3 - offset of the tile rect, for rotation anim for ex
		point2d uvoffset[4];
		struct {
			point2d
				planarworldoffset, // move origin in plane space in world.
				tilerectoffset, // move tile against origin
				crop1, // crop from 0,0
				crop2; // crop from 1,1 both in tile rect.
		};
	};

	union{
		// we dont use sectors, current one is constraint.
		// origin, u , v
		int8_t uvalig[6];
		// tez = tex z source
		struct { int8_t owal, otez, uwal, utez, vwal, vtez; }; // wals are always wals of this sector.
	};
	uint8_t uvmapkind; // uv amappings, regular, polar, hex, flipped variants etc. paralax.
	uint8_t tilingkind; // normal, polar, hex etc.

	short renderflags; // new flags;
// ------- runtime gneerated data
	// for portals case - we dont care and use original world for everything.
	// interpolator will lerp worldpositions, regardless of poly location
	point3d uvcoords[3]; // world uv vectors. generated per poly. origin, u ,v
	float uvform[6]; // scalexy, panxy
	// can in theory use object space and encode it.

} surf_t;

typedef struct
{
	union {
		point2d pos;
		struct {
			float x, y;
		};
	};

	/* ai
	*Positive values: Point to the next wall in the loop

	wal[w].n + w gives the absolute index of the next wall
	Used to traverse walls in order around a sector
	Negative values: Mark special wall positions in loops

	-1: Indicates the last wall in a loop
	-2, -3: Mark end positions for clipped polygons

	Looking at the code patterns, ideally only one wall per loop should have a negative n value - the last wall that closes the loop.
	*/

	long n, ns, nw; //n:rel. wall ind.; ns & nw : nextsect & nextwall_of_sect
	long owner; //for dragging while editing, other effects during game
	long surfn;
	surf_t surf, *xsurf; //additional malloced surfs when (surfn > 1)
	uint16_t mflags[4]; // modflags
	union {
		int64_t tags8b[8];
		int32_t tags[16]; // standard tag is 4bytes
		int16_t tags2b[32];
	};

} wall_t;

typedef struct
{
	union { transform tr; struct { point3d p, r, d, f; }; };
	point3d v, av;           //Position velocity, Angular velocity (direction=axis, magnitude=vel)
	float fat, mas, moi;     //Physics (moi=moment of inertia)
	long tilnum;             //Model file. Ex:"TILES000.ART|64","CARDBOARD.PNG","CACO.KV6","HAND.KCM","IMP.MD3"
	unsigned short asc, rsc, gsc, bsc; //Color scales. 4096 is no change
	long owner;
	union { long tag; struct { short lotag, hitag; }; };
	long tim, otim;          //Time (in milliseconds) for animation

	//Bit0:Blocking, Bit2:1WayOtherSide, Bit5,Bit4:Face/Wall/Floor/.., Bit6:1side, Bit16:IsLight, Bit17-19:SpotAx(1-6), Bit20-29:SpotWid, Bit31:Invisible
	union { long flags; struct { char _f1, _f2, _f3, pal; }; }; // temporary pal storage

	long sect; //Current sector
	// to access next or prev sprite in sector of this sprite..
	long sectn, sectp; // doubly-linked list of indices
	int32_t tags[16];
	///
	uint8_t modid; // mod id - for game processors, like duke, doom, etc. 0 is reserved for core entities.
	uint16_t classid; // instead of implicit class recognition by spritenum or pal - use explicit. so for ex. we can just make 3d rpg rocket as prop.
	uint8_t clipmask; // block, hitscan, trigger, - for physics engine
	uint8_t linkmask; // damage, signal, use, - everything for linking with other communicators

	uint8_t renderflags; // isinvisible, one-way,
	uint8_t renderclass; // billboard, flat, flat-box, flat-box-bupe, pipe-down-aligned, voxel, mesh, sdf, etc.

		struct {
			unsigned int pal	: 6;    // 64 pals
			unsigned int filt   : 3; // |9, for ex find edges, invert, mask etc.
			unsigned int blend	: 4; // |13, additive, multiply, etc.
			unsigned int _		: 3; // 16
		} fx;
	// float3 [0..1] for local positioning based on scaling.
	point3d anchor;
	// scale and pan - inside of the generated rect, dont affect world size.
	float uv[4];
} spri_t;

typedef struct
{
	float minx, miny, maxx, maxy; //bounding box
	float z[2];      //ceil&flor height
	point2d grad[2]; //ceil&flor grad. grad.x = norm.x/norm.z, grad.y = norm.y/norm.z
	surf_t surf[2];  //ceil&flor texture info
	wall_t *wall;
	long n, nmax;    //n:numwalls, nmax:walls malloced (nmax >= n)
	long headspri;   //hd sprite index (-1 if none)
	long foglev;
	long owner;      //for dragging while editing, other effects during game
	int32_t tags[16];
	uint16_t mflags[4];
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