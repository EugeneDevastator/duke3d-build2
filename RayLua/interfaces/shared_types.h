//
// Created by omnis on 11/10/2025.
//

#ifndef BB_SHARED_TYPES_H
#define BB_SHARED_TYPES_H


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

#define FLOOR 1
#define CEIL 0

 // Convenience macros for flag operations
#define HAS_FLAG(flags, flag)    ((flags) & (flag))
 #define FLAG_ISOFF(flags, flag)    !((flags) & (flag))
#define SET_FLAG(flags, flag)    ((flags) |= (flag))
#define CLEAR_FLAG(flags, flag)  ((flags) &= ~(flag))
#define TOGGLE_FLAG(flags, flag) ((flags) ^= (flag))

#define PAN_TO_UV (1.0/8.0) // uvscale = pan * p2uv
#include "buildmath.h"


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
	point3d uvpt[5];// Origin Upt Vpt Corner(O+U+V) Eye;
	float eyebias;
	float persp_ratio; // 1 = perspective, 0 =ortho

} uvw_cords;

//--------------------------------------------------------------------------------------------------

#endif //RAYLIB_LUA_IMGUI_SHARED_TYPES_H