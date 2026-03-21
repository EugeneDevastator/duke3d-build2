//
// Created by omnis on 2/27/2026.
//

#ifndef BB_MAPFORM_B2_H
#define BB_MAPFORM_B2_H

#include <stdint.h>
#include <stdbool.h>
#include <buildmath.h>
// obsolte flags
// #define SPRITE_B2_BLOCKING       (1 << 0)   // 1
// #define SPRITE_B2_1              (1 << 1)   // 1
// #define SPRITE_B2_FLIP_X         (1 << 2)   // 4
// #define SPRITE_B2_FLIP_Y         (1 << 3)   // 8
// #define SPRITE_B2_FACING         (1 << 4)   // 16
// #define SPRITE_B2_FLAT_POLY      (1 << 5)   // 32
// #define SPRITE_B2_ONE_SIDED      (1 << 6)   // 64
 #define SPRITE_B2_IS_LIGHT			(1 << 16)   // 64
// #define SPRITE_B2_IS_DYNAMIC     (1 << 17)    // for dynamic lights and all dynamic stuff.

#define CLIP_MOVE		(1<<0) // blocking
#define CLIP_HIT		(1<<1) // hitscan
#define CLIP_SIGHT		(1<<2) // vision block
#define CLIP_AOE		(1<<3) // aoe, hitradius block.
#define CLIP_USEACTION	(1<<4) // use raycasts
#define CLIP_IS_TRIGGER	(1<<5) // doesnt block. / dope, would need trigger mask or script.
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

#define GEO_NO_BUNCHING (1<<0)
#define GEO_EMIT_WITHOUT_CLIPPING (1<<1) // will be clipped by zbuffer.
#define GEO_PARALLAX_DISCAQRD (1<<2)

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

// ------ Duke Nukem tiling and coords.
// 1024 build units(x,y) correspond to 64 pixels. at 8,8 repeat.
// 1024 build units(x,y) correspond to 32 pixels. at 4,4 repeat
// 8192 z build units correspond to 32 pixels at 8x8 repeat.

#define TAG_COUNT_PER_SECT 32

// placeholders for readability
// Dont use in main parser!
#define TEZ_OS 0
#define TEZ_RAWZ 0
#define TEZ_CEIL 0

#define TEZ_NW		 (1<<0)  // use this or following wall(wal[x].n+x)
#define TEZ_NS		 (1<<1) // this or next sect, resulting wall is determined by own sector order.
#define TEZ_FLOR	 (1<<2)  // use floor or ceil
#define TEZ_SLOPE	 (1<<3) // slope or rawz;
#define TEZ_CLOSEST	 (1<<4) // closest height point instead of arbitrary.
#define TEZ_FURTHEST (1<<5) // furthest height point instead of arbitrary.
#define TEZ_WORLD_X	 (1<<6) // unit x
#define TEZ_WORLD_Y	 (1<<7)  // unit y
#define TEZ_WORLDZ1	 (1<<8) // unit z
#define TEZ_WALL_ORTHO	 (1<<9) // aligned to slope if use slope.
#define TEZ_IS_ORTHO_TO_PREV	 (1<<10) // generate ortho in plane V from U, c from v
#define TEZ_MAKE_UNIT_VEC	 (1<<11) // define as unit vector instead of full vector.
#define TEZ_TRAP	 (1<<12) //  to fit 4-pointed trap, be it cap or wall surf

#if 1 // =================================== STABLE STORAGE ==========================
#pragma pack(push, 1)
typedef uint64_t bb_uid_t;  // global unique identifier. classless. storage use only.

typedef struct {
	uint16_t otez; // TEZ flags
	uint16_t utez;
	uint16_t vtez;
	uint16_t ctez; // 4th corner vector C = o + v + c
	uint8_t preflags; // quick option?.
} procuvgen64_t; // procedural uv data


typedef struct {
	int16_t r, g, b, a; // /32767.0 = -1..1, can be negative
	uint16_t mantissa;   // /65535.0 = 0..1
	int16_t  exp;        // scale = mantissa * 2^exp, negative exp = dim
} color_hdr_t;

// float scale =  (luma.mantissa / 65535.0f) * powf(2.0f, luma.exp);
// float R = (chroma.r / 32767.0f) * scale;

typedef struct { int64_t x,y,z; } point64_t;
typedef struct { int32_t x,y,z; } point32_t;
typedef struct { int16_t x,y,z; } point16_t;

