#if 0 //To compile, type "nmake drawcone.c"

!if 1
	#this makefile for VC6 and cwinmain.c/cwinmain.h
drawcone.exe: drawcone.obj cwinmain.obj kdisasm.obj
	 link      drawcone.obj cwinmain.obj kdisasm.obj\
	 ddraw.lib dinput.lib dxguid.lib gdi32.lib user32.lib /opt:nowin98
	 del drawcone.obj
drawcone.obj: drawcone.c cwinmain.h; cl /c /TP drawcone.c /Ox /G6Fy /MD /QIfist /DSTANDALONE
cwinmain.obj: cwinmain.c           ; cl /c /TP cwinmain.c /Ox /G6Fy /MD
kdisasm.obj:  kdisasm.c            ; cl /c /TP kdisasm.c  /Ox /G6Fy /MD
!endif

!if 0
	#this makefile for GCC and cwinmain.c/cwinmain.h
drawcone.exe:       drawcone.o cwinmain.o kdisasm.o
	\Dev-Cpp\bin\gcc drawcone.o cwinmain.o kdisasm.o\
	-o drawcone.exe -pipe -Os -s -lddraw -ldinput -ldxguid -lgdi32 -luser32 -mwindows
drawcone.o: drawcone.c cwinmain.h; \Dev-Cpp\bin\gcc -c -x c++ drawcone.c -o drawcone.o -pipe -O2 -s -DSTANDALONE
cwinmain.o: cwinmain.c           ; \Dev-Cpp\bin\gcc -c -x c++ cwinmain.c -o cwinmain.o -pipe -O2 -s
kdisasm.o:  kdisasm.c            ; \Dev-Cpp\bin\gcc -c -x c++ kdisasm.c  -o kdisasm.o  -pipe -O2 -s
!endif

#NOTE: relies on nmake.exe, no matter which compiler (VC vs. GCC) is selected. To use gcc's make util:
#      change: '!if 0' to: 'ifdef dontdefineme'; and at end, change: '!endif' to: 'endif'

!if 0
#endif

#define RENDMETH 2

	//i7-920@Brown:
	//RENDMETH: MT1:  MT8:
	//     0   13.88 13.39 (speed test: sphere count)
	//     1    6.13  1.74 (speed test: sphere fill rate)
	//     2    5.02  5.00 (speed test: cones)
	//     3    1.64  0.47 (interesting object)
	//     4    0.43  0.22 (test cases)

#if 0
//--------------------------------------------------------------------------------------------------
	//DRAWCONE.H would be:

	//Compiler define: Add this to the compile line to use an integer-based Z-buffer (*1048576.0/hz)
	//Note: the integer-based Z-buffer is slower (but fewer artifacts) than default floating point.

typedef struct { double x, y, z; } dpoint3d;
typedef struct { INT_PTR f, p; int x, y; } tiletype;

#define DRAWCONE_NOCAP0 1
#define DRAWCONE_NOCAP1 2
#define DRAWCONE_FLAT0 4
#define DRAWCONE_FLAT1 8
#define DRAWCONE_CENT 16
#define DRAWCONE_NOPHONG 32
#define DRAWCONE_NOCONE 64
#define DRAWCONE_CULL_BACK 0
#define DRAWCONE_CULL_FRONT 128
#define DRAWCONE_CULL_NONE 256

	//cputype: (1<<25) indicates SSE support, (1<<26) indicates SSE2 support
	//dd: color frame (must be 32-bit BGRA)
	//zbufoff: z_buffer_ptr - color_buffer_ptr (pitch MUST be the same)
	//ipos,irig,idow,ifor: pos.&ori. of camera. For convenience and calculation of shade angle.
	//hx,hy,hz: horizon offset, field of view. For 90 degree L-R fov, use: hx=hz=dd.x/2; hy=dd.y/2;
extern void drawcone_setup (int cputype, int numcpu, tiletype *dd, INT_PTR zbufoff,
	point3d *ipos, point3d *irig, point3d *idow, point3d *ifor, double hx, double hy, double hz);
extern void drawcone_setup (int cputype, int numcpu, tiletype *dd, INT_PTR zbufoff,
	dpoint3d *ipos, dpoint3d *irig, dpoint3d *idow, dpoint3d *ifor, double hx, double hy, double hz);

	//shade: controls contrast of shading. I like to use 38.4.
extern void drawsph (double x, double y, double z, double rad, int col, double shade);
extern void drawsph (double x, double y, double z, double rad, int col, double shade, int flags);

	//Supports cylinders, cones, and spheres. Note: use negative radius to flatten end.
extern void drawcone (double x0, double y0, double z0, double r0,
							 double x1, double y1, double z1, double r1, int col, double shade, int flags);

//--------------------------------------------------------------------------------------------------
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include <emmintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef STANDALONE
#include "cwinmain.h"
#else
typedef struct { INT_PTR f, p, x, y; } tiletype;
#endif

#ifndef _MSC_VER
# define _inline inline
# define GAS_ATT(x) \
	".att_syntax prefix\n\t" \
	x \
	".intel_syntax noprefix\n\t"
#endif

#if !defined(min)
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#if !defined(max)
#define max(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef _InterlockedExchangeAdd
#define _InterlockedExchangeAdd InterlockedExchangeAdd
#endif

#ifdef _MSC_VER
#define cvttss2si(f) _mm_cvtt_ss2si(_mm_set_ss(f))
#define ALIGN(i) __declspec(align(i))
typedef struct { float f[4]; } v4sf;
typedef struct { int f[4]; } v4si;
typedef struct { int f[2]; } v2si;
#else
__CRT_INLINE int cvttss2si (float f) { int i; __asm__ __volatile__("cvttss2si %1, %0": "=r" (i): "m" (f):); return(i); }
#define ALIGN(i) __attribute__((aligned(i)))
typedef float v4sf __attribute__((mode(V4SF)));
typedef int v4si __attribute__((mode(V4SI)));
typedef int v2si __attribute__((mode(V2SI)));
#endif
#define V4SI(v,n) (((int*)(v))[n])
#define V4SF(v,n) (((float*)(v))[n])

#define MAXXDIM 2048
#define MAXYDIM 2048
#define PI 3.14159265358979323

#define DRAWCONE_NOCAP0 1
#define DRAWCONE_NOCAP1 2
#define DRAWCONE_FLAT0 4
#define DRAWCONE_FLAT1 8
#define DRAWCONE_CENT 16
#define DRAWCONE_NOPHONG 32
#define DRAWCONE_NOCONE 64
#define DRAWCONE_CULL_BACK 0
#define DRAWCONE_CULL_FRONT 128
#define DRAWCONE_CULL_NONE 256

typedef struct { float x, y; } point2d;
typedef struct { float x, y, z; } point3d;
typedef struct { float x, y, z, w; } point4d;
typedef struct { double x, y; } dpoint2d;
typedef struct { double x, y, z; } dpoint3d;
typedef struct { int x0, y0, x1, y1; float hx, hy, hz, rhzup20, gds, sphclip[4]; } view_t;

static tiletype gdd;
static INT_PTR zbufoff;
static int cputype, gnumcpu;
static dpoint3d ipos, irig, idow, ifor;
static view_t gview;

#define LNPIX 2
#define NPIX (1<<LNPIX)
static int ireciplo[NPIX+1] = {0};
static float freciplo[NPIX+1] = {0};

typedef struct { int most[MAXYDIM][2], ymin, ymax, dummy[2]; } rast_t;
ALIGN(16) static rast_t imost[9]; //rasts: 0:cone_trap, 1-2:spheres, 3-4:circs, 5-8:htrun_temp_region

static int rr, gg, bb, gusephong = 0;
static ALIGN(16) v4si dqcolmul;
static point3d glig, glig2;
static dpoint3d np0, np10, dir;
static float kxx, kxy, kxz, kyy, kyz, kzz;
static float vzvz, vznp10z, gkx, gky, gkz, oZc, ok2, ok3, rk0, gdr, r0, r1;
static float gok2, gZc0, grr0, gvzp0z, gsgn, gligmul;
ALIGN(16) static point4d fnp10, gp0, gdir;
ALIGN(16) static int dqsgn[4];
ALIGN(16) static float dqnp0x[4], dqnp0y[4], dqnp0z[4], dqnp10x[4], dqnp10y[4], dqnp10z[4];
ALIGN(16) static float dqdirx[4], dqdiry[4], dqdirz[4], dqdirzhz[4];
ALIGN(16) static float dqvzvz[4], dqvznp10z[4], dqgot[4], dqgok2[4];
ALIGN(16) static float dqhx[4], dqhz[4], dqp0x[4], dqp0y[4], dqp0z[4], dqvzp0z[4], dqZc0[4], dqrr0[4];
ALIGN(16) static float dqligx[4], dqligy[4], dqligzhz[4], dqligmul[4], dqlig2x[4], dqlig2y[4], dqlig2z[4];
ALIGN(16) static float dqkxx[4], dqkxy[4], dqkxz[4], dqkyy[4], dqkyz[4], dqkzz[4];
ALIGN(16) static float dqkx[4], dqky[4], dqkz[4], dqoZc[4], dqok2[4], dqrk0[4], dqdr[4], dqr0[4];
static void (*glsl_shader4_func)(int *ixs, int iy, float *fdeps, int *icols);

//--------------------------------------------------------------------------------------------------
	//Ken Silverman's multithread library (http://advsys.net/ken)

	//Simple library that takes advantage of multi-threading. For 2 CPU HT, simply do this:
	//Change: for(i=v0;i<v1;i++) myfunc(i);
	//    To: htrun(myfunc,0,v0,v1,2);

#define MAXCPU 64
static HANDLE gthand[MAXCPU-1];
static HANDLE ghevent[2][MAXCPU-1]; //WARNING: WaitForMultipleObjects has a limit of 64 threads
static int gnthreadcnt = 0, glincnt[2];
#define getnextindexsynced() _InterlockedExchangeAdd((long *)&glincnt[0],1)
static void (*ghtcallfunc)(int, void *);
static void *ghtdata;
static void htuninit (void)
{
	int i, j;
	for(i=gnthreadcnt-1;i>=0;i--)
	{
		TerminateThread(gthand[i],0); //Dangerous if using system resources in thread, but we are not
		for(j=1;j>=0;j--)
			{ if (ghevent[j][i] != (HANDLE)-1) { CloseHandle(ghevent[j][i]); ghevent[j][i] = (HANDLE)-1; } }
	}
}
static unsigned int __stdcall ghtfunc (void *_)
{
	void (*lcallfunc)(int, void *);
	void *luserdata;
	int i, thnum = (int)_, endcnt;

	while (1)
	{
		WaitForSingleObject(ghevent[0][thnum],INFINITE);
		lcallfunc = ghtcallfunc; luserdata = ghtdata; endcnt = glincnt[1];
		while ((i = getnextindexsynced()) < endcnt) lcallfunc(i,luserdata);
		SetEvent(ghevent[1][thnum]);
	}
}
static void htrun (void (*lcallfunc)(int, void *), void *luserdata, int v0, int v1, int lnumcpu)
{
	int i, threaduse; unsigned win98requiresme;

		//1 CPU requested; execute here and quit.
	if (lnumcpu <= 1) { for(i=v0;i<v1;i++) lcallfunc(i,luserdata); return; }

		//Initialize new threads if necessary
	threaduse = min(lnumcpu,MAXCPU)-1;
	while (gnthreadcnt < threaduse)
	{
		if (!gnthreadcnt) { SetThreadAffinityMask(GetCurrentThread(),1<<(lnumcpu-1)); atexit(htuninit); }
		for(i=0;i<2;i++) ghevent[i][gnthreadcnt] = CreateEvent(0,0,0,0);
		gthand[gnthreadcnt] = (HANDLE)_beginthreadex(0,0,ghtfunc,(void *)gnthreadcnt,0,&win98requiresme);
		SetThreadAffinityMask(gthand[gnthreadcnt],1<<gnthreadcnt);
		gnthreadcnt++;
	}

		//Start other threads
	ghtcallfunc = lcallfunc; ghtdata = luserdata; glincnt[0] = v0; glincnt[1] = v1;
	for(i=threaduse-1;i>=0;i--) SetEvent(ghevent[0][i]);

		//Do some processing in this thread too :)
	while ((i = getnextindexsynced()) < v1) lcallfunc(i,luserdata);

		//Wait for all other threads to finish (for safety reasons)
	WaitForMultipleObjects(threaduse,&ghevent[1][0],1,INFINITE);
}

//--------------------------------------------------------------------------------------------------

void drawcone_setup (int lcputype, int lnumcpu, tiletype *ndd, INT_PTR lzbufoff,
	dpoint3d *npos, dpoint3d *nrig, dpoint3d *ndow, dpoint3d *nfor,
	double hx, double hy, double hz)
{
	float f;
	int i;

	cputype = lcputype; gnumcpu = lnumcpu; gdd = (*ndd); zbufoff = lzbufoff;
	ipos = (*npos); irig = (*nrig); idow = (*ndow); ifor = (*nfor);
	gview.hx = (float)hx; gview.hy = (float)hy; gview.hz = (float)hz;

	gview.rhzup20 = 1048576.f/(float)hz;
	gview.gds = (float)(hx*hx + hy*hy + hz*hz);
	f = (float)(hz*hz);
	gview.sphclip[0] = (float)max(gdd.x-hx,hx); gview.sphclip[1] = (float)sqrt(gview.sphclip[0]*gview.sphclip[0] + f);
	gview.sphclip[2] = (float)max(gdd.y-hy,hy); gview.sphclip[3] = (float)sqrt(gview.sphclip[2]*gview.sphclip[2] + f);

	if (!ireciplo[0])
	{
		ireciplo[0] = 1; freciplo[0] = 1.0;
		for(i=1;i<=NPIX;i++) { freciplo[i] = 1.f/((float)i); ireciplo[i] = (int)(freciplo[i]*4096.f); }
	}

	vzvz = hz*hz;
	for(i=4-1;i>=0;i--) dqvzvz[i] = vzvz;

	glig.x = (float)(irig.x + irig.y + irig.z);
	glig.y = (float)(idow.x + idow.y + idow.z);
	glig.z = (float)(ifor.x + ifor.y + ifor.z);
	f = 0.999f/(float)sqrt(glig.x*glig.x + glig.y*glig.y + glig.z*glig.z);
	glig.x *= f; glig.y *= f; glig.z *= f;

	for(i=4-1;i>=0;i--) { dqligx[i] = glig.x; dqligy[i] = glig.y; }
	dqligzhz[0] = glig.z*(float)hz; for(i=4-1;i>0;i--) dqligzhz[i] = dqligzhz[0];
	for(i=4-1;i>=0;i--) { dqhx[i] = gview.hx; dqhz[i] = gview.hz; }
}

void drawcone_setup (int lcputype, int lnumcpu, tiletype *ndd, INT_PTR lzbufoff,
	point3d *npos, point3d *nrig, point3d *ndow, point3d *nfor,
	double hx, double hy, double hz)
{
	dpoint3d dpos, drig, ddow, dfor;
	dpos.x = npos->x; dpos.y = npos->y; dpos.z = npos->z;
	drig.x = nrig->x; drig.y = nrig->y; drig.z = nrig->z;
	ddow.x = ndow->x; ddow.y = ndow->y; ddow.z = ndow->z;
	dfor.x = nfor->x; dfor.y = nfor->y; dfor.z = nfor->z;
	drawcone_setup(lcputype,lnumcpu,ndd,lzbufoff,&dpos,&drig,&ddow,&dfor,hx,hy,hz);
}

static void getperpvec (double nx, double ny, double nz, double *ax, double *ay, double *az, double *bx, double *by, double *bz)
{
	double f;

	if (fabs(nx) < fabs(ny))
		  { f = 1.0/sqrt(ny*ny + nz*nz); (*ax) =  0.0; (*ay) = nz*f; (*az) =-ny*f; }
	else { f = 1.0/sqrt(nx*nx + nz*nz); (*ax) =-nz*f; (*ay) =  0.0; (*az) = nx*f; }
	(*bx) = ny*(*az) - nz*(*ay);
	(*by) = nz*(*ax) - nx*(*az);
	(*bz) = nx*(*ay) - ny*(*ax);
	f = 1.0/sqrt((*bx)*(*bx) + (*by)*(*by) + (*bz)*(*bz)); (*bx) *= f; (*by) *= f; (*bz) *= f;
}

