//
// Created by omnis on 2/20/2026.
//

#ifndef RAYLIB_LUA_IMGUI_RENDER_TYPES_H
#define RAYLIB_LUA_IMGUI_RENDER_TYPES_H

#include "..\interfaces\shared_types.h"
typedef struct {
	transform tr;
	uint16_t tilnum;
	uint16_t pal;
	uint8_t galnum;
	point3d color;
	uint16_t lum;
	uint32_t sprid;
} spripoly_t;

typedef struct {
	int vert0;                          // Index into first vertex in eyepolv
	int b2sect, b2wall, b2slab;         // Build2 geometry references
	int b2hashn;                        // Hash chain for polygon matching
	int curcol, flags;                  // Color and rendering flags
	tile_t *tpic;                       // Texture tile pointer
	int tilnum;
	int galnum;
	float shade;
	float ouvmat[9];                    // inverse perspective transformation
	point3d norm;                       // Surface normal vector
	int rdepth;
	// triangulation data
	int triidstart, tricnt; // start ids and num of indice
	bool hasuvs;
	int8_t isflor;
	// uv data
	point3d worlduvs[3]; // origin, u ,v
	float* uvform;
	int slabid;
	int c1,c2,e1,e2;
	int pal;
	float alpha;
} eyepol_t;

typedef struct {
	int vert0;                          // Index into first vertex in eyepolv
	int triidstart, tricnt; // start ids and num of indice

	uint32_t id; // spr or sec
	uint8_t id2; // wall or flor
	uint8_t id3; // slab.
	enum fragRenderMode fragmode;
	enum vertRenderMode vertmode;
	transform tr;
	point3d norm;                       // Surface normal vector
	int pal;

	int tilnum;
	int galnum;
	int rdepth;
	float alpha;
	renderflags32_t rflags;

	// uv data
	bool hasuvs;
	point3d *worlduvs; // origin, u ,v
	float* uvform;

	// debug data
	int c1,c2,e1,e2;

} polyout_t;
#endif //RAYLIB_LUA_IMGUI_RENDER_TYPES_H