typedef struct { point64_t p,r,f,d; } transform64_t;
typedef struct { point32_t p,r,f,d; } transform32_t;
typedef struct { point16_t p,r,f,d; } transform16_t;

typedef struct {
	// use zdim for possible volumetric textures, impostors,
	point16_t scale;
	point16_t pan;
	point16_t cropA;
	point16_t cropB;
	point16_t rot; // rot z used for 2d uvs.

	uint8_t scaling_mode; // fit, use texel density, etc
	uint8_t mapping_kind; // uv amappings, regular, polar, hex, triang, parallax, etc.
	uint16_t tile_ordering; // 1 bit = flip, 2 bits = 4 rotations. 3bits X 4n tiles sharing vert. = 12 bits
	uint16_t _pad;
} uvform128_t; // generic uvform for 2d and 3d volumetric mappings.

// basic info on graphic representation
typedef struct {
	uint16_t galnum;
	uint16_t tilnum;
	uint16_t pal;
	uint16_t fx;  // parallax/shader/distortion effects
	color_hdr_t color;
	uint8_t blend_kind;   // compositing mode
	uint8_t optic_flags;   // optic_flags_resolv8_t
	uint16_t geoflags; // geoflags16_t

	procuvgen64_t procuv;
	uvform128_t uvform;
} mat_surf;

typedef struct {
	uint16_t volpal;
	uint16_t fx;  // parallax/shader/distortion effects
	color_hdr_t color_scatter;
	color_hdr_t color_transmit;
	uint16_t density_mul_f;

	uint8_t blend_kind;   // compositing mode
	uint8_t optic_flags;   // getting, light, shadows etc.
	uvform128_t uvform; // for 3D perlin noise for ex.
} mat_volume;


typedef struct {
	uint8_t mode;
	uint16_t a,b,c; // wall ids or local sprite or global sprite.
} slopehint_t;

typedef struct {
	transform32_t localbounds; // relative to transform.
	point32_t v, av, mcenter;           //Position velocity, Angular velocity (direction=axis, magnitude=vel), center of mass
	int32_t mas, drag, k1, k2; //as float
} physdata_store;


typedef struct {
	bb_uid_t id;
	bb_uid_t dataid; // reference to any additional composite data. not needed, can be in dynamic data.
	uint32_t lotag; // lowtag usually classid.
	uint32_t hitag; // tX, quick send. here, because it is very common in build maps.
	uint32_t commtag; // rx tax, receiver id  by default just dupe it with hitag.
	int32_t cmdtag; // some info storage
	uint8_t typeflags; // some flags related only to this instance.
} dataheader;

typedef struct {
	dataheader head;
	uint64_t nsec; // 0 = no link, so offset by 1 after read.
	uint16_t entersurf; // including 0 1 as caps;
	uint16_t thisspri; // for arbitrary portal. pick one from other surf. 0 means use surf-form.
	uint16_t clipmask; // for this side only!
	mat_surf mat;
} surf_store; // type for fundamental surface def, non visual.

typedef struct {
	dataheader head;
	uint16_t surfconstraint; // 0 = none, 1 2 = cceil flor, 3+ = walls; for movement purposes.
	transform64_t tr;
	transform16_t view_transform; // always  relative to original transform.
	uint16_t scriptflags; // script_flags16_t
	uint16_t clipmask; // block, hitscan, trigger, - for physics engine // aka layer
	physdata_store physics;
	mat_surf material;
} sprite_store;

typedef struct {
	surf_store surf0; // must have at least one; will get header from this one as well.

	int64_t x,y;
	uint8_t nxsurfs;
} wall_store;

typedef struct{
	surf_store surf0; // must have at least one; will get header from this one as well.

	slopehint_t slopehint;
	point64_t gradxy_zpos; //ceil&flor grad. grad.x = norm.x/norm.z, grad.y = norm.y/norm.z
	// grad.z is z value.//ceil&flor height pos.
	uint8_t nxsurfs;
} cap_t;

typedef struct // SECT STORE
{
	dataheader head;
	cap_t caps[2]; // 0=ceil, 1=floor;
	uint16_t originwall; // persistent first wall storage.
	mat_volume volmat; // volumetric material

	uint16_t n_walls;
	uint32_t n_spris;
	// wall_store [n_walls]; // walls
	// uint32_t [n_spris]; // sprite indices in order
} sect_store;

