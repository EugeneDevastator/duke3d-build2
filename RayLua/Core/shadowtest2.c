#include "monoclip.h"
#include "shadowtest2.h"

#include <stdbool.h>

#include "scenerender.h"
#if 0
shadowtest2.exe: shadowtest2.obj winmain.obj build2.obj drawpoly.obj drawcone.obj drawkv6.obj kplib.obj;
				link shadowtest2.obj winmain.obj build2.obj drawpoly.obj drawcone.obj drawkv6.obj kplib.obj\
	ddraw.lib dinput.lib dxguid.lib ole32.lib user32.lib gdi32.lib kernel32.lib /opt:nowin98
	del shadowtest2.obj

#zbufmode=/DUSEINTZ
zbufmode=

shadowtest2.obj: shadowtest2.c; cl /c /TP shadowtest2.c /Ox /G6Fy /MD /QIfist $(zbufmode) /DSTANDALONE
build2.obj:      build2.c;      cl /c /TP build2.c      /Ox /G6Fy /MD         $(zbufmode)
drawpoly.obj:    drawpoly.c;    cl /c /TP drawpoly.c    /Ox /G6Fy /MD /QIfist $(zbufmode)
drawcone.obj:    drawcone.c;    cl /c /TP drawcone.c    /Ox /G6Fy /MD /QIfist $(zbufmode)
drawkv6.obj:     drawkv6.c;     cl /c /TP drawkv6.c     /Ox /G6Fy /MD /QIfist $(zbufmode) /DUSEKZ
kplib.obj:       kplib.c;       cl /c /TP kplib.c       /Ox /G6Fy /MD
winmain.obj:     winmain.cpp;   cl /c /TP winmain.cpp   /Ox /G6Fy /MD
!if 0
#endif

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define PI 3.14159265358979323
#pragma warning(disable:4731)

#define USESSE2 0
#define USENEWLIGHT 1 //FIXFIXFIX
#define USEGAMMAHACK 1 //FIXFIXFIX
int renderinterp = 1;
int compact2d = 0;
#if 0

typedef struct { char filnam[232]; float u0, v0, ux, uy, vx, vy; } sky_t;

skytest.sky:
-----------------------------------------
"skytest.png" 0    0  256 0  0 256 //front
"skytest.png" 0  256  256 0  0 256 //right
"skytest.png" 0  512  256 0  0 256 //back
"skytest.png" 0  768  256 0  0 256 //left
"skytest.png" 0 1024  256 0  0 256 //up
"skytest.png" 0 1280  256 0  0 256 //down

#endif

#if 0
drawpol (..)
{
	...
	for each visible light:
	{
		Prepare light ramping (12 floats/poly)
		Project&write raster list (xi,x0,y0,yl,dir)
	}
	for each y:
	{
		//generate active list rasters from shadows touching this y
		//sort span lefts
		//sort span rights
		//precalc light math for y
		while (x not done)
		{
			xe = whole_raster_right
			for active lights:
			{
				find next left  >= x on light's spans
				find next right >= x on light's spans
				xe = min(xe,left,right);
			}
			for active lights { sum RGB }
			draw x..xe,y
		}
	}
}

drawscreen (..)
{
	render current view to lists, including texture info

		//Generate shadow polygon list
	for each light in map:
		for each visible face in both light and current view, store:
			xyz loop, norm, light_index

	render current view from list
}
#endif

//--------------------------------------------------------------------------------------------------
static tiletype gdd;
int shadowtest2_rendmode = 1;
extern int drawpoly_numcpu;
int shadowtest2_updatelighting = 1;
#ifdef STANDALONE
	//For debug only!
static int gcnt, curgcnt = 0x7fffffff, fixposx, fixposy;
#endif

	//Sorting

unsigned int *shadowtest2_sectgot = 0; //WARNING:code uses x86-32 bit shift trick!
static unsigned int *shadowtest2_sectgotmal = 0;
static int shadowtest2_sectgotn = 0;
#define CLIP_PORTAL_FLAG 8
#define MAX_PORTAL_DEPTH 4
//Translation & rotation
static mapstate_t *curMap;
static player_transform *gps;
//static point3d b->gnadd;
//static double b->xformmat[9], b->xformmatc, b->xformmats;

	//Texture mapping parameters
static tile_t *gtpic;

static int gcurcol;
static int taginc= 30000;
#define LIGHTMAX 256 //FIX:make dynamic!
lightpos_t shadowtest2_light[LIGHTMAX];
static lightpos_t *glp;
int shadowtest2_numlights = 0, shadowtest2_useshadows = 1, shadowtest2_numcpu = 0;
float shadowtest2_ambrgb[3] = {32.0,32.0,32.0};
__declspec(align(16)) static float g_qamb[4]; //holder for SSE to avoid degenerates
static point3d slightpos[LIGHTMAX], slightdir[LIGHTMAX];
static float spotwid[LIGHTMAX];

//--------------------------------------------------------------------------------------------------
extern void htrun (void (*dacallfunc)(int), int v0, int v1, int danumcpu);
extern double distpoint2line2 (double x, double y, double x0, double y0, double x1, double y1);

	//KPLIB.H (excerpt):
extern int kzaddstack (const char *);

	//DRAWKV6.H:
typedef struct
{
	float hx[8], hy[8], hz[8], rhzup20[8];
	short wmin[8], wmax[8];
	short ighyxyx[4], igyxyx[4]; //32-bit only!
	intptr_t ddp, ddf, ddx, ddy, zbufoff;
	point3d p, r, d, f;
} drawkv6_frame_t;
drawkv6_frame_t drawkv6_frame;

	//DRAWCONE.H:
