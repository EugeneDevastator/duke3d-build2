//
// Created by omnis on 11/10/2025.
//

#ifndef RAYLIB_LUA_IMGUI_SHARED_TYPES_H
#define RAYLIB_LUA_IMGUI_SHARED_TYPES_H
#include <stdbool.h>
#include <stdint.h>

// type defines
// Declare an arena for type 'typ' with base name 'name'
// Creates: name (pointer), name##n (count), name##mal (capacity), name##siz (sizeof)
#define ARENA(typ, name) \
typ* name = NULL; \
int name##n = 0; \
int name##mal = 0; \
int name##siz = sizeof(typ)

#define ARENA_DECL(typ, name) \
extern typ* name; \
extern int name##n; \

// Add and assign value to arena, returns pointer to new element
// Expand arena to hold at least 'count' total elements
#define ARENA_EXPAND(name, count) \
do { \
if ((name##n+count) > name##mal) { \
name##mal += (count); \
name = realloc(name, name##mal * name##siz); \
} \
} while(0)

// Add and assign value to arena, returns pointer to new element
// Assumes sufficient capacity already allocated
#define ARENA_ADD(name, val) ( \
name[name##n] = (val), \
&name[name##n++] \
)

// Add without assignment, returns pointer to new element
#define ARENA_PUSH(name) ( \
(name##n >= name##mal) ? \
(name##mal = name##mal ? (name##mal << 1) : 16, \
name = realloc(name, name##mal * name##siz)) : name, \
&name[name##n++] \
)

// Free arena
#define ARENA_FREE(name) \
do { \
free(name); \
name = NULL; \
name##n = 0; \
name##mal = 0; \
} while(0)

// Reset without freeing
#define ARENA_RESET(name) (name##n = 0)
// --------------- struct arena --------------------

#define ARENAST_DECLARE(typ, name) \
struct { \
typ* data; \
int count; \
int capacity; \
} name = {0}

#define ARENAST_EXPAND(arena, needed) \
do { \
if ((arena).count + (needed) > (arena).capacity) { \
(arena).capacity = ((arena).count + (needed)) * 2; \
(arena).data = realloc((arena).data, (arena).capacity * sizeof(*(arena).data)); \
} \
} while(0)

#define ARENAST_PUSH(arena) \
(((arena).count >= (arena).capacity) ? \
((arena).capacity = (arena).capacity ? (arena).capacity * 2 : 16, \
(arena).data = realloc((arena).data, (arena).capacity * sizeof(*(arena).data))) : (arena).data, \
&(arena).data[(arena).count++])

#define ARENAST_ADD(arena, val) \
(*(ARENA_PUSH(arena)) = (val), &(arena).data[(arena).count-1])

#define ARENAST_FREE(arena) \
do { \
free((arena).data); \
(arena) = (typeof(arena)){0}; \
} while(0)

#define ARENAST_RESET(arena) ((arena).count = 0)
// ------------------------------------------------
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
#define SPRITE_IS_WALL_PLANE     (1 << 4)   // 16
#define SPRITE_IS_FLOOR_PLANE    (1 << 5)   // 32
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
#define SPRITE_B2_IS_DYNAMIC     (1 << 17)    // for dynamic lights and all dynamic stuff.

#define SURF_PARALLAX_DISCARD (1<<16) // marker for old build style parallax mode.

#define GEO_NO_BUNCHING (1<<1)

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



//Bit0:Blocking, Bit2:1WayOtherSide, Bit5,Bit4:Face/Wall/Floor/.., Bit6:1side, Bit16:IsLight, Bit17-19:SpotAx(1-6), Bit20-29:SpotWid, Bit31:Invisible
// ------ Duke Nukem tiling and coords.
// 1024 build units(x,y) correspond to 64 pixels. at 8,8 repeat.
// 1024 build units(x,y) correspond to 32 pixels. at 4,4 repeat
// 8192 z build units correspond to 32 pixels at 8x8 repeat.

#define TAG_COUNT_PER_SECT 16

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

typedef struct {
	float x, y, z;
} point3d;
typedef struct {
	float x, y, z, w;
} quat;
typedef struct {
	point3d uvpt[5];// Origin Upt Vpt Corner(O+U+V) Eye;
	float eyebias;
	float persp_ratio; // 1 = perspective, 0 =ortho

} uvw_cords;

// placeholders for readability
// Dont use in main parser!
#define TEZ_OS 0
#define TEZ_RAWZ 0
#define TEZ_CEIL 0

#define TEZ_NW		 (1<<0)  // use this or following wall(wal[x].n+x)
#define TEZ_NS		 (1<<1) // this or next sect
#define TEZ_FLOR	 (1<<2)  // use floor or ceil
#define TEZ_SLOPE	 (1<<3) // slope or rawz;
#define TEZ_CLOSEST	 (1<<4) // closest height point instead of arbitrary.
#define TEZ_FURTHEST (1<<5) // furthest height point instead of arbitrary.
#define TEZ_WORLDZ1	 (1<<6) // use unit world z vector

typedef struct {
	unsigned int otez : 7; // TEZ flags
	unsigned int utez : 7;
	unsigned int vtez : 7;
	unsigned int ctez : 7; // 4th corner vector C = o + v + c
	unsigned int o_do_regen : 1; // do we regen it on movement, if not, then vectors wont regenerate.
	unsigned int u_do_regen : 1; // v persp. vp1 = o +v; vp2 = o+v+vp.
	unsigned int v_do_regen : 1; // v persp. vp1 = o +v; vp2 = o+v+vp.
	unsigned int c_do_regen : 1; // v persp. vp1 = o +v; vp2 = o+v+vp.
} procuvgen32_t; // procedural uv data



typedef struct { double x, y, z; } dpoint3d; 	//Note: pol doesn't support loops as dpoint3d's!
typedef struct { float x, y; } point2d;

typedef struct {
	point3d p, r, d, f;
} transform;


typedef struct {
	unsigned int is_visible : 1;
	unsigned int is_light : 1; // for lights just store them. in separate struct.
	unsigned int shadow_get : 1;
	unsigned int shadow_cast : 1; //  for lights = ambient
	unsigned int is_dblside : 1; //  for lights - marks if it appies to backfaces,
	unsigned int q: 2;        // queue 0-3: opaque, atest, transparent, postproc
	unsigned int blend_mode : 3;  // 0-7 blend modes
	unsigned int vert_mode : 4; // for anything that affects geometry
	unsigned int frag_mode : 4;   // for surface effects 1 = parallax.
	unsigned int RESERVED : 4;    // padding to 32 bits
} renderflags32_t;

typedef struct {
	// use z for possible volumetric tex impostors
point3d scale;
point3d pan;
point3d anchorA;
point3d anchorB;
point3d rot; // rot z is for deault uvs

uint8_t mapping_kind; // uv amappings, regular, polar, hex, flipped variants etc. paralax.
uint16_t tile_ordering; // normal, polar, hex etc.

} uvform_t;

typedef struct {
	unsigned int is_bunchbreak : 1;
	unsigned int is_mask_emitter : 1; // can be alpha test
	unsigned int is_portal : 1; // for new style portals
	unsigned int is_traversal_blocker : 1; // for full masks for ex.
	unsigned int RESERVED : 6;    // padding to 32 bits
} geoflags8_t;


typedef struct {
	point3d ori;    // origin
	point3d u;      // U axis
	point3d v;		// V axis
	point3d c;    // 4th corner (BR) - unused in planar/persp mode
	point3d norm;   // projection direction (normalized)
	point3d eye;	// eye/apex point (push far along -N for ortho)
} uv_world_t;  // generated uv structure. runtime only.

enum vertRenderMode { // not part of the flags because must be chosen before any flags take place
	vmode_billbord = 0,     // also is very sprite- specific.
	vmode_wall = 0,     // also is very sprite- specific.
	vmode_vbord = 1,
	vmode_quad = 2,
	vmode_voxel = 3,
	vmode_mesh = 4,
	vmode_procedural = 5
};

// flats aka surfs.
enum fragRenderMode {
	fmode_flat,
	fmode_parallaxcyl,
	fmode_parallaxrect,
	fmode_parallaxdome,
	fmode_cubemap,
};

enum blendb2Mode {
	bmode_alpha = 0 , // standard blending aka normal
	bmode_add =1,
	bmode_mul =2 ,
	bmode_vectoradd = 3, // blend as vectors.in complex space
	bmode_error = 4,
};


typedef struct {
	renderflags32_t rflags;
	// need some geometry flags for build, like do we skip, emit mask etc. could be better than just transparency.
	float uv[8]; // scale xy, pan xy, crop Axy Bxy
	point3d anchor; // normalized
	point3d color;
	int16_t lum; // yes allow negative values, why not.
} sprview;

typedef struct {
	point3d v, av;           //Position velocity, Angular velocity (direction=axis, magnitude=vel)
	float fat, mas, moi;     //Physics (moi=moment of inertia)
	uint16_t clipmask; // block, hitscan, trigger, - for physics engine // aka layer
} physdata;
// on asset loadin.
// art collection id + tilenum ==> runtime atlasnum and atlaspos.
// art collections are separate from mods, and mods depend on their id so match is coincidental!
// art collection have formats: art, waf, pngs.
// inside it resolves into raw array of rgba images.
// then packed into atlases
// then lookup tables are made.
	//Map format:

typedef struct // surf_t
{
	// TEXTURE PARAMS.
	union {
		struct {
			uint32_t tilnum : 15;  // 32k tiles
			uint32_t galnum : 8;   // 256 gals
			uint32_t pal : 9;      // 512 pals.
		};
		uint32_t packed_tile_data;
	};
	enum fragRenderMode frMode;
	procuvgen32_t uvgen;
	uvform_t uvform;//[9]; // scale xy, pan xy, crop AB, rotation


	//Bit0:Blocking, Bit2:RelativeAlignment, Bit5:1Way, Bit16:IsParallax, Bit17:IsSkybox
	uint32_t flags;	short lotag, hitag;
	float alpha;

	// wtez is second skew vector, originating at v end. by default parallel to u. but can be inverted for trapezoid map.


// ------------
	uint8_t reflection_strength;
	uint8_t pass_strength;
	uint8_t absorb_strength; // can be infered 1-pass-refl.
	uint8_t ior;

	unsigned short asc, rsc, gsc, bsc; //4096 is no change

// ------- runtime gneerated data
	// for portals case - we dont care and use original world for everything.
	// interpolator will lerp worldpositions, regardless of poly location
	point3d rt_uvs[5]; // world uv vectors. generated per poly. origin, uv, c, eye

	// can in theory use object space and encode it.

} surf_t;

typedef struct // wall t
{
	uint32_t guid; // unique per wall. surfs alway follow top-bottom order.
	signed long n, ns, nw; //n:rel. wall ind.; ns & nw : nextsect & nextwall_of_sect
	signed long nschain, nwchain; // for multiportal.
	long owner; //for dragging while editing, other effects during game
	union {
		point2d pos;
		struct {
			float x, y;
		};
	};

	/* ai
	*Positive values: Point to the next wall in the loop
	wal[Wid].n is relative index to tihs wall's index.
	wal[Wid].n + Wid gives the absolute index of the next wall
	Used to traverse walls in order around a sector
	Negative values: Mark special wall positions in loops

	-1: Indicates the last wall in a loop
	-2, -3: Mark end positions for clipped polygons

	Looking at the code patterns, ideally only one wall per loop should have a negative n value - the last wall that closes the loop.
	*/

	// difference between ns and nschain is that ns points right to the target opening
	// but chains will be looped, similar to walls.


	uint8_t surfn;
	uint8_t geoflags;
	surf_t surf, xsurf[3]; //additional malloced surfs when (surfn > 1)
	int16_t mflags[4]; // modflags
	int32_t tags[16]; // standard tag is 4bytes
	int8_t tempflags; // used only in editor for data transfers.

} wall_t;

typedef struct // spri_t
{
	uint32_t guid; // uniq per sprite. automatic.
	union { transform tr; struct { point3d p, r, d, f; }; };

	//long tilnum;             //Model file. Ex:"TILES000.ART|64","CARDBOARD.PNG","CACO.KV6","HAND.KCM","IMP.MD3"
	union {
		uint32_t packed_tile_data;
		struct {
			uint32_t tilnum : 15;  // 32k tiles
			uint32_t galnum : 8;   // 256 gals
			uint32_t pal : 9;      // 512 pals.
		};
	};

	long owner;
	short lotag, hitag;
	long sect; //Current sector
	// to access next or prev sprite in sector of this sprite..
	// doubly-linked list of indices  inside sprites sector
	// bounded by -1 on both sides.
	long sectn, sectp;

	int32_t tags[16];
	long tim, otim;          //Time (in milliseconds) for animation
	int8_t walcon; // wall constraint -1 -2 = floor ceil, resolve as 2-wc; -3 = none;
	//Bit0:Blocking, Bit2:1WayOtherSide, Bit5,Bit4:Face/Wall/Floor/.., Bit6:1side, Bit16:IsLight, Bit17-19:SpotAx(1-6), Bit20-29:SpotWid, Bit31:Invisible
	long flags;
	uint8_t modid; // mod id - for game processors, like duke, doom, etc. 0 is reserved for core entities.
	uint16_t classid; // instead of implicit class recognition by spritenum or pal - use explicit. so for ex. we can just make 3d rpg rocket as prop.
	uint8_t linkmask; // damage, signal, use, - everything for linking with other communicators

	sprview view;
	physdata phys;
} spri_t;

typedef struct
{
	// BuildEngine base data
	uint32_t guid; // uniq per sector
	uint16_t areaid; // for trigger purposes. 0 = none
	float z[2];      //ceil&flor height
	point2d grad[2]; //ceil&flor grad. grad.x = norm.x/norm.z, grad.y = norm.y/norm.z
	surf_t surf[2];  //ceil&flor texture info
	wall_t *wall;
	long n, nmax;    //n:numwalls, nmax:walls malloced (nmax >= n)
	long headspri;   //hd sprite index (-1 if none)
	float minx, miny, maxx, maxy; //bounding box

	// other

	long owner;      //for dragging while editing, other effects during game
	int32_t tags[TAG_COUNT_PER_SECT];
	uint16_t mflags[4];
	short scriptid,lotag,hitag;
	// int nwperim - perimeter walls, would be first in sequence
	// int nwnested - nested walls for fully inner sectors
	// could be purely runtime info.
	// but also inverted sector should be much easier to do.
	int32_t destpn[2]; // nextsec flor ceil
	uint8_t hintw1,hintw2,hintmode; // wall hints for procedural slope.
	// first is always used as 2verts w, w+nw, second is just as point
	// mode is follow 1 follow 2 follow 1+2 or ignore.

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

#define pBREAKSBUNCH .flags & SURF_PARALLAX_DISCARD
#endif //RAYLIB_LUA_IMGUI_SHARED_TYPES_H