typedef struct {
	dataheader head;
	transform64_t origin; //will align origin wall to this transform.
	uint8_t kind; // 0 - normal. 1 - inverted. 2- procedural subtract, 3- overlay.
	uint8_t chunkflags; //
	// rotation and position transforms.
	// use some wall as origin, or even sprite.
} chunk_store;

// todo - check ken's png lib;
typedef struct {
	char type[4];
	bb_uid_t guid;
	uint32_t length;
	//unsigned char[length]; // data
	uint32_t crc;
} data_block; // dynamic data store.

typedef struct {
	uint8_t  magic[4];   // "BB2\0"
	uint16_t ver_major;
	uint16_t ver_minor;
	transform64_t start;
	uint64_t n_chunks;
	uint64_t n_sects;
	uint64_t n_sprites;
	// stable data stored here [chunks][sect1 [ walls1 ].. ][sprites]
	uint64_t data_offset; // start of blob section
	uint64_t n_blocks; // start of blob section
} map_b2_store_header_t;
#pragma pack(pop)
#endif

#if 1// ======================== UNIT CONVERSIONS ========================
// 1 unit = 1/65536 meter (shift 16)
// Max range: ~140 Tm (covers solar system)
// Resolution: ~0.015 mm (overkill for shooter, good for physics)
// float safe locally: full precision within ~128m of origin
// double safe: always

//  The distance to the nearest star, Proxima Centauri, is approximately 4,000,000 Gm.
//  9,223,372,036,854,775,807  int64 maxval. signed
//     ^   ^   ^   ^   ^
//     tm  gm  mm  km  m
// ---- int64 (defined elsewhere, shown for reference) ----
// shift 16, res ~0.015mm, max ~140 Tm

// ---- int32 ----
// shift 10, res ~0.977mm, max ~2100 km
// use for: large local spaces
#define SPACE_SCALE_SHIFT 16
#define SPACE_SCALE_F (1.0f / (float)(1 << SPACE_SCALE_SHIFT))
#define SPACE_SCALE_D (1.0  / (double)(1 << SPACE_SCALE_SHIFT))

#define SPACE32_SCALE_SHIFT 10
#define SPACE32_SCALE_F (1.0f / (float)(1 << SPACE32_SCALE_SHIFT))

#define SPACE16_SCALE_SHIFT 10
#define SPACE16_SCALE_F (1.0f / (float)(1 << SPACE16_SCALE_SHIFT))

// Convert stored int64 position unit to meters (float)
// NOTE: precision degrades beyond ~128m from origin due to float mantissa
static float int64_to_space_f(int64_t dim) {
	return (float)dim * SPACE_SCALE_F;
}

// Use this when computing distances between two far points
// subtract in int64 first, THEN convert — keeps precision
//float int64_delta_to_space_f(int64_t a, int64_t b) {
//  return (float)(a - b) * SPACE_SCALE_F;
//}
/*
// Double version for world-level calculations
double int64_to_space_d(int64_t dim) {
	return (double)dim * SPACE_SCALE_D;
}

// Convert meters back to int64 units
int64_t space_f_to_int64(float meters) {
	return (int64_t)(meters * (float)(1 << SPACE_SCALE_SHIFT));
}
point3d p64_to_point3d(point64_t p) {
	point3d ret;
	ret.x=int64_to_space_f(p.x);
	ret.y=int64_to_space_f(p.y);
	ret.z= int64_to_space_f(p.z);
	return ret;
}

transform tr64_to_transform(transform64_t t) {
	transform ret;
	ret.p = p64_to_point3d(t.p);
	ret.r = p64_to_point3d(t.r);
	ret.f = p64_to_point3d(t.f);
	ret.d = p64_to_point3d(t.d);
	return ret;
}
*/
/*
float int32_to_space_f(int32_t dim) {
	return (float)dim * SPACE32_SCALE_F;
}

float int32_delta_to_space_f(int32_t a, int32_t b) {
	return (float)(a - b) * SPACE32_SCALE_F;
}

int32_t space_f_to_int32(float meters) {
	return (int32_t)(meters * (float)(1 << SPACE32_SCALE_SHIFT));
}

float int16_to_space_f(int16_t dim) {
	return (float)dim * SPACE16_SCALE_F;
}

float int16_delta_to_space_f(int16_t a, int16_t b) {
	return (float)(a - b) * SPACE16_SCALE_F;
}

int16_t space_f_to_int16(float meters) {
	return (int16_t)(meters * (float)(1 << SPACE16_SCALE_SHIFT));
}*/
#endif

