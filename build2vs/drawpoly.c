#if 0 //To compile, type: "nmake drawpoly.c"
drawpoly.exe: drawpoly.obj kplib.obj winmain.obj; link drawpoly kplib winmain ddraw.lib dinput.lib ole32.lib dxguid.lib user32.lib gdi32.lib winmm.lib /opt:nowin98
	del drawpoly.obj
drawpoly.obj: drawpoly.c sysmain.h; cl /c /J /TP drawpoly.c  /Ox      /G6Fy /Gs /MD /QIfist /DSTANDALONE
kplib.obj: kplib.c;                 cl /c /J /TP kplib.c     /Ox /Ob2 /G6Fy /Gs /MD
winmain.obj: winmain.cpp sysmain.h; cl /c /J /TP winmain.cpp /Ox /Ob2 /G6Fy /Gs /MD /DUSEKZ /DNOSOUND
!if 0
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include "drawpoly.h"
#include "Core/kplib.h"

#define MAXXDIM 4096
#define MAXYDIM 2160
#define SCISDIST 0.05
#define CPULIMIT 64
#define CASTDIST 2048 //CASTDIST<<LDISTPREC < 2^31
#define LHSCALE 0 //Range: {0,1,2,3}
#define LDISTPREC 16 //16 for 32*16 pmaddwd, 19 for 16*16 pmaddwd
#define DISTPREC (1<<LDISTPREC)
#define TESTDEPTH 0 //0:normal render, 1:test both
#define ZSCALE (1<<23) //1<<(20..26)

//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------

double drawpoly_anginc = 2.0;
int drawpoly_numcpu = 0;

static float frecipi[MAXXDIM+MAXYDIM];
static long recipi32[sizeof(frecipi)/sizeof(frecipi[0])];
static __int64 recipi64[sizeof(frecipi)/sizeof(frecipi[0])];

static cam_t gcc;
static float gcx, gcy;
static __int64 qposzupldist;
static int grendflags, poszupldist, aipi[2];
static point3d gprojadd;
static long g_rgbmul;
static __int64 g_qrgbmul;

//--------------------------------------------------------------------------------------------------
	//Simple library that takes advantage of multi-threading. For 2 CPU HT, simply do this:
	//Change: for(i=v0;i<v1;i++) myfunc(i);
	//    To: htrun(myfunc,v0,v1,2);

#define MAXCPU 64
static HANDLE gthand[MAXCPU-1];
static HANDLE ghevent[2][MAXCPU-1]; //WARNING: WaitForMultipleObjects has a limit of 64 threads
static int gnthreadcnt = 0, glincnt[2];
static __forceinline int getnextindexsynced () { _asm mov eax, 1 _asm lock xadd glincnt, eax }
static void (*ghtcallfunc)(int);
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
	void (*dacallfunc)(int);
	int i, thnum = (int)_, endcnt;

	while (1)
	{
		WaitForSingleObject(ghevent[0][thnum],INFINITE);
		dacallfunc = ghtcallfunc; endcnt = glincnt[1];
		while ((i = getnextindexsynced()) < endcnt) dacallfunc(i);
		SetEvent(ghevent[1][thnum]);
	}
}

void htrun (void (*dacallfunc)(int), int v0, int v1, int danumcpu)
{
	int i, threaduse; unsigned win98requiresme;

		//1 CPU requested; execute here and quit.
	if (danumcpu <= 1) { for(i=v0;i<v1;i++) dacallfunc(i); return; }

		//Initialize new threads if necessary
	threaduse = min(danumcpu,MAXCPU)-1;
	while (gnthreadcnt < threaduse)
	{
		if (!gnthreadcnt) atexit(htuninit);
		for(i=0;i<2;i++) ghevent[i][gnthreadcnt] = CreateEvent(0,0,0,0);
		gthand[gnthreadcnt] = (HANDLE)_beginthreadex(0,0,ghtfunc,(void *)gnthreadcnt,0,&win98requiresme);
		gnthreadcnt++;
	}

		//Start other threads
	ghtcallfunc = dacallfunc; glincnt[0] = v0; glincnt[1] = v1;
	for(i=threaduse-1;i>=0;i--) SetEvent(ghevent[0][i]);

		//Do some processing in this thread too :)
	while ((i = getnextindexsynced()) < v1) dacallfunc(i);

		//Wait for all other threads to finish (for safety reasons)
	WaitForMultipleObjects(threaduse,&ghevent[1][0],1,INFINITE);
}

#if 0
static int htnumcpu;
static void (*htcallfunc)(int);
void htstart (void (*dacallfunc)(int), int v0, int v1, int danumcpu)
{
	int i, threaduse; unsigned win98requiresme;

		//1 CPU requested; execute here and quit.
	htnumcpu = danumcpu; htcallfunc = dacallfunc;
	if (danumcpu <= 1) { for(i=v0;i<v1;i++) dacallfunc(i); return; }

		//Initialize new threads if necessary
	threaduse = min(danumcpu,MAXCPU)-1;
	while (gnthreadcnt < threaduse)
	{
		if (!gnthreadcnt) atexit(htuninit);
		for(i=0;i<2;i++) ghevent[i][gnthreadcnt] = CreateEvent(0,0,0,0);
		gthand[gnthreadcnt] = (HANDLE)_beginthreadex(0,0,ghtfunc,(void *)gnthreadcnt,0,&win98requiresme);
		gnthreadcnt++;
	}

		//Start other threads
	ghtcallfunc = dacallfunc; glincnt[0] = v0; glincnt[1] = v1;
	for(i=threaduse-1;i>=0;i--) SetEvent(ghevent[0][i]);
}

void htfinish (void)
{
	int i, v1, threaduse;

	if (htnumcpu <= 1) return;
	v1 = glincnt[1];
	threaduse = min(htnumcpu,MAXCPU)-1;

		//Do some processing in this thread too :)
	while ((i = getnextindexsynced()) < v1) htcallfunc(i);

		//Wait for all other threads to finish (for safety reasons)
	WaitForMultipleObjects(threaduse,&ghevent[1][0],1,INFINITE);
}
#endif

//--------------------------------------------------------------------------------------------------

#define SELFMODVAL(dalab,daval) \
{  void *_daddr; unsigned long _oprot; \
	_asm { mov _daddr, offset dalab } \
	VirtualProtect(_daddr,sizeof(daval),PAGE_EXECUTE_READWRITE,&_oprot); \
	switch(sizeof(daval)) \
	{  case 1: *(char *)_daddr = daval; break; \
		case 2: *(short *)_daddr = daval; break; \
		case 4: *(long *)_daddr = daval; break; \
		case 8: *(__int64 *)_daddr = daval; break; \
	} \
	VirtualProtect(_daddr,sizeof(daval),PAGE_EXECUTE,&_oprot); \
	/*FlushInstructionCache(GetCurrentProcess(),_daddr,sizeof(daval));*/ \
}

static __forceinline long bsf (long a) { _asm bsf eax, a }
static __forceinline long bsr (long a) { _asm bsr eax, a }

static __forceinline long mulshr16 (long a, long d)
{
	_asm
	{
		mov eax, a
		imul d
		shrd eax, edx, 16
	}
}

static __forceinline long umulshr16 (long a, long d)
{
	_asm
	{
		mov eax, a
		mul d
		shrd eax, edx, 16
	}
}

static void drawpix (long x, long y, long c)
{
	if (((unsigned long)x >= gcc.c.x) || ((unsigned long)y >= gcc.c.y)) return;
	*(long *)(y*gcc.c.p + (x<<2) + gcc.c.f) = c;
}

static void drawline2d (float x0, float y0, float x1, float y1, long col)
{
	float f;
	int i, dx, dy, ipx[2], ipy[2];

		  if (x0 <       0) { if (x1 <       0) return; y0 = (      0-x0)*(y1-y0)/(x1-x0)+y0; x0 =       0; }
	else if (x0 > gcc.c.x) { if (x1 > gcc.c.x) return; y0 = (gcc.c.x-x0)*(y1-y0)/(x1-x0)+y0; x0 = gcc.c.x; }
		  if (y0 <       0) { if (y1 <       0) return; x0 = (      0-y0)*(x1-x0)/(y1-y0)+x0; y0 =       0; }
	else if (y0 > gcc.c.y) { if (y1 > gcc.c.y) return; x0 = (gcc.c.y-y0)*(x1-x0)/(y1-y0)+x0; y0 = gcc.c.y; }
		  if (x1 <       0) {                           y1 = (      0-x1)*(y1-y0)/(x1-x0)+y1; x1 =       0; }
	else if (x1 > gcc.c.x) {                           y1 = (gcc.c.x-x1)*(y1-y0)/(x1-x0)+y1; x1 = gcc.c.x; }
		  if (y1 <       0) {                           x1 = (      0-y1)*(x1-x0)/(y1-y0)+x1; y1 =       0; }
	else if (y1 > gcc.c.y) {                           x1 = (gcc.c.y-y1)*(x1-x0)/(y1-y0)+x1; y1 = gcc.c.y; }

	x1 -= x0; y1 -= y0;
	i = ceil(max(fabs(x1),fabs(y1))); if (!i) return;
	f = 65536.0/(float)i;
	ipx[0] = (int)(x0*65536.0); ipx[1] = (int)(x1*f);
	ipy[0] = (int)(y0*65536.0); ipy[1] = (int)(y1*f);
	for(;i>0;i--)
	{
		drawpix(ipx[0]>>16,ipy[0]>>16,col);
		ipx[0] += ipx[1]; ipy[0] += ipy[1];
	}
}

static void drawline3d (float x0, float y0, float z0, float x1, float y1, float z1, long col)
{
	float ox, oy, oz, f;

	ox = x0-gcc.p.x; oy = y0-gcc.p.y; oz = z0-gcc.p.z;
	x0 = ox*gcc.r.x + oy*gcc.r.y + oz*gcc.r.z;
	y0 = ox*gcc.d.x + oy*gcc.d.y + oz*gcc.d.z;
	z0 = ox*gcc.f.x + oy*gcc.f.y + oz*gcc.f.z;

	ox = x1-gcc.p.x; oy = y1-gcc.p.y; oz = z1-gcc.p.z;
	x1 = ox*gcc.r.x + oy*gcc.r.y + oz*gcc.r.z;
	y1 = ox*gcc.d.x + oy*gcc.d.y + oz*gcc.d.z;
	z1 = ox*gcc.f.x + oy*gcc.f.y + oz*gcc.f.z;

	if (z0 < SCISDIST)
	{
		if (z1 < SCISDIST) return;
		f = (SCISDIST-z0)/(z1-z0); x0 += (x1-x0)*f; y0 += (y1-y0)*f; z0 = SCISDIST;
	}
	else if (z1 < SCISDIST)
	{
		f = (SCISDIST-z1)/(z1-z0); x1 += (x1-x0)*f; y1 += (y1-y0)*f; z1 = SCISDIST;
	}

	ox = gcc.h.z/z0;
	oy = gcc.h.z/z1;
	drawline2d(x0*ox+gcc.h.x,y0*ox+gcc.h.y,x1*oy+gcc.h.x,y1*oy+gcc.h.y,col);
}

#ifndef DISABLEHEIGHTMAP
#define MIDBUFSIZ 2048
#define MIDBUFENTRIES (2048+1) //NOTE: Must change asm code when changing this :/
static long *midbuf = 0; //midbuf[MIDBUFSIZ][MIDBUFENTRIES][2];
static long midbuf0[MIDBUFSIZ], midbuf1[MIDBUFSIZ]; //FIX:dynamic alloc
__declspec(align(4)) static short smidprojlut[MIDBUFSIZ][2];

typedef struct { long *nvt, leng; float sx[2], sy[2], d[2]; } gline_t;
static gline_t glt[MAXXDIM*2+MAXYDIM*2+256];
static tiltyp *gtt;
static float ghsc = 1.f;
#endif

#pragma warning(disable:4799) //I know how to use EMMS
static __forceinline long addusb (long a, long b)
{
	_asm
	{
		movd mm0, a
		movd mm1, b
		paddusb mm0, mm1
		movd eax, mm0
	}
}

static __forceinline long subusb (long a, long b)
{
	_asm
	{
		movd mm0, a
		movd mm1, b
		psubusb mm0, mm1
		movd eax, mm0
	}
}

#ifndef _MSC_VER
static _inline long rgb_scale (long c0, long c1)
{
	unsigned char *u0, *u1;
	u0 = (unsigned char *)&c0;
	u1 = (unsigned char *)&c1;
	u0[0] = (unsigned char)min((((long)u0[0])*((long)u1[0]))>>7,255);
	u0[1] = (unsigned char)min((((long)u0[1])*((long)u1[1]))>>7,255);
	u0[2] = (unsigned char)min((((long)u0[2])*((long)u1[2]))>>7,255);
	return(c0);
}
#else
static _inline long rgb_scale (long c0, long c1)
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

#ifndef DISABLEHEIGHTMAP

static __forceinline long dmulrethigh (long b, long c, long a, long d)
{
#if 1
	_asm
	{
		mov eax, a
		imul d
		mov ecx, eax
		push edx
		mov eax, b
		imul c
		sub eax, ecx
		pop ecx
		sbb edx, ecx
		mov eax, edx
	}
#else
	return((b>>16)*(c>>16) - (a>>16)*(d>>16)); //approx
#endif
}

static __int64 qtexxor;
static long ltexxor, ltexxord8;
static void voxraycast (int index)
{
	static const __int64 qmask8000 = 0x0000000080008000I64;
	float f, ff, vd0, vd1, vx0, vy0, vz0, vx1, vy1, vz1, rvx, rvy, vx, vy;
	__int64 qfraci[2], syxi;
	long *nvt, leng, texbase, texmask;
	unsigned int ifrac[2], ifraci[2];
	int ihz, hhz, h, r, g, b, rr, gg, bb, ri, gi, bi, camcf, vxg0, vyg0;
	int tp[2], ti[2], ipi[2], ipofs[2], spx[2], spy[2], od, ohz, di, hi;
	int i, j, d, de, k, p, pi, x, y, sy, ip, col, sxp, syp, sxi, syi, donecnt;
	unsigned char *uptr, *uptr2;

	leng = glt[index].leng; if (!leng) return;
	d  = (int)(glt[index].d[0]*DISTPREC);
	de = (int)(glt[index].d[1]*DISTPREC);
	if (d >= de) { midbuf0[index] = midbuf1[index] = 0; return; }
	nvt = glt[index].nvt;
	vx0 = glt[index].sx[0]*gcc.r.x + glt[index].sy[0]*gcc.d.x + gprojadd.x;
	vy0 = glt[index].sx[0]*gcc.r.y + glt[index].sy[0]*gcc.d.y + gprojadd.y;
	vz0 = glt[index].sx[0]*gcc.r.z + glt[index].sy[0]*gcc.d.z + gprojadd.z;
	vx1 = glt[index].sx[1]*gcc.r.x + glt[index].sy[1]*gcc.d.x + gprojadd.x;
	vy1 = glt[index].sx[1]*gcc.r.y + glt[index].sy[1]*gcc.d.y + gprojadd.y;
	vz1 = glt[index].sx[1]*gcc.r.z + glt[index].sy[1]*gcc.d.z + gprojadd.z;

	vd0 = sqrt(vx0*vx0 + vy0*vy0);
	vd1 = sqrt(vx1*vx1 + vy1*vy1);
	rvx = 1.0/fabs(vx0); vxg0 = (vx0>0);
	rvy = 1.0/fabs(vy0); vyg0 = (vy0>0);

	ff = glt[index].d[0]/vd0;
	vx = vx0*ff + gcc.p.x; x = floor(vx);
	vy = vy0*ff + gcc.p.y; y = floor(vy);
	spx[0] = (int)((gcc.p.x - (float)x)*65536.0);
	spy[0] = (int)((gcc.p.y - (float)y)*65536.0);
	spx[1] = 65536-spx[0]; //spx[1] = (int)(((float)(x+1) - gcc.p.x)*65536.0);
	spy[1] = 65536-spy[0]; //spy[1] = (int)(((float)(y+1) - gcc.p.y)*65536.0);
	ip = x*aipi[1] + y*aipi[0];

	f = vd0*((float)DISTPREC);
	ti[0] = (int)(f*rvx); if (ti[0] <= 0) ti[0] = 0x7fffffff;
	ti[1] = (int)(f*rvy); if (ti[1] <= 0) ti[1] = 0x7fffffff;
	tp[0] = umulshr16(ti[0],spx[vxg0]);
	tp[1] = umulshr16(ti[1],spy[vyg0]);
	if (vx0 < 0) ipi[0] =      -4; else ipi[0] =      4;
	if (vy0 < 0) ipi[1] = -gtt->p; else ipi[1] = gtt->p;
	texbase = gtt->f; texmask = ((gtt->x*gtt->y-1)<<2);

	if (leng > 0) {               p = (int)&nvt[(leng-1)*2]; pi = -8; camcf = (int)&nvt[  -1*2]; }
				else { leng = -leng; p = (int)&nvt[       0*2]; pi = +8; camcf = (int)&nvt[leng*2]; }
	sy = leng-1; //ihz = (int)gcc.hz;

	f = 32767.0*65536.0/max(max(vd0,vd1),max(fabs(vz0),fabs(vz1))*ghsc); //f <= 327...
	ff = frecipi[leng]*f;
	sxi = (vd1-vd0)*ff     ; sxp = vd1*f     -sxi;
	syi = (vz1-vz0)*ff*ghsc; syp = vz1*f*ghsc-syi;

		//skip near
	if ((d == 0.0) || (!(grendflags&RENDFLAGS_COVSID)))
		  hhz = (signed)(((*(unsigned long *)((ip&texmask)+texbase))^ltexxor)>>(24-LDISTPREC+LHSCALE)) - poszupldist; //no sides
	else hhz = ((signed)(0x80000000>>(23-LDISTPREC+LHSCALE))) - poszupldist; //fill sides
	while (dmulrethigh(hhz,sxp,syp,d) < 0)
	{
		p += pi; sy--; if (sy < 0) { midbuf0[index] = midbuf1[index] = 0; return; }
		sxp -= sxi; syp -= syi;
	}
	midbuf0[index] = sy+1;

	if (!(grendflags&RENDFLAGS_INTERP))
	{
		do //Perfect cubes
		{
			i = (*(long *)((ip&texmask)+texbase))^ltexxor;
			hhz = (signed)((unsigned)i>>(24-LDISTPREC+LHSCALE)) - poszupldist;

			while (dmulrethigh(hhz,sxp,syp,d) < 0) //wall
			{
				*(long *)p = rgb_scale(subusb(i,(-k)&0x040404),g_rgbmul);
				*(long *)(p+4) = (((unsigned)hhz)>>16)+(d&0xffff0000);
				p += pi; if (p == camcf) goto rccubebreak;
				sxp -= sxi; syp -= syi;
			}

			k = (((unsigned)(tp[1]-tp[0]))>>31);
			d = min(tp[k],de); tp[k] += ti[k]; ip += ipi[k];

			while (dmulrethigh(hhz,sxp,syp,d) < 0) //floor
			{
				*(long *)p = rgb_scale(addusb(i,0x040404),g_rgbmul);
				*(long *)(p+4) = (((unsigned)hhz)>>16)+(d&0xffff0000);
				p += pi; if (p == camcf) goto rccubebreak;
				sxp -= sxi; syp -= syi;
			}

		} while (d < de);
rccubebreak:;
		midbuf1[index] = (labs(camcf-p)>>3)-1;
	}
	else
	{
			 //Interpolated cubes
		ipofs[0] = (-vxg0)&ipi[0];
		ipofs[1] = (-vyg0)&ipi[1];

		ifraci[0] = (int)(vy0*rvx*16777216.0); ifrac[0] = mulshr16(ifraci[0],spx[vxg0]);
		ifraci[1] = (int)(vx0*rvy*16777216.0); ifrac[1] = mulshr16(ifraci[1],spy[vyg0]);
		ifraci[0] <<= 8; ifrac[0] = (ifrac[0]<<8) + (spy[0]<<16);
		ifraci[1] <<= 8; ifrac[1] = (ifrac[1]<<8) + (spx[0]<<16);

			//Initial color (for zenith point)
		uptr = (unsigned char *)((ip&texmask)+texbase); uptr2 = &uptr[aipi[0]];
		i = min(max(spx[0],0),65535);
		j = min(max(spy[0],0),65535);
		bb = ((int)uptr [0]); bb = (((int)uptr [4])-bb)*i + (bb<<16);
		gg = ((int)uptr [1]); gg = (((int)uptr [5])-gg)*i + (gg<<16);
		rr = ((int)uptr [2]); rr = (((int)uptr [6])-rr)*i + (rr<<16);
		 b = ((int)uptr2[0]);  b = (((int)uptr2[4])- b)*i + ( b<<16);
		 g = ((int)uptr2[1]);  g = (((int)uptr2[5])- g)*i + ( g<<16);
		 r = ((int)uptr2[2]);  r = (((int)uptr2[6])- r)*i + ( r<<16);
		bb += mulshr16(b-bb,j);
		gg += mulshr16(g-gg,j);
		rr += mulshr16(r-rr,j);
#if 0
		do
		{
			k = (((unsigned)(tp[1]-tp[0]))>>31); od = d; d = min(tp[k],de); tp[k] += ti[k];
			uptr = (unsigned char *)(((ip+ipofs[k])&texmask)+texbase); uptr2 = &uptr[aipi[k]];
			b  = (((int)uptr2[0])-((int)uptr[0]))*(ifrac[k]>>16) + (((int)uptr[0])<<16);
			g  = (((int)uptr2[1])-((int)uptr[1]))*(ifrac[k]>>16) + (((int)uptr[1])<<16);
			r  = (((int)uptr2[2])-((int)uptr[2]))*(ifrac[k]>>16) + (((int)uptr[2])<<16);
			h  =((((int)uptr2[3])-((int)uptr[3]))*(ifrac[k]>>16) + (((int)uptr[3])<<16)) ^ ltexxord8;
			ip += ipi[k]; ifrac[k] += ifraci[k];
			ohz = hhz;
#if (LHSCALE-LDISTPREC+16 >= 0)
			hhz = (h>>(LHSCALE-LDISTPREC+16))-poszupldist;
#else
			hhz = (h<<(LDISTPREC-LHSCALE-16))-poszupldist;
#endif
			for(y=sy;dmulrethigh(hhz,sxp,syp,d) < 0;y--,sxp-=sxi,syp-=syi) if (y < 0) break;
			if (sy > y)
			{
				i = recipi32[sy-y];
				bi = ((  b- bb+65535)>>16)*i;
				gi = ((  g- gg+65535)>>16)*i;
				ri = ((  r- rr+65535)>>16)*i;
				di = ((  d- od+65535)>>16)*i;
				hi = ((hhz-ohz+65535)>>16)*i;
				do
				{
					*(long *)p = rgb_scale((rr&0xff0000)+((gg>>8)&0xff00)+(bb>>16),g_rgbmul);
					*(long *)(p+4) = (((unsigned)ohz)>>16)+(od&0xffff0000); //d = k*sqrt(x^2 + y^2), hhz/8 = k*abs(z)
					bb += bi; gg += gi; rr += ri; ohz += hi; od += di;
					p += pi; sy--;
				} while (sy > y);
				if (y < 0) { midbuf1[index] = 0; return; }
			}
			bb = b; gg = g; rr = r;
		} while (d < de);
		midbuf1[index] = sy;
#else
		qfraci[0] = (__int64)ifraci[0]; qfraci[1] = (__int64)ifraci[1];
		syp = -syp; syi = -syi; //Hack for pmaddwd
		syxi = (((__int64)syi)<<32) + ((__int64)sxi);
		donecnt = -1;
		tp[0] += 32768; tp[1] += 32768; de += 32768;

		_asm
		{
			push esi
			push edi

				;eax:[     temp     ]
				;ebx:[--------------]
				;ecx:[     temp     ]
				;edx:[       p      ]
				;esi:[      ip      ]
				;edi:[      sy      ]
				;ebp:[--------------]
				;esp:[--------------]
				;mm0:[  newh   newr   newg   newb  ]
				;mm1:[ (d_h    d_l)  (hhz_h  hhz_l)]
				;mm2:[             temp            ]
				;mm3:[             temp            ]
				;mm4:[ syp_h  syp_l  sxp_h  sxp_l  ]
				;mm5:[            (free)           ]
				;mm6:[  xxxx   oldr   oldg   oldb  ]
				;mm7:[              0              ]

			pxor mm7, mm7
#if 0
			mov eax, uptr ;initial color (nearest)
			movd mm6, [eax]
			punpcklbw mm6, mm7
			psllw mm6, 7
#else
			movd mm6, bb ;initial color (bilinear)
			punpckldq mm6, gg
			movd mm5, rr
			psrld mm6, 9
			psrld mm5, 9
			packssdw mm6, mm5
#endif
			movd mm4, sxp
			punpckldq mm4, syp
			mov esi, ip
			mov edi, sy
			mov edx, p

begpix:  mov eax, tp[4]
			sub eax, tp[0]
			shr eax, 31

			mov ecx, ipofs[eax*4] ;uptr = (unsigned char *)(((ip+ipofs[k])&texmask)+texbase);
			add ecx, esi
			and ecx, texmask
			add ecx, texbase
			movd mm0, [ecx]
			pxor mm0, qtexxor
			add ecx, aipi[eax*4]
			movd mm1, [ecx]
			pxor mm1, qtexxor
			punpcklbw mm0, mm7
			punpcklbw mm1, mm7

			movd mm2, ifrac[eax*4]
			pshufw mm3, mm2, 0x55

			psllw mm0, 7             ;mm0:[ 0  h  0  r  0  g  0  b ]
			psllw mm1, 7             ;mm1:[ 0  h2 0  r2 0  g2 0  b2]
			psubw mm1, mm0
			psrlw mm3, 1
			pmulhw mm1, mm3
			paddw mm1, mm1
			paddw mm0, mm1
			pshufw mm1, mm0, 0xff
			psrad mm1, 16+7-LDISTPREC+LHSCALE
			psubd mm1, qposzupldist

			paddd mm2, qfraci[eax*8]
			movd ifrac[eax*4], mm2
			mov ecx, tp[eax*4]
			cmp ecx, de
			jge short preendpix
			punpckldq mm1, tp[eax*4]
backfrompreendpix:
			add ecx, ti[eax*4]
			mov tp[eax*4], ecx

			add esi, ipi[eax*4]

				;for(y=sy;dmulrethigh(hhz,sxp,syp,d) < 0;y--,sxp-=sxi,syp-=syi) if (y < 0) break;
				;   mm4:[ syp_h  .....  sxp_h  .....  ]
				;   mm1:[   d_h    d_l  hhz_h  hhz_l  ]
			mov ecx, edi
#if 0
				;16*16 multiply
			pshufw mm2, mm1, 0xdd  ;mm2: [  d_h hhz_h   d_h hhz_h]
srchpix: pshufw mm3, mm4, 0xdd  ;mm3: [syp_h sxp_h syp_h sxp_h]
			pmaddwd mm3, mm2
			movd eax, mm3
			test eax, eax
			jge short donesrchpix
			psubd mm4, syxi
			sub edi, 1
			jge short srchpix
donesrchpix:
#else
				;32*16 multiply
			pshufw mm2, mm1, 0xd8  ;mm2: [  d_h hhz_h   d_l hhz_l]
			pxor mm2, qmask8000    ;0x0000000080008000
			jmp short srchpix
prebegsrchpix:
			psubd mm4, syxi
			sub edi, 1
			jl short donesrchpix
srchpix: pshufw mm3, mm4, 0xdd  ;mm3: [syp_h sxp_h syp_h sxp_h]
			pmaddwd mm3, mm2       ;mm3: [(d_h*syp_h+hhz_h*sxp_h) (d_l*syp_h+hhz_l*sxp_h)    ]
			pshufw mm5, mm3, 0xe   ;mm5: [        xxx             (d_h*syp_h+hhz_h*sxp_h)    ]
			psrad mm3, 16          ;mm3: [        xxx             (d_l*syp_h+hhz_l*sxp_h)>>16]
			paddd mm3, mm5
			movd eax, mm3
			test eax, eax
			jl short prebegsrchpix
donesrchpix:
			pshufw mm2, mm1, 0xdd  ;mm2: [  d_h hhz_h   d_h hhz_h]
#endif

			sub ecx, edi
			jle short skippix

			movq mm3, mm0
			psubw mm3, mm6
			pmulhw mm3, recipi64[ecx*8] ;NOTE: recipi64[1] is invalid, but doesn't matter
startpix:
			movq mm1, mm6
#if 0
			psraw mm1, 7
#else
			pmulhw mm1, g_qrgbmul
#endif
			packuswb mm1, mm7
			punpckldq mm1, mm2
			movq [edx], mm1 ;write [d...hhz.XxRrGgBb]  (d:sqrt(x^2+y^2), hhz:+/-z, Pyth.dist=sqrt(d^2+hhz^2))
			paddw mm6, mm3
			add edx, pi
			sub ecx, 1
			jnz short startpix
			test edi, edi
			js short endpix
skippix:
			movq mm6, mm0
			jmp short begpix

preendpix:
			punpckldq mm1, de
			add donecnt, 1
			jz short backfrompreendpix

endpix:
			mov eax, index
			mov midbuf1[eax*4], edi

			pop edi
			pop esi
		}
#endif
	}
outsy:;
	_asm emms
}