#define DRAWCONE_NOCAP0 1
#define DRAWCONE_NOCAP1 2
#define DRAWCONE_FLAT0 4
#define DRAWCONE_FLAT1 8
#define DRAWCONE_CENT 16
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
inline void memset8(void *d, long v, long n) {
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
int argb_interp(int c0, int c1, int mul15) {
	_asm
			{
				punpcklbw mm0, c0
				punpcklbw mm1, c1
				psrlw mm0, 8
				psrlw mm1, 8
				psubw mm1, mm0
				paddw mm1, mm1
				pshufw mm2, mul15, 0
				pmulhw mm1, mm2
				paddw mm1, mm0
				packuswb mm1, mm1
				movd eax, mm1
				emms
				}
}

static int prepbunch (int id, bunchverts_t *twal, bunchgrp *b)
{
	cam_t gcam = b->cam;
	wall_t *wal;
	double f, x, y, x0, y0, x1, y1;
	int i, n;

	wal = curMap->sect[b->bunch[id].sec].wall;
	i = b->bunch[id].wal0; twal[0].i = i;
	x0 = wal[i].x; y0 = wal[i].y; i += wal[i].n;
	x1 = wal[i].x; y1 = wal[i].y; f = b->bunch[id].fra0;
	twal[0].x = (x1-x0)*f + x0;
	twal[0].y = (y1-y0)*f + y0;
	if ((b->bunch[id].wal0 == b->bunch[id].wal1) && (b->bunch[id].fra0 < b->bunch[id].fra1))
	{     //Hack for left side clip
		f = b->bunch[id].fra1;
		twal[1].x = (x1-x0)*f + x0;
		twal[1].y = (y1-y0)*f + y0;
		return(1);
	}
	twal[1].x = x1;
	twal[1].y = y1; n = 1;
	while (i != b->bunch[id].wal1)
	{
		twal[n].i = i; n++; i += wal[i].n;
		twal[n].x = wal[i].x;
		twal[n].y = wal[i].y;
	}
	if (b->bunch[id].fra1 > 0.0)
	{
		x = wal[i].x; y = wal[i].y; f = b->bunch[id].fra1;
		twal[n].i = i; n++; i += wal[i].n;
		twal[n].x = (wal[i].x-x)*f + x;
		twal[n].y = (wal[i].y-y)*f + y;
	}
	return(n);
}

	//bunchfront intersection structure:
	//   bun: bunch index (always b1)
	//   sid: which way bunch intersects 1:/, 2:backslash
	//   wal: wall index on b0's sector {0..sec[s1].n-1}
	//   fra: intersection point ratio (wal to wal_next) {0.0..1.0}



	//See BUNCHFRONT2.KC for derivation.
	//Returns:
	//   0: NO OVERLAP
	//   1: FRONT:RED(b0)
	//   2: FRONT:GREEN(b1)
	//   3: UNSORTABLE!
static int bunchfront (int b0, int b1, int fixsplitnow, bunchgrp *b)
{
	cam_t gcam = b->cam;
	bunchverts_t *twal[2];
	wall_t *wal;
	double d, a[2], x0, y0, x1, y1, x2, y2, x3, y3, t0, t1, t2, t3;
	double x, y, ix, iy, tix, tiy, x10, y10, x23, y23, x20, y20, otx0, oty0, otx1, oty1, tx0, ty0, tx1, ty1, u, t, d0, d1;
	int i, j, twaln[2], oj, ind[2], sid, cnt, gotsid, startsid, obfintn;

	if (b0 == b1) return(0);

		//FIXFIXFIXFIXFIX:remove block
#if 0 //This doen't work if 'B' of {A<B<C} is hidden from view :/
	sect_t *sec;
	int s0, s1, w, ns, nw;
	sec = curMap->sect;
	s0 = bunch[b0].sec;
	s1 = bunch[b1].sec;
	if (s0 != s1)
	{
		for(w=sec[s0].n-1;w>=0;w--)
		{
			ns = sec[s0].wall[w].ns;
			nw = sec[s0].wall[w].nw;
			while (((unsigned)ns < (unsigned)curMap->numsects) && (ns != s0))
			{
				if (ns == s1) goto good; //s0 and s1 are neighbors
				i = ns;
				ns = sec[i].wall[nw].ns;
				nw = sec[i].wall[nw].nw;
			}
		}
		return(0); //bunches not on neighboring sectors are designated as incomparable
good:;
	}
#endif

		//FIXFIXFIXFIXFIX
	//if ((((unsigned)b0 >= (unsigned)bunchn) || ((unsigned)b1 >= (unsigned)bunchn)) || (!bunch))
	//{
	//   char tbuf[1024];
	//   sprintf(tbuf,"b0=%d/0x%08x\nb1=%d/0x%08x\nbunchn=%d/0x%08x",b0,b0,b1,b1,bunchn,bunchn);
	//   MessageBox(ghwnd,tbuf,prognam,MB_OK);
	//   ExitProcess(0);
	//}

	twal[0] = (bunchverts_t *)_alloca((curMap->sect[b->bunch[b0].sec].n+curMap->sect[b->bunch[b1].sec].n+2)*sizeof(bunchverts_t));
	twaln[0] = prepbunch(b0,twal[0],b); twal[1] = &twal[0][twaln[0]+1];
	twaln[1] = prepbunch(b1,twal[1],b);

		//Offset vertices (BUNCHNEAR of scansector() already puts them safely in front)
	for(j=2-1;j>=0;j--) for(i=twaln[j];i>=0;i--) { twal[j][i].x -= gcam.p.x; twal[j][i].y -= gcam.p.y; }

	//{ //FIXFIXFIXFIXFIX
	//char tbuf[1024]; sprintf(tbuf,"cnt=%d\n",(1<<31)-1-gcnt);
	//for(j=0;j<2;j++)
	//{
	//   for(i=0;i<=twaln[0];i++) sprintf(&tbuf[strlen(tbuf)],"%f,%f  ",twal[j][i].x,twal[j][i].y);
	//   sprintf(&tbuf[strlen(tbuf)],"\n");
	//}
	//MessageBox(ghwnd,tbuf,prognam,MB_OK);

	if (twal[0][0].y*twal[1][twaln[1]].x >= twal[1][twaln[1]].y*twal[0][0].x) return(0); //no overlap (whole bunch)
	if (twal[1][0].y*twal[0][twaln[0]].x >= twal[0][twaln[0]].y*twal[1][0].x) return(0); //no overlap (whole bunch)


	a[0] = 0; a[1] = 0;
#if 0 //FIXFIXFIXFIXFIX:old (doesn't test unsortable)
		//Calculate difference of area in overlapping section
	for(j=2-1;j>=0;j--)
	{
		x2 = twal[1-j][0].x; x3 = twal[1-j][twaln[1-j]].x;
		y2 = twal[1-j][0].y; y3 = twal[1-j][twaln[1-j]].y; i = twaln[j];
		for(i--;i>=0;i--)
		{
			x0 = twal[j][i].x; x1 = twal[j][i+1].x;
			y0 = twal[j][i].y; y1 = twal[j][i+1].y;

				//(x0-x1)*t + x2*u = x0
				//(y0-y1)*t + y2*u = y0
			t0 = x1*y2 - x2*y1; if (t0 >= 0.0) continue; //no overlap
			t1 = x0*y3 - x3*y0; if (t1 <= 0.0) continue; //no overlap
			t2 = x0*y2 - x2*y0; if (t2 <= 0.0) t0 = 0.0; else t0 = t2/(t2-t0);
			t3 = x1*y3 - x3*y1; if (t3 >= 0.0) t1 = 1.0; else t1 = t1/(t1-t3);
			a[j] += (x0*y1 - x1*y0)*(t1-t0); //add area(*2) of triangular slice, clipped by other bunch
		}
	}
	if (a[0] < a[1]) return(1);
	if (a[0] > a[1]) return(2);
	return(0);
#else
		//Calculate the areas between the 2 bunches (superset of above algo - can determine if unsortable)
	ind[0] = 0; ind[1] = 0; cnt = 0; otx0 = 0; oty0 = 0; otx1 = 0; oty1 = 0;
	j = 0; gotsid = 0; startsid = -1; obfintn = b->bfintn;
	while (1)
	{
		sid = (twal[0][ind[0]].x*twal[1][ind[1]].y < twal[0][ind[0]].y*twal[1][ind[1]].x);
		if (ind[1-sid] > 0)
		{
			x2 = twal[sid][ind[sid]].x; x0 = twal[1-sid][ind[1-sid]-1].x; x1 = twal[1-sid][ind[1-sid]-0].x;
			y2 = twal[sid][ind[sid]].y; y0 = twal[1-sid][ind[1-sid]-1].y; y1 = twal[1-sid][ind[1-sid]-0].y;

				//intersect() inline
				//(x1-x0)*t + (x2-x3)*u = (x2-x0)
				//(y1-y0)*t + (y2-y3)*u = (y2-y0)
			x10 = x1-x0; x20 = x2-x0;
			y10 = y1-y0; y20 = y2-y0;
			d =  x10*y2 - y10*x2; if (d == 0.0) goto overflow/*FIX?*/;
			u = (x10*y20 - y10*x20)/d; ix = (1.0-u)*x2; iy = (1.0-u)*y2;

			if (!sid) { tx0 = ix; ty0 = iy; tx1 = x2; ty1 = y2; }
				  else { tx0 = x2; ty0 = y2; tx1 = ix; ty1 = iy; }
			oj = j; if (u != 0.0) { j = ((u >= 0.0) == sid); if (!gotsid) { gotsid = 1; oj = j; startsid = j; } }
			if (cnt)
			{
				if (j == oj)
				{
						//---ot0-ot1
						// \  |  |
						//   t0  |
						//     \t1
					d = oty0*tx0 - otx0*ty0 + otx1*ty1 - oty1*tx1; a[j] += d;
				}
				else
				{
						//NOTE:must use original wall vertices to get correct value of t!
					wal = curMap->sect[b->bunch[b0].sec].wall; i = twal[0][ind[0]-1].i;
					x0 = wal[i].x-gcam.p.x; y0 = wal[i].y-gcam.p.y; i += wal[i].n;
					x1 = wal[i].x-gcam.p.x; y1 = wal[i].y-gcam.p.y;
					wal = curMap->sect[b->bunch[b1].sec].wall; i = twal[1][ind[1]-1].i;
					x2 = wal[i].x-gcam.p.x; y2 = wal[i].y-gcam.p.y; i += wal[i].n;
					x3 = wal[i].x-gcam.p.x; y3 = wal[i].y-gcam.p.y;

						//intersect() inline
						//(x1-x0)*t + (x2-x3)*u = (x2-x0)
						//(y1-y0)*t + (y2-y3)*u = (y2-y0)
					x10 = x1-x0; x23 = x2-x3; x20 = x2-x0;
					y10 = y1-y0; y23 = y2-y3; y20 = y2-y0;
					d =  x10*y23 - y10*x23; if (d == 0.0) goto overflow/*FIX?*/; d = 1.0/d;
					t = (x20*y23 - y20*x23)*d; tix = x10*t + x0; tiy = y10*t + y0;

					d0 = (oty0-oty1)*tix + (otx1-otx0)*tiy; a[oj] += d0;
					d1 = ( ty1- ty0)*tix + ( tx0- tx1)*tiy; a[ j] += d1;

					if ((fixsplitnow) && (b->bfintn < BFINTMAX))
					{
						b->bfint[b->bfintn].bun = b1;
						b->bfint[b->bfintn].sid = startsid+1; startsid ^= 1;
						b->bfint[b->bfintn].wal = twal[0][ind[0]-1].i;
						b->bfint[b->bfintn].fra = t;
						b->bfintn++;
					}
				}
			}
overflow:otx0 = tx0; oty0 = ty0; otx1 = tx1; oty1 = ty1;
			cnt++;
		}
		ind[sid]++; if (ind[sid] > twaln[sid]) break;
	}
		//WARNING:1e-7's necessary for precision loss while calculating (ix,iy) - even for horz/vert lines
	if ((a[0] <= +1e-7) && (a[1] >= -1e-7)) return(0);
	if (a[0] <= +1e-7) return(1);
	if (a[1] >= -1e-7) return(2);
	return(3);
#endif
}

static void scansector (int sectnum, bunchgrp* b)
{
	cam_t gcam = b->cam;
	#define BUNCHNEAR 1e-7
	sect_t *sec;
	wall_t *wal;
	bfint_t tbf;
	double f, dx0, dy0, dx1, dy1, f0, f1;
	int i, j, k, m, o, ie, obunchn, realobunchn, obfintn;

	if (sectnum < 0) return;
	b->sectgot[sectnum>>5] |= (1<<sectnum);

	sec = &curMap->sect[sectnum]; wal = sec->wall;

	obunchn = b->bunchn; realobunchn = b->bunchn;
	for(i=0,ie=sec->n;i<ie;i++)
	{
		j = wal[i].n+i;
		dx0 = wal[i].x-gcam.p.x; dy0 = wal[i].y-gcam.p.y;
		dx1 = wal[j].x-gcam.p.x; dy1 = wal[j].y-gcam.p.y; if (dy1*dx0 <= dx1*dy0) goto docont; //Back-face cull

			//clip to near plane .. result is parametric fractions f0&f1
		f0 = dx0 * b->xformmatc + dy0 * b->xformmats;
		f1 = dx1 * b->xformmatc + dy1 * b->xformmats;
			  if (f0 <= BUNCHNEAR) { if (f1 <= BUNCHNEAR) goto docont;
											 f0 = (BUNCHNEAR-f0)/(f1-f0); f1 = 1.0; if (f0 >= f1) goto docont; }
		else if (f1 <= BUNCHNEAR) { f1 = (BUNCHNEAR-f0)/(f1-f0); f0 = 0.0; if (f0 >= f1) goto docont; }
		else                      { f0 = 0.0;                    f1 = 1.0; }

		k = b->bunch[b->bunchn-1].wal1;
		if ((b->bunchn > obunchn) && (wal[k].n+k == i) && (b->bunch[b->bunchn-1].fra1 == 1.0))
		{
			b->bunch[b->bunchn-1].wal1 = i; //continue from previous wall (typical case)
			b->bunch[b->bunchn-1].fra1 = f1;
			if ((b->bunchn-1 > obunchn) && (b->bunch[obunchn].wal0 == j) && (b->bunch[obunchn].fra0 == 0.0))
			{
				b->bunchn--; //attach to left side of 1st bunch on loop
				b->bunch[obunchn].wal0 = b->bunch[b->bunchn].wal0;
				b->bunch[obunchn].fra0 = b->bunch[b->bunchn].fra0;
			}
		}
		else if ((b->bunchn > obunchn) && (b->bunch[obunchn].wal0 == j) && (b->bunch[obunchn].fra0 == 0.0))
		{
			b->bunch[obunchn].wal0 = i; //update left side of 1st bunch on loop
			b->bunch[obunchn].fra0 = f0;
		}
		else
		{
			if (b->bunchn >= b->bunchmal)
			{
				b->bunchmal <<= 1;
				b->bunch     = (bunch_t       *)realloc(b->bunch    ,b->bunchmal*sizeof(b->bunch[0]));
				b->bunchgot  = (unsigned int  *)realloc(b->bunchgot ,((b->bunchmal+31)&~31)>>3);
				b->bunchgrid = (unsigned char *)realloc(b->bunchgrid,((b->bunchmal-1)*b->bunchmal)>>1);
			}
			b->bunch[b->bunchn].wal0 = i; b->bunch[b->bunchn].fra0 = f0; //start new b->bunch
			b->bunch[b->bunchn].wal1 = i; b->bunch[b->bunchn].fra1 = f1;
			b->bunch[b->bunchn].sec = sectnum; b->bunchn++;
		}
docont:;
		if (j < i) obunchn = b->bunchn;
	}

	for(obunchn=realobunchn;obunchn<b->bunchn;obunchn++)
	{
			//insert bunch
			//  0 1 2 3 4
			//0
			//1 x
			//2 x x
			//3 x x x
			//4 x x x x
			//5 ? ? ? ? ?
			//0,1,3,6,10,15,21,28,36,45,55,..
		j = (((obunchn-1)*obunchn)>>1); b->bfintn = 0;
		for(i=0;i<obunchn;i++) b->bunchgrid[j+i] = bunchfront(obunchn,i,1,b);

		if (!b->bfintn) continue;

			//sort bfint's
		for(j=1;j<b->bfintn;j++)
			for(i=0;i<j;i++)
			{
					//              bfint[i].wal vs. bfint[j].wal ?
					//0    bunch[obunchn].wal0........bunch[obunchn].wal1       sec->n
					//0....bunch[obunchn].wal1        bunch[obunchn].wal0.......sec->n
				m = b->bfint[i].wal; o = b->bfint[j].wal;
				if (b->bunch[obunchn].wal0 > b->bunch[obunchn].wal1) //handle wall index wrap-around
				{
					if (m <= b->bunch[obunchn].wal1) m += sec->n;
					if (o <= b->bunch[obunchn].wal1) o += sec->n;
				}
				if (m < o) continue;
				if ((b->bfint[i].wal == b->bfint[j].wal) && (b->bfint[i].fra <= b->bfint[j].fra)) continue;

				tbf = b->bfint[i]; b->bfint[i] = b->bfint[j]; b->bfint[j] = tbf;
			}

			//combine null or tiny bunches
		obfintn = b->bfintn; b->bfintlut[0] = 0; b->bfintn = 1;
		for(i=1;i<obfintn;i++)
			if ((b->bfint[i-1].wal != b->bfint[i].wal) || (b->bfint[i].fra-b->bfint[i-1].fra >= 2e-7))
				{ b->bfintlut[b->bfintn] = i; b->bfintn++; }
		b->bfintlut[b->bfintn] = obfintn;

			//obunchn gets its ass split 'b->bfintn' times into a total of 'b->bfintn+1' pieces
		if (b->bunchn+b->bfintn > b->bunchmal)
		{
			b->bunchmal = max(b->bunchmal<<1,b->bunchn+b->bfintn);
			b->bunch     = (bunch_t       *)realloc(b->bunch    ,b->bunchmal*sizeof(b->bunch[0]));
			b->bunchgot  = (unsigned int  *)realloc(b->bunchgot ,((b->bunchmal+31)&~31)>>3);
			b->bunchgrid = (unsigned char *)realloc(b->bunchgrid,((b->bunchmal-1)*b->bunchmal)>>1);
		}

			//Shove not-yet-processed neighbors to end of list. WARNING:be careful with indices/for loop order!
		for(k=0;k<b->bfintn;k++) b->bunch[b->bunchn+b->bfintn-1-k] = b->bunch[obunchn+k+1];
		for(k=b->bfintn-1;k>=0;k--) b->bunch[obunchn+k+1] = b->bunch[obunchn];
		for(k=b->bfintn-1;k>=0;k--)
		{
			b->bunch[obunchn+k  ].wal1 = b->bfint[b->bfintlut[k  ]  ].wal; b->bunch[obunchn+k  ].fra1 = max(b->bfint[b->bfintlut[k  ]  ].fra-1e-7,0.0);
			b->bunch[obunchn+k+1].wal0 = b->bfint[b->bfintlut[k+1]-1].wal; b->bunch[obunchn+k+1].fra0 = min(b->bfint[b->bfintlut[k+1]-1].fra+1e-7,1.0);
		}
		b->bunchn += b->bfintn;

			//  0 1 2 3 4 5 6
			//0
			//1 x
			//2 x x
			//3 x x x
			//4 x x x x
			//5 ? ? ? - ?
			//6 ? ? ? - ? 0
			//7 ? ? ? - ? 0 0
		for(m=obunchn;m<obunchn+b->bfintn+1;m++) //re-front all 'b->bfintn+1' pieces, using hints from bfint list
		{
			j = (((m-1)*m)>>1);
			for(k=0;k<obunchn;k++)
			{
				if (m > obunchn       ) for(o=b->bfintlut[m-obunchn-1];o<b->bfintlut[m-obunchn  ];o++) if (b->bfint[o].bun == k) { b->bunchgrid[j+k] = b->bfint[o].sid  ; goto bunchgrid_got; }
				if (m < obunchn+b->bfintn) for(o=b->bfintlut[m-obunchn  ];o<b->bfintlut[m-obunchn+1];o++) if (b->bfint[o].bun == k) { b->bunchgrid[j+k] = b->bfint[o].sid^3; goto bunchgrid_got; }
				b->bunchgrid[j+k] = bunchfront(m,k,0,b);
bunchgrid_got:;
			}
			for(;k<m;k++) b->bunchgrid[j+k] = 0;
		}
		obunchn += b->bfintn;
	}

		//remove null bunches (necessary for proper operation)
	for(m=b->bunchn-1;m>=realobunchn;m--)
	{
		if (b->bunch[m].wal0 != b->bunch[m].wal1) continue;
		if (b->bunch[m].fra0 < b->bunch[m].fra1) continue;
		b->bunchn--; b->bunch[m] = b->bunch[b->bunchn];
		j = (((b->bunchn-1)*b->bunchn)>>1);
		memcpy(&b->bunchgrid[((m-1)*m)>>1],&b->bunchgrid[j],m*sizeof(b->bunchgrid[0]));
		for(i=m+1;i<b->bunchn;i++) b->bunchgrid[(((i-1)*i)>>1)+m] = ((b->bunchgrid[j+i]&1)<<1) + (b->bunchgrid[j+i]>>1);
	}

}

static void xformprep (double hang, bunchgrp *b)
{
	cam_t gcam = b->cam;
	double f; f = atan2(gcam.f.y,gcam.f.x)+hang; //WARNING: "f = 1/sqrt; c *= f; s *= f;" form has singularity - don't use :/
	b->xformmatc = cos(f); b->xformmats = sin(f);
	b->xformmat[0] = gcam.r.y*b->xformmatc - gcam.r.x*b->xformmats; b->xformmat[1] = gcam.r.z; b->xformmat[2] = gcam.r.x*b->xformmatc + gcam.r.y*b->xformmats;
	b->xformmat[3] = gcam.d.y*b->xformmatc - gcam.d.x*b->xformmats; b->xformmat[4] = gcam.d.z; b->xformmat[5] = gcam.d.x*b->xformmatc + gcam.d.y*b->xformmats;
	b->xformmat[6] = gcam.f.y*b->xformmatc - gcam.f.x*b->xformmats; b->xformmat[7] = gcam.f.z; b->xformmat[8] = gcam.f.x*b->xformmatc + gcam.f.y*b->xformmats;
	b->gnadd.x = -gcam.h.x*b->xformmat[0] - gcam.h.y*b->xformmat[1] + gcam.h.z*b->xformmat[2];
	b->gnadd.y = -gcam.h.x*b->xformmat[3] - gcam.h.y*b->xformmat[4] + gcam.h.z*b->xformmat[5];
	b->gnadd.z = -gcam.h.x*b->xformmat[6] - gcam.h.y*b->xformmat[7] + gcam.h.z*b->xformmat[8];
}

static void xformbac (double rx, double ry, double rz, dpoint3d *o, bunchgrp *b)
{
	o->x = rx*b->xformmat[0] + ry*b->xformmat[3] + rz*b->xformmat[6];
	o->y = rx*b->xformmat[1] + ry*b->xformmat[4] + rz*b->xformmat[7];
	o->z = rx*b->xformmat[2] + ry*b->xformmat[5] + rz*b->xformmat[8];
}

#ifdef STANDALONE
	//Assumes poly is clipped by mono_bool() to visible screen area
static void drawpol_aftclip (int plothead0, int plothead1) //this function for debug only!
{
	kgln_t *overt, *vert;
	tiltyp *pic, gtt;
	double f;
	int i, j, on, n, plothead[2];

	if ((plothead0|plothead1) < 0) return;
	plothead[0] = plothead0; plothead[1] = plothead1;

	n = 2; for(j=0;j<2;j++) for(i=mp[plothead[j]].n;i!=plothead[j];i=mp[i].n) n++;
	if (n < 3) return;

	overt = (kgln_t *)_alloca(n*sizeof(overt[0]));
	vert = (kgln_t *)_alloca(n*2*sizeof(vert[0]));

	on = 0;
	for(j=0;j<2;j++)
	{
		i = plothead[j];
		do
		{
			if (j) i = mp[i].p;

			overt[on].x = mp[i].x*b->xformmat[0] + mp[i].y*b->xformmat[1] + b->gnadd.x;
			overt[on].y = mp[i].x*b->xformmat[3] + mp[i].y*b->xformmat[4] + b->gnadd.y;
			overt[on].z = mp[i].x*b->xformmat[6] + mp[i].y*b->xformmat[7] + b->gnadd.z;
			on++;

			if (!j) i = mp[i].n;
		} while (i != plothead[j]);
	}

		//clip
	n = 0;
	for(i=on-1,j=0;j<on;i=j,j++)
	{
		#define ASCISDIST 0.001
		if (overt[i].z >= ASCISDIST) { vert[n] = overt[i]; n++; }
		if ((overt[i].z >= ASCISDIST) != (overt[j].z >= ASCISDIST))
		{
			f = (ASCISDIST-overt[j].z)/(overt[i].z-overt[j].z);
			vert[n].x = (overt[i].x-overt[j].x)*f + overt[j].x;
			vert[n].y = (overt[i].y-overt[j].y)*f + overt[j].y;
			vert[n].z = ASCISDIST; n++;
		}
	}
	if (n < 3) return;

		//project & find x extents
	for(i=0;i<n;i++)
	{
		f = gcam.h.z/vert[i].z;
		vert[i].x = vert[i].x*f + gcam.h.x;
		vert[i].y = vert[i].y*f + gcam.h.y;
		vert[i].n = i+1;
	}
	vert[n-1].n = 0;

	pic = &gtpic->tt;
	gtt.f = pic->f; gtt.p = pic->p; gtt.x = pic->x; gtt.y = pic->y; gtt.z = 1.0; gtt.shsc = 2.0; gtt.lowermip = pic->lowermip;

	i = RENDFLAGS_OUVMAT|RENDFLAGS_NODEPTHTEST|RENDFLAGS_NOTRCP|RENDFLAGS_GMAT;
	if (renderinterp) i |= RENDFLAGS_INTERP;
	drawpoly(&gtt,(vertyp *)vert,n,gcurcol,(((unsigned)gcurcol)>>24)/16.0,b->gouvmat,i);

	if (shadowtest2_rendmode != 2)
	{
		double x0, y0, x1, y1, fx, fy, f;

		for(i=n-1,j=0;i>=0;j=i,i--) drawline2d(&gcam.c,vert[i].x,vert[i].y,vert[j].x,vert[j].y,0x808080);

			//Find centroid of polygon (copied from Build2, which is from TAGPNT2.BAS 09/14/2006)
		fx = 0.0; fy = 0.0; f = 0.0;
		for(i=n-1,j=0;i>=0;j=i,i--)
		{
			x0 = vert[i].x; y0 = vert[i].y;
			x1 = vert[j].x; y1 = vert[j].y;
			fx += ((x0+x1)*x0 + x1*x1)*(y1-y0);
			fy += ((y0+y1)*y0 + y1*y1)*(x0-x1);
			f += (x0+x1)*(y1-y0);
		}
		f = 1.0/(f*3.0); i = (int)(fx*f); j = (int)(fy*f);
													  drawpix(&gcam.c,i+0,j-1,0xffffff); drawpix(&gcam.c,i+1,j-1,0xffffff);
		drawpix(&gcam.c,i-1,j+0,0xffffff); drawpix(&gcam.c,i+0,j+0,0x000000); drawpix(&gcam.c,i+1,j+0,0x000000); drawpix(&gcam.c,i+2,j+0,0xffffff);
		drawpix(&gcam.c,i-1,j+1,0xffffff); drawpix(&gcam.c,i+0,j+1,0x000000); drawpix(&gcam.c,i+1,j+1,0x000000); drawpix(&gcam.c,i+2,j+1,0xffffff);
													  drawpix(&gcam.c,i+0,j+2,0xffffff); drawpix(&gcam.c,i+1,j+2,0xffffff);
	}
}
#endif

eyepol_t *eyepol = 0; // 4096 eyepol_t's = 192KB
point3d *eyepolv = 0; //16384 point2d's  = 128KB
int eyepoln = 0, glignum = 0;
int eyepolmal = 0, eyepolvn = 0, eyepolvmal = 0;

void eyepol_drawfunc (int ind)
{
	kgln_t *vert;
	point2d *lipv;
	float f, fx, fy;
	int i, j, n;

	n = eyepol[ind+1].vert0-eyepol[ind].vert0; lipv = &eyepolv[eyepol[ind].vert0];
	vert = (kgln_t *)_alloca(n*sizeof(vert[0]));
	for(i=0;i<n;i++)
	{
		vert[i].x = lipv[i].x;
		vert[i].y = lipv[i].y;
		vert[i].n = i+1;
	}
	vert[n-1].n = 0;

	//i = RENDFLAGS_OUVMAT|RENDFLAGS_NODEPTHTEST|RENDFLAGS_NOTRCP|RENDFLAGS_GMAT;
	//if (renderinterp) i |= RENDFLAGS_INTERP;
	//drawpoly_flat_threadsafe(&eyepol[ind].tpic->tt,(vertyp *)vert,n,eyepol[ind].curcol,(((unsigned)eyepol[ind].curcol)>>24)/16.0,eyepol[ind].ouvmat,i,gcam);

	if (shadowtest2_rendmode == 1)
	{
	//	for(i=n-1,j=0;i>=0;j=i,i--) drawline2d(&gcam.c,vert[i].x,vert[i].y,vert[j].x,vert[j].y,0xa0a0a0); //WARNING:fusing this with centroid algo below fails.. compiler bug?
#if 0
		fx = 0.f; fy = 0.f;
		for(i=n-1,j=0;i>=0;j=i,i--) { fx += vert[i].x; fy += vert[i].y; }
		f = 1.f/(float)n; i = (int)(fx*f); j = (int)(fy*f);
#else
			//Find centroid of polygon
		fx = 0.f; fy = 0.f; f = 0.f;
		for(i=n-1,j=0;i>=0;j=i,i--)
		{
			float fx0, fy0, fx1, fy1;
			fx0 = vert[i].x; fy0 = vert[i].y;
			fx1 = vert[j].x; fy1 = vert[j].y;
			fx += ((fx0+fx1)*fx0 + fx1*fx1)*(fy1-fy0);
			fy += ((fy0+fy1)*fy0 + fy1*fy1)*(fx0-fx1);
			f += (fx0+fx1)*(fy1-fy0);
		}
		f = 1.f/(f*3.f); i = (int)(fx*f); j = (int)(fy*f);
#endif
	//												  drawpix(&gcam.c,i+0,j-1,0xffffff); drawpix(&gcam.c,i+1,j-1,0xffffff);
	//	drawpix(&gcam.c,i-1,j+0,0xffffff); drawpix(&gcam.c,i+0,j+0,0x000000); drawpix(&gcam.c,i+1,j+0,0x000000); drawpix(&gcam.c,i+2,j+0,0xffffff);
	//	drawpix(&gcam.c,i-1,j+1,0xffffff); drawpix(&gcam.c,i+0,j+1,0x000000); drawpix(&gcam.c,i+1,j+1,0x000000); drawpix(&gcam.c,i+2,j+1,0xffffff);
	//												  drawpix(&gcam.c,i+0,j+2,0xffffff); drawpix(&gcam.c,i+1,j+2,0xffffff);
	}
}
/*
	Purpose: Renders visible geometry polygons to world space eyepol array
	Converts 3D polygon vertices to world coordinates (no screen projection)
	Handles texture mapping setup (UV coordinates, skybox mapping)
	Stores polygons in eyepol[] array with world space vertices
	Manages different rendering modes (walls, skybox, parallax sky)
 */
static void drawtagfunc_ws(int rethead0, int rethead1, bunchgrp *b)
{
	cam_t gcam = b->orcam;
	float f,fx,fy, g, *fptr;
	int i, j, k, h, rethead[2];

	if ((rethead0|rethead1) < 0) { mono_deloop(rethead1); mono_deloop(rethead0); return; }
	rethead[0] = rethead0; rethead[1] = rethead1;

	// Put on FIFO in world space:
	for(h=0;h<2;h++)
	{
		i = rethead[h];
		do
		{
			if (h)
				i = mp[i].p;

			if (eyepolvn >= eyepolvmal)
			{
				eyepolvmal = max(eyepolvmal<<1,16384);
				eyepolv = (point3d *)realloc(eyepolv,eyepolvmal*sizeof(point3d));
			}
			f = gcam.h.z/(/*mp[i].x*b->xformmat[6]*/ + mp[i].y*b->xformmat[7] + b->gnadd.z);
			fx        =  (mp[i].x*b->xformmat[0] + mp[i].y*b->xformmat[1] + b->gnadd.x)*f + gcam.h.x;
			fy        =  (mp[i].x*b->xformmat[3] + mp[i].y*b->xformmat[4] + b->gnadd.y)*f + gcam.h.y;

#if (USEINTZ)
			f = 1.0/((b->gouvmat[0]*fx + b->gouvmat[3]*fy + b->gouvmat[6])*1048576.0*256.0);
#else
			f = 1.0/((b->gouvmat[0]*fx + b->gouvmat[3]*fy + b->gouvmat[6])*gcam.h.z);
#endif
			eyepolv[eyepolvn].x = ((fx-gcam.h.x)*gcam.r.x + (fy-gcam.h.y)*gcam.d.x + (gcam.h.z)*gcam.f.x)*f + gcam.p.x;
			eyepolv[eyepolvn].y = ((fx-gcam.h.x)*gcam.r.y + (fy-gcam.h.y)*gcam.d.y + (gcam.h.z)*gcam.f.y)*f + gcam.p.y;
			eyepolv[eyepolvn].z = ((fx-gcam.h.x)*gcam.r.z + (fy-gcam.h.y)*gcam.d.z + (gcam.h.z)*gcam.f.z)*f + gcam.p.z;

			eyepolvn++;

			if (!h) i = mp[i].n;
		} while (i != rethead[h]);
		mono_deloop(rethead[h]);
	}

	if (eyepoln+1 >= eyepolmal)
	{
		eyepolmal = max(eyepolmal<<1,4096);
		eyepol = (eyepol_t *)realloc(eyepol,eyepolmal*sizeof(eyepol_t));
		eyepol[0].vert0 = 0;
	}

	if (b->gflags < 2)
		memcpy((void *)eyepol[eyepoln].ouvmat,(void *)b->gouvmat,sizeof(b->gouvmat[0])*9);
	else
	{
		// Skybox texture mapping (same as original)
		f = (((float)64)+1.15f)/((float)64); fptr = eyepol[eyepoln].ouvmat;
		switch(b->gflags)
		{
			case 14: fptr[0] = +f                 ; fptr[3] = f*+2.f; fptr[6] =     0.f; //Front
						fptr[1] = -1.f               ; fptr[4] =    0.f; fptr[7] =     0.f;
						fptr[2] = (f*- 5.f - 1.f)/6.f; fptr[5] =    0.f; fptr[8] = f*+12.f;
						break;
			case 13: fptr[0] = +1.f               ; fptr[3] =    0.f; fptr[6] =     0.f; //Right
						fptr[1] = -f                 ; fptr[4] = f*+2.f; fptr[7] =     0.f;
						fptr[2] = (f*-17.f - 1.f)/6.f; fptr[5] =    0.f; fptr[8] = f*+12.f;
						break;
			case 15: fptr[0] = -f                 ; fptr[3] = f*-2.f; fptr[6] =     0.f; //Back
						fptr[1] = +1.f               ; fptr[4] =    0.f; fptr[7] =     0.f;
						fptr[2] = (f*-29.f - 1.f)/6.f; fptr[5] =    0.f; fptr[8] = f*+12.f;
						break;
			case 12: fptr[0] = -1.f               ; fptr[3] =    0.f; fptr[6] =     0.f; //Left
						fptr[1] = +f                 ; fptr[4] = f*-2.f; fptr[7] =     0.f;
						fptr[2] = (f*-41.f - 1.f)/6.f; fptr[5] =    0.f; fptr[8] = f*+12.f;
						break;
			case 11: fptr[0] = +f                 ; fptr[3] = f*+2.f; fptr[6] =     0.f; //Top
						fptr[1] = (f*-17.f - 1.f)/6.f; fptr[4] =    0.f; fptr[7] = f*-12.f;
						fptr[2] = -1.f               ; fptr[5] =    0.f; fptr[8] =     0.f;
						break;
			case 10: fptr[0] = -f                 ; fptr[3] = f*+2.f; fptr[6] =     0.f; //Bottom
						fptr[1] = (f*+ 5.f + 1.f)/6.f; fptr[4] =    0.f; fptr[7] = f*+12.f;
						fptr[2] = +1.f               ; fptr[5] =    0.f; fptr[8] =     0.f;
						break;
		}
		for(i=9-3;i>=0;i-=3)
		{
			float ox, oy, oz;
			ox = fptr[i+0]*65536.f; oy = fptr[i+1]*65536.f; oz = fptr[i+2]*65536.f;
			fptr[i+0] = ox*gcam.r.x + oy*gcam.r.y + oz*gcam.r.z;
			fptr[i+1] = ox*gcam.d.x + oy*gcam.d.y + oz*gcam.d.z;
			fptr[i+2] = ox*gcam.f.x + oy*gcam.f.y + oz*gcam.f.z;
		}
		gentex_xform(fptr,b);
	}

	eyepol[eyepoln].tpic = gtpic;
	eyepol[eyepoln].curcol = gcurcol;
	eyepol[eyepoln].flags = (b->gflags != 0);
	eyepol[eyepoln].b2sect = b->gligsect;
	eyepol[eyepoln].b2wall = b->gligwall;
	eyepol[eyepoln].b2slab = b->gligslab;
	memcpy((void *)&eyepol[eyepoln].norm,(void *)&b->gnorm,sizeof(b->gnorm));
	eyepoln++;
	eyepol[eyepoln].vert0 = eyepolvn;
	eyepol[eyepoln].rdepth = b->recursion_depth;
}

/*
	Purpose: Renders skybox faces as background
	Generates 6 cube faces for skybox rendering
	Clips each face against view frustum
	Projects cube vertices to screen space
	Calls drawtagfunc for each visible skybox face
	Uses special texture mapping flags (b->gflags = 10-15 for different cube faces)
*/

static void skytagfunc (int rethead0, int rethead1, bunchgrp* b)
{
	cam_t gcam = b->cam;
	#define SSCISDIST 0.000001 //Reduces probability of glitch further
	dpoint3d otp[4], tp[8];
	double f, ox, oy, oz;
	int i, j, n, p, plothead[2];
	static const signed char cubeverts[6][4][3] =
	{
		-1,-1,+1, +1,-1,+1, +1,+1,+1, -1,+1,+1, //Up
		-1,+1,-1, +1,+1,-1, +1,-1,-1, -1,-1,-1, //Down
		-1,-1,-1, -1,-1,+1, -1,+1,+1, -1,+1,-1, //Left
		+1,+1,-1, +1,+1,+1, +1,-1,+1, +1,-1,-1, //Right
		-1,-1,+1, -1,-1,-1, +1,-1,-1, +1,-1,+1, //Front
		+1,+1,+1, +1,+1,-1, -1,+1,-1, -1,+1,+1, //Back
	};

	if ((rethead0|rethead1) < 0) { mono_deloop(rethead1); mono_deloop(rethead0); return; }

	for(p=0;p<6;p++)
	{
			//rotate
		for(i=0;i<4;i++)
		{
			ox = (float)cubeverts[p][i][0]; oy = (float)cubeverts[p][i][1];
			otp[i].x = oy*b->xformmatc - ox*b->xformmats; otp[i].y = cubeverts[p][i][2];
			otp[i].z = ox*b->xformmatc + oy*b->xformmats;
		}

			//clip
		n = 0;
		for(i=4-1,j=0;j<4;i=j,j++)
		{
			//Near-Plane Clipping (Sutherland-Hodgman)
			if (otp[i].z >= SSCISDIST) { tp[n] = otp[i]; n++; }
			if ((otp[i].z >= SSCISDIST) != (otp[j].z >= SSCISDIST))
			{
				f = (SSCISDIST-otp[j].z)/(otp[i].z-otp[j].z);
				tp[n].x = (otp[i].x-otp[j].x)*f + otp[j].x;
				tp[n].y = (otp[i].y-otp[j].y)*f + otp[j].y;
				tp[n].z = SSCISDIST; n++;
			}
		}
		if (n < 3) goto skiprest;

			//project & find x extents, persp proj.
			//Formula: screen_x = (world_x * focal_length) / depth + center_x
		for(i=0;i<n;i++)
		{
			f = gcam.h.z/tp[i].z;
			tp[i].x = tp[i].x*f + gcam.h.x;
			tp[i].y = tp[i].y*f + gcam.h.y;
		}

			//generate vmono
		mono_genfromloop(&plothead[0],&plothead[1],tp,n);

		b->gflags = p+10;
		mono_bool(rethead0,rethead1,plothead[0],plothead[1],MONO_BOOL_AND,b, drawtagfunc_ws);
		mono_deloop(plothead[1]); mono_deloop(plothead[0]);
skiprest:;
	}
	mono_deloop(rethead1); mono_deloop(rethead0);
}

	// lignum: index of light source (shadowtest2_light[] index)
	//  vert0: index into 1st vertex of ligpolv
	// b2sect: Build2 sector
	// b2wall: Build2 wall (-2=ceil,-1=flor,&0x40000000=spri,else wall)
	// b2slab: Build2 slab number for wall
	//b2hashn: hash(b2sect,b2wall,b2slab) next index
#define LIGHASHSIZ 1024
static int ligpolmaxvert = 0;
#define lighash(sect,wall,slab) ((((slab)<<6)+((sect)<<4)+(wall))&(LIGHASHSIZ-1))
/*
	Purpose: Generates shadow polygon lists for lighting
	Converts screen-space polygons back to 3D world coordinates
	Stores shadow-casting geometry in ligpol[] arrays per light source
	Used during shadow map generation phase (mode 4)
	Creates hash table for fast polygon lookup by sector/wall/slab
 */
static void ligpoltagfunc (int rethead0, int rethead1, bunchgrp *b)
{
	cam_t gcam = b->cam;
	float f, fx, fy, fz;
	int i, j, rethead[2];

	if ((rethead0|rethead1) < 0) { mono_deloop(rethead1); mono_deloop(rethead0); return; }

		//Use this for dynamic lights only! (doesn't seem to help speed much)
	//if ((shadowtest2_rendmode == 4) && (!(shadowtest2_sectgot[b->gligsect>>5]&(1<<b->gligsect)))) return;

		//Put on FIFO:
	rethead[0] = rethead0; rethead[1] = rethead1;
	for(j=0;j<2;j++)
	{
		i = rethead[j];
		do
		{
			if (j) i = mp[i].p;

			if (glp->ligpolvn >= glp->ligpolvmal)
			{
				glp->ligpolvmal = max(glp->ligpolvmal<<1,1024);
				glp->ligpolv = (point3d *)realloc(glp->ligpolv,glp->ligpolvmal*sizeof(point3d));
			}

			f = gcam.h.z/(/*mp[i].x*b->xformmat[6]*/ + mp[i].y*b->xformmat[7] + b->gnadd.z);
			fx        =  (mp[i].x*b->xformmat[0] + mp[i].y*b->xformmat[1] + b->gnadd.x)*f + gcam.h.x;
			fy        =  (mp[i].x*b->xformmat[3] + mp[i].y*b->xformmat[4] + b->gnadd.y)*f + gcam.h.y;

#if (USEINTZ)
			f = 1.0/((b->gouvmat[0]*fx + b->gouvmat[3]*fy + b->gouvmat[6])*1048576.0*256.0);
#else
			f = 1.0/((b->gouvmat[0]*fx + b->gouvmat[3]*fy + b->gouvmat[6])*gcam.h.z);
#endif
			glp->ligpolv[glp->ligpolvn].x = ((fx-gcam.h.x)*gcam.r.x + (fy-gcam.h.y)*gcam.d.x + (gcam.h.z)*gcam.f.x)*f + gcam.p.x;
			glp->ligpolv[glp->ligpolvn].y = ((fx-gcam.h.x)*gcam.r.y + (fy-gcam.h.y)*gcam.d.y + (gcam.h.z)*gcam.f.y)*f + gcam.p.y;
			glp->ligpolv[glp->ligpolvn].z = ((fx-gcam.h.x)*gcam.r.z + (fy-gcam.h.y)*gcam.d.z + (gcam.h.z)*gcam.f.z)*f + gcam.p.z;

			glp->ligpolvn++;
			if (!j) i = mp[i].n;
		} while (i != rethead[j]);
		mono_deloop(rethead[j]);
	}

	if (glp->ligpoln+1 >= glp->ligpolmal)
	{
		glp->ligpolmal = max(glp->ligpolmal<<1,256);
		glp->ligpol = (ligpol_t *)realloc(glp->ligpol,glp->ligpolmal*sizeof(ligpol_t));
		glp->ligpol[0].vert0 = 0;
	}
	glp->ligpol[glp->ligpoln].b2sect = b->gligsect;
	glp->ligpol[glp->ligpoln].b2wall = b->gligwall;
	glp->ligpol[glp->ligpoln].b2slab = b->gligslab;
	i = lighash(b->gligsect,b->gligwall,b->gligslab);
	glp->ligpol[glp->ligpoln].b2hashn = glp->lighashead[i]; glp->lighashead[i] = glp->ligpoln;
	ligpolmaxvert = max(ligpolmaxvert,glp->ligpolvn-glp->ligpol[glp->ligpoln].vert0);
	glp->ligpoln++;
	glp->ligpol[glp->ligpoln].vert0 = glp->ligpolvn;
}


/*
	Purpose: Portal traversal and sector visibility
	Manages sector-to-sector transitions through portals
	Updates visibility lists when moving between connected areas
	Handles clipping of view regions as camera moves through world
	Maintains mph[] (mono polygon hierarchy) for spatial partitioning
	"tag" refers to sector IDs
*/
static void changetagfunc (int rethead0, int rethead1, bunchgrp *b)
{
	if ((rethead0|rethead1) < 0) return;
	int mapsect = b->gnewtagsect;
	if ((b->gdoscansector)
		&& (!(b->sectgot[mapsect>>5]&(1<<mapsect))))
		scansector(mapsect,b);

	mph_check(mphnum);
	mph[mphnum].head[0] = rethead0;
	mph[mphnum].head[1] = rethead1;
	mph[mphnum].tag = b->gnewtag;
	mphnum++;
}
	//flags&1: do and
	//flags&2: do sub
	//flags&4: reverse cut for sub
static void drawpol_befclip (int tag1, int newtag1, int newtagsect, int plothead0, int plothead1, int flags, bunchgrp* b)
{
	int mtag = tag1 + taginc*b->recursion_depth;
	int tagsect = tag1;
	int mnewtag = newtag1 == -1 ? -1 : newtag1 + taginc*b->recursion_depth;
	b->gnewtagsect = newtagsect;
	cam_t gcam = b->cam;
	#define BSCISDIST 0.000001 //Reduces probability of glitch further
	//#define BSCISDIST 0.0001 //Gaps undetectable
	//#define BSCISDIST 0.1 //Huge gaps
	void (*mono_output)(int h0, int h1, bunchgrp *b);
	dpoint3d *otp, *tp;
	double f, ox, oy, oz;
	int i, j, k, l, h, on, n, plothead[2], imin, imax, i0, i1, omph0, omph1;

	if ((plothead0|plothead1) < 0) return;
	plothead[0] = plothead0; plothead[1] = plothead1;

	n = 2;
	for (h = 0; h < 2; h++)
		for (i = mp[plothead[h]].n; i != plothead[h]; i = mp[i].n)
			n++;
	otp = (dpoint3d *) _alloca(n * sizeof(dpoint3d));
	tp = (dpoint3d *) _alloca(n * sizeof(dpoint3d) * 2);

		//rotate, converting vmono to simple point3d loop
	on = 0;
	for(h=0;h<2;h++) // halfplane?
	{
		i = plothead[h];
		do
		{
			if (h) i = mp[i].p;

			ox = mp[i].x-gcam.p.x; oy = mp[i].y-gcam.p.y;
			otp[on].x = oy*b->xformmatc - ox*b->xformmats;
			otp[on].y = mp[i].z-gcam.p.z;
			otp[on].z = ox*b->xformmatc + oy*b->xformmats; on++;

			if (!h) i = mp[i].n;
		} while (i != plothead[h]);
		mono_deloop(plothead[h]);
	}

		//clip
	n = 0;
	for(i=on-1,j=0;j<on;i=j,j++)
	{
		if (otp[i].z >= BSCISDIST) { tp[n] = otp[i]; n++; }
		if ((otp[i].z >= BSCISDIST) != (otp[j].z >= BSCISDIST))
		{
			f = (BSCISDIST-otp[j].z)/(otp[i].z-otp[j].z);
			tp[n].x = (otp[i].x-otp[j].x)*f + otp[j].x;
			tp[n].y = (otp[i].y-otp[j].y)*f + otp[j].y;
			tp[n].z = BSCISDIST; n++;
		}
	}
	if (n < 3) return;

		//project & find x extents
	for(i=0;i<n;i++)
	{
		f = gcam.h.z/tp[i].z;
		tp[i].x = tp[i].x*f + gcam.h.x;
		tp[i].y = tp[i].y*f + gcam.h.y;
	}

		//generate vmono
	mono_genfromloop(&plothead[0], &plothead[1], tp, n);
	if ((plothead[0] | plothead[1]) < 0) {
		mono_deloop(plothead[0]);
		mono_deloop(plothead[1]);
		return;
	}

	if (flags&1 || flags&8)
	{
		if (mnewtag >= 0)
		{
			b->gnewtagsect = newtagsect;
			b->gnewtag = mnewtag; omph0 = mphnum;
			b->gdoscansector =  !(flags&8);
			int bop = MONO_BOOL_AND;//(flags & CLIP_PORTAL_FLAG) ? MONO_BOOL_SUB_REV : MONO_BOOL_AND;
			for (i = mphnum - 1; i >= 0; i--)
				if (mph[i].tag == mtag)
					mono_bool(
						mph[i].head[0],
						mph[i].head[1],
						plothead[0],
						plothead[1],
						bop,
						b,
						changetagfunc);
			{
				for(l=omph0;l<mphnum;l++)
				{
					mph[omph0] = mph[l]; k = omph0; omph0++;
					for(j=omph0-1;j>=0;j--) //Join monos
					{
						if (mph[j].tag != b->gnewtag) continue;
						if (!mono_join(mph[j].head[0],mph[j].head[1],mph[k].head[0],mph[k].head[1],&i0,&i1)) continue;
						for(i=2-1;i>=0;i--) { mono_deloop(mph[k].head[i]); mono_deloop(mph[j].head[i]); }
						omph0--; mph[k] = mph[omph0];
						mph[j].head[0] = i0; mph[j].head[1] = i1; k = j;
					}
				}
				mphnum = omph0;
			}
		}
		else { // newtag == -1
			if (shadowtest2_rendmode == 4)
				mono_output = ligpoltagfunc;
				//add to light list // this will process point lights. otherwize will only use plr light.
			else if (b->gflags < 2) mono_output = drawtagfunc_ws;
			else mono_output = skytagfunc; //calls drawtagfunc inside
			for (i = mphnum - 1; i >= 0; i--)
				if (mph[i].tag == mtag)
					mono_bool(mph[i].head[0], mph[i].head[1], plothead[0], plothead[1],MONO_BOOL_AND, b, mono_output);
		}
	}
	if (flags&2)
	{
		if (!(flags&4)) j = MONO_BOOL_SUB;
					  else j = MONO_BOOL_SUB_REV; // when floor.

		b->gnewtag = mtag;
		b->gnewtagsect = tagsect;
		b->gdoscansector = 0; omph0 = mphnum; omph1 = mphnum;
		for(i=mphnum-1;i>=0;i--)
		{
			if (mph[i].tag != mtag) continue;
			mono_bool(mph[i].head[0],mph[i].head[1],plothead[0],plothead[1],j,b,changetagfunc);
			mono_deloop(mph[i].head[1]);
			mono_deloop(mph[i].head[0]);

			omph0--; mph[i] = mph[omph0];
		}

			//valid mph's stored in 2 blocks: (0<=?<omph0), (omph1<=?<mphnum)

			for(l=omph1;l<mphnum;l++)
			{
				mph[omph0] = mph[l]; k = omph0; omph0++;
				for(j=omph0-1;j>=0;j--) //Join monos
				{
					if (mph[j].tag != b->gnewtag) continue;
					if (!mono_join(mph[j].head[0], mph[j].head[1], mph[k].head[0], mph[k].head[1], &i0, &i1)) continue;
					for (i = 2 - 1; i >= 0; i--) {
						mono_deloop(mph[k].head[i]);
						mono_deloop(mph[j].head[i]);
					}
					omph0--;
					mph[k] = mph[omph0];
					mph[j].head[0] = i0;
					mph[j].head[1] = i1;
					k = j;
				}
			}
		mphnum = omph0;

	}

	mono_deloop(plothead[1]);
	mono_deloop(plothead[0]);
}



	//FIXFIXFIX: clean this up!
static void gentex_xform (float *ouvmat, bunchgrp *b)
{
	cam_t gcam = b->cam;
	float ax, ay, az, bx, by, bz, cx, cy, cz, p0x, p0y, p0z, p1x, p1y, p1z, p2x, p2y, p2z, f;

	ax = ouvmat[3]; bx = ouvmat[6]; cx = ouvmat[0];
	ay = ouvmat[4]; by = ouvmat[7]; cy = ouvmat[1];
	az = ouvmat[5]; bz = ouvmat[8]; cz = ouvmat[2];
	p2x = ay*bz - az*by; p2y = az*bx - ax*bz; p2z = ax*by - ay*bx; f = p2x*cx + p2y*cy + p2z*cz;
	p0x = by*cz - bz*cy; p0y = bz*cx - bx*cz; p0z = bx*cy - by*cx;
	p1x = cy*az - cz*ay; p1y = cz*ax - cx*az; p1z = cx*ay - cy*ax;
#if (USEINTZ)
	f = 1.0 / (f*16.0);
#else
	f = 1048576.0*16.0 / (f*gcam.h.z);
#endif
	ax = (1.0/65536.0 )*f;
	ay = ((float)64)*f;
	az = ((float)64)*f;
	ouvmat[0] = p2x*ax; ouvmat[1] = p0x*ay; ouvmat[2] = p1x*az;
	ouvmat[3] = p2y*ax; ouvmat[4] = p0y*ay; ouvmat[5] = p1y*az;
	ouvmat[6] = p2z*ax; ouvmat[7] = p0z*ay; ouvmat[8] = p1z*az;
	ouvmat[6] = ouvmat[6]*gcam.h.z - ouvmat[0]*gcam.h.x - ouvmat[3]*gcam.h.y;
	ouvmat[7] = ouvmat[7]*gcam.h.z - ouvmat[1]*gcam.h.x - ouvmat[4]*gcam.h.y;
	ouvmat[8] = ouvmat[8]*gcam.h.z - ouvmat[2]*gcam.h.x - ouvmat[5]*gcam.h.y;

	if (renderinterp)
	{
		ouvmat[1] -= ouvmat[0]*32768.0; ouvmat[2] -= ouvmat[0]*32768.0;
		ouvmat[4] -= ouvmat[3]*32768.0; ouvmat[5] -= ouvmat[3]*32768.0;
		ouvmat[7] -= ouvmat[6]*32768.0; ouvmat[8] -= ouvmat[6]*32768.0;
	}
}

static void gentex_sky (surf_t *sur, bunchgrp *b)
{
	cam_t gcam = b->cam;
	float f, g, h;
	int i;

	if (b->gflags >= 2) return; //if texture is skybox, return early

		//Crappy paper sky :/
	h = 65536;
	f = atan2(gcam.f.y,gcam.f.x)         *-h/PI*2.f;
	g = asin(min(max(gcam.f.z,-1.f),1.f))*-h/PI*2.f;
	b->gouvmat[0] = sur->uv[0].x*h + f; b->gouvmat[3] = sur->uv[1].x*h; b->gouvmat[6] = sur->uv[2].x*h;
	b->gouvmat[1] = sur->uv[0].y*h + g; b->gouvmat[4] = sur->uv[1].y*h; b->gouvmat[7] = sur->uv[2].y*h;
	b->gouvmat[2] =              h    ; b->gouvmat[5] =            0.f; b->gouvmat[8] =            0.f;
	gentex_xform(b->gouvmat,b);
}

static void gentex_ceilflor (sect_t *sec, wall_t *wal, surf_t *sur, int isflor, bunchgrp *b)
{
	cam_t gcam = b->cam;
	float f, g, fz, ax, ay, wx, wy, ox, oy, oz, fk[6];
	int i;

	if (!(sur->flags&4)) //Not relative alignment
	{
			//(sur->uv[1].x)*x + (sur->uv[2].x)*y = (u-sur->uv[0].x)
			//(sur->uv[1].y)*x + (sur->uv[2].y)*y = (v-sur->uv[0].y)
		fk[0] = sur->uv[1].x; fk[2] = sur->uv[2].x;
		fk[1] = sur->uv[1].y; fk[3] = sur->uv[2].y;
		fz = 1.0; ax = -sur->uv[0].x; ay = -sur->uv[0].y;
	}
	else //Relative alignment
	{
		wx = wal[wal[0].n].x-wal[0].x;
		wy = wal[wal[0].n].y-wal[0].y;
		fk[0] = sur->uv[1].x*wx - sur->uv[2].x*wy; fk[2] = sur->uv[1].x*wy + sur->uv[2].x*wx;
		fk[1] = sur->uv[1].y*wx - sur->uv[2].y*wy; fk[3] = sur->uv[1].y*wy + sur->uv[2].y*wx;
		fz = sqrt(wx*wx + wy*wy);
		ax = (wx*wal[0].x + wy*wal[0].y)*sur->uv[1].x + (wx*wal[0].y - wy*wal[0].x)*sur->uv[2].x - sur->uv[0].x*fz;
		ay = (wx*wal[0].x + wy*wal[0].y)*sur->uv[1].y + (wx*wal[0].y - wy*wal[0].x)*sur->uv[2].y - sur->uv[0].y*fz;
	}

	f = fk[0]*fk[3] - fk[1]*fk[2]; if (f > 0.f) f = 1.f/f;
	for(i=6;i>=0;i-=3)
	{          //u,v:
		fk[4] = (i==3)*fz + ax;
		fk[5] = (i==6)*fz + ay;
		ox = (fk[3]*fk[4] - fk[2]*fk[5])*f;
		oy = (fk[0]*fk[5] - fk[1]*fk[4])*f;
		oz = getslopez(sec,isflor,ox,oy);
		ox -= gcam.p.x; oy -= gcam.p.y; oz -= gcam.p.z;
		b->gouvmat[i+0] = ox*gcam.r.x + oy*gcam.r.y + oz*gcam.r.z;
		b->gouvmat[i+1] = ox*gcam.d.x + oy*gcam.d.y + oz*gcam.d.z;
		b->gouvmat[i+2] = ox*gcam.f.x + oy*gcam.f.y + oz*gcam.f.z;
	}

	for(i=9-1;i>=0;i--) b->gouvmat[i] *= 256.f;

	b->gouvmat[3] -= b->gouvmat[0]; b->gouvmat[4] -= b->gouvmat[1]; b->gouvmat[5] -= b->gouvmat[2];
	b->gouvmat[6] -= b->gouvmat[0]; b->gouvmat[7] -= b->gouvmat[1]; b->gouvmat[8] -= b->gouvmat[2];

	gentex_xform(b->gouvmat, b);
}

static void gentex_wall (kgln_t *npol2, surf_t *sur, bunchgrp *b)
{
	cam_t gcam = b->cam;
	float f, g, ox, oy, oz, rdet, fk[24];
	int i;

	for(i=0;i<3;i++)
	{
		ox = npol2[i].x-gcam.p.x; oy = npol2[i].y-gcam.p.y; oz = npol2[i].z-gcam.p.z;
		npol2[i].x = ox*gcam.r.x + oy*gcam.r.y + oz*gcam.r.z;
		npol2[i].y = ox*gcam.d.x + oy*gcam.d.y + oz*gcam.d.z;
		npol2[i].z = ox*gcam.f.x + oy*gcam.f.y + oz*gcam.f.z;
	}

		//sx = npol2[i].x*gcam.h.z/npol2[i].z+gcam.h.x
		//sy = npol2[i].y*gcam.h.z/npol2[i].z+gcam.h.y
		//npol2[i].u = (b->gouvmat[1]*sx + b->gouvmat[4]*sy + b->gouvmat[7])/(b->gouvmat[0]*sx + b->gouvmat[3]*sy + b->gouvmat[6])
		//npol2[i].v = (b->gouvmat[2]*sx + b->gouvmat[5]*sy + b->gouvmat[8])/(b->gouvmat[0]*sx + b->gouvmat[3]*sy + b->gouvmat[6])
		//npol2[i].z =                                            1/(b->gouvmat[0]*sx + b->gouvmat[3]*sy + b->gouvmat[6])
		//   Solve ^ for b->gouvmat[*]
	fk[0] = npol2[0].z; fk[3] = npol2[0].x*gcam.h.z + npol2[0].z*gcam.h.x; fk[6] = npol2[0].y*gcam.h.z + npol2[0].z*gcam.h.y;
	fk[1] = npol2[1].z; fk[4] = npol2[1].x*gcam.h.z + npol2[1].z*gcam.h.x; fk[7] = npol2[1].y*gcam.h.z + npol2[1].z*gcam.h.y;
	fk[2] = npol2[2].z; fk[5] = npol2[2].x*gcam.h.z + npol2[2].z*gcam.h.x; fk[8] = npol2[2].y*gcam.h.z + npol2[2].z*gcam.h.y;
	fk[12] = fk[4]*fk[8] - fk[5]*fk[7];
	fk[13] = fk[5]*fk[6] - fk[3]*fk[8];
	fk[14] = fk[3]*fk[7] - fk[4]*fk[6];
	fk[18] = fk[2]*fk[7] - fk[1]*fk[8];
	fk[19] = fk[0]*fk[8] - fk[2]*fk[6];
	fk[20] = fk[1]*fk[6] - fk[0]*fk[7];
	fk[21] = fk[1]*fk[5] - fk[2]*fk[4];
	fk[22] = fk[2]*fk[3] - fk[0]*fk[5];
	fk[23] = fk[0]*fk[4] - fk[1]*fk[3];
	b->gouvmat[6] = fk[12] + fk[13] + fk[14];
	b->gouvmat[0] = fk[18] + fk[19] + fk[20];
	b->gouvmat[3] = fk[21] + fk[22] + fk[23];

	fk[15] = b->gouvmat[6]*fk[0] + b->gouvmat[0]*fk[3] + b->gouvmat[3]*fk[6];
	fk[16] = b->gouvmat[6]*fk[1] + b->gouvmat[0]*fk[4] + b->gouvmat[3]*fk[7];
	fk[17] = b->gouvmat[6]*fk[2] + b->gouvmat[0]*fk[5] + b->gouvmat[3]*fk[8];

	fk[ 9] = fk[15]*npol2[0].u;
	fk[10] = fk[16]*npol2[1].u;
	fk[11] = fk[17]*npol2[2].u;
	b->gouvmat[7] = fk[12]*fk[9] + fk[13]*fk[10] + fk[14]*fk[11];
	b->gouvmat[1] = fk[18]*fk[9] + fk[19]*fk[10] + fk[20]*fk[11];
	b->gouvmat[4] = fk[21]*fk[9] + fk[22]*fk[10] + fk[23]*fk[11];

	fk[ 9] = fk[15]*npol2[0].v;
	fk[10] = fk[16]*npol2[1].v;
	fk[11] = fk[17]*npol2[2].v;
	b->gouvmat[8] = fk[12]*fk[9] + fk[13]*fk[10] + fk[14]*fk[11];
	b->gouvmat[2] = fk[18]*fk[9] + fk[19]*fk[10] + fk[20]*fk[11];
	b->gouvmat[5] = fk[21]*fk[9] + fk[22]*fk[10] + fk[23]*fk[11];

	rdet = 1.0/(fk[0]*fk[12] + fk[1]*fk[13] + fk[2]*fk[14]);

#if (USEINTZ)
	g = gcam.h.z*rdet/(1048576.0*256.0);
#else
	g = rdet;
#endif
													 b->gouvmat[0] *= g; b->gouvmat[3] *= g; b->gouvmat[6] *= g; g *= rdet*65536.0;
	f = (float)64*g;            b->gouvmat[1] *= f; b->gouvmat[4] *= f; b->gouvmat[7] *= f;
	f = (float)64*g;            b->gouvmat[2] *= f; b->gouvmat[5] *= f; b->gouvmat[8] *= f;

	if (renderinterp)
	{
		b->gouvmat[1] -= b->gouvmat[0]*32768.0; b->gouvmat[2] -= b->gouvmat[0]*32768.0;
		b->gouvmat[4] -= b->gouvmat[3]*32768.0; b->gouvmat[5] -= b->gouvmat[3]*32768.0;
		b->gouvmat[7] -= b->gouvmat[6]*32768.0; b->gouvmat[8] -= b->gouvmat[6]*32768.0;
	}
}

/*
the mono engine produces camera-space polygons that are clipped to not overlap.
The plothead[0] and plothead[1] contain monotone polygon pairs representing
the final visible geometry ready for 2D projection.
The b parameter is a bunch index - this function processes one "bunch" (visible sector group) at a time. The traversal logic is in the caller that:
*/

static void drawalls (int bid, mapstate_t* map, bunchgrp* b)
{
	cam_t gcam = b->cam;
	// === VARIABLE DECLARATIONS ===
	//extern void loadpic (tile_t *);
	#define MAXVERTS 256 //FIX:timebomb: assumes there are never > 256 sectors connected at same vertex
	vertlist_t verts[MAXVERTS];
	bunchverts_t *twal;
	int twaln;
	dpoint3d pol[4], npol[6];
	kgln_t npol2[3];
	double opolz[4];
	sect_t *sec;
	wall_t *wal;
	surf_t *sur;
	point2d *grad;
	double f, fz, dx;
	int i, j, k, l, m, n, s, ns, isflor, plothead[2], wn, w, ww, nw, vn, ws, wi, we, kval[4], imin, imax;
	int ks[2], ke[2], col, n0, n1;

	// === SETUP SECTOR AND WALL DATA ===
	s = b->bunch[bid].sec;
	sec = curMap->sect;
	wal = sec[s].wall;
#if 0
	i = bunch[bid].wal0;
	x0 = wal[i].x; y0 = wal[i].y; i += wal[i].n;
	x1 = wal[i].x; y1 = wal[i].y; f = bunch[bid].fra0;
	twalx0 = (x1-x0)*f + x0;
	twaly0 = (y1-y0)*f + y0;

	i = b->bunch[bid].wal1;
	x0 = wal[i].x; y0 = wal[i].y; i += wal[i].n;
	x1 = wal[i].x; y1 = wal[i].y; f = bunch[bid].fra1;
	twalx1 = (x1-x0)*f + x0;
	twaly1 = (y1-y0)*f + y0;
#endif

	twal = (bunchverts_t *)_alloca((curMap->sect[b->bunch[bid].sec].n+1)*sizeof(bunchverts_t));
	twaln = prepbunch(bid,twal,b);
	b->gligsect = s; b->gligslab = 0;

	// === BUNCH MANAGEMENT: DELETE CURRENT BUNCH FROM GRID ===
	// Removes processed bunch and compacts the bunch grid structure
	// Example: bunchn=6,closest=2 (before op)
	//	 0 1 2 3 4
	//0
	//1  0:x
	//2  1:. . ? ? ?
	//3  3:x x .
	//4  6:x x . x
	//5 10:x x . x x
	b->bunchn--; b->bunch[bid] = b->bunch[b->bunchn];
	j = (((b->bunchn-1)*b->bunchn)>>1);
	memcpy(&b->bunchgrid[((bid-1)*bid)>>1],&b->bunchgrid[j],bid*sizeof(b->bunchgrid[0]));
	for(i=bid+1;i<b->bunchn;i++) b->bunchgrid[(((i-1)*i)>>1)+bid] = ((b->bunchgrid[j+i]&1)<<1) + (b->bunchgrid[j+i]>>1);
	if (b->has_portal_clip)
		int a =0;
	// === DRAW CEILINGS & FLOORS ===
	for(isflor=0;isflor<2;isflor++) // floor ceil
	{
		// here, when we draw sector of the exit portal we get glitches when it would draw a triangle with point below the camera resulting in triangle spanning entire vertical of the screen

		     // Back-face culling: skip if camera is on wrong side of surface
			// need to get original slope, as if camera was in origin.
	//if (!b->has_portal_clip)
		float surfpos = getslopez(&sec[s],isflor,b->cam.p.x,b->cam.p.y);
		if ((b->cam.p.z >= surfpos) == isflor) // ignore backfaces
				continue;

		// Setup surface properties (height, gradient, color)
		fz = sec[s].z[isflor]; grad = &sec[s].grad[isflor];
		gcurcol = (min(sec[s].surf[isflor].asc>>8,255)<<24) +
					 (min(sec[s].surf[isflor].rsc>>5,255)<<16) +
					 (min(sec[s].surf[isflor].gsc>>5,255)<< 8) +
					 (min(sec[s].surf[isflor].bsc>>5,255)    );
		gcurcol = argb_interp(gcurcol,(gcurcol&0xff000000)+((gcurcol&0xfcfcfc)>>2),(int)(compact2d*24576.0));

		// Calculate surface normal vector
		b->gnorm.x = grad->x;
		b->gnorm.y = grad->y;
		b->gnorm.z = 1.f; if (isflor) { b->gnorm.x = -b->gnorm.x; b->gnorm.y = -b->gnorm.y; b->gnorm.z = -b->gnorm.z; }
		f = 1.0/sqrt(b->gnorm.x*b->gnorm.x + b->gnorm.y*b->gnorm.y + 1); b->gnorm.x *= f; b->gnorm.y *= f; b->gnorm.z *= f;

			//plane point: (wal[0].x,wal[0].y,fz)
			//plane norm: <grad->x,grad->y,1>
			//
			//   (wal[i].x-wal[0].x)*grad->x +
			//   (wal[i].y-wal[0].y)*grad->y +
			//   (?       -      fz)*      1 = 0
		// Build polygon for ceiling/floor using plane equation:
		plothead[0] = -1; plothead[1] = -1;
		for (ww = twaln; ww >= 0; ww -= twaln) plothead[isflor] = mono_ins(
			                                       plothead[isflor], twal[ww].x, twal[ww].y,
			                                       b->gnorm.z * -1e32);
		//do not replace w/single zenith point - ruins precision
		i = isflor ^ 1;
		for (ww = 0; ww <= twaln; ww++) plothead[i] = mono_ins(plothead[i], twal[ww].x, twal[ww].y,
		                                                       (wal[0].x - twal[ww].x) * grad->x + (
			                                                       wal[0].y - twal[ww].y) * grad->y + fz);
		plothead[i] = mp[plothead[i]].n;

		// Setup texture and rendering flags
		sur = &sec[s].surf[isflor];
		gtpic = &gtile[sur->tilnum];
		//if (!gtpic->tt.f) loadpic(gtpic);
		if (sec[s].surf[isflor].flags & (1 << 17)) { b->gflags = 2; } //skybox ceil/flor
		else if (sec[s].surf[isflor].flags & (1 << 16)) {  //parallaxing ceil/flor
			b->gflags = 1;
			gentex_sky(sur, b);
		}
		else {
			b->gflags = 0;
			gentex_ceilflor(&sec[s], wal, sur, isflor, b);
		}
		b->gligwall = isflor - 2;

		int myport = sec[s].tags[1];
		if (myport>=0)
			int asd=1;
		bool skipport= (b->has_portal_clip && s==b->testignoresec && isflor == b->testignorewall);
		bool needsfinal = b->recursion_depth == MAX_PORTAL_DEPTH;
		bool isportal = myport>=0 && portals[myport].destpn >=0 && portals[myport].surfid == isflor;
		if (isportal && !needsfinal && !skipport) {
			int endpn = portals[myport].destpn;
			drawpol_befclip(s, portals[endpn].sect+taginc, portals[endpn].sect,
				plothead[0],plothead[1],  3, b);
			draw_hsr_enter_portal(map, myport, b,plothead[0],plothead[1]);
		}

		else {
			drawpol_befclip(s,-1,-1,plothead[0],plothead[1],(isflor<<2)+3,b);
		}
	}

	// === DRAW WALLS ===
	for(ww=0;ww<twaln;ww++)
	{
		// Get wall vertices and setup wall segment
		vn = getwalls_imp(s,twal[ww].i,verts,MAXVERTS,map);
		w = twal[ww].i; nw = wal[w].n+w;
		sur = &wal[w].surf;

		// Calculate wall length and setup color/normal
		dx = sqrt((wal[nw].x-wal[w].x)*(wal[nw].x-wal[w].x) + (wal[nw].y-wal[w].y)*(wal[nw].y-wal[w].y));
		gcurcol = (min(sur->asc>>8,255)<<24) +
					 (min(sur->rsc>>5,255)<<16) +
					 (min(sur->gsc>>5,255)<< 8) +
					 (min(sur->bsc>>5,255)    );
		b->gnorm.x = wal[w].y-wal[nw].y;
		b->gnorm.y = wal[nw].x-wal[w].x;
		b->gnorm.z = 0;
		f = 1.0/sqrt(b->gnorm.x*b->gnorm.x + b->gnorm.y*b->gnorm.y); b->gnorm.x *= f; b->gnorm.y *= f;
		// Setup base wall quad (floor to ceiling)
		pol[0].x = twal[ww  ].x; pol[0].y = twal[ww  ].y; pol[0].z = getslopez(&sec[s],0,pol[0].x,pol[0].y); //pol[0].n = 1;
		pol[1].x = twal[ww+1].x; pol[1].y = twal[ww+1].y; pol[1].z = getslopez(&sec[s],0,pol[1].x,pol[1].y); //pol[1].n = 1;
		pol[2].x = twal[ww+1].x; pol[2].y = twal[ww+1].y; pol[2].z = getslopez(&sec[s],1,pol[2].x,pol[2].y); //pol[2].n = 1;
		pol[3].x = twal[ww  ].x; pol[3].y = twal[ww  ].y; pol[3].z = getslopez(&sec[s],1,pol[3].x,pol[3].y); //pol[3].n =-3;

		// === WALL SEGMENT SUBDIVISION LOOP =		   // Process wall in segments, clipping against adjacent sectors
		opolz[3] = pol[0].z; opolz[2] = pol[1].z;
		for(m=0;m<=(vn<<1);m++) //Warning: do not reverse for loop!
		{
			// Update Z-coordinates for current segment
			opolz[0] = opolz[3]; opolz[1] = opolz[2];
			if (m == (vn<<1)) { opolz[2] = pol[2].z; opolz[3] = pol[3].z; }
			else
			{
				opolz[2] = getslopez(&sec[verts[m>>1].s],m&1,pol[2].x,pol[2].y);
				opolz[3] = getslopez(&sec[verts[m>>1].s],m&1,pol[3].x,pol[3].y);
			}
			//if ((opolz[0] >= opolz[3]) && (opolz[1] >= opolz[2])) continue; //Early-out optimization: skip walls with 0 height

			// Skip zero-height wall segments (optimization)
			if ((max(pol[0].z,opolz[0]) >= min(pol[3].z,opolz[3])-1e-4) &&
				 (max(pol[1].z,opolz[1]) >= min(pol[2].z,opolz[2])-1e-4)) continue; //Early-out optimization: skip walls with 0 height FIXFIXFIXFIX

			/*Most critical usage - determines visible wall segments
			Intersects current wall trapezoid with adjacent sector geometry
			Returns monotone polygon pair representing visible portion
			If no intersection, wall segment is completely occluded*/

			//if (!intersect_traps_mono(pol[0].x,pol[0].y, pol[1].x,pol[1].y, pol[0].z,pol[1].z,pol[2].z,pol[3].z, opolz[0],opolz[1],opolz[2],opolz[3], &plothead[0],&plothead[1])) continue;
			// Calculate intersection of wall segment with clipping trapezoids
			f = 1e-7; //FIXFIXFIXFIX:use ^ ?
			if (!intersect_traps_mono(pol[0].x,pol[0].y, pol[1].x,pol[1].y, pol[0].z-f,pol[1].z-f,pol[2].z+f,pol[3].z+f, opolz[0]-f,opolz[1]-f,opolz[2]+f,opolz[3]+f, &plothead[0],&plothead[1]))
				continue;

			// Render wall segment if visible

			if ((!(m & 1)) || (wal[w].surf.flags & (1 << 5))) //Draw wall here //(1<<5): 1-way
			{
				//	gtpic = &gtile[sur->tilnum]; if (!gtpic->tt.f) loadpic(gtpic);
				if (sur->flags & (1 << 17))
				{ b->gflags = 2; } //skybox ceil/flor
				else if (sur->flags & (1 << 16)) {
					b->gflags = 1;
					gentex_sky(sur, b);
				} //parallaxing ceil/flor
				else {
					// Calculate UV mapping for wall texture
					npol2[0].x = wal[w].x;
					npol2[0].y = wal[w].y;
					npol2[0].z = getslopez(&sec[s], 0, wal[w].x, wal[w].y);
					npol2[1].x = wal[nw].x;
					npol2[1].y = wal[nw].y;
					npol2[1].z = npol2[0].z;
					npol2[2].x = wal[w].x;
					npol2[2].y = wal[w].y;
					npol2[2].z = npol2[0].z + 1.f;
					// Determine reference Z-level texture alignment
					if (!(sur->flags & 4)) f = sec[s].z[0];
					else if (!vn) f = sec[s].z[1]; //White walls don't have verts[]!
					else if (!m) f = sec[verts[0].s].z[0];
					else f = sec[verts[(m - 1) >> 1].s].z[0];
					// Apply UV coordinates with proper scaling
					npol2[0].u = sur->uv[0].x;
					npol2[0].v = sur->uv[2].y * (npol2[0].z - f) + sur->uv[0].y;
					npol2[1].u = sur->uv[1].x * dx + npol2[0].u;
					npol2[1].v = sur->uv[1].y * dx + npol2[0].v;
					npol2[2].u = sur->uv[2].x + npol2[0].u;
					npol2[2].v = sur->uv[2].y + npol2[0].v;
					b->gflags = 0;
					gentex_wall(npol2, sur, b);
				}
				b->gligwall = w;
				b->gligslab = m;
				ns = -1;
				/* notes:
				 *	b->gligsect = s;        // Current sector
					b->gligwall = w;        // Wall index
					b->gligslab = m;        // Segment/slab number (0,1,2... for each vertical division)*/
			} else {
				ns = verts[m >> 1].s; // Portal to adjacent sector
			}
			// Render the wall polygon
			int myport = wal[w].tags[1];
			if (myport >= 0 && portals[myport].destpn >=0 && portals[myport].kind == PORT_WALL) {
					int endp = portals[myport].destpn;
				drawpol_befclip(s, portals[endp].sect+taginc, portals[endp].sect, plothead[0], plothead[1],  3, b);
				draw_hsr_enter_portal(map, myport, b,plothead[0],plothead[1]);
			} else {
				// could be 7 or 3, .111 or .011
				drawpol_befclip(s, ns, ns, plothead[0], plothead[1], ((m > vn) << 2) + 3, b);
			}
		}

	}
}
/*
 *The function operates in different modes based on shadowtest2_rendmode:
	Mode 2 (Standard Rendering):

	Renders visible geometry from camera viewpoint
	Populates eyepol[] array with screen-space polygons
	Each polygon contains texture mapping data and surface normals
	Mode 4 (Shadow Map Generation):

	Renders from light source positions
	Generates shadow polygon lists (ligpol[]) for each light
	Creates visibility data for shadow casting
*/
void reset_context() {
	eyepoln = 0; eyepolvn = 0;
}
void draw_hsr_polymost(cam_t *cc, mapstate_t *map, int dummy){
	bunchgrp bs;
	bs.cam = *cc;
	bs.orcam = *cc;
	bs.recursion_depth = 0;
	bs.has_portal_clip = false;
	draw_hsr_polymost_ctx(map,&bs);
}

void draw_hsr_polymost_ctx (mapstate_t *lgs, bunchgrp *newctx) {
	if (!newctx) {
		return;
	}
	int prehead1 = -1;
	int prehead2 = -1;
	int presect = -1;

	int recursiveDepth = newctx->recursion_depth;
	bunchgrp *b;
	b = newctx;
	b->sectgotn = 0;
	b->sectgot = 0;
	b->sectgotmal = 0;
	b->bunchgot=0;
	b->bunchn=0;
	b->bunchmal=0;
	b->bunchgrid =0;
	cam_t gcam = b->cam;
	cam_t oricam = b->orcam;

	wall_t *wal;
	spri_t *spr;
	dpoint3d dpos, drig, ddow, dfor;
	dpoint3d fp, bord[4], bord2[8];
	double f, d;
	unsigned int *uptr;
	int i, j, k, n, s, w, closest, col, didcut, halfplane;

	if (shadowtest2_rendmode == 4)
	{
		glp = &shadowtest2_light[glignum];
	//	if ((!(glp->flags&1)) || (!shadowtest2_useshadows)) return;
	}

	curMap = lgs;

	//if ((lgs->numsects <= 0) || ((unsigned)gcam.cursect >= (unsigned)lgs->numsects))
	//{
	////if (shadowtest2_rendmode != 4)
	////{
	////	for(i=0,j=gcam.c.f;i<gcam.c.y;i++,j+=gcam.c.p) memset8((void *)j,0x00000000,gcam.c.x<<2);
	////	for(i=0,j=gcam.z.f;i<gcam.z.y;i++,j+=gcam.z.p) memset8((void *)j,0x7f7f7f7f,gcam.z.x<<2);
	////}
	////if (shadowtest2_rendmode != 4) eyepoln = 0; //Prevents drawpollig() from crashing
	////	return;
	//}
	if (!b->bunchmal)
	{
		b->bunchmal = 64;
		b->bunch     = (bunch_t       *)malloc(b->bunchmal*sizeof(b->bunch[0]));
		b->bunchgot  = (unsigned int  *)malloc(((b->bunchmal+31)&~31)>>3);
		b->bunchgrid = (unsigned char *)malloc(((b->bunchmal-1)*b->bunchmal)>>1);
	}
	if (lgs->numsects > b->sectgotn)
	{
		if (b->sectgotmal) free((void *)b->sectgotmal);
		b->sectgotn = ((lgs->numsects+127)&~127);
		b->sectgotmal = (unsigned int *)malloc((b->sectgotn>>3)+16); //NOTE:malloc doesn't guarantee 16-byte alignment!
		b->sectgot = (unsigned int *)((((intptr_t)b->sectgotmal)+15)&~15);
	}
	if ((shadowtest2_rendmode != 4) && (lgs->numsects > shadowtest2_sectgotn))
	{
		if (shadowtest2_sectgotmal) free((void *)shadowtest2_sectgotmal);
		shadowtest2_sectgotn = ((lgs->numsects+127)&~127);
		shadowtest2_sectgotmal = (unsigned int *)malloc((shadowtest2_sectgotn>>3)+16); //NOTE:malloc doesn't guarantee 16-byte alignment!
		shadowtest2_sectgot = (unsigned int *)((((intptr_t)shadowtest2_sectgotmal)+15)&~15);
	}
	if (!mphmal)
		mono_initonce();

#ifdef STANDALONE
	fixposx = 0; fixposy = 32;
	//fixposx = 600; fixposy = 256;
#endif

		//Hack to keep camera away from sector line; avoids clipping glitch in drawpol_befclip/changetagfunc
//wal = lgs->sect[gcam.cursect].wall;
//for(i=lgs->sect[gcam.cursect].n-1;i>=0;i--)
//{
//	#define WALHAK 1e-3
//	j = wal[i].n+i;
//	d = distpoint2line2(gcam.p.x,gcam.p.y,wal[i].x,wal[i].y,wal[j].x,wal[j].y); if (d >= WALHAK*WALHAK) continue;
//	fp.x = wal[j].x-wal[i].x;
//	fp.y = wal[j].y-wal[i].y;
//	f = (WALHAK - sqrt(d))/sqrt(fp.x*fp.x + fp.y*fp.y);
//	gcam.p.x -= fp.y*f;
//	gcam.p.y += fp.x*f;
//}

	if (shadowtest2_rendmode != 4)
	{
			//Horrible hacks for internal build2 global variables
		dpos.x = 0.0; dpos.y = 0.0; dpos.z = 0.0;
		drig.x = 1.0; drig.y = 0.0; drig.z = 0.0;
		ddow.x = 0.0; ddow.y = 1.0; ddow.z = 0.0;
		dfor.x = 0.0; dfor.y = 0.0; dfor.z = 1.0;
	//	drawpoly_setup(                           (tiletype *)&gcam.c,gcam.z.f-gcam.c.f,&dpos,&drig,&ddow,&dfor,gcam.h.x,gcam.h.y,gcam.h.z);
		//drawcone_setup(cputype,shadowtest2_numcpu,(tiletype *)&gcam.c,gcam.z.f-gcam.c.f,&dpos,&drig,&ddow,&dfor,gcam.h.x,gcam.h.y,gcam.h.z);
		// drawkv6_setup(&drawkv6_frame,            (tiletype *)&gcam.c,gcam.z.f-gcam.c.f,&dpos,&drig,&ddow,&dfor,gcam.h.x,gcam.h.y,gcam.h.z);

		for(i=shadowtest2_numlights-1;i>=0;i--)
		{
				//Transform shadowtest2_light to screen space
			fp.x = shadowtest2_light[i].p.x-gcam.p.x;
			fp.y = shadowtest2_light[i].p.y-gcam.p.y;
			fp.z = shadowtest2_light[i].p.z-gcam.p.z;
			slightpos[i].x = fp.x*gcam.r.x + fp.y*gcam.r.y + fp.z*gcam.r.z;
			slightpos[i].y = fp.x*gcam.d.x + fp.y*gcam.d.y + fp.z*gcam.d.z;
			slightpos[i].z = fp.x*gcam.f.x + fp.y*gcam.f.y + fp.z*gcam.f.z;

			fp.x = shadowtest2_light[i].f.x;
			fp.y = shadowtest2_light[i].f.y;
			fp.z = shadowtest2_light[i].f.z;
			f = fp.x*fp.x + fp.y*fp.y + fp.z*fp.z;
			if (f > 0.f) { f = 1.f/sqrt(f); fp.x *= f; fp.y *= f; fp.z *= f; }
			slightdir[i].x = fp.x*gcam.r.x + fp.y*gcam.r.y + fp.z*gcam.r.z;
			slightdir[i].y = fp.x*gcam.d.x + fp.y*gcam.d.y + fp.z*gcam.d.z;
			slightdir[i].z = fp.x*gcam.f.x + fp.y*gcam.f.y + fp.z*gcam.f.z;

			spotwid[i] = shadowtest2_light[i].spotwid;
		}
#if (USEGAMMAHACK == 0)
		f = 1.f;
#else
		f = 3072.f;
#endif
		g_qamb[0] = shadowtest2_ambrgb[0]*f;
		g_qamb[1] = shadowtest2_ambrgb[1]*f;
		g_qamb[2] = shadowtest2_ambrgb[2]*f;
		g_qamb[3] = 0.f;
		//eyepoln = 0; eyepolvn = 0;
	}
	else
	{
		if (lgs->numsects > glp->sectgotn)
		{
			if (glp->sectgotmal) free((void *)glp->sectgotmal);
			glp->sectgotn = ((lgs->numsects+127)&~127);
			glp->sectgotmal = (unsigned int *)malloc((glp->sectgotn>>3)+16); //NOTE:malloc doesn't guarantee 16-byte alignment!
			glp->sectgot = (unsigned int *)((((intptr_t)glp->sectgotmal)+15)&~15);
		}
		if (glp->lighasheadn <= 0)
		{
			glp->lighasheadn = LIGHASHSIZ;
			glp->lighashead = (int *)realloc(glp->lighashead,glp->lighasheadn*sizeof(glp->lighashead[0]));
			memset(glp->lighashead,-1,glp->lighasheadn*sizeof(glp->lighashead[0]));
		}
	}

	for(halfplane=0;halfplane<2;halfplane++) {

		if (!b->has_portal_clip)
			b->currenthalfplane = halfplane;
		else if (b->currenthalfplane != halfplane)
			continue;

		if (shadowtest2_rendmode == 4)
		{
			if (!halfplane) gcam.r.x = 1; else gcam.r.x = -1;
			gcam.d.x = 0; gcam.f.x = 0;
			gcam.r.y = 0; gcam.d.y = 0; gcam.f.y = -gcam.r.x;
			gcam.r.z = 0; gcam.d.z = 1; gcam.f.z = 0;
			xformprep(0.0, b);

			xformbac(-65536.0,-65536.0,1.0,&bord2[0], b);
			xformbac(+65536.0,-65536.0,1.0,&bord2[1], b);
			xformbac(+65536.0,+65536.0,1.0,&bord2[2], b);
			xformbac(-65536.0,+65536.0,1.0,&bord2[3], b);
			n = 4; didcut = 1;
		}
		else {
			xformprep(((double)halfplane)*PI, b);

			// NEW CODE - Use much larger bounds:
			float large_bound = 1e6f;
			xformbac(-large_bound, -large_bound, gcam.h.z, &bord[0],b);
			xformbac(+large_bound, -large_bound, gcam.h.z, &bord[1],b);
			xformbac(+large_bound, +large_bound, gcam.h.z, &bord[2],b);
			xformbac(-large_bound, +large_bound, gcam.h.z, &bord[3],b);

			//Clip screen to front plane
			n = 0; didcut = 0;
			for(i=4-1,j=0;j<4;i=j,j++)
			{
				if (bord[i].z >= SCISDIST) { bord2[n] = bord[i]; n++; }
				if ((bord[i].z >= SCISDIST) != (bord[j].z >= SCISDIST))
				{
					f = (SCISDIST-bord[i].z)/(bord[j].z-bord[i].z);
					bord2[n].x = (bord[j].x-bord[i].x)*f + bord[i].x;
					bord2[n].y = (bord[j].y-bord[i].y)*f + bord[i].y;
					bord2[n].z = (bord[j].z-bord[i].z)*f + bord[i].z;
					n++; didcut = 1;
				}
			}
			if (n < 3) break;
		}

		memset8(b->sectgot,0,(lgs->numsects+31)>>3);
		for(j=0;j<n;j++)
		{
			f = gcam.h.z/bord2[j].z;
			bord2[j].x = bord2[j].x*f + gcam.h.x;
			bord2[j].y = bord2[j].y*f + gcam.h.y;
		}



		if (!b->has_portal_clip) {
			//FIX! once means not each frame! (of course it doesn't hurt functionality)
			// Standard case: clear existing state and create new viewport
			for (i = mphnum - 1; i >= 0; i--) {
				mono_deloop(mph[i].head[1]);
				mono_deloop(mph[i].head[0]);
			}

			mono_genfromloop(&mph[0].head[0], &mph[0].head[1], bord2, n);
			mph[0].tag = gcam.cursect;
			mphnum = 1;
		} else {
			// dont do anything, because clipping is done by drawpol_befclip.
		}

		b->bunchn = 0; scansector(gcam.cursect,b);
		while (b->bunchn)
		{
			memset(b->bunchgot,0,(b->bunchn+7)>>3);

			for(i=b->bunchn-1;i>0;i--) //assume: bunchgrid[(((j-1)*j)>>1)+i] = bunchfront(j,i,0); is valid iff:{i<j}
			{

				for(k=(((i-1)*i)>>1),j=0;j<     i;k+=1,j++)
					if (b->bunchgrid[k]&2) goto nogood;
				for(k+=j            ,j++;j<b->bunchn;k+=j,j++)
					if (b->bunchgrid[k]&1) goto nogood;
				break;
nogood:; }
			closest = i;

			drawalls(closest,lgs,b);
		}

		if (shadowtest2_rendmode == 4) uptr = glp->sectgot;
										  else uptr = shadowtest2_sectgot;

		if (!halfplane)
		{
			memcpy(uptr,b->sectgot,(lgs->numsects+31)>>3);
		}
		else
		{
			if (false) // && !(cputype&(1<<25))) //Got SSE
			{
				for(i=((lgs->numsects+31)>>5)-1;i>=0;i--)
					uptr[i] |= b->sectgot[i];
			}
			else
			{
				// Convert to portable C - process 16 bytes at a time using uint64_t
				size_t total_bytes = ((lgs->numsects+127)&~127)>>3;
				size_t chunks = total_bytes / 16;

				// Process 16-byte chunks (2 x uint64_t)
				uint64_t* uptr64 = (uint64_t*)uptr;
				uint64_t* sectgot64 = (uint64_t*)b->sectgot;

				for(size_t j = 0; j < chunks; j++) {
					size_t idx = j * 2;
					uptr64[idx] |= sectgot64[idx];
					uptr64[idx + 1] |= sectgot64[idx + 1];
				}
			}
		}

		if (!didcut) break;
	}
}

static void draw_hsr_enter_portal( mapstate_t* map, int myport, bunchgrp *parentctx, int plothead0, int plothead1) {
	if (parentctx->recursion_depth >= MAX_PORTAL_DEPTH) {
		return;
	}
	cam_t ncam = parentctx->cam;
	int endp = portals[myport].destpn;
	int entry = portals[myport].anchorspri;

	int tgtspi = portals[endp].anchorspri;
	int ignw = portals[endp].surfid;
	int igns = portals[endp].sect;

	spri_t tgs = map->spri[tgtspi];
	spri_t ent = map->spri[entry];
	float dx = tgs.p.x - ent.p.x;
	float dy = tgs.p.y - ent.p.y;
	float dz = tgs.p.z - ent.p.z;
	ncam.p.x+=dx;
	ncam.p.y+=dy;
	ncam.p.z+=dz;
	ncam.cursect = portals[endp].sect;
//	ncam.f = s.f;
//	ncam.r = s.r;
//	ncam.d = s.d;


	bunchgrp newctx={};
	newctx.recursion_depth = parentctx->recursion_depth+1;
	newctx.cam = ncam;
	newctx.orcam = parentctx->orcam;

	newctx.has_portal_clip = true;
	newctx.portal_clip[0] = plothead0;
	newctx.portal_clip[1] = plothead1;
	newctx.sectgotn = 0;
	newctx.sectgot = 0;
	newctx.sectgotmal = 0;
	newctx.bunchgot=0;
	newctx.bunchn=0;
	newctx.bunchmal=0;
	newctx.bunchgrid =0;
	newctx.testignorewall = ignw;
	newctx.testignoresec = igns;
	newctx.gnewtagsect=-1;
	newctx.gnewtag=-1;
	newctx.currenthalfplane = parentctx->currenthalfplane;
	draw_hsr_polymost_ctx(map, &newctx);
}

typedef struct { int sect; point3d p; float rgb[3]; int useshadow; } drawkv6_lightpos_t;
void drawsprites ()
{
}

void shadowtest2_setcam (cam_t *ncam)
{
	//gcam = *ncam;
}

#if (USENEWLIGHT == 0)
typedef struct { float n2, d2, n1, d1, n0, d0, filler0[2], glk[12], bsc, gsc, rsc, filler1[1]; } hlighterp_t;
#else
typedef struct { float gk[16], gk2[12], bsc, gsc, rsc, filler1[1]; } hlighterp_t;
__declspec(align(16)) static const float hligterp_maxzero[4] = {0.f,0.f,0.f,0.f};
#endif
void prepligramp (float *ouvmat, point3d *norm, int lig, void *hl)
{

}


int shadowtest2_isgotsectintersect (int lignum)
{
	int i, leng;
	unsigned int *u0, *u1;

	leng = min(shadowtest2_sectgotn,shadowtest2_light[lignum].sectgotn);
	u0 = shadowtest2_sectgot;
	u1 = shadowtest2_light[lignum].sectgot;
	i = (leng>>5); if (u0[i]&u1[i]&((1<<leng)-1)) return(1); //WARNING:code uses x86-32 bit shift trick!
#if 0
	for(i--;i>=0;i--) if (u0[i]&u1[i]) return(1);
	return(0);
#else
	for(i--;((i&3)!=3);i--) if (u0[i]&u1[i]) return(1);
	if (i < 0) return(0);
	_asm
	{
		push esi
		mov eax, i
		mov ecx, u0
		mov edx, u1
		xorps xmm7, xmm7

begit:movaps xmm0, [ecx+eax*4-12]
		andps  xmm0, [edx+eax*4-12]
		cmpneqps xmm0, xmm7
		movmskps esi, xmm0
		test esi, esi
		jnz endit
		sub eax, 4
		jg short begit

		xor eax, eax
		jmp short skpit
endit:mov eax, 1
skpit:pop esi
	}
#endif
}

void shadowtest2_dellight (int i)
{
	if (shadowtest2_light[i].ligpolv   ) free((void *)shadowtest2_light[i].ligpolv   );
	if (shadowtest2_light[i].ligpol    ) free((void *)shadowtest2_light[i].ligpol    );
	if (shadowtest2_light[i].lighashead) free((void *)shadowtest2_light[i].lighashead);
	if (shadowtest2_light[i].sectgotmal) free((void *)shadowtest2_light[i].sectgotmal);
	shadowtest2_numlights--;
	shadowtest2_light[i] = shadowtest2_light[shadowtest2_numlights];
}

void shadowtest2_ligpolreset (int ind)
{
	lightpos_t *lp;
	int i, ie;

	if (ind >= 0) { i = ind; ie = ind+1; } else { i = 0; ie = shadowtest2_numlights; ligpolmaxvert = 0;/*FIX?*/ }

	for(;i<ie;i++)
	{
		lp = &shadowtest2_light[i];
		if (lp->lighasheadn <= 0)
		{
			lp->lighasheadn = LIGHASHSIZ;
			lp->lighashead = (int *)realloc(lp->lighashead,lp->lighasheadn*sizeof(lp->lighashead[0]));
		}
		memset(lp->lighashead,-1,lp->lighasheadn*sizeof(lp->lighashead[0]));
		lp->ligpoln = 0; lp->ligpolvn = 0;
	}
}

void shadowtest2_uninit ()
{

}

void shadowtest2_init ()
{
	shadowtest2_ligpolreset(-1);
}

//--------------------------------------------------------------------------------------------------

void drawpollig(int ei) {

}
#if 0
!endif
#endif