ALIGN(16) static const float dq32768f[4] = {32768.f,32768.f,32768.f,32768.f}, dq32767f[4] = {32767.f,32767.f,32767.f,32767.f};
ALIGN(16) static const int dqzeros[4] = {0,0,0,0}, dq16384i[4] = {16384,16384,16384,16384};

#ifdef _MSC_VER

__declspec(naked) static void glsl_shader4_cone (int *ixs, int iy, float *fdeps, int *icols)
{
	_asm
	{
		mov eax, [esp+4] ;ixs           ;xmm6 = qvx = (float)ixs[]-gview.hx
		cvtdq2ps xmm6, [eax]
		subps xmm6, dqhx

		cvtsi2ss xmm7, [esp+8] ;iy      ;xmm7 = qvy = (float)iy-gview.hy;
		subss xmm7, gview.hy
		shufps xmm7, xmm7, 0

		movaps xmm0, dqkxx     ;xmm0 = Za = (vx*kxx + vy*kxy + kxz)*vx + (vy*kyy + kyz)*vy + kzz;
		movaps xmm1, dqkxy
		mulps xmm0, xmm6
		mulps xmm1, xmm7
		addps xmm0, dqkxz
		addps xmm0, xmm1
		mulps xmm0, xmm6
		movaps xmm1, dqkyy
		mulps xmm1, xmm7
		addps xmm1, dqkyz
		mulps xmm1, xmm7
		addps xmm0, xmm1
		addps xmm0, dqkzz

		movaps xmm5, dqkx      ;xmm5 = Zb = vx*gkx + vy*gky + gkz;
		mulps xmm5, xmm6
		movaps xmm1, dqky
		mulps xmm1, xmm7
		addps xmm5, dqkz
		addps xmm5, xmm1

		movaps xmm3, xmm5      ;xmm5 = t = (sqrt(max(Zb*Zb - Za*oZc,0.0))*gsgn - Zb)/Za;
		mulps xmm5, xmm5
		rcpps xmm4, xmm0
		mulps xmm0, dqoZc
		subps xmm5, xmm0
		maxps xmm5, dqzeros
#if 0
		rsqrtps xmm5, xmm5     ;faster but causes horrible NPIXx1 pixel artifacts in ellipse arc :/
		rcpps xmm5, xmm5
#else
		sqrtps xmm5, xmm5
#endif
		xorps xmm5, dqsgn
		subps xmm5, xmm3
		mulps xmm5, xmm4

		movaps xmm0, xmm6      ;hit.x = vx*t
		movaps xmm1, xmm7      ;hit.y = vy*t
		movaps xmm2, dqhz      ;hit.z = vz*t
		mulps xmm0, xmm5
		mulps xmm1, xmm5
		mulps xmm2, xmm5
		mov eax, [esp+12] ;fdeps         ;fdeps[] = gview.hz*t;
		movaps [eax], xmm2

		movaps xmm4, dqnp10x   ;xmm4 = v10 = vx*np10.x + vy*np10.y + vznp10z;
		mulps xmm4, xmm6
		movaps xmm3, dqnp10y
		mulps xmm3, xmm7
		addps xmm4, dqvznp10z
		addps xmm4, xmm3

		mulps xmm5, xmm4       ;xmm5 = g = (v10*t - ok2)*rk0;
		subps xmm5, dqok2
		mulps xmm5, dqrk0

			;normx[] = (vx*t - (np10.x*g + np0.x))*f + dir.x;
			;normy[] = (vy*t - (np10.y*g + np0.y))*f + dir.y;
			;normz[] = (vz*t - (np10.z*g + np0.z))*f + dir.z;
		movaps xmm3, dqnp10x
		mulps xmm3, xmm5
		addps xmm3, dqnp0x
		subps xmm0, xmm3
		movaps xmm3, dqnp10y
		mulps xmm3, xmm5
		addps xmm3, dqnp0y
		subps xmm1, xmm3
		movaps xmm3, dqnp10z
		mulps xmm3, xmm5
		addps xmm3, dqnp0z
		subps xmm2, xmm3

		mulps xmm5, dqdr       ;xmm5 = f = 1.0 / (gdr*g + r0);
		addps xmm5, dqr0
		rcpps xmm5, xmm5

		mulps xmm0, xmm5
		mulps xmm1, xmm5
		mulps xmm2, xmm5
		addps xmm0, dqdirx
		addps xmm1, dqdiry
		addps xmm2, dqdirz

		movaps xmm3, xmm0
		movaps xmm4, xmm1
		movaps xmm5, xmm2

		mulps xmm0, dqlig2x    ;qt[] = normx[]*glig2.x + normy[]*glig2.y + normz[]*glig2.z;
		mulps xmm1, dqlig2y
		mulps xmm2, dqlig2z
		addps xmm0, xmm1
		addps xmm2, xmm0

		cmp gusephong, 0
		jz short skipphong

		mulps xmm3, xmm6       ;qd[] = (normx[]*qvx[] + normy[]*qvy[] + normz[]*gview.hz)*qt[]*gligmul - (glig.x*qvx[] + glig.y*qvy[] + dqligzhz[]);
		mulps xmm4, xmm7
		mulps xmm5, dqhz
		addps xmm3, xmm4
		addps xmm3, xmm5
		mulps xmm3, xmm2
		mulps xmm3, dqligmul
		movaps xmm0, dqligx
		movaps xmm1, dqligy
		mulps xmm0, xmm6
		mulps xmm1, xmm7
		addps xmm0, dqligzhz
		addps xmm0, xmm1
		subps xmm3, xmm0

		mulps xmm6, xmm6       ;xmm0 = v2 = vx*vx + vy*vy + vzvz;
		mulps xmm7, xmm7
		addps xmm6, dqvzvz
		addps xmm6, xmm7
		rcpps xmm0, xmm6       ;xmm0 = qrv2 = 1.0/v2;

		movaps xmm1, xmm3      ;xmm1 = oqd[] = qd[];
		mulps xmm3, xmm3       ;qd[] = qd[]*qd[]*qrv2[]; qd[] *= qd[]; qt[] += (oqd[]>0.0)*qd[]*qd[];
		mulps xmm3, xmm0
		mulps xmm3, xmm3
		pcmpgtd xmm1, dqzeros
		mulps xmm3, xmm3
		andps xmm3, xmm1
		addps xmm2, xmm3
skipphong:

		mulps xmm2, dq32768f   ;icols[] = cvttss2si(qd[]*32768.0)+16384;
		cvttps2dq xmm2, xmm2
		paddd xmm2, dq16384i
		mov eax, [esp+16] ;icols
		movaps [eax], xmm2

		ret
	}
}

__declspec(naked) static void glsl_shader4_sphere (int *ixs, int iy, float *fdeps, int *icols)
{
	_asm
	{
		mov eax, [esp+4] ;ixs           ;xmm6 = qvx = (float)ixs[]-gview.hx
		cvtdq2ps xmm6, [eax]
		subps xmm6, dqhx

		cvtsi2ss xmm7, [esp+8] ;iy      ;xmm7 = qvy = (float)iy-gview.hy;
		subss xmm7, gview.hy
		shufps xmm7, xmm7, 0

		movaps xmm0, xmm6      ;xmm0 = qrv2 = vx*vx + vy*vy + vzvz;
		mulps xmm0, xmm0
		movaps xmm1, xmm7
		mulps xmm1, xmm1
		addps xmm0, dqvzvz
		addps xmm0, xmm1
		rcpps xmm5, xmm0       ;xmm5 = qrv2 = 1.0/v2;

		movaps xmm2, dqp0x     ;xmm2 = Zb = vx*gp0.x + vy*gp0.y + gvzp0z;
		mulps xmm2, xmm6
		movaps xmm3, dqp0y
		mulps xmm3, xmm7
		addps xmm2, dqvzp0z
		addps xmm2, xmm3

		movaps xmm3, xmm2      ;xmm3 = qt = (Zb - sqrt(max(Zb*Zb + v2*gZc0,0))*gsgn)*rv2;
		mulps xmm0, dqZc0
		mulps xmm2, xmm2
		addps xmm2, xmm0
		maxps xmm2, dqzeros
		rsqrtps xmm2, xmm2
		rcpps xmm2, xmm2
		xorps xmm2, dqsgn
		subps xmm3, xmm2
		mulps xmm3, xmm5

		movaps xmm0, xmm6      ;hit.x = vx*t
		movaps xmm1, xmm7      ;hit.y = vy*t
		movaps xmm2, dqhz      ;hit.z = vz*t
		mulps xmm0, xmm3
		mulps xmm1, xmm3
		mulps xmm2, xmm3
		mov eax, [esp+12] ;fdeps         ;fdeps[] = gview.hz*t;
		movaps [eax], xmm2

		subps xmm0, dqp0x      ;xmm0 = normx[] = (vx*t - gp0.x)*grr0;
		subps xmm1, dqp0y      ;xmm1 = normy[] = (vy*t - gp0.y)*grr0;
		subps xmm2, dqp0z      ;xmm2 = normz[] = (vz*t - gp0.z)*grr0;
		mulps xmm0, dqrr0
		mulps xmm1, dqrr0
		mulps xmm2, dqrr0

		movaps xmm3, xmm0      ;xmm3 = qt[] = normx[]*glig2.x + normy[]*glig2.y + normz[]*glig2.z;
		movaps xmm4, xmm1
		mulps xmm3, dqlig2x
		mulps xmm4, dqlig2y
		addps xmm3, xmm4
		movaps xmm4, xmm2
		mulps xmm4, dqlig2z
		addps xmm3, xmm4

		cmp gusephong, 0
		jz short skipphong

		mulps xmm0, xmm6       ;qd[] = (normx[]*qvx[] + normy[]*qvy[] + normz[]*gview.hz)*qt[]*gligmul - (glig.x*qvx[] + glig.y*qvy[] + dqligzhz[]);
		mulps xmm1, xmm7
		mulps xmm2, dqhz
		addps xmm0, xmm1
		addps xmm0, xmm2
		mulps xmm0, xmm3
		mulps xmm0, dqligmul
		mulps xmm6, dqligx
		mulps xmm7, dqligy
		subps xmm0, dqligzhz
		addps xmm6, xmm7
		subps xmm0, xmm6

		movaps xmm4, xmm0      ;xmm1 = oqd[] = qd[];
		mulps xmm0, xmm0       ;qd[] = qd[]*qd[]*qrv2[]; qd[] *= qd[]; qt[] += (oqd[]>0.0)*qd[]*qd[];
		mulps xmm0, xmm5
		mulps xmm0, xmm0
		pcmpgtd xmm4, dqzeros
		mulps xmm0, xmm0
		andps xmm0, xmm4
		addps xmm3, xmm0
skipphong:

		mulps xmm3, dq32768f   ;icols[] = cvttss2si(qd[]*32768.0)+16384;
		cvttps2dq xmm3, xmm3
		paddd xmm3, dq16384i
		mov eax, [esp+16] ;icols
		movaps [eax], xmm3

		ret
	}
}

__declspec(naked) static void glsl_shader4_plane (int *ixs, int iy, float *fdeps, int *icols)
{
	_asm
	{
		mov eax, [esp+4] ;ixs           ;xmm6 = vx = (float)ixs[]-gview.hx
		cvtdq2ps xmm6, [eax]
		subps xmm6, dqhx

		cvtsi2ss xmm7, [esp+8] ;iy      ;xmm7 = (float)iy-gview.hy;
		subss xmm7, gview.hy
		shufps xmm7, xmm7, 0

		movaps xmm0, dqnp10x   ;fdeps[] = gok2/(vx*fnp10.x + vy*fnp10.y + vznp10z);
		mulps xmm0, xmm6
		movaps xmm1, dqnp10y
		mulps xmm1, xmm7
		addps xmm0, dqvznp10z
		addps xmm0, xmm1
		rcpps xmm0, xmm0
		mulps xmm0, dqgok2
		mov eax, [esp+12] ;fdeps
		movaps [eax], xmm0

			;for(c=0;c<4;c++)
			;{
			;   t = dqgot[c];
			;   d = (gdir.x*qvx[c] + gdir.y*qvy[c] + gdir.z*gview.hz)*t*gligmul - (glig.x*qvx[c] + glig.y*qvy[c] + glig.z*gview.hz);
			;   if (d > 0.0) { d = d*d*qrv2[c]; d *= d; t += d*d; }
			;   icols[c] = cvttss2si(t*32768.0)+32768;
			;}
		xorps xmm0, xmm0
		cmp gusephong, 0
		jz short skipphong

		movaps xmm0, xmm6
		mulps xmm0, xmm0       ;xmm2 = qrv2 = 1.0/(vx*vx + vy*vy + vzvz);
		movaps xmm1, xmm7
		mulps xmm1, xmm1
		addps xmm0, dqvzvz
		addps xmm0, xmm1
		rcpps xmm2, xmm0

		movaps xmm0, dqdirx    ;xmm0 = qd[] = (gdir.x*qvx[] + gdir.y*qvy[] + dqdirzhz[])*dqgot[]*dqligmul[] - (glig.x*qvx[] + glig.y*qvy[] + dqligzhz[]);
		movaps xmm1, dqdiry
		mulps xmm0, xmm6
		mulps xmm1, xmm7
		addps xmm0, dqdirzhz
		addps xmm0, xmm1
		movaps xmm1, dqgot
		mulps xmm1, dqligmul
		mulps xmm0, xmm1
		mulps xmm6, dqligx
		mulps xmm7, dqligy
		subps xmm0, dqligzhz
		addps xmm6, xmm7
		subps xmm0, xmm6

		movaps xmm1, xmm0      ;oqd[] = qd[]; qd[] = qd[]*qd[]*qrv2[]; qd[] *= qd[]; qd[] = (oqd[]>0.0)*qd[]*qd[] + dqgot[];
		mulps xmm0, xmm0
		mulps xmm0, xmm2       ;qrv2
		mulps xmm0, xmm0
		pcmpgtd xmm1, dqzeros
		mulps xmm0, xmm0
		andps xmm0, xmm1
skipphong:

		addps xmm0, dqgot      ;icols[] = cvttss2si(qd[]*32768.0)+16384;
		mulps xmm0, dq32768f
		cvttps2dq xmm0, xmm0
		paddd xmm0, dq16384i
		mov eax, [esp+16] ;icols
		movaps [eax], xmm0

		ret
	}
}

#else
#ifdef __GNUC__

static void glsl_shader4_cone (int *ixs, int iy, float *fdeps, int *icols)
{
	float rv2, vx, vy, normx, normy, normz, d, f, g, t, Za, Zb, v10, v2;
	int c;

	vy = (float)iy-gview.hy;
	for(c=0;c<4;c++)
	{
		vx = (float)ixs[c]-gview.hx;

		rv2 = 1.f/(vx*vx + vy*vy + vzvz);
		v10 = vx*fnp10.x + vy*fnp10.y + vznp10z;

		Za = (vx*kxx + vy*kxy + kxz)*vx + (vy*kyy + kyz)*vy + kzz;
		Zb = vx*gkx + vy*gky + gkz;
		t = (sqrt(max(Zb*Zb - Za*oZc,0.f))*gsgn - Zb)/Za; g = (v10*t - ok2)*rk0;
		f = 1.f / (gdr*g + r0); fdeps[c] = gview.hz*t;
		normx = (vx*t     - (np10.x*g + np0.x))*f + dir.x;
		normy = (vy*t     - (np10.y*g + np0.y))*f + dir.y;
		normz = (fdeps[c] - (np10.z*g + np0.z))*f + dir.z;
		t = normx*glig2.x + normy*glig2.y + normz*glig2.z;
		if (gusephong)
		{
			d = (normx*vx + normy*vy + normz*gview.hz)*t*gligmul - (glig.x*vx + glig.y*vy + glig.z*gview.hz);
			if (d > 0.0) { d = d*d*rv2; d *= d; t += d*d; }
		}
		icols[c] = cvttss2si(t*32768.0)+16384;
	}
}