static long v2h_iymul[MAXXDIM], v2h_iyadd[MAXXDIM];
#endif
static long v2h_lmost[MAXYDIM+8], v2h_rmost[MAXYDIM+8];
#ifndef DISABLEHEIGHTMAP
static long v2h_lmost2[MAXYDIM], v2h_vmost[MAXYDIM];
static long g_j, g_iwx1;
static float g_fy0, g_fy1;

#define USENEWHREND 1
#define USENEWVREND 1
#if ((USENEWHREND != 0) || (USENEWVREND != 0))
typedef struct { long x0, x1, y; } spanlist_t;
#define MAXSPANS (MAXXDIM*64)
static spanlist_t g_spanlist[MAXSPANS]; //FIX: could overflow!
#endif

static void voxhrend (int y)
{
	float f, fx0, fx1;
	long i, j, p, pe, ix, iy, ixi, sx0, sx1, cbuf, zbuf;

	if (y < 0)
	{
#if (TESTDEPTH == 0)
		static long omidbuf = 0, ozbuf = 0, ocbuf = 0;
		if ((long)midbuf != omidbuf)
			{ omidbuf = (long)midbuf; SELFMODVAL(selfmod_midbuf-4,omidbuf+MIDBUFENTRIES*8); }
		if (ozbuf != gcc.z.f)
		{
			ozbuf = gcc.z.f;
			SELFMODVAL(selfmod_zbufa-4,ozbuf);
			SELFMODVAL(selfmod_zbufb-4,ozbuf);
		}
		if (ocbuf != gcc.c.f) { ocbuf = gcc.c.f; SELFMODVAL(selfmod_cbuf-4,ocbuf); }
#endif
		return;
	}

#if (USENEWHREND != 0)
	fx0 = (g_spanlist[y].y-gcy)*g_fy0 + gcx; sx0 = max(fx0,g_spanlist[y].x0); sx0 = max(sx0,0);
	fx1 = (g_spanlist[y].y-gcy)*g_fy1 + gcx; sx1 = min(fx1,g_spanlist[y].x1); sx1 = min(sx1,g_iwx1);
	y = g_spanlist[y].y;
#else
	fx0 = (y-gcy)*g_fy0 + gcx; sx0 = max(fx0,0);
	fx1 = (y-gcy)*g_fy1 + gcx; sx1 = min(fx1,g_iwx1);
#endif
	f = ((float)g_j)*65536.0/(fx1-fx0); ixi = (int)f; ix = (int)((sx0-fx0)*f);

	i = (sx1-sx0)*ixi + ix; j = (g_j<<16);
	while ((sx0 <= sx1) && (i >= j)) { sx1--; i  -= ixi; } //ix < (g_j<<16)
	while ((sx0 <= sx1) && (ix < 0)) { sx0++; ix += ixi; } //ix >= 0
#if (USENEWHREND != 0)
	while (1)
	{
		if (sx0 > sx1) return;
		iy = (ix>>16); if (((unsigned)(y-midbuf0[iy])) < (unsigned)midbuf1[iy]) break;
		sx0++; ix += ixi;
	}
	while (1)
	{
		if (sx0 > sx1) return;
		iy = (i>>16); if (((unsigned)(y-midbuf0[iy])) < (unsigned)midbuf1[iy]) break;
		sx1--; i -= ixi;
	}
#endif

	p = y*gcc.c.p + (sx0<<2); pe = ((sx1-sx0)<<2) + p;
#if (TESTDEPTH != 0)
	cbuf = gcc.c.f; zbuf = gcc.z.f;
	for(;p<=pe;ix+=ixi,p+=4)
	{
		iy = (ix>>16); if (((unsigned)(y-midbuf0[iy])) >= (unsigned)midbuf1[iy]) continue;
		j = midbuf[(iy+1/*FIX*/)*(MIDBUFENTRIES*2)+(y<<1)+1];
		j = ((long)smidprojlut[iy][0])*((j    )>>16) +
			 ((long)smidprojlut[ 0][1])*((j<<16)>>16);
		if (j >= *(long *)(p+zbuf)) continue;
		*(long *)(p+zbuf) = j;
#if (TESTDEPTH == 0)
		*(long *)(p+cbuf) = midbuf[(iy+1/*FIX*/)*(MIDBUFENTRIES*2)+(y<<1)+0];
#elif (TESTDEPTH == 1)
		*(long *)(p+cbuf) = (((int)(j>>12))&255)*0x10101;
#endif
	}
#else
	if (p > pe) return;
	_asm
	{
		push esi
		push edi

		mov esi, y
		mov edi, p
		mov edx, ix
begith:
		mov eax, edx
		shr eax, 16
#if (USENEWHREND == 0)
		mov ecx, esi
		sub ecx, midbuf0[eax*4]
		cmp ecx, midbuf1[eax*4]
		jae short skipith
#endif

		lea ecx, [eax+esi]
		movd mm2, smidprojlut[eax*4]
		shl eax, 14
		movq mm0, [eax+ecx*8+0x88888888] _asm selfmod_midbuf:

		pshufw mm1, mm0, 0xb
		;movd ecx, mm1
		;cmp ecx, 0x04000000 ;glevel
		;jg short skipith
		pmaddwd mm1, mm2
		movd ecx, mm1
		cmp ecx, dword ptr [edi+0x88888888] _asm selfmod_zbufa:
		jge short skipith
		movd dword ptr [edi+0x88888888], mm1 _asm selfmod_zbufb:
		movd dword ptr [edi+0x88888888], mm0 _asm selfmod_cbuf:
skipith:
		add edx, ixi
		add edi, 4
		cmp edi, pe
		jle short begith

		pop edi
		pop esi
		emms
	}
#endif
}

static void voxvrend (int y)
{
	long i, j, ix, cbuf, zbuf, sx0, sx1;

	if (y < 0)
	{
#if (TESTDEPTH == 0)
		static long omidbuf = 0, ozbuf = 0, ocbuf = 0;
		if ((long)midbuf != omidbuf)
			{ omidbuf = (long)midbuf; SELFMODVAL(selfmod_midbuf-4,omidbuf+MIDBUFENTRIES*8); }
		if (ozbuf != gcc.z.f)
		{
			ozbuf = gcc.z.f;
			SELFMODVAL(selfmod_zbufa-4,ozbuf);
			SELFMODVAL(selfmod_zbufb-4,ozbuf);
		}
		if (ocbuf != gcc.c.f) { ocbuf = gcc.c.f; SELFMODVAL(selfmod_cbuf-4,ocbuf); }
#endif
		return;
	}

#if (USENEWVREND != 0)
	sx0 = g_spanlist[y].x0; sx1 = g_spanlist[y].x1; y = g_spanlist[y].y;
	while (1)
	{
		if (sx0 > sx1) return;
		ix = ((v2h_iymul[sx0]*y+v2h_iyadd[sx0])>>16);
		if (((unsigned)(sx0-midbuf0[ix])) < (unsigned)midbuf1[ix]) break;
		sx0++;
	}
	while (1)
	{
		if (sx0 > sx1) return;
		ix = ((v2h_iymul[sx1-1]*y+v2h_iyadd[sx1-1])>>16);
		if (((unsigned)(sx1-1-midbuf0[ix])) < (unsigned)midbuf1[ix]) break;
		sx1--;
	}
#else
	sx0 = v2h_lmost[y];
	sx1 = v2h_rmost[y];
#endif

	i = y*gcc.c.p;
#if (TESTDEPTH != 0)
	cbuf = gcc.c.f + i;
	zbuf = gcc.z.f + i;
	for(i=sx0;i<sx1;i++)
	{
		ix = ((v2h_iymul[i]*y+v2h_iyadd[i])>>16);
		if (((unsigned)(i-midbuf0[ix])) >= (unsigned)midbuf1[ix]) continue;
		j = midbuf[(ix+1/*FIX*/)*(MIDBUFENTRIES*2)+(i<<1)+1];
		j = ((long)smidprojlut[ix][0])*((j    )>>16) +
			 ((long)smidprojlut[ 0][1])*((j<<16)>>16);
		if (j >= *(long *)((i<<2)+zbuf)) continue;
		*(long *)((i<<2)+zbuf) = j;
#if (TESTDEPTH == 0)
		*(long *)((i<<2)+cbuf) = midbuf[(ix+1/*FIX*/)*(MIDBUFENTRIES*2)+(i<<1)+0];
#elif (TESTDEPTH == 1)
		*(long *)((i<<2)+cbuf) = (((int)(j>>12))&255)*0x10101;
#endif
	}
#else
	if (sx0 >= sx1) return;
	_asm
	{
		push edi

		mov edx, sx0
		mov edi, i
begitv:
		mov eax, v2h_iymul[edx*4]
		imul eax, y
		add eax, v2h_iyadd[edx*4]

		shr eax, 16
#if (USENEWVREND == 0)
		mov ecx, edx
		sub ecx, midbuf0[eax*4]
		cmp ecx, midbuf1[eax*4]
		jae short skipitv
#endif

		lea ecx, [eax+edx]
		movd mm2, smidprojlut[eax*4]
		shl eax, 14
		movq mm0, [eax+ecx*8+0x88888888] _asm selfmod_midbuf:

		pshufw mm1, mm0, 0xb
		;movd ecx, mm1
		;cmp ecx, 0x04000000 ;glevel
		;jg short skipitv
		pmaddwd mm1, mm2
		movd ecx, mm1
		cmp ecx, dword ptr [edx*4+edi+0x88888888] _asm selfmod_zbufa:
		jge short skipitv
		movd dword ptr [edx*4+edi+0x88888888], mm1 _asm selfmod_zbufb:
		movd dword ptr [edx*4+edi+0x88888888], mm0 _asm selfmod_cbuf:
skipitv:
		add edx, 1
		cmp edx, sx1
		jl short begitv

		pop edi
		emms
	}
#endif
}