#if 1 // ==================== RUNTIME FORMATS, Editor  ====================
typedef struct {
	unsigned int active : 1;
	unsigned int initial_state : 1; // on/off open/closed etc.
	unsigned int state : 6+8; // custom state for statemachines. or user flags
} script_flags16_t;

typedef struct {
	unsigned int shadow_get : 1;
	unsigned int shadow_cast : 1; //  for lights = ambient
	unsigned int light_get : 1;
	unsigned int light_pass : 1;
	unsigned int is_dblsided : 1; //  for lights - marks if it appies to backfaces,
	unsigned int q: 2;        // queue 0-3: opaque, atest, transparent, overdraw
	unsigned int pad : 1; //  for lights - marks if it appies to backfaces,
} optic_flags_resolv8_t; // struct for triangulated rendering options. post-mono

typedef struct {
	unsigned int is_bunchbreak : 1;
	unsigned int is_mask_emitter : 1; // can be alpha test
	unsigned int is_tele_portal : 1; // for new style portals
	unsigned int is_light_portal : 1; // for new style portals
	unsigned int is_monoclip_blocker : 1; // for full masks for ex.
	unsigned int is_locked_to_sector : 1; // for sprites that can be outside of sector bounds
	unsigned int RESERVED : 12;    // padding to 8
} geoflags16_t;

typedef struct {
	point3d ori;    // origin
	point3d u;      // U axis
	point3d v;		// V axis
	point3d c;      // 4th corner (O+U+V) - unused in planar/persp mode
	point3d norm;   // projection direction (normalized)
	point3d eye;	// eye/apex point (push far along -N for ortho)
	float eyeratio; // lerp argument for eye distance adjst. mostly for effects.
} uv_world_t;  // generated uv structure. runtime only.

// game-dependent. but can have in-engine
typedef struct {
	char reltype[3]; // PRT, POS, ROT, UVT,
	uint32_t guid;
	uint32_t guid2;
	uint32_t linkageId;
} relation;

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


#define LIGHT_SOURCE_DEFAULT (lightsource_t){BBPONE,1,0,1,0,BBPTLEN(360),BBPZERO,BBPONE}
typedef struct {
	unsigned int is_renderable : 1;
	unsigned int shadow_get : 1;
	unsigned int shadow_cast : 1; //  for lights = ambient
	unsigned int light_get : 1;
	unsigned int light_pass : 1;
	unsigned int is_dblsided : 1; //  for lights - marks if it appies to backfaces,
	unsigned int q: 3;        // queue 0-3: opaque, atest, transparent, postproc
	unsigned int blend_mode : 4;  // 0-7 blend modes
	unsigned int vert_mode : 4; // for anything that affects geometry
	unsigned int frag_mode : 5;   // for surface effects 1 = parallax.
} renderflags32_t; // struct for triangulated rendering options. post-mono


typedef struct {
	// use z for possible volumetric tex impostors
	point3d scale;
	point3d pan;
	point3d cropA;
	point3d cropB;
	point3d rot; // rot z is for deault uvs

	uint8_t scaling_mode; // fit, use texel density,
	uint8_t mapping_kind; // uv amappings, regular, polar, hex, triang, parallax, etc.
	uint16_t tile_ordering; // 1 bit = flip, 2 bits = 4 rotations. 3bits X 4n tiles sharing vert. = 12 bits
} uvform_t; // generic uvform for 2d and 3d volumetric mappings.

#define UVFORM_DEFAULT (uvform_t){{1,1,1},{0,0,0},{0,0,0},{1,1,1},{0,0,0},0,0}


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
} physdata_t;
// on asset loadin.
// art collection id + tilenum ==> runtime atlasnum and atlaspos.
// art collections are separate from mods, and mods depend on their id so match is coincidental!
// art collection have formats: art, waf, pngs.
// inside it resolves into raw array of rgba images.
// then packed into atlases
// then lookup tables are made.

