//
// Created by omnis on 2/27/2026.
//

#ifndef BB_MAPFORM_B2_H
#define BB_MAPFORM_B2_H

#include <stdint.h>
#include <stdbool.h>
#include <buildmath.h>

#define SPRITE_B2_BLOCKING         (1 << 0)   // 1
#define SPRITE_B2_1         (1 << 1)   // 1
#define SPRITE_B2_FLIP_X           (1 << 2)   // 4
#define SPRITE_B2_FLIP_Y           (1 << 3)   // 8
#define SPRITE_B2_FACING        (1 << 4)   // 16
#define SPRITE_B2_FLAT_POLY    (1 << 5)   // 32
#define SPRITE_B2_ONE_SIDED        (1 << 6)   // 64
#define SPRITE_B2_IS_LIGHT     (1 << 16)   // 64
#define SPRITE_B2_IS_DYNAMIC     (1 << 17)    // for dynamic lights and all dynamic stuff.

#define CLIP_MOVE		(1<<0) // blocking
#define CLIP_HIT		(1<<1) // hitscan
#define CLIP_SIGHT		(1<<2) // vision block
#define CLIP_AOE		(1<<3) // aoe, hitradius block.
#define CLIP_USEACTION	(1<<4) // use raycasts
#define CLIP_IS_TRIGGER	(1<<5) // doesnt block.
#define CLIP_USER_1	(1<<6) // doesnt block.
#define CLIP_USER_2	(1<<7) // doesnt block.
#define CLIP_USER_3	(1<<8) // doesnt block.
#define CLIP_TEAM_PLR	(1<<9)
#define CLIP_TEAM_ENEMY	(1<<10)
#define CLIP_TEAM_NEUT	(1<<11)
#define CLIP_TEAM1	(1<<12)
#define CLIP_TEAM2	(1<<13)
#define CLIP_TEAM3	(1<<14)
#define CLIP_TEAM4	(1<<15)

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
#define TILING_TRIAG	(1<<3)
// use flags for 4 orientations x2 mirrored options = 8 values
#define TILING_XMIRR	(1<<4)
#define TILING_YMIRR	(1<<5)

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
#define TEZ_WORLDZ1	 (1<<6)
#define TEZ_WORLD_X	 ((1<<6) | (1<<0))
#define TEZ_WORLD_Y	 ((1<<6) | (1<<1))
#define TEZ_WORLD_Z	 ((1<<6) | (1<<2))

// will only be used for animated entities.
typedef struct {
	unsigned int refkind : 4;
	// 0 = self, 1 = wall, 2 = sprite, 3= projector
	unsigned int otez : 7; // TEZ flags
	unsigned int utez : 7;
	unsigned int vtez : 7;
	unsigned int ctez : 7; // 4th corner vector C = o + v + c

} procuvgen32_t; // procedural uv data

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
	point3d cropA;
	point3d cropB;
	point3d rot; // rot z is for deault uvs

	uint8_t mapping_kind; // uv amappings, regular, polar, hex, flipped variants etc. paralax.
	uint16_t tile_ordering; //
} uvform_t;
#define UVFORM_DEFAULT (uvform_t){{1,1,1},{0,0,0},{0,0,0},{1,1,1},{0,0,0},0,0}

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
	uint32_t sectid;
	uint16_t wid;
	uint16_t flags;
} surfref;

typedef struct {
	point3d colorhdr; // this is color block.
	float maxdist;
	float distance_bias; // for intensity calc.
	float forward_offset;
	point3d spot_axes;
	point3d spot_fades; // 1 = fade to center.
	point3d spot_fadexp; // exponent
} lightsource_t;

typedef struct {
	renderflags32_t rflags;
	// need some geometry flags for build, like do we skip, emit mask etc. could be better than just transparency.
	uvform_t uv;// scale xy, pan xy, crop Axy Bxy
	point3d anchor; // normalized
	transform viewtr; // lets go full double transform since view transform is mostly needed.
	point3d color; // this is color block.
	int16_t lum; // yes allow negative values, why not.
} sprview;