static void drawgrou (tiltyp *tt, point3d *clipoly, long clipn, int darendflags)
{
	point3d otp, midprojsc;
	float wx0, wy0, wx1, wy1, fx0, fy0, fx1, fy1, fmulv, faddv, faddv2, ux, uy, uz;
	float f, g, fx, fy, gx, gy, x0, y0, x1, y1, x2, y2, x3, y3;
	float px, py, vx, vy, det, d;
	long ix, iy, ixi, iyi, oic[2], ic[2], iwx1, iwy1, cbuf, zbuf;
	long i, j, k, l, p, pe, x, y, xx, yy, xs, ys, xe, ye, sx0, sy0, sx1, sy1;
	short smidprojscz;

	gtt = tt; grendflags = darendflags;

	if (gcc.f.z == 0) f = 1048575.f; else f = min(max(gcc.h.z/gcc.f.z,-1048575.f),1048575.f);
	gcx = gcc.r.z*f + gcc.h.x;
	gcy = gcc.d.z*f + gcc.h.y;

	otp = gcc.p; if (!(darendflags&RENDFLAGS_INTERP)) { gcc.p.x += .5; gcc.p.y += .5; }
	aipi[1] = 4; aipi[0] = gtt->p;
	poszupldist = (int)(gcc.p.z*ghsc*DISTPREC);
	qposzupldist = (__int64)(poszupldist - 32768);

	if (!(grendflags&RENDFLAGS_FLIPHEIGHT)) ltexxor = 0; else ltexxor = 0xff000000;
	ltexxord8 = (ltexxor>>8);
	qtexxor = ((__int64)ltexxor);

	gprojadd.x = (-gcc.h.x)*gcc.r.x + (-gcc.h.y)*gcc.d.x + gcc.h.z*gcc.f.x;
	gprojadd.y = (-gcc.h.x)*gcc.r.y + (-gcc.h.y)*gcc.d.y + gcc.h.z*gcc.f.y;
	gprojadd.z = (-gcc.h.x)*gcc.r.z + (-gcc.h.y)*gcc.d.z + gcc.h.z*gcc.f.z;


	f = drawpoly_anginc*0.5;
	wx0 = (float)(f-drawpoly_anginc); wx1 = (float)(gcc.c.x+drawpoly_anginc-1-f);
	wy0 = (float)(f-drawpoly_anginc); wy1 = (float)(gcc.c.y+drawpoly_anginc-1-f);
	iwx1 = gcc.c.x-1;
	iwy1 = gcc.c.y-1;

	fx = wx0-gcx; fy = wy0-gcy; gx = wx1-gcx; gy = wy1-gcy;
	x0 = x3 = wx0; y0 = y1 = wy0; x1 = x2 = wx1; y2 = y3 = wy1;
	if (fy < 0)
	{
		if (fx < 0) { f = sqrt( fx*fy); x0 = gcx-f; y0 = gcy-f; }
		if (gx > 0) { f = sqrt(-gx*fy); x1 = gcx+f; y1 = gcy-f; }
	}
	if (gy > 0)
	{
		if (gx > 0) { f = sqrt( gx*gy); x2 = gcx+f; y2 = gcy+f; }
		if (fx < 0) { f = sqrt(-fx*gy); x3 = gcx-f; y3 = gcy+f; }
	}
	if (x0 > x1) { if (fx < 0) y0 = fx/gx*fy + gcy; else y1 = gx/fx*fy + gcy; }
	if (y1 > y2) { if (fy < 0) x1 = fy/gy*gx + gcx; else x2 = gy/fy*gx + gcx; }
	if (x2 < x3) { if (fx < 0) y3 = fx/gx*gy + gcy; else y2 = gx/fx*gy + gcy; }
	if (y3 < y0) { if (fy < 0) x0 = fy/gy*fx + gcx; else x3 = gy/fy*fx + gcx; }
		//This makes precision errors cause pixels to overwrite rather than omit
	f = 1.0/16.0;
	x0 -= f; x1 += f;
	y1 -= f; y2 += f;
	x3 -= f; x2 += f;
	y0 -= f; y3 += f;

	ux = gcc.r.y*gcc.d.z - gcc.r.z*gcc.d.y;
	uy = gcc.r.z*gcc.d.x - gcc.r.x*gcc.d.z;
	uz = gcc.r.x*gcc.d.y - gcc.r.y*gcc.d.x;
	f = ((float)ZSCALE)/((ux*gcc.f.x + uy*gcc.f.y + uz*gcc.f.z)*gcc.h.z*8.0);
	midprojsc.x = ux*f;
	midprojsc.y = uy*f;
	midprojsc.z = uz*f/ghsc;
	smidprojscz = (short)midprojsc.z;

	for(l=2-1;l>=0;l--)
	{
		if (l) { if (fy >= 0) continue; j = (long)((x1-x0)/drawpoly_anginc); } //top tri (gcx,gcy),(x0,wy0),(x1,wy0)
		  else { if (gy <= 0) continue; j = (long)((x2-x3)/drawpoly_anginc); } //bot tri (gcx,gcy),(x2,wy1),(x3,wy1)

		if (j <= 0) continue;
		if (j > MIDBUFSIZ-1) j = MIDBUFSIZ-1;

		fx0 = gcx; fy0 = gcy;
		if (l) { fy1 = wy0; fmulv = (x1-x0)/((float)j); faddv = fmulv*.5 + x0; }
		  else { fy1 = wy1; fmulv = (x2-x3)/((float)j); faddv = fmulv*.5 + x3; }
		g = 1.0/(fy1-fy0);
		if (fy0 < fy1) { oic[0] = max((int)ceil(fy0),0); oic[1] = min((int)floor(fy1),iwy1); }
					 else { oic[0] = max((int)ceil(fy1),0); oic[1] = min((int)floor(fy0),iwy1); }
		for(i=0;i<j;i++)
		{
			fx1 = ((float)i)*fmulv + faddv;
			f = (fx1-fx0)*g;
			glt[i].sx[0] = ((float)oic[0]-fy0)*f + fx0;
			glt[i].sx[1] = ((float)oic[1]-fy0)*f + fx0;
			k = (glt[i].sx[1] < glt[i].sx[0]);
			if ((glt[i].sx[1-k] < wx0) || (glt[i].sx[k] > wx1)) { glt[i].leng = 0; continue; }
			ic[0] = oic[0]; ic[1] = oic[1];
			if (glt[i].sx[k] < wx0) { ic[k] = (int)floor((wx0-fx0)/f + fy0) + (f >=0); glt[i].sx[k] = ((float)ic[k]-fy0)*f + fx0; }
			k ^= 1;
			if (glt[i].sx[k] > wx1) { ic[k] = (int)floor((wx1-fx0)/f + fy0) + (f < 0); glt[i].sx[k] = ((float)ic[k]-fy0)*f + fx0; }
			glt[i].sy[0] = (float)ic[0];
			glt[i].sy[1] = (float)ic[1];
			glt[i].leng = ic[1]-ic[0]+1;
			if ((gcc.f.z < 0) == l)
			{
				f = glt[i].sx[0]; glt[i].sx[0] = glt[i].sx[1]; glt[i].sx[1] = f;
				f = glt[i].sy[0]; glt[i].sy[0] = glt[i].sy[1]; glt[i].sy[1] = f;
				glt[i].leng = -glt[i].leng;
			}
			glt[i].nvt = &midbuf[(i+1/*FIX*/)*(MIDBUFENTRIES*2)+(ic[0]<<1)+0];

			if (!clipn) { glt[i].d[0] = 0; glt[i].d[1] = CASTDIST; continue; }
			px = otp.x; vx = fx1*gcc.r.x + fy1*gcc.d.x + gprojadd.x;
			py = otp.y; vy = fx1*gcc.r.y + fy1*gcc.d.y + gprojadd.y;
			f = 1.0/sqrt(vx*vx + vy*vy); vx *= f; vy *= f;
			smidprojlut[i][0] = (short)(vx*midprojsc.x + vy*midprojsc.y);
			smidprojlut[i][1] = smidprojscz;

			glt[i].d[0] = CASTDIST;
			glt[i].d[1] = 0;
			for(k=0,p=clipn-1;k<clipn;p=k,k++)
			{
					//px + vx*d = (clipoly[k].x-clipoly[p].x)*e + clipoly[p].x
					//py + vy*d = (clipoly[k].y-clipoly[p].y)*e + clipoly[p].y
					//
					//vx*d + (clipoly[p].x-clipoly[k].x)*e = (clipoly[p].x-px)
					//vy*d + (clipoly[p].y-clipoly[k].y)*e = (clipoly[p].y-py)
				det = vx*(clipoly[p].y-clipoly[k].y) - vy*(clipoly[p].x-clipoly[k].x); if (det == 0) continue;
				f = 1.0/det;
				d = vx*(clipoly[p].y-py) - vy*(clipoly[p].x-px);
				d *= f; if ((d < 0) || (d > 1)) continue; //FIXFIX: fixme
				d = (clipoly[p].x-px)*(clipoly[p].y-clipoly[k].y) - (clipoly[p].y-py)*(clipoly[p].x-clipoly[k].x);
				d *= f;
				if (d < glt[i].d[0]) glt[i].d[0] = d;
				if (d > glt[i].d[1]) glt[i].d[1] = d;
			}
			if (glt[i].d[0] <        0) glt[i].d[0] = 0;
			if (glt[i].d[1] > CASTDIST) glt[i].d[1] = CASTDIST;
		}

		htrun(voxraycast,0,j,drawpoly_numcpu);

		if (l) { f = 1.0/(wy0-gcy); fy0 = (x0-gcx)*f; fy1 = (x1-gcx)*f; ys = 0; ye = min(floor(gcy),iwy1+1); }
		  else { f = 1.0/(wy1-gcy); fy0 = (x3-gcx)*f; fy1 = (x2-gcx)*f; ys = max(ceil(gcy),0); ye = iwy1+1; }
		if ((l) ^ (gcc.f.z < 0))
		{
			for(i=j-1;i>=0;i--)
				{ k = midbuf1[i]; midbuf1[i] = max(midbuf0[i]-k-1,0); midbuf0[i] = glt[i].sy[0]+k+1; }
		}
		else
		{
			for(i=j-1;i>=0;i--)
				{ k = midbuf0[i]; midbuf0[i] = glt[i].sy[0]-k+1; midbuf1[i] = max(-midbuf1[i]+k-1,0); }
		}

		g_j = j; g_fy0 = fy0; g_fy1 = fy1; g_iwx1 = iwx1;
		voxhrend(-1);
#if (USENEWHREND == 0)
		htrun(voxhrend,ys,ye,drawpoly_numcpu);
#else
		if (ys >= ye) continue;
		fmulv *= g; faddv = (faddv-gcx)*g - fmulv*.5; faddv2 = faddv+fmulv*2;
		fx0 = gcx-0.5; fx1 = gcx+0.5; //extend slightly to avoid rounding artifacts
		oic[0] = oic[1] = min(max(midbuf0[0],ys),ye-1); k = 0;
		midbuf0[g_j] = midbuf0[g_j-1]; midbuf1[g_j] = 0; //Hack to avoid cleanup loop at end
		for(i=0;i<=g_j;i++)
		{
			ic[0] = max(midbuf0[i]           ,ys); while (ic[0] < oic[0]) v2h_lmost2[--oic[0]] = i;
			ic[1] = min(midbuf1[i]+midbuf0[i],ye); while (ic[1] > oic[1]) v2h_lmost2[oic[1]++] = i;
			for(j=min(ic[0],oic[1]);oic[0]<j;)
			{
				y = oic[0]++; if (k >= MAXSPANS) continue; f = ((float)y)-gcy;
				g_spanlist[k].x0 = (int)((((float)v2h_lmost2[y])*fmulv+faddv )*f + fx0);
				g_spanlist[k].x1 = (int)((((float)i            )*fmulv+faddv2)*f + fx1);
				g_spanlist[k].y = y; k++;
			}
			for(j=max(ic[1],oic[0]);oic[1]>j;)
			{
				y = --oic[1]; if (k >= MAXSPANS) continue; f = ((float)y)-gcy;
				g_spanlist[k].x0 = (int)((((float)v2h_lmost2[y])*fmulv+faddv )*f + fx0);
				g_spanlist[k].x1 = (int)((((float)i            )*fmulv+faddv2)*f + fx1);
				g_spanlist[k].y = y; k++;
			}
		}
		htrun(voxhrend,0,k,drawpoly_numcpu);
#endif
	}
	for(l=2-1;l>=0;l--)
	{
		if (l) { if (fx >= 0) continue; j = (long)((y3-y0)/drawpoly_anginc); } //lef tri (gcx,gcy),(wx0,y3),(wx0,y0)
		  else { if (gx <= 0) continue; j = (long)((y2-y1)/drawpoly_anginc); } //rig tri (gcx,gcy),(wx1,y1),(wx1,y2)

		if (j <= 0) continue;
		if (j > MIDBUFSIZ-1) j = MIDBUFSIZ-1;

		fx0 = gcx; fy0 = gcy;
		if (l) { fx1 = wx0; fmulv = (y3-y0)/((float)j); faddv = fmulv*.5 + y0; }
		  else { fx1 = wx1; fmulv = (y2-y1)/((float)j); faddv = fmulv*.5 + y1; }
		g = 1.0/(fx1-fx0);
		if (fx0 < fx1) { oic[0] = max((int)ceil(fx0),0); oic[1] = min((int)floor(fx1),iwx1); }
					 else { oic[0] = max((int)ceil(fx1),0); oic[1] = min((int)floor(fx0),iwx1); }
		for(i=0;i<j;i++)
		{
			fy1 = ((float)i)*fmulv + faddv;
			f = (fy1-fy0)*g;
			glt[i].sy[0] = ((float)oic[0]-fx0)*f + fy0;
			glt[i].sy[1] = ((float)oic[1]-fx0)*f + fy0;
			k = (glt[i].sy[1] < glt[i].sy[0]);
			if ((glt[i].sy[1-k] < wy0) || (glt[i].sy[k] > wy1)) { glt[i].leng = 0; continue; }
			ic[0] = oic[0]; ic[1] = oic[1];
			if (glt[i].sy[k] < wy0) { ic[k] = (int)floor((wy0-fy0)/f + fx0) + (f >=0); glt[i].sy[k] = ((float)ic[k]-fx0)*f + fy0; }
			k ^= 1;
			if (glt[i].sy[k] > wy1) { ic[k] = (int)floor((wy1-fy0)/f + fx0) + (f < 0); glt[i].sy[k] = ((float)ic[k]-fx0)*f + fy0; }
			glt[i].sx[0] = (float)ic[0];
			glt[i].sx[1] = (float)ic[1];
			glt[i].leng = ic[1]-ic[0]+1;
			if ((gcc.f.z < 0) == l)
			{
				f = glt[i].sx[0]; glt[i].sx[0] = glt[i].sx[1]; glt[i].sx[1] = f;
				f = glt[i].sy[0]; glt[i].sy[0] = glt[i].sy[1]; glt[i].sy[1] = f;
				glt[i].leng = -glt[i].leng;
			}
			glt[i].nvt = &midbuf[(i+1/*FIX*/)*(MIDBUFENTRIES*2)+(ic[0]<<1)+0];

			if (!clipn) { glt[i].d[0] = 0; glt[i].d[1] = CASTDIST; continue; }
			px = otp.x; vx = fx1*gcc.r.x + fy1*gcc.d.x + gprojadd.x;
			py = otp.y; vy = fx1*gcc.r.y + fy1*gcc.d.y + gprojadd.y;
			f = 1.0/sqrt(vx*vx + vy*vy); vx *= f; vy *= f;
			smidprojlut[i][0] = (short)(vx*midprojsc.x + vy*midprojsc.y);
			smidprojlut[i][1] = smidprojscz;

			glt[i].d[0] = CASTDIST;
			glt[i].d[1] = 0;
			for(k=0,p=clipn-1;k<clipn;p=k,k++)
			{
					//px + vx*d = (clipoly[k].x-clipoly[p].x)*e + clipoly[p].x
					//py + vy*d = (clipoly[k].y-clipoly[p].y)*e + clipoly[p].y
					//
					//vx*d + (clipoly[p].x-clipoly[k].x)*e = (clipoly[p].x-px)
					//vy*d + (clipoly[p].y-clipoly[k].y)*e = (clipoly[p].y-py)
				det = vx*(clipoly[p].y-clipoly[k].y) - vy*(clipoly[p].x-clipoly[k].x); if (det == 0) continue;
				f = 1.0/det;
				d = vx*(clipoly[p].y-py) - vy*(clipoly[p].x-px);
				d *= f; if ((d < 0.0) || (d > 1.0)) continue; //FIXFIX: fixme
				d = (clipoly[p].x-px)*(clipoly[p].y-clipoly[k].y) - (clipoly[p].y-py)*(clipoly[p].x-clipoly[k].x);
				d *= f;
				if (d < glt[i].d[0]) glt[i].d[0] = d;
				if (d > glt[i].d[1]) glt[i].d[1] = d;
			}
			if (glt[i].d[0] <        0) glt[i].d[0] = 0;
			if (glt[i].d[1] > CASTDIST) glt[i].d[1] = CASTDIST;
		}

		htrun(voxraycast,0,j,drawpoly_numcpu);

		if (l) { f = 1.0/(wx0-gcx); fx0 = (y0-gcy)*f; fx1 = (y3-gcy)*f; xs = 0; xe = min(floor(gcx),iwx1); }
		  else { f = 1.0/(wx1-gcx); fx0 = (y1-gcy)*f; fx1 = (y2-gcy)*f; xs = max(ceil(gcx),0); xe = iwx1; }
		if (xs > xe) continue;
		if ((l) ^ (gcc.f.z < 0))
		{
			for(i=j-1;i>=0;i--)
				{ k = midbuf1[i]; midbuf1[i] = max(midbuf0[i]-k-1,0); midbuf0[i] = glt[i].sx[0]+k+1; }
		}
		else
		{
			for(i=j-1;i>=0;i--)
				{ k = midbuf0[i]; midbuf0[i] = glt[i].sx[0]-k+1; midbuf1[i] = max(-midbuf1[i]+k-1,0); }
		}

		y = ye = min(max((int)gcy,0),iwy1);

		for(x=xs;x<=xe;x++)
		{
			fy0 = (x-gcx)*fx0 + gcy; sy0 = max(fy0,0);
			fy1 = (x-gcx)*fx1 + gcy; sy1 = min(fy1,iwy1);
			f = ((float)j)*65536.0/(fy1-fy0); iyi = (int)f; iy = (int)((sy0-fy0)*f);

			v2h_iymul[x] = iyi;
			v2h_iyadd[x] = iy-sy0*iyi;

			i = (sy1-sy0)*iyi + iy - (j<<16);
			while ((sy0 <= sy1) && (i >= 0)) { sy1--; i  -= iyi; } //iy < (j<<16)
			while ((sy0 <= sy1) && (iy < 0)) { sy0++; iy += iyi; } //iy >= 0

			sy0 = min(max(sy0  ,  0),iwy1+1);
			sy1 = min(max(sy1+1,sy0),iwy1+1);

			if (l) { if (x == xs) { ic[0] = sy0; ic[1] = sy1; } }
			  else { if (x == xe) { ic[0] = sy0; ic[1] = sy1; } }

			while (sy0 <  y) v2h_lmost[ --y] = x;
			while (sy1 > ye) v2h_lmost[ye++] = x;
			while (sy0 >  y) v2h_rmost[y++ ] = x;
			while (sy1 < ye) v2h_rmost[--ye] = x;
		}
		while (y < ye) { v2h_rmost[y] = x; y++; }

		voxvrend(-1);
#if (USENEWVREND == 0)
		htrun(voxvrend,ic[0],ic[1],drawpoly_numcpu);
#else
		g_j = j;
		xe++; if (xs >= xe) continue;

		{
		float fxi, fxp, ox[2], oy[2], nx[2], ny[2];

		fmulv *= g; faddv = (faddv-gcy)*g - fmulv*.5; faddv2 = faddv+fmulv;
		fy0 = gcy-.5; fy1 = gcy+.5; k = 0; //extend slightly to avoid rounding artifacts
		midbuf0[g_j] = midbuf0[g_j-1]; midbuf1[g_j] = 0; //Hack to avoid cleanup loop at end
		i = 0; ox[0] = max(midbuf0[i],xs); oy[0] = (((float)i)*fmulv+faddv)*(((float)ox[0])-gcx) + fy0;
		ox[1] = ox[0]; oy[1] = oy[0];
		for(i=0;i<=g_j;i++)
		{
			nx[0] = midbuf0[i]; //max(midbuf0[i],xs);
			ny[0] = (((float)i)*fmulv+faddv)*(((float)nx[0])-gcx) + fy0;
			if (midbuf1[i] > 0)
			{
				nx[1] = midbuf1[i]+midbuf0[i]; //min(midbuf1[i]+midbuf0[i],xe);
				ny[1] = (((float)i)*fmulv+faddv2)*(((float)nx[1])-gcx) + fy1;
			} else { nx[1] = nx[0]; ny[1] = ny[0]; }

			for(p=2-1;p>=0;p--)
			{
				y = (int)(oy[p]); ye = (int)(ny[p]); if (y > ye) { j = y; y = ye; ye = j; }
				y = max(y,ic[0]); ye = min(ye,ic[1]);
				//if (0)
				//   { fxi = (nx[p]-ox[p])/(ny[p]-oy[p]); fxp = ((float)y-oy[p])*fxi+ox[p]; }
				//else
				//{
				//   fxi = 0;
				//   if ((oy[p] > ny[p]) == p) fxp = min(nx[p],ox[p]);
				//                        else fxp = max(nx[p],ox[p]);
				//}
				if ((oy[p] > ny[p]) == p) pe = (int)min(nx[p],ox[p]);
											else pe = (int)max(nx[p],ox[p]);
				for(;y<ye;y++) //,fxp+=fxi)
				{
					//j = min(max((int)fxp,v2h_lmost[y]),v2h_rmost[y]);
					j = min(max(pe,v2h_lmost[y]),v2h_rmost[y]);
					if (v2h_vmost[y] < 0) { v2h_vmost[y] = j; continue; }
					x = v2h_vmost[y]; v2h_vmost[y] = -1; if (k >= MAXSPANS) continue;
					if (j < x) { g_spanlist[k].x0 = j; g_spanlist[k].x1 = x; }
							else { g_spanlist[k].x0 = x; g_spanlist[k].x1 = j; }
					g_spanlist[k].y = y; k++;
				}
				ox[p] = nx[p]; oy[p] = ny[p];
			}
		}
#if 0
		for(i=0;i<k;i++)
		{
			y = g_spanlist[i].y*gcc.c.p + gcc.c.f;
			for(x=g_spanlist[i].x0;x<g_spanlist[i].x1;x++) *(long *)(y + (x<<2)) += 0x3f3d37;
		}
#else
		htrun(voxvrend,0,k,drawpoly_numcpu);
#endif
		}
#endif
	}
	gcc.p = otp;
}

static long getpolycover (vertyp *pt, long n, float dep0, float dep1,
								  long *minx, long *miny, long *maxx, long *maxy, long *lmost, long *rmost)
{
	point3d tp, *np, *cp, *scv;
	float f, g, ax[2], ay[2], az[2], hx, hy, hz;
	long i, j, k, sx0, sx1, sx, sy, iy0, iy1, *dir, hstat, cpn;

	if (n < 3) return(0);
	np  = (point3d *)_alloca(sizeof( np[0])*n);
	dir = (long    *)_alloca(sizeof(dir[0])*n);
	cp  = (point3d *)_alloca(sizeof( cp[0])*n*2);
	scv = (point3d *)_alloca(sizeof(scv[0])*n*2);

	for(i=0;i<n;i++)
	{
		tp.x = pt[i].x-gcc.p.x;
		tp.y = pt[i].y-gcc.p.y;
		tp.z = pt[i].z-gcc.p.z;
		np[i].x = tp.x*gcc.r.x + tp.y*gcc.r.y + tp.z*gcc.r.z;
		np[i].y = tp.x*gcc.d.x + tp.y*gcc.d.y + tp.z*gcc.d.z;
		np[i].z = tp.x*gcc.f.x + tp.y*gcc.f.y + tp.z*gcc.f.z;
	}

	hx = (np[1].z-np[0].z)*(np[2].y-np[0].y) - (np[1].y-np[0].y)*(np[2].z-np[0].z);
	hy = (np[1].x-np[0].x)*(np[2].z-np[0].z) - (np[1].z-np[0].z)*(np[2].x-np[0].x);
	hz = (np[1].y-np[0].y)*(np[2].x-np[0].x) - (np[1].x-np[0].x)*(np[2].y-np[0].y);
	f = 1.0/sqrt(hx*hx + hy*hy + hz*hz); hx *= f; hy *= f; hz *= f;

		//Find convex polygon in screen coordinates (cp[],cpn) of convex translational sweep
	for(i=n-1,j=0;i>=0;j=i,i--)
	{
		tp.x = (np[j].y-np[i].y)*hz - (np[j].z-np[i].z)*hy;
		tp.y = (np[j].z-np[i].z)*hx - (np[j].x-np[i].x)*hz;
		tp.z = (np[j].x-np[i].x)*hy - (np[j].y-np[i].y)*hx;
		dir[i] = (np[i].x*tp.x + np[i].y*tp.y + np[i].z*tp.z >= 0);
	}

	g = -(np[0].x*hx + np[0].y*hy + np[0].z*hz);
		  if (g > dep1) hstat = 0;
	else if (g < dep0)
	{
			//Back-face cull
		for(i=n-1;i>=0;i--) if (dir[i]) break;
		if (i < 0) return(0);
		hstat = 1;
	}
	else
	{
		for(i=n-1;i>=0;i--) if (dir[i]) break;
		if (i < 0) //Render whole screen :/
		{
			(*minx) = 0; (*maxx) = gcc.c.x;
			(*miny) = 0; (*maxy) = gcc.c.y;
			for(sy=gcc.c.y-1;sy>=0;sy--) { lmost[sy] = 0; rmost[sy] = gcc.c.x; }
			return(1);
		}
		hstat = 2;
	}

	ax[0] = hx*dep0; ay[0] = hy*dep0; az[0] = hz*dep0;
	ax[1] = hx*dep1; ay[1] = hy*dep1; az[1] = hz*dep1;

	cpn = 0;
	if (hstat < 2)
	{
		for(i=n-1,j=0;i>=0;j=i,i--)
		{
			if (dir[i] != dir[j])
			{
				k = dir[i]^hstat;
				cp[cpn].x = np[j].x+ax[k];
				cp[cpn].y = np[j].y+ay[k];
				cp[cpn].z = np[j].z+az[k]; cpn++;
			}
			k = dir[i]^hstat^1;
			cp[cpn].x = np[j].x+ax[k];
			cp[cpn].y = np[j].y+ay[k];
			cp[cpn].z = np[j].z+az[k]; cpn++;
		}
		if (!hstat) //Make it CW order for backside
			for(i=(cpn>>1)-1;i>=0;i--)
				{ tp = cp[i]; cp[i] = cp[cpn-1-i]; cp[cpn-1-i] = tp; }
	}
	else
	{
		for(i=n-1,j=0;i>=0;j=i,i--)
			if (dir[j] > dir[i]) break;
		do
		{
			cp[cpn].x = np[j].x+ax[0];
			cp[cpn].y = np[j].y+ay[0];
			cp[cpn].z = np[j].z+az[0]; cpn++;
			j++; if (j >= n) j = 0;
		} while (dir[j]);
		cp[cpn].x = np[j].x+ax[0];
		cp[cpn].y = np[j].y+ay[0];
		cp[cpn].z = np[j].z+az[0]; cpn++;
		for(i=cpn-1;i>=0;i--)
		{
			cp[cpn].x = cp[i].x+ax[1]-ax[0];
			cp[cpn].y = cp[i].y+ay[1]-ay[0];
			cp[cpn].z = cp[i].z+az[1]-az[0]; cpn++;
		}
	}

	(*minx) = gcc.c.x; (*maxx) = 0;
	(*miny) = gcc.c.y; (*maxy) = 0;
#if 0
		//FIX: this algo needs to clip to the Z-plane!
	for(i=cpn-1;i>=0;i--)
	{
		cp[i].z = gcc.hz/cp[i].z;
		cp[i].x = cp[i].x*cp[i].z + gcc.hx;
		cp[i].y = cp[i].y*cp[i].z + gcc.hy;
	}

	for(j=0,i=cpn-1;i>=0;j=i,i--)
	{
		iy0 = min(max(ceil(cp[i].y),0),gcc.c.y);
		iy1 = min(max(ceil(cp[j].y),0),gcc.c.y); if (iy0 == iy1) continue;
		g = (cp[j].x-cp[i].x)/(cp[j].y-cp[i].y);
		if (iy0 < iy1)
		{
			if (iy0 < (*miny)) (*miny) = iy0;
			if (iy1 > (*maxy)) (*maxy) = iy1;
			f = (iy0-cp[i].y)*g + cp[i].x;
			for(;iy0<iy1;iy0++,f+=g)
				{ rmost[iy0] = (int)min(f,gcc.c.x); if (rmost[iy0] > (*maxx)) (*maxx) = rmost[iy0]; }
		}
		else
		{
			f = (iy1-cp[i].y)*g + cp[i].x;
			for(;iy1<iy0;iy1++,f+=g)
				{ lmost[iy1] = (int)max(f,0); if (lmost[iy1] < (*minx)) (*minx) = lmost[iy1]; }
		}
	}
#else
		//Calculate screen vectors
	for(j=0,i=cpn-1;i>=0;j=i,i--)
	{
		scv[i].x = cp[i].y*cp[j].z - cp[i].z*cp[j].y;
		scv[i].y = cp[i].z*cp[j].x - cp[i].x*cp[j].z;
		scv[i].z = cp[i].x*cp[j].y - cp[i].y*cp[j].x;
		if (*(long *)&scv[i].x != 0) { f = 1.0/scv[i].x; scv[i].y *= f; scv[i].z *= f; }
		scv[i].z = ((float)gcc.h.x) - scv[i].z*((float)gcc.h.z);
	}

	//tp.z = gcc.hz;
	for(sy=0;sy<gcc.c.y;sy++)
	{
		tp.y = (float)sy-gcc.h.y;

			//Find lmost&rmost using scv[]
		sx0 = 0; sx1 = gcc.c.x;
		for(i=cpn-1;i>=0;i--)
		{
			//sx = (long)(gcc.hx - scv[i].y*tp.y - scv[i].z*tp.z);
			sx = (long)(scv[i].z - scv[i].y*tp.y);
			if (*(long *)&scv[i].x == 0) { if (sx > gcc.h.x) { sx1 = 0; break; } continue; }
			if (*(long *)&scv[i].x > 0) { if (sx > sx0) sx0 = sx; }
										  else { if (sx < sx1) sx1 = sx; }
		}
		lmost[sy] = sx0;
		rmost[sy] = sx1;
		if (sx0 >= sx1) continue;
		if (sx0  < (*minx)) (*minx) = sx0;
		if (sx1  > (*maxx)) (*maxx) = sx1;
		if (sy   < (*miny)) (*miny) = sy;
		if (sy+1 > (*maxy)) (*maxy) = sy+1;
	}
#endif
	return(((*minx) < (*maxx)) && ((*miny) < (*maxy)));
}