typedef struct {
	// store it in two walls - in runtime aggregate for simplicity.
	// for clearcoat - additional surface material, not universal
	color4f reflect;
	color4f transmit;
	color4f scatter;
	color4f fresnel_f0; // per-channel, needed for gold/copper accurate curves 	// alpha is lerp parameter.
	float anisotropy; // -1 to 1, for brushed metals
	float roughness;
	float pass_fade_bias;   // dstance fade bias
	float pass_distance_bias;   // dstance fade bias
	float pass_dir_bias;    // lerp between having light source in original pos, and 1 = purely perpendicular.

	//Specular vs diffuse reflect becomes a roughness question - roughness=0 IS mirror, roughness=1 IS full diffuse. One param, physically correct.
} surfmat_t;

// ================== Additional non-stable data.
typedef struct {
	uint32_t id;
	uint32_t type;
	uint32_t data_offset;   // byte offset into blob
	uint32_t data_size;
	uint32_t children_offset; // offset into id array
	uint32_t n_children;
} ComponentRecord;

typedef struct { // LIGHT SOURCE STORAGE
	bb_uid_t id;
	point3d colorhdr; // this is color block.
	float multiplier; // additional mul.
	float maxdist; // distance to stop clipper; 0 = no stop;
	float distance_fade_bias; // for intensity calc only! 0 = default light.
	float forward_offset_power; // for lights that are behind wall, sun, etd. real offset = 2^offset
	point3d spot_angles_deg; // value = degrees.
	point3d spot_fage_deg; //  0= hard. value = degrees
	point3d spot_fadepower; // exponent,
	point3d distance_smear; // for pillar- lights, format unclear. 0 on axis = pillar. so probably per axis multipliers.
	// more than 1 will result in shrinking, while 0 will produce a pillar.
	// and would need shader formula fro local, not world axes.
} lightsource_t;

typedef struct {
	bb_uid_t id;
	transform tr;
	uint32_t secid;
	bb_uid_t data;
} entity_stored_t;

typedef struct
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
	short lotag, hitag;
	physdata_t phys;
	uint16_t gameflags; // difficulty modes, classes, multiplayer? there's a lot.
	uint16_t signalmask; // damage, signal, use, - everything for linking with other communicators

	// -------------- RUNTIME DATA

	long owner;

	// to access next or prev sprite in sector of this sprite..
	// doubly-linked list of indices  inside sprites sector
	// bounded by -1 on both sides.
	long sectn, sectp;

	// --------- COLD DATA -----------, probably all move to datablock.
	long flags;
	int32_t tags[16];
	uint16_t walcon; // surf constraint 0,1 = ceil,flor, 3+  = wall id -2
	//Bit0:Blocking, Bit2:1WayOtherSide, Bit5,Bit4:Face/Wall/Floor/.., Bit6:1side, Bit16:IsLight, Bit17-19:SpotAx(1-6), Bit20-29:SpotWid, Bit31:Invisible
	//long flags;

} spri_t;

typedef struct // surf_t
{
	bb_uid_t id;
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
	procuvgen64_t uvgen;
	uvform_t uvform;//[9]; // scale xy, pan xy, crop AB, rotation

	//Bit0:Blocking, Bit2:RelativeAlignment, Bit5:1Way, Bit16:IsParallax, Bit17:IsSkybox
	uint32_t flags;	short lotag, hitag;
	surfmat_t defmat; // default material

// ------------
	uint8_t reflection_strength;
	uint8_t pass_strength;
	uint8_t absorb_strength; // can be infered 1-pass-refl.
	uint8_t ior;

	unsigned short asc, rsc, gsc, bsc; //4096 is no change
	float alpha;
// ------- runtime gneerated data
	// for portals case - we dont care and use original world for everything.
	// interpolator will lerp worldpositions, regardless of poly location
	point3d rt_uvs[5]; // world uv vectors. generated per poly. origin(world), uv(local pos, rot=world), c, eye

	// can in theory use object space and encode it.

} surf_t;

typedef struct // wall t
{
	bb_uid_t id;
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
	uint32_t dataid; // reference to any additional data.
	uint32_t tguid; // unique per wall.

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
// ============= RUNTIME DATA ============
	long owner; //for dragging while editing, other effects during game
	int16_t mflags[4]; // modflags
	int32_t tags[16]; // standard tag is 4bytes
	int8_t tempflags; // used only in editor for data transfers.

} wall_t;

typedef struct
{
	bb_uid_t id;
	// BuildEngine base data
	uint32_t dataid; // reference to any additional data.
	uint32_t tguid;   // uniq per sect.
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

#endif

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