static void glsl_shader4_sphere (int *ixs, int iy, float *fdeps, int *icols)
{
	float d, t, rv2, vx, vy, normx, normy, normz, Zb, v2;
	int c;

	vy = (float)iy-gview.hy;
	for(c=0;c<4;c++)
	{
		vx = (float)ixs[c]-gview.hx;

		v2 = vx*vx + vy*vy + vzvz; rv2 = 1.0/v2;
		Zb = vx*gp0.x + vy*gp0.y + gvzp0z;
		t = (Zb - sqrt(max(Zb*Zb + v2*gZc0,0.0))*gsgn)*rv2;
		fdeps[c]  = gview.hz*t;
		normx = (vx*t     - gp0.x)*grr0;
		normy = (vy*t     - gp0.y)*grr0;
		normz = (fdeps[c] - gp0.z)*grr0;
		t = normx*glig2.x + normy*glig2.y + normz*glig2.z;
		if (gusephong)
		{
			d = (normx*vx + normy*vy + normz*gview.hz)*t*gligmul - (glig.x*vx + glig.y*vy + glig.z*gview.hz);
			if (d > 0.0) { d = d*d*rv2; d *= d; t += d*d; }
		}
		icols[c] = cvttss2si(t*32768.0)+16384;
	}
}

static void glsl_shader4_plane (int *ixs, int iy, float *fdeps, int *icols)
{
	asm volatile
	(
		".intel_syntax noprefix\n\t"

		"cvtdq2ps xmm6, [%0]\n\t" //ixs        xmm6 = vx = (float)ixs[]-gview.hx
		"subps xmm6, %[dqhx]\n\t"

		"cvtsi2ss xmm7, [%1]\n\t" //iy      xmm7 = (float)iy-gview.hy
		"subss xmm7, %[gviewhy]\n\t"
		"shufps xmm7, xmm7, 0\n\t"

		"movaps xmm0, %[dqnp10x]\n\t"   //fdeps[] = gok2/(vx*fnp10.x + vy*fnp10.y + vznp10z);
		"mulps xmm0, xmm6\n\t"
		"movaps xmm1, %[dqnp10y]\n\t"
		"mulps xmm1, xmm7\n\t"
		"addps xmm0, %[dqvznp10z]\n\t"
		"addps xmm0, xmm1\n\t"
		"rcpps xmm0, xmm0\n\t"
		"mulps xmm0, %[dqgok2]\n\t"
		"movaps [%2], xmm0\n\t" //fdeps

			//for(c=0;c<4;c++)
			//{
			//   t = dqgot[c];
			//   d = (gdir.x*qvx[c] + gdir.y*qvy[c] + gdir.z*gview.hz)*t*gligmul - (glig.x*qvx[c] + glig.y*qvy[c] + glig.z*gview.hz);
			//   if (d > 0.0) { d = d*d*qrv2[c]; d *= d; t += d*d; }
			//   icols[c] = cvttss2si(t*32768.0)+32768;
			//}
		"xorps xmm0, xmm0\n\t"
		"cmp %[gusephong], 0\n\t"
		"jz short skipphong\n\t"

		"movaps xmm0, xmm6\n\t"
		"mulps xmm0, xmm0\n\t" //xmm2 = qrv2 = 1.0/(vx*vx + vy*vy + vzvz);
		"movaps xmm1, xmm7\n\t"
		"mulps xmm1, xmm1\n\t"
		"addps xmm0, %[dqvzvz]\n\t"
		"addps xmm0, xmm1\n\t"
		"rcpps xmm2, xmm0\n\t"

		"movaps xmm0, %[dqdirx]\n\t" //xmm0 = qd[] = (gdir.x*qvx[] + gdir.y*qvy[] + dqdirzhz[])*dqgot[]*dqligmul[] - (glig.x*qvx[] + glig.y*qvy[] + dqligzhz[]);
		"movaps xmm1, %[dqdiry]\n\t"
		"mulps xmm0, xmm6\n\t"
		"mulps xmm1, xmm7\n\t"
		"addps xmm0, %[dqdirzhz]\n\t"
		"addps xmm0, xmm1\n\t"
		"movaps xmm1, %[dqgot]\n\t"
		"mulps xmm1, %[dqligmul]\n\t"
		"mulps xmm0, xmm1\n\t"
		"mulps xmm6, %[dqligx]\n\t"
		"mulps xmm7, %[dqligy]\n\t"
		"subps xmm0, %[dqligzhz]\n\t"
		"addps xmm6, xmm7\n\t"
		"subps xmm0, xmm6\n\t"

		"movaps xmm1, xmm0\n\t" //oqd[] = qd[]; qd[] = qd[]*qd[]*qrv2[]; qd[] *= qd[]; qd[] = (oqd[]>0.0)*qd[]*qd[] + dqgot[];
		"mulps xmm0, xmm0\n\t"
		"mulps xmm0, xmm2\n\t" //qrv2
		"mulps xmm0, xmm0\n\t"
		"pcmpgtd xmm1, %[dqzeros]\n\t"
		"mulps xmm0, xmm0\n\t"
		"andps xmm0, xmm1\n\t"
"skipphong:\n\t"

		"addps xmm0, %[dqgot]\n\t" //icols[] = cvttss2si(qd[]*32768.0)+16384;
		"mulps xmm0, %[dq32768f]\n\t"
		"cvttps2dq xmm0, xmm0\n\t"
		"paddd xmm0, %[dq16384i]\n\t"
		"movaps [%3], xmm0\n\t" //icols

		".att_syntax prefix\n\t"
		:
		: "r" (ixs), "g" (iy), "r" (fdeps), "r" (icols)
		  ,[dqhx]      "m" (dqhx)
		  ,[gviewhy]   "m" (gview.hy)
		  ,[dqnp10x]   "m" (dqnp10x)
		  ,[dqnp10y]   "m" (dqnp10y)
		  ,[dqvznp10z] "m" (dqvznp10z)
		  ,[dqgok2]    "m" (dqgok2)
		  ,[gusephong] "m" (gusephong)
		  ,[dqvzvz]    "m" (dqvzvz)
		  ,[dqdirx]    "m" (dqdirx)
		  ,[dqdiry]    "m" (dqdiry)
		  ,[dqdirzhz]  "m" (dqdirzhz)
		  ,[dqgot]     "m" (dqgot)
		  ,[dqligmul]  "m" (dqligmul)
		  ,[dqligx]    "m" (dqligx)
		  ,[dqligy]    "m" (dqligy)
		  ,[dqligzhz]  "m" (dqligzhz)
		  ,[dqzeros]   "m" (dqzeros)
		  ,[dq32768f]  "m" (dq32768f)
		  ,[dq16384i]  "m" (dq16384i)
		: "memory"
	);
}

#else

static void glsl_shader4_cone (int *ixs, int iy, float *fdeps, int *icols)
{
	float rv2, vx, vy, normx, normy, normz, d, f, g, t, Za, Zb, v10, v2;
	int c;

	vy = (float)iy-gview.hy;
	for(c=0;c<4;c++)
	{
		vx = (float)ixs[c]-gview.hx;

		rv2 = 1.f/(vx*vx + vy*vy + vzvz);
		v10 = vx*fnp10.x + vy*fnp10.y + vznp10z;

		Za = (vx*kxx + vy*kxy + kxz)*vx + (vy*kyy + kyz)*vy + kzz;
		Zb = vx*gkx + vy*gky + gkz;
		t = (sqrt(max(Zb*Zb - Za*oZc,0.f))*gsgn - Zb)/Za; g = (v10*t - ok2)*rk0;
		f = 1.f / (gdr*g + r0); fdeps[c] = gview.hz*t;
		normx = (vx*t     - (np10.x*g + np0.x))*f + dir.x;
		normy = (vy*t     - (np10.y*g + np0.y))*f + dir.y;
		normz = (fdeps[c] - (np10.z*g + np0.z))*f + dir.z;
		t = normx*glig2.x + normy*glig2.y + normz*glig2.z;
		if (gusephong)
		{
			d = (normx*vx + normy*vy + normz*gview.hz)*t*gligmul - (glig.x*vx + glig.y*vy + glig.z*gview.hz);
			if (d > 0.0) { d = d*d*rv2; d *= d; t += d*d; }
		}
		icols[c] = cvttss2si(t*32768.0)+16384;
	}
}

static void glsl_shader4_sphere (int *ixs, int iy, float *fdeps, int *icols)
{
	float d, t, rv2, vx, vy, normx, normy, normz, Zb, v2;
	int c;

	vy = (float)iy-gview.hy;
	for(c=0;c<4;c++)
	{
		vx = (float)ixs[c]-gview.hx;

		v2 = vx*vx + vy*vy + vzvz; rv2 = 1.0/v2;
		Zb = vx*gp0.x + vy*gp0.y + gvzp0z;
		t = (Zb - sqrt(max(Zb*Zb + v2*gZc0,0.0))*gsgn)*rv2;
		fdeps[c]  = gview.hz*t;
		normx = (vx*t     - gp0.x)*grr0;
		normy = (vy*t     - gp0.y)*grr0;
		normz = (fdeps[c] - gp0.z)*grr0;
		t = normx*glig2.x + normy*glig2.y + normz*glig2.z;
		if (gusephong)
		{
			d = (normx*vx + normy*vy + normz*gview.hz)*t*gligmul - (glig.x*vx + glig.y*vy + glig.z*gview.hz);
			if (d > 0.0) { d = d*d*rv2; d *= d; t += d*d; }
		}
		icols[c] = cvttss2si(t*32768.0)+16384;
	}
}

static void glsl_shader4_plane (int *ixs, int iy, float *fdeps, int *icols)
{
	float d, t, ot, rv2, vx, vy;
	int c;

	vy = (float)iy-gview.hy;
	for(c=0;c<4;c++)
	{
		vx = (float)ixs[c]-gview.hx;

		rv2 = 1.0/(vx*vx + vy*vy + vzvz);
		fdeps[c] = gok2/(vx*fnp10.x + vy*fnp10.y + vznp10z);

		t = gdir.x*glig2.x + gdir.y*glig2.y + gdir.z*glig2.z;
		if (gusephong)
		{
			d = (gdir.x*vx + gdir.y*vy + gdir.z*gview.hz)*t*gligmul - (glig.x*vx + glig.y*vy + glig.z*gview.hz);
			if (d > 0.0) { d = d*d*rv2; d *= d; t += d*d; }
		}
		icols[c] = cvttss2si(t*32768.0)+16384;
	}
}

#endif
#endif

static void rast_cpy (rast_t *d, rast_t *s)
{
	d->ymin = s->ymin;
	d->ymax = s->ymax; if (s->ymin >= s->ymax) return;
	memcpy(&d->most[s->ymin][0],&s->most[s->ymin][0],(s->ymax - s->ymin)*sizeof(int)*2);
}

static void rast_or (rast_t *d, rast_t *s)
{
	int y, y0, y1, y2, y3;

	if (s->ymin >= s->ymax) return;
	if (d->ymin >= d->ymax) { rast_cpy(d,s); return; }
	y0 = min(s->ymin,d->ymin); y1 = max(s->ymin,d->ymin);
	y2 = min(s->ymax,d->ymax); y3 = max(s->ymax,d->ymax);
	if ((s->ymin < d->ymin) && (y0 < y1)) { memcpy(&d->most[y0][0],&s->most[y0][0],(y1-y0)*sizeof(int)*2); }
	for(y=y1;y<y2;y++)
	{
		if (d->most[y][0] >= d->most[y][1]) //if dest invalid, copy src
		{
			d->most[y][0] = s->most[y][0];
			d->most[y][1] = s->most[y][1];
		}
		else
		{
			d->most[y][0] = min(d->most[y][0],s->most[y][0]);
			d->most[y][1] = max(d->most[y][1],s->most[y][1]);
		}
	}
	if ((s->ymax > d->ymax) && (y2 < y3)) { memcpy(&d->most[y2][0],&s->most[y2][0],(y3-y2)*sizeof(int)*2); }
	d->ymin = y0;
	d->ymax = y3;
}

static void rast_and (rast_t *d, rast_t *s)
{
	int y, y0, y1, ny0, ny1;

	y0 = max(s->ymin,d->ymin);
	y1 = min(s->ymax,d->ymax);
	ny0 = 0x7fffffff; ny1 = 0;
	for(y=y0;y<y1;y++)
	{
		d->most[y][0] = max(d->most[y][0],s->most[y][0]);
		d->most[y][1] = min(d->most[y][1],s->most[y][1]);
		if (d->most[y][0] < d->most[y][1]) { if (ny0 == 0x7fffffff) ny0 = y; ny1 = y+1; }
	}
	d->ymin = ny0;
	d->ymax = ny1;
}

	//This must be final step
static void rast_sub (rast_t *d0, rast_t *d1, rast_t *s0, rast_t *s1)
{
	int x0, x1, x2, x3, y, y0, y1, y2, y3;

	y0 = s0->ymin; y1 = s0->ymax;
	y2 = s1->ymin; y3 = s1->ymax;
	d0->ymin = 0x7fffffff; d0->ymax = 0;
	for(y=y0;y<y1;y++)
	{
		x0 = s0->most[y][0]; x1 = s0->most[y][1];
		if ((y >= y2) && (y < y3)) { x2 = s1->most[y][0]; x3 = s1->most[y][1]; }
									 else { x2 = 0x7fffffff;     x3 = 0;              }

			//     0  1
			//   --+--+--
			//A  2 |  | 3
			//B  2 | 3|
			//C  23|  |
			//D    |2 | 3
			//E    |23|
			//F    |  |23
			//G     32

			  if ((x2 >= x1) || (x3 <= x0) || (x2 >= x3)) { d0->most[y][0] =         x0; d0->most[y][1] = x1; d1->most[y][0] = 0x7fffffff; d1->most[y][1] =  0; }           //C,F,G
		else if ((x2 <= x0) && (x3 >= x1))               { d0->most[y][0] = 0x7fffffff; d0->most[y][1] =  0; d1->most[y][0] = 0x7fffffff; d1->most[y][1] =  0; continue; } //A
		else if ((x2 >  x0) && (x3 <  x1))               { d0->most[y][0] =         x0; d0->most[y][1] = x2; d1->most[y][0] =         x3; d1->most[y][1] = x1; }           //E
		else if ((x2 <= x0)              )               { d0->most[y][0] =         x3; d0->most[y][1] = x1; d1->most[y][0] = 0x7fffffff; d1->most[y][1] =  0; }           //B
		else                                             { d0->most[y][0] =         x0; d0->most[y][1] = x2; d1->most[y][0] = 0x7fffffff; d1->most[y][1] =  0; }           //D
		if (d0->ymin == 0x7fffffff) d0->ymin = y;
		d0->ymax = y+1;
	}
	d1->ymin = d0->ymin; d1->ymax = d0->ymax;
}