static void drawpolytopo (tiltyp *tt, vertyp *p, int n, int rgbmul, float hsc, float *ouvmat, int darendflags)
{
	cam_t occ;
	point3d *np;
	float f, g, ox, oy, oz, ux, uy, uz, vx, vy, vz, ax, ay, az, au, av, bx, by, bz, bu, bv, cu, cv;
	float nx, ny, nz;
	long i, j, sx, sy, minx, miny, maxx, maxy, lmost[MAXYDIM], rmost[MAXYDIM], revmat;

#ifdef USEINTZ
	f = tt->shsc*hsc*32.0;
#else
	f = tt->shsc*hsc/8.0; //FIXFIXFIXFIX
#endif
	if (!getpolycover(p,n,0,f,&minx,&miny,&maxx,&maxy,lmost,rmost)) return;

	if ((long)midbuf == -1) return;
	if (!midbuf)
	{
		midbuf = (long *)malloc(MIDBUFSIZ*MIDBUFENTRIES*2*sizeof(long)); //FIXFIX: free(midbuf)!
		if (!midbuf) { midbuf = (long *)-1; return; }
	}

#if 0
		//Debug only
	for(sy=miny;sy<maxy;sy++)
		if (lmost[sy] < rmost[sy])
			memset((void *)(gcc.c.p*sy + (lmost[sy]<<2) + gcc.c.f),0x80,(rmost[sy]-lmost[sy])<<2);
#endif

	occ = gcc;
	ghsc = ((float)(256>>LHSCALE))/f;
	np = (point3d *)_alloca(sizeof(np[0])*n);

	nx = (p[1].z-p[0].z)*(p[2].y-p[0].y) - (p[1].y-p[0].y)*(p[2].z-p[0].z);
	ny = (p[1].x-p[0].x)*(p[2].z-p[0].z) - (p[1].z-p[0].z)*(p[2].x-p[0].x);
	nz = (p[1].y-p[0].y)*(p[2].x-p[0].x) - (p[1].x-p[0].x)*(p[2].y-p[0].y);
	f /= sqrt(nx*nx + ny*ny + nz*nz); nx *= f; ny *= f; nz *= f;

	gcc.c.x = maxx-minx; gcc.c.y = maxy-miny; gcc.c.f += miny*gcc.c.p + (minx<<2);
	gcc.z.x = maxx-minx; gcc.z.y = maxy-miny; gcc.z.f += miny*gcc.z.p + (minx<<2);
	gcc.h.x -= (float)minx;
	gcc.h.y -= (float)miny;

	for(i=0;i<n;i++)
	{
		vx = p[i].x-occ.p.x+nx;
		vy = p[i].y-occ.p.y+ny;
		vz = p[i].z-occ.p.z+nz;
		np[i].x = vx*occ.r.x + vy*occ.r.y + vz*occ.r.z;
		np[i].y = vx*occ.d.x + vy*occ.d.y + vz*occ.d.z;
		np[i].z = vx*occ.f.x + vy*occ.f.y + vz*occ.f.z;
	}

		//Parametric plane equation
	ax = np[1].x-np[0].x; bx = np[2].x-np[0].x;
	ay = np[1].y-np[0].y; by = np[2].y-np[0].y;
	az = np[1].z-np[0].z; bz = np[2].z-np[0].z;
	au =  p[1].u- p[0].u; bu =  p[2].u- p[0].u;
	av =  p[1].v- p[0].v; bv =  p[2].v- p[0].v;

		//.5 stuff is hack for QIfist rounding
	cu = p[0].u - .5/((float)tt->x);
	cv = p[0].v - .5/((float)tt->y);

	f = au*bv - av*bu; if (f == 0) return;
	f = 1.0/f; au *= f; av *= f; bu *= f; bv *= f;
	revmat = (f < 0.0);
	f = cv*bu - cu*bv;
	g = cu*av - cv*au;
	ox = ax*f + bx*g + np[0].x; ux = ax*bv - bx*av; vx = bx*au - ax*bu;
	oy = ay*f + by*g + np[0].y; uy = ay*bv - by*av; vy = by*au - ay*bu;
	oz = az*f + bz*g + np[0].z; uz = az*bv - bz*av; vz = bz*au - az*bu;

	gcc.r.z = uz*vy - uy*vz;
	gcc.d.z = ux*vz - uz*vx;
	gcc.f.z = uy*vx - ux*vy;
	f = -1.0/sqrt(gcc.r.z*gcc.r.z + gcc.d.z*gcc.d.z + gcc.f.z*gcc.f.z);
	if (revmat) f = -f;
	gcc.r.z *= f; gcc.d.z *= f; gcc.f.z *= f;

	g = ((float)tt->x)*f;
	gcc.r.x = (vz*gcc.d.z - vy*gcc.f.z)*g;
	gcc.d.x = (vx*gcc.f.z - vz*gcc.r.z)*g;
	gcc.f.x = (vy*gcc.r.z - vx*gcc.d.z)*g;

	g = ((float)tt->y)*f;
	gcc.r.y = (uy*gcc.f.z - uz*gcc.d.z)*g;
	gcc.d.y = (uz*gcc.r.z - ux*gcc.f.z)*g;
	gcc.f.y = (ux*gcc.d.z - uy*gcc.r.z)*g;

	gcc.p.x = -(ox*gcc.r.x + oy*gcc.d.x + oz*gcc.f.x);
	gcc.p.y = -(ox*gcc.r.y + oy*gcc.d.y + oz*gcc.f.y);
	gcc.p.z = -(ox*gcc.r.z + oy*gcc.d.z + oz*gcc.f.z);

	for(i=0;i<n;i++)
	{
		vx = np[i].x; vy = np[i].y; vz = np[i].z;
		np[i].x = vx*gcc.r.x + vy*gcc.d.x + vz*gcc.f.x + gcc.p.x;
		np[i].y = vx*gcc.r.y + vy*gcc.d.y + vz*gcc.f.y + gcc.p.y;
	 //np[i].z = vx*gcc.r.z + vy*gcc.d.z + vz*gcc.f.z + gcc.p.z;
	}

	g_rgbmul = rgbmul;
	_asm
	{
		pxor mm7, mm7
		movd mm0, rgbmul
		punpcklbw mm0, mm7
		psllw mm0, 2
		movq g_qrgbmul, mm0
		emms
	}
	drawgrou(tt,np,n,darendflags);
	gcc = occ;

#if 0
		//Debug only
	ux = p[1].x-p[0].x; vx = p[2].x-p[0].x;
	uy = p[1].y-p[0].y; vy = p[2].y-p[0].y;
	uz = p[1].z-p[0].z; vz = p[2].z-p[0].z;
	ax = uy*vz - uz*vy;
	ay = uz*vx - ux*vz;
	az = ux*vy - uy*vx;
	f = 1.0/sqrt(ax*ax + ay*ay + az*az); ax *= f; ay *= f; az *= f;

	f = tt->shsc*hsc*32;
	for(i=0,j=n-1;i<n;j=i,i++)
	{
		drawline3d(nx+p[i].x     ,ny+p[i].y     ,nz+p[i].z     ,nx+p[j].x     ,ny+p[j].y     ,nz+p[j].z     ,0xffffff);
		drawline3d(nx+p[i].x+ax*f,ny+p[i].y+ay*f,nz+p[i].z+az*f,nx+p[j].x+ax*f,ny+p[j].y+ay*f,nz+p[j].z+az*f,0xffffff);
		drawline3d(nx+p[i].x     ,ny+p[i].y     ,nz+p[i].z     ,nx+p[i].x+ax*f,ny+p[i].y+ay*f,nz+p[i].z+az*f,0xffffff);
	}
#endif
}
#endif

#define LFLATSTEPSIZ 3
#define FLATSTEPSIZ (1<<LFLATSTEPSIZ)
static float g_mat[9], g_di8, g_ui8, g_vi8, fone = 1.f, grflatstepsiz = (1.f/((float)FLATSTEPSIZ));
static float freciplut[8] = {0.f,1.f/1.f,1.f/2.f,1.f/3.f,1.f/4.f,1.f/5.f,1.f/6.f,1.f/7.f};
static long reciplut[8] = {0,65536/1,65536/2,65536/3,65536/4,65536/5,65536/6,65536/7};
static long g_ttps, g_ymsk, g_xmsk, g_ttf, g_ttp;
__declspec(align(16)) static float qmul[8][4] =
{
	 .000f,.000f,.000f,.000f, .125f,.125f,.125f,.125f, .250f,.250f,.250f,.250f, .375f,.375f,.375f,.375f,
	 .500f,.500f,.500f,.500f, .625f,.625f,.625f,.625f, .750f,.750f,.750f,.750f, .875f,.875f,.875f,.875f,
};
#define MAXHLINS 16384
static signed short ghlinbuf[MAXHLINS][3];

