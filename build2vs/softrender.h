//
// Created by omnis on 11/21/2025.
//

#ifndef BUILD2_SOFTRENDER_H
#define BUILD2_SOFTRENDER_H
#include <cstdarg>
#ifndef _MSC_VER
static _inline int rgb_scale (int c0, int c1)
{
	unsigned char *u0, *u1;
	u0 = (unsigned char *)&c0;
	u1 = (unsigned char *)&c1;
	u0[0] = (unsigned char)min((((int)u0[0])*((int)u1[0]))>>7,255);
	u0[1] = (unsigned char)min((((int)u0[1])*((int)u1[1]))>>7,255);
	u0[2] = (unsigned char)min((((int)u0[2])*((int)u1[2]))>>7,255);
	return(c0);
}
#else
static _inline int rgb_scale (int c0, int c1)
{
	_asm
	{
		punpcklbw mm0, c0
		punpcklbw mm1, c1
		pmulhuw mm0, mm1
		psrlw mm0, 7
		packuswb mm0, mm0
		movd eax, mm0
		emms
	}
}
#endif
#define LFLATSTEPSIZ 3
#define FLATSTEPSIZ (1<<LFLATSTEPSIZ)
#define SCISDIST .001
#include "drawpoly.h"
#include "Core/mapcore.h"

int argb_interp (int c0, int c1, int mul15);

void drawpoly_flat_threadsafe (tiltyp *tt, vertyp *pt, int pn, int rgbmul, float hsc, float *ouvmat, int rendflags, cam_t &gcam);

void drawpollig (int ei);

//if (a < 0) return(0); else if (a > b) return(b); else return(a);
static int lbound0 (int a, int b) //b MUST be >= 0
;

// static __forceinline unsigned int bsf (unsigned int a) { _asm bsf eax, a }
// static __forceinline unsigned int bsr (unsigned int a) { _asm bsr eax, a }
int uptil1 (unsigned int *lptr, int z);

//NOTE: font is stored vertically first! (like .ART files)
extern const uint64_t font6x8[];
void print6x8 (tiltyp *ldd, int ox, int y, int fcol, int bcol, const char *fmt, ...);

void drawpix (tiltyp *ldd, int x, int y, int col);

void drawline2d (tiltyp *ldd, float x0, float y0, float x1, float y1, int col);

void drawline3d (cam_t *lcam, float x0, float y0, float z0, float x1, float y1, float z1, int col);

#ifdef STANDALONE
	//For debug only!
void drawpolsol (cam_t *lcam, point2d *pt, int pn, int lignum);

//For debug only!
void drawpolsol (cam_t *lcam, point3d *vt, int num, int lignum);
#endif


#endif //BUILD2_SOFTRENDER_H