static void draw_dohlin (int iy, void *_)
{
	ALIGN(16) float fdeps[(MAXXDIM>>LNPIX)+4];
	ALIGN(16) int icols[(MAXXDIM>>LNPIX)+4], ixs[(MAXXDIM>>LNPIX)+4];
	float *zptr;
	int i, ix0, ix1, ix, ixe, ixsn, *cptr;

	ix0 = max(((int *)_)[0],    0);
	ix1 = min(((int *)_)[1],gdd.x); if (ix0 >= ix1) return;

	cptr = (int *)(gdd.p*iy + gdd.f);
	zptr = (float *)(((INT_PTR)cptr) + zbufoff);

#if 1
	ixsn = 0; for(ix=ix0,ixe=ix1-1;ix<ixe;ix+=NPIX) { ixs[ixsn] = ix; ixsn++; }
	ixs[ixsn] = ixe; ixsn++;
#else
		//make most of shader4 by rounding ixsn up to next multiple of 4: BUGGY!
	ixsn = ((ix1-ix0+(NPIX*2-2))>>LNPIX); ixsn = (ixsn+(NPIX-1))&(-NPIX);
	int k, m; k = (ix0<<16); m = ((ix1-1-ix0)<<16)/(ixsn-1);
	for(i=0;i<ixsn-1;i++,k+=m) { ixs[i] = (k>>16); }
	ixs[ixsn-1] = ix1-1;
#endif
	for(i=0;i<ixsn;i+=4) glsl_shader4_func(&ixs[i],iy,&fdeps[i],&icols[i]);
	ixs[ixsn] = ix1; //hack to force end

#ifndef _MSC_VER
	float fdep, fdepi;
	int icol, icoli;

	icol = icols[0]; fdep = fdeps[0]; ixsn = 1;
	for(ix=ix0;ix<ix1;ix=ixe)
	{
		ixe = ixs[ixsn]; i = ixe-ix;
		icoli = ((icols[ixsn]-icol)*ireciplo[i])>>12;
		fdepi = (fdeps[ixsn]-fdep)*freciplo[i];
		ixsn++;
		for(i=ix;i<ixe;i++,icol+=icoli,fdep+=fdepi)
		{
			if (fdep >= zptr[i]) continue;
			//if (i != ix) continue;
			zptr[i] = fdep;
			cptr[i] = (min((rr*icol)>>14,255)<<16) + (min((gg*icol)>>14,255)<<8) + min((bb*icol)>>14,255);
		}
	}
#else
	_asm
	{
		push esi
		push edi

			;eax:cptr   xmm0:fdep
			;ebx:       xmm1:icol
			;ecx:zptr   xmm2:fdepi
			;edx:temp   xmm3:icoli
			;esi:ix     xmm4:
			;edi:ixsn   xmm5:
			;ebp:       xmm6:
			;esp:       xmm7:temp

		mov esi, ix0
		mov edi, 1
		movss xmm0, fdeps[0]
		movss xmm1, icols[0]
		mov eax, cptr
		mov ecx, zptr
topit:   mov edx, ixs[edi*4]
			sub edx, esi
			jle short endit
				movss xmm3, icols[edi*4]
				psubd xmm3, xmm1
				movss xmm7, ireciplo[edx*4]
				pmuludq xmm3, xmm7
				psrad xmm3, 12

				movss xmm2, fdeps[edi*4]
				subss xmm2, xmm0
				mulss xmm2, freciplo[edx*4]
begit:         ucomiss xmm0, [ecx+esi*4]
					ja short skpit
						movss [ecx+esi*4], xmm0
						pshuflw xmm7, xmm1, 0x00
						pmulhuw xmm7, dqcolmul
						packuswb xmm7, xmm7
						movss [eax+esi*4], xmm7
skpit:         addss xmm0, xmm2
					paddd xmm1, xmm3
					add esi, 1
					cmp esi, ixs[edi*4]
					jl short begit
			add edi, 1
			cmp esi, ix1
			jl short topit
endit:pop edi
		pop esi
	}
#endif
}

typedef struct { double con[6], cx, cy, cz; } dsdh_t;
static void drawsph_dohlin (int iy, void *_)
{
	double d, insqr;
	dsdh_t *dsdh;
	int sx[2];

	dsdh = (dsdh_t *)_;

	insqr = (dsdh->con[2]*iy + dsdh->con[4])*iy + dsdh->con[5]; if (insqr <= 0.0) return;
	insqr = sqrt(insqr);
	d = dsdh->con[1]*iy + dsdh->con[3];
	if (dsdh->con[0] < 0.0)
	{
		sx[0] = cvttss2si((float)(d - insqr))+1;
		sx[1] = cvttss2si((float)(d + insqr))+1;
		if ((d-gview.hx)*dsdh->cx + (iy-gview.hy)*dsdh->cy + gview.hz*dsdh->cz < 0.0) return;
	} //0  sx[1]  sx[0]  gdd.x
	else if (dsdh->cx > 0.0) //(sx[0]-gview.hx)*dsdh->cx + (iy-gview.hy)*dsdh->cy + gview.hz*dsdh->cz > 0.0)
		  {            sx[0] = cvttss2si((float)(d + insqr))+1; sx[1] = gdd.x; }
	else { sx[0] = 0; sx[1] = cvttss2si((float)(d - insqr))+1;                }

	draw_dohlin(iy,&sx[0]);
}
static void drawsph_dohlin_fullscreen (int iy, void *_) { int sx[2]; sx[0] = 0; sx[1] = gdd.x; draw_dohlin(iy,&sx[0]); }
void drawsph (double cx, double cy, double cz, double rad, int curcol, double shadefac, int flags)
{
	double g, cxcx, cycy, Za, Zb, Zc, rZa, insqr;
	int i, sy0, sy1, insph;
	dsdh_t dsdh;

	Za = cx-ipos.x; Zb = cy-ipos.y; Zc = cz-ipos.z;
	cz = Za*ifor.x + Zb*ifor.y + Zc*ifor.z; if (cz+rad <= 0.0) return;
	cx = Za*irig.x + Zb*irig.y + Zc*irig.z;
	cy = Za*idow.x + Zb*idow.y + Zc*idow.z;

	if (rad >= 0.0) gusephong = 1; else { gusephong = 0; rad = -rad; }

		//Frustum early-out
	if ((cz*gview.sphclip[0] + rad*gview.sphclip[1] < fabs(cx)*gview.hz) ||
		 (cz*gview.sphclip[2] + rad*gview.sphclip[3] < fabs(cy)*gview.hz)) return;

	g = shadefac*(-1.0/128.0); glig2.x = (float)(glig.x*g); glig2.y = (float)(glig.y*g); glig2.z = (float)(glig.z*g);
	gligmul = -256.f/(float)shadefac;

		//3D Sphere projection (13 *'s)
		//Input: rad:rad, center:cx,cy,cz
	cxcx = cx*cx; cycy = cy*cy; g = rad*rad - cxcx - cycy - cz*cz;
	if (g >= 0.f)
	{
		if (!(flags&(DRAWCONE_CULL_FRONT|DRAWCONE_CULL_NONE))) return;
		sy0 = 0; sy1 = gdd.y; insph = 1;
	}
	else
	{
		dsdh.con[0] = g + cxcx; if (dsdh.con[0] == 0.0) return;
		dsdh.con[1] = cx*cy; dsdh.con[1] += dsdh.con[1];
		dsdh.con[2] = g + cycy;
		dsdh.con[5] = gview.hx*cx + gview.hy*cy - gview.hz*cz;
		dsdh.con[3] = -cx*dsdh.con[5] - gview.hx*g; dsdh.con[3] += dsdh.con[3];
		dsdh.con[4] = -cy*dsdh.con[5] - gview.hy*g; dsdh.con[4] += dsdh.con[4];
		dsdh.con[5] = dsdh.con[5]*dsdh.con[5] + g*gview.gds;

		Za = dsdh.con[1]*dsdh.con[1] - dsdh.con[0]*dsdh.con[2]*4.0;
		Zb = dsdh.con[1]*dsdh.con[3] - dsdh.con[0]*dsdh.con[4]*2.0;
		Zc = dsdh.con[3]*dsdh.con[3] - dsdh.con[0]*dsdh.con[5]*4.0;
		if (Za < 0.0)
		{
			insqr = sqrt(max(Zb*Zb - Za*Zc,0.0)); rZa = 1.0/Za;
			sy0 = cvttss2si((float)(max((-Zb + insqr)*rZa  ,           -1.0 )))+1;
			sy1 = cvttss2si((float)(min((-Zb - insqr)*rZa+1,(float)(gdd.y-1))))+1; if (sy0 >= sy1) return;
		} else { sy0 = 0; sy1 = gdd.y; }

		rZa = 1.0/dsdh.con[0];
		g = rZa*-.5; dsdh.con[1] *= g; dsdh.con[3] *= g;
		dsdh.con[2] = dsdh.con[1]*dsdh.con[1]     - dsdh.con[2]*rZa;
		dsdh.con[4] = dsdh.con[1]*dsdh.con[3]*2.0 - dsdh.con[4]*rZa;
		dsdh.con[5] = dsdh.con[3]*dsdh.con[3]     - dsdh.con[5]*rZa;
		insph = 0;
	}

	if (flags&DRAWCONE_NOPHONG) gusephong = 0;
	gp0.x = (float)cx; gp0.y = (float)cy; gp0.z = (float)cz; r0 = (float)rad;
	gZc0 = r0*r0 - (gp0.x*gp0.x + gp0.y*gp0.y + gp0.z*gp0.z);
	grr0 = 1.f/r0;
	gvzp0z = gp0.z*gview.hz;
	if ((flags&DRAWCONE_CULL_FRONT) || ((flags&DRAWCONE_CULL_NONE) && (insph))) { gsgn = -1.f; } else { gsgn = 1.f; }


	rr = ((curcol>>16)&255);
	gg = ((curcol>> 8)&255);
	bb = ((curcol    )&255);
#ifdef _MSC_VER
	V4SI(&dqcolmul,0) = (((curcol>>8)&255)<<18) + ((curcol&255)<<2); V4SI(&dqcolmul,1) = (((curcol>>16)&255)<<2);
	for(i=4-1;i>=0;i--)
	{
		dqp0x[i] = gp0.x; dqp0y[i] = gp0.y; dqp0z[i] = gp0.z; dqvzp0z[i] = gvzp0z; dqZc0[i] = gZc0; dqrr0[i] = grr0;
		dqlig2x[i] = glig2.x; dqlig2y[i] = glig2.y; dqlig2z[i] = glig2.z; dqligmul[i] = gligmul;
		dqsgn[i] = (*(int *)&gsgn)&0x80000000;
	}
#endif

	glsl_shader4_func = glsl_shader4_sphere;
	dsdh.cx = cx; dsdh.cy = cy; dsdh.cz = cz;
	if (insph) htrun(drawsph_dohlin_fullscreen,    0,sy0,sy1,min((sy1-sy0)>>3,gnumcpu));
			else htrun(drawsph_dohlin           ,&dsdh,sy0,sy1,min((sy1-sy0)>>3,gnumcpu));
}
void drawsph (double cx, double cy, double cz, double rad, int curcol, double shadefac)
	{ drawsph(cx,cy,cz,rad,curcol,shadefac,0); }

static void drawcone_dohlin (int iy, void *_)
{
	if (imost[5].ymin < imost[5].ymax) draw_dohlin(iy,&imost[5].most[iy][0]);
	if (imost[6].ymin < imost[6].ymax) draw_dohlin(iy,&imost[6].most[iy][0]);
	if (imost[7].ymin < imost[7].ymax) draw_dohlin(iy,&imost[7].most[iy][0]);
	if (imost[8].ymin < imost[8].ymax) draw_dohlin(iy,&imost[8].most[iy][0]);
}
void drawcone (double px0, double py0, double pz0, double pr0,
					double px1, double py1, double pz1, double pr1, int curcol, double shadefac, int flags)
{
	dpoint3d pt[4], p0, p1, p10, np1;
	dpoint2d pt2[8];
	double nr0, nr1, Zc0, Zc1, rr0, rr1, vzp0z, vzp1z;
	double opr0, opr1, kx, ky, kz, xa[4], ya[4], za[4], cosang, sinang, cosang2, k0;
	double a, b, d, e, f, g, h, c, s, d2, dx, dy, dz, dr, cx, cy, cz, ax, ay, az, bx, by, bz, fx, fxinc;
	double rad, con[6], cxcx, cycy, czcz, rkx, rky, rkz, nx, ny, nz, Za, Zb, Zc, rZa, insqr;
	int i, j, k, n, pn, pn2, imin, imax, iy, iy0, iy1, isflat[2], iy2[8], isback[2], isins[2], insdrawcone, *pmost;
	int sx, sy, sy0, sy1, incone;
	#define SCISDIST 1e-6

	cx = px0-ipos.x; cy = py0-ipos.y; cz = pz0-ipos.z;
	px0 = cx*irig.x + cy*irig.y + cz*irig.z;
	py0 = cx*idow.x + cy*idow.y + cz*idow.z;
	pz0 = cx*ifor.x + cy*ifor.y + cz*ifor.z;
	cx = px1-ipos.x; cy = py1-ipos.y; cz = pz1-ipos.z;
	px1 = cx*irig.x + cy*irig.y + cz*irig.z;
	py1 = cx*idow.x + cy*idow.y + cz*idow.z;
	pz1 = cx*ifor.x + cy*ifor.y + cz*ifor.z;

	gusephong = !(flags&DRAWCONE_NOPHONG);
	g = shadefac*(-1.0/128.0); glig2.x = (float)(glig.x*g); glig2.y = (float)(glig.y*g); glig2.z = (float)(glig.z*g);
	gligmul = -256.f/(float)shadefac;

	isflat[0] = (pr0 < 0.0); pr0 = max(fabs(pr0),1e-6);
	isflat[1] = (pr1 < 0.0); pr1 = max(fabs(pr1),1e-6);
	if (flags&DRAWCONE_FLAT0) isflat[0] = 1;
	if (flags&DRAWCONE_FLAT1) isflat[1] = 1;
	if (flags&DRAWCONE_CENT)
	{
		dx = px1-px0; dy = py1-py0; dz = pz1-pz0; dr = pr1-pr0;
		f = dr/(dx*dx + dy*dy + dz*dz); g = sqrt(f*dr + 1.0);
		d = pr0*f; px0 += dx*d; py0 += dy*d; pz0 += dz*d; pr0 *= g;
		d = pr1*f; px1 += dx*d; py1 += dy*d; pz1 += dz*d; pr1 *= g;
	}

	p0.x = px0; p0.y = py0; p0.z = pz0; opr0 = pr0;
	p1.x = px1; p1.y = py1; p1.z = pz1; opr1 = pr1;

	for(i=9-1;i>=0;i--) { imost[i].ymin = 0x7fffffff; imost[i].ymax = 0; }

	pn = 0;

	dx = px1-px0; dy = py1-py0; dz = pz1-pz0; dr = pr1-pr0;
	if (dx*dx + dy*dy + dz*dz <= dr*dr) //gulped/can render single sphere
	{
		if (pr1 > pr0) { px0 = px1; py0 = py1; pz0 = pz1; pr0 = pr1; }
		f = 1.0/max(pr0,1e-6); px0 *= f; py0 *= f; pz0 *= f;
		incone = 1; goto skipmore;
	}
	incone = 0;

		//Transform cone to cylinder by normalizing endpoint radii
	f = 1.0/max(pr0,1e-6); px0 *= f; py0 *= f; pz0 *= f;
	f = 1.0/max(pr1,1e-6); px1 *= f; py1 *= f; pz1 *= f;

//Generate cone's middle quad ----------------------------------------------------------------------
	dx = px1-px0; dy = py1-py0; dz = pz1-pz0;
		//ix = dx*t + px0
		//iy = dy*t + py0
		//iz = dz*t + pz0
		//dx*ix + dy*iy + dz*iz = 0
	d2 = dx*px0 + dy*py0 + dz*pz0; d2 = (px0*px0 + py0*py0 + pz0*pz0) - d2*d2/(dx*dx + dy*dy + dz*dz);
	if (d2 <= 1.0)
	{
		if (pz0 > pz1) { px0 = px1; py0 = py1; pz0 = pz1; }
		incone = -1;
		goto skipmore;
	}
	a = 1.0/sqrt(d2); b = sqrt(1.0-a*a);

	bx = dy*pz0 - dz*py0;
	by = dz*px0 - dx*pz0;
	bz = dx*py0 - dy*px0;
	f = 1.0/sqrt(bx*bx + by*by + bz*bz); bx *= f; by *= f; bz *= f;
	ax = dy*bz - dz*by;
	ay = dz*bx - dx*bz;
	az = dx*by - dy*bx;
	f = 1.0/sqrt(ax*ax + ay*ay + az*az); ax *= f; ay *= f; az *= f;

	xa[0] = px0+ax*a-bx*b; ya[0] = py0+ay*a-by*b; za[0] = pz0+az*a-bz*b;
	xa[1] = px0+ax*a+bx*b; ya[1] = py0+ay*a+by*b; za[1] = pz0+az*a+bz*b;
	xa[2] = px1+ax*a+bx*b; ya[2] = py1+ay*a+by*b; za[2] = pz1+az*a+bz*b;
	xa[3] = px1+ax*a-bx*b; ya[3] = py1+ay*a-by*b; za[3] = pz1+az*a-bz*b;

	for(k=0;k<4;k+=2)
	{
		if (!k) { cx = px0; cy = py0; cz = pz0; }
			else { cx = px1; cy = py1; cz = pz1; }
		f = 1.0 - 1.0/(cx*cx + cy*cy + cz*cz); cx *= f; cy *= f; cz *= f;
		getperpvec(cx,cy,cz,&ax,&ay,&az,&bx,&by,&bz); f = sqrt(f);
		for(i=k+1;i>=k;i--)
		{
			c = (xa[i]-cx)*ax + (ya[i]-cy)*ay + (za[i]-cz)*az;
			s = (xa[i]-cx)*bx + (ya[i]-cy)*by + (za[i]-cz)*bz;
			g = f/sqrt(c*c + s*s); c *= g; s *= g;
			pt[pn].x = ax*c + bx*s + cx;
			pt[pn].y = ay*c + by*s + cy;
			pt[pn].z = az*c + bz*s + cz; pn++;
		}
	}

	pn2 = 0;
	for(i=pn-1,j=0;j<pn;i=j,j++)
	{
		if ((pt[i].z >= SCISDIST) != (pt[j].z >= SCISDIST))
		{
			g = (SCISDIST-pt[j].z)/(pt[i].z-pt[j].z);
			f = (float)(gdd.x>>1)/((pt[i].z-pt[j].z)*g + pt[j].z);
			pt2[pn2].x = ((pt[i].x-pt[j].x)*g + pt[j].x)*f + gview.hx;
			pt2[pn2].y = ((pt[i].y-pt[j].y)*g + pt[j].y)*f + gview.hy;
			pn2++;
		}
		if (pt[j].z >= SCISDIST)
		{
			f = gview.hz/pt[j].z;
			pt2[pn2].x = pt[j].x*f + gview.hx;
			pt2[pn2].y = pt[j].y*f + gview.hy;
			pn2++;
		}
	}
	if (pn2 < 3) return;

	for(i=pn2-1;i>=0;i--) iy2[i] = cvttss2si((float)(min(max(pt2[i].y,-1.0),(float)(gdd.y-1))))+1;

	imin = 0; imax = 0;
	for(i=pn2-1,j=0;j<pn2;i=j,j++)
	{
		if (iy2[i] == iy2[j]) continue;
		if (iy2[i] < iy2[j]) { k = 0; iy0 = iy2[i]; iy1 = iy2[j]; if (iy0 < iy2[imin]) imin = i; }
							 else { k = 1; iy0 = iy2[j]; iy1 = iy2[i]; if (iy1 > iy2[imax]) imax = i; }
		fxinc = (pt2[j].x-pt2[i].x)/(pt2[j].y-pt2[i].y); fx = (iy0-pt2[i].y)*fxinc + pt2[i].x;
		pmost = &imost[0].most[0][k];
		for(iy=iy0;iy<iy1;iy++,fx+=fxinc) pmost[iy*2] = cvttss2si((float)(min(max(fx,0.0),(float)gdd.x)))+1;
	}

	imost[0].ymin = iy2[imin]; imost[0].ymax = iy2[imax];
//--------------------------------------------------------------------------------------------------
skipmore:;

//Write globals for shader -------------------------------------------------------------------------
	r0 = (float)opr0; r1 = (float)opr1;

	p10.x = p1.x-p0.x; p10.y = p1.y-p0.y; p10.z = p1.z-p0.z; gdr = r1-r0;

		//setup raytrace cone/cylinder
	f = p10.x*p10.x + p10.y*p10.y + p10.z*p10.z;
	cosang2 = 1.0 - gdr*gdr/f;
	cosang = sqrt(cosang2); sinang = sqrt(1.0-cosang2); if (r0 < r1) sinang = -sinang; if (r0 == r1) sinang = 0.0;
	if (f != 0.0) { f = sinang/sqrt(f); dir.x = p10.x*f; dir.y = p10.y*f; dir.z = p10.z*f; } else { dir.x = 1.0; dir.y = 0.0; dir.z = 0.0; }
	nr0   =       cosang*r0; nr1   =       cosang*r1;
	np0.x = p0.x + dir.x*r0; np1.x = p1.x + dir.x*r1;
	np0.y = p0.y + dir.y*r0; np1.y = p1.y + dir.y*r1;
	np0.z = p0.z + dir.z*r0; np1.z = p1.z + dir.z*r1;
	np10.x = np1.x-np0.x; np10.y = np1.y-np0.y; np10.z = np1.z-np0.z;
	k0 = np10.x*np10.x + np10.y*np10.y + np10.z*np10.z; rk0 = 1.f/(float)k0;
	ok2 = (float)(np0.x*np10.x + np0.y*np10.y + np0.z*np10.z);
	ok3 = (float)(np1.x*np10.x + np1.y*np10.y + np1.z*np10.z);

		//h = v*f
		//dot(h-c,n)^2 = dot(h-c,h-c)*cosang2
		//kcalc "((hx-cx)*nx+(hy-cy)*ny+(hz-cz)*nz)~2-((hx-cx)~2+(hy-cy)~2+(hz-cz)~2)*cosang2"

	kxx = (float)(gdr*gdr - p10.y*p10.y - p10.z*p10.z);
	kyy = (float)(gdr*gdr - p10.x*p10.x - p10.z*p10.z);
	kzz = (float)(gdr*gdr - p10.x*p10.x - p10.y*p10.y);
	kxy = (float)(p10.x*p10.y*2.0);
	kxz = (float)(p10.x*p10.z*2.0);
	kyz = (float)(p10.y*p10.z*2.0);

	f = p10.x*p1.x + p10.y*p1.y + p10.z*p1.z - gdr*r1;
	g = p10.x*p0.x + p10.y*p0.y + p10.z*p0.z - gdr*r0;
	gkx = (float)(f*p0.x - g*p1.x);
	gky = (float)(f*p0.y - g*p1.y);
	gkz = (float)(f*p0.z - g*p1.z);

	h = 0.0;
	f = p0.x*r1 - p1.x*r0; h += f*f;
	f = p0.y*r1 - p1.y*r0; h += f*f;
	f = p0.z*r1 - p1.z*r0; h += f*f;
	f = p0.x*p1.y - p1.x*p0.y; h -= f*f;
	f = p0.y*p1.z - p1.y*p0.z; h -= f*f;
	f = p0.z*p1.x - p1.z*p0.x; h -= f*f;
	oZc = (float)h;

	if (!isflat[0])
	{
		Zc0 = r0*r0 - (p0.x*p0.x + p0.y*p0.y + p0.z*p0.z);
	} else { p0.x = 0.0; p0.y = 0.0; p0.z = 0.0; Zc0 = -1.0; }
	if (!isflat[1])
	{
		Zc1 = r1*r1 - (p1.x*p1.x + p1.y*p1.y + p1.z*p1.z);
	} else { p1.x = 0.0; p1.y = 0.0; p1.z = 0.0; Zc1 = -1.0; }

	rr0 = 1.0/r0;
	rr1 = 1.0/r1;

	fnp10.x = (float)np10.x; fnp10.y = (float)np10.y; fnp10.z = 0.f; fnp10.w = 0.f; vznp10z = (float)(np10.z*gview.hz);
	vzp0z = p0.z*gview.hz;
	vzp1z = p1.z*gview.hz;
	kxz *= gview.hz; kyz *= gview.hz; kzz *= gview.hz*gview.hz; gkz *= gview.hz;

	for(i=4-1;i>=0;i--)
	{
		dqnp0x[i] = (float)np0.x; dqnp10x[i] = (float)np10.x; dqlig2x[i] = glig2.x;
		dqnp0y[i] = (float)np0.y; dqnp10y[i] = (float)np10.y; dqlig2y[i] = glig2.y;
		dqnp0z[i] = (float)np0.z; dqnp10z[i] = (float)np10.z; dqlig2z[i] = glig2.z;
		dqvznp10z[i] = vznp10z;
		dqligmul[i] = gligmul;
		dqkxx[i] = kxx; dqkxy[i] = kxy; dqkxz[i] = kxz; dqkyy[i] = kyy; dqkyz[i] = kyz; dqkzz[i] = kzz;
		dqkx[i] = gkx; dqky[i] = gky; dqkz[i] = gkz; dqoZc[i] = oZc; dqrk0[i] = rk0;
	}

//--------------------------------------------------------------------------------------------------

	isback[0] = (np0.x*np10.x + np0.y*np10.y + np0.z*np10.z < 0.0);
	isback[1] = (np1.x*np10.x + np1.y*np10.y + np1.z*np10.z > 0.0);
	isins[0] = (p0.x*p0.x + p0.y*p0.y + p0.z*p0.z < opr0*opr0);
	isins[1] = (p1.x*p1.x + p1.y*p1.y + p1.z*p1.z < opr1*opr1);
	insdrawcone = ((incone < 0) && ((isback[0] && isback[1]) || (isins[0] && !isflat[0]) || (isins[1] && !isflat[1]))); //inside cone
	if ((insdrawcone) && (!(flags&(DRAWCONE_CULL_FRONT|DRAWCONE_CULL_NONE)))) return;

	for(i=0;i<2;i++)
	{
		if (!i) { cx = p0.x; cy = p0.y; cz = p0.z; rad = opr0; }
			else { cx = p1.x; cy = p1.y; cz = p1.z; rad = opr1; }
		if (rad <= 0.0) { continue; }

//Generate sphere rasts ----------------------------------------------------------------------------
			//3D Sphere projection (13 *'s)
			//Input: rad:rad, center:cx,cy,cz
		if (!isins[i])
		{
			cxcx = cx*cx; cycy = cy*cy; g = rad*rad - cxcx - cycy - cz*cz;
			con[0] = g + cxcx;
			con[1] = cx*cy; con[1] += con[1];
			con[2] = g + cycy;
			con[5] = gview.hx*cx + gview.hy*cy - gview.hz*cz;
			con[3] = -cx*con[5] - gview.hx*g; con[3] += con[3];
			con[4] = -cy*con[5] - gview.hy*g; con[4] += con[4];
			con[5] = con[5]*con[5] + g*gview.gds;
			if (/*(g < 0.f) &&*/ (con[0] != 0.0))
			{
				Za = con[1]*con[1] - con[0]*con[2]*4.0;
				Zb = con[1]*con[3] - con[0]*con[4]*2.0;
				Zc = con[3]*con[3] - con[0]*con[5]*4.0;
				if (Za < 0.0)
				{
					insqr = sqrt(max(Zb*Zb - Za*Zc,0.0)); rZa = 1.0/Za;
					sy0 = cvttss2si((float)max((-Zb + insqr)*rZa  ,            -1.0))+1;
					sy1 = cvttss2si((float)min((-Zb - insqr)*rZa+1,(float)(gdd.y-1)))+1;
				} else { sy0 = 0; sy1 = gdd.y; }

				con[1] *= .5; con[3] *= .5; rZa = 1.0/con[0];
				for(sy=sy0;sy<sy1;sy++)
				{
					Za = con[0];
					Zb = con[1]*sy + con[3];
					Zc = con[2]*sy*sy + con[4]*sy + con[5];
					insqr = Zb*Zb - Za*Zc; if (insqr <= 0.0) continue;
					insqr = sqrt(insqr);
					imost[i+1].most[sy][0] = cvttss2si((float)min(max((-Zb + insqr)*rZa,-1.f),(float)(gdd.x-1)))+1;
					imost[i+1].most[sy][1] = cvttss2si((float)min(max((-Zb - insqr)*rZa,-1.f),(float)(gdd.x-1)))+1;

					if (Za > 0.0)
					{
						if (cx < 0.0) imost[i+1].most[sy][0] = 0;
									else imost[i+1].most[sy][1] = gdd.x;
					}
					else if (((imost[i+1].most[sy][0]+imost[i+1].most[sy][1])*.5-gview.hx)*cx + (sy-gview.hy)*cy + gview.hz*cz < 0.0)
					{
						imost[i+1].most[sy][0] = 0x7fffffff;
						imost[i+1].most[sy][1] = 0;
						continue;
					}
					if (imost[i+1].most[sy][0] < imost[i+1].most[sy][1])
					{
						imost[i+1].ymin = min(imost[i+1].ymin,sy  );
						imost[i+1].ymax = max(imost[i+1].ymax,sy+1);
					}
				}
			}
		}
		else
		{
			imost[i+1].ymin = 0; imost[i+1].ymax = gdd.y;
			for(sy=0;sy<gdd.y;sy++) { imost[i+1].most[sy][0] = 0; imost[i+1].most[sy][1] = gdd.x; }
		}

//Generate circle rasts ----------------------------------------------------------------------------
		if (incone > 0) continue;
		if (i == 0) { cx = np0.x; cy = np0.y; cz = np0.z; rad = nr0; }
				 else { cx = np1.x; cy = np1.y; cz = np1.z; rad = nr1; }
		kx = np10.x; ky = np10.y; kz = np10.z;
		//if (isback[i]) continue; //back-face cull
		f = 1.0/sqrt(kx*kx + ky*ky + kz*kz); kx *= f; ky *= f; kz *= f;

			//3D circle projection (39 *'s)
			//Input: radius:1, center:cx,cy,cz, unit_normal:kx,ky,kz
		cxcx = cx*cx; cycy = cy*cy; czcz = cz*cz;
		rkx = rad*kx; rky = rad*ky; rkz = rad*kz;
		nx = cy*kz - cz*ky; ny = cz*kx - cx*kz; nz = cx*ky - cy*kx;
		con[0] = nx*nx + rkx*rkx - cycy - czcz;
		con[1] = (nx*ny + rkx*rky + cx*cy)*2.0;
		con[2] = ny*ny + rky*rky - cxcx - czcz;
		con[3] = (nx*nz + rkx*rkz + cx*cz)*gview.hz - con[0]*gview.hx;
		con[4] = (ny*nz + rky*rkz + cy*cz)*gview.hz - con[2]*gview.hy;
		con[3] += con[3] - con[1]*gview.hy;
		con[4] += con[4] - con[1]*gview.hx;
		con[5] = (nz*nz + rkz*rkz - cxcx - cycy)*gview.hz*gview.hz
					- con[0]*gview.hx*gview.hx - con[1]*gview.hx*gview.hy
					- con[2]*gview.hy*gview.hy - con[3]*gview.hx - con[4]*gview.hy;

		Za = con[1]*con[1] - con[0]*con[2]*4.0;
		Zb = con[1]*con[3] - con[0]*con[4]*2.0;
		Zc = con[3]*con[3] - con[0]*con[5]*4.0;
		if (Za < 0.0)
		{
			insqr = sqrt(max(Zb*Zb - Za*Zc,0.0)); rZa = 1.0/Za;
			sy0 = cvttss2si((float)max((-Zb + insqr)*rZa  ,            -1.0))+1;
			sy1 = cvttss2si((float)min((-Zb - insqr)*rZa+1,(float)(gdd.y-1)))+1;
		} else { sy0 = 0; sy1 = gdd.y; }

		b = cx*kx + cy*ky + cz*kz;
		e = rad*rad - (cx*cx + cy*cy + cz*cz);

		con[1] *= .5; con[3] *= .5; rZa = 1.0/con[0];
		for(sy=sy0;sy<sy1;sy++)
		{
			static int sxs[4];

			Za = con[0];
			Zb = con[1]*sy + con[3];
			Zc = con[2]*sy*sy + con[4]*sy + con[5];
			insqr = Zb*Zb - Za*Zc; if (insqr <= 0.0) continue;
			insqr = sqrt(insqr);

			sxs[0] = 0; n = 1;
			sxs[n] = cvttss2si((float)((-Zb + insqr)*rZa))+1; n += ((sxs[n] > 0) && (sxs[n] < gdd.x));
			sxs[n] = cvttss2si((float)((-Zb - insqr)*rZa))+1; n += ((sxs[n] > 0) && (sxs[n] < gdd.x));
			sxs[n] = gdd.x; n++;
			if ((n == 4) && (sxs[1] > sxs[2])) { j = sxs[1]; sxs[1] = sxs[2]; sxs[2] = j; }

			imost[i+3].most[sy][0] = 0x7fffffff;
			imost[i+3].most[sy][1] = 0;
			for(j=1;j<n;j++)
			{
				dx = (float)(sxs[j-1]+sxs[j]-1)*.5-gview.hx; dy = (sy-gview.hy); dz = gview.hz;

				a = dx*kx + dy*ky + dz*kz; if (a*b < 0.0) continue; //back-face cull
				c = dx*cx + dy*cy + dz*cz;
				d = dx*dx + dy*dy + dz*dz;

					//   //vars: dx,dy,dz, cx,cy,cz,rad, kx,ky,kz
					//ix = dx*t, iy = dy*t, iz = dz*t            //ray
					//(ix-cx)*kx + (iy-cy)*ky + (iz-cz)*kz = 0   //plane
					//(ix-cx)^2 + (iy-cy)^2 + (iz-cz)^2 < rad^2  //in circ
				if ((b*d - a*c*2)*b >= a*a*e) continue; //outside circ

				if (imost[i+3].most[sy][0] == 0x7fffffff) imost[i+3].most[sy][0] = sxs[j-1];
				imost[i+3].most[sy][1] = sxs[j];
			}

			if (imost[i+3].most[sy][0] < imost[i+3].most[sy][1])
			{
				imost[i+3].ymin = min(imost[i+3].ymin,sy  );
				imost[i+3].ymax = max(imost[i+3].ymax,sy+1);
			}
		}
//--------------------------------------------------------------------------------------------------
	}

	rr = ((curcol>>16)&255);
	gg = ((curcol>> 8)&255);
	bb = ((curcol    )&255);
#ifdef _MSC_VER
	V4SI(&dqcolmul,0) = (gg<<18) + (bb<<2); V4SI(&dqcolmul,1) = (rr<<2);
#endif

	if ((flags&DRAWCONE_CULL_FRONT) || ((insdrawcone) && (flags&DRAWCONE_CULL_NONE)))
	{
		gdr = -gdr; r0 = -r0; dir.x = -dir.x; dir.y = -dir.y; dir.z = -dir.z;

			//Draw cone's middle
		if (insdrawcone)
		{
			sy0 = 0; sy1 = gdd.y;
			imost[5].ymin = sy0; imost[5].ymax = sy1; imost[6].ymin = 0x7fffffff; imost[6].ymax = 0;
			for(sy=0;sy<gdd.y;sy++) { imost[5].most[sy][0] = 0; imost[5].most[sy][1] = gdd.x; }
		}
		else { rast_cpy(&imost[5],&imost[0]); }
		for(i=0;i<2;i++)
		{
			j = i^isback[1]^1;
			if (!isins[j]) rast_and(&imost[j+3],&imost[j+1]); //optional:fixes rare artifacts related to circle projection precision
			if (!isback[j])
			{
				if (!insdrawcone) { rast_or(&imost[5],&imost[j+3]); imost[6].ymin = 0x7fffffff; imost[6].ymax = 0; }
								 else { rast_and(&imost[5],&imost[j+3]); imost[6].ymin = 0x7fffffff; imost[6].ymax = 0; }
			}
			else
			{
				if (i && isback[0] && isback[1]) //both sides have holes; hack to support 2 rasts [5],[6] input per hlin
				{
					rast_sub(&imost[5],&imost[7],&imost[5],&imost[j+3]);
					rast_sub(&imost[6],&imost[8],&imost[6],&imost[j+3]);
				}
				else rast_sub(&imost[5],&imost[6],&imost[5],&imost[j+3]);
			}
		}
		if (!(flags&DRAWCONE_NOCONE))
		{
			sy0 = min(min(imost[5].ymin,imost[6].ymin),min(imost[7].ymin,imost[8].ymin)); sy0 = min(max(sy0,0),gdd.y);
			sy1 = max(max(imost[5].ymax,imost[6].ymax),max(imost[7].ymax,imost[8].ymax)); sy1 = min(max(sy1,0),gdd.y);
#ifdef STANDALONE
			if (!keystatus[0x38])
#endif
			{
			glsl_shader4_func = glsl_shader4_cone;
			gsgn = -1.f;
			for(k=4-1;k>=0;k--)
			{
				dqp0x[k] = gp0.x; dqp0y[k] = gp0.y; dqp0z[k] = gp0.z; dqvzp0z[k] = gvzp0z; dqZc0[k] = gZc0;
				dqsgn[k] = (*(int *)&gsgn)&0x80000000;
				dqdirx[k] = (float)dir.x; dqdiry[k] = (float)dir.y; dqdirz[k] = (float)dir.z;
				dqok2[k] = ok2; dqdr[k] = gdr; dqr0[k] = r0;
			}
			if (sy0 < sy1) htrun(drawcone_dohlin,0,sy0,sy1,min((sy1-sy0)>>3,gnumcpu));
			}
#ifdef STANDALONE
			else { for(k=5;k<=8;k++) for(sy=max(imost[k].ymin,0);sy<min(imost[k].ymax,gdd.y);sy++) drawhlin(&gdd,imost[k].most[sy][0],imost[k].most[sy][1],sy,0xff0000); }
#endif
			imost[7].ymin = 0x7fffffff; imost[7].ymax = 0;
			imost[8].ymin = 0x7fffffff; imost[8].ymax = 0;
		}

		gdr = -gdr; r0 = -r0; dir.x = -dir.x; dir.y = -dir.y; dir.z = -dir.z;

			//Draw cone's ends
		for(i=0;i<2;i++)
		{
			if ((!i) && (flags&DRAWCONE_NOCAP0)) continue;
			if (( i) && (flags&DRAWCONE_NOCAP1)) continue;

			if (isflat[i])
			{
				if (isback[i]) { rast_cpy(&imost[5],&imost[i+3]); imost[6].ymin = 0x7fffffff; imost[6].ymax = 0; }
							 else { continue; }
			}
			else if (incone < 0)
			{
				if (isback[i]) { rast_cpy(&imost[5],&imost[i+3]); imost[6].ymin = 0x7fffffff; imost[6].ymax = 0; }
							 else { rast_sub(&imost[5],&imost[6],&imost[i+1],&imost[i+3]); }
			}
			else
			{
				rast_cpy(&imost[5],&imost[i+1]);

				if (imost[0].ymin < imost[0].ymax)
				{
						//if ((sx-gview.hx)*Za + (sy-gview.hy)*Zb + gview.hz*Zc > 0.0) is_on_keep_side;
					Za = pt[i*2].y*pt[i*2+1].z - pt[i*2].z*pt[i*2+1].y;
					Zb = pt[i*2].z*pt[i*2+1].x - pt[i*2].x*pt[i*2+1].z;
					Zc = pt[i*2].x*pt[i*2+1].y - pt[i*2].y*pt[i*2+1].x;
					for(sy=imost[5].ymin;sy<imost[5].ymax;sy++)
					{
						sx = cvttss2si((float)min(max(gview.hx - ((sy-gview.hy)*Zb + gview.hz*Zc)/Za,-1.0),(float)gdd.x-1))+1;
						if (Za >= 0.0) imost[5].most[sy][0] = max(imost[5].most[sy][0],sx);
									 else imost[5].most[sy][1] = min(imost[5].most[sy][1],sx);
					}
				}

				if (isback[i]) { rast_or(&imost[5],&imost[i+3]); imost[6].ymin = 0x7fffffff; imost[6].ymax = 0; }
							 else { rast_sub(&imost[5],&imost[6],&imost[5],&imost[i+3]); }
			}
			sy0 = min(max(min(imost[5].ymin,imost[6].ymin),0),gdd.y);
			sy1 = min(max(max(imost[5].ymax,imost[6].ymax),0),gdd.y);
#ifdef STANDALONE
			if (!keystatus[0x38])
			{
#endif
				if (sy0 < sy1)
				{
					if (!isflat[i])
					{
						glsl_shader4_func = glsl_shader4_sphere; gsgn =-1.f;
						if (!i) { gp0.x = (float)p0.x; gp0.y = (float)p0.y; gp0.z = (float)p0.z; gvzp0z = (float)vzp0z; gZc0 = (float)Zc0; grr0 = (float)-rr0; }
							else { gp0.x = (float)p1.x; gp0.y = (float)p1.y; gp0.z = (float)p1.z; gvzp0z = (float)vzp1z; gZc0 = (float)Zc1; grr0 = (float)-rr1; }
						for(k=4-1;k>=0;k--)
						{
							dqp0x[k] = gp0.x; dqp0y[k] = gp0.y; dqp0z[k] = gp0.z; dqvzp0z[k] = gvzp0z; dqZc0[k] = gZc0; dqrr0[k] = grr0;
							dqsgn[k] = (*(int *)&gsgn)&0x80000000;
						}
					}
					else
					{
						glsl_shader4_func = glsl_shader4_plane;
						f = 1.0/sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
						if (!i) { gok2 = ok2;         }
							else { gok2 = ok3; f = -f; }
						gok2 *= gview.hz;
						for(k=4-1;k>=0;k--) dqgok2[k] = gok2;
						gdir.x = (float)(dir.x*f); gdir.y = (float)(dir.y*f); gdir.z = (float)(dir.z*f);
						for(k=4-1;k>=0;k--) { dqdirx[k] = gdir.x; dqdiry[k] = gdir.y; }
						dqdirzhz[0] = gdir.z*gview.hz; for(k=4-1;k>0;k--) dqdirzhz[k] = dqdirzhz[0];
						dqgot[0] = gdir.x*glig2.x + gdir.y*glig2.y + gdir.z*glig2.z; for(k=1;k<4;k++) dqgot[k] = dqgot[0];
					}
					htrun(drawcone_dohlin,0,sy0,sy1,min((sy1-sy0)>>3,gnumcpu));
				}
#ifdef STANDALONE
			} else { for(k=5;k<=6;k++) for(sy=max(imost[k].ymin,0);sy<min(imost[k].ymax,gdd.y);sy++) drawhlin(&gdd,imost[k].most[sy][0],imost[k].most[sy][1],sy,0x0000ff<<(i<<3)); }
#endif
		}
	}
	else
	{
			//Draw cone's middle
		rast_cpy(&imost[5],&imost[0]);
		for(i=0;i<2;i++)
		{
			j = i^isback[1];
			if (!isins[j]) rast_and(&imost[j+3],&imost[j+1]); //optional:fixes rare artifacts related to circle projection precision
			if (isback[j]) { rast_or(&imost[5],&imost[j+3]); imost[6].ymin = 0x7fffffff; imost[6].ymax = 0; }
						 else { rast_sub(&imost[5],&imost[6],&imost[5],&imost[j+3]); }
		}
		if (!(flags&DRAWCONE_NOCONE))
		{
			sy0 = min(max(min(imost[5].ymin,imost[6].ymin),0),gdd.y);
			sy1 = min(max(max(imost[5].ymax,imost[6].ymax),0),gdd.y);
#ifdef STANDALONE
			if (!keystatus[0x38])
#endif
			{
			glsl_shader4_func = glsl_shader4_cone;
			gsgn = 1.f;
			for(k=4-1;k>=0;k--)
			{
				dqp0x[k] = gp0.x; dqp0y[k] = gp0.y; dqp0z[k] = gp0.z; dqvzp0z[k] = gvzp0z; dqZc0[k] = gZc0;
				dqsgn[k] = (*(int *)&gsgn)&0x80000000;
				dqdirx[k] = (float)dir.x; dqdiry[k] = (float)dir.y; dqdirz[k] = (float)dir.z;
				dqok2[k] = ok2; dqdr[k] = gdr; dqr0[k] = r0;
			}
			if (sy0 < sy1) htrun(drawcone_dohlin,0,sy0,sy1,min((sy1-sy0)>>3,gnumcpu));
			}
#ifdef STANDALONE
			else { for(k=5;k<=6;k++) for(sy=max(imost[k].ymin,0);sy<min(imost[k].ymax,gdd.y);sy++) drawhlin(&gdd,imost[k].most[sy][0],imost[k].most[sy][1],sy,0xff0000); }
#endif
		}

			//Draw cone's ends
		for(i=0;i<2;i++)
		{
			if ((!i) && (flags&DRAWCONE_NOCAP0)) continue;
			if (( i) && (flags&DRAWCONE_NOCAP1)) continue;
			if (isflat[i])
			{
				if (isback[i]) continue;
				rast_cpy(&imost[5],&imost[i+3]); imost[6].ymin = 0x7fffffff; imost[6].ymax = 0;
			}
			else
			{
				rast_cpy(&imost[5],&imost[i+1]);

				if (imost[0].ymin < imost[0].ymax)
				{
						//if ((sx-gview.hx)*Za + (sy-gview.hy)*Zb + gview.hz*Zc > 0.0) is_on_keep_side;
					Za = pt[i*2].y*pt[i*2+1].z - pt[i*2].z*pt[i*2+1].y;
					Zb = pt[i*2].z*pt[i*2+1].x - pt[i*2].x*pt[i*2+1].z;
					Zc = pt[i*2].x*pt[i*2+1].y - pt[i*2].y*pt[i*2+1].x;
					for(sy=imost[5].ymin;sy<imost[5].ymax;sy++)
					{
						sx = cvttss2si((float)min(max(gview.hx - ((sy-gview.hy)*Zb + gview.hz*Zc)/Za,-1.0),(float)gdd.x-1))+1;
						if (Za >= 0.0) imost[5].most[sy][0] = max(imost[5].most[sy][0],sx);
									 else imost[5].most[sy][1] = min(imost[5].most[sy][1],sx);
					}
				}

				if (!isback[i]) { rast_or(&imost[5],&imost[i+3]); imost[6].ymin = 0x7fffffff; imost[6].ymax = 0; }
							  else { rast_sub(&imost[5],&imost[6],&imost[5],&imost[i+3]); }
			}
			sy0 = min(max(min(imost[5].ymin,imost[6].ymin),0),gdd.y);
			sy1 = min(max(max(imost[5].ymax,imost[6].ymax),0),gdd.y);
#ifdef STANDALONE
			if (!keystatus[0x38])
			{
#endif
				if (sy0 < sy1)
				{
					if (!isflat[i])
					{
						glsl_shader4_func = glsl_shader4_sphere; gsgn = 1.f;
						if (!i) { gp0.x = (float)p0.x; gp0.y = (float)p0.y; gp0.z = (float)p0.z; gvzp0z = (float)vzp0z; gZc0 = (float)Zc0; grr0 = (float)rr0; }
							else { gp0.x = (float)p1.x; gp0.y = (float)p1.y; gp0.z = (float)p1.z; gvzp0z = (float)vzp1z; gZc0 = (float)Zc1; grr0 = (float)rr1; }
						for(k=4-1;k>=0;k--)
						{
							dqp0x[k] = gp0.x; dqp0y[k] = gp0.y; dqp0z[k] = gp0.z; dqvzp0z[k] = gvzp0z; dqZc0[k] = gZc0; dqrr0[k] = grr0;
							dqsgn[k] = (*(int *)&gsgn)&0x80000000;
						}
					}
					else
					{
						glsl_shader4_func = glsl_shader4_plane;
						f = 1.0/sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
						if (!i) { gok2 = ok2; f =-f; }
							else { gok2 = ok3;        }
						gdir.x = (float)(dir.x*f); gdir.y = (float)(dir.y*f); gdir.z = (float)(dir.z*f);
						for(k=4-1;k>=0;k--) { dqdirx[k] = gdir.x; dqdiry[k] = gdir.y; }
						dqdirzhz[0] = gdir.z*gview.hz; for(k=4-1;k>0;k--) dqdirzhz[k] = dqdirzhz[0];
						dqgot[0] = gdir.x*glig2.x + gdir.y*glig2.y + gdir.z*glig2.z; for(k=4-1;k>0;k--) dqgot[k] = dqgot[0];
						gok2 *= gview.hz;
						for(k=4-1;k>=0;k--) dqgok2[k] = gok2;
					}
					htrun(drawcone_dohlin,0,sy0,sy1,min((sy1-sy0)>>3,gnumcpu));
				}
#ifdef STANDALONE
			} else { for(k=5;k<=6;k++) for(sy=max(imost[k].ymin,0);sy<min(imost[k].ymax,gdd.y);sy++) drawhlin(&gdd,imost[k].most[sy][0],imost[k].most[sy][1],sy,0x0000ff<<(i<<3)); }
#endif
		}
	}

#ifdef STANDALONE
	if (keystatus[0xb8])
	{
		for(k=0;k<=4;k++)
			for(sy=max(imost[k].ymin,0);sy<min(imost[k].ymax,gdd.y);sy++)
			{
				drawpix(&gdd,imost[k].most[sy][0],sy,0xff8080);
				drawpix(&gdd,imost[k].most[sy][1],sy,0x80ff80);
			}
	}
#endif
}

#ifdef STANDALONE

static HINSTANCE ghinst;
//--------------------------------------------------------------------------------------------------
#ifdef _MSC_VER
extern int kdisasm (unsigned char *ibuf, char *obuf, int bits);

static EXCEPTION_RECORD gexception_record;
static CONTEXT gexception_context;

static int exception_getaddr (LPEXCEPTION_POINTERS pxi)
{
	memcpy(&gexception_record,pxi->ExceptionRecord,sizeof(EXCEPTION_RECORD));
	memcpy(&gexception_context,pxi->ContextRecord,sizeof(CONTEXT));
	return(EXCEPTION_EXECUTE_HANDLER);
}

static void showexception (void)
{
		//EXCEPTION_RECORD:
		//   DWORD ExceptionCode, ExceptionFlags;
		//   EXCEPTION_RECORD *ExceptionRecord;
		//   PVOID ExceptionAddress;
		//   DWORD NumberParameters;
		//   ULONG_PTR ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
		//EXCEPTION_CONTEXT:
		//   DWORD ContextFlags;
		//   DWORD Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;                  //CONTEXT_DEBUG_REGISTERS (not in CONTEXT_FULL)
		//   FLOATING_SAVE_AREA FloatSave;                        //CONTEXT_FLOATING_POINT
		//   DWORD SegGs, SegFs, SegEs, SegDs;                    //CONTEXT_SEGMENTS
		//   DWORD Edi, Esi, Ebx, Edx, Ecx, Eax;                  //CONTEXT_INTEGER
		//   DWORD Ebp, Eip, SegCs, EFlags, Esp, SegSs;           //CONTEXT_CONTROL
		//   BYTE ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION]; //CONTEXT_EXTENDED_REGISTERS
	int i, j, k, i0, i1;
	unsigned char tbuf[1024];
	static char buf[65536], *optr; //WARNING: assumes n->bufalloc has enough bytes!
	optr = buf;

	{
	RECT rect;
	rect.left = 0; rect.right  = 0x7fffffff;
	rect.top  = 0; rect.bottom = 0x7fffffff;
	ClipCursor(&rect);
	ShowCursor(1);
	}

	optr += sprintf(optr,"Exception %08x",gexception_record.ExceptionCode);
	if (gexception_record.ExceptionCode)
	{
		optr += sprintf(optr,"; code @ %08x tried to ",gexception_record.ExceptionAddress);
		if (gexception_record.NumberParameters >= 2)
		{
			switch(gexception_record.ExceptionInformation[0])
			{
				case 0: optr += sprintf(optr,"read"); break;
				case 1: optr += sprintf(optr,"write"); break;
				case 8: optr += sprintf(optr,"execute"); break;
				default: break;
			}
			optr += sprintf(optr," %08x",gexception_record.ExceptionInformation[1]);
		}
		else optr += sprintf(optr,"do something bad:");
	}
	optr += sprintf(optr,"\r\n\r\n");

	optr += sprintf(optr,"   EAX: %08x     ESI: %08x\r\n",gexception_context.Eax,gexception_context.Esi);
	optr += sprintf(optr,"   EBX: %08x     EDI: %08x\r\n",gexception_context.Ebx,gexception_context.Edi);
	optr += sprintf(optr,"   ECX: %08x     EBP: %08x\r\n",gexception_context.Ecx,gexception_context.Ebp);
	optr += sprintf(optr,"   EDX: %08x     ESP: %08x\r\n",gexception_context.Edx,gexception_context.Esp);
	optr += sprintf(optr,"   EIP: %08x  EFLAGS: %08x\r\n\r\n",gexception_context.Eip,gexception_context.EFlags);
	optr += sprintf(optr," FP_CW: %08x  FP_SW: %08x  FP_TW: %08x\r\n",gexception_context.FloatSave.ControlWord,gexception_context.FloatSave.StatusWord,gexception_context.FloatSave.TagWord);
	for(i=0;i<8;i++)
	{
		optr += sprintf(optr,"  FP_%d: ",i);
		for(j=0;j<10;j++) optr += sprintf(optr,"%02x ",gexception_context.FloatSave.RegisterArea[i*10+j]);
		optr += sprintf(optr,"\r\n");
	}
	optr += sprintf(optr,"\r\n");
	for(i=0;i<8;i++)
	{
		optr += sprintf(optr,"  XMM%d: ",i);
		for(j=0;j<16;j++)
		{
			optr += sprintf(optr,"%02x ",gexception_context.ExtendedRegisters[i*16+j+160]);
			if ((j&3) == 3) optr += sprintf(optr," ");
		}
		optr += sprintf(optr,"\r\n");
	}
	optr += sprintf(optr,"\r\n");
	for(i=0;i<8;i++)
	{
		optr += sprintf(optr,"  XMM%d: ",i);
		for(j=0;j<16;j+=4) optr += sprintf(optr,"%12g ",*(float *)(((int)gexception_context.ExtendedRegisters) + i*16 + j + 160));
		optr += sprintf(optr,"\r\n");
	}
	optr += sprintf(optr,"\r\n");
	for(i=0;i<8;i++)
	{
		optr += sprintf(optr,"  XMM%d: ",i);
		for(j=0;j<16;j+=8) optr += sprintf(optr,"%25g ",*(double *)(((int)gexception_context.ExtendedRegisters) + i*16 + j + 160));
		optr += sprintf(optr,"\r\n");
	}

#if 1
	#define DISASMRANGE 64
	__try { i = 0; memcpy(tbuf,(void *)(((int)gexception_context.Eip)-DISASMRANGE*2),DISASMRANGE*3+16); }
	__except(EXCEPTION_EXECUTE_HANDLER) { i = -1; }
	if (!i)
	{
		optr += sprintf(optr,"...\r\n");
		for(i=0;i<DISASMRANGE*3;i+=j)
		{
			char tbuf2[64];
			j = kdisasm((unsigned char *)&tbuf[i],tbuf2,32);
			if (i < DISASMRANGE) continue;

			if (i == DISASMRANGE*2) { i0 = optr-buf; optr += sprintf(optr,"\r\n"); }
			optr += sprintf(optr,"%x:",((int)gexception_context.Eip)-DISASMRANGE*2+i);
			for(k=0;k<j;k++) optr += sprintf(optr,"%02x ",tbuf[i+k]);
			if (j < 9) optr += sprintf(optr,"%*c",27-j*3,32);
			optr += sprintf(optr,"%s\r\n",tbuf2);
			if (i == DISASMRANGE*2) { i1 = optr-buf; optr += sprintf(optr,"\r\n"); }
		}
		optr += sprintf(optr,"...\r\n");
	}
#endif
	HWND hwnd;
	HFONT hfont;
	MSG msg;
	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,"edit","Exception! (ESC:quit)",WS_VISIBLE|WS_CAPTION|WS_POPUP|WS_VSCROLL|ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL,50,50,640,1024,ghwnd,0,ghinst,0);
	hfont = CreateFont(-10,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Courier");
	SendMessage(hwnd,WM_SETFONT,(WPARAM)hfont,0);
	SendMessage(hwnd,EM_SETSEL,0,-1);
	SendMessage(hwnd,EM_REPLACESEL,0,(LPARAM)buf);
	i = SendMessage(hwnd,EM_LINEINDEX,SendMessage(hwnd,EM_LINEFROMCHAR,i0,0)-32,0);
	SendMessage(hwnd,EM_SETSEL,i,i);
	SendMessage(hwnd,EM_SCROLLCARET,0,0);
	SendMessage(hwnd,EM_SETSEL,i0,i1);
	while (1)
	{
		if (PeekMessage(&msg,0,0,0,PM_REMOVE))
		{
			if ((msg.message == WM_CHAR) && ((msg.wParam&255) == 27)) ExitProcess(0);
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Sleep(15);
	}
}

#define CATCHBEG __try
#define CATCHEND __except(exception_getaddr(GetExceptionInformation())){showexception();ExitProcess(0);}
#else
#define CATCHBEG
#define CATCHEND
#endif
//--------------------------------------------------------------------------------------------------

	//FPS counter
#define FPSSIZ 128
static int fpsometer[FPSSIZ], fpsind[FPSSIZ], numframes = 0;

static int *zbuffermem = 0, zbuffersiz = 0;

//--------------------------------------------------------------------------------------------------

static _inline int testflag (int c)
{
#ifdef _MSC_VER
	_asm
	{
		mov ecx, c
		pushfd
		pop eax
		mov edx, eax
		xor eax, ecx
		push eax
		popfd
		pushfd
		pop eax
		xor eax, edx
		mov eax, 1
		jne menostinx
		xor eax, eax
		menostinx:
	}
#else
	int a;
	__asm__ __volatile__ (
		"# testflag\n\t"
		"pushf\n\t"
		"popl %%eax\n\t"
		"movl %%eax, %%ebx\n\t"
		"xorl %%ecx, %%eax\n\t"
		"pushl %%eax\n\t"
		"popf\n\t"
		"pushf\n\t"
		"popl %%eax\n\t"
		"xorl %%ebx, %%eax\n\t"
		"movl $1, %%eax\n\t"
		"jne 0f\n\t"
		"xorl %%eax, %%eax\n\t"
		"0:"
		: "=a" (a)
		: "c" (c)
		: "ebx","cc"
	);
	return a;
#endif
}

static _inline void cpuid (int a, int *s)
{
#ifdef _MSC_VER
	_asm
	{
		push ebx
		push esi
		mov eax, a
		cpuid
		mov esi, s
		mov dword ptr [esi+0], eax
		mov dword ptr [esi+4], ebx
		mov dword ptr [esi+8], ecx
		mov dword ptr [esi+12], edx
		pop esi
		pop ebx
	}
#else
	__asm__ __volatile__ (
		"# cpuid\n\t"
		"cpuid\n\t"
		"movl %%eax, (%%esi)\n\t"
		"movl %%ebx, 4(%%esi)\n\t"
		"movl %%ecx, 8(%%esi)\n\t"
		"movl %%edx, 12(%%esi)"
		: "+a" (a)
		: "S" (s)
		: "ebx","ecx","edx","memory","cc"
	);
#endif
}

	//Bit numbers of return value:
	//0:FPU, 4:RDTSC, 15:CMOV, 22:MMX+, 23:MMX, 25:SSE, 26:SSE2, 30:3DNow!+, 31:3DNow!
static int getcputype (void)
{
	int i, cpb[4], cpid[4];
	if (!testflag(0x200000)) return(0);
	cpuid(0,cpid); if (!cpid[0]) return(0);
	cpuid(1,cpb); i = (cpb[3]&~((1<<22)|(1<<30)|(1<<31)));
	cpuid(0x80000000,cpb);
	if (((unsigned)cpb[0]) > 0x80000000)
	{
		cpuid(0x80000001,cpb);
		i |= (cpb[3]&(1<<31));
		if (!((cpid[1]^0x68747541)|(cpid[3]^0x69746e65)|(cpid[2]^0x444d4163))) //AuthenticAMD
			i |= (cpb[3]&((1<<22)|(1<<30)));
	}
	if (i&(1<<25)) i |= (1<<22); //SSE implies MMX+ support
	return(i);
}

//--------------------------------------------------------------------------------------------------

static void orthorotate (double ox, double oy, double oz, dpoint3d *ist, dpoint3d *ihe, dpoint3d *ifo)
{
	double f, t, dx, dy, dz, rr[9];

	dx = sin(ox); ox = cos(ox);
	dy = sin(oy); oy = cos(oy);
	dz = sin(oz); oz = cos(oz);
	f = ox*oz; t = dx*dz; rr[0] =  t*dy + f; rr[7] = -f*dy - t;
	f = ox*dz; t = dx*oz; rr[1] = -f*dy + t; rr[6] =  t*dy - f;
	rr[2] = dz*oy; rr[3] = -dx*oy; rr[4] = ox*oy; rr[8] = oz*oy; rr[5] = dy;
	ox = ist->x; oy = ihe->x; oz = ifo->x;
	ist->x = ox*rr[0] + oy*rr[3] + oz*rr[6];
	ihe->x = ox*rr[1] + oy*rr[4] + oz*rr[7];
	ifo->x = ox*rr[2] + oy*rr[5] + oz*rr[8];
	ox = ist->y; oy = ihe->y; oz = ifo->y;
	ist->y = ox*rr[0] + oy*rr[3] + oz*rr[6];
	ihe->y = ox*rr[1] + oy*rr[4] + oz*rr[7];
	ifo->y = ox*rr[2] + oy*rr[5] + oz*rr[8];
	ox = ist->z; oy = ihe->z; oz = ifo->z;
	ist->z = ox*rr[0] + oy*rr[3] + oz*rr[6];
	ihe->z = ox*rr[1] + oy*rr[4] + oz*rr[7];
	ifo->z = ox*rr[2] + oy*rr[5] + oz*rr[8];
	//orthonormalize(ist,ihe,ifo);
}

	//Compatible with memset except:
	//   1. All 32 bits of v are used and expected to be filled
	//   2. Writes max((n+7)&~7,8) bytes
	//   3. Assumes d is aligned on 8 byte boundary
static void memset8 (void *d, int v, int n)
{
#ifdef _MSC_VER
	_asm
	{
		mov edx, d
		mov ecx, n
		movd mm0, v
		punpckldq mm0, mm0
memset8beg:
		movntq qword ptr [edx], mm0
		add edx, 8
		sub ecx, 8
		jg short memset8beg
		emms
	}
#else
	asm volatile
	(
		"# memset8\n\t"
		".intel_syntax noprefix\n\t"
		GAS_ATT( "movd %2, %%mm0\n\t" )
		"punpckldq mm0, mm0\n\t"
		"memset8beg: "
		"movntq qword ptr [edx], mm0\n\t"
		"add edx, 8\n\t"
		"sub ecx, 8\n\t"
		"jg short memset8beg\n\t"
		"emms\n\t"
		".att_syntax prefix\n\t"
		: "+d" (d),
		  "+c" (n)
		:  "m" (v)
		: "cc","memory"
	);
#endif
}

typedef struct { dpoint3d p[2]; double r[2]; int col; } spr_t;
static int numsprites, *nocap;
static spr_t *spr = 0;
static void loadkcm (char *filnam)
{
	FILE *fil;
	dpoint3d dp;
	double d, dr, dr2;
	int i, j;

	fil = fopen(filnam,"rb"); if (!fil) { /*MessageBox(ghwnd,"loadkcm failed",prognam,MB_OK);*/ return; }
	fread(&i,4,1,fil);
	if (i != 0x204d434b) //"KCM "
		{ fclose(fil); MessageBox(ghwnd,"loadkcm failed (bad header)",prognam,MB_OK); return; }
	fread(&numsprites,4,1,fil);
	spr = (spr_t *)malloc(numsprites*sizeof(spr_t));
	fread(spr,numsprites*sizeof(spr_t),1,fil);
	fclose(fil);

		//Don't draw spheres that are fully encapsulated by others
		//O(n^2) algo, but only done once so who cares :P
	i = ((((numsprites<<1)+31)>>5)<<2);
	nocap = (int *)malloc(i);
	memset(nocap,0,i);
	for(i=(numsprites<<1)-1;i>=0;i--)
		for(j=(numsprites<<1)-1;j>=0;j--)
		{
			if (i == j) continue;
			dp.x = spr[j>>1].p[j&1].x - spr[i>>1].p[i&1].x;
			dp.y = spr[j>>1].p[j&1].y - spr[i>>1].p[i&1].y;
			dp.z = spr[j>>1].p[j&1].z - spr[i>>1].p[i&1].z;
			dr   = spr[j>>1].r[j&1]   - spr[i>>1].r[i&1]; dr2 = dr*dr;
			d = dp.x*dp.x + dp.y*dp.y + dp.z*dp.z;
			if ((dr >= 0) && (d <= dr2)) nocap[i>>5] |= (1<<i);
			if ((dr <= 0) && (d <= dr2)) nocap[j>>5] |= (1<<j);
		}
	//for(j=0,i=(numsprites<<1)-1;i>=0;i--) if (nocap[i>>5]&(1<<i)) j++; printf("%d/%d\n",j,numsprites); //debug only
}
static void drawkcm (int flags)
{
	int i, j;

	for(i=numsprites-1;i>=0;i--)
	{
		j = flags;
		if (nocap[i>>4]&(1<<((i<<1)+0))) j |= DRAWCONE_NOCAP0;
		if (nocap[i>>4]&(1<<((i<<1)+1))) j |= DRAWCONE_NOCAP1;
		drawcone(spr[i].p[0].x,spr[i].p[0].y,spr[i].p[0].z,max(spr[i].r[0],.001),
					spr[i].p[1].x,spr[i].p[1].y,spr[i].p[1].z,max(spr[i].r[1],.001),spr[i].col,48.0,j);
	}
}

	//sc=128 is no change
static int scalecol (int i, int sc)
{
	return((min((((i>>16)&255)*sc)>>7,255)<<16) +
			 (min((((i>> 8)&255)*sc)>>7,255)<< 8) +
			 (min((((i    )&255)*sc)>>7,255)    ));
}

//-----------------------------------------------------------------------------
	//FCALCSIN: Fast CALCulation of SIN table
typedef struct { double c, s, ci, si, m; } fcalcsin_t;
	//ang0: Angle start (radians, often 0.f)
	//angi: Angle increment (radians/inc)
	// amp: Amplitude (usually 1.f)
static void fcalcsin_init (fcalcsin_t *fcs, double ang0, double angi, double amp)
{
	double hai, n;
		//4 trig, 9 fmul, 6 fadd (faster)
	hai = angi*.5; fcs->m = sin(hai);
	n = cos(hai)*fcs->m*2; fcs->m = fcs->m*fcs->m*-2;
	fcs->c = cos(ang0)*amp; fcs->s = sin(ang0)*amp;
	fcs->ci = fcs->c*fcs->m - fcs->s*n;
	fcs->si = fcs->c*n + fcs->s*fcs->m; fcs->m += fcs->m;
}
static _inline void fcalcsin_inc (fcalcsin_t *fcs)
{
	fcs->c += fcs->ci; fcs->ci += fcs->c*fcs->m;
	fcs->s += fcs->si; fcs->si += fcs->s*fcs->m;
}
static _inline void fcalcsin_incsin (fcalcsin_t *fcs)
	{ fcs->s += fcs->si; fcs->si += fcs->s*fcs->m; }
//-----------------------------------------------------------------------------

static void showzbuffunc (int sy, void *_)
{
	static const int colmullut[8] = {0x000000,0x000001,0x000100,0x000101,0x010000,0x010001,0x010100,0x010101};
	float *zptr;
	int i, sx, *iptr;

	iptr = (int *)(gdd.p*sy + gdd.f); zptr = (float *)(((INT_PTR)iptr)+zbufoff);
	for(sx=0;sx<gdd.x;sx++)
	{
#if (USEINTZ == 0)
		i = cvttss2si((float)(zptr[sx]*1024.0));
#else
		i = cvttss2si((float)(zptr[sx]*4096.0));
#endif
		iptr[sx] = (i&255)*colmullut[(i>>8)&7];
	}
}

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	SYSTEM_INFO si;
	static double otim, tim, dtim, ntim, tim0, tim1;
	static float cmousx = 0.f, cmousy = 0.f, cmousz = 0.f, hzmul = 1.f, a0 = 0.f, a1 = 0.f, a2 = 0.f;
	static int obstatus, bstatus = 0, pauseit = 0, cullmode = 0;
	double d;
	float f;
	int i, j, k, l, m, numcpu, flags;

	CATCHBEG {

	ghinst = hinst;

	xres = 800; yres = 600; prognam = "Drawcone by Ken Silverman";

#if (RENDMETH == 2)
	ipos.x = 0; ipos.y =-3; ipos.z =-7; a0 = 0.0f; a1 = 0.6f;
#else
	ipos.x = 0; ipos.y = 0; ipos.z =-4; a0 = 0.0f; a1 = 0.0f;
#endif

	cputype = getcputype();

	GetSystemInfo(&si);
	numcpu = min(si.dwNumberOfProcessors,MAXCPU);

	memset(fpsometer,0x7f,sizeof(fpsometer)); for(i=0;i<FPSSIZ;i++) fpsind[i] = i; numframes = 0;

	loadkcm("c:/doc/ken/mag6d/magapps/storkman.kcm");

	if (!initapp(hinst)) return(-1);

	{
	POINT p0;
	RECT rect;
	GetCursorPos(&p0);
	rect.left = p0.x; rect.right  = p0.x+1;
	rect.top  = p0.y; rect.bottom = p0.y+1;
	ClipCursor(&rect);
	ShowCursor(0);
	}

	while (!breath())
	{
		if (keystatus[1]) { quitloop(); return(-1); }
		otim = tim; tim = klock(); dtim = tim-otim;

		for(i=0;i<8;i++)
			if (keystatus[i+0x3b])
			{
				numcpu = i+1;
				memset(fpsometer,0x7f,sizeof(fpsometer)); for(i=0;i<FPSSIZ;i++) fpsind[i] = i; numframes = 0;
				break;
			}

		if (keystatus[0x35]) // /
		{
			keystatus[0x35] = 0;
#if (RENDMETH == 2)
			ipos.x = 0; ipos.y =-3; ipos.z =-7; a0 = 0.0f; a1 = 0.6f;
#else
			ipos.x = 0; ipos.y = 0; ipos.z =-4; a0 = 0.0f; a1 = 0.0f;
#endif
		}
		if (keystatus[0x9c]) //KPEnter
		{
			keystatus[0x9c] = 0;
			cullmode++; if (cullmode >= 3) cullmode = 0;
		}
		if (keystatus[0x21]) //F
		{
			keystatus[0x21] = 0;
			ipos.x = -0.58; ipos.y = -0.70; ipos.z = -0.69; a0 = 1.09f; a1 = 0.30f;
		}

			  if (bstatus&2) { cmousy += dmousy; }
		else if (bstatus&1) { cmousx += dmousx; cmousz += dmousy; }
		else
		{
			a0 += (float)dmousx*.008f;
			a1 += (float)dmousy*.008f; a1 = (float)min(max(a1,-PI/2.f),+PI/2.f);
			irig.x = 1; irig.y = 0; irig.z = 0;
			idow.x = 0; idow.y = 1; idow.z = 0;
			ifor.x = 0; ifor.y = 0; ifor.z = 1;
			orthorotate(0,a1,a0,&irig,&idow,&ifor);
		}
		if (dmousx|dmousy) { dmousx = 0; dmousy = 0; }

		f = (float)dtim*2.f;
		if (keystatus[0x2a]) f *= 1.0/16.0;
		if (keystatus[0x36]) f *= 8.0/1.0;
		if (keystatus[0xcb]) { ipos.x -= irig.x*f; ipos.y -= irig.y*f; ipos.z -= irig.z*f; } //left
		if (keystatus[0xcd]) { ipos.x += irig.x*f; ipos.y += irig.y*f; ipos.z += irig.z*f; } //right
		if (keystatus[0xc8]) { ipos.x += ifor.x*f; ipos.y += ifor.y*f; ipos.z += ifor.z*f; } //up
		if (keystatus[0xd0]) { ipos.x -= ifor.x*f; ipos.y -= ifor.y*f; ipos.z -= ifor.z*f; } //down
		if (keystatus[0x9d]) { ipos.x -= idow.x*f; ipos.y -= idow.y*f; ipos.z -= idow.z*f; } //RCtrl
		if (keystatus[0x52]) { ipos.x += idow.x*f; ipos.y += idow.y*f; ipos.z += idow.z*f; } //KP0
		if (keystatus[0x33]|keystatus[0x34]) //,.
		{
			d = sqrt(ipos.x*ipos.x + ipos.y*ipos.y + ipos.z*ipos.z);
			ipos.x += ifor.x*d; ipos.y += ifor.y*d; ipos.z += ifor.z*d;
			orthorotate(0,0,(keystatus[0x33]-keystatus[0x34])*dtim,&irig,&idow,&ifor);
			ipos.x -= ifor.x*d; ipos.y -= ifor.y*d; ipos.z -= ifor.z*d;
		}
		if (keystatus[0xb5]) //KP/
		{
			f = hzmul; hzmul *= (float)pow(3.0,-dtim);
			if ((f > 1.f) && (hzmul <= 1.f)) { hzmul = 1.f; keystatus[0xb5] = 0; }
		}
		if (keystatus[0x37]) //KP*
		{
			f = hzmul; hzmul *= (float)pow(3.0,+dtim);
			if ((f < 1.f) && (hzmul >= 1.f)) { hzmul = 1.f; keystatus[0x37] = 0; }
		}
		if (keystatus[0x19]) { keystatus[0x19] = 0; pauseit ^= 1; }

		if (!pauseit) ntim = tim;

		if (!startdirectdraw(&gdd.f,&gdd.p,&gdd.x,&gdd.y)) goto skipdd;

		drawrectfill(&gdd,0,0,gdd.x,gdd.y,0x333333);

		i = gdd.y*gdd.p+256;
		if ((i > zbuffersiz) || (!zbuffermem))  //Increase Z buffer size if too small
		{
			if (zbuffermem) { free(zbuffermem); zbuffermem = 0; }
			zbuffersiz = i; zbuffermem = (int *)malloc(zbuffersiz);
		}

			//zbuffer aligns its memory to the same pixel boundaries as the screen!
			//WARNING: Pentium 4's L2 cache has severe slowdowns when 65536-64 <= (zbufoff&65535) < 64
		zbufoff = (((((INT_PTR)zbuffermem)-gdd.f-128)+255)&~255)+128;

		if (cputype&(1<<25)) //Got SSE
			  { for(j=0,i=gdd.f+zbufoff;j<gdd.y;j++,i+=gdd.p) memset8((void *)i,0x7f7f7f7f,gdd.x<<2); }
		else { for(j=0,i=gdd.f+zbufoff;j<gdd.y;j++,i+=gdd.p) memset((void *)i,0x7f,gdd.x<<2); }

		drawcone_setup(cputype,numcpu,&gdd,zbufoff,&ipos,&irig,&idow,&ifor,
				((float)gdd.x)*.5,((float)gdd.y)*.5,((float)gdd.x)*.5*hzmul);

		flags = 0;
		if (cullmode == 0) flags |= DRAWCONE_CULL_BACK;
		if (cullmode == 1) flags |= DRAWCONE_CULL_FRONT;
		if (cullmode == 2) flags |= DRAWCONE_CULL_NONE;
		if (keystatus[0x36]) flags |= DRAWCONE_NOPHONG;

		tim0 = klock();
#if (RENDMETH == 0)
		fcalcsin_t fcs;
		double dx, dy, dz, rad, rad2, zmul, zadd;
		int n;
		n = 8192; rad = 2.0;
		zmul = rad*2.0/(double)n; zadd = zmul*-.5 - rad; rad2 = rad*rad;
		fcalcsin_init(&fcs,0.0,(1.0-(sqrt(5.0)-1.0)/2.0)*PI*2.0,1.0);
		for(i=n-1;i>=0;i--,fcalcsin_inc(&fcs))
		{
			dz = (double)i*zmul+zadd; d = sqrt(rad2 - dz*dz);
			drawsph(fcs.c*d,fcs.s*d,dz,0.04,0x608070,38.4,flags);
		}
#elif (RENDMETH == 1)
		int x, y, z;
		for(z=-2;z<=2;z++)
			for(y=-2;y<=2;y++)
				for(x=-2;x<=2;x++)
					if ((labs(x) >= 2) || (labs(y) >= 2) || (labs(z) >= 2))
						drawsph(x-ipos.x,y-ipos.y,z-ipos.z,1.0,0x808080,38.4,flags);
#elif (RENDMETH == 2)
		fcalcsin_t fcs[4];

			//Rounded cones flying around torus coil
		fcalcsin_init(&fcs[0],ntim         - PI/  8.0,PI/ 4.0,1.0);
		fcalcsin_init(&fcs[1],ntim                   ,PI/ 4.0,1.0);
		fcalcsin_init(&fcs[2],ntim*(PI/64.0)-PI/128.0,PI/64.0,1.0);
		fcalcsin_init(&fcs[3],ntim*(PI/64.0)         ,PI/64.0,1.0);
		for(i=128;i>0;i--)
		{
			double c0, c1; c0 = fcs[0].c+4.0; c1 = fcs[1].c+4.0;
			drawcone(fcs[2].c*c0,fcs[0].s,fcs[2].s*c0,.2,
						fcs[3].c*c1,fcs[1].s,fcs[3].s*c1,.3,scalecol(0x667380,cvttss2si(fcs[2].s*16.0)+128),38.4,flags);
			for(j=4-1;j>=0;j--) fcalcsin_inc(&fcs[j]);
		}

			//Water drops flying around circle
		fcalcsin_init(&fcs[0],ntim            ,PI/8.0,4.0);
		fcalcsin_init(&fcs[1],ntim+PI*3.0/32.0,PI/8.0,4.0);
		for(i=32;i>0;i--,fcalcsin_inc(&fcs[0]),fcalcsin_inc(&fcs[1]))
			drawcone(fcs[0].c,0.0,fcs[0].s,.01,fcs[1].c,0.0,fcs[1].s,.25,0x606030,38.4,flags);

			//Blue nucleus
		drawsph(0,0,0,1.2,0x2d5a4d,38.4,flags);
#elif (RENDMETH == 3)
		drawcone(0,3.1,0,+0.1, 0,-1.0,0,+0.3, 0x908070,38.4,flags);
		drawcone(0,2.1,0,-0.2, 0,-0.8,0,-0.5, 0x809080,38.4,flags);
		drawcone(0,1.1,0,-0.3, 0,-0.6,0,-0.7, 0x708090,38.4,flags);

		d = ntim+PI*0.0/3.0; drawcone(-cos(d)*.2,1.5,-sin(d)*.2,0.25, +cos(d)*1,-2,+sin(d)*1,0.01, 0xff8080,38.4,flags);
		d = ntim+PI*2.0/3.0; drawcone(-cos(d)*.2,1.5,-sin(d)*.2,0.25, +cos(d)*1,-2,+sin(d)*1,0.01, 0x80ff80,38.4,flags);
		d = ntim+PI*4.0/3.0; drawcone(-cos(d)*.2,1.5,-sin(d)*.2,0.25, +cos(d)*1,-2,+sin(d)*1,0.01, 0x8080ff,38.4,flags);
		drawsph(0,-2.4,0,1.00, 0xc0c0c0,38.4,flags);

		drawcone(cmousx*.01,cmousy*.01,cmousz*.01,-0.25, 2,-1,-2,-.5, 0x406090,38.4,flags);

#elif (RENDMETH == 4)
		//flags |= DRAWCONE_NOCAP0|DRAWCONE_NOCAP1;
		drawsph(0.0,-2.5,-0.5,0.3, 0x808080,38.4,flags);
		drawcone(-0.5,-1.5,-0.5,0.1, +0.5,-1.5,-0.5,0.4, 0x808080,38.4,flags);
		drawcone(-0.5,-0.5,-0.5,0.1, +0.5,-0.5,-0.5,0.4, 0x808080,38.4,flags|DRAWCONE_FLAT0);
		drawcone(-0.5,+0.5,-0.5,0.1, +0.5,+0.5,-0.5,0.4, 0x808080,38.4,flags|DRAWCONE_FLAT1);
		drawcone(-0.5,+1.5,-0.5,0.1, +0.5,+1.5,-0.5,0.4, 0x808080,38.4,flags|DRAWCONE_FLAT0|DRAWCONE_FLAT1);
#else
		drawkcm(flags);
#endif
		tim1 = klock();

		if (keystatus[0x1d]) { htrun(showzbuffunc,0,0,gdd.y,numcpu); }

			//FPS counter
		fpsometer[numframes&(FPSSIZ-1)] = (int)((tim1-tim0)*1000000.0); numframes++;
			//Fast sort when already sorted... otherwise slow!
		j = min(numframes,FPSSIZ)-1;
		for(k=0;k<j;k++)
			if (fpsometer[fpsind[k]] > fpsometer[fpsind[k+1]])
			{
				m = fpsind[k+1];
				for(l=k;l>=0;l--)
				{
					fpsind[l+1] = fpsind[l];
					if (fpsometer[fpsind[l]] <= fpsometer[m]) break;
				}
				fpsind[l+1] = m;
			}
		i = ((fpsometer[fpsind[j>>1]]+fpsometer[fpsind[(j+1)>>1]])>>1); //Median

		print6x8(&gdd,0, 0,0xffffff,0,"%.2fms THR:%d",(double)i*.001,numcpu);
		print6x8(&gdd,0,16,0xffffff,0,"%.2f %.2f %.2f %.2f %.2f",ipos.x,ipos.y,ipos.z,a0,a1);

		stopdirectdraw();
		nextpage();
skipdd:;
		i = 15; MsgWaitForMultipleObjects(0,0,0,i,QS_KEY|QS_MOUSE|QS_PAINT); //smartsleep

		obstatus = bstatus;
	}
	uninitapp();
	if (nocap) { free(nocap); nocap = 0; }
	if (spr  ) { free(spr  ); spr   = 0; }

	} CATCHEND

	return(0);
}

#endif

#if 0
!endif
#endif