static void htflatnear (int hind)
{
	float f, d, u, v, vx, vy, di8, ui8, vi8;
	long id, iu, iv, idi, iui, ivi, p, p2, pe, sx0, sx1, sy;
	long *cbuf, *zbuf, ttps, ymsk, xmsk, ttf;
	char ch;

	if (hind < 0)
	{
#if (!((TESTDEPTH != 0) || (_MSC_VER == 0)))
		if (hind == -2)
		{
			static long ozbuf = 0, ocbuf = 0;
			if (gcc.z.f != ozbuf)
			{
				ozbuf = gcc.z.f;
				SELFMODVAL(selfmod_zbuf0 -4,((long)ozbuf));
				SELFMODVAL(selfmod_zbuf1 -4,((long)ozbuf));
				SELFMODVAL(selfmod_zbuf0m-4,((long)ozbuf));
				SELFMODVAL(selfmod_zbuf1m-4,((long)ozbuf));
			}
			if (gcc.c.f != ocbuf)
			{
				ocbuf = gcc.c.f;
				SELFMODVAL(selfmod_cbuf -4,ocbuf);
				SELFMODVAL(selfmod_cbufm-4,ocbuf);
			}
		}
		else
		{
			if (!(grendflags&(RENDFLAGS_ALPHAMASK|RENDFLAGS_RALPHAMASK)))
			{
				static long ottps = -1, ottf = 0, onodt = 0;
				if (g_ttps != ottps) { ottps = g_ttps; SELFMODVAL(selfmod_shift-1,((char)ottps)); }
				if (g_ttf != ottf) { ottf = g_ttf; SELFMODVAL(selfmod_ttf-4,ottf); }
				if ((grendflags&RENDFLAGS_NODEPTHTEST) != onodt)
				{
					onodt = (grendflags&RENDFLAGS_NODEPTHTEST);
						//3b 94 37 88 88 88 88     cmp edx, [edi+esi+0x88888888]
						//0f 2f ac 37 88 88 88 88  comiss xmm5, [edi+esi+0x88888888]
						//0f 2e ac 37 88 88 88 88  ucomiss xmm5, [edi+esi+0x88888888]
						//
						//7f/7d ??                 jg/jge short ??
						//
						//eb ?? xx xx xx xx xx     jmp short selfmod_zbuf0_skip:
						//xx xx
						//selfmod_zbuf0_skip:
						//
						//8d 84 37 88 88 88 88     lea eax, [edi+esi+0x88888888]  <--this is faster with Evaldraw
						//3e 8d 84 37 88 88 88 88  lea eax, ds:[edi+esi+0x88888888]
						//
						//f9                       stc
						//8b 84 37 88 88 88 88     mov eax, [edi+esi+0x88888888]  <--this is faster with shadowtest2
						//3e 8b 84 37 88 88 88 88  mov eax, ds:[edi+esi+0x88888888]
						//
						//eb [bytes2skip]          jmp short [label]

#ifdef USEINTZ
					if (!onodt) { SELFMODVAL(selfmod_zbuf0-7,((short)0x943b)); }
							 else { SELFMODVAL(selfmod_zbuf0-7,((short)0x848b)); }
#else
					if (!onodt) { SELFMODVAL(selfmod_zbuf0-8,((short)0x2e0f)); }
							 else { SELFMODVAL(selfmod_zbuf0-8,((short)0x08eb)); } //WARNING: Assumes short 2-byte jcc instruction follows!
#endif
				}
			}
			else
			{
				static long ottpsm = -1, ottfm = 0, onodtm = 0;
				static char ojns_js = 0x79;
				if (g_ttps != ottpsm) { ottpsm = g_ttps; SELFMODVAL(selfmod_shiftm-1,((char)ottpsm)); }
				if (g_ttf != ottfm) { ottfm = g_ttf; SELFMODVAL(selfmod_ttfm-4,ottfm); }
				if ((grendflags&RENDFLAGS_NODEPTHTEST) != onodtm)
				{
					onodtm = (grendflags&RENDFLAGS_NODEPTHTEST);
#ifdef USEINTZ
					if (!onodtm) { SELFMODVAL(selfmod_zbuf0m-7,((short)0x943b)); }
							  else { SELFMODVAL(selfmod_zbuf0m-7,((short)0x848b)); }
#else
					if (!onodtm) { SELFMODVAL(selfmod_zbuf0m-8,((short)0x2e0f)); }
							  else { SELFMODVAL(selfmod_zbuf0m-8,((short)0x08eb)); } //WARNING: Assumes short 2-byte jcc instruction follows!
#endif
				}

					//0x78 [byteoffs]  js  short lab
					//0x79 [byteoffs]  jns short lab
				if (grendflags&RENDFLAGS_RALPHAMASK) p = 0x78; else p = 0x79;
				if (p != ojns_js) { ojns_js = p; SELFMODVAL(selfmod_jns_js-2,ojns_js); }
			}
		}
#endif
		return;
	}

	sx0 = (long)ghlinbuf[hind][0];
	sx1 = (long)ghlinbuf[hind][1];
	sy  = (long)ghlinbuf[hind][2];

	p = (gcc.c.p>>2)*sy; pe = sx1+p; p += sx0;
	if (p >= pe) return;

	zbuf = (long *)gcc.z.f;
	cbuf = (long *)gcc.c.f;
	ttf = g_ttf; ttps = g_ttps; xmsk = g_xmsk; ymsk = g_ymsk;

	vx = (float)sx0; vy = (float)sy;
	d = g_mat[0]*vx + g_mat[3]*vy + g_mat[6];
	u = g_mat[1]*vx + g_mat[4]*vy + g_mat[7];
	v = g_mat[2]*vx + g_mat[5]*vy + g_mat[8];
	f = 1.0/d;
	id = (long)(  f);
	iu = (long)(u*f);
	iv = (long)(v*f);
#if ((TESTDEPTH != 0) || (_MSC_VER == 0))
	d += g_di8;
	do
	{
		f = 1.0/d; u += g_ui8; v += g_vi8; d += g_di8;
		idi = ((((long)(  f))-id)>>LFLATSTEPSIZ);
		iui = ((((long)(u*f))-iu)>>LFLATSTEPSIZ);
		ivi = ((((long)(v*f))-iv)>>LFLATSTEPSIZ);
		p2 = min(p+FLATSTEPSIZ,pe);
		do //Nearest. Note:Brute force attempt at asm failed
		{
#ifdef USEINTZ
			if (id < zbuf[p])
			{
				zbuf[p] = id;
#if (TESTDEPTH == 0)
				cbuf[p] = rgb_scale(*(long *)(((iv>>ttps)&ymsk) + ((iu>>14)&xmsk) + ttf),g_rgbmul);
#else
				cbuf[p] = ((id>>12)&255)*0x10101;
#endif
#else
			if (f < *(float *)&zbuf[p]) //FIX:not calculating d per pixel :/
			{
				*(float *)&zbuf[p] = f;
#if (TESTDEPTH == 0)
				cbuf[p] = rgb_scale(*(long *)(((iv>>ttps)&ymsk) + ((iu>>14)&xmsk) + ttf),g_rgbmul);
#else
				cbuf[p] = (((long)(f*256.0))&255)*0x10101;
#endif
#endif
			}
			id += idi; iu += iui; iv += ivi; p++;
		} while (p < p2);
	} while (p < pe);
#else
	xmsk >>= 2; p <<= 2; pe <<= 2;
	_asm
	{
		push ebx
		push esi
		push edi

		movss xmm1, v
		movss xmm0, u
		unpcklps xmm1, xmm0 ;xmm1: [. . u v]
		movss xmm0, d
		movlhps xmm0, xmm1 ;xmm0: [u v . d]

		movss xmm2, g_vi8
		movss xmm1, g_ui8
		unpcklps xmm2, xmm1 ;xmm2: [. . ui8 vi8]
		movss xmm1, g_di8
		movlhps xmm1, xmm2 ;xmm1: [ui8 vi8 . di8]

		movd mm0, iv
		punpckldq mm0, iu ;mm0: [iu iv]
#ifdef USEINTZ
		mov edx, id
#else
		rcpss xmm5, xmm0
		;movss xmm5, f ;WARNING: A full day of debugging resulted in: VC6 thinks 'f' is done with, and is corrupt (-1.#IND) at this point
#endif

		movd mm3, g_rgbmul
		punpcklbw mm3, mm3
		psrlw mm3, 7

		mov edi, p
//--------------------------------------------------------------------------------------------------
//Align steps to right side to avoid horizon hopping

		mov eax, d
		xor eax, g_di8
		jns short prebegit1
		mov eax, pe
		sub eax, p
		shr eax, 2
		and eax, FLATSTEPSIZ-1
		jz short prebegit1

			; edx:             [ id]
			; edi:             [ p ]
			; mm0:         [ iu  iv]
			; mm3:         [ RmGmBm]
			;xmm0: [ u   v   .   d ]
			;xmm1: [ui8 vi8  .  di8]
		shl eax, 2
		movaps xmm4, xmm1
		mulps xmm4, qmul[eax*4]
		addps xmm0, xmm4   ;u += g_ui8*frac; v += g_vi8*frac; d += g_di8*frac;
		;rcpss xmm2, xmm0 ;noticably faster but looks too crappy
		movss xmm2, fone
		divss xmm2, xmm0   ;f = 1.0/d;
		movhlps xmm3, xmm0 ;xmm3: [. . u v]
		shufps xmm2, xmm2, 0 ;xmm2: [f f f f]
		mulps xmm3, xmm2   ;xmm3: [. . u*f v*f]

		lea esi, [edi+eax]

		cvtps2pi mm1, xmm3 ;mm0: [(int)u*f (int)v*f]
		psubd mm1, mm0

#ifndef USEINTZ
		subss xmm2, xmm5
		mulss xmm2, freciplut[eax]
#endif

		mov ecx, reciplut[eax]

		movd eax, mm1
		imul ecx
		shrd eax, edx, 16
		movd mm2, eax

		psrlq mm1, 32
		movd eax, mm1
		imul ecx
		shrd eax, edx, 16
		movd mm5, eax
		punpckldq mm2, mm5
		movq mm1, mm2

#ifdef USEINTZ
		cvtss2si eax, xmm2
		sub eax, id
		imul ecx
		shrd eax, edx, 16
		mov ebx, eax

		mov edx, id
#endif

		test grendflags, RENDFLAGS_ALPHAMASK|RENDFLAGS_RALPHAMASK
		jnz short intoit1m
		jmp short intoit1
//--------------------------------------------------------------------------------------------------
prebegit1:
		test grendflags, RENDFLAGS_ALPHAMASK|RENDFLAGS_RALPHAMASK
		jnz short begit1m
begit1:
		addps xmm0, xmm1   ;u += g_ui8; v += g_vi8; d += g_di8;
		;rcpss xmm2, xmm0 ;noticably faster but looks too crappy
		movss xmm2, fone
		divss xmm2, xmm0   ;f = 1.0/d;
		movhlps xmm3, xmm0 ;xmm3: [. . u v]
		shufps xmm2, xmm2, 0 ;xmm2: [f f f f]
		mulps xmm3, xmm2   ;xmm3: [. . u*f v*f]

		cvtps2pi mm1, xmm3 ;mm0: [(int)u*f (int)v*f]
		psubd mm1, mm0
		psrad mm1, LFLATSTEPSIZ ;mm1: [iui ivi]
#ifdef USEINTZ
		cvtss2si ebx, xmm2
		sub ebx, edx
		sar ebx, LFLATSTEPSIZ ;ebx: idi
#else
		subss xmm2, xmm5
		mulss xmm2, grflatstepsiz ;grflatstepsiz=1/FLATSTEPSIZ
#endif

		lea esi, [edi+FLATSTEPSIZ*4] ;esi:p2
intoit1:
		cmp esi, pe
		jl short near_preskip
		mov esi, pe
near_preskip:
		sub edi, esi

begit2:
#ifdef USEINTZ
		cmp edx, [edi+esi+0x88888888] _asm selfmod_zbuf0:
		jg short near_skip
		mov [edi+esi+0x88888888], edx _asm selfmod_zbuf1:
#else
		ucomiss xmm5, [edi+esi+0x88888888] _asm selfmod_zbuf0:
		ja short near_skip
		movss [edi+esi+0x88888888], xmm5 _asm selfmod_zbuf1:
#endif
		movd eax, mm0
		shr eax, 0x88 _asm selfmod_shift:
		pextrw ecx, mm0, 3
		and eax, ymsk
		and ecx, xmsk

#if 0
		movd mm2, [eax+ecx*4+0x88888888] _asm selfmod_ttf: ;no rgbmul
#else
		punpcklbw mm2, [eax+ecx*4+0x88888888] _asm selfmod_ttf:
		pmulhuw mm2, mm3 ;rgbmul
		packuswb mm2, mm2
#endif
		movd dword ptr [edi+esi+0x88888888], mm2 _asm selfmod_cbuf:

near_skip:
		paddd mm0, mm1 ;iu += iui; iv += ivi;
#ifdef USEINTZ
		add edx, ebx
#else
		addss xmm5, xmm2
#endif
		add edi, 4
		jl short begit2

		add edi, esi
		cmp esi, pe
		jne short begit1
		jmp short enditall

begit1m:
		addps xmm0, xmm1   ;u += g_ui8; v += g_vi8; d += g_di8;
		;rcpss xmm2, xmm0 ;noticably faster but looks too crappy
		movss xmm2, fone
		divss xmm2, xmm0   ;f = 1.0/d;
		movhlps xmm3, xmm0 ;xmm3: [. . u v]
		shufps xmm2, xmm2, 0 ;xmm2: [f f f f]
		mulps xmm3, xmm2   ;xmm3: [. . u*f v*f]

		cvtps2pi mm1, xmm3 ;mm0: [(int)u*f (int)v*f]
		psubd mm1, mm0
		psrad mm1, LFLATSTEPSIZ ;mm1: [iui ivi]
#ifdef USEINTZ
		cvtss2si ebx, xmm2
		sub ebx, edx
		sar ebx, LFLATSTEPSIZ ;ebx: idi
#else
		subss xmm2, xmm5
		mulss xmm2, grflatstepsiz ;grflatstepsiz=1/FLATSTEPSIZ
#endif

		lea esi, [edi+FLATSTEPSIZ*4] ;esi:p2
intoit1m:
		cmp esi, pe
		jl short near_preskipm
		mov esi, pe
near_preskipm:
		sub edi, esi

begit2m:
#ifdef USEINTZ
		cmp edx, [edi+esi+0x88888888] _asm selfmod_zbuf0m:
		jg short near_skipm
#else
		ucomiss xmm5, [edi+esi+0x88888888] _asm selfmod_zbuf0m:
		ja short near_skipm
#endif
		movd eax, mm0
		shr eax, 0x88 _asm selfmod_shiftm:
		pextrw ecx, mm0, 3
		and eax, ymsk
		and ecx, xmsk

		mov eax, [eax+ecx*4+0x88888888] _asm selfmod_ttfm:
		test eax, eax
		jns short near_skipm _asm selfmod_jns_js:
		movd mm2, eax
#ifdef USEINTZ
		mov [edi+esi+0x88888888], edx _asm selfmod_zbuf1m:
#else
		movss [edi+esi+0x88888888], xmm5 _asm selfmod_zbuf1m:
#endif

#if 1
		punpcklbw mm2, mm2
		pmulhuw mm2, mm3 ;rgbmul
		packuswb mm2, mm2
#endif
		movd dword ptr [edi+esi+0x88888888], mm2 _asm selfmod_cbufm:

near_skipm:
		paddd mm0, mm1 ;iu += iui; iv += ivi;
#ifdef USEINTZ
		add edx, ebx
#else
		addss xmm5, xmm2
#endif
		add edi, 4
		jl short begit2m

		add edi, esi
		cmp esi, pe
		jne short begit1m

enditall:
		pop edi
		pop esi
		pop ebx
		emms
	}
#endif

	//p >>= 2; pe >>= 2;
	//for(;p<pe;p++) cbuf[p] = (((long)((*(float *)&zbuf[p])*256.0))&255)*0x10101;
}

static void htflatbilin (int hind)
{
	float f, d, u, v, vx, vy;
	long id, iu, iv, idi, iui, ivi, p, p2, pe, sx0, sx1, sy;
	long iuf, ivf, r0, g0, b0, r1, g1, b1;
	long *cbuf, *zbuf, ttps, ymsk, xmsk, ttf, ttp;
	unsigned char *uptr;

	if (hind < 0)
	{
#if (!((TESTDEPTH != 0) || (_MSC_VER == 0)))
		if (hind == -2)
		{
			static long ozbuf = 0, ocbuf = 0;
			if (gcc.z.f != ozbuf)
			{
				ozbuf = gcc.z.f;
				SELFMODVAL(selfmod_zbuf0 -4,((long)ozbuf));
				SELFMODVAL(selfmod_zbuf1 -4,((long)ozbuf));
				SELFMODVAL(selfmod_zbuf0m-4,((long)ozbuf));
				SELFMODVAL(selfmod_zbuf1m-4,((long)ozbuf));
			}
			if (gcc.c.f != ocbuf)
			{
				ocbuf = gcc.c.f;
				SELFMODVAL(selfmod_cbuf -4,ocbuf);
				SELFMODVAL(selfmod_cbufm-4,ocbuf);
			}
		}
		else
		{
			if (!(grendflags&(RENDFLAGS_ALPHAMASK|RENDFLAGS_RALPHAMASK)))
			{
				static long ottps = -1, ottf = 0, ottp = 0, onodt = 0;
				if (g_ttps != ottps) { ottps = g_ttps; SELFMODVAL(selfmod_shift-1,((char)ottps)); }
				if ((g_ttp != ottp) || (g_ttf != ottf))
				{
					ottp = g_ttp;
					SELFMODVAL(selfmod_ttf_ttp0-4,g_ttf+g_ttp);
					SELFMODVAL(selfmod_ttf_ttp1-4,g_ttf+g_ttp);
				}
				if (g_ttf != ottf)
				{
					ottf = g_ttf;
					SELFMODVAL(selfmod_ttf0-4,ottf);
					SELFMODVAL(selfmod_ttf1-4,ottf);
				}
				if ((grendflags&RENDFLAGS_NODEPTHTEST) != onodt)
				{
					onodt = (grendflags&RENDFLAGS_NODEPTHTEST);
#ifdef USEINTZ
					if (!onodt) { SELFMODVAL(selfmod_zbuf0-7,((short)0x943b)); }
							 else { SELFMODVAL(selfmod_zbuf0-7,((short)0x848b)); }
#else
					if (!onodt) { SELFMODVAL(selfmod_zbuf0-8,((short)0x2e0f)); }
							 else { SELFMODVAL(selfmod_zbuf0-8,((short)0x0ceb)); } //WARNING: Assumes long 6-byte jcc instruction follows!
#endif
				}
			}
			else
			{
				static long ottpsm = -1, ottfm = 0, ottpm = 0, onodtm = 0;
				static char ojns_js = 0x79;
				if (g_ttps != ottpsm) { ottpsm = g_ttps; SELFMODVAL(selfmod_shiftm-1,((char)ottpsm)); }
				if ((g_ttp != ottpm) || (g_ttf != ottfm))
				{
					ottpm = g_ttp;
					SELFMODVAL(selfmod_ttf_ttp0m-4,g_ttf+g_ttp);
					SELFMODVAL(selfmod_ttf_ttp1m-4,g_ttf+g_ttp);
				}
				if (g_ttf != ottfm)
				{
					ottfm = g_ttf;
					SELFMODVAL(selfmod_ttf0m-4,ottfm);
					SELFMODVAL(selfmod_ttf1m-4,ottfm);
				}
				if ((grendflags&RENDFLAGS_NODEPTHTEST) != onodtm)
				{
					onodtm = (grendflags&RENDFLAGS_NODEPTHTEST);
#ifdef USEINTZ
					if (!onodtm) { SELFMODVAL(selfmod_zbuf0m-7,((short)0x943b)); }
							  else { SELFMODVAL(selfmod_zbuf0m-7,((short)0x848b)); }
#else
					if (!onodtm) { SELFMODVAL(selfmod_zbuf0m-8,((short)0x2e0f)); }
							  else { SELFMODVAL(selfmod_zbuf0m-8,((short)0x0ceb)); } //WARNING: Assumes long 6-byte jcc instruction follows!
#endif
				}

					//0x78 [byteoffs]  js  short lab
					//0x79 [byteoffs]  jns short lab
				if (grendflags&RENDFLAGS_RALPHAMASK) p = 0x78; else p = 0x79;
				if (p != ojns_js) { ojns_js = p; SELFMODVAL(selfmod_jns_js-2,ojns_js); }
			}
		}
#endif
		return;
	}

	sx0 = (long)ghlinbuf[hind][0];
	sx1 = (long)ghlinbuf[hind][1];
	sy  = (long)ghlinbuf[hind][2];

	p = (gcc.c.p>>2)*sy; pe = sx1+p; p += sx0;
	if (p >= pe) return;

	vx = (float)sx0; vy = (float)sy;
	d = g_mat[0]*vx + g_mat[3]*vy + g_mat[6];
	u = g_mat[1]*vx + g_mat[4]*vy + g_mat[7];
	v = g_mat[2]*vx + g_mat[5]*vy + g_mat[8];
	f = 1.0/d;
	id = (long)(  f);
	iu = (long)(u*f);
	iv = (long)(v*f);

	xmsk = (g_xmsk>>2);
#if ((TESTDEPTH != 0) || (_MSC_VER == 0))
	d += g_di8;
	zbuf = (long *)gcc.z.f;
	cbuf = (long *)gcc.c.f;
	ttf = g_ttf; ttps = g_ttps; ymsk = g_ymsk; ttp = g_ttp;
	do
	{
		f = 1.0/d; u += g_ui8; v += g_vi8; d += g_di8;
		idi = ((((long)(  f))-id)>>LFLATSTEPSIZ);
		iui = ((((long)(u*f))-iu)>>LFLATSTEPSIZ);
		ivi = ((((long)(v*f))-iv)>>LFLATSTEPSIZ);
		p2 = min(p+FLATSTEPSIZ,pe);
		do //Bilinear 2x2
		{
			if (id < zbuf[p]) //FIX:doesn't work for USEINTZ==0!
			{
				zbuf[p] = id;
				uptr = (unsigned char *)(((iv>>ttps)&ymsk) + (((iu>>16)&xmsk)<<2) + ttf);
				iuf = (iu&65535); ivf = (iv&65535);
				b0 = (((int)uptr[4])-((int)uptr[0]))*iuf + (((int)uptr[0])<<16);
				g0 = (((int)uptr[5])-((int)uptr[1]))*iuf + (((int)uptr[1])<<16);
				r0 = (((int)uptr[6])-((int)uptr[2]))*iuf + (((int)uptr[2])<<16); uptr += ttp;
				b1 = (((int)uptr[4])-((int)uptr[0]))*iuf + (((int)uptr[0])<<16);
				g1 = (((int)uptr[5])-((int)uptr[1]))*iuf + (((int)uptr[1])<<16);
				r1 = (((int)uptr[6])-((int)uptr[2]))*iuf + (((int)uptr[2])<<16);
				b0 += ((b1-b0)>>8)*(ivf>>8);
				g0 += ((g1-g0)>>8)*(ivf>>8);
				r0 += ((r1-r0)>>8)*(ivf>>8);
#if (TESTDEPTH == 0)
				cbuf[p] = rgb_scale((r0&0xff0000)+((g0>>8)&0xff00)+(b0>>16),g_rgbmul);
#else
				cbuf[p] = ((id>>12)&255)*0x10101;
#endif
			}
			id += idi; iu += iui; iv += ivi; p++;
		} while (p < p2);
	} while (p < pe);
#else
	p <<= 2; pe <<= 2;
	_asm
	{
		push ebx
		push esi
		push edi

		movss xmm1, u
		movss xmm0, v
		unpcklps xmm1, xmm0 ;xmm1: [. . v u]
		movss xmm0, d
		movlhps xmm0, xmm1 ;xmm0: [v u . d]

		movss xmm2, g_ui8
		movss xmm1, g_vi8
		unpcklps xmm2, xmm1 ;xmm2: [. . vi8 ui8]
		movss xmm1, g_di8
		movlhps xmm1, xmm2 ;xmm1: [vi8 ui8 . di8]

		movd mm4, iu
		punpckldq mm4, iv ;mm4: [iv iu]
#ifdef USEINTZ
		mov edx, id
#else
		rcpss xmm5, xmm0
		;movss xmm5, f ;WARNING: A full day of debugging resulted in: VC6 thinks 'f' is done with, and is corrupt (-1.#IND) at this point
#endif

		movd mm3, g_rgbmul
		punpcklbw mm3, mm3
		psrlw mm3, 4

		pxor mm7, mm7

		mov edi, p
//--------------------------------------------------------------------------------------------------
//Align steps to right side to avoid horizon hopping

		mov eax, d
		xor eax, g_di8
		jns short prebegit1
		mov eax, pe
		sub eax, p
		shr eax, 2
		and eax, FLATSTEPSIZ-1
		jz short prebegit1

			; edx:             [ id]
			; edi:             [ p ]
			; mm0:         [ iu  iv]
			; mm3:         [ RmGmBm]
			;xmm0: [ u   v   .   d ]
			;xmm1: [ui8 vi8  .  di8]
		shl eax, 2
		movaps xmm4, xmm1
		mulps xmm4, qmul[eax*4]
		addps xmm0, xmm4   ;u += g_ui8*frac; v += g_vi8*frac; d += g_di8*frac;
		;rcpss xmm2, xmm0 ;noticably faster but looks too crappy
		movss xmm2, fone
		divss xmm2, xmm0   ;f = 1.0/d;
		movhlps xmm3, xmm0 ;xmm3: [. . u v]
		shufps xmm2, xmm2, 0 ;xmm2: [f f f f]
		mulps xmm3, xmm2   ;xmm3: [. . u*f v*f]

		lea esi, [edi+eax]

		cvtps2pi mm5, xmm3 ;mm0: [(int)u*f (int)v*f]
		psubd mm5, mm4

#ifndef USEINTZ
		subss xmm2, xmm5
		mulss xmm2, freciplut[eax]
#endif

		mov ecx, reciplut[eax]

		movd eax, mm5
		imul ecx
		shrd eax, edx, 16
		movd mm2, eax

		psrlq mm5, 32
		movd eax, mm5
		imul ecx
		shrd eax, edx, 16
		movd mm6, eax
		punpckldq mm2, mm6
		movq mm5, mm2

#ifdef USEINTZ
		cvtss2si eax, xmm2
		sub eax, id
		imul ecx
		shrd eax, edx, 16
		mov ebx, eax

		mov edx, id
#endif

		test grendflags, RENDFLAGS_ALPHAMASK|RENDFLAGS_RALPHAMASK
		jnz short intoit1m
		jmp short intoit1
//--------------------------------------------------------------------------------------------------
prebegit1:
		test grendflags, RENDFLAGS_ALPHAMASK|RENDFLAGS_RALPHAMASK
		jnz short begit1m
begit1:
		addps xmm0, xmm1   ;u += g_ui8; v += g_vi8; d += g_di8;
		;rcpss xmm2, xmm0 ;noticably faster but looks too crappy
		movss xmm2, fone
		divss xmm2, xmm0   ;f = 1.0/d;
		movhlps xmm3, xmm0 ;xmm3: [. . u v]
		shufps xmm2, xmm2, 0 ;xmm2: [f f f f]
		mulps xmm3, xmm2   ;xmm3: [. . u*f v*f]

		cvtps2pi mm5, xmm3 ;mm4: [(int)v*f (int)u*f]
		psubd mm5, mm4
		psrad mm5, LFLATSTEPSIZ ;mm5: [ivi iui]
#ifdef USEINTZ
		cvtss2si ebx, xmm2
		sub ebx, edx
		sar ebx, LFLATSTEPSIZ ;ebx: idi
#else
		subss xmm2, xmm5
		mulss xmm2, grflatstepsiz ;grflatstepsiz=1/FLATSTEPSIZ
#endif

		lea esi, [edi+FLATSTEPSIZ*4] ;esi:p2
intoit1:
		cmp esi, pe
		jl short near_preskip
		mov esi, pe
near_preskip:
		sub edi, esi

begbilin:
#ifdef USEINTZ
		cmp edx, [edi+esi+0x88888888] _asm selfmod_zbuf0:
		jge short skipbilin
		mov [edi+esi+0x88888888], edx _asm selfmod_zbuf1:
#else
		ucomiss xmm5, [edi+esi+0x88888888] _asm selfmod_zbuf0:
		jae short skipbilin
		movss [edi+esi+0x88888888], xmm5 _asm selfmod_zbuf1:
#endif

		pshufw mm0, mm4, 0xd
		movd eax, mm0
		mov ecx, eax
		shr eax, 0x88 _asm selfmod_shift:
		and ecx, xmsk
		and eax, g_ymsk

		pshufw mm6, mm4, 0
		psrlw mm6, 10

		movd mm0, [eax+ecx*4+0x88888888] _asm selfmod_ttf0:
		movd mm2, [eax+ecx*4+0x88888888] _asm selfmod_ttf_ttp0:
		add ecx, 1
		and ecx, xmsk
		movd mm1, [eax+ecx*4+0x88888888] _asm selfmod_ttf1:
		punpcklbw mm0, mm7
		punpcklbw mm1, mm7
		psubw mm1, mm0
		pmullw mm1, mm6
		psllw mm0, 6
		paddw mm0, mm1

		movd mm1, [eax+ecx*4+0x88888888] _asm selfmod_ttf_ttp1:
		punpcklbw mm2, mm7
		punpcklbw mm1, mm7
		psubw mm1, mm2
		pmullw mm1, mm6
		psllw mm2, 6
		paddw mm1, mm2

		pshufw mm6, mm4, 0xaa
		psrlw mm6, 1

		psubw mm1, mm0
		pmulhw mm1, mm6
		pavgw mm0, mm7
		paddw mm0, mm1
#if 0
		psrlw mm0, 5 ;no rgbmul
#else
		pmulhuw mm0, mm3
#endif
		packuswb mm0, mm0
		movd [edi+esi+0x88888888], mm0 _asm selfmod_cbuf:
skipbilin:
#ifdef USEINTZ
		add edx, ebx
#else
		addss xmm5, xmm2
#endif
		paddd mm4, mm5 ;iu += iui; iv += ivi;
		add edi, 4
		jl short begbilin

		add edi, esi
		cmp esi, pe
		jne short begit1
		jmp short enditall

begit1m:
		addps xmm0, xmm1   ;u += g_ui8; v += g_vi8; d += g_di8;
		;rcpss xmm2, xmm0 ;noticably faster but looks too crappy
		movss xmm2, fone
		divss xmm2, xmm0   ;f = 1.0/d;
		movhlps xmm3, xmm0 ;xmm3: [. . u v]
		shufps xmm2, xmm2, 0 ;xmm2: [f f f f]
		mulps xmm3, xmm2   ;xmm3: [. . u*f v*f]

		cvtps2pi mm5, xmm3 ;mm4: [(int)v*f (int)u*f]
		psubd mm5, mm4
		psrad mm5, LFLATSTEPSIZ ;mm5: [ivi iui]
#ifdef USEINTZ
		cvtss2si ebx, xmm2
		sub ebx, edx
		sar ebx, LFLATSTEPSIZ ;ebx: idi
#else
		subss xmm2, xmm5
		mulss xmm2, grflatstepsiz ;grflatstepsiz=1/FLATSTEPSIZ
#endif

		lea esi, [edi+FLATSTEPSIZ*4] ;esi:p2
intoit1m:
		cmp esi, pe
		jl short near_preskipm
		mov esi, pe
near_preskipm:
		sub edi, esi

begbilinm:
#ifdef USEINTZ
		cmp edx, [edi+esi+0x88888888] _asm selfmod_zbuf0m:
		jge short skipbilinm
#else
		ucomiss xmm5, [edi+esi+0x88888888] _asm selfmod_zbuf0m:
		jae short skipbilinm
#endif
		pshufw mm0, mm4, 0xd
		movd eax, mm0
		mov ecx, eax
		shr eax, 0x88 _asm selfmod_shiftm:
		and ecx, xmsk
		and eax, g_ymsk

		pshufw mm6, mm4, 0
		psrlw mm6, 10

		movd mm0, [eax+ecx*4+0x88888888] _asm selfmod_ttf0m:
		movd mm2, [eax+ecx*4+0x88888888] _asm selfmod_ttf_ttp0m:
		add ecx, 1
		and ecx, xmsk
		movd mm1, [eax+ecx*4+0x88888888] _asm selfmod_ttf1m:
		punpcklbw mm0, mm7
		punpcklbw mm1, mm7
		psubw mm1, mm0
		pmullw mm1, mm6
		psllw mm0, 6
		paddw mm0, mm1

		movd mm1, [eax+ecx*4+0x88888888] _asm selfmod_ttf_ttp1m:
		punpcklbw mm2, mm7
		punpcklbw mm1, mm7
		psubw mm1, mm2
		pmullw mm1, mm6
		psllw mm2, 6
		paddw mm1, mm2

		pshufw mm6, mm4, 0xaa
		psrlw mm6, 1

		psubw mm1, mm0
		pmulhw mm1, mm6
		pavgw mm0, mm7
		paddw mm0, mm1
#if 0
		psrlw mm0, 5 ;no rgbmul
#else
		pmulhuw mm0, mm3
#endif
		packuswb mm0, mm0

		movd eax, mm0
		test eax, eax
		jns short skipbilinm _asm selfmod_jns_js:

#ifdef USEINTZ
		mov [edi+esi+0x88888888], edx _asm selfmod_zbuf1m:
#else
		movss [edi+esi+0x88888888], xmm5 _asm selfmod_zbuf1m:
#endif
		movd [edi+esi+0x88888888], mm0 _asm selfmod_cbufm:
skipbilinm:
#ifdef USEINTZ
		add edx, ebx
#else
		addss xmm5, xmm2
#endif
		paddd mm4, mm5 ;iu += iui; iv += ivi;
		add edi, 4
		jl short begbilinm

		add edi, esi
		cmp esi, pe
		jne short begit1m

enditall:
		pop edi
		pop esi
		pop ebx
		emms
	}
#endif
}

	//(04/18/2008: Code copied from POLFILL.BAS)
	//In:  pt:  complex polygon, sector format (pt is read-only / not clobbered)
	//     ptn: number of polygon vertices
	//Out: sortlst: properly ordered vertex list (left side as viewed from inside) for filling polygon left to right
	//             sortlst, bit 30: 1=last vertex of bunch, 0=else
	//             sortlst, bit 31: 0=left, 1=right
	//     Returns: number of vertices (line segments) on sortlst
long polysortfill (vertyp *pt, long ptn, long *sortlst)
{
	#define SIMPOLY 32
	typedef struct { long v, l; } bunch_t;
	bunch_t *bunch, bunchmem[SIMPOLY];
	float x, y, x1, y1, x2, y2;
	long i, j, k, b, e, gap, *bunchi, nbunch, nbunch0, *bsortlst, bsortn;
	char *got, gotmem[SIMPOLY];

	if (ptn <= SIMPOLY) { if (ptn < 3) return(0); got = gotmem; bunch = bunchmem; } //avoid _alloca for simple polygons
	else { got = (char *)_alloca(ptn*sizeof(got[0])); bunch = (bunch_t *)_alloca(ptn*sizeof(bunch[0])); }
	memset(got,0,ptn);

		//Get bunchi & nbunch
	i = ptn-1; nbunch = 0;
	do
	{
		nbunch0 = nbunch;
		do
		{
			got[i] = 1; j = pt[i].n;
			if ((pt[j].y < pt[i].y) != (pt[pt[j].n].y < pt[j].y)) { bunch[nbunch].v = j; bunch[nbunch].l = nbunch-1; nbunch++; }
			i = j;
		} while (!got[i]);
		bunch[nbunch0].l = nbunch-1;
		do { i--; } while ((i >= 2) && (got[i]));
	} while (i >= 2);

	if (nbunch <= 2) //If only 2 bunches, do simple sort
	{
		if (nbunch < 2) return(0);
		k = (pt[bunch[1].v].y > pt[bunch[0].v].y); i = bunch[k].v; k ^= 1; j = 0;
		do { sortlst[j] = i;            j++; i = pt[i].n; } while (i != bunch[k].v); if (j) sortlst[j-1] |= 0x40000000; k ^= 1;
		do { sortlst[j] = i|0x80000000; j++; i = pt[i].n; } while (i != bunch[k].v); if (j) sortlst[j-1] |= 0x40000000;
		return(j);
	}
	else
	{
		bsortlst = (long *)_alloca(nbunch*sizeof(bsortlst[0]));
		bunchi = (long *)_alloca(nbunch*sizeof(bunchi[0]));
		for(i=nbunch-1;i>=0;i--) bunchi[i] = i;

			//Sort bunches by y's
		for(gap=(nbunch>>1);gap>0;gap>>=1)
			for(i=0;i<nbunch-gap;i++)
				for(j=i;j>=0;j-=gap)
				{
					if (pt[bunch[bunchi[j]].v].y <= pt[bunch[bunchi[j+gap]].v].y) break;
					k = bunchi[j]; bunchi[j] = bunchi[j+gap]; bunchi[j+gap] = k;
				}

			//Generate bsortlst - go though bunch tops in increasing order
		for(k=0,bsortn=0;bsortn<nbunch;k++)
		{
			b = bunchi[k]; i = bunch[b].v;
			if (pt[pt[i].n].y < pt[i].y) { got[b] = 2; got[bunch[b].l] = 2; continue; } //Don't compare these bunch any more (too high)
			x = pt[i].x; y = pt[i].y;

				//Find where new bunch should be inserted on sort list by
				//finding first bunch on bsortlst to the right of point (x,y)
				//At the same time, insert 2 new places for the 2 new bunch
			for(i=bsortn-1;i>=0;i--)
			{
				j = bsortlst[i];
				if (got[j] == 1)
				{
					e = bunch[j].v;
					if (pt[pt[e].n].y >= pt[e].y) { while (pt[pt[e].n].y < y) e = pt[e].n; }
													 else { while (pt[pt[e].n].y > y) e = pt[e].n; }
					x1 = pt[e].x-x; x2 = pt[pt[e].n].x-x;
					if ((x1 < 0.f) != (x2 < 0.f)) { y1 = pt[e].y-y; y2 = pt[pt[e].n].y-y; if ((x2*y1 < x1*y2) != (y1 < 0.f)) break; }
					else if (x1 < 0.f) break;
				}
				bsortlst[i+2] = j;
			}
			bsortlst[(i+1)|1] = b; bsortlst[(i&~1)+2] = bunch[b].l; bsortn += 2; //Insert the 2 new bunches
		}
	}

	for(j=0,k=0;k<bsortn;k++)
	{
		i = bunch[bsortlst[k]].v;
		if (pt[pt[i].n].y < pt[i].y) { for(;pt[pt[i].n].y <  pt[i].y;i=pt[i].n) if (pt[pt[i].n].y != pt[i].y) sortlst[j++] = i;            }
										else { for(;pt[pt[i].n].y >= pt[i].y;i=pt[i].n) if (pt[pt[i].n].y != pt[i].y) sortlst[j++] = i|0x80000000; }
		if (j) sortlst[j-1] |= 0x40000000;
	}

#if 0
		//Debug only!
	extern char keystatus[256];
	if (keystatus[0x2a])
	{
		keystatus[0x2a] = 0;
		for(i=0;i<ptn;i++) printf("pt[%d]: x=%f,y=%f,n=%d\n",i,pt[i].x,pt[i].y,pt[i].n);
		for(i=0;i<nbunch;i++) printf("bunch[%d]: v=%d l=%d\n",i,bunch[i].v,bunch[i].l);
		for(i=0;i<bsortn;i++) printf("bsortlst[%d] = %d\n",i,bsortlst[i]);
		for(i=0;i<j;i++) printf("sortlst[%d] = 0x%08x\n",i,sortlst[i]);
		printf("\n");
	}
#endif

	return(j);
}

#define FIXNEWFILL 1
#if (FIXNEWFILL != 0)
	//Note: requires gcc.c to be set!
void poly_raster (vertyp *pt, long pn, void (*rastfunc)(int i))
{
#define PR_USEFLOAT 1
#if (PR_USEFLOAT != 0)
	typedef struct { long i0, i1; float pos, inc; } rast_t;
#else
	typedef struct { long i0, i1, pos, inc; } rast_t; float f, g; //FIX:overflows in BUILD2 easily :/
#endif
	rast_t *rast, rtmp;
	long i, j, k, y, iy0, iy1, pn2, pn3, pn4, ymin, ymax, hlincnt;

	rast = (rast_t *)_alloca(pn*sizeof(rast_t));

	ymin = 0x7fffffff; ymax = 0x80000000; pn2 = 0; j = -1; iy1 = 0;
	for(i=0;i<pn;i++)
	{
		if (i != j)  iy0 = (long)min(max(ceil(pt[i].y),0),gcc.c.y); else iy0 = iy1;
		j = pt[i].n; iy1 = (long)min(max(ceil(pt[j].y),0),gcc.c.y); if (iy0 == iy1) continue;
		if (iy0 < iy1) { rast[pn2].i0 = i; rast[pn2].i1 = j; if (iy0 < ymin) ymin = iy0; }
					 else { rast[pn2].i0 = j; rast[pn2].i1 = i; if (iy0 > ymax) ymax = iy0; }
		pn2++;
	}

		//Shell sort top y's
	for(k=(pn2>>1);k;k>>=1)
		for(i=0;i<pn2-k;i++)
			for(j=i;j>=0;j-=k)
			{
				if (pt[rast[j].i0].y <= pt[rast[j+k].i0].y) break;
				rtmp = rast[j]; rast[j] = rast[j+k]; rast[j+k] = rtmp;
			}

	pn3 = 0; pn4 = 0; hlincnt = 0;
	for(y=ymin;y<ymax;y++)
	{
		for(i=pn3-1;i>=0;i--)
		{
			if ((float)y >= pt[rast[i].i1].y)
			{     //Delete line segments
				pn3--;
				for(j=i;j<pn3;j++) rast[j] = rast[j+1];
			}
			else if (rast[i+1].pos < rast[i].pos)
			{     //Refresh sort (needed for degenerate poly/intersections)
				rtmp = rast[i];
				for(j=i+1;(j < pn3) && (rast[j].pos < rtmp.pos);j++) rast[j-1] = rast[j];
				rast[j-1] = rtmp;
			}
		}

			//Insert line segments
		while ((pn4 < pn2) && ((float)y >= pt[rast[pn4].i0].y))
		{
			rtmp.i0 = rast[pn4].i0; rtmp.i1 = rast[pn4].i1;
#if (PR_USEFLOAT != 0)
			rtmp.inc = (pt[rtmp.i1].x - pt[rtmp.i0].x)/(pt[rtmp.i1].y - pt[rtmp.i0].y);
			rtmp.pos = ((float)y - pt[rtmp.i0].y)*rtmp.inc + pt[rtmp.i0].x;
#else
			g = (pt[rtmp.i1].x - pt[rtmp.i0].x)/(pt[rtmp.i1].y - pt[rtmp.i0].y);
			f = ((float)y - pt[rtmp.i0].y)*g + pt[rtmp.i0].x;
			rtmp.inc = ((long)(g*65536.0));
			rtmp.pos = ((long)(f*65536.0))+65535;
#endif

			for(j=pn3;(j > 0) && (rast[j-1].pos > rtmp.pos);j--) rast[j] = rast[j-1];
			rast[j] = rtmp;

			pn3++; pn4++;
		}

			//Draw hlines xor style
#if (PR_USEFLOAT != 0)
		for(i=0;i<pn3;i+=2)
		{
			ghlinbuf[hlincnt][0] = (short)min(max(rast[i  ].pos,0.f),(float)gcc.c.x);
			ghlinbuf[hlincnt][1] = (short)min(max(rast[i+1].pos,0.f),(float)gcc.c.x);
			ghlinbuf[hlincnt][2] = (short)y;
			hlincnt++; if (hlincnt >= MAXHLINS) { htrun(rastfunc,0,hlincnt,drawpoly_numcpu); hlincnt = 0; }
			//hlincnt++; if (!(hlincnt&255)) { if (hlincnt > 256) htfinish(); htstart(rastfunc,hlincnt-256,hlincnt,drawpoly_numcpu); } //FIX:hlincnt can overflow!
		}
#else
		for(i=0;i<pn3;i+=2)
		{
			ghlinbuf[hlincnt][0] = (short)min(max(rast[i  ].pos>>16,0),gcc.c.x);
			ghlinbuf[hlincnt][1] = (short)min(max(rast[i+1].pos>>16,0),gcc.c.x);
			ghlinbuf[hlincnt][2] = (short)y;
			hlincnt++; if (hlincnt >= MAXHLINS) { htrun(rastfunc,0,hlincnt,drawpoly_numcpu); hlincnt = 0; }
		}
#endif
			//Inc x-steps
		for(i=pn3-1;i>=0;i--) rast[i].pos += rast[i].inc;
	}
	if (hlincnt) htrun(rastfunc,0,hlincnt,drawpoly_numcpu);
	//if (hlincnt >= 256) htfinish(); if (hlincnt&255) htrun(rastfunc,hlincnt&~255,hlincnt,drawpoly_numcpu); //FIX:hlincnt can overflow!
}
#endif

	//Quick&dirty flat polygon renderer
static void drawpolyflat (tiltyp *tt, vertyp *pt, int num, int rgbmul, float hsc, float *ouvmat, int rendflags)
{
	__declspec(align(16)) static float dpqmulval[4] = {0,1,2,3}, dpqfours[4] = {4,4,4,4};
	//__declspec(align(16)) static float dpqmagic[4] = {4194304.f*3.f,4194304.f*3.f,4194304.f*3.f,4194304.f*3.f};
	//__declspec(align(16)) static long dpqsub[4] = {0x4b400000,0x4b400000,0x4b400000,0x4b400000};
	point3d *npt;
	vertyp *pn;
	float ax, ay, az, au, av, bx, by, bz, bu, bv, cx, cy, cz, cu, cv;
	float d, f, g, t, p0x, p0y, p0z, p1x, p1y, p1z, p2x, p2y, p2z;
	long i, j, k, l, n, sx0, sx1, sy, x, xi, y, minx, miny, maxx, maxy;
	long *tlong, ordn, splitn, osplitn, *lptr, *isy, yy0, yy1, ind[4];
	char *gotpt;

	if (!(rendflags&RENDFLAGS_NOTRCP))
	{
			//Translate & Rotate
		npt = (point3d *)_alloca(num*sizeof(npt[0]));
		for(i=0;i<num;i++)
		{
			ax = pt[i].x-gcc.p.x;
			ay = pt[i].y-gcc.p.y;
			az = pt[i].z-gcc.p.z;
			npt[i].x = ax*gcc.r.x + ay*gcc.r.y + az*gcc.r.z;
			npt[i].y = ax*gcc.d.x + ay*gcc.d.y + az*gcc.d.z;
			npt[i].z = ax*gcc.f.x + ay*gcc.f.y + az*gcc.f.z;
		}

			//Clip to near plane
		pn = (vertyp *)_alloca((num*2)*sizeof(pn[0]));
		tlong = (long *)_alloca(num*2*sizeof(tlong[0])); splitn = osplitn = 0;
		gotpt = (char *)_alloca(num); memset(gotpt,0,num);
		i = 0; n = 0; l = 0;
		while (1)
		{
			j = pt[i].n; gotpt[i] = 1;
			if (npt[i].z >= SCISDIST)
			{
				pn[n].x = npt[i].x; pn[n].u = pt[i].u;
				pn[n].y = npt[i].y; pn[n].v = pt[i].v;
				pn[n].z = npt[i].z; pn[n].n = n+1; n++;
			}
			if ((npt[i].z >= SCISDIST) != (npt[j].z >= SCISDIST))
			{
				f = (SCISDIST-npt[j].z)/(npt[i].z-npt[j].z);
				pn[n].x = (npt[i].x-npt[j].x)*f + npt[j].x; pn[n].u = (pt[i].u-pt[j].u)*f + pt[j].u;
				pn[n].y = (npt[i].y-npt[j].y)*f + npt[j].y; pn[n].v = (pt[i].v-pt[j].v)*f + pt[j].v;
				pn[n].z = SCISDIST; pn[n].n = n+1; if (npt[i].z >= SCISDIST) { tlong[splitn] = n; splitn++; }
				n++;
			}
			if (!gotpt[j]) { i = j; continue; }
			if (n-l >= 3) { pn[n-1].n = l; l = n; osplitn = splitn; } else { n = l; splitn = osplitn; }
			do { i++; } while ((i < num) && (gotpt[i])); if (i >= num) break;
		}
		if (n < 3) return;

			//Re-wire splits (remove degenerate line segments)
		for(i=1;i<splitn;i++)
		{
			ind[0] = tlong[i]; ind[1] = pn[ind[0]].n;
			for(j=0;j<i;j++)
			{
				ind[2] = tlong[j]; ind[3] = pn[ind[2]].n;
				if (fabs(pn[ind[0]].x-pn[ind[3]].x) + fabs(pn[ind[2]].x-pn[ind[1]].x) +
					 fabs(pn[ind[0]].y-pn[ind[3]].y) + fabs(pn[ind[2]].y-pn[ind[1]].y) <
					 fabs(pn[ind[0]].x-pn[ind[1]].x) + fabs(pn[ind[2]].x-pn[ind[3]].x) +
					 fabs(pn[ind[0]].y-pn[ind[1]].y) + fabs(pn[ind[2]].y-pn[ind[3]].y))
					{ k = pn[ind[0]].n; pn[ind[0]].n = pn[ind[2]].n; pn[ind[2]].n = k; }
			}
		}

			//Project to screen
		isy = (long *)_alloca(n*sizeof(isy[0]));
		minx = gcc.c.x; maxx = 0;
		miny = gcc.c.y; maxy = 0;
		for(i=n-1;i>=0;i--)
		{
			f = gcc.h.z/pn[i].z;
			pn[i].x = pn[i].x*f + gcc.h.x; g = min(max(pn[i].x,0.0),gcc.c.x); x = (long)g;
			pn[i].y = pn[i].y*f + gcc.h.y; g = min(max(pn[i].y,0.0),gcc.c.y); y = (long)g; isy[i] = (long)ceil(g);
			if (x < minx) minx = x;
			if (x > maxx) maxx = x;
			if (y < miny) miny = y;
			if (y > maxy) maxy = y;
		}
		if ((maxx <= minx) || (maxy <= miny)) return;

			//Calculate facing of polygon
		t = 0.f;
		for(i=0;i<n;i++) { j = pn[i].n; k = pn[j].n; t += (pn[i].x-pn[k].x)*pn[j].y; }
		if (!(rendflags&RENDFLAGS_CULLNONE))
		{
			if ((!(rendflags&RENDFLAGS_CULLFRONT)) && (t <= 0)) return;
			if (( (rendflags&RENDFLAGS_CULLFRONT)) && (t >= 0)) return;
		}
		if (t < 0) //Flip poly
		{
			for(i=n-1;i>=0;i--) tlong[i] = pn[i].n;
			for(i=n-1;i>=0;i--) pn[tlong[i]].n = i;
		}
	}
	else
	{
#if (FIXNEWFILL == 0)
		tlong = (long *)_alloca(num*sizeof(tlong[0]));
		isy   = (long *)_alloca(num*sizeof(  isy[0]));
		for(i=num-1;i>=0;i--) isy[i] = (long)ceil(min(max(pt[i].y,0.0),gcc.c.y));
#endif
		pn = pt; n = num;
	}

	if (!(rendflags&RENDFLAGS_GMAT))
	{
			//Parametric plane equation
		if (!(rendflags&RENDFLAGS_OUVMAT))
		{
			i = 0; j = pt[i].n; k = pt[j].n;
			ax = npt[j].x-npt[i].x; bx = npt[k].x-npt[i].x; cx = npt[i].x;
			ay = npt[j].y-npt[i].y; by = npt[k].y-npt[i].y; cy = npt[i].y;
			az = npt[j].z-npt[i].z; bz = npt[k].z-npt[i].z; cz = npt[i].z;
			au =  pt[j].u- pt[i].u; bu =  pt[k].u- pt[i].u; cu =  fmod(pt[i].u,1.f);
			av =  pt[j].v- pt[i].v; bv =  pt[k].v- pt[i].v; cv =  fmod(pt[i].v,1.f);
		}
		else
		{
			point3d nouvmat[3];
			for(i=0;i<3;i++)
			{
				ax = ouvmat[i*3+0]-gcc.p.x; ay = ouvmat[i*3+1]-gcc.p.y; az = ouvmat[i*3+2]-gcc.p.z;
				nouvmat[i].x = ax*gcc.r.x + ay*gcc.r.y + az*gcc.r.z;
				nouvmat[i].y = ax*gcc.d.x + ay*gcc.d.y + az*gcc.d.z;
				nouvmat[i].z = ax*gcc.f.x + ay*gcc.f.y + az*gcc.f.z;
			}
			ax = nouvmat[1].x-nouvmat[0].x; bx = nouvmat[2].x-nouvmat[0].x; cx = nouvmat[0].x;
			ay = nouvmat[1].y-nouvmat[0].y; by = nouvmat[2].y-nouvmat[0].y; cy = nouvmat[0].y;
			az = nouvmat[1].z-nouvmat[0].z; bz = nouvmat[2].z-nouvmat[0].z; cz = nouvmat[0].z;
			au = 1.f; bu = 0.f; cu = 0.f;
			av = 0.f; bv = 1.f; cv = 0.f;
		}

			//FIXFIXFIX
			//Quick & dirty estimation of distance
		//d2 = (posx-glipos.x)*glifor.x + (posy-glipos.y)*glifor.y + (posz-glipos.z)*glifor.z; //perp. distance
		//d2 /= sqrt(rigx*rigx + rigy*rigy + rigz*rigz + dowx*dowx + dowy*dowy + dowz*dowz + forx*forx + fory*fory + forz*forz); //voxel size in world units
		//d2 *= grhzup20/524288.0; //MIP FACTOR; FIXFIXFIX:need ability to change externally!
		//for(d=1.0;(tt->lowermip) && (d2 >= d);tt=tt->lowermip,d+=d);
		//d = 256.0; while ((tt->lowermip) && (pn[0].z > d)) { d *= 4.0; tt = tt->lowermip; } //FIXFIXFIX

			//ax*a + bx*b + cx = x
			//ay*a + by*b + cy = y
			//az*a + bz*b + cz = z
			//au*a + bu*b + cu = u
			//av*a + bv*b + cv = v
			//
			//gcc.p.x + (gcc.r.x*vx + gcc.d.x*vy + gcc.f.x*vz)*d = pt[0].x + (pt[1].x-pt[0].x)*u + (pt[2].x-pt[0].x)*v
			//gcc.p.y + (gcc.r.y*vx + gcc.d.y*vy + gcc.f.y*vz)*d = pt[0].y + (pt[1].y-pt[0].y)*u + (pt[2].y-pt[0].y)*v
			//gcc.p.z + (gcc.r.z*vx + gcc.d.z*vy + gcc.f.z*vz)*d = pt[0].z + (pt[1].z-pt[0].z)*u + (pt[2].z-pt[0].z)*v
			//
			//p1x*u + p2x*v + p3x*d = p0x
			//p1y*u + p2y*v + p3y*d = p0y
			//p1z*u + p2z*v + p3z*d = p0z
			//
			//Many steps of derivation skipped... as big simplifications occurred later :/
		p2x = ay*bz - az*by; p2y = az*bx - ax*bz; p2z = ax*by - ay*bx; f = p2x*cx + p2y*cy + p2z*cz; //if (f < 0) return;
		p0x = by*cz - bz*cy; p0y = bz*cx - bx*cz; p0z = bx*cy - by*cx;
		p1x = cy*az - cz*ay; p1y = cz*ax - cx*az; p1z = cx*ay - cy*ax;
#ifdef USEINTZ
		d = 1.0 / (f*(ZSCALE>>19));
#else
		d = 1048576.0 / (f*gcc.h.z*(float)(ZSCALE>>19));
#endif
		ax = (1.0/65536.0 )*d;
		ay = ((float)tt->x)*d;
		az = ((float)tt->y)*d;
		if (rendflags&RENDFLAGS_INTERP) { cu -= .5/((float)tt->x); cv -= .5/((float)tt->y); } //.5 for QIfist rounding

		g_mat[0] = p2x*ax; g_mat[1] = (p0x*au + p1x*bu + p2x*cu)*ay; g_mat[2] = (p0x*av + p1x*bv + p2x*cv)*az;
		g_mat[3] = p2y*ax; g_mat[4] = (p0y*au + p1y*bu + p2y*cu)*ay; g_mat[5] = (p0y*av + p1y*bv + p2y*cv)*az;
		g_mat[6] = p2z*ax; g_mat[7] = (p0z*au + p1z*bu + p2z*cu)*ay; g_mat[8] = (p0z*av + p1z*bv + p2z*cv)*az;
		g_mat[6] = g_mat[6]*gcc.h.z - g_mat[0]*gcc.h.x - g_mat[3]*gcc.h.y;
		g_mat[7] = g_mat[7]*gcc.h.z - g_mat[1]*gcc.h.x - g_mat[4]*gcc.h.y;
		g_mat[8] = g_mat[8]*gcc.h.z - g_mat[2]*gcc.h.x - g_mat[5]*gcc.h.y;
	}
	else
	{
		memcpy(g_mat,ouvmat,sizeof(g_mat[0])*9);
		if (rendflags&RENDFLAGS_INTERP)
		{
			f = g_mat[0]*32768; g_mat[1] -= f; g_mat[2] -= f;
			f = g_mat[3]*32768; g_mat[4] -= f; g_mat[5] -= f;
			f = g_mat[6]*32768; g_mat[7] -= f; g_mat[8] -= f;
		}
	}
	g_di8 = g_mat[0]*FLATSTEPSIZ;
	g_ui8 = g_mat[1]*FLATSTEPSIZ;
	g_vi8 = g_mat[2]*FLATSTEPSIZ;

	g_rgbmul = ((rgbmul&0xffffff)|0x80000000);
	g_ttp = tt->p; g_ttf = tt->f; i = bsr(g_ttp);
	g_xmsk = (1<<bsr(tt->x))-1; g_xmsk <<= 2;
	g_ymsk = (1<<bsr(tt->y))-1; g_ymsk <<= i;
	g_ttps = 16-i;
	grendflags = rendflags;
	if (!(rendflags&RENDFLAGS_INTERP)) htflatnear(-1); else htflatbilin(-1); //init self-modifying code

#if (FIXNEWFILL == 0)
	ordn = polysortfill(pn,n,tlong); miny = 0x7fffffff; maxy = 0x80000000;
	for(k=0;k<ordn;k++)
	{
		i = (tlong[k]&0x3fffffff); j = pn[i].n;
		if (isy[j] < isy[i]) { yy0 = isy[j]; yy1 = isy[i]; lptr = &v2h_lmost[4]; }
							 else { yy0 = isy[i]; yy1 = isy[j]; lptr = &v2h_rmost[4]; }
		if (isy[i] != isy[j])
		{
			g = (pn[j].x-pn[i].x)/(pn[j].y-pn[i].y); f = (yy0-pn[i].y)*g + pn[i].x;
#if 0
			for(y=yy0;y<yy1;y++) { lptr[y] = (long)ceil(f); f += g; }
#else
			_asm
			{
				mov eax, 0x5f80 ;round +inf
				mov l, eax
				ldmxcsr l

				mov edx, yy0
				mov ecx, lptr
				mov eax, yy1
				sub edx, eax
				lea ecx, [ecx+eax*4]
				movss xmm2, f         ;xmm2:  -  , -  , -  , f
				movss xmm0, g         ;xmm0:  -  , -  , -  , g

				test edx, 3
				jz short dqskip
	 dqrast1:cvtss2si eax, xmm2
				mov [ecx+edx*4], eax
				addss xmm2, xmm0
				add edx, 1
				jge short dqend
				test edx, 3
				jnz short dqrast1

	  dqskip:shufps xmm2, xmm2, 0  ;xmm2:  f  , f  , f  , f
				shufps xmm0, xmm0, 0  ;xmm0:  g  , g  , g  , g
				movaps xmm1, xmm0     ;xmm1:  g  , g  , g  , g
				mulps xmm0, dpqmulval ;xmm0: 3g  ,2g  ,1g  ,0g
				addps xmm0, xmm2      ;xmm0: 3g+f,2g+f, g+f,  f
				mulps xmm1, dpqfours  ;xmm1: 4g  ,4g  ,4g  ,4g

	 dqrast4:cvtps2pi mm0, xmm0
				movhlps xmm2, xmm0
				cvtps2pi mm1, xmm2
				movq [ecx+edx*4], mm0
				movq [ecx+edx*4+8], mm1

				;movaps xmm2, xmm0
				;addps xmm2, dpqmagic
				;psubd xmm2, dpqsub  ;P4 ONLY!
				;movups [ecx+edx*4], xmm2

				addps xmm0, xmm1
				add edx, 4
				jl short dqrast4

		dqend:emms

				mov eax, 0x3f80 ;round -inf
				mov l, eax
				ldmxcsr l
			}
#endif
		}

		if (!(tlong[k]&0x80000000)) continue;
		if (yy0 < miny) miny = yy0; //Accumulate consecutive y-extent of right side
		if (yy1 > maxy) maxy = yy1;
		if (!(tlong[k]&0x40000000)) continue;

		for(i=miny;i<maxy;i++)
		{
			ghlinbuf[i][0] = (short)min(max(v2h_lmost[i+4],0),gcc.c.x);
			ghlinbuf[i][1] = (short)min(max(v2h_rmost[i+4],0),gcc.c.x);
			ghlinbuf[i][2] = (short)i;
		}
		if (!(rendflags&RENDFLAGS_INTERP)) htrun(htflatnear ,miny,maxy,drawpoly_numcpu); //Draw bunch
												else htrun(htflatbilin,miny,maxy,drawpoly_numcpu);
		miny = 0x7fffffff; maxy = 0x80000000;
	}
#else
	if (!(rendflags&RENDFLAGS_INTERP)) poly_raster(pn,n,htflatnear);
											else poly_raster(pn,n,htflatbilin);
#endif
}

	//Cover-up function for what will eventually be a triangulator..
void drawpoly (tiltyp *tt, vertyp *pt, int n, int rgbmul, float hsc, float *ouvmat, int rendflags)
{
#ifndef DISABLEHEIGHTMAP
	if ((!(rendflags&RENDFLAGS_HEIGHT)) || (hsc == 0.f))
#endif
													  drawpolyflat(tt,pt,n,rgbmul,hsc,ouvmat,rendflags);
#ifndef DISABLEHEIGHTMAP
												else drawpolytopo(tt,pt,n,rgbmul,hsc,ouvmat,rendflags);
#endif
}

void drawpoly_setup (tiletype *dd, intptr_t lzbufoff, point3d *lpos, point3d *lrig, point3d *ldow, point3d *lfor, float hx, float hy, float hz)
{
	gcc.c.f = dd->f;          gcc.c.p = dd->p; gcc.c.x = dd->x; gcc.c.y = dd->y;
	gcc.z.f = dd->f+lzbufoff; gcc.z.p = dd->p; gcc.z.x = dd->x; gcc.z.y = dd->y;
	gcc.p = (*lpos); gcc.r = (*lrig); gcc.d = (*ldow); gcc.f = (*lfor);
	gcc.h.x = hx; gcc.h.y = hy; gcc.h.z = hz;

		//init self-modifying code
	htflatnear(-2);
	htflatbilin(-2);
}

void drawpoly_setup (tiletype *dd, intptr_t lzbufoff, dpoint3d *lpos, dpoint3d *lrig, dpoint3d *ldow, dpoint3d *lfor, float hx, float hy, float hz)
{
	point3d fpos, frig, fdow, ffor;
	fpos.x = lpos->x; fpos.y = lpos->y; fpos.z = lpos->z;
	frig.x = lrig->x; frig.y = lrig->y; frig.z = lrig->z;
	fdow.x = ldow->x; fdow.y = ldow->y; fdow.z = ldow->z;
	ffor.x = lfor->x; ffor.y = lfor->y; ffor.z = lfor->z;
	drawpoly_setup(dd,lzbufoff,&fpos,&frig,&fdow,&ffor,hx,hy,hz);
}

void drawpoly_init (void)
{
	SYSTEM_INFO si;
	long i;

	if (!drawpoly_numcpu)
	{
		GetSystemInfo(&si);
		drawpoly_numcpu = min(max(si.dwNumberOfProcessors,1),CPULIMIT);
	}

	for(i=1;i<sizeof(frecipi)/sizeof(frecipi[0]);i++)
	{
		frecipi[i] = 1.0/((float)i);
		recipi32[i] = (int)floor(frecipi[i]*65535.0);
		recipi64[i] = (((__int64)recipi32[i])    ) + (((__int64)recipi32[i])<<16) +
						  (((__int64)recipi32[i])<<32) + (((__int64)recipi32[i])<<48);
	}
#ifndef DISABLEHEIGHTMAP
	memset(v2h_vmost,-1,sizeof(v2h_vmost));
#endif
}

//--------------------------------------------------------------------------------------------------

void applyshade (tiltyp *tt, long shx, long shy)
{
	unsigned long *uptr;
	long i, j, k, x, y, dx, dy, ak[4], po2, xmsk, msk;

	uptr = (unsigned long *)tt->f;
	po2 = (tt->p>>2);
	xmsk = tt->x-1;
	msk = tt->y*po2-1;
	for(y=i=j=0;y<tt->y;y++,j+=po2)
		for(x=0,i=j;x<tt->x;x++,i++)
		{
			ak[0] = (uptr[((x-  1)&xmsk)+j]>>24);
			ak[1] = (uptr[((x+  1)&xmsk)+j]>>24);
			ak[2] = (uptr[((i-po2)& msk)  ]>>24);
			ak[3] = (uptr[((i+po2)& msk)  ]>>24);
			dx = (ak[1]-ak[0])*1;
			dy = (ak[3]-ak[2])*2;
			k = ((dx*shx + dy*shy)>>16);
			uptr[i] = (min(max((signed)((uptr[i]>> 0)&255)+k,0),255)<< 0) +
						 (min(max((signed)((uptr[i]>> 8)&255)+k,0),255)<< 8) +
						 (min(max((signed)((uptr[i]>>16)&255)+k,0),255)<<16) +
						 (uptr[i]&0xff000000);
		}
}

void fixtex4grou (tiltyp *tt)
{
#if (TESTDEPTH == 1)
	long i; for(i=0;i<tt->x*tt->y;i++) *(long *)(tt->f + (i<<2)) |= 0xff000000;
#endif

	memcpy((void *)(tt->p*tt->y + tt->f),(void *)tt->f,tt->p+4); //Copy top to bottom
}

#if defined(_MSC_VER)
static void divconst_setdenom (long *twolongstate, long denom)
{
	_asm
	{
		mov eax, denom
		mov edx, 2
		cmp eax, edx
		jae short dc_l1
		neg eax
		jmp short dc_l2
dc_l1:lea ecx, [eax+eax]
		div ecx
dc_l2:mov ecx, twolongstate
		mov [ecx+4], eax ;dc_mul
		mul dword ptr denom
		xor eax, eax
		cmp edx, 1
		setc al
		mov [ecx], eax ;dc_add
	}
}
static _inline long divconst (long *twolongstate, long numer)
{
	_asm
	{
		mov eax, numer
		mov edx, twolongstate
		add eax, [edx] ;dc_add
		mul dword ptr [edx+4] ;dc_mul
		mov eax, edx
	}
}
#else
static void divconst_setdenom (long *twolongstate, long denom)
	{ twolongstate[0] = divconst_denom; divconst_denom = denom; }
static long divconst (long *twolongstate, long numer) { return(numer/twolongstate[0]); }
#endif

	//Scale texture to next higher pow2. Uses box_sum_mip (no bilinear)
	//Integrate area of double box. Assumes read area is smaller than write area
	//���������
	//�  ��ͻ �
	//���������
	//�  ��ͼ �
	//���������
void scaletex_boxsum (tiltyp *rt, tiltyp *wt)
{
	long i, rx, ry, rxf, ryf, nxm1, nym1, col0[4], col1[4], col2, ds, divstate[4][2];
	long x, y, xx, yy, lwx, lwy, *lptr;
	unsigned char *ucptr, *ucptr2;

	lwx = bsr(wt->x);
	lwy = bsr(wt->y);

	divconst_setdenom(&divstate[0][0],1);
	divconst_setdenom(&divstate[1][0],rt->x);
	divconst_setdenom(&divstate[2][0],rt->y);
	divconst_setdenom(&divstate[3][0],rt->x*rt->y);
	nxm1 = wt->x-1; nym1 = wt->y-1;
	for(y=nym1,yy=nym1*rt->y;y>=0;y--,yy-=rt->y)
	{
		ry = (yy>>lwy); ryf = ((yy+rt->y)&nym1); ucptr2 = (unsigned char *)(ry*rt->p + rt->f);
		lptr = (long *)(y*wt->p + wt->f);
		for(x=nxm1,xx=nxm1*rt->x;x>=0;x--,xx-=rt->x)
		{
			rx = (xx>>lwx); rxf = ((xx+rt->x)&nxm1); ucptr = &ucptr2[rx<<2];
			for(i=0;i<4;i++) col0[i] = ucptr[i];
			ds = 0;
			if (rxf < rt->x) { ds = 1; for(i=0;i<4;i++) col0[i] = (ucptr[i+4]-col0[i])*rxf + col0[i]*rt->x; }
			if (ryf < rt->y)
			{
				ds += 2;
				for(i=0;i<4;i++) col1[i] = ucptr[rt->p+i];
				if (rxf < rt->x) { for(i=0;i<4;i++) col1[i] = (ucptr[rt->p+4+i]-col1[i])*rxf + col1[i]*rt->x; }
				for(i=0;i<4;i++) col0[i] = (col1[i]-col0[i])*ryf + col0[i]*rt->y;
			}
			for(i=0,col2=0;i<4;i++) col2 += (divconst(&divstate[ds][0],col0[i])<<(i<<3));
			lptr[x] = col2;
		}
	}
}

tiltyp *genmiptiltyp (tiltyp *tt)
{
	tiltyp *ntt;
	long i, j, x, y, r, g, b, *rptr0, *rptr1, *wptr;

	if ((!tt) || (tt->lowermip) || (tt->x < 1) || (tt->y < 1)) return(0);

	ntt = (tiltyp *)malloc(sizeof(tiltyp)); if (!ntt) return(0);
	ntt->x = (tt->x>>1);
	ntt->y = (tt->y>>1);
	ntt->p = (tt->p>>1);
	ntt->f = (long)malloc((ntt->y+1)*ntt->p + 4); if (!ntt->f) { free(ntt); return(0); }
	ntt->z = tt->z;
	ntt->shsc = tt->shsc;
	ntt->lowermip = 0;
	tt->lowermip = ntt;

	for(y=0;y<ntt->y;y++)
	{
		wptr  = (long *)(( y      )*ntt->p + ntt->f);
		rptr0 = (long *)(((y<<1)  )* tt->p +  tt->f);
		rptr1 = (long *)(((y<<1)+1)* tt->p +  tt->f);
		for(x=0;x<ntt->x;x++)
		{
			wptr[x] = ((( (rptr0[x<<1]    &0xff00ff) + ( rptr0[(x<<1)+1]    &0xff00ff) +
							 ( rptr1[x<<1]    &0xff00ff) + ( rptr1[(x<<1)+1]    &0xff00ff) + 0x20002)&0x3fc03fc)>>2) +
						 (((((rptr0[x<<1]>>8)&0xff00ff) + ((rptr0[(x<<1)+1]>>8)&0xff00ff) +
							 ((rptr1[x<<1]>>8)&0xff00ff) + ((rptr1[(x<<1)+1]>>8)&0xff00ff) + 0x20002)&0x3fc03fc)<<6);
		}
	}
	fixtex4grou(ntt);

	return(ntt);
}

	//cover-up function for kpzload: mallocs an extra line to make filter happy
	//flags: &1:signed:^0x80000000, &2:flip:^0xff000000
void kpzload4grou (const char *filnam, tiltyp *tt, float shsc, int flags)
{
	tiltyp pow2t, *ntt;
	char *buf;
	long x, y, nx, ny, lnx, lny, leng, xorval, *lptr;

	tt->f = 0; tt->lowermip = 0;
#if 0
	FILE *fil = fopen(filnam,"rb"); if (!fil) return;
	fseek(fil,0,SEEK_END); leng = ftell(fil); fseek(fil,0,SEEK_SET);
	buf = (char *)malloc(leng+4); if (!buf) { fclose(fil); return; } //FIXFIX:+4 avoids bug in KPLIB overrunning!
	fread(buf,leng,1,fil);
	fclose(fil);
#else
	if (!kzopen(filnam)) return;
	leng = kzfilelength();
	buf = (char *)malloc(leng+4); if (!buf) { kzclose(); return; } //FIXFIX:+4 avoids bug in KPLIB overrunning!
	kzread(buf,leng);
	kzclose();
#endif

	kpgetdim(buf,leng,(int *)&tt->x,(int *)&tt->y);

		//Allocate texture to next higher pow2
	if (tt->x <= 1) lnx = 0; else lnx = bsr(tt->x-1)+1;
	if (tt->y <= 1) lny = 0; else lny = bsr(tt->y-1)+1;
	nx = (1<<lnx); ny = (1<<lny);

	tt->p = (nx<<2); tt->f = (long)malloc((ny+1)*tt->p + 4);
	if (!tt->f) { free(buf); return; }
	if (kprender(buf,leng,tt->f,tt->p,nx,ny,0,0) < 0)
		{ free(buf); free((void *)tt->f); tt->f = 0; return; }
	free(buf);

		//Scale texture to next higher pow2. Uses box_sum_mip (no bilinear)
	if ((tt->x != nx) || (tt->y != ny))
	{
		pow2t.f = tt->f; pow2t.p = tt->p; pow2t.x = nx; pow2t.y = ny;
		scaletex_boxsum((tiltyp *)tt,&pow2t);
		tt->x = nx; tt->y = ny;
	}

	if (flags&3)
	{
		xorval = 0;
		if (flags&1) xorval ^= 0x80000000;
		if (flags&2) xorval ^= 0xff000000;
		for(y=tt->y-1;y>=0;y--)
		{
			lptr = (long *)(y*tt->p + tt->f);
			for(x=tt->x-1;x>=0;x--) lptr[x] ^= xorval;
		}
	}

	fixtex4grou(tt);
	tt->shsc = shsc;

		//Generate all lower mip-maps here:
	//for(ntt=tt;ntt=genmiptiltyp(ntt););
}

#ifdef STANDALONE

#include "sysmain.h"
#define PI 3.14159265358979323

tiltyp vox        = {0}; //generated at runtime in initapp
tiltyp voxcyl     = {0}; //generated at runtime in initapp
tiltyp voxgap     = {0}; //generated at runtime in initapp
tiltyp water[3]   = {0}; //realtime procedural water
//tiltyp tt_brick   = {0}; //not pow2
tiltyp tt_brick1  = {0};
tiltyp tt_bumps   = {0};
tiltyp tt_cblob   = {0};
tiltyp tt_cloud   = {0};
tiltyp tt_diamond = {0};
tiltyp tt_f1      = {0};
tiltyp tt_f2      = {0};
tiltyp tt_f3      = {0};
tiltyp tt_ken     = {0};
tiltyp tt_metal1  = {0};
tiltyp tt_styro   = {0};
tiltyp tt_wood    = {0};
tiltyp tt_testnon2= {0};

cam_t cam;
long obstatus, bstatus, numframes = 0;

	//NOTE: font is stored vertically first! (like .ART files)
static const __int64 font6x8[] = //256 DOS chars, from: DOSAPP.FON (tab blank)
{
	0x3E00000000000000,0x6F6B3E003E455145,0x1C3E7C3E1C003E6B,0x3000183C7E3C1800,
	0x7E5C180030367F36,0x000018180000185C,0x0000FFFFE7E7FFFF,0xDBDBC3FF00000000,
	0x0E364A483000FFC3,0x6000062979290600,0x0A7E600004023F70,0x2A1C361C2A003F35,
	0x0800081C3E7F0000,0x7F361400007F3E1C,0x005F005F00001436,0x22007F017F090600,
	0x606060002259554D,0x14B6FFB614000060,0x100004067F060400,0x3E08080010307F30,
	0x08083E1C0800081C,0x0800404040407800,0x3F3C3000083E083E,0x030F3F0F0300303C,
	0x0000000000000000,0x0003070000065F06,0x247E247E24000307,0x630000126A2B2400,
	0x5649360063640813,0x0000030700005020,0x00000000413E0000,0x1C3E080000003E41,
	0x08083E080800083E,0x0800000060E00000,0x6060000008080808,0x0204081020000000,
	0x00003E4549513E00,0x4951620000407F42,0x3649494922004649,0x2F00107F12141800,
	0x494A3C0031494949,0x0305097101003049,0x0600364949493600,0x6C6C00001E294949,
	0x00006CEC00000000,0x2400004122140800,0x2241000024242424,0x0609590102000814,
	0x7E001E555D413E00,0x49497F007E111111,0x224141413E003649,0x7F003E4141417F00,
	0x09097F0041494949,0x7A4949413E000109,0x00007F0808087F00,0x4040300000417F41,
	0x412214087F003F40,0x7F00404040407F00,0x04027F007F020402,0x3E4141413E007F08,
	0x3E00060909097F00,0x09097F005E215141,0x3249494926006619,0x3F0001017F010100,
	0x40201F003F404040,0x3F403C403F001F20,0x0700631408146300,0x4549710007087008,
	0x0041417F00000043,0x0000201008040200,0x01020400007F4141,0x8080808080800402,
	0x2000000007030000,0x44447F0078545454,0x2844444438003844,0x38007F4444443800,
	0x097E080008545454,0x7CA4A4A418000009,0x0000007804047F00,0x8480400000407D00,
	0x004428107F00007D,0x7C0000407F000000,0x04047C0078041804,0x3844444438000078,
	0x380038444444FC00,0x44784400FC444444,0x2054545408000804,0x3C000024443E0400,
	0x40201C00007C2040,0x3C6030603C001C20,0x9C00006C10106C00,0x54546400003C60A0,
	0x0041413E0800004C,0x0000000077000000,0x02010200083E4141,0x3C2623263C000001,
	0x3D001221E1A11E00,0x54543800007D2040,0x7855555520000955,0x2000785554552000,
	0x5557200078545555,0x1422E2A21C007857,0x3800085555553800,0x5555380008555455,
	0x00417C0100000854,0x0000004279020000,0x2429700000407C01,0x782F252F78007029,
	0x3400455554547C00,0x7F097E0058547C54,0x0039454538004949,0x3900003944453800,
	0x21413C0000384445,0x007C20413D00007D,0x3D00003D60A19C00,0x40413C00003D4242,
	0x002466241800003D,0x29006249493E4800,0x16097F00292A7C2A,0x02097E8840001078,
	0x0000785555542000,0x4544380000417D00,0x007D21403C000039,0x7A0000710A097A00,
	0x5555080000792211,0x004E51514E005E55,0x3C0020404D483000,0x0404040404040404,
	0x506A4C0817001C04,0x0000782A34081700,0x0014080000307D30,0x0814000814001408,
	0x55AA114411441144,0xEEBBEEBB55AA55AA,0x0000FF000000EEBB,0x0A0A0000FF080808,
	0xFF00FF080000FF0A,0x0000F808F8080000,0xFB0A0000FE0A0A0A,0xFF00FF000000FF00,
	0x0000FE02FA0A0000,0x0F0800000F080B0A,0x0F0A0A0A00000F08,0x0000F80808080000,
	0x080808080F000000,0xF808080808080F08,0x0808FF0000000808,0x0808080808080808,
	0xFF0000000808FF08,0x0808FF00FF000A0A,0xFE000A0A0B080F00,0x0B080B0A0A0AFA02,
	0x0A0AFA02FA0A0A0A,0x0A0A0A0AFB00FF00,0xFB00FB0A0A0A0A0A,0x0A0A0B0A0A0A0A0A,
	0x0A0A08080F080F08,0xF808F8080A0AFA0A,0x08080F080F000808,0x00000A0A0F000000,
	0xF808F8000A0AFE00,0x0808FF00FF080808,0x08080A0AFB0A0A0A,0xF800000000000F08,
	0xFFFFFFFFFFFF0808,0xFFFFF0F0F0F0F0F0,0xFF000000000000FF,0x0F0F0F0F0F0FFFFF,
	0xFE00241824241800,0x01017F0000344A4A,0x027E027E02000003,0x1800006349556300,
	0x2020FC00041C2424,0x000478040800001C,0x3E00085577550800,0x02724C00003E4949,
	0x0030595522004C72,0x1800182418241800,0x2A2A1C0018247E24,0x003C02023C00002A,
	0x0000002A2A2A2A00,0x4A4A510000242E24,0x00514A4A44000044,0x20000402FC000000,
	0x2A08080000003F40,0x0012241224000808,0x0000000609090600,0x0008000000001818,
	0x02023E4030000000,0x0900000E010E0100,0x3C3C3C0000000A0D,0x000000000000003C,
};
static void print6x8 (long ox, long y, long fcol, long bcol, const char *fmt, ...)
{
	va_list arglist;
	char st[1024], *c, *v;
	long i, j, ie, x, *lp, *lpx;

	if (!fmt) return;
	va_start(arglist,fmt);
	if (_vsnprintf((char *)&st,sizeof(st)-1,fmt,arglist)) st[sizeof(st)-1] = 0;
	va_end(arglist);

	lp = (long *)(y*cam.c.p+cam.c.f);
	for(j=1;j<256;y++,lp=(long *)(((long)lp)+cam.c.p),j+=j)
		if ((unsigned long)y < cam.c.y)
			for(c=st,x=ox;*c;c++,x+=6)
			{
				v = (char *)(((long)*c)*6 + (long)font6x8);
				ie = min(cam.c.x-x,6); lpx = &lp[x];
				for(i=max(-x,0);i<ie;i++) { if (v[i]&j) lpx[i] = fcol; else if (bcol >= 0) lpx[i] = bcol; }
				if ((*c) == 9) { if (bcol >= 0) for(i=6;i<18;i++) lpx[i] = bcol; x += 2*6; }
			}
}

	//Compatible with memset except:
	//   1. All 32 bits of v are used and expected to be filled
	//   2. Writes max((n+7)&~7,8) bytes
	//   3. Assumes d is aligned on 8 byte boundary
static void memset8 (void *d, long v, long n)
{
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
}

static _inline void fcossin (float a, float *c, float *s)
{
	_asm
	{
		fld a
		fsincos
		mov eax, c
		fstp dword ptr [eax]
		mov eax, s
		fstp dword ptr [eax]
	}
}

void orthorotate (float ox, float oy, float oz, point3d *ist, point3d *ihe, point3d *ifo)
{
	float f, t, dx, dy, dz, rr[9];

	fcossin(ox,&ox,&dx);
	fcossin(oy,&oy,&dy);
	fcossin(oz,&oz,&dz);
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

void uninitapp ()
{
	int i;

	if (cam.z.z) { free((void *)cam.z.z); cam.z.x = cam.z.y = cam.z.p = cam.z.f = cam.z.z = 0; }
	for(i=3-1;i>=0;i--)
		if (water[i].f) { free((void *)water[i].f); water[i].f = 0; }
	if (vox.f        ) { free((void *)vox.f        ); vox.f        = 0; }
	if (tt_testnon2.f) { free((void *)tt_testnon2.f); tt_testnon2.f= 0; }
	if (tt_wood.f    ) { free((void *)tt_wood.f    ); tt_wood.f    = 0; }
	if (tt_styro.f   ) { free((void *)tt_styro.f   ); tt_styro.f   = 0; }
	if (tt_metal1.f  ) { free((void *)tt_metal1.f  ); tt_metal1.f  = 0; }
	if (tt_ken.f     ) { free((void *)tt_ken.f     ); tt_ken.f     = 0; }
	if (tt_f3.f      ) { free((void *)tt_f3.f      ); tt_f3.f      = 0; }
	if (tt_f2.f      ) { free((void *)tt_f2.f      ); tt_f2.f      = 0; }
	if (tt_f1.f      ) { free((void *)tt_f1.f      ); tt_f1.f      = 0; }
	if (tt_diamond.f ) { free((void *)tt_diamond.f ); tt_diamond.f = 0; }
	if (tt_cloud.f   ) { free((void *)tt_cloud.f   ); tt_cloud.f   = 0; }
	if (tt_cblob.f   ) { free((void *)tt_cblob.f   ); tt_cblob.f   = 0; }
	if (tt_bumps.f   ) { free((void *)tt_bumps.f   ); tt_bumps.f   = 0; }
	if (tt_brick1.f  ) { free((void *)tt_brick1.f  ); tt_brick1.f  = 0; }
	//if (tt_brick.f   ) { free((void *)tt_brick.f   ); tt_brick.f   = 0; }
}

long initapp (long argc, char **argv)
{
	float f;
	int i, j, k, x, y, dx, dy, col, hei[3];
	unsigned char *uptr;

	xres = 800; yres = 600; colbits = 32; fullscreen = 0; prognam = "DrawPoly by Ken Silverman";

	for(i=argc-1;i>0;i--)
	{
		if ((argv[i][0] != '/') && (argv[i][0] != '-')) { /*argfilindex = i;*/ continue; }
		if (!stricmp(&argv[i][1],"win")) { fullscreen = 0; continue; }
		if (!stricmp(&argv[i][1],"full")) { fullscreen = 1; continue; }
		if (!memicmp(&argv[i][1],"cpu=",4)) { drawpoly_numcpu = min(max(atol(&argv[i][5]),1),CPULIMIT); continue; }
		//if (argv[i][1] == '?') { showinfo(); return(-1); }
		if ((argv[i][1] >= '0') && (argv[i][1] <= '9'))
		{
			k = 0; x = 0;
			for(j=1;;j++)
			{
				if ((argv[i][j] >= '0') && (argv[i][j] <= '9'))
					{ k = (k*10+argv[i][j]-48); continue; }
				switch (x)
				{
					case 0: xres = k; break;
					case 1: yres = k; break;
					case 2: /*colbits = k;*/ fullscreen = 1; break;
				}
				if (!argv[i][j]) break;
				x++; if (x > 2) break;
				k = 0;
			}
		}
	}
	if (xres > MAXXDIM) xres = MAXXDIM;
	if (yres > MAXYDIM) yres = MAXYDIM;

	drawpoly_init();

	//kpzload4grou("brick.png"  ,&tt_brick  ,1.0  ,1); // -1,2
	kpzload4grou("brick1.png" ,&tt_brick1 ,  0.5 ,2); //  4,217
	kpzload4grou("bumps.png"  ,&tt_bumps  , 20.0 ,3); // -3,5
	kpzload4grou("cblob.png"  ,&tt_cblob  , 10.0 ,0); //  0,19
	kpzload4grou("cloud.png"  ,&tt_cloud  ,  1.0 ,0); //  255
	kpzload4grou("diamond.png",&tt_diamond,100.0 ,3); // -3,5
	kpzload4grou("f1dr.png"   ,&tt_f1     ,  0.6 ,2); //127,255
	kpzload4grou("f2.png"     ,&tt_f2     ,  0.6 ,2); // 30,255
	kpzload4grou("f3.png"     ,&tt_f3     ,  0.08,2); //  0,254
	kpzload4grou("ken.png"    ,&tt_ken    ,  2.0 ,0); //  0,8
	kpzload4grou("metal1.png" ,&tt_metal1 ,  0.25,2); //  1,240
	kpzload4grou("styro.png"  ,&tt_styro  , 20.0 ,2); //  0,6
	kpzload4grou("wood.png"   ,&tt_wood   , 10.0 ,1); // -2,4
	//kpzload4grou("0190_limesto3.jpg",&tt_testnon2,1.0,0);
	//kpzload4grou("fuk.png",&tt_testnon2,1.0,0);

#if 0
	kpzload4grou("\\kwin\\voxlap\\vxl\\tomland.png",&vox,8.0,0);
#else
	vox.x = 128; vox.y = 128; vox.p = (vox.x<<2); vox.f = (long)malloc(vox.p*(vox.y+1)+4); vox.shsc = 1.0;
	for(y=0;y<vox.y;y++)
		for(x=0;x<vox.x;x++)
		{
			col = *(long *)(tt_wood.p*(y&(tt_wood.y-1)) + ((x&(tt_wood.x-1))<<2) + tt_wood.f);
			f = (((col>>16)&255) < 116)*12+128;
			f += (cos(x*PI*2/vox.x)+sin(y*PI*2/vox.y))*48-32;
			f += (((*(long *)(tt_cloud.p*(y&(tt_cloud.y-1)) + ((x&(tt_cloud.x-1))<<2) + tt_cloud.f)>>16)&255)-128)*.4;
			*(long *)(vox.p*y + (x<<2) + vox.f) = (((int)f)<<24)+(col&0xffffff);
		}

	for(y=0;y<vox.y;y++)
		for(x=0;x<vox.x;x++)
		{
			uptr = (unsigned char *)(vox.p*y + (x<<2) + vox.f);

			hei[0] = (*(unsigned char *)(vox.p*( y   &(vox.y-1)) + (( x   &(vox.x-1))<<2) + vox.f + 3));
			hei[1] = (*(unsigned char *)(vox.p*( y   &(vox.y-1)) + (((x+1)&(vox.x-1))<<2) + vox.f + 3));
			hei[2] = (*(unsigned char *)(vox.p*((y+1)&(vox.y-1)) + (( x   &(vox.x-1))<<2) + vox.f + 3));
			dx = hei[1]-hei[0];
			dy = hei[2]-hei[0];
			i = dx*768 - dy*512 + 32768;
			uptr[0] = min(max((uptr[0]*i)>>15,4),255-4);
			uptr[1] = min(max((uptr[1]*i)>>15,4),255-4);
			uptr[2] = min(max((uptr[2]*i)>>15,4),255-4);
			//uptr[3] = min(max((labs(y-64))*4,0),255); //roof
#if (TESTDEPTH == 1)
			uptr[3] = 255;
#endif
		}

	cam.p.z = -32.f; //hack
	applyshade(&vox,1<<16,1<<16);
	fixtex4grou(&vox);
#endif

#if 1
	for(i=0;i<3;i++)
	{
		water[i].x = 128; water[i].y = 128; water[i].p = (water[i].x<<2);
		water[i].f = (long)malloc(water[i].p*(water[i].y+1)+4);
		water[i].shsc = 2.0;
		memset((void *)water[i].f,0,water[i].p*water[i].y);
	}
#endif

#if 1
	voxcyl.x = 64; voxcyl.y = 64; voxcyl.p = (voxcyl.x<<2);
	voxcyl.f = (long)malloc(voxcyl.p*(voxcyl.y+1)+4); voxcyl.shsc = 1.0;
	f = ((float)(voxcyl.x-1))*.5;
	for(y=0;y<voxcyl.y;y++)
		for(x=0;x<voxcyl.x;x++)
		{
			hei[0] = *(long *)(tt_wood.p*(y&(tt_wood.y-1)) + ((x&(tt_wood.x-1))<<2) + tt_wood.f);
			col = *(long *)(vox.p*(y&(vox.y-1)) + ((x&(vox.x-1))<<2) + vox.f);
			*(long *)(voxcyl.p*y + (x<<2) + voxcyl.f) = (col&0xffffff) +
				(((int)(min(  (((hei[0]>>16)&255) < 116)*24 +
								  255-8.0*sqrt(f*f+1 - ((float)x-f)*((float)x-f))  ,255)  ))<<24);
		}
	applyshade(&voxcyl,0,-1<<16);
	fixtex4grou(&voxcyl);
#endif

#if 1
	voxgap.x = 64; voxgap.y = 64; voxgap.p = (voxgap.x<<2);
	voxgap.f = (long)malloc(voxgap.p*(voxgap.y+1)+4); voxgap.shsc = 1.0;
	for(y=0;y<voxgap.y;y++)
		for(x=0;x<voxgap.x;x++)
		{
			col = *(long *)(vox.p*(y&(vox.y-1)) + ((x&(vox.x-1))<<2) + vox.f);
			*(long *)(voxgap.p*y + (x<<2) + voxgap.f) = (col&0xffffff) + ((32+(rand()&15))<<24);
		}
	fixtex4grou(&voxgap);
#endif

	cam.p.x = 0; cam.p.y = 0; cam.p.z = 0;
	cam.r.x = 1; cam.r.y = 0; cam.r.z = 0;
	cam.d.x = 0; cam.d.y = 0; cam.d.z = 1;
	cam.f.x = 0; cam.f.y =-1; cam.f.z = 0;

	cam.z.f = cam.z.p = cam.z.x = cam.z.y = cam.z.z = 0;

	return(0);
}

void doframe ()
{
	static double tim, otim, dt, zoom = 1.0, periodavg = 0.0;
	static long runwater = 0, rendflags = RENDFLAGS_INTERP|RENDFLAGS_HEIGHT|RENDFLAGS_COVSID;
	float f, fmousx, fmousy, fx, fy, fz;
	vertyp vt[16];
	int i, y, n;

	otim = tim; readklock(&tim); dt = tim-otim;
	obstatus = bstatus; readmouse(&fmousx,&fmousy,&bstatus);
	readkeyboard(); if (keystatus[1]) quitloop();

	f = dt*64.f;
	if (keystatus[0x2a]) f /= 8.f;
	if (keystatus[0x36]) f *= 8.f;
	fx = ((float)(keystatus[0xcd]-keystatus[0xcb]))*f;
	fy = ((float)(keystatus[0x52]-keystatus[0x9d]))*f;
	fz = ((float)(keystatus[0xc8]-keystatus[0xd0]))*f;
	cam.p.x += cam.r.x*fx + cam.d.x*fy + cam.f.x*fz;
	cam.p.y += cam.r.y*fx + cam.d.y*fy + cam.f.y*fz;
	cam.p.z += cam.r.z*fx + cam.d.z*fy + cam.f.z*fz;
	if (!(bstatus&2)) orthorotate(cam.r.z*.10,fmousy*.01,fmousx*.01,&cam.r,&cam.d,&cam.f);
					 else orthorotate(fmousx*-.01,fmousy*.01,         0,&cam.r,&cam.d,&cam.f);
	if (keystatus[0xb5]) //KP/
		{ f = zoom; zoom *= pow(3.0,-dt); if ((f > 1) && (zoom <= 1)) { zoom = 1.0; keystatus[0xb5] = 0; } }
	if (keystatus[0x37]) //KP*
		{ f = zoom; zoom *= pow(3.0,+dt); if ((f < 1) && (zoom >= 1)) { zoom = 1.0; keystatus[0x37] = 0; } }
	if (keystatus[0x4c]) zoom = 1.0; //KP5
	if (ext_keystatus[0x35]) // /
	{
		ext_keystatus[0x35] = 0;
		zoom = 1.0;
		for(i=0;i<3;i++) memset((void *)water[i].f,0,water[i].p*water[i].y);
		runwater = 0;
	}
	if (keystatus[0x4b]) { keystatus[0x4b] = 0; drawpoly_anginc = max(drawpoly_anginc*.5 , 1.0); } //KP4
	if (keystatus[0x4d]) { keystatus[0x4d] = 0; drawpoly_anginc = min(drawpoly_anginc*2.0,64.0); } //KP6
	//if (keystatus[0x4b]) { drawpoly_anginc = max(drawpoly_anginc-dt, 1.0); } //KP4
	//if (keystatus[0x4d]) { drawpoly_anginc = min(drawpoly_anginc+dt,64.0); } //KP6

	if (ext_keystatus[0x2]) { ext_keystatus[0x2] = 0; rendflags = 0; }
	if (ext_keystatus[0x3]) { ext_keystatus[0x3] = 0; rendflags = RENDFLAGS_INTERP; }
	if (ext_keystatus[0x4]) { ext_keystatus[0x4] = 0; rendflags = RENDFLAGS_HEIGHT; }
	if (ext_keystatus[0x5]) { ext_keystatus[0x5] = 0; rendflags = RENDFLAGS_INTERP|RENDFLAGS_HEIGHT; }
	if (ext_keystatus[0x6]) { ext_keystatus[0x6] = 0; rendflags = RENDFLAGS_HEIGHT|RENDFLAGS_COVSID; }
	if (ext_keystatus[0x7]) { ext_keystatus[0x7] = 0; rendflags = RENDFLAGS_INTERP|RENDFLAGS_HEIGHT|RENDFLAGS_COVSID; }

	//-----------------------------------------------------------------------------------------------

	if (!startdirectdraw(&cam.c.f,&cam.c.p,&cam.c.x,&cam.c.y)) goto skipdd;
	cam.h.x = ((float)cam.c.x)*.5; cam.h.y = ((float)cam.c.y)*.5; cam.h.z = cam.h.x*zoom;

	i = cam.c.p*cam.c.y+256;
	if (i > cam.z.p*cam.z.y)
		cam.z.z = (long)realloc((void *)cam.z.z,i); //note: tiltyp.z used for unusual purpose (malloced ptr)
	cam.z.x = cam.c.x; cam.z.y = cam.c.y; cam.z.p = cam.c.p;

		//zbuffer aligns its memory to the same pixel boundaries as the screen!
		//WARNING: Pentium 4's L2 cache has severe slowdowns when 65536-64 <= (zbufoff&65535) < 64
	cam.z.f = ((cam.z.z - cam.c.f + 127)&~255) + cam.c.f + 128;
	for(y=0;y<cam.c.y;y++) memset8((void *)(cam.c.p*y + cam.z.f),0x7f7f7f7f,cam.c.x<<2);

	drawpoly_setup((tiletype *)&cam.c,cam.z.f-cam.c.f,&cam.p,&cam.r,&cam.d,&cam.f,cam.h.x,cam.h.y,cam.h.z);

#if 1
	clearscreen(0x20a0b0); //FIX
#if 0
	ghsc = ((float)(8>>LHSCALE))/(vox.shsc*1.0); drawgrou(&vox,0,0,rendflags);
#else
	if (vox.x == 1024) f = 160.0; else f = 32.0;
	vt[0].x = cam.p.x-256; vt[0].y = cam.p.y-256; vt[0].z = f; vt[0].n = 1;
	vt[1].x = cam.p.x+256; vt[1].y = cam.p.y-256; vt[1].z = f; vt[1].n = 2;
	vt[2].x = cam.p.x+256; vt[2].y = cam.p.y+256; vt[2].z = f; vt[2].n = 3;
	vt[3].x = cam.p.x-256; vt[3].y = cam.p.y+256; vt[3].z = f; vt[3].n = 0;
	vt[0].u = vt[0].x/((float)vox.x); vt[0].v = vt[0].y/((float)vox.y);
	vt[1].u = vt[1].x/((float)vox.x); vt[1].v = vt[1].y/((float)vox.y);
	vt[2].u = vt[2].x/((float)vox.x); vt[2].v = vt[2].y/((float)vox.y);
	vt[3].u = vt[3].x/((float)vox.x); vt[3].v = vt[3].y/((float)vox.y);
	//fx = sin(tim*7.4)*.35+1.0; fy = cos(tim*9.3)*.29+1.0; for(i=0;i<4;i++) { vt[i].u *= fx; vt[i].v *= fy; }
	//f = sin(tim*7.4)*.35; for(i=0;i<4;i++) { vt[i].u += vt[i].v*f; }
	drawpoly(&vox,vt,4,0x808080,1.0,0,rendflags);
#endif

#if 1
		//ceiling square
	vt[0].x =  0; vt[0].y = 64-64; vt[0].z = -80; vt[0].u = 0; vt[0].v = 0; vt[0].n = 1;
	vt[1].x = 64; vt[1].y = 64-64; vt[1].z = -80; vt[1].u =.5; vt[1].v = 0; vt[1].n = 2;
	vt[2].x = 64; vt[2].y =  0-64; vt[2].z = -80; vt[2].u =.5; vt[2].v =.5; vt[2].n = 3;
	vt[3].x =  0; vt[3].y =  0-64; vt[3].z = -80; vt[3].u = 0; vt[3].v =.5; vt[3].n = 0;
	drawpoly(&tt_f1,vt,4,0x808080,1.0,0,rendflags);
#endif

#if 1
		//upright equilateral polygon
	n = min(5,sizeof(vt)/sizeof(vt[0])); f = (PI*2)/(float)n;
	for(i=0;i<n;i++)
	{
		vt[i].x = cos(((float)i+.5+tim)*f)*64;
		vt[i].y = -128;
		vt[i].z = sin(((float)i+.5+tim)*f)*64-40;
		vt[i].u = cos(((float)i+.125)*f)*.5;
		vt[i].v = sin(((float)i+.125)*f)*.5;
		vt[i].n = i+1;
	}
	vt[n-1].n = 0;
	drawpoly(&tt_brick1,vt,n,0x808080,1.0,0,rendflags);
	//drawpoly(&tt_brick1,vt,n,0x808080,1.0+sin(tim*4.0)*.5,0,rendflags);
	//tt_brick1,tt_bumps,tt_cblob,tt_cloud,tt_diamond,tt_f1,tt_f2,tt_f3,tt_ken,tt_metal1,tt_styro,tt_wood
#endif

#if 1
	{
	unsigned long *uptr;
	long j, x, y, dx, dy, ak[4], *lptr, *wptr;

	if (bstatus&1)
	{
		typedef struct { long x, y; float frq, off, amp; } watemit_t;
		watemit_t watemit[] =
		{
			{ 12, 38,0.071,0.73,65535.0}, { 12, 38,0.113,1.74,32767.0}, { 12, 38,0.237,2.86,16383.0},
			{ 89, 12,0.085,1.81,65535.0}, { 89, 12,0.127,2.37,32767.0}, { 89, 12,0.213,0.83,16383.0},
			{ 38,115,0.068,2.84,65535.0}, { 38,115,0.133,0.37,32767.0}, { 38,115,0.225,1.02,16383.0},
			{115, 89,0.064,2.34,65535.0}, {115, 89,0.137,0.57,32767.0}, {115, 89,0.227,0.95,16383.0},
			{ 64, 64,0.057,0.91,65535.0}, { 64, 64,0.141,1.23,32767.0}, { 64, 64,0.206,2.15,16383.0},
		};
		for(i=0;i<sizeof(watemit)/sizeof(watemit[0]);i++)
			*(long *)(watemit[i].y*water[numframes&1].p + (watemit[i].x<<2) + water[numframes&1].f) +=
				sin(((float)numframes)*watemit[i].frq + watemit[i].off)*watemit[i].amp;
		runwater = 2;
	}

	if (!(runwater&1))
	{
		if (!runwater) runwater = 1;
		lptr = (long *)water[numframes&1].f;
		uptr = (unsigned long *)water[numframes&1^1].f;
		wptr = (long *)water[2].f;
		for(y=i=0;y<128;y++)
			for(x=0;x<128;x++,i++)
			{
				ak[0] = lptr[((x-1)&(128-1))+y*128];
				ak[1] = lptr[((x+1)&(128-1))+y*128];
				ak[2] = lptr[(i-128)&(128*128-1)];
				ak[3] = lptr[(i+128)&(128*128-1)];
				j = ((ak[0] + ak[1] + ak[2] + ak[3])>>1) - uptr[i]; uptr[i] = j-(j>>12);
				dx = ((ak[1]-ak[0])>>8);
				dy = ((ak[3]-ak[2])>>8);
				j = ((j>>11)+128)&255;
				wptr[i] = min(max(dx+dy+((128-j)>>2)+128,0),255)*0x000101 + (j<<24);
			}
		fixtex4grou(&water[2]);
	}

		//wall square
	vt[0].x = -160; vt[0].y = 64; vt[0].z =-128+64; vt[0].u = 0; vt[0].v = 0; vt[0].n = 1;
	vt[1].x = -160; vt[1].y =-64; vt[1].z =-128+64; vt[1].u = 2; vt[1].v = 0; vt[1].n = 2;
	vt[2].x = -160; vt[2].y =-64; vt[2].z =   0+64; vt[2].u = 2; vt[2].v = 2; vt[2].n = 3;
	vt[3].x = -160; vt[3].y = 64; vt[3].z =   0+64; vt[3].u = 0; vt[3].v = 2; vt[3].n = 0;
	drawpoly(&water[2],vt,4,0x808080,1.0,0,rendflags);
	//drawpoly(&water[2],vt,4,0x808080,1.0,0,rendflags&~RENDFLAGS_HEIGHT);
	}
#endif

#if 1
		//'KEN'
	vt[0].x = 112; vt[0].y = -96; vt[0].z =-48; vt[0].u = 0; vt[0].v = 0; vt[0].n = 1;
	vt[1].x = 133; vt[1].y = -60; vt[1].z =-48; vt[1].u = 1; vt[1].v = 0; vt[1].n = 2;
	vt[2].x = 133; vt[2].y = -60; vt[2].z = -8; vt[2].u = 1; vt[2].v = 1; vt[2].n = 3;
	vt[3].x = 112; vt[3].y = -96; vt[3].z = -8; vt[3].u = 0; vt[3].v = 1; vt[3].n = 0;
	f = tim; //*.125;
	for(i=0;i<4;i++)
	{
		vt[i].u = cos(((float)i)*PI*.5 + f + sin(f*1.1)*.55)*(sin(f*1.2)*.4+1.2)+0.4;
		vt[i].v = sin(((float)i)*PI*.5 + f + sin(f*0.9)*.64)*(sin(f*1.4)*.3+1.2)+0.3;
	}

	//tt_brick1,tt_bumps,tt_cblob,tt_cloud,tt_diamond,tt_f1,tt_f2,tt_f3,tt_ken,tt_metal1,tt_styro,tt_wood

	drawpoly(&tt_ken,vt,4,0x808080,1.0,0,rendflags);
#endif

#if 1
		//Rotating cylinder
	fcossin(tim*1.0,&fx,&fy); f = 16; fx *= f; fy *= f;

	vt[0].x = -80-fx; vt[0].y = +64-fy; vt[0].z =-32; vt[0].u = 0.01; vt[0].v = 0.01; vt[0].n = 1;
	vt[1].x = -80+fx; vt[1].y = +64+fy; vt[1].z =-32; vt[1].u = 0.99; vt[1].v = 0.01; vt[1].n = 2;
	vt[2].x = -80+fx; vt[2].y = +64+fy; vt[2].z =  0; vt[2].u = 0.99; vt[2].v = 0.99; vt[2].n = 3;
	vt[3].x = -80-fx; vt[3].y = +64-fy; vt[3].z =  0; vt[3].u = 0.01; vt[3].v = 0.99; vt[3].n = 0;
	drawpoly(&voxcyl,vt,4,0x808080,.5,0,rendflags);

	vt[0].x = -80+fx; vt[0].y = +64+fy; vt[0].z =-32; vt[0].u = 0.99; vt[0].v = 0.01; vt[0].n = 1;
	vt[1].x = -80-fx; vt[1].y = +64-fy; vt[1].z =-32; vt[1].u = 0.01; vt[1].v = 0.01; vt[1].n = 2;
	vt[2].x = -80-fx; vt[2].y = +64-fy; vt[2].z =  0; vt[2].u = 0.01; vt[2].v = 0.99; vt[2].n = 3;
	vt[3].x = -80+fx; vt[3].y = +64+fy; vt[3].z =  0; vt[3].u = 0.99; vt[3].v = 0.99; vt[3].n = 0;
	drawpoly(&voxcyl,vt,4,0x808080,.5,0,rendflags);
#endif

#if 1
		//Test joint
	vt[0].x = -80; vt[0].y = -80; vt[0].z =-32; vt[0].u =.26; vt[0].v =.24; vt[0].n = 1;
	vt[1].x = -80; vt[1].y =-112; vt[1].z =-32; vt[1].u =.74; vt[1].v =.24; vt[1].n = 2;
	vt[2].x = -80; vt[2].y =-112; vt[2].z =+ 0; vt[2].u =.74; vt[2].v =.76; vt[2].n = 3;
	vt[3].x = -80; vt[3].y = -80; vt[3].z =+ 0; vt[3].u =.26; vt[3].v =.76; vt[3].n = 0;
	drawpoly(&vox,vt,4,0x808080,0.5,0,rendflags);

	vt[0].x =-112; vt[0].y = -80; vt[0].z =-32; vt[0].u =.26; vt[0].v =.24; vt[0].n = 1;
	vt[1].x = -80; vt[1].y = -80; vt[1].z =-32; vt[1].u =.74; vt[1].v =.24; vt[1].n = 2;
	vt[2].x = -80; vt[2].y = -80; vt[2].z =+ 0; vt[2].u =.74; vt[2].v =.76; vt[2].n = 3;
	vt[3].x =-112; vt[3].y = -80; vt[3].z =+ 0; vt[3].u =.26; vt[3].v =.76; vt[3].n = 0;
	drawpoly(&vox,vt,4,0x808080,0.5,0,rendflags);

	vt[0].x = -96; vt[0].y = -80; vt[0].z =-32; vt[0].u = 0; vt[0].v = 0; vt[0].n = 1;
	vt[1].x = -80; vt[1].y = -96; vt[1].z =-32; vt[1].u = 1; vt[1].v = 0; vt[1].n = 2;
	vt[2].x = -80; vt[2].y = -96; vt[2].z =+ 0; vt[2].u = 1; vt[2].v = 1; vt[2].n = 3;
	vt[3].x = -96; vt[3].y = -80; vt[3].z =+ 0; vt[3].u = 0; vt[3].v = 1; vt[3].n = 0;
	drawpoly(&voxgap,vt,4,0x808080,0.5,0,rendflags);
#endif
#else
		//Walvox map :)

		//Front
	vt[0].x = -64; vt[0].y = -64; vt[0].z =-64; vt[0].u = 0; vt[0].v = 0; vt[0].n = 1;
	vt[1].x = +64; vt[1].y = -64; vt[1].z =-64; vt[1].u = 1; vt[1].v = 0; vt[1].n = 2;
	vt[2].x = +64; vt[2].y = -64; vt[2].z =+64; vt[2].u = 1; vt[2].v = 1; vt[2].n = 3;
	vt[3].x = -64; vt[3].y = -64; vt[3].z =+64; vt[3].u = 0; vt[3].v = 1; vt[3].n = 0;
	drawpoly(&tt_brick1,vt,4,0x808080,1.0,0,rendflags);

		//Left
	vt[0].x = -64; vt[0].y = +64; vt[0].z =-64; vt[0].u = 0; vt[0].v = 0; vt[0].n = 1;
	vt[1].x = -64; vt[1].y = -64; vt[1].z =-64; vt[1].u = 1; vt[1].v = 0; vt[1].n = 2;
	vt[2].x = -64; vt[2].y = -64; vt[2].z =+64; vt[2].u = 1; vt[2].v = 1; vt[2].n = 3;
	vt[3].x = -64; vt[3].y = +64; vt[3].z =+64; vt[3].u = 0; vt[3].v = 1; vt[3].n = 0;
	drawpoly(&tt_brick1,vt,4,0x808080,1.0,0,rendflags);

		//Right
	vt[0].x = +64; vt[0].y = -64; vt[0].z =-64; vt[0].u = 0; vt[0].v = 0; vt[0].n = 1;
	vt[1].x = +64; vt[1].y = +64; vt[1].z =-64; vt[1].u = 1; vt[1].v = 0; vt[1].n = 2;
	vt[2].x = +64; vt[2].y = +64; vt[2].z =+64; vt[2].u = 1; vt[2].v = 1; vt[2].n = 3;
	vt[3].x = +64; vt[3].y = -64; vt[3].z =+64; vt[3].u = 0; vt[3].v = 1; vt[3].n = 0;
	drawpoly(&tt_f2,vt,4,0x808080,1.0,0,rendflags);

		//Back
	vt[0].x = +64; vt[0].y = +96; vt[0].z =-64; vt[0].u = 0; vt[0].v = 0; vt[0].n = 1;
	vt[1].x = -64; vt[1].y = +96; vt[1].z =-64; vt[1].u = 1; vt[1].v = 0; vt[1].n = 2;
	vt[2].x = -64; vt[2].y = +96; vt[2].z =+64; vt[2].u = 1; vt[2].v = 1; vt[2].n = 3;
	vt[3].x = +64; vt[3].y = +96; vt[3].z =+64; vt[3].u = 0; vt[3].v = 1; vt[3].n = 0;
	drawpoly(&tt_f1,vt,4,0x808080,2.0,0,rendflags);

		//Floor
	vt[0].x = -64; vt[0].y = -64; vt[0].z =+64; vt[0].u = 0; vt[0].v = 0; vt[0].n = 1;
	vt[1].x = +64; vt[1].y = -64; vt[1].z =+64; vt[1].u = 1; vt[1].v = 0; vt[1].n = 2;
	vt[2].x = +64; vt[2].y = +64; vt[2].z =+64; vt[2].u = 1; vt[2].v = 1; vt[2].n = 3;
	vt[3].x = -64; vt[3].y = +64; vt[3].z =+64; vt[3].u = 0; vt[3].v = 1; vt[3].n = 0;
	drawpoly(&tt_f3,vt,4,0x808080,1.0,0,rendflags);

		//Ceil
	vt[0].x = -64; vt[0].y = +64; vt[0].z =-64; vt[0].u = 0; vt[0].v = 0; vt[0].n = 1;
	vt[1].x = +64; vt[1].y = +64; vt[1].z =-64; vt[1].u = 1; vt[1].v = 0; vt[1].n = 2;
	vt[2].x = +64; vt[2].y = -64; vt[2].z =-64; vt[2].u = 1; vt[2].v = 1; vt[2].n = 3;
	vt[3].x = -64; vt[3].y = -64; vt[3].z =-64; vt[3].u = 0; vt[3].v = 1; vt[3].n = 0;
	drawpoly(&tt_metal1,vt,4,0x808080,1.0,0,rendflags);

#endif

		//Testing texture sizes for non-pow2
	vt[0].x = -64; vt[0].y = -96; vt[0].z =-128; vt[0].u =-1; vt[0].v =-1; vt[0].n = 1;
	vt[1].x = +64; vt[1].y = -96; vt[1].z =-128; vt[1].u = 2; vt[1].v =-1; vt[1].n = 2;
	vt[2].x = +64; vt[2].y = -96; vt[2].z =+  0; vt[2].u = 2; vt[2].v = 2; vt[2].n = 3;
	vt[3].x = -64; vt[3].y = -96; vt[3].z =+  0; vt[3].u =-1; vt[3].v = 2; vt[3].n = 0;
	if (tt_testnon2.f) drawpoly(&tt_testnon2,vt,4,0x808080,1.0,0,rendflags);


		//Test complex polygons
	if (!(rendflags&RENDFLAGS_HEIGHT))
	{
		n = 8; f = (PI*2)/(float)n;
		for(i=0;i<n;i++)
		{
			vt[i].x = 48;
			vt[i].y = cos(((float)i+.5+tim)*f)*28-24;
			vt[i].z = sin(((float)i+.5+tim)*f)*28+8;
			vt[i].u = vt[i].y/64.0;
			vt[i].v = vt[i].z/64.0;
			vt[i].n = i+1;
		}
		vt[n-1].n = 0;

		n = 4; f = (PI*2)/(float)n;
		for(i=0;i<n;i++)
		{
			vt[i+8].x = 48;
			vt[i+8].y = cos(((float)i+.5-tim)*f)*8-34;
			vt[i+8].z = sin(((float)i+.5-tim)*f)*8+8;
			vt[i+8].u = vt[n+i].x/64.0;
			vt[i+8].v = vt[n+i].y/64.0;
			vt[i+8].n = i+8-1;

			vt[i+12].x = 48;
			vt[i+12].y = cos(((float)i+.5-tim)*f)*8-14;
			vt[i+12].z = sin(((float)i+.5-tim)*f)*8+8;
			vt[i+12].u = vt[n+i].x/64.0;
			vt[i+12].v = vt[n+i].y/64.0;
			vt[i+12].n = i+12-1;
		}
		vt[8].n = 12-1; vt[12].n = 16-1;
		n = 16;
		drawpoly(&tt_f3,vt,n,0x808080,1.0,0,rendflags|RENDFLAGS_CULLNONE);
	}

	periodavg += (dt-periodavg)*.05;
	print6x8(0,cam.c.y-24,0xffffff,-1,"FOV=%.1f%c",atan(cam.h.x/cam.h.z)*360.0/PI,248);
	print6x8(0,cam.c.y-16,0xffffff,-1,"anginc=%g",drawpoly_anginc);
	print6x8(0,cam.c.y-8,0xffffff,-1,"%.1f FPS",1.0/periodavg);
	//print6x8(cam.c.x/2,cam.c.y-8,0xffffff,-1,"%f,%f,%f",cam.p.x,cam.p.y,cam.p.z);

	stopdirectdraw();
	nextpage();
	numframes++;
skipdd:;
}
#endif

#if 0
!endif
#endif
