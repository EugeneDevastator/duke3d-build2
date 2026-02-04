//
// Created by omnis on 10/25/2025.
//

#ifndef BUILD2_ARTLOADER_H
#define BUILD2_ARTLOADER_H
#include "kplib.h"
#include "stdint.h"
#define USEGROU 1
typedef union {
	uint32_t asint;
	struct {
		uint8_t animate_number : 6;    // bits 0-5: animate number (0-63)
		uint8_t animate_type : 2;      // bits 6-7: animate type (0=NoAnm, 1=Oscil, 2=AnmFd, 3=AnmBk)
		int8_t x_center_offset;        // bits 8-15: signed X-center offset
		int8_t y_center_offset;        // bits 16-23: signed Y-center offset
		uint8_t anim_speed :4;            // bits 24-28: animation speed
	};
} picanm_t;

typedef struct tiltyp {
	intptr_t f, p;           // f=frame buffer pointer, p=pitch/stride
	int x, y, z;            // x,y=dimensions, z=depth/format info
	float shsc;             // shsc=suggested height scale
	struct tiltyp *lowermip; // pointer to lower mipmap level
} tiltyp;
typedef struct
{
	// could be 'tiles014|4323'
	char filnam[240]; //Must fit packet header, sector&wall index, null terminator in 256 byte packet
	tiltyp tt; //union! if (!tt.p), it's a 3D model, tt.f points to raw model data, tt.x is type
	long namcrc32, hashnext;
} tile_t;

// responsible for storing info and temporary image data.
// image pixel data will be purged after it is loaded into gpu, so gallery will remain only as info source.
typedef struct {
	tile_t *gtile;
	int gnumtiles;
	int gmaltiles;
	uint16_t* sizex;
	uint16_t* sizey;
	picanm_t* picanm_data;  // Changed from uint16_t* picanm_t
	int gtilehashead[1024];
	char curmappath[260];
	unsigned char globalpal[256][4];
	unsigned char gammlut[256];
	unsigned char gotpal;
	uint16_t numtiles;
} gallery;

extern gallery g_gals[16];

static long nullpic [64+1][64]; //Null set icon (image not found)
static __forceinline unsigned int bsf (unsigned int a) { _asm bsf eax, a }
static __forceinline unsigned int bsr (unsigned int a) { _asm bsr eax, a }

static unsigned char gammlut[256], gotpal = 0;
extern tile_t *gtile;
extern unsigned char globalpal[256][4];
tile_t* getGtile(int i);
unsigned char* getColor(int idx);
void galfreetextures(int gal_idx);
void loadpic (tile_t *tpic, char* rootpath);
void setgammlut (double gammval);
void LoadPal(const char *basepath);
int loadgal(int gal_idx, const char* path);
#endif //BUILD2_ARTLOADER_H
