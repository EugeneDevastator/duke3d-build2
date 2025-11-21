//
// Created by omnis on 10/25/2025.
//

#ifndef BUILD2_ARTLOADER_H
#define BUILD2_ARTLOADER_H

#include "kplib.h"
#include "mapcore.h"

#define USEGROU 1
/*
typedef struct tiltyp {
	intptr_t f, p;           // f=frame buffer pointer, p=pitch/stride
	int x, y, z;            // x,y=dimensions, z=depth/format info
	float shsc;             // shsc=suggested height scale
	tiltyp *lowermip;       // pointer to lower mipmap level
} tiltyp;
*/
//static long nullpic [64+1][64]; //Null set icon (image not found)
static __forceinline unsigned int bsf (unsigned int a) { _asm bsf eax, a }
static __forceinline unsigned int bsr (unsigned int a) { _asm bsr eax, a }
extern unsigned char gammlut[256], gotpal;
extern tile_t *gtile;

//void loadpic (tile_t *tpic);

#endif //BUILD2_ARTLOADER_H