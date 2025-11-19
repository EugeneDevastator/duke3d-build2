//
// Created by omnis on 10/25/2025.
//

#ifndef BUILD2_ARTLOADER_H
#define BUILD2_ARTLOADER_H
#include "kplib.h"
#include "stdint.h"
#define USEGROU 1
typedef struct tiltyp {
	intptr_t f, p;           // f=frame buffer pointer, p=pitch/stride
	int x, y, z;            // x,y=dimensions, z=depth/format info
	float shsc;             // shsc=suggested height scale
	struct tiltyp *lowermip; // pointer to lower mipmap level
	uint32_t anmdata;
} tiltyp;

static long nullpic [64+1][64]; //Null set icon (image not found)
static __forceinline unsigned int bsf (unsigned int a) { _asm bsf eax, a }
static __forceinline unsigned int bsr (unsigned int a) { _asm bsr eax, a }
typedef struct
{
	// could be 'tiles014|4323'
	char filnam[240]; //Must fit packet header, sector&wall index, null terminator in 256 byte packet
	tiltyp tt; //union! if (!tt.p), it's a 3D model, tt.f points to raw model data, tt.x is type
	long namcrc32, hashnext;
} tile_t;
static unsigned char gammlut[256], gotpal = 0;
extern tile_t *gtile;
extern unsigned char globalpal[256][4];
tile_t* getGtile(int i);
unsigned char* getColor(int idx);

void loadpic (tile_t *tpic, char* rootpath);
void setgammlut (double gammval);
void LoadPal(const char *basepath);
#endif //BUILD2_ARTLOADER_H