typedef struct {
	point3d v, av, pcenter;           //Position velocity, Angular velocity (direction=axis, magnitude=vel), center of mass
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

// ------------
	uint8_t reflection_strength;
	uint8_t pass_strength;
	uint8_t absorb_strength; // can be infered 1-pass-refl.
	uint8_t ior;

	unsigned short asc, rsc, gsc, bsc; //4096 is no change

// ------- runtime gneerated data
	// for portals case - we dont care and use original world for everything.
	// interpolator will lerp worldpositions, regardless of poly location
	point3d rt_uvs[5]; // world uv vectors. generated per poly. origin(world), uv(local pos, rot=world), c, eye

	// can in theory use object space and encode it.

} surf_t;

typedef struct // wall t
{
	// HOT DATA
	union {
		point2d pos;
		struct {
			float x, y;
		};
	};
	signed long n, ns, nw; //n:rel. wall ind.; ns & nw : nextsect & nextwall_of_sect
	signed long nschain, nwchain; // references for loops of touching walls.
	uint8_t surfn; // - remove
	uint8_t geoflags;
	surf_t surf, xsurf[3]; //additional malloced surfs when (surfn > 1)

	// COLD DATA
	uint32_t guid; // unique per wall. surfs alway follow top-bottom order.
	long owner; //for dragging while editing, other effects during game

	/*
	*Positive values: Point to the next wall in the loop
	wal[Wid].n is relative index to tihs wall's index.
	wal[Wid].n + Wid gives the absolute index of the next wall
	Used to traverse walls in order around a sector
	Negative values: Mark special wall positions in loops

	-1: Indicates the last wall in a loop
	-2, -3: Mark end positions for clipped polygons

	Looking at the code patterns, ideally only one wall per loop should have a negative n value - the last wall that closes the loop.
	*/

	int16_t mflags[4]; // modflags
	int32_t tags[16]; // standard tag is 4bytes
	int8_t tempflags; // used only in editor for data transfers.

} wall_t;

typedef struct // spri_t
{
	// ----- FAST DATA --------------
	union { transform tr; struct { point3d p, r, d, f; }; };

	//long tilnum;             //Model file. Ex:"TILES000.ART|64","CARDBOARD.PNG","CACO.KV6","HAND.KCM","IMP.MD3"
	union {
		uint32_t packed_tile_data;
		struct {
			uint32_t tilnum : 14;  // 16k tiles
			uint32_t galnum : 9;   // 512 gals
			uint32_t pal : 9;      // 512 pals.
		};
	};
	sprview view;

	long sect; //Current sector
	// to access next or prev sprite in sector of this sprite..
	// doubly-linked list of indices  inside sprites sector
	// bounded by -1 on both sides.
	long sectn, sectp;

	// --------- COLD DATA -----------, probably all move to datablock.

	uint32_t dataptr; // reference to any additional data.
	uint32_t guid; // uniq per sprite. automatic.
	long owner;
	short lotag, hitag;
	physdata phys;
	int32_t tags[16];
	long tim, otim;          //Time (in milliseconds) for animation
	int8_t walcon; // wall constraint -1 -2 = floor ceil, resolve as 2-wc; -3 = none;
	//Bit0:Blocking, Bit2:1WayOtherSide, Bit5,Bit4:Face/Wall/Floor/.., Bit6:1side, Bit16:IsLight, Bit17-19:SpotAx(1-6), Bit20-29:SpotWid, Bit31:Invisible
	long flags;
	uint8_t modid; // mod id - for game processors, like duke, doom, etc. 0 is reserved for core entities.
	uint16_t classid; // instead of implicit class recognition by spritenum or pal - use explicit. so for ex. we can just make 3d rpg rocket as prop.
	uint8_t linkmask; // damage, signal, use, - everything for linking with other communicators


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

typedef struct
{
	point3d startpos, startrig, startdow, startfor;
	transform startr; // todo - use.
	long startsectn;
	int numsects, malsects; sect_t *sect;
	int numspris, malspris; spri_t *spri;
	int blankheadspri;

#define MAXLIGHTS 256
	int light_spri[MAXLIGHTS], light_sprinum;
} mapstate_t;

#define pBREAKSBUNCH .flags & SURF_PARALLAX_DISCARD
#endif //RAYLIB_LUA_IMGUI_MAPFORM_B2_H