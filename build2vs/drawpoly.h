#ifndef KEN_DRAWPOLY_H
#define KEN_DRAWPOLY_H
#include "Core/artloader.h"
#include "Core/mapcore.h"
#define RENDFLAGS_INTERP      (1<< 0) //nearest vs. bilinear interpolation
#define RENDFLAGS_HEIGHT      (1<< 1) //flat vs. height mapped
#define RENDFLAGS_COVSID      (1<< 2) //for heightmapped: cover sides
#define RENDFLAGS_CULLBACK     0
#define RENDFLAGS_CULLFRONT   (1<< 3)
#define RENDFLAGS_CULLNONE    (1<< 4)
#define RENDFLAGS_ALPHAMASK   (1<< 5)
#define RENDFLAGS_RALPHAMASK  (1<< 6)
#define RENDFLAGS_OUVMAT      (1<< 7) //if 0, uses 1st 3 uv's. if !=0: uses ouvmat
#define RENDFLAGS_NODEPTHTEST (1<< 8) //!=0 : no depth test
#define RENDFLAGS_GMAT        (1<< 9) //!= : ouvmat acts as g_mat (overrides RENDFLAGS_OUVMAT)
#define RENDFLAGS_NOTRCP      (1<<10) //!= : no translation, rotation, clip, and project
#define RENDFLAGS_FLIPHEIGHT  (1<<11)
/*
typedef struct { float x, y, z; } point3d;
typedef struct { double x, y, z; } dpoint3d;
typedef struct { intptr_t f, p, x, y; } tiletype;
// Structure definition for tile/bitmap data
typedef struct tiltyp {
	intptr_t f, p;           // f=frame buffer pointer, p=pitch/stride
	int x, y, z;            // x,y=dimensions, z=depth/format info
	float shsc;             // shsc=suggested height scale
	tiltyp *lowermip;       // pointer to lower mipmap level
} tiltyp;
*/
//typedef struct { tiltyp c, z; point3d p, r, d, f, h; } cam_t;
typedef struct { float x, y, z, u, v; int n; } vertyp;

extern double drawpoly_anginc; //Raycast resolution. Default: 2.0
extern int drawpoly_numcpu;

	//Must call once before calling drawtopoly
extern void drawpoly_init (void);

	//Must call once per frame (or cbuf/zbuf/camera change)
extern void drawpoly_setup (tiletype *dd, intptr_t lzbufoff,  point3d *lpos,  point3d *lrig,  point3d *ldow,  point3d *lfor, float hx, float hy, float hz);
extern void drawpoly_setup (tiletype *dd, intptr_t lzbufoff, dpoint3d *lpos, dpoint3d *lrig, dpoint3d *ldow, dpoint3d *lfor, float hx, float hy, float hz);

	//Draw polygon!
extern void drawpoly (tiltyp *tt, vertyp *pt, int n, int rgbmul, float hsc, float *ouvmat, int rendflags);

	//cover-up function for kpzload: mallocs an extra line to make filter happy
	//flags: &1:signed:^0x80000000, &2:flip:^0xff000000
extern void kpzload4grou (const char *filnam, tiltyp *tt, float shsc, int flags);

	//Call this after modifying texture (such as animated water)
extern void fixtex4grou (tiltyp *tt);

	//Call this to scale non-power of 2 textures to next higher power of 2 (supports in-buffer conversion)
extern void scaletex_boxsum (tiltyp *rt, tiltyp *wt);

	//Optional function to apply lighting based on heightmap
extern void applyshade (tiltyp *tt, int shx, int shy);

#endif
