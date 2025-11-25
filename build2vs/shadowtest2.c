#include "softrender.h"
#include "Core/monoclip.h"
#include "shadowtest2.h"

#include "build2.h"
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

#include "sysmain.h"
#include "drawpoly.h"
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

#ifdef STANDALONE
	//For debug only!
static int gcnt, curgcnt = 0x7fffffff, fixposx, fixposy;
#endif

	//Sorting
static unsigned int *sectgot = 0;      //WARNING:code uses x86-32 bit shift trick!
unsigned int *shadowtest2_sectgot = 0; //WARNING:code uses x86-32 bit shift trick!
static unsigned int *sectgotmal = 0, *shadowtest2_sectgotmal = 0;
static int sectgotn = 0, shadowtest2_sectgotn = 0;

static bunch_t *bunch = 0;
static unsigned int *bunchgot = 0;
static unsigned char *bunchgrid = 0;
static int bunchn, bunchmal = 0;

	//Translation & rotation
static mapstate_t *gst;
static player_transform *gps;
static cam_t gcam;
static point3d gnadd;
static double xformmat[9], xformmatc, xformmats;

	//Texture mapping parameters
static tile_t *gtpic;
static float gouvmat[9];
static int gcurcol;

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
extern int getwalls (int s, int w, vertlist_t *ver, int maxverts);
extern double distpoint2line2 (double x, double y, double x0, double y0, double x1, double y1);
extern int cputype;

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
extern void drawkv6_setup (drawkv6_frame_t *frame, tiletype *dd, intptr_t lzbufoff, dpoint3d *lipos, dpoint3d *lirig, dpoint3d *lidow, dpoint3d *lifor, float hx, float hy, float hz);
drawkv6_frame_t drawkv6_frame;

	//DRAWCONE.H:
extern void drawcone_setup (int, int, tiletype *, intptr_t,  point3d *,  point3d *,  point3d *,  point3d *, double, double, double);
extern void drawcone_setup (int, int, tiletype *, intptr_t, dpoint3d *, dpoint3d *, dpoint3d *, dpoint3d *, double, double, double);
extern void drawsph (double, double, double, double, int, double);
extern void drawcone (double, double, double, double, double, double, double, double, int, double, int);
#define DRAWCONE_NOCAP0 1
#define DRAWCONE_NOCAP1 2
#define DRAWCONE_FLAT0 4
#define DRAWCONE_FLAT1 8
#define DRAWCONE_CENT 16
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------

static int prepbunch (int b, bunchverts_t *twal)
{
	wall_t *wal;
	double f, x, y, x0, y0, x1, y1;
	int i, n;

	wal = gst->sect[bunch[b].sec].wall;
	i = bunch[b].wal0; twal[0].i = i;
	x0 = wal[i].x; y0 = wal[i].y; i += wal[i].n;
	x1 = wal[i].x; y1 = wal[i].y; f = bunch[b].fra0;
	twal[0].x = (x1-x0)*f + x0;
	twal[0].y = (y1-y0)*f + y0;
	if ((bunch[b].wal0 == bunch[b].wal1) && (bunch[b].fra0 < bunch[b].fra1))
	{     //Hack for left side clip
		f = bunch[b].fra1;
		twal[1].x = (x1-x0)*f + x0;
		twal[1].y = (y1-y0)*f + y0;
		return(1);
	}
	twal[1].x = x1;
	twal[1].y = y1; n = 1;
	while (i != bunch[b].wal1)
	{
		twal[n].i = i; n++; i += wal[i].n;
		twal[n].x = wal[i].x;
		twal[n].y = wal[i].y;
	}
	if (bunch[b].fra1 > 0.0)
	{
		x = wal[i].x; y = wal[i].y; f = bunch[b].fra1;
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
#define BFINTMAX 256
static bfint_t bfint[BFINTMAX];
static int bfintn, bfintlut[BFINTMAX+1];

	//See BUNCHFRONT2.KC for derivation.
	//Returns:
	//   0: NO OVERLAP
	//   1: FRONT:RED(b0)
	//   2: FRONT:GREEN(b1)
	//   3: UNSORTABLE!
static int bunchfront (int b0, int b1, int fixsplitnow)
{
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
	sec = gst->sect;
	s0 = bunch[b0].sec;
	s1 = bunch[b1].sec;
	if (s0 != s1)
	{
		for(w=sec[s0].n-1;w>=0;w--)
		{
			ns = sec[s0].wall[w].ns;
			nw = sec[s0].wall[w].nw;
			while (((unsigned)ns < (unsigned)gst->numsects) && (ns != s0))
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

	twal[0] = (bunchverts_t *)_alloca((gst->sect[bunch[b0].sec].n+gst->sect[bunch[b1].sec].n+2)*sizeof(bunchverts_t));
	twaln[0] = prepbunch(b0,twal[0]); twal[1] = &twal[0][twaln[0]+1];
	twaln[1] = prepbunch(b1,twal[1]);

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
	j = 0; gotsid = 0; startsid = -1; obfintn = bfintn;
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
					wal = gst->sect[bunch[b0].sec].wall; i = twal[0][ind[0]-1].i;
					x0 = wal[i].x-gcam.p.x; y0 = wal[i].y-gcam.p.y; i += wal[i].n;
					x1 = wal[i].x-gcam.p.x; y1 = wal[i].y-gcam.p.y;
					wal = gst->sect[bunch[b1].sec].wall; i = twal[1][ind[1]-1].i;
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

					if ((fixsplitnow) && (bfintn < BFINTMAX))
					{
						bfint[bfintn].bun = b1;
						bfint[bfintn].sid = startsid+1; startsid ^= 1;
						bfint[bfintn].wal = twal[0][ind[0]-1].i;
						bfint[bfintn].fra = t;
						bfintn++;
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

static void scansector (int sectnum)
{
	#define BUNCHNEAR 1e-7
	sect_t *sec;
	wall_t *wal;
	bfint_t tbf;
	double f, dx0, dy0, dx1, dy1, f0, f1;
	int i, j, k, m, o, ie, obunchn, realobunchn, obfintn;

	if (sectnum < 0) return;
	sectgot[sectnum>>5] |= (1<<sectnum);

	sec = &gst->sect[sectnum]; wal = sec->wall;

	obunchn = bunchn; realobunchn = bunchn;
	for(i=0,ie=sec->n;i<ie;i++)
	{
		j = wal[i].n+i;
		dx0 = wal[i].x-gcam.p.x; dy0 = wal[i].y-gcam.p.y;
		dx1 = wal[j].x-gcam.p.x; dy1 = wal[j].y-gcam.p.y; if (dy1*dx0 <= dx1*dy0) goto docont; //Back-face cull

			//clip to near plane .. result is parametric fractions f0&f1
		f0 = dx0*xformmatc + dy0*xformmats;
		f1 = dx1*xformmatc + dy1*xformmats;
			  if (f0 <= BUNCHNEAR) { if (f1 <= BUNCHNEAR) goto docont;
											 f0 = (BUNCHNEAR-f0)/(f1-f0); f1 = 1.0; if (f0 >= f1) goto docont; }
		else if (f1 <= BUNCHNEAR) { f1 = (BUNCHNEAR-f0)/(f1-f0); f0 = 0.0; if (f0 >= f1) goto docont; }
		else                      { f0 = 0.0;                    f1 = 1.0; }

		k = bunch[bunchn-1].wal1;
		if ((bunchn > obunchn) && (wal[k].n+k == i) && (bunch[bunchn-1].fra1 == 1.0))
		{
			bunch[bunchn-1].wal1 = i; //continue from previous wall (typical case)
			bunch[bunchn-1].fra1 = f1;
			if ((bunchn-1 > obunchn) && (bunch[obunchn].wal0 == j) && (bunch[obunchn].fra0 == 0.0))
			{
				bunchn--; //attach to left side of 1st bunch on loop
				bunch[obunchn].wal0 = bunch[bunchn].wal0;
				bunch[obunchn].fra0 = bunch[bunchn].fra0;
			}
		}
		else if ((bunchn > obunchn) && (bunch[obunchn].wal0 == j) && (bunch[obunchn].fra0 == 0.0))
		{
			bunch[obunchn].wal0 = i; //update left side of 1st bunch on loop
			bunch[obunchn].fra0 = f0;
		}
		else
		{
			if (bunchn >= bunchmal)
			{
				bunchmal <<= 1;
				bunch     = (bunch_t       *)realloc(bunch    ,bunchmal*sizeof(bunch[0]));
				bunchgot  = (unsigned int  *)realloc(bunchgot ,((bunchmal+31)&~31)>>3);
				bunchgrid = (unsigned char *)realloc(bunchgrid,((bunchmal-1)*bunchmal)>>1);
			}
			bunch[bunchn].wal0 = i; bunch[bunchn].fra0 = f0; //start new bunch
			bunch[bunchn].wal1 = i; bunch[bunchn].fra1 = f1;
			bunch[bunchn].sec = sectnum; bunchn++;
		}
docont:;
		if (j < i) obunchn = bunchn;
	}

	for(obunchn=realobunchn;obunchn<bunchn;obunchn++)
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
		j = (((obunchn-1)*obunchn)>>1); bfintn = 0;
		for(i=0;i<obunchn;i++) bunchgrid[j+i] = bunchfront(obunchn,i,1);

		if (!bfintn) continue;

			//sort bfint's
		for(j=1;j<bfintn;j++)
			for(i=0;i<j;i++)
			{
					//              bfint[i].wal vs. bfint[j].wal ?
					//0    bunch[obunchn].wal0........bunch[obunchn].wal1       sec->n
					//0....bunch[obunchn].wal1        bunch[obunchn].wal0.......sec->n
				m = bfint[i].wal; o = bfint[j].wal;
				if (bunch[obunchn].wal0 > bunch[obunchn].wal1) //handle wall index wrap-around
				{
					if (m <= bunch[obunchn].wal1) m += sec->n;
					if (o <= bunch[obunchn].wal1) o += sec->n;
				}
				if (m < o) continue;
				if ((bfint[i].wal == bfint[j].wal) && (bfint[i].fra <= bfint[j].fra)) continue;

				tbf = bfint[i]; bfint[i] = bfint[j]; bfint[j] = tbf;
			}

			//combine null or tiny bunches
		obfintn = bfintn; bfintlut[0] = 0; bfintn = 1;
		for(i=1;i<obfintn;i++)
			if ((bfint[i-1].wal != bfint[i].wal) || (bfint[i].fra-bfint[i-1].fra >= 2e-7))
				{ bfintlut[bfintn] = i; bfintn++; }
		bfintlut[bfintn] = obfintn;

#if 0
		if (sectnum == 0)
		{
			char tbuf[2048];
			sprintf(tbuf,"bef (bunchn=%d obunchn=%d bfintn=%d)\n",bunchn,obunchn,bfintn);
			for(m=0;m<bunchn;m++)
			{
				sprintf(&tbuf[strlen(tbuf)],"%d (%2d: %2d %15.12f %2d %15.12f) : ",m,bunch[m].sec,bunch[m].wal0,bunch[m].fra0,bunch[m].wal1,bunch[m].fra1);
				for(k=0;k<m;k++) sprintf(&tbuf[strlen(tbuf)],"%d ",bunchgrid[(((m-1)*m)>>1)+k]);
				sprintf(&tbuf[strlen(tbuf)],"\n");
			}
			MessageBox(ghwnd,tbuf,prognam,MB_OK);
		}
#endif

			//obunchn gets its ass split 'bfintn' times into a total of 'bfintn+1' pieces
		if (bunchn+bfintn > bunchmal)
		{
			bunchmal = max(bunchmal<<1,bunchn+bfintn);
			bunch     = (bunch_t       *)realloc(bunch    ,bunchmal*sizeof(bunch[0]));
			bunchgot  = (unsigned int  *)realloc(bunchgot ,((bunchmal+31)&~31)>>3);
			bunchgrid = (unsigned char *)realloc(bunchgrid,((bunchmal-1)*bunchmal)>>1);
		}

			//Shove not-yet-processed neighbors to end of list. WARNING:be careful with indices/for loop order!
		for(k=0;k<bfintn;k++) bunch[bunchn+bfintn-1-k] = bunch[obunchn+k+1];
		for(k=bfintn-1;k>=0;k--) bunch[obunchn+k+1] = bunch[obunchn];
		for(k=bfintn-1;k>=0;k--)
		{
			bunch[obunchn+k  ].wal1 = bfint[bfintlut[k  ]  ].wal; bunch[obunchn+k  ].fra1 = max(bfint[bfintlut[k  ]  ].fra-1e-7,0.0);
			bunch[obunchn+k+1].wal0 = bfint[bfintlut[k+1]-1].wal; bunch[obunchn+k+1].fra0 = min(bfint[bfintlut[k+1]-1].fra+1e-7,1.0);
		}
		bunchn += bfintn;

			//  0 1 2 3 4 5 6
			//0
			//1 x
			//2 x x
			//3 x x x
			//4 x x x x
			//5 ? ? ? - ?
			//6 ? ? ? - ? 0
			//7 ? ? ? - ? 0 0
		for(m=obunchn;m<obunchn+bfintn+1;m++) //re-front all 'bfintn+1' pieces, using hints from bfint list
		{
			j = (((m-1)*m)>>1);
			for(k=0;k<obunchn;k++)
			{
				if (m > obunchn       ) for(o=bfintlut[m-obunchn-1];o<bfintlut[m-obunchn  ];o++) if (bfint[o].bun == k) { bunchgrid[j+k] = bfint[o].sid  ; goto bunchgrid_got; }
				if (m < obunchn+bfintn) for(o=bfintlut[m-obunchn  ];o<bfintlut[m-obunchn+1];o++) if (bfint[o].bun == k) { bunchgrid[j+k] = bfint[o].sid^3; goto bunchgrid_got; }
				bunchgrid[j+k] = bunchfront(m,k,0);
bunchgrid_got:;
			}
			for(;k<m;k++) bunchgrid[j+k] = 0;
		}

#if 0
		if (sectnum == 0)
		{
			char tbuf[2048];
			sprintf(tbuf,"aft (bunchn=%d obunchn=%d bfintn=%d)\n",bunchn,obunchn,bfintn);
			for(m=0;m<bunchn;m++)
			{
				sprintf(&tbuf[strlen(tbuf)],"%d (%2d: %2d %15.12f %2d %15.12f) : ",m,bunch[m].sec,bunch[m].wal0,bunch[m].fra0,bunch[m].wal1,bunch[m].fra1);
				for(k=0;k<m;k++) sprintf(&tbuf[strlen(tbuf)],"%d ",bunchgrid[(((m-1)*m)>>1)+k]);
				sprintf(&tbuf[strlen(tbuf)],"\n");
			}
			MessageBox(ghwnd,tbuf,prognam,MB_OK);
		}
#endif

		obunchn += bfintn;
	}

		//remove null bunches (necessary for proper operation)
	for(m=bunchn-1;m>=realobunchn;m--)
	{
		if (bunch[m].wal0 != bunch[m].wal1) continue;
		if (bunch[m].fra0 < bunch[m].fra1) continue;
		bunchn--; bunch[m] = bunch[bunchn];
		j = (((bunchn-1)*bunchn)>>1);
		memcpy(&bunchgrid[((m-1)*m)>>1],&bunchgrid[j],m*sizeof(bunchgrid[0]));
		for(i=m+1;i<bunchn;i++) bunchgrid[(((i-1)*i)>>1)+m] = ((bunchgrid[j+i]&1)<<1) + (bunchgrid[j+i]>>1);
	}

}

static void xformprep (double hang)
{
	double f; f = atan2(gcam.f.y,gcam.f.x)+hang; //WARNING: "f = 1/sqrt; c *= f; s *= f;" form has singularity - don't use :/
	xformmatc = cos(f); xformmats = sin(f);
	xformmat[0] = gcam.r.y*xformmatc - gcam.r.x*xformmats; xformmat[1] = gcam.r.z; xformmat[2] = gcam.r.x*xformmatc + gcam.r.y*xformmats;
	xformmat[3] = gcam.d.y*xformmatc - gcam.d.x*xformmats; xformmat[4] = gcam.d.z; xformmat[5] = gcam.d.x*xformmatc + gcam.d.y*xformmats;
	xformmat[6] = 0                                      ; xformmat[7] = gcam.f.z; xformmat[8] = gcam.f.x*xformmatc + gcam.f.y*xformmats;
	gnadd.x = -gcam.h.x*xformmat[0] - gcam.h.y*xformmat[1] + gcam.h.z*xformmat[2];
	gnadd.y = -gcam.h.x*xformmat[3] - gcam.h.y*xformmat[4] + gcam.h.z*xformmat[5];
	gnadd.z = -gcam.h.x*xformmat[6] - gcam.h.y*xformmat[7] + gcam.h.z*xformmat[8];
}

static void xformbac (double rx, double ry, double rz, dpoint3d *o)
{
	o->x = rx*xformmat[0] + ry*xformmat[3];//+ rz*xformmat[6];
	o->y = rx*xformmat[1] + ry*xformmat[4] + rz*xformmat[7];
	o->z = rx*xformmat[2] + ry*xformmat[5] + rz*xformmat[8];
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

			overt[on].x = mp[i].x*xformmat[0] + mp[i].y*xformmat[1] + gnadd.x;
			overt[on].y = mp[i].x*xformmat[3] + mp[i].y*xformmat[4] + gnadd.y;
			overt[on].z = mp[i].x*xformmat[6] + mp[i].y*xformmat[7] + gnadd.z;
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
	drawpoly(&gtt,(vertyp *)vert,n,gcurcol,(((unsigned)gcurcol)>>24)/16.0,gouvmat,i);

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

static eyepol_t *eyepol = 0; // 4096 eyepol_t's = 192KB
static point2d *eyepolv = 0; //16384 point2d's  = 128KB
int eyepoln = 0, glignum = 0;
static int eyepolmal = 0, eyepolvn = 0, eyepolvmal = 0;
static int gligsect, gligwall, gligslab, gflags;
static point3d gnorm;
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

	i = RENDFLAGS_OUVMAT|RENDFLAGS_NODEPTHTEST|RENDFLAGS_NOTRCP|RENDFLAGS_GMAT;
	if (renderinterp) i |= RENDFLAGS_INTERP;
	drawpoly_flat_threadsafe(&eyepol[ind].tpic->tt,(vertyp *)vert,n,eyepol[ind].curcol,(((unsigned)eyepol[ind].curcol)>>24)/16.0,eyepol[ind].ouvmat,i,gcam);

	if (shadowtest2_rendmode == 1)
	{
		for(i=n-1,j=0;i>=0;j=i,i--) drawline2d(&gcam.c,vert[i].x,vert[i].y,vert[j].x,vert[j].y,0xa0a0a0); //WARNING:fusing this with centroid algo below fails.. compiler bug?
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
													  drawpix(&gcam.c,i+0,j-1,0xffffff); drawpix(&gcam.c,i+1,j-1,0xffffff);
		drawpix(&gcam.c,i-1,j+0,0xffffff); drawpix(&gcam.c,i+0,j+0,0x000000); drawpix(&gcam.c,i+1,j+0,0x000000); drawpix(&gcam.c,i+2,j+0,0xffffff);
		drawpix(&gcam.c,i-1,j+1,0xffffff); drawpix(&gcam.c,i+0,j+1,0x000000); drawpix(&gcam.c,i+1,j+1,0x000000); drawpix(&gcam.c,i+2,j+1,0xffffff);
													  drawpix(&gcam.c,i+0,j+2,0xffffff); drawpix(&gcam.c,i+1,j+2,0xffffff);
	}
}

static void gentex_xform(float *f);
/*
	Purpose: Renders visible geometry polygons to screen
	Converts 3D polygon vertices to 2D screen coordinates
	Handles texture mapping setup (UV coordinates, skybox mapping)
	Stores polygons in eyepol[] array for later rendering
	Manages different rendering modes (walls, skybox, parallax sky)
 */
static void drawtagfunc (int rethead0, int rethead1)
{
	float f, g, *fptr;
	int i, j, k, h, rethead[2];

	if ((rethead0|rethead1) < 0) { mono_deloop(rethead1); mono_deloop(rethead0); return; }
	rethead[0] = rethead0; rethead[1] = rethead1;

#if 0
		//Old code to draw polygon immediately:
	for(i=2-1;i>=0;i--) { k = rethead[i]; do { print6x8(&gcam.c,mp[k].x-3,mp[k].y-5,0xffffff,-1,"x"); k = mp[k].n; } while (k != rethead[i]); }
	drawpol_aftclip(rethead0,rethead1); for(i=2-1;i>=0;i--) mono_deloop(rethead[i]); return;
#endif

		//Put on FIFO:
	for(h=0;h<2;h++)
	{
		i = rethead[h]; //k = eyepolvn;
		do
		{
			if (h) i = mp[i].p;

			if (eyepolvn >= eyepolvmal)
			{
				eyepolvmal = max(eyepolvmal<<1,16384); //den_01.map room of death uses ~15000 max
				eyepolv = (point2d *)realloc(eyepolv,eyepolvmal*sizeof(point2d));
			}

			f =          gcam.h.z/(/*mp[i].x*xformmat[6]*/ + mp[i].y*xformmat[7] + gnadd.z);
			eyepolv[eyepolvn].x = (mp[i].x*xformmat[0] + mp[i].y*xformmat[1] + gnadd.x)*f + gcam.h.x;
			eyepolv[eyepolvn].y = (mp[i].x*xformmat[3] + mp[i].y*xformmat[4] + gnadd.y)*f + gcam.h.y;

			eyepolvn++;

			if (!h) i = mp[i].n;
		} while (i != rethead[h]);
		mono_deloop(rethead[h]);
	}

	if (eyepoln+1 >= eyepolmal)
	{
		eyepolmal = max(eyepolmal<<1,4096); //den_01.map room of death uses ~ 3100 max
		eyepol = (eyepol_t *)realloc(eyepol,eyepolmal*sizeof(eyepol_t));
		eyepol[0].vert0 = 0;
	}

	if (gflags < 2)
		memcpy((void *)eyepol[eyepoln].ouvmat,(void *)gouvmat,sizeof(gouvmat[0])*9);
	else
	{
		f = (((float)gtpic->tt.x)+1.15f)/((float)gtpic->tt.x); fptr = eyepol[eyepoln].ouvmat;
		switch(gflags)
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
		gentex_xform(fptr);
	}

	eyepol[eyepoln].tpic = gtpic;
	eyepol[eyepoln].curcol = gcurcol;
	eyepol[eyepoln].flags = (gflags != 0);
	eyepol[eyepoln].b2sect = gligsect;
	eyepol[eyepoln].b2wall = gligwall;
	eyepol[eyepoln].b2slab = gligslab;
	memcpy((void *)&eyepol[eyepoln].norm,(void *)&gnorm,sizeof(gnorm));
	eyepoln++;
	eyepol[eyepoln].vert0 = eyepolvn;
}
/*
	Purpose: Renders skybox faces as background
	Generates 6 cube faces for skybox rendering
	Clips each face against view frustum
	Projects cube vertices to screen space
	Calls drawtagfunc for each visible skybox face
	Uses special texture mapping flags (gflags = 10-15 for different cube faces)
*/
static void skytagfunc (int rethead0, int rethead1)
{
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
			otp[i].x = oy*xformmatc - ox*xformmats; otp[i].y = cubeverts[p][i][2];
			otp[i].z = ox*xformmatc + oy*xformmats;
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

		gflags = p+10;
		mono_bool(rethead0,rethead1,plothead[0],plothead[1],MONO_BOOL_AND,drawtagfunc);
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
static void ligpoltagfunc (int rethead0, int rethead1)
{
	float f, fx, fy, fz;
	int i, j, rethead[2];

	if ((rethead0|rethead1) < 0) { mono_deloop(rethead1); mono_deloop(rethead0); return; }

		//Use this for dynamic lights only! (doesn't seem to help speed much)
	//if ((shadowtest2_rendmode == 4) && (!(shadowtest2_sectgot[gligsect>>5]&(1<<gligsect)))) return;

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

			f = gcam.h.z/(/*mp[i].x*xformmat[6]*/ + mp[i].y*xformmat[7] + gnadd.z);
			fx        =  (mp[i].x*xformmat[0] + mp[i].y*xformmat[1] + gnadd.x)*f + gcam.h.x;
			fy        =  (mp[i].x*xformmat[3] + mp[i].y*xformmat[4] + gnadd.y)*f + gcam.h.y;

#if (USEINTZ)
			f = 1.0/((gouvmat[0]*fx + gouvmat[3]*fy + gouvmat[6])*1048576.0*256.0);
#else
			f = 1.0/((gouvmat[0]*fx + gouvmat[3]*fy + gouvmat[6])*gcam.h.z);
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
	glp->ligpol[glp->ligpoln].b2sect = gligsect;
	glp->ligpol[glp->ligpoln].b2wall = gligwall;
	glp->ligpol[glp->ligpoln].b2slab = gligslab;
	i = lighash(gligsect,gligwall,gligslab);
	glp->ligpol[glp->ligpoln].b2hashn = glp->lighashead[i]; glp->lighashead[i] = glp->ligpoln;
	ligpolmaxvert = max(ligpolmaxvert,glp->ligpolvn-glp->ligpol[glp->ligpoln].vert0);
	glp->ligpoln++;
	glp->ligpol[glp->ligpoln].vert0 = glp->ligpolvn;
}

static int gnewtag, gdoscansector;
/*
	Purpose: Portal traversal and sector visibility
	Manages sector-to-sector transitions through portals
	Updates visibility lists when moving between connected areas
	Handles clipping of view regions as camera moves through world
	Maintains mph[] (mono polygon hierarchy) for spatial partitioning
	"tag" refers to sector IDs
*/
static void changetagfunc (int rethead0, int rethead1)
{
	if ((rethead0|rethead1) < 0) return;

	if ((gdoscansector) && (!(sectgot[gnewtag>>5]&(1<<gnewtag)))) scansector(gnewtag);

	mph_check(mphnum);
	mph[mphnum].head[0] = rethead0;
	mph[mphnum].head[1] = rethead1;
	mph[mphnum].tag = gnewtag;
	mphnum++;
}

	//flags&1: do and
	//flags&2: do sub
	//flags&4: reverse cut for sub
static void drawpol_befclip (int tag, int newtag, int plothead0, int plothead1, int flags)
{
	#define BSCISDIST 0.000001 //Reduces probability of glitch further
	//#define BSCISDIST 0.0001 //Gaps undetectable
	//#define BSCISDIST 0.1 //Huge gaps
	void (*mono_output)(int h0, int h1);
	dpoint3d *otp, *tp;
	double f, ox, oy, oz;
	int i, j, k, l, h, on, n, plothead[2], imin, imax, i0, i1, omph0, omph1;

	if ((plothead0|plothead1) < 0) return;
	plothead[0] = plothead0; plothead[1] = plothead1;

#if 0
	for(h=0;h<2;h++)
	{
		i = plothead[h]; j = 0;
		do
		{
			float ox2, oy2, oz2;

			if (h) i = mp[i].p;

			ox = mp[i].x-gcam.p.x; oy = mp[i].y-gcam.p.y;
			ox2 = oy*xformmatc - ox*xformmats; oy2 = mp[i].z-gcam.p.z;
			oz2 = ox*xformmatc + oy*xformmats;

			if (oz2 > BSCISDIST)
			{
				f = gcam.h.z/oz2;
				ox2 = ox2*f + gcam.h.x;
				oy2 = oy2*f + gcam.h.y;
				print6x8((tiltyp *)&gdd,ox2-3,oy2-4,0xffffff,-1,"o");
			}
			j++;

			if (!h) i = mp[i].n;
		} while (i != plothead[h]);

		print6x8((tiltyp *)&gdd,fixposx+h*8,fixposy,0xffffff,-1,"%d",j);
	}
	print6x8((tiltyp *)&gdd,fixposx+24,fixposy,0xffffff,-1,"%d->%d",tag,newtag);
	fixposy += 8;
#endif

	n = 2; for(h=0;h<2;h++) for(i=mp[plothead[h]].n;i!=plothead[h];i=mp[i].n) n++;
	otp = (dpoint3d *)_alloca(n*sizeof(dpoint3d));
	tp = (dpoint3d *)_alloca(n*sizeof(dpoint3d)*2);

		//rotate, converting vmono to simple point3d loop
	on = 0;
	for(h=0;h<2;h++)
	{
		i = plothead[h];
		do
		{
			if (h) i = mp[i].p;

			ox = mp[i].x-gcam.p.x; oy = mp[i].y-gcam.p.y;
			otp[on].x = oy*xformmatc - ox*xformmats; otp[on].y = mp[i].z-gcam.p.z;
			otp[on].z = ox*xformmatc + oy*xformmats; on++;

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
	mono_genfromloop(&plothead[0],&plothead[1],tp,n);
	if ((plothead[0]|plothead[1]) < 0) { mono_deloop(plothead[0]); mono_deloop(plothead[1]); return; }

#ifdef STANDALONE
	if ((gcnt <= 1) && (keystatus[0x38]))
	{
#if 1
		drawpol_aftclip(plothead[0],plothead[1]);
#endif
#if 1
		print6x8(&gcam.c,fixposx,fixposy,0xffffff,0,"%d -> %d",tag,newtag); fixposy += 8; if (fixposy >= gdd.y) { fixposy = 0; fixposx += 256; }
		for(j=0;j<mphnum;j++)
		{
			if (mph[j].tag != tag) continue;
			for(h=0;h<2;h++)
			{
				i = mph[j].head[h];
				do
				{
					print6x8(&gcam.c,fixposx,fixposy,0xff8000,0,"   %6.2f %6.2f",mp[i].x,mp[i].y); fixposy += 8; if (fixposy >= gdd.y) { fixposy = 0; fixposx += 256; }
					i = mp[i].n;
				} while (i != mph[j].head[h]);
				fixposy += 8;
			}
		}
		for(h=0;h<2;h++)
		{
			//for(i=mp[plothead[h]].n;i!=plothead[h];i=mp[i].n) drawline2d(&gcam.c,mp[mp[i].p].x,mp[mp[i].p].y,mp[i].x,mp[i].y,0xffffff);
			i = plothead[h];
			do
			{
				if (keystatus[0x36]) if ((fabs(mp[i].x) < 1e-12) && (mp[i].y > 0)) mp[i].x = 0;
				print6x8(&gcam.c,fixposx,fixposy,0xffffff,0,"   %6.2f %6.2f",mp[i].x,mp[i].y); fixposy += 8; if (fixposy >= gdd.y) { fixposy = 0; fixposx += 256; }
				i = mp[i].n;
			} while (i != plothead[h]);
			fixposy += 8;
		}
#endif
		for(i=2-1;i>=0;i--) mono_deloop(plothead[i]);
		return;
	}
#endif

	if (flags&1)
	{
		if (newtag >= 0)
		{
			gnewtag = newtag; gdoscansector = 1; omph0 = mphnum;
			for(i=mphnum-1;i>=0;i--) if (mph[i].tag == tag) mono_bool(mph[i].head[0],mph[i].head[1],plothead[0],plothead[1],MONO_BOOL_AND,changetagfunc); //follow portal

#ifdef STANDALONE
			if (!keystatus[0x1d])
#endif
			{
				for(l=omph0;l<mphnum;l++)
				{
					mph[omph0] = mph[l]; k = omph0; omph0++;
					for(j=omph0-1;j>=0;j--) //Join monos
					{
						if (mph[j].tag != gnewtag) continue;
						if (!mono_join(mph[j].head[0],mph[j].head[1],mph[k].head[0],mph[k].head[1],&i0,&i1)) continue;
						for(i=2-1;i>=0;i--) { mono_deloop(mph[k].head[i]); mono_deloop(mph[j].head[i]); }
						omph0--; mph[k] = mph[omph0];
						mph[j].head[0] = i0; mph[j].head[1] = i1; k = j;
					}
				}
				mphnum = omph0;
			}
		}
		else
		{
				  if (shadowtest2_rendmode == 4) mono_output = ligpoltagfunc; //add to light list // this will process point lights. otherwize will only use plr light.
			else if (gflags < 2)                mono_output =   drawtagfunc; //draw wall
			else                                mono_output =    skytagfunc; //calls drawtagfunc inside
			for(i=mphnum-1;i>=0;i--) if (mph[i].tag == tag) mono_bool(mph[i].head[0],mph[i].head[1],plothead[0],plothead[1],MONO_BOOL_AND,mono_output);
		}
	}
	if (flags&2)
	{
		if (!(flags&4)) j = MONO_BOOL_SUB;
					  else j = MONO_BOOL_SUB_REV;

		gnewtag = tag; gdoscansector = 0; omph0 = mphnum; omph1 = mphnum;
		for(i=mphnum-1;i>=0;i--)
		{
			if (mph[i].tag != tag) continue;
			mono_bool(mph[i].head[0],mph[i].head[1],plothead[0],plothead[1],j,changetagfunc);
			mono_deloop(mph[i].head[1]);
			mono_deloop(mph[i].head[0]);

			omph0--; mph[i] = mph[omph0];
		}

			//valid mph's stored in 2 blocks: (0<=?<omph0), (omph1<=?<mphnum)
#ifdef STANDALONE
		if (!keystatus[0x1d])
		{
#endif
			for(l=omph1;l<mphnum;l++)
			{
				mph[omph0] = mph[l]; k = omph0; omph0++;
				for(j=omph0-1;j>=0;j--) //Join monos
				{
					if (mph[j].tag != gnewtag) continue;
					if (!mono_join(mph[j].head[0],mph[j].head[1],mph[k].head[0],mph[k].head[1],&i0,&i1)) continue;
					for(i=2-1;i>=0;i--) { mono_deloop(mph[k].head[i]); mono_deloop(mph[j].head[i]); }
					omph0--; mph[k] = mph[omph0];
					mph[j].head[0] = i0; mph[j].head[1] = i1; k = j;
				}
			}
#ifdef STANDALONE
		}
		else
		{
			for(l=omph1;l<mphnum;l++) { mph[omph0] = mph[l]; k = omph0; omph0++; }
		}
#endif
		mphnum = omph0;

	}
	mono_deloop(plothead[1]);
	mono_deloop(plothead[0]);
}



	//FIXFIXFIX: clean this up!
static void gentex_xform (float *ouvmat)
{
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
	ay = ((float)gtpic->tt.x)*f;
	az = ((float)gtpic->tt.y)*f;
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

static void gentex_sky (surf_t *sur)
{
	float f, g, h;
	int i;

	if (gflags >= 2) return; //if texture is skybox, return early

		//Crappy paper sky :/
	h = 65536;
	f = atan2(gcam.f.y,gcam.f.x)         *-h/PI*2.f;
	g = asin(min(max(gcam.f.z,-1.f),1.f))*-h/PI*2.f;
	gouvmat[0] = sur->uv[0].x*h + f; gouvmat[3] = sur->uv[1].x*h; gouvmat[6] = sur->uv[2].x*h;
	gouvmat[1] = sur->uv[0].y*h + g; gouvmat[4] = sur->uv[1].y*h; gouvmat[7] = sur->uv[2].y*h;
	gouvmat[2] =              h    ; gouvmat[5] =            0.f; gouvmat[8] =            0.f;
	gentex_xform(gouvmat);
}

static void gentex_ceilflor (sect_t *sec, wall_t *wal, surf_t *sur, int isflor)
{
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
		gouvmat[i+0] = ox*gcam.r.x + oy*gcam.r.y + oz*gcam.r.z;
		gouvmat[i+1] = ox*gcam.d.x + oy*gcam.d.y + oz*gcam.d.z;
		gouvmat[i+2] = ox*gcam.f.x + oy*gcam.f.y + oz*gcam.f.z;
	}

	for(i=9-1;i>=0;i--) gouvmat[i] *= 256.f;

	gouvmat[3] -= gouvmat[0]; gouvmat[4] -= gouvmat[1]; gouvmat[5] -= gouvmat[2];
	gouvmat[6] -= gouvmat[0]; gouvmat[7] -= gouvmat[1]; gouvmat[8] -= gouvmat[2];

	gentex_xform(gouvmat);
}

static void gentex_wall (kgln_t *npol2, surf_t *sur)
{
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
		//npol2[i].u = (gouvmat[1]*sx + gouvmat[4]*sy + gouvmat[7])/(gouvmat[0]*sx + gouvmat[3]*sy + gouvmat[6])
		//npol2[i].v = (gouvmat[2]*sx + gouvmat[5]*sy + gouvmat[8])/(gouvmat[0]*sx + gouvmat[3]*sy + gouvmat[6])
		//npol2[i].z =                                            1/(gouvmat[0]*sx + gouvmat[3]*sy + gouvmat[6])
		//   Solve ^ for gouvmat[*]
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
	gouvmat[6] = fk[12] + fk[13] + fk[14];
	gouvmat[0] = fk[18] + fk[19] + fk[20];
	gouvmat[3] = fk[21] + fk[22] + fk[23];

	fk[15] = gouvmat[6]*fk[0] + gouvmat[0]*fk[3] + gouvmat[3]*fk[6];
	fk[16] = gouvmat[6]*fk[1] + gouvmat[0]*fk[4] + gouvmat[3]*fk[7];
	fk[17] = gouvmat[6]*fk[2] + gouvmat[0]*fk[5] + gouvmat[3]*fk[8];

	fk[ 9] = fk[15]*npol2[0].u;
	fk[10] = fk[16]*npol2[1].u;
	fk[11] = fk[17]*npol2[2].u;
	gouvmat[7] = fk[12]*fk[9] + fk[13]*fk[10] + fk[14]*fk[11];
	gouvmat[1] = fk[18]*fk[9] + fk[19]*fk[10] + fk[20]*fk[11];
	gouvmat[4] = fk[21]*fk[9] + fk[22]*fk[10] + fk[23]*fk[11];

	fk[ 9] = fk[15]*npol2[0].v;
	fk[10] = fk[16]*npol2[1].v;
	fk[11] = fk[17]*npol2[2].v;
	gouvmat[8] = fk[12]*fk[9] + fk[13]*fk[10] + fk[14]*fk[11];
	gouvmat[2] = fk[18]*fk[9] + fk[19]*fk[10] + fk[20]*fk[11];
	gouvmat[5] = fk[21]*fk[9] + fk[22]*fk[10] + fk[23]*fk[11];

	rdet = 1.0/(fk[0]*fk[12] + fk[1]*fk[13] + fk[2]*fk[14]);

#if (USEINTZ)
	g = gcam.h.z*rdet/(1048576.0*256.0);
#else
	g = rdet;
#endif
													 gouvmat[0] *= g; gouvmat[3] *= g; gouvmat[6] *= g; g *= rdet*65536.0;
	f = (float)gtpic->tt.x*g;            gouvmat[1] *= f; gouvmat[4] *= f; gouvmat[7] *= f;
	f = (float)gtpic->tt.y*g;            gouvmat[2] *= f; gouvmat[5] *= f; gouvmat[8] *= f;

	if (renderinterp)
	{
		gouvmat[1] -= gouvmat[0]*32768.0; gouvmat[2] -= gouvmat[0]*32768.0;
		gouvmat[4] -= gouvmat[3]*32768.0; gouvmat[5] -= gouvmat[3]*32768.0;
		gouvmat[7] -= gouvmat[6]*32768.0; gouvmat[8] -= gouvmat[6]*32768.0;
	}
}
/*
the mono engine produces camera-space polygons that are clipped to not overlap.
The plothead[0] and plothead[1] contain monotone polygon pairs representing
the final visible geometry ready for 2D projection.
The b parameter is a bunch index - this function processes one "bunch" (visible sector group) at a time. The traversal logic is in the caller that:
*/

static void drawalls (int b)
{
	// === VARIABLE DECLARATIONS ===
	extern void loadpic (tile_t *);
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
	s = bunch[b].sec;
	sec = gst->sect;

	wal = sec[s].wall;
#if 0
	i = bunch[b].wal0;
	x0 = wal[i].x; y0 = wal[i].y; i += wal[i].n;
	x1 = wal[i].x; y1 = wal[i].y; f = bunch[b].fra0;
	twalx0 = (x1-x0)*f + x0;
	twaly0 = (y1-y0)*f + y0;

	i = bunch[b].wal1;
	x0 = wal[i].x; y0 = wal[i].y; i += wal[i].n;
	x1 = wal[i].x; y1 = wal[i].y; f = bunch[b].fra1;
	twalx1 = (x1-x0)*f + x0;
	twaly1 = (y1-y0)*f + y0;
#endif

	twal = (bunchverts_t *)_alloca((gst->sect[bunch[b].sec].n+1)*sizeof(bunchverts_t));
	twaln = prepbunch(b,twal);
	gligsect = s; gligslab = 0;

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
	bunchn--; bunch[b] = bunch[bunchn];
	j = (((bunchn-1)*bunchn)>>1);
	memcpy(&bunchgrid[((b-1)*b)>>1],&bunchgrid[j],b*sizeof(bunchgrid[0]));
	for(i=b+1;i<bunchn;i++) bunchgrid[(((i-1)*i)>>1)+b] = ((bunchgrid[j+i]&1)<<1) + (bunchgrid[j+i]>>1);

	// === DRAW CEILINGS & FLOORS ===
	for(isflor=0;isflor<2;isflor++)
	{
		// Back-face culling: skip if camera is on wrong side of surface
		if ((gcam.p.z >= getslopez(&sec[s],isflor,gcam.p.x,gcam.p.y)) == isflor) continue;

		// Setup surface properties (height, gradient, color)
		fz = sec[s].z[isflor]; grad = &sec[s].grad[isflor];
		gcurcol = (min(sec[s].surf[isflor].asc>>8,255)<<24) +
					 (min(sec[s].surf[isflor].rsc>>5,255)<<16) +
					 (min(sec[s].surf[isflor].gsc>>5,255)<< 8) +
					 (min(sec[s].surf[isflor].bsc>>5,255)    );
		gcurcol = argb_interp(gcurcol,(gcurcol&0xff000000)+((gcurcol&0xfcfcfc)>>2),(int)(compact2d*24576.0));

		// Calculate surface normal vector
		gnorm.x = grad->x;
		gnorm.y = grad->y;
		gnorm.z = 1.f; if (isflor) { gnorm.x = -gnorm.x; gnorm.y = -gnorm.y; gnorm.z = -gnorm.z; }
		f = 1.0/sqrt(gnorm.x*gnorm.x + gnorm.y*gnorm.y + 1); gnorm.x *= f; gnorm.y *= f; gnorm.z *= f;

			//plane point: (wal[0].x,wal[0].y,fz)
			//plane norm: <grad->x,grad->y,1>
			//
			//   (wal[i].x-wal[0].x)*grad->x +
			//   (wal[i].y-wal[0].y)*grad->y +
			//   (?       -      fz)*      1 = 0
		// Build polygon for ceiling/floor using plane equation:
		plothead[0] = -1; plothead[1] = -1;
		for(ww=twaln;ww>=0;ww-=twaln) plothead[isflor] = mono_ins(plothead[isflor],twal[ww].x,twal[ww].y,gnorm.z*-1e32); //do not replace w/single zenith point - ruins precision
		i = isflor^1;
		for(ww=0;ww<=twaln;ww++) plothead[i] = mono_ins(plothead[i],twal[ww].x,twal[ww].y,(wal[0].x-twal[ww].x)*grad->x + (wal[0].y-twal[ww].y)*grad->y + fz);
		plothead[i] = mp[plothead[i]].n;

		// Setup texture and rendering flags
		sur = &sec[s].surf[isflor]; gtpic = &gtile[sur->tilnum]; if (!gtpic->tt.f) loadpic(gtpic);
			  if (sec[s].surf[isflor].flags&(1<<17)) { gflags = 2;                  } //skybox ceil/flor
		else if (sec[s].surf[isflor].flags&(1<<16)) { gflags = 1; gentex_sky(sur); } //parallaxing ceil/flor
															else { gflags = 0; gentex_ceilflor(&sec[s],wal,sur,isflor); }
		gligwall = isflor-2;
		drawpol_befclip(s,-1,plothead[0],plothead[1],(isflor<<2)+3);
#ifdef STANDALONE
		gcnt--; if (gcnt <= 0) return;
#endif
	}

	// === DRAW WALLS ===
	for(ww=0;ww<twaln;ww++)
	{
		// Get wall vertices and setup wall segment
		vn = getwalls(s,twal[ww].i,verts,MAXVERTS);
		w = twal[ww].i; nw = wal[w].n+w;
		sur = &wal[w].surf;

		// Calculate wall length and setup color/normal
		dx = sqrt((wal[nw].x-wal[w].x)*(wal[nw].x-wal[w].x) + (wal[nw].y-wal[w].y)*(wal[nw].y-wal[w].y));
		gcurcol = (min(sur->asc>>8,255)<<24) +
					 (min(sur->rsc>>5,255)<<16) +
					 (min(sur->gsc>>5,255)<< 8) +
					 (min(sur->bsc>>5,255)    );
		gnorm.x = wal[w].y-wal[nw].y;
		gnorm.y = wal[nw].x-wal[w].x;
		gnorm.z = 0;
		f = 1.0/sqrt(gnorm.x*gnorm.x + gnorm.y*gnorm.y); gnorm.x *= f; gnorm.y *= f;
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
			if (!intersect_traps_mono(pol[0].x,pol[0].y, pol[1].x,pol[1].y, pol[0].z-f,pol[1].z-f,pol[2].z+f,pol[3].z+f, opolz[0]-f,opolz[1]-f,opolz[2]+f,opolz[3]+f, &plothead[0],&plothead[1])) continue;

			// Render wall segment if visible
			if ((!(m&1)) || (wal[w].surf.flags&(1<<5))) //Draw wall here //(1<<5): 1-way
			{
				gtpic = &gtile[sur->tilnum]; if (!gtpic->tt.f) loadpic(gtpic);
					  if (sur->flags&(1<<17)) { gflags = 2;                  } //skybox ceil/flor
				else if (sur->flags&(1<<16)) { gflags = 1; gentex_sky(sur); } //parallaxing ceil/flor
				else
				{
					// Calculate UV mapping for wall texture
					npol2[0].x = wal[ w].x; npol2[0].y = wal[ w].y; npol2[0].z = getslopez(&sec[s],0,wal[w].x,wal[w].y);
					npol2[1].x = wal[nw].x; npol2[1].y = wal[nw].y; npol2[1].z = npol2[0].z;
					npol2[2].x = wal[ w].x; npol2[2].y = wal[ w].y; npol2[2].z = npol2[0].z + 1.f;
					// Determine reference Z-level texture alignment
						  if (!(sur->flags&4)) f = sec[                s].z[0];
					else if (!vn)             f = sec[                s].z[1]; //White walls don't have verts[]!
					else if (!m)              f = sec[verts[       0].s].z[0];
					else                      f = sec[verts[(m-1)>>1].s].z[0];
					// Apply UV coordinates with proper scaling
					npol2[0].u = sur->uv[0].x;                 npol2[0].v = sur->uv[2].y*(npol2[0].z-f) + sur->uv[0].y;
					npol2[1].u = sur->uv[1].x*dx + npol2[0].u; npol2[1].v = sur->uv[1].y*dx             + npol2[0].v;
					npol2[2].u = sur->uv[2].x    + npol2[0].u; npol2[2].v = sur->uv[2].y                + npol2[0].v;
					gflags = 0; gentex_wall(npol2,sur);
				}
				gligwall = w; gligslab = m; ns = -1;
				/* notes:
				 *	gligsect = s;        // Current sector
					gligwall = w;        // Wall index
					gligslab = m;        // Segment/slab number (0,1,2... for each vertical division)*/

			} else ns = verts[m>>1].s; // Portal to adjacent sector

			// Render the wall polygon
			drawpol_befclip(s,ns,plothead[0],plothead[1],((m>vn)<<2)+3);
#ifdef STANDALONE
			gcnt--; if (gcnt <= 0) return; // Debug: limit polygon count
#endif
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

void draw_hsr_polymost (cam_t *cc, mapstate_t *lgs, player_transform *lps, int cursect)
{
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
		if ((!(glp->flags&1)) || (!shadowtest2_useshadows)) return;
	}
 	gcam = (*cc); gst = lgs; gps = lps;

	if ((lgs->numsects <= 0) || ((unsigned)cursect >= (unsigned)lgs->numsects))
	{
		if (shadowtest2_rendmode != 4)
		{
			for(i=0,j=gcam.c.f;i<gcam.c.y;i++,j+=gcam.c.p) memset8((void *)j,0x00000000,gcam.c.x<<2);
			for(i=0,j=gcam.z.f;i<gcam.z.y;i++,j+=gcam.z.p) memset8((void *)j,0x7f7f7f7f,gcam.z.x<<2);
		}
		if (shadowtest2_rendmode != 4) eyepoln = 0; //Prevents drawpollig() from crashing
		return;
	}
	if (!bunchmal)
	{
		bunchmal = 64;
		bunch     = (bunch_t       *)malloc(bunchmal*sizeof(bunch[0]));
		bunchgot  = (unsigned int  *)malloc(((bunchmal+31)&~31)>>3);
		bunchgrid = (unsigned char *)malloc(((bunchmal-1)*bunchmal)>>1);
	}
	if (lgs->numsects > sectgotn)
	{
		if (sectgotmal) free((void *)sectgotmal);
		sectgotn = ((lgs->numsects+127)&~127);
		sectgotmal = (unsigned int *)malloc((sectgotn>>3)+16); //NOTE:malloc doesn't guarantee 16-byte alignment!
		sectgot = (unsigned int *)((((intptr_t)sectgotmal)+15)&~15);
	}
	if ((shadowtest2_rendmode != 4) && (lgs->numsects > shadowtest2_sectgotn))
	{
		if (shadowtest2_sectgotmal) free((void *)shadowtest2_sectgotmal);
		shadowtest2_sectgotn = ((lgs->numsects+127)&~127);
		shadowtest2_sectgotmal = (unsigned int *)malloc((shadowtest2_sectgotn>>3)+16); //NOTE:malloc doesn't guarantee 16-byte alignment!
		shadowtest2_sectgot = (unsigned int *)((((intptr_t)shadowtest2_sectgotmal)+15)&~15);
	}
	if (!mphmal) mono_initonce();

#ifdef STANDALONE
	fixposx = 0; fixposy = 32;
	//fixposx = 600; fixposy = 256;
#endif

		//Hack to keep camera away from sector line; avoids clipping glitch in drawpol_befclip/changetagfunc
	wal = lgs->sect[cursect].wall;
	for(i=lgs->sect[cursect].n-1;i>=0;i--)
	{
		#define WALHAK 1e-3
		j = wal[i].n+i;
		d = distpoint2line2(gcam.p.x,gcam.p.y,wal[i].x,wal[i].y,wal[j].x,wal[j].y); if (d >= WALHAK*WALHAK) continue;
		fp.x = wal[j].x-wal[i].x;
		fp.y = wal[j].y-wal[i].y;
		f = (WALHAK - sqrt(d))/sqrt(fp.x*fp.x + fp.y*fp.y);
		gcam.p.x -= fp.y*f;
		gcam.p.y += fp.x*f;
	}

	if (shadowtest2_rendmode != 4)
	{
			//Horrible hacks for internal build2 global variables
		dpos.x = 0.0; dpos.y = 0.0; dpos.z = 0.0;
		drig.x = 1.0; drig.y = 0.0; drig.z = 0.0;
		ddow.x = 0.0; ddow.y = 1.0; ddow.z = 0.0;
		dfor.x = 0.0; dfor.y = 0.0; dfor.z = 1.0;
		drawpoly_setup(                           (tiletype *)&gcam.c,gcam.z.f-gcam.c.f,&dpos,&drig,&ddow,&dfor,gcam.h.x,gcam.h.y,gcam.h.z);
		drawcone_setup(cputype,shadowtest2_numcpu,(tiletype *)&gcam.c,gcam.z.f-gcam.c.f,&dpos,&drig,&ddow,&dfor,gcam.h.x,gcam.h.y,gcam.h.z);
		 drawkv6_setup(&drawkv6_frame,            (tiletype *)&gcam.c,gcam.z.f-gcam.c.f,&dpos,&drig,&ddow,&dfor,gcam.h.x,gcam.h.y,gcam.h.z);

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

#ifdef STANDALONE
		if (keystatus[0x4a]) { keystatus[0x4a] = 0; curgcnt = max(curgcnt-1,1); } //KP-
		if (keystatus[0x4e]) { keystatus[0x4e] = 0; curgcnt++;                  } //KP+
		if (keystatus[0xc7]) { keystatus[0x4a] = 0; curgcnt = 1;                } //Home
		if (keystatus[0xcf]) { keystatus[0x4e] = 0; curgcnt = 0x7fffffff;       } //End
#endif

		eyepoln = 0; eyepolvn = 0;
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
#ifdef STANDALONE
	gcnt = curgcnt;
#endif

	for(halfplane=0;halfplane<2;halfplane++)
	{
		if (shadowtest2_rendmode == 4)
		{
			if (!halfplane) gcam.r.x = 1; else gcam.r.x = -1;
							  gcam.d.x = 0; gcam.f.x = 0;
			gcam.r.y = 0; gcam.d.y = 0; gcam.f.y = -gcam.r.x;
			gcam.r.z = 0; gcam.d.z = 1; gcam.f.z = 0;
			xformprep(0.0);

			xformbac(-65536.0,-65536.0,1.0,&bord2[0]);
			xformbac(+65536.0,-65536.0,1.0,&bord2[1]);
			xformbac(+65536.0,+65536.0,1.0,&bord2[2]);
			xformbac(-65536.0,+65536.0,1.0,&bord2[3]);
			n = 4; didcut = 1;
		}
		else
		{
			xformprep(((double)halfplane)*PI);

			i = 0; //i = 16;
			xformbac(         i-gcam.h.x,         i-gcam.h.y,gcam.h.z,&bord[0]);
			xformbac(gcam.c.x-i-gcam.h.x,         i-gcam.h.y,gcam.h.z,&bord[1]);
			xformbac(gcam.c.x-i-gcam.h.x,gcam.c.y-i-gcam.h.y,gcam.h.z,&bord[2]);
			xformbac(         i-gcam.h.x,gcam.c.y-i-gcam.h.y,gcam.h.z,&bord[3]);

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

		memset8(sectgot,0,(lgs->numsects+31)>>3);

		for(j=0;j<n;j++)
		{
			f = gcam.h.z/bord2[j].z;
			bord2[j].x = bord2[j].x*f + gcam.h.x;
			bord2[j].y = bord2[j].y*f + gcam.h.y;
		}

			//FIX! once means not each frame! (of course it doesn't hurt functionality)
		for(i=mphnum-1;i>=0;i--) { mono_deloop(mph[i].head[1]); mono_deloop(mph[i].head[0]); }
		mono_genfromloop(&mph[0].head[0],&mph[0].head[1],bord2,n); mph[0].tag = cursect; mphnum = 1;

		bunchn = 0; scansector(cursect);
		while (bunchn)
		{
			memset(bunchgot,0,(bunchn+7)>>3);
#if 0
			closest = 0;
			for(i=1;i<bunchn;i++)
			{
				if (bunchgot[i>>5]&(1<<i)) continue;
				j = bunchfront(i,closest,0); if (!j) continue;
				if (j == 3) continue; //FIXFIXFIXFIX
				bunchgot[i>>5] |= (1<<i);
				if (j == 1) { closest = i; i = 0; }
			}
#else
			//{
			//char tbuf[1024]; sprintf(tbuf,"cnt=%d\n",(1<<31)-1-gcnt);
			//for(i=0;i<bunchn;i++)
			//{
			//   for(j=0;j<bunchn;j++) sprintf(&tbuf[strlen(tbuf)],"bf(%d,%d)=%2d ",i,j,bunchfront(i,j,0));
			//   sprintf(&tbuf[strlen(tbuf)],"\n");
			//   //sprintf(&tbuf[strlen(tbuf)],"bunch %d: (%d:%f) (%d:%f)\n",i,bunch[i].wal0,bunch[i].fra0,bunch[i].wal1,bunch[i].fra1);
			//}
			//sprintf(&tbuf[strlen(tbuf)],"\n");
			//MessageBox(ghwnd,tbuf,prognam,MB_OK);
			//}

			for(i=bunchn-1;i>0;i--) //assume: bunchgrid[(((j-1)*j)>>1)+i] = bunchfront(j,i,0); is valid iff:{i<j}
			{
					//for(j=bunchn-1;j>=0;j--) if (bunchfront(i,j,0)&2) goto nogood;
				for(k=(((i-1)*i)>>1),j=0;j<     i;k+=1,j++) if (bunchgrid[k]&2) goto nogood;
				for(k+=j            ,j++;j<bunchn;k+=j,j++) if (bunchgrid[k]&1) goto nogood;
				break;
nogood:; }
			closest = i;
#endif

			drawalls(closest);
#ifdef STANDALONE
			if (gcnt <= 0) break; //FIX
#endif
		}

#ifdef STANDALONE
#if 1
		if ((shadowtest2_rendmode == 1) && (!keystatus[0x38]))
		{
			extern void loadpic (tile_t *); //gcnt = 1;
			gtpic = &gtile[0]; if (!gtpic->tt.f) loadpic(gtpic);
			gouvmat[0] =     0; gouvmat[3] =     0; gouvmat[6] = 1; //d
			gouvmat[1] = 65536; gouvmat[4] =     0; gouvmat[7] = 0; //u
			gouvmat[2] =     0; gouvmat[5] = 65536; gouvmat[8] = 0; //v
			for(i=mphnum-1;i>=0;i--)
			{
				gcurcol = ((mph[i].tag*0x4357267+0x23457)&0x0f0f0f)+0x202020;
				drawpol_aftclip(mph[i].head[0],mph[i].head[1]);
			}
		}
#endif

#if 0
		n = 0;
		for(i=mphnum-1;i>=0;i--) for(s=0;s<2;s++) { j = mph[i].head[s]; if (j < 0) continue; do { j = mp[j].n; n++; } while (j != mph[i].head[s]); }
		print6x8(&gcam.c,96,16,0xffffff,0,"mp lists num =%6d, mphnum=%d",n,mphnum);

		w = 0;
		i = mpempty; do { i = mp[i].n; w++; } while (i != mpempty);
		print6x8(&gcam.c,96,24,0xffffff,0," mpempty num =%6d",w);

		//print6x8(&gcam.c,96,32,0xffffff,0,"memory leak! =%6d",MPMAX-(n+w));
#endif

#if 0
		fixposy += 8; if (fixposy >= gdd.y) { fixposy = 0; fixposx += 256; }
		for(k=0;k<mphnum;k++)
		{
			print6x8(&gcam.c,fixposx,fixposy,0xffffff,0,"%d (area=%f)",mph[k].tag,mono_area(mph[k].head[0],mph[k].head[1])); fixposy += 8; if (fixposy >= gdd.y) { fixposy = 0; fixposx += 256; }
			for(s=0;s<2;s++)
			{
				i = mph[k].head[s]; if (i < 0) continue;
				do
				{
					j = mp[i].n;
					print6x8(&gcam.c,fixposx,fixposy,0xffffff,0,"%8.2f %8.2f",mp[i].x,mp[i].y); fixposy += 8; if (fixposy >= gdd.y) { fixposy = 0; fixposx += 256; }
					i = j;
				} while (i != mph[k].head[s]);
				fixposy += 2;
			}
		}
#endif
#endif

		if (shadowtest2_rendmode == 4) uptr = glp->sectgot;
										  else uptr = shadowtest2_sectgot;

		if (!halfplane)
		{
			memcpy(uptr,sectgot,(lgs->numsects+31)>>3);
		}
		else
		{
			if (!(cputype&(1<<25))) //Got SSE
				{ for(i=((lgs->numsects+31)>>5)-1;i>=0;i--) uptr[i] |= sectgot[i]; }
			else
			{
				i = (((lgs->numsects+127)&~127)>>3);
				_asm
				{
					mov eax, uptr
					mov edx, sectgot
					mov ecx, i
		  begor: sub ecx, 16
					movaps xmm0, [eax+ecx]
					orps xmm0, [edx+ecx]
					movaps [eax+ecx], xmm0
					jg short begor
				}
			}
		}

		if (!didcut) break;
	}
}

typedef struct { int sect; point3d p; float rgb[3]; int useshadow; } drawkv6_lightpos_t;
extern drawkv6_lightpos_t drawkv6_light[MAXLIGHTS];
extern int drawkv6_numlights;
extern float drawkv6_ambrgb[3];
void drawsprites (void)
{
	spri_t *spr;
	int i, s, w;

	if (!shadowtest2_sectgot) return;

	if (shadowtest2_numlights)
	{
		drawkv6_numlights = shadowtest2_numlights;
		for(i=shadowtest2_numlights-1;i>=0;i--)
		{
			float ox, oy, oz;

			drawkv6_light[i].sect      = shadowtest2_light[i].sect;
			drawkv6_light[i].p         = shadowtest2_light[i].p;
			drawkv6_light[i].rgb[0]    = shadowtest2_light[i].rgb[0];
			drawkv6_light[i].rgb[1]    = shadowtest2_light[i].rgb[1];
			drawkv6_light[i].rgb[2]    = shadowtest2_light[i].rgb[2];
			drawkv6_light[i].useshadow = (shadowtest2_light[i].flags&1);
			//xformpos(&drawkv6_light[i].p.x,&drawkv6_light[i].p.y,&drawkv6_light[i].p.z);

			ox = drawkv6_light[i].p.x-gps->ipos.x;
			oy = drawkv6_light[i].p.y-gps->ipos.y;
			oz = drawkv6_light[i].p.z-gps->ipos.z;
			drawkv6_light[i].p.x = ox*gps->irig.x + oy*gps->irig.y + oz*gps->irig.z;
			drawkv6_light[i].p.y = ox*gps->idow.x + oy*gps->idow.y + oz*gps->idow.z;
			drawkv6_light[i].p.z = ox*gps->ifor.x + oy*gps->ifor.y + oz*gps->ifor.z;
		}
		drawkv6_ambrgb[0] = shadowtest2_ambrgb[0];
		drawkv6_ambrgb[1] = shadowtest2_ambrgb[1];
		drawkv6_ambrgb[2] = shadowtest2_ambrgb[2];
	} else drawkv6_numlights = -1;

	gcam.p.x = 0; gcam.p.y = 0; gcam.p.z = 0;
	gcam.r.x = 1; gcam.r.y = 0; gcam.r.z = 0;
	gcam.d.x = 0; gcam.d.y = 1; gcam.d.z = 0;
	gcam.f.x = 0; gcam.f.y = 0; gcam.f.z = 1;
	for(s=uptil1(shadowtest2_sectgot,gst->numsects);s>0;s=uptil1(shadowtest2_sectgot,s-1))
		for(w=gst->sect[s-1].headspri;w>=0;w=gst->spri[w].sectn)
		{
			spr = &gst->spri[w];
			if (!(spr->flags&0x80000000)) drawsprite(&gcam,spr); //Draw non-invisible sprites
		}

	drawkv6_numlights = -1;
}

void shadowtest2_setcam (cam_t *ncam)
{
	gcam = *ncam;
}

#if (USENEWLIGHT == 0)
typedef struct { float n2, d2, n1, d1, n0, d0, filler0[2], glk[12], bsc, gsc, rsc, filler1[1]; } hlighterp_t;
#else
typedef struct { float gk[16], gk2[12], bsc, gsc, rsc, filler1[1]; } hlighterp_t;
__declspec(align(16)) static const float hligterp_maxzero[4] = {0.f,0.f,0.f,0.f};
#endif
void prepligramp (float *ouvmat, point3d *norm, int lig, hlighterp_t *hl)
{
#if (USENEWLIGHT == 0)
	float f, ox, oy, oz, p0x, p0y, p0z, p1x, p1y, p1z, p2x, p2y, p2z;

		//Prepare light ramping
#if (USEINTZ)
	f = 1.0/(1048576.0*256.0);
#else
	f = 1.0/gcam.h.z;
#endif
	p0x = slightpos[lig].x*ouvmat[0] - f;
	p0y = slightpos[lig].y*ouvmat[0];
	p0z = slightpos[lig].z*ouvmat[0];
	p1x = slightpos[lig].x*ouvmat[3];
	p1y = slightpos[lig].y*ouvmat[3] - f;
	p1z = slightpos[lig].z*ouvmat[3];
	p2x = slightpos[lig].x*ouvmat[6] + gcam.h.x*f;
	p2y = slightpos[lig].y*ouvmat[6] + gcam.h.y*f;
	p2z = slightpos[lig].z*ouvmat[6] - gcam.h.z*f;
	hl->glk[ 0] = (p0x*p0x + p0y*p0y + p0z*p0z)  ;
	hl->glk[ 1] = (p1x*p0x + p1y*p0y + p1z*p0z)*2;
	hl->glk[ 2] = (p2x*p0x + p2y*p0y + p2z*p0z)*2;
	hl->glk[ 3] = (p1x*p1x + p1y*p1y + p1z*p1z)  ;
	hl->glk[ 4] = (p1x*p2x + p1y*p2y + p1z*p2z)*2;
	hl->glk[ 5] = (p2x*p2x + p2y*p2y + p2z*p2z)  ;
	f = 1024.f;
	ox = (p0x*norm->x + p0y*norm->y + p0z*norm->z)*f;
	oy = (p1x*norm->x + p1y*norm->y + p1z*norm->z)*f;
	oz = (p2x*norm->x + p2y*norm->y + p2z*norm->z)*f;
	hl->glk[ 6] = ouvmat[0]*ox               ;
	hl->glk[ 7] = ouvmat[3]*ox + ouvmat[0]*oy;
	hl->glk[ 8] = ouvmat[6]*ox + ouvmat[0]*oz;
	hl->glk[ 9] = ouvmat[3]*oy               ;
	hl->glk[10] = ouvmat[3]*oz + ouvmat[6]*oy;
	hl->glk[11] =                ouvmat[6]*oz;

	hl->d2 = hl->glk[0];
	hl->n2 = hl->glk[6];
#else
	float f, k, k0, k1, k3, k4, k5, k6, k7, k8, k9, ka, kb;

		//Prepare light ramping
	//point3d pt; pt.x = 0; pt.y = 0; pt.z = 1.0/(ouvmat[0]*gcam.h.x + ouvmat[3]*gcam.h.y + ouvmat[6]); k0 = pt.x*norm->x + pt.y*norm->y + pt.z*norm->z;
	//k0 = norm->z/(ouvmat[0]*gcam.h.x + ouvmat[3]*gcam.h.y + ouvmat[6]);
	k0 = (norm->x*norm->x + norm->y*norm->y + norm->z*gcam.h.z) / ((ouvmat[0]*(norm->x+gcam.h.x) + ouvmat[3]*(norm->y+gcam.h.y) + ouvmat[6])*gcam.h.z);

	hl->gk[15] = gcam.h.x*norm->x + gcam.h.y*norm->y - gcam.h.z*norm->z;

	k3 = slightpos[lig].x*norm->x - k0;
	k4 = slightpos[lig].x*norm->y;
	k5 =-slightpos[lig].x*hl->gk[15] + gcam.h.x*k0;
	k6 = slightpos[lig].y*norm->x;
	k7 = slightpos[lig].y*norm->y - k0;
	k8 =-slightpos[lig].y*hl->gk[15] + gcam.h.y*k0;
	k9 = slightpos[lig].z*norm->x;
	ka = slightpos[lig].z*norm->y;
	kb =-slightpos[lig].z*hl->gk[15] - gcam.h.z*k0;
	hl->gk[0] = (k3*k3 + k6*k6 + k9*k9)*1; //x*x
	hl->gk[1] = (k3*k4 + k6*k7 + k9*ka)*2; //x*y
	hl->gk[2] = (k4*k4 + k7*k7 + ka*ka)*1; //y*y
	hl->gk[3] = (k3*k5 + k6*k8 + k9*kb)*2; //x
	hl->gk[4] = (k4*k5 + k7*k8 + ka*kb)*2; //y
	hl->gk[5] = (k5*k5 + k8*k8 + kb*kb)*1; //1

	k = (slightpos[lig].x*norm->x + slightpos[lig].y*norm->y + slightpos[lig].z*norm->z - k0)*-256*16;
	hl->gk[6] = k*norm->x;
	hl->gk[7] = k*norm->y;
	hl->gk[8] = k*-hl->gk[15];

	if (spotwid[lig] > -1)
	{
		k = slightpos[lig].x*slightdir[lig].x + slightpos[lig].y*slightdir[lig].y + slightpos[lig].z*slightdir[lig].z;
		k1 = 1.0/(1.0-spotwid[lig]);
		hl->gk[ 9] = (k*norm->x - k0*slightdir[lig].x)*k1;
		hl->gk[10] = (k*norm->y - k0*slightdir[lig].y)*k1;
		hl->gk[11] = ((gcam.h.x*slightdir[lig].x + gcam.h.y*slightdir[lig].y - gcam.h.z*slightdir[lig].z)*k0 - k*hl->gk[15])*k1;
		hl->gk[12] = -spotwid[lig]*k1;
	} else { hl->gk[9] = 0; hl->gk[10] = 0; hl->gk[11] = 0; hl->gk[12] = 1; }

	hl->gk[13] = -norm->x;
	hl->gk[14] = -norm->y;
#endif
	hl->bsc = shadowtest2_light[lig].rgb[0];
	hl->gsc = shadowtest2_light[lig].rgb[1];
	hl->rsc = shadowtest2_light[lig].rgb[2];
#if (USEGAMMAHACK != 0)
	f = 16384.0; hl->bsc *= f; hl->gsc *= f; hl->rsc *= f;
#endif
	hl->filler1[0] = 0.f; //Make sure this is not denormal!
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
#ifdef STANDALONE

#pragma comment(lib,"comdlg32.lib")
#include <commdlg.h>

static int numframes = 0;

static double dtotclk, odtotclk, dtim;

	//FPS counter
#define FPSSIZ 64
static int fpsind[FPSSIZ];
static float fpsdtim[FPSSIZ];

static cam_t cam;
static dpoint3d startpos, startrig, startdow, startfor, dcamp;
static int *zbuffermem = 0, zbuffersiz = 0, cursect;
static int shadowtest_numcpu;

static dpoint3d ligvel[LIGHTMAX];


	//For debug only!
static int setclipboardtext (char *st)
{
	HANDLE hbuf;
	int i, j;
	char *cptr;

	for(i=0,j=0;st[i];i++) if (st[i] == 13) j++;
	if (!OpenClipboard(ghwnd)) return(0);
	EmptyClipboard();
	hbuf = GlobalAlloc(GMEM_MOVEABLE,i+j+1); if (!hbuf) { CloseClipboard(); return(0); }

	cptr = (char *)GlobalLock(hbuf);
	for(i=0;st[i];i++) { *cptr++ = st[i]; if (st[i] == 13) *cptr++ = 10; }
	*cptr++ = 0;
	GlobalUnlock(hbuf);

	SetClipboardData(CF_TEXT,hbuf);
	CloseClipboard();

	//NOTE: Clipboard owns hbuf - don't do "GlobalFree(hbuf);"!
	return(1);
}

static void resetview (void)
{
		//EVILNESS: doubles can become denormal when converted to single precision; killing fps!
	if (fabs(startrig.x) <= 1.4013e-45) startrig.x = 0.0;
	if (fabs(startrig.y) <= 1.4013e-45) startrig.y = 0.0;
	if (fabs(startrig.z) <= 1.4013e-45) startrig.z = 0.0;
	if (fabs(startdow.x) <= 1.4013e-45) startdow.x = 0.0;
	if (fabs(startdow.y) <= 1.4013e-45) startdow.y = 0.0;
	if (fabs(startdow.z) <= 1.4013e-45) startdow.z = 0.0;
	if (fabs(startfor.x) <= 1.4013e-45) startfor.x = 0.0;
	if (fabs(startfor.y) <= 1.4013e-45) startfor.y = 0.0;
	if (fabs(startfor.z) <= 1.4013e-45) startfor.z = 0.0;

	cam.p.x = startpos.x; cam.p.y = startpos.y; cam.p.z = startpos.z;
	cam.r.x = startrig.x; cam.r.y = startrig.y; cam.r.z = startrig.z;
	cam.d.x = startdow.x; cam.d.y = startdow.y; cam.d.z = startdow.z;
	cam.f.x = startfor.x; cam.f.y = startfor.y; cam.f.z = startfor.z;
	cam.h.x = xres/2; cam.h.y = yres/2; cam.h.z = cam.h.x;
	dcamp = startpos;
}

static void grablightsfrommap (void)
{
	lightpos_t *lp;
	int i;
	mapstate_t sst = *gst;

	shadowtest2_numlights = 0;
	for(i=0;i<sst.numspris;i++)
	{
		if (!(sst.spri[i].flags&(1<<16))) continue;

		lp = &shadowtest2_light[shadowtest2_numlights];
		lp->sect   = sst.spri[i].sect;
		lp->sprilink = i;
		lp->p      = sst.spri[i].p;
		lp->rgb[0] = sst.spri[i].bsc/8192.0;
		lp->rgb[1] = sst.spri[i].gsc/8192.0;
		lp->rgb[2] = sst.spri[i].rsc/8192.0;
		lp->flags  = 1;
		lp->sectgot      = 0;
		lp->sectgotmal   = 0;
		lp->sectgotn     = 0;
		lp->lighashead   = 0;
		lp->lighasheadn  = 0;
		lp->ligpol       = 0;
		lp->ligpoln      = 0;
		lp->ligpolmal    = 0;
		lp->ligpolv      = 0;
		lp->ligpolvn     = 0;
		lp->ligpolvmal   = 0;

		ligvel[shadowtest2_numlights].x = 0.0;
		ligvel[shadowtest2_numlights].y = 0.0;
		ligvel[shadowtest2_numlights].z = 0.0;
		shadowtest2_numlights++;
	}
	if (shadowtest2_numlights) shadowtest2_rendmode = 0;
}

static void resetfps (void)
{
	int i;
	for(i=0;i<FPSSIZ;i++) { fpsdtim[i] = 1e32; fpsind[i] = i; } numframes = 0;
}

static void uninitapp (void)
{
	if (zbuffermem) { free(zbuffermem); zbuffermem = 0; } zbuffersiz = 0;
}

static long initapp (long argc, char **argv)
{
	int i, j, k, l, argnoslash[8], argnoslashcnt = 0, numcpu = 0;
	char *filnam;

	xres = 800; yres = 600; colbits = 32; fullscreen = 0; prognam = "ShadowTest2";
	for(i=1;i<argc;i++)
	{
		if ((argv[i][0] != '/') && (argv[i][0] != '-'))
		{
			if (argnoslashcnt < sizeof(argnoslash)/sizeof(argnoslash[0])) argnoslash[argnoslashcnt++] = i;
			continue;
		}
		if (argv[i][1] == '?') { MessageBox(0,"shadowtest2 [.MAP] [/#x#(x)] [/?]",prognam,MB_OK); return(-1); }
		if (!memicmp(&argv[i][1],"cpu=",4)) { numcpu = min(max(atol(&argv[i][5]),1),64/*MAXCPU*/); continue; }
		if ((argv[i][1] >= '0') && (argv[i][1] <= '9'))
		{
			k = 0; l = 0;
			for(j=1;;j++)
			{
				if ((argv[i][j] >= '0') && (argv[i][j] <= '9')) { k = (k*10+argv[i][j]-48); continue; }
				switch (l)
				{
					case 0: xres = k; break;
					case 1: yres = k; break;
					case 2: fullscreen = 1; break;
				}
				if (!argv[i][j]) break;
				l++; if (l > 2) break;
				k = 0;
			}
		}
	}

	drawpoly_init();
	if (build2_init() < 0) { MessageBox(ghwnd,"Build2_init failed",prognam,MB_OK); return(-1); }

	if (!argnoslashcnt)
	{
		filnam = "doortest.map";

		 //FIXFIXFIXFIX: former bunchfront() splitting problems.. remove!
	 ////filnam = "fukl.map";    //used 2 crashes at start.. bug fixed.. remove!
	 ////filnam = "balcony.map"; //used 2 crashes instantly due to some light.. bug fixed.. remove!
	 ////filnam = "motel.map";   //used 2 crashes near start.. bug fixed.. remove!

		if (!build2_loadmap(filnam,&cursect,&startpos,&startrig,&startdow,&startfor)) { MessageBox(ghwnd,".MAP not found",prognam,MB_OK); return(-1); }
	}
	else
	{
		for(j=0;j<argnoslashcnt;j++)
			if (!build2_loadmap(argv[argnoslash[j]],&cursect,&startpos,&startrig,&startdow,&startfor))
			{
				char tbuf[MAX_PATH];
				sprintf(tbuf,"%s.map",argv[argnoslash[j]]);
				build2_loadmap(tbuf,&cursect,&startpos,&startrig,&startdow,&startfor);
			}
	}

	//startpos.x = 0.4589626193; startpos.y = -11.1178150177; startpos.z = 0.3460207283; //FIX! for fuk9.map only!

	resetview();
	grablightsfrommap();
	resetfps();

	if (numcpu) drawpoly_numcpu = numcpu;
	shadowtest2_numcpu = drawpoly_numcpu;

	return(0);
}
/*
static void doframe ()
{
	static dpoint3d dp, dcamr, dcamd, dcamf;
	static int bstatus, obstatus, ischanged = 3, movelights = 0;
	lightpos_t *lp;
	float f, g, fx, fy, fmousx, fmousy;
	intptr_t p, zbufoff;
	int i, j, i0, i1, lig, x, y;

	readkeyboard(); if (ext_keystatus[1]) { quitloop(); return; }
	odtotclk = dtotclk; readklock(&dtotclk); dtim = dtotclk-odtotclk;
	obstatus = bstatus; readmouse(&fmousx,&fmousy,(long *)&bstatus);

	f = dtim*4;
	if (keystatus[0x36]) f *= 4;
	if (keystatus[0x2a]) f *= .25;
	dp.x = dp.y = dp.z = 0.f;
	if (keystatus[0xcb]) { dp.x -= cam.r.x*f; dp.y -= cam.r.y*f; dp.z -= cam.r.z*f; } //left
	if (keystatus[0xcd]) { dp.x += cam.r.x*f; dp.y += cam.r.y*f; dp.z += cam.r.z*f; } //right
	if (keystatus[0xc8]) { dp.x += cam.f.x*f; dp.y += cam.f.y*f; dp.z += cam.f.z*f; } //forward
	if (keystatus[0xd0]) { dp.x -= cam.f.x*f; dp.y -= cam.f.y*f; dp.z -= cam.f.z*f; } //back
	if (keystatus[0x9d]) { dp.x -= cam.d.x*f; dp.y -= cam.d.y*f; dp.z -= cam.d.z*f; } //Rctrl
	if (keystatus[0x52]) { dp.x += cam.d.x*f; dp.y += cam.d.y*f; dp.z += cam.d.z*f; } //KP0
	if (keystatus[0xb8]) { dcamp.x += dp.x; dcamp.y += dp.y; dcamp.z += dp.z; } else build2_hitmove(&cursect,&dcamp,&dp,0.25,1,0,0);
	cam.p.x = (float)dcamp.x; cam.p.y = (float)dcamp.y; cam.p.z = (float)dcamp.z;
	if (keystatus[0x4b]) { cam.h.x += f*64.0; } //KP4
	if (keystatus[0x4d]) { cam.h.x -= f*64.0; } //KP6
	if (keystatus[0x48]) { cam.h.y += f*64.0; } //KP8
	if (keystatus[0x50]) { cam.h.y -= f*64.0; } //KP2
	if (keystatus[0x35]) { keystatus[0x35] = 0; resetview(); } // /
	if (keystatus[0x4c]) { cam.h.x = xres/2; cam.h.y = yres/2; cam.h.z = cam.h.x; } //KP5
	if (keystatus[0xb5]) { g = cam.h.z; cam.h.z *= pow(1.25,-f); if ((g > cam.h.x) && (cam.h.z <= cam.h.x)) { cam.h.z = cam.h.x; keystatus[0xb5] = 0; } } //KP/
	if (keystatus[0x37]) { g = cam.h.z; cam.h.z *= pow(1.25,+f); if ((g < cam.h.x) && (cam.h.z >= cam.h.x)) { cam.h.z = cam.h.x; keystatus[0x37] = 0; } } //KP*
	if (!(bstatus&2)) orthorotate(cam.r.z*.05,fmousy*.01,fmousx*.01,&cam.r,&cam.d,&cam.f);
					 else orthorotate(fmousx*-.01,fmousy*.01,       0.0,&cam.r,&cam.d,&cam.f);
	if (fabs(cam.r.x) <= 1e-32) cam.r.x = 0.0;
	if (fabs(cam.r.y) <= 1e-32) cam.r.y = 0.0;
	if (fabs(cam.r.z) <= 1e-32) cam.r.z = 0.0;
	if (fabs(cam.d.x) <= 1e-32) cam.d.x = 0.0;
	if (fabs(cam.d.y) <= 1e-32) cam.d.y = 0.0;
	if (fabs(cam.d.z) <= 1e-32) cam.d.z = 0.0;
	if (fabs(cam.f.x) <= 1e-32) cam.f.x = 0.0;
	if (fabs(cam.f.y) <= 1e-32) cam.f.y = 0.0;
	if (fabs(cam.f.z) <= 1e-32) cam.f.z = 0.0;

	for(i=0;i<8;i++)
		if (keystatus[i+0x3b]) //F1-F8
		{
			drawpoly_numcpu = i+1; shadowtest_numcpu = drawpoly_numcpu; shadowtest2_numcpu = drawpoly_numcpu;
			resetfps();
			break;
		}
	for(i=0;i<5;i++) //'1'-'5'
	{
		if (keystatus[i+0x02])
		{
			keystatus[i+0x02] = 0;
			shadowtest2_rendmode = i;
			if ((!shadowtest2_numlights) && (!shadowtest2_rendmode)) shadowtest2_rendmode = 2;
			clearscreen(0); if (zbuffermem) memset8((void *)zbuffermem,0,zbuffersiz);
			resetfps();
		}
	}
	if (keystatus[0x17]) { keystatus[0x17] = 0; sst.p[0].rendinterp ^= 1; resetfps(); } //I
	if (keystatus[0x26]) //L
	{
		keystatus[0x26] = 0;
		if (keystatus[0x1d]|keystatus[0x9d]) //Ctrl+L: load map
		{
			static char fileselectnam[MAX_PATH+1];
			ddflip2gdi();
			OPENFILENAME ofn = { sizeof(OPENFILENAME),ghwnd,0,"MAP\0*.map\0GRP\0*.grp\0All Files\0*.*\0\0",0,0,1,fileselectnam,MAX_PATH,0,0,0,"Select .MAP to Load",OFN_HIDEREADONLY|OFN_NOCHANGEDIR,0,0,".MAP",0,0,0};
			if (GetOpenFileName(&ofn))
			{
				if (!build2_loadmap(fileselectnam,&cursect,&startpos,&startrig,&startdow,&startfor))
					{ MessageBox(ghwnd,".MAP not found",prognam,MB_OK); }
				else
				{
					i = strlen(fileselectnam);
					if ((i >= 4) && (!stricmp(&fileselectnam[i-4],".grp")))
						kzaddstack(fileselectnam);
					else
					{
						resetview();
						for(i=shadowtest2_numlights-1;i>=0;i--) shadowtest2_dellight(i);
						grablightsfrommap();
						ischanged = 3;
						resetfps();
					}
				}
			}
			keystatus[0x1d] = keystatus[0x9d] = 0;
		}
		else
		{
			sst.light_sprinum ^= -1; //List of active light sprite indices
		}
	}


	if (keystatus[0x1f]) { keystatus[0x1f] = 0; shadowtest2_useshadows ^= 1; resetfps(); } //S
	if (keystatus[0xd2]) //Ins (insert light at pos)
	{
		keystatus[0xd2] = 0;

		if (shadowtest2_numlights < LIGHTMAX)
		{
			static const float ligdefrgb[8][3] = {1,1,1, 0,0,1, 0,1,0, 1,0,0, 0,1,1, 1,0,1, 1,1,0, .5,.5,.5};
			lp = &shadowtest2_light[shadowtest2_numlights];

			lp->sect = cursect;
			lp->sprilink = -1;
			lp->p.x = dcamp.x;
			lp->p.y = dcamp.y;
			lp->p.z = dcamp.z;
			lp->f.x = cam.f.x;
			lp->f.y = cam.f.y;
			lp->f.z = cam.f.z;
			lp->spotwid = -1.0;
			lp->rgb[0] = ligdefrgb[shadowtest2_numlights&7][0];
			lp->rgb[1] = ligdefrgb[shadowtest2_numlights&7][1];
			lp->rgb[2] = ligdefrgb[shadowtest2_numlights&7][2];
			lp->flags = 1;
			lp->sectgot      = 0;
			lp->sectgotmal   = 0;
			lp->sectgotn     = 0;
			lp->lighashead   = 0;
			lp->lighasheadn  = 0;
			lp->ligpol       = 0;
			lp->ligpoln      = 0;
			lp->ligpolmal    = 0;
			lp->ligpolv      = 0;
			lp->ligpolvn     = 0;
			lp->ligpolvmal   = 0;

			if (!(keystatus[0x1d]|keystatus[0x9d]))
			{
				ligvel[shadowtest2_numlights].x = cam.f.x;
				ligvel[shadowtest2_numlights].y = cam.f.y;
				ligvel[shadowtest2_numlights].z = cam.f.z;
				lp->spotwid = 0.25;
			}
			else
			{
				ligvel[shadowtest2_numlights].x = 0.0;
				ligvel[shadowtest2_numlights].y = 0.0;
				ligvel[shadowtest2_numlights].z = 0.0;
				lp->spotwid = 0.5;
			}
			shadowtest2_numlights++;
			ischanged = 3; shadowtest2_rendmode = 4; resetfps();
		}
	}

	if (keystatus[0xd3]) //Del (delete nearest light)
	{
		keystatus[0xd3] = 0;
		if (shadowtest2_numlights > 0)
		{
			g = 1e32; j = 0;
			for(i=shadowtest2_numlights-1;i>=0;i--)
			{
				dp.x = shadowtest2_light[i].p.x - dcamp.x;
				dp.y = shadowtest2_light[i].p.y - dcamp.y;
				dp.z = shadowtest2_light[i].p.z - dcamp.z;
				f = dp.x*dp.x + dp.y*dp.y + dp.z*dp.z; if (f < g) { g = f; j = i; }
			}
			shadowtest2_dellight(j);
			ligvel[j] = ligvel[shadowtest2_numlights];
			if (!shadowtest2_numlights) shadowtest2_rendmode = 2;
			resetfps();
			ischanged = 3;
		}
	}
	if (keystatus[0xcf]) { ischanged = 3; } //End
	if (keystatus[0x32]) //M (toggle moving lights)
	{
		keystatus[0x32] = 0;
		if ((shadowtest2_numlights > 0) && (ligvel[0].x == 0.0) && (ligvel[0].y == 0.0) && (ligvel[0].z == 0.0))
		{
			for(i=0;i<shadowtest2_numlights;i++)
			{
				//if (shadowtest2_light[i].sprilink < 0) continue;
					//UNIFORM spherical randomization (see spherand.c)
				ligvel[i].z = (((double)rand())/32768.0)*2.0-1.0;
				f = (((double)rand())/32768.0)*(PI*2.0); ligvel[i].x = cos(f); ligvel[i].y = sin(f);
				f = sqrt(1.0-ligvel[i].z*ligvel[i].z); ligvel[i].x *= f; ligvel[i].y *= f;
				f = 0.5f; ligvel[i].x *= f; ligvel[i].y *= f; ligvel[i].z *= f; //scale vector

				lp = &shadowtest2_light[i]; lp->f.x = ligvel[i].x; lp->f.y = ligvel[i].y; lp->f.z = ligvel[i].z;
			}
		}
		else
		{
			for(i=0;i<shadowtest2_numlights;i++)
			{
				//if (shadowtest2_light[i].sprilink < 0) continue;
				ligvel[i].x = 0.f; ligvel[i].y = 0.f; ligvel[i].z = 0.f;
			}
		}
	}
	if (keystatus[0x58]) //F12: capture startpos to clipbaord
	{
		char tbuf[1024];
		keystatus[0x58] = 0;
		sprintf(tbuf,"   startpos.x=%20.16f;startpos.y=%20.16f;startpos.z=%20.16f;\r"
						 "   startrig.x=%20.16f;startrig.y=%20.16f;startrig.z=%20.16f;\r"
						 "   startdow.x=%20.16f;startdow.y=%20.16f;startdow.z=%20.16f;\r"
						 "   startfor.x=%20.16f;startfor.y=%20.16f;startfor.z=%20.16f;\r",
						 cam.p.x,cam.p.y,cam.p.z,
						 cam.r.x,cam.r.y,cam.r.z,
						 cam.d.x,cam.d.y,cam.d.z,
						 cam.f.x,cam.f.y,cam.f.z);
		setclipboardtext(tbuf);
		MessageBox(ghwnd,"pos&ori copied to clipboard",prognam,MB_OK);
	}

	for(i=0;i<shadowtest2_numlights;i++)
	{
		dpoint3d dp2, norm;
		int hitsect, hitwall;

		if ((ligvel[i].x == 0.0) && (ligvel[i].y == 0.0) && (ligvel[i].z == 0.0)) continue;
		ischanged |= 1;

		dp.x = shadowtest2_light[i].p.x;
		dp.y = shadowtest2_light[i].p.y;
		dp.z = shadowtest2_light[i].p.z;
		f = dtim*4.0;
		dp2.x = ligvel[i].x*f;
		dp2.y = ligvel[i].y*f;
		dp2.z = ligvel[i].z*f;
		if (build2_hitmove(&shadowtest2_light[i].sect,&dp,&dp2,0.05,1,&hitsect,&hitwall))
		{
			if (hitwall < 0)
			{
				sect_t *sec = &sst.sect[hitsect];
				norm.x = sec->grad[hitwall&1].x;
				norm.y = sec->grad[hitwall&1].y;
				norm.z = 1.f; if (hitwall == -1) { gnorm.x = -gnorm.x; gnorm.y = -gnorm.y; gnorm.z = -gnorm.z; }
				f = 1.0/sqrt(norm.x*norm.x + norm.y*norm.y + 1.0); norm.x *= f; norm.y *= f; norm.z *= f;
			}
			else if (!(hitwall&0x40000000))
			{
				wall_t *wal = sst.sect[hitsect].wall;
				norm.x = wal[hitwall].y - wal[wal[hitwall].n+hitwall].y;
				norm.y = wal[wal[hitwall].n+hitwall].x - wal[hitwall].x;
				norm.z = 0;
				f = 1.0/sqrt(norm.x*norm.x + norm.y*norm.y); norm.x *= f; norm.y *= f;
			}
			else
			{
				spri_t *spr = &sst.spri[hitwall&0x3fffffff];
				norm.x = spr->p.x-dp.x;
				norm.y = spr->p.y-dp.y;
				norm.z = spr->p.z-dp.z;
				f = 1.0/sqrt(norm.x*norm.x + norm.y*norm.y + norm.z*norm.z); norm.x *= f; norm.y *= f; norm.z *= f;
			}

			f = (ligvel[i].x*norm.x + ligvel[i].y*norm.y + ligvel[i].z*norm.z)*-2;
			ligvel[i].x += norm.x*f;
			ligvel[i].y += norm.y*f;
			ligvel[i].z += norm.z*f;

			lp = &shadowtest2_light[i];
			lp->f.x = ligvel[i].x; lp->f.y = ligvel[i].y; lp->f.z = ligvel[i].z;
		}
		shadowtest2_light[i].p.x = dp.x;
		shadowtest2_light[i].p.y = dp.y;
		shadowtest2_light[i].p.z = dp.z;

		j = shadowtest2_light[i].sprilink;
		if ((unsigned)j < (unsigned)sst.numspris)
		{
			sst.spri[j].p = shadowtest2_light[i].p;
			changesprisect(j,shadowtest2_light[i].sect);
		}
	}


	if (startdirectdraw((long *)&gdd.f,(long *)&gdd.p,(long *)&gdd.x,(long *)&gdd.y))
	{
		i = gdd.p*gdd.y+256;
		if ((i > zbuffersiz) || (!zbuffermem)) //Increase Z buffer size if too small
		{
			if (zbuffermem) free(zbuffermem);
			zbuffersiz = i;
			zbuffermem = (int *)malloc(zbuffersiz);
		}
			//zbuffer aligns its memory to the same pixel boundaries as the screen!
			//WARNING: Pentium 4's L2 cache has severe slowdowns when 65536-64 <= (zbufoff&65535) < 64
		zbufoff = (intptr_t)zbuffermem-gdd.f;
		//zbufoff = (((((intptr_t)zbuffermem)-gdd.f-128)+255)&~255)+128;

		if ((!shadowtest2_rendmode) || (shadowtest2_rendmode == 3))
		{
			for(i=0,p=gdd.f+zbufoff;i<gdd.y;i++,p+=gdd.p) memset8((void *)p,0x7f7f7f7f,gdd.x<<2);
		}

		cam.c.f = gdd.f; cam.c.p = gdd.p; cam.c.x = gdd.x; cam.c.y = gdd.y;
		cam.z = cam.c; cam.z.f = (intptr_t)gdd.f+zbufoff;
		dcamp.x = (double)cam.p.x; dcamp.y = (double)cam.p.y; dcamp.z = (double)cam.p.z;
		dcamr.x = (double)cam.r.x; dcamr.y = (double)cam.r.y; dcamr.z = (double)cam.r.z;
		dcamd.x = (double)cam.d.x; dcamd.y = (double)cam.d.y; dcamd.z = (double)cam.d.z;
		dcamf.x = (double)cam.f.x; dcamf.y = (double)cam.f.y; dcamf.z = (double)cam.f.z;

			//Hacks to link with Build2
		extern playerstruct_t *gdps; gdps = &sst.p[0];
		sst.p[0].ipos.x = cam.p.x; sst.p[0].ipos.y = cam.p.y; sst.p[0].ipos.z = cam.p.z;
		sst.p[0].irig.x = cam.r.x; sst.p[0].irig.y = cam.r.y; sst.p[0].irig.z = cam.r.z;
		sst.p[0].idow.x = cam.d.x; sst.p[0].idow.y = cam.d.y; sst.p[0].idow.z = cam.d.z;
		sst.p[0].ifor.x = cam.f.x; sst.p[0].ifor.y = cam.f.y; sst.p[0].ifor.z = cam.f.z;
		sst.p[0].cursect = cursect;
		sst.p[0].ghx = cam.h.x; sst.p[0].ghy = cam.h.y; sst.p[0].ghz = cam.h.z;
		sst.p[0].editmode = 3.0; sst.p[0].compact2d = 0.0;

		switch (shadowtest2_rendmode)
		{
			case 0: build2_render((tiletype *)&cam.c,cam.z.f-cam.c.f,cursect,&dcamp,&dcamr,&dcamd,&dcamf,cam.h.x,cam.h.y,cam.h.z); break;
			case 1: case 2:
				if (curgcnt != 0x7fffffff) clearscreen(0);
				draw_hsr_polymost(&cam,&sst,&sst.p[0],cursect);
				if ((eyepoln) && (!keystatus[0x38])) htrun(eyepol_drawfunc,0,eyepoln,shadowtest2_numcpu); //Empty FIFO
				drawsprites();

				if (shadowtest2_rendmode == 1)
				{
					print6x8(&gcam.c,(gdd.x>>1)-60+7*8, 0,0xffffff,0,"cnt:%10d",curgcnt);
				 //print6x8(&gcam.c,(gdd.x>>1)-60+0*8, 8,0xffffff,0,"sectgotn:%10d",sectgotn);
				 //print6x8(&gcam.c,(gdd.x>>1)-60+2*8,16,0xffffff,0,"bunchmal:%10d",bunchmal);
				 //print6x8(&gcam.c,(gdd.x>>1)-60+5*8,24,0xffffff,0,"mpmal:%10d",mpmal);
				 //print6x8(&gcam.c,(gdd.x>>1)-60+4*8,32,0xffffff,0,"mphmal:%10d",mphmal);

						//debug bunches
					bunchverts_t twal[1024];
					double d, dx0, dy0, dx1, dy1;
					int twaln;
					for(i=bunchn-1;i>=0;i--)
					{
						twaln = prepbunch(i,twal);
						for(j=0;j<=twaln;j++)
						{
							d = 64.0;
							dx0 = -((twal[j+0].x-cam.p.x)*xformmats - (twal[j+0].y-cam.p.y)*xformmatc)*d + gdd.x*.50;
							dy0 = -((twal[j+0].x-cam.p.x)*xformmatc + (twal[j+0].y-cam.p.y)*xformmats)*d + gdd.y*.95;
							if (j < twaln)
							{
								dx1 = -((twal[j+1].x-cam.p.x)*xformmats - (twal[j+1].y-cam.p.y)*xformmatc)*d + gdd.x*.50;
								dy1 = -((twal[j+1].x-cam.p.x)*xformmatc + (twal[j+1].y-cam.p.y)*xformmats)*d + gdd.y*.95;
								drawline2d(&cam.c,dx0,dy0,dx1,dy1,0xc0ffff);

								d = ((GetTickCount()+i*1266)&2047)*(1.0/2048.0);
								print6x8(&cam.c,(dx1-dx0)*d+dx0-3,(dy1-dy0)*d+dy0-4,0xc0ffff,-1,"%d",i);
							}
						}
					}
					drawline2d(&cam.c,gdd.x*.50,gdd.y*.95,gdd.x*.50,gdd.y*.95 - 16,0xffffff);
					print6x8(&gcam.c,gdd.x*.50-3,gdd.y*.95-4,0xffffff,-1,"o");
					for(j=0;j<bunchn;j++)
					{
						print6x8(&gcam.c,15-12,50+j*8,0xffffff,-1,"%d",j);
						print6x8(&gcam.c,15+j*8,50-12,0xffffff,-1,"%d",j);
						for(i=0;i<j;i++) print6x8(&gcam.c,15+i*8,50+j*8,0xffff80,-1,"%d",bunchgrid[(((j-1)*j)>>1)+i]);
					}
				}

				break;
			case 3:
				{
				float x0, y0, z0, x1, y1, z1, z2, z3;
				int s, w, w2, col;
				gcam = cam;
				clearscreen(0);
				for(s=0;s<sst.numsects;s++)
					for(w=0;w<sst.sect[s].n;w++)
					{
						w2 = sst.sect[s].wall[w].n+w;

						x0 = sst.sect[s].wall[w].x;
						y0 = sst.sect[s].wall[w].y;
						z0 = getslopez(&sst.sect[s],0,x0,y0);
						z1 = getslopez(&sst.sect[s],1,x0,y0);

						x1 = sst.sect[s].wall[w2].x;
						y1 = sst.sect[s].wall[w2].y;
						z2 = getslopez(&sst.sect[s],0,x1,y1);
						z3 = getslopez(&sst.sect[s],1,x1,y1);

						if (sst.sect[s].wall[w].ns < 0) col = 0xffffff; else col = 0xff0000;
						drawline3d(&cam,x0,y0,z0,x1,y1,z2,col);
						drawline3d(&cam,x0,y0,z1,x1,y1,z3,col);
						drawline3d(&cam,x0,y0,z0,x0,y0,z1,col);
						drawline3d(&cam,x1,y1,z2,x1,y1,z3,col);
					}
				}
				break;
			case 4:
				shadowtest2_rendmode = 2; draw_hsr_polymost(&cam,&sst,&sst.p[0],cursect); shadowtest2_rendmode = 4;

				if (ischanged == 3)
				{
					cam_t ncam; ncam = cam;
					ischanged = 0;
					shadowtest2_ligpolreset(-1);
					for(glignum=0;glignum<shadowtest2_numlights;glignum++)
					{
						ncam.p = shadowtest2_light[glignum].p;
						draw_hsr_polymost(&ncam,&sst,&sst.p[0],shadowtest2_light[glignum].sect);
					}
				}
				else if (ischanged == 1)
				{
					cam_t ncam; ncam = cam;
					ischanged = 0;
					for(glignum=0;glignum<shadowtest2_numlights;glignum++)
					{
						if ((ligvel[glignum].x == 0.0) && (ligvel[glignum].y == 0.0) && (ligvel[glignum].z == 0.0)) continue;

						if (!shadowtest2_isgotsectintersect(glignum))
						{
								//Current view doesn't intersect sector list of previously rendered light polygon list of this light source
								//Even so, a check should still occur occasionally
							if ((rand()&255) > 10) continue;
						}

						shadowtest2_ligpolreset(glignum);
						ncam.p = shadowtest2_light[glignum].p;
						draw_hsr_polymost(&ncam,&sst,&sst.p[0],shadowtest2_light[glignum].sect);
					}
				}

				shadowtest2_setcam(&cam);
				htrun(drawpollig,0,eyepoln,shadowtest2_numcpu);

#if 0
				for(i=eyepoln-1;i>=0;i--)
				{
					int v, v0, v1, v2;
					v0 = eyepol[i].vert0; v1 = eyepol[i+1].vert0;
					//drawpolsol(&cam,&eyepolv[v0],v1-v0,3);

#if 0
					x = 0; y = 0; j = 0;
					for(v=v0;v<v1;v++) { x += eyepolv[v].x; y += eyepolv[v].y; j++; }
					if (j) { x /= j; y /= j; }
#else
						//Find centroid of polygon
					fx = 0; fy = 0; f = 0;
					for(i0=v1-1,i1=v0;i1<v1;i0=i1,i1++)
					{
						float fx0, fy0, fx1, fy1;
						fx0 = eyepolv[i0].x; fy0 = eyepolv[i0].y;
						fx1 = eyepolv[i1].x; fy1 = eyepolv[i1].y;
						fx += ((fx0+fx1)*fx0 + fx1*fx1)*(fy1-fy0);
						fy += ((fy0+fy1)*fy0 + fy1*fy1)*(fx0-fx1);
						f += (fx0+fx1)*(fy1-fy0);
					}
					f = 1.0/(f*3.0); x = (int)(fx*f); y = (int)(fy*f);
#endif

					for(lig=0;lig<shadowtest2_numlights;lig++)
					{
						lp = &shadowtest2_light[lig];
						for(j=lp->lighashead[lighash(eyepol[i].b2sect,eyepol[i].b2wall,eyepol[i].b2slab)];j>=0;j=lp->ligpol[j].b2hashn)
						{
							if ((lp->ligpol[j].b2sect != eyepol[i].b2sect) || (lp->ligpol[j].b2wall != eyepol[i].b2wall) || (lp->ligpol[j].b2slab != eyepol[i].b2slab))
								{ print6x8(&gcam.c,x,y,0xffffff,0,"xx"); y += 8; continue; }
							print6x8(&gcam.c,x,y,0xffffff,0,"%d",lig); y += 8;
						}
					}

					for(v=v1-1,v2=v0;v2<v1;v=v2,v2++) { drawline2d(&cam.c,eyepolv[v].x,eyepolv[v].y,eyepolv[v2].x,eyepolv[v2].y,0xffffff); }
				}
#endif
#if 0
				for(lig=0;lig<shadowtest2_numlights;lig++)
				{
					lp = &shadowtest2_light[lig];
					for(i=lp->ligpoln-1;i>=0;i--)
					{
						int v, v0, v1, v2;
						v0 = lp->ligpol[i].vert0; v1 = lp->ligpol[i+1].vert0;
						drawpolsol(&cam,&lp->ligpolv[v0],v1-v0,lig);
						for(v=v1-1,v2=v0;v2<v1;v=v2,v2++) drawline3d(&cam,lp->ligpolv[v].x,lp->ligpolv[v].y,lp->ligpolv[v].z,lp->ligpolv[v2].x,lp->ligpolv[v2].y,lp->ligpolv[v2].z,0xffffff);
					}
				}
#endif

				drawsprites();
				break;
		}

		print6x8(&gcam.c,gdd.x-192,16,0xffffff,-1,"%10.6f%10.6f%10.6f",cam.p.x,cam.p.y,cam.p.z);
		print6x8(&gcam.c,gdd.x-192,26,0xffffff,-1,"%10.6f%10.6f%10.6f",cam.r.x,cam.r.y,cam.r.z);
		print6x8(&gcam.c,gdd.x-192,36,0xffffff,-1,"%10.6f%10.6f%10.6f",cam.d.x,cam.d.y,cam.d.z);
		print6x8(&gcam.c,gdd.x-192,46,0xffffff,-1,"%10.6f%10.6f%10.6f",cam.f.x,cam.f.y,cam.f.z);

		f = 1.f;
		dpoint3d ncamp; ncamp.x = (double)cam.p.x*f; ncamp.y = (double)cam.p.y*f; ncamp.z = (double)cam.p.z*f;
		drawpoly_setup(                           (tiletype *)&cam.c.f,cam.z.f-cam.c.f,&ncamp,&dcamr,&dcamd,&dcamf,cam.h.x,cam.h.y,cam.h.z);
		drawcone_setup(cputype,shadowtest2_numcpu,(tiletype *)&cam.c.f,cam.z.f-cam.c.f,&ncamp,&dcamr,&dcamd,&dcamf,cam.h.x,cam.h.y,cam.h.z);
		 drawkv6_setup(&drawkv6_frame,            (tiletype *)&cam.c.f,cam.z.f-cam.c.f,&ncamp,&dcamr,&dcamd,&dcamf,cam.h.x,cam.h.y,cam.h.z);
		for(i=0;i<shadowtest2_numlights;i++)
		{
			if (shadowtest2_light[i].sprilink >= 0) continue;
			drawsph(shadowtest2_light[i].p.x*f,shadowtest2_light[i].p.y*f,shadowtest2_light[i].p.z*f,f*.05,
				(min(max((int)(shadowtest2_light[i].rgb[0]*256),0),255)    ) +
				(min(max((int)(shadowtest2_light[i].rgb[1]*256),0),255)<< 8) +
				(min(max((int)(shadowtest2_light[i].rgb[2]*256),0),255)<<16),38.4);
		}

			//FPS counter
		fpsdtim[numframes&(FPSSIZ-1)] = dtim; numframes++;

			//Fast sort when already sorted... otherwise slow!
		i = min(numframes,FPSSIZ)-1;
		for(j=0;j<i;j++)
			if (fpsdtim[fpsind[j]] > fpsdtim[fpsind[j+1]])
			{
				y = fpsind[j+1];
				for(x=j;x>=0;x--) { fpsind[x+1] = fpsind[x]; if (fpsdtim[fpsind[x]] <= fpsdtim[y]) break; }
				fpsind[x+1] = y;
			}
		f = (fpsdtim[fpsind[i>>1]] + fpsdtim[fpsind[(i+1)>>1]])*.5; //Median
		print6x8(&gcam.c,0,0,0xffffff,0,"%6.2f fps",1.0/f);
		print6x8(&gcam.c,0,8,0xffffff,0,"%6.2f ms/f",f*1000.0);
		for(i=0;i<FPSSIZ;i++)
		{
			drawpix(&gcam.c,i,f*10000.0,0xe06060);
			drawpix(&gcam.c,i,min(fpsdtim[(numframes+i)&(FPSSIZ-1)]*10000.0,gdd.y-1),0xc0c0c0);
		}
		print6x8(&gcam.c,0,16,0xffffff,0,"rendmode:%d,MT:%d",shadowtest2_rendmode+1,shadowtest2_numcpu);

		stopdirectdraw();
		nextpage();
	}
}*/
#endif
void drawpollig(int ei) {

    __declspec(align(16)) static const float dpqmulval[4] = {0,1,2,3}, dpqfours[4] = {4,4,4,4};
    __declspec(align(16)) float qamb[4]; //holder for SSE to avoid degenerates
#define PR0_USEFLOAT 0
#define PR1_USEFLOAT 1
#if (PR0_USEFLOAT != 0)
    typedef struct { int y0, y1; float pos, inc; } rast_t;
#else
    typedef struct { int y0, y1, pos, inc; } rast_t;
#endif
    rast_t *rast, rtmp;
#if (PR1_USEFLOAT != 0)
    typedef struct { int y0, y1; float pos, inc; } lrast_t;
#else
    typedef struct { int y0, y1, pos, inc; } lrast_t;
#endif
#define LRASTMAX 8192             //FIX:make dynamic!
    lrast_t lrast[LRASTMAX];          //FIX:make dynamic! FIX:make thread safe!
    lrast_t *prast[LIGHTMAX], lrtmp; //FIX:make dynamic!
    int plnum[LIGHTMAX], lpn3[LIGHTMAX], lpn4[LIGHTMAX], lpn5[LIGHTMAX], plnumi; //FIX:make dynamic!
    //typedef struct { float n2, d2, n1, d1, n0, d0, filler0[2], bsc, gsc, rsc, filler1[1]; } hlighterp_t;
    __declspec(align(16)) hlighterp_t hl[LIGHTMAX];
    hlighterp_t *hlptr, *liglst[LIGHTMAX];
    int liglstn;
    static const float fone = 1.f;
    __declspec(align(16)) float qlig[4], g_rgbmul[4];
    __declspec(align(8)) unsigned short qs[4], qsi[4];
    __declspec(align(16)) const double dmagic[2] = {6755399441055744.0,6755399441055744.0}; //3*2^51
    __int64 qddmul, qmask;
    int lmask0, lmask1;
    point3d tp, norm;
    point3d *lvt, *lvt2;
    point2d *pt, *lpt;
    int lig, olignum;
    lightpos_t *lp;
    float f, g, ox, oy, oz, d, u, v, vx, vy, di8, ui8, vi8, od, *ouvmat;
    __declspec(align(8)) int iw[2], iwi[2];
    intptr_t l, padd;
    int id, idi, oid, oidi, p, p2, sy, *zptr, *lptr, xalign;
    int ttps, ymsk, xmsk, xshift, ttf, ttp, rgbmul; //, nrgbmul;
    int i, j, k, x, xe, xe2, xe3, iy0, iy1, pn, pn2, pn3, pn4, ymin, ymax, lnum, lpn, olpn2, lpn2, col;

    pt = &eyepolv[eyepol[ei].vert0]; pn = eyepol[ei+1].vert0-eyepol[ei].vert0;

    i =            pn  *sizeof(rast_t );
    j = ligpolmaxvert*2*sizeof(point2d);
    k = ligpolmaxvert  *sizeof(point3d);
    l = (intptr_t)_alloca(i+j+k);
    rast = (rast_t  *)(l);
    lpt  = (point2d *)(l+i);
    lvt2 = (point3d *)(l+i+j);

    ymin = 0x7fffffff; ymax = 0x80000000; pn2 = 0; j = -1; iy1 = 0;
    for(i=0;i<pn;i++)
    {
        if (i != j)                  iy0 = (int)min(max(ceil(pt[i].y),0),gcam.c.y); else iy0 = iy1;
        j = i+1; if (j >= pn) j = 0; iy1 = (int)min(max(ceil(pt[j].y),0),gcam.c.y); if (iy0 == iy1) continue;
        if (iy0 < iy1) { rast[pn2].y0 = iy0; rast[pn2].y1 = iy1; if (iy0 < ymin) ymin = iy0; }
        else { rast[pn2].y0 = iy1; rast[pn2].y1 = iy0; if (iy0 > ymax) ymax = iy0; }
#if (PR0_USEFLOAT != 0)
        rast[pn2].inc = (pt[j].x - pt[i].x)/(pt[j].y - pt[i].y);
        rast[pn2].pos = ((float)rast[pn2].y0 - pt[i].y)*rast[pn2].inc + pt[i].x;
#else
        g = (pt[j].x - pt[i].x)/(pt[j].y - pt[i].y);
        f = ((float)rast[pn2].y0 - pt[i].y)*g + pt[i].x;
        rast[pn2].inc = ((int)(g*65536.0));
        rast[pn2].pos = ((int)(f*65536.0))+65535;
#endif
        pn2++;
    }

    ouvmat = eyepol[ei].ouvmat;

    f = 1.0/2.f;
    g_rgbmul[0] = ((eyepol[ei].curcol    )&255)*f;
    g_rgbmul[1] = ((eyepol[ei].curcol>> 8)&255)*f;
    g_rgbmul[2] = ((eyepol[ei].curcol>>16)&255)*f;
    g_rgbmul[3] = 0.f;

    //Translate & Rotate
    ox = eyepol[ei].norm.x; oy = eyepol[ei].norm.y; oz = eyepol[ei].norm.z;
    norm.x = ox*gcam.r.x + oy*gcam.r.y + oz*gcam.r.z;
    norm.y = ox*gcam.d.x + oy*gcam.d.y + oz*gcam.d.z;
    norm.z = ox*gcam.f.x + oy*gcam.f.y + oz*gcam.f.z;

    lpn2 = 0; plnum[0] = 0; plnumi = 0; olignum = -1;
    if (!(eyepol[ei].flags&1))
    {
        for(lig=0;lig<shadowtest2_numlights;lig++)
        {
            lp = &shadowtest2_light[lig];

            if ((!(lp->flags&1)) || (!shadowtest2_useshadows))
            {
                //No shadow mode needs back-face cull
                f = ouvmat[0]*gcam.h.x + ouvmat[3]*gcam.h.y + ouvmat[6];
#if (USEINTZ)
                if (((slightpos[lig].x*norm.x + slightpos[lig].y*norm.y + slightpos[lig].z*norm.z)*f*(1048576.0*256.0) < gcam.h.z*norm.z) == (f >= 0)) continue;
#else
                if ((((slightpos[lig].x*norm.x + slightpos[lig].y*norm.y + slightpos[lig].z*norm.z)*f <= norm.z) == (f > 0)) && (fabs(f) > 1e-4)) continue;
#endif

                prast[plnumi] = &lrast[lpn2];
                if (plnumi > 0) { plnum[plnumi-1] = lpn2-olpn2; } olpn2 = lpn2;

                prepligramp(ouvmat,&norm,lig,&hl[plnumi]);
                plnumi++; if (plnumi >= LIGHTMAX) return;

                lrast[lpn2].y0 = ymin; lrast[lpn2].inc = 0;
                lrast[lpn2].y1 = ymax; lrast[lpn2].pos = 0;
                lpn2++; if (lpn2 >= LRASTMAX) return;
                continue;
            }

            for(l=lp->lighashead[lighash(eyepol[ei].b2sect,eyepol[ei].b2wall,eyepol[ei].b2slab)];l>=0;l=lp->ligpol[l].b2hashn)
            {
                if ((lp->ligpol[l].b2sect != eyepol[ei].b2sect) || (lp->ligpol[l].b2wall != eyepol[ei].b2wall) || (lp->ligpol[l].b2slab != eyepol[ei].b2slab)) continue;

                lvt = &lp->ligpolv[lp->ligpol[l].vert0]; lnum = lp->ligpol[l+1].vert0-lp->ligpol[l].vert0;

                for(i=0;i<lnum;i++)
                {
                    ox = lvt[i].x-gcam.p.x; oy = lvt[i].y-gcam.p.y; oz = lvt[i].z-gcam.p.z;
                    lvt2[i].x = ox*gcam.r.x + oy*gcam.r.y + oz*gcam.r.z;
                    lvt2[i].y = ox*gcam.d.x + oy*gcam.d.y + oz*gcam.d.z;
                    lvt2[i].z = ox*gcam.f.x + oy*gcam.f.y + oz*gcam.f.z;
                }

                lpn = 0;
                for(i=lnum-1,j=0;j<lnum;i=j,j++)
                {
                    if (lvt2[i].z >= SCISDIST)
                    {
                        f = gcam.h.z/lvt2[i].z;
                        lpt[lpn].x = lvt2[i].x*f + gcam.h.x;
                        lpt[lpn].y = lvt2[i].y*f + gcam.h.y;
                        lpn++;
                    }
                    if ((lvt2[i].z >= SCISDIST) != (lvt2[j].z >= SCISDIST))
                    {
                        f = (SCISDIST-lvt2[i].z)/(lvt2[j].z-lvt2[i].z); g = gcam.h.z/SCISDIST;
                        lpt[lpn].x = ((lvt2[j].x-lvt2[i].x)*f + lvt2[i].x)*g + gcam.h.x;
                        lpt[lpn].y = ((lvt2[j].y-lvt2[i].y)*f + lvt2[i].y)*g + gcam.h.y;
                        lpn++;
                    }
                }
                if (lpn < 3) continue;

                //use lpt,lpn
                j = -1; iy1 = 0;
                for(i=0;i<lpn;i++)
                {
                    if (i != j)                   iy0 = (int)min(max(ceil(lpt[i].y),ymin),ymax); else iy0 = iy1;
                    j = i+1; if (j >= lpn) j = 0; iy1 = (int)min(max(ceil(lpt[j].y),ymin),ymax); if (iy0 == iy1) continue;

                    if (lig != olignum)
                    {
                        olignum = lig; prast[plnumi] = &lrast[lpn2];
                        if (plnumi > 0) { plnum[plnumi-1] = lpn2-olpn2; } olpn2 = lpn2;
                        prepligramp(ouvmat,&norm,lig,&hl[plnumi]);
                        plnumi++; if (plnumi >= LIGHTMAX) return;
                    }

                    if (iy0 < iy1) { lrast[lpn2].y0 = iy0; lrast[lpn2].y1 = iy1; }
                    else { lrast[lpn2].y0 = iy1; lrast[lpn2].y1 = iy0; }
#if (PR1_USEFLOAT != 0)
                    lrast[lpn2].inc = (lpt[j].x - lpt[i].x)/(lpt[j].y - lpt[i].y);
                    lrast[lpn2].pos = ((float)lrast[lpn2].y0 - lpt[i].y)*lrast[lpn2].inc + lpt[i].x+.5; //FIX:what makes this .5 hack necessary?
#else
                    g = (lpt[j].x - lpt[i].x)/(lpt[j].y - lpt[i].y);
                    f = ((float)lrast[lpn2].y0 - lpt[i].y)*g + lpt[i].x;
                    lrast[lpn2].inc = ((int)(g*65536.0));
                    lrast[lpn2].pos = ((int)(f*65536.0))+65535;
#endif
                    lpn2++; if (lpn2 >= LRASTMAX) return;
                }
            }
        }
        if (plnumi > 0) plnum[plnumi-1] = lpn2-olpn2;
        for(i=4-1;i>=0;i--) qamb[i] = g_qamb[i];
    }
	// If use this only - all will be brighht.
    else //parallaxing sky does not use shadows
    {
#if (USEGAMMAHACK == 0)
        f = 256.f;
#else
        f = 256.f*16384.f;
#endif
        qamb[0] = f; qamb[1] = f; qamb[2] = f; qamb[3] = 0.f;  // ambient parallax..
    }

    //Shell sort top y's
    for(k=(pn2>>1);k;k>>=1)
        for(i=0;i<pn2-k;i++)
            for(j=i;j>=0;j-=k)
            {
                if (rast[j].y0 <= rast[j+k].y0) break;
                rtmp = rast[j]; rast[j] = rast[j+k]; rast[j+k] = rtmp;
            }
    pn3 = 0; pn4 = 0;

    for(l=plnumi-1;l>=0;l--)
    {
        //Shell sort top y's for light polies
        for(k=(plnum[l]>>1);k;k>>=1)
            for(i=0;i<plnum[l]-k;i++)
                for(j=i;j>=0;j-=k)
                {
                    if (prast[l][j].y0 <= prast[l][j+k].y0) break;
                    lrtmp = prast[l][j]; prast[l][j] = prast[l][j+k]; prast[l][j+k] = lrtmp;
                }
        lpn3[l] = 0; lpn4[l] = 0;
    }

    di8 = ouvmat[0]*FLATSTEPSIZ;
    ui8 = ouvmat[1]*FLATSTEPSIZ;
    vi8 = ouvmat[2]*FLATSTEPSIZ;
    rgbmul = ((eyepol[ei].curcol&0xffffff)|0x80000000);
    ttp = eyepol[ei].tpic->tt.p; ttf = eyepol[ei].tpic->tt.f; i = bsr(ttp);
    xmsk = (1<<bsr(eyepol[ei].tpic->tt.x))-1; xmsk <<= 2; xshift = bsr(eyepol[ei].tpic->tt.x)+2;
    ymsk = (1<<bsr(eyepol[ei].tpic->tt.y))-1; ymsk <<= i;
    ttps = 16-i;
    qddmul = (__int64)((ttp<<16)+4);
    qmask = (__int64)(((eyepol[ei].tpic->tt.y-1)<<16) + (eyepol[ei].tpic->tt.x-1));
    lmask0 = ((eyepol[ei].tpic->tt.x-1)<<2); lmask1 = ~lmask0;

    for(sy=ymin;sy<ymax;sy++)
    {
        for(i=pn3-1;i>=0;i--)
        {
            if (sy >= rast[i].y1)
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
        while ((pn4 < pn2) && (sy >= rast[pn4].y0))
        {
            rtmp = rast[pn4];
            for(j=pn3;(j > 0) && (rast[j-1].pos > rtmp.pos);j--) rast[j] = rast[j-1];
            rast[j] = rtmp;

            pn3++; pn4++;
        }

        //Same code as above, but for lights
        for(l=plnumi-1;l>=0;l--)
        {
            for(i=lpn3[l]-1;i>=0;i--)
            {
                if (sy >= prast[l][i].y1)
                {     //Delete line segments
                    lpn3[l]--;
                    for(j=i;j<lpn3[l];j++) prast[l][j] = prast[l][j+1];
                }
                else if (prast[l][i+1].pos < prast[l][i].pos)
                {     //Refresh sort (needed for degenerate poly/intersections)
                    lrtmp = prast[l][i];
                    for(j=i+1;(j < lpn3[l]) && (prast[l][j].pos < lrtmp.pos);j++) prast[l][j-1] = prast[l][j];
                    prast[l][j-1] = lrtmp;
                }
            }
            //Insert line segments // no lights without it.
            while ((lpn4[l] < plnum[l]) && (sy >= prast[l][lpn4[l]].y0))
            {
                lrtmp = prast[l][lpn4[l]];
                for(j=lpn3[l];(j > 0) && (prast[l][j-1].pos > lrtmp.pos);j--) prast[l][j] = prast[l][j-1];
                prast[l][j] = lrtmp;

                lpn3[l]++; lpn4[l]++;
            }

            lpn5[l] = 0;
        }

        if (pn3)
        {
            //prep sy: some lightmap calc.
            i = sy*sy;
            for(lig=plnumi-1;lig>=0;lig--)
            {
#if (USENEWLIGHT == 0)
                hl[lig].d1 = hl[lig].glk[1]*sy + hl[lig].glk[2]; hl[lig].d0 = hl[lig].glk[3]*i + hl[lig].glk[ 4]*sy + hl[lig].glk[ 5];
                hl[lig].n1 = hl[lig].glk[7]*sy + hl[lig].glk[8]; hl[lig].n0 = hl[lig].glk[9]*i + hl[lig].glk[10]*sy + hl[lig].glk[11];
#else
                hl[lig].gk2[0] = hl[lig].gk[0];  hl[lig].gk2[4] = sy*hl[lig].gk[1]  + hl[lig].gk[3]; hl[lig].gk2[8] = i*hl[lig].gk[2] + sy*hl[lig].gk[4] + hl[lig].gk[5];
                hl[lig].gk2[1] = hl[lig].gk[9];  hl[lig].gk2[5] = sy*hl[lig].gk[10] + hl[lig].gk[11];
                hl[lig].gk2[2] = hl[lig].gk[6];  hl[lig].gk2[6] = sy*hl[lig].gk[7]  + hl[lig].gk[8];
                hl[lig].gk2[3] = hl[lig].gk[13]; hl[lig].gk2[7] = sy*hl[lig].gk[14] + hl[lig].gk[15];
#endif
            }
            i = gcam.c.p*sy;
            zptr = (int *)(gcam.z.f+i);
            lptr = (int *)(gcam.c.f+i);
        }
        //Draw hlines xor style
        for(i=0;i<pn3;i+=2)
        {
#if (PR0_USEFLOAT != 0)
            x  = (int)min(max(rast[i  ].pos,0.f),(float)gcam.c.x);
            xe = (int)min(max(rast[i+1].pos,0.f),(float)gcam.c.x);
#else
            x  = (int)min(max(rast[i  ].pos>>16,0),gcam.c.x);
            xe = (int)min(max(rast[i+1].pos>>16,0),gcam.c.x);
#endif

            //Prepare texture mapping
            if (di8 < 0) j = ((x-xe+1)&(FLATSTEPSIZ-1)); else j = 0; //Hack to avoid horizon crossing artifact from interpolation
            xalign = x-j; vx = (float)xalign; vy = (float)sy;
            d = ouvmat[0]*vx + ouvmat[3]*vy + ouvmat[6]; f = 1.0/d;
            u = ouvmat[1]*vx + ouvmat[4]*vy + ouvmat[7];
            v = ouvmat[2]*vx + ouvmat[5]*vy + ouvmat[8];
            d += di8;
            id    = (int)(  f);
            iw[0] = (int)(u*f);
            iw[1] = (int)(v*f);
            if (j)
            {
                f = 1.0/d; d += di8; u += ui8; v += vi8;
                idi    = ((((int)(  f))-id   )>>LFLATSTEPSIZ); id    += idi   *j;
                iwi[0] = ((((int)(u*f))-iw[0])>>LFLATSTEPSIZ); iw[0] += iwi[0]*j;
                iwi[1] = ((((int)(v*f))-iw[1])>>LFLATSTEPSIZ); iw[1] += iwi[1]*j;
            }

            //Render Z's
            padd = (intptr_t)&zptr[xe]; p = ((x-xe)<<2);
#if 0
            od = d; oid = id; oidi = idi;
            p2 = min(p+((FLATSTEPSIZ-j)<<2),0); goto zbuf_in2it;
            do
            {
                f = 1.0/d; d += di8;
                idi = ((((int)f)-id)>>LFLATSTEPSIZ);
                p2 = min(p+(FLATSTEPSIZ<<2),0);
            zbuf_in2it: do { *(int *)(padd+p) = id;/*FIX:USEINTZ only!*/ id += idi; p += 4; } while (p < p2);
            } while (p < 0);
            d = od; id = oid; idi = oidi;
#else
            vx = (float)x;
            _asm
                    {
                    mov eax, ouvmat
                    mov ecx, p
                    mov edx, padd

                    movss xmm0, vx
                    movss xmm1, [eax]     ;xmm1: ouvmat[0]
                    movss xmm2, vy
                    mulss xmm0, xmm1
                    mulss xmm2, [eax+3*4]
                    addss xmm0, xmm2
                    addss xmm0, [eax+6*4] ;xmm0: ouvmat[0]*vx + ouvmat[3]*vy + ouvmat[6]

                    add edx, ecx
                    test edx, 12
                    jz short zbufskp1
                    zbufbeg1: rcpss xmm2, xmm0
                    addss xmm0, xmm1
#if (USEINTZ)
                    cvttss2si eax, xmm2
                    mov [edx], eax
#else
                    movss [edx], xmm2
#endif
                    add edx, 4
                    add ecx, 4
                    jge short zbufend
                    test edx, 12
                    jnz short zbufbeg1
                    zbufskp1: sub edx, ecx

                    shufps xmm1, xmm1, 0
                    shufps xmm0, xmm0, 0
                    movaps xmm2, xmm1
                    mulps xmm2, dpqmulval ;{0,1,2,3}
                    mulps xmm1, dpqfours  ;{4,4,4,4}
                    addps xmm0, xmm2

                    add ecx, 16
                    jg short zbufend1

                    zbufbeg4: rcpps xmm2, xmm0
                    addps xmm0, xmm1
#if ((USESSE2 != 0) || (!USEINTZ))
#if (USEINTZ)
                    cvttps2dq xmm2, xmm2
#endif
                    movaps [edx+ecx-16], xmm2
#else
                    cvttps2pi mm0, xmm2
                    movhlps xmm2, xmm2
                    cvttps2pi mm1, xmm2
                    movq [edx+ecx-16], mm0
                    movq [edx+ecx-8], mm1
#endif
                    add ecx, 16
                    jle short zbufbeg4

                    zbufend1: sub ecx, 16
                    jz short zbufend
                    rcpps xmm2, xmm0

#if ((USESSE2 != 0) || (!USEINTZ))
#if (USEINTZ)
                    cvttps2dq xmm2, xmm2
#endif
                    zbufend2: movss [edx+ecx], xmm2
#else
                zbufend2: cvttss2si eax, xmm2
                    mov [edx+ecx], eax
#endif
                    shufps xmm2, xmm2, 0x39
                    add ecx, 4
                    jl short zbufend2
                    zbufend:
#if ((USESSE2 == 0) && (USEINTZ))
                    emms
#endif
                    }
#endif

            while (x < xe)
            {
                xe2 = xe;

                liglstn = 0;
                for(l=plnumi-1;l>=0;l--)
                {
                    while (lpn5[l] < lpn3[l])
                    {
#if (PR1_USEFLOAT != 0)
                        j = ((int)prast[l][lpn5[l]].pos);
#else
                        j = (prast[l][lpn5[l]].pos>>16);
#endif
                        if (j <= x) lpn5[l]++; else { if (j+1 < xe2) xe2 = j+1; break; }
                    }
                    if (lpn5[l]&1) { liglst[liglstn] = &hl[l]; liglstn++; }
                }

#if 0
                qlig[0] = qamb[0]; qlig[1] = qamb[1]; qlig[2] = qamb[2];
                for(lig=liglstn-1;lig>=0;lig--)
                {
                    hlptr = liglst[lig];
#if (USENEWLIGHT == 0)
                f = ((hlptr->n2*x + hlptr->n1)*x + hlptr->n0) /
                    ((hlptr->d2*x + hlptr->d1)*x + hlptr->d0);
#else
                //see conelight.kc for derivation
                f = 1.0/sqrt((x*hlptr->gk2[0] + hlptr->gk2[4])*x + hlptr->gk2[8]); //k0
                f =         ((x*hlptr->gk2[1] + hlptr->gk2[5])*f + hlptr->gk[12])  //k1
                            *          (x*hlptr->gk2[2] + hlptr->gk2[6])*f                   //k2
                            *          (x*hlptr->gk2[3] + hlptr->gk2[7])*f                   //k3
                            *          (x*hlptr->gk2[3] + hlptr->gk2[7])*f;
                f = sqrt(max(f,0.0));
#endif
                qlig[0] += f*hlptr->bsc;
                qlig[1] += f*hlptr->gsc;
                qlig[2] += f*hlptr->rsc;
				}
                qs[0] = min((int)(qlig[0]*g_rgbmul[0]),32767);
                qs[1] = min((int)(qlig[1]*g_rgbmul[1]),32767);
                qs[2] = min((int)(qlig[2]*g_rgbmul[2]),32767);
#else
            	// Vectorized per-pixel lighting with quadratic falloff
                _asm
                        {
                        movaps xmm7, qamb
                        mov eax, liglstn
                        sub eax, 1
                        js short endlig0
                        cvtsi2ss xmm2, x

                        ;movss xmm3, xmm2 ;FIX
                        ;mulss xmm3, xmm3 ;FIX

                        beglig0: mov edx, liglst[eax*4]
#if (USENEWLIGHT == 0)
                        movss xmm0, hlighterp_t.n2[edx]
                        movss xmm1, hlighterp_t.d2[edx]
                        mulss xmm0, xmm2
                        mulss xmm1, xmm2
                        addss xmm0, hlighterp_t.n1[edx]
                        addss xmm1, hlighterp_t.d1[edx]
                        mulss xmm0, xmm2
                        mulss xmm1, xmm2
                        addss xmm0, hlighterp_t.n0[edx]
                        addss xmm1, hlighterp_t.d0[edx]
                        rcpss xmm1, xmm1
                        mulss xmm0, xmm1
                        ;divss xmm0, xmm1
#else
                        movss xmm0, xmm2
                        shufps xmm0, xmm0, 0 //<-- pshufd xmm0, sx, 0
                        mulps xmm0, hlighterp_t.gk2[edx+0]
                        addps xmm0, hlighterp_t.gk2[edx+4*4]
                        mulss xmm0, xmm2
                        addss xmm0, hlighterp_t.gk2[edx+8*4]
                        rsqrtss xmm1, xmm0
                        shufps xmm1, xmm1, 0
                        shufps xmm0, xmm0, 0xf9  ;xmm0:[k3 k3 k2 k1]
                        mulps xmm0, xmm1
                        addss xmm0, hlighterp_t.gk[edx+12*4]  ;xmm0:[k3 k3 k2 k1]      [k3 k2 k3 k1]
                        movhlps xmm1, xmm0       ;xmm1:[ ?  ? k3 k3]
                        mulps xmm0, xmm1
                        movss xmm1, xmm0
                        shufps xmm0, xmm0, 1 //<-- pshufd xmm0, sx, 0
                        mulss xmm0, xmm1
                        maxps xmm0, hligterp_maxzero
#endif
                        shufps xmm0, xmm0, 0
                        mulps xmm0, hlighterp_t.bsc[edx]
                        addps xmm7, xmm0
                        sub eax, 1
                        jns short beglig0

                        endlig0: mulps xmm7, g_rgbmul
#if (USEGAMMAHACK)
                        rsqrtps xmm7, xmm7
                        rcpps xmm7, xmm7
#endif
                        cvtps2pi mm7, xmm7
                        movhlps xmm7, xmm7
                        cvtps2pi mm6, xmm7
                        packssdw mm7, mm6
                        movq qs, mm7
                        emms
                        }
#endif

                //Render cols
                j = ((x-xalign)&(FLATSTEPSIZ-1)); if (j) { xe3 = min(x-j+FLATSTEPSIZ,xe2); goto shline_in2it; }
                do
                {
#if (1)
                    f = 1.0/d; d += di8; u += ui8; v += vi8; //slow&accurate
                    iwi[0] = ((((int)(u*f))-iw[0])>>LFLATSTEPSIZ);
                    iwi[1] = ((((int)(v*f))-iw[1])>>LFLATSTEPSIZ);
#else
                    _asm
                            {
                            movss xmm0, d
                            ;rcpss xmm1, xmm0 ;Texture mapping is too fuzzy with this :/
                            movss xmm1, fone
                            divss xmm1, xmm0 ;xmm1:1.0/d
                            addss xmm0, di8
                            movss xmm2, u
                            movss xmm3, v
                            addss xmm2, ui8
                            addss xmm3, vi8
                            movss d, xmm0
                            movss u, xmm2
                            movss v, xmm3
                            mulss xmm2, xmm1
                            mulss xmm3, xmm1

#if (USESSE2 == 0)
                    unpcklps xmm2, xmm3 ;fast&inaccurate
                    cvtps2pi mm0, xmm2
                    psubd mm0, iw
                    psrad mm0, LFLATSTEPSIZ
                    movq iwi, mm0
                    emms
#else
                    unpcklps xmm2, xmm3  ;fast&accurate

                    cvtps2pd xmm2, xmm2  ;NOTE:requires SSE2!
                    addpd xmm2, dmagic   ;NOTE:requires SSE2!

                    movd eax, xmm2       ;NOTE:requires SSE2!
                    sub eax, iw[0]
                    sar eax, LFLATSTEPSIZ
                    mov iwi[0], eax

                    movhlps xmm2, xmm2
                    movd eax, xmm2       ;NOTE:requires SSE2!
                    sub eax, iw[4]
                    sar eax, LFLATSTEPSIZ
                    mov iwi[4], eax
#endif
					}
#endif

                    xe3 = min(x+FLATSTEPSIZ,xe2);
                shline_in2it:;
#if 0
                    qlig[0] = qamb[0]; qlig[1] = qamb[1]; qlig[2] = qamb[2];
                    for(lig=liglstn-1;lig>=0;lig--)
                    {
                        hlptr = liglst[lig];
#if (USENEWLIGHT == 0)
                    f = ((hlptr->n2*xe3 + hlptr->n1)*xe3 + hlptr->n0) /
                        ((hlptr->d2*xe3 + hlptr->d1)*xe3 + hlptr->d0);
#else
                    //see conelight.kc for derivation
                    f = 1.0/sqrt((xe3*hlptr->gk2[0] + hlptr->gk2[4])*xe3 + hlptr->gk2[8]); //k0
                    f =         ((xe3*hlptr->gk2[1] + hlptr->gk2[5])*f + hlptr->gk[12])    //k1
                                *          (xe3*hlptr->gk2[2] + hlptr->gk2[6])*f                     //k2
                                *          (xe3*hlptr->gk2[3] + hlptr->gk2[7])*f                     //k3
                                *          (xe3*hlptr->gk2[3] + hlptr->gk2[7])*f;
                    f = sqrt(max(f,0.0));
#endif
                    qlig[0] += f*hlptr->bsc;
                    qlig[1] += f*hlptr->gsc;
                    qlig[2] += f*hlptr->rsc;
					}
                    qsi[0] = min((int)(qlig[0]*g_rgbmul[0]),32767);
                    qsi[1] = min((int)(qlig[1]*g_rgbmul[1]),32767);
                    qsi[2] = min((int)(qlig[2]*g_rgbmul[2]),32767);
                    _asm movq mm7, qsi
#else
                    _asm
                            {
                            movaps xmm7, qamb
                            mov eax, liglstn
                            sub eax, 1
                            js short endlig1
                            cvtsi2ss xmm2, xe3

                            beglig1: mov edx, liglst[eax*4]
#if (USENEWLIGHT == 0)
                            movss xmm0, hlighterp_t.n2[edx]
                            movss xmm1, hlighterp_t.d2[edx]
                            mulss xmm0, xmm2
                            mulss xmm1, xmm2
                            addss xmm0, hlighterp_t.n1[edx]
                            addss xmm1, hlighterp_t.d1[edx]
                            mulss xmm0, xmm2
                            mulss xmm1, xmm2
                            addss xmm0, hlighterp_t.n0[edx]
                            addss xmm1, hlighterp_t.d0[edx]
                            rcpss xmm1, xmm1
                            mulss xmm0, xmm1
                            ;divss xmm0, xmm1
#else
                            movss xmm0, xmm2
                            shufps xmm0, xmm0, 0 //<-- pshufd xmm0, sx, 0
                            mulps xmm0, hlighterp_t.gk2[edx+0]
                            addps xmm0, hlighterp_t.gk2[edx+4*4]
                            mulss xmm0, xmm2
                            addss xmm0, hlighterp_t.gk2[edx+8*4]
                            rsqrtss xmm1, xmm0
                            shufps xmm1, xmm1, 0
                            shufps xmm0, xmm0, 0xf9  ;xmm0:[k3 k3 k2 k1]
                            mulps xmm0, xmm1
                            addss xmm0, hlighterp_t.gk[edx+12*4]  ;xmm0:[k3 k3 k2 k1]      [k3 k2 k3 k1]
                            movhlps xmm1, xmm0       ;xmm1:[ ?  ? k3 k3]
                            mulps xmm0, xmm1
                            movss xmm1, xmm0
                            shufps xmm0, xmm0, 1 //<-- pshufd xmm0, sx, 0
                            mulss xmm0, xmm1
                            maxps xmm0, hligterp_maxzero
#endif
                            shufps xmm0, xmm0, 0
                            mulps xmm0, hlighterp_t.bsc[edx]
                            addps xmm7, xmm0
                            sub eax, 1
                            jns short beglig1

                            endlig1: mulps xmm7, g_rgbmul
#if (USEGAMMAHACK)
                            rsqrtps xmm7, xmm7
                            rcpps xmm7, xmm7
#endif
                            cvtps2pi mm7, xmm7
                            movhlps xmm7, xmm7
                            cvtps2pi mm6, xmm7
                            packssdw mm7, mm6
                            }
#endif
                    _asm
                            {
                            movq mm6, qs
                            psubw mm7, mm6
                            psraw mm7, LFLATSTEPSIZ

                            movq mm5, mm7 ;add 1 if negative to avoid overflow
                            psraw mm5, 15
                            psubw mm7, mm5
                            }
#if (0)
                    if (!gps->rendinterp)
                    {
                        do
                        {
                            j = *(int *)(((iw[1]>>ttps)&ymsk) + ((iw[0]>>14)&xmsk) + ttf);
                            _asm
                                    {
                                    punpcklbw mm0, j
                                    pmulhuw mm0, mm6
                                    psrlw mm0, 6
                                    packuswb mm0, mm0
                                    mov eax, lptr
                                    mov edx, x
                                    movd [eax+edx*4], mm0
                                    paddw mm6, mm7
                                    }
                            iw[0] += iwi[0]; iw[1] += iwi[1]; x++;
                        } while (x < xe3);
                    }
                    else
                    {
                        do
                        {
                            int r0, g0, b0, r1, g1, b1;
                            unsigned char *u0, *u1, *u2, *u3;
                            j = ((iw[1]>>ttps)&ymsk) + ttf;
                            u0 = (unsigned char *)(j + ( (iw[0]>>14)   &xmsk));
                            u1 = (unsigned char *)(j + (((iw[0]>>14)+4)&xmsk));
                            u2 = (unsigned char *)(u0+ttp);
                            u3 = (unsigned char *)(u1+ttp); j = (iw[0]&65535);
                            b0 = ((((int)u1[0]-(int)u0[0])*j)>>16) + (int)u0[0];
                            g0 = ((((int)u1[1]-(int)u0[1])*j)>>16) + (int)u0[1];
                            r0 = ((((int)u1[2]-(int)u0[2])*j)>>16) + (int)u0[2];
                            b1 = ((((int)u3[0]-(int)u2[0])*j)>>16) + (int)u2[0];
                            g1 = ((((int)u3[1]-(int)u2[1])*j)>>16) + (int)u2[1];
                            r1 = ((((int)u3[2]-(int)u2[2])*j)>>16) + (int)u2[2]; j = (iw[1]&65535);
                            b0 += (((b1-b0)*j)>>16);
                            g0 += (((g1-g0)*j)>>16);
                            r0 += (((r1-r0)*j)>>16);
                            j = (r0<<16)+(g0<<8)+b0;
                            _asm
                                    {
                                    punpcklbw mm0, j
                                    pmulhuw mm0, mm6
                                    psrlw mm0, 6
                                    packuswb mm0, mm0
                                    mov eax, lptr
                                    mov edx, x
                                    movd [eax+edx*4], mm0
                                    paddw mm6, mm7
                                    }
                            iw[0] += iwi[0]; iw[1] += iwi[1]; x++;
                        } while (x < xe3);
                    }
#else
                	// === SCANLINE RASTERIZATION LOOP ===
                	// High-performance texture mapping with lighting
                	// Uses fixed-point arithmetic for speed
                    if (renderinterp) // Nearest neighbor sampling
                    {
                        _asm
                                {
                                push esi
                                push edi

                                movq mm4, iw
                                mov edx, ttf

                                mov ecx, x
                                mov eax, xe3
                                mov edi, lptr
                                lea edi, [edi+eax*4]
                                sub ecx, eax
                                near_beg: pshufw mm0, mm4, 0xdd  ;mm0:[? ?    vi  ui]
                                pand mm0, qmask

                                pmaddwd mm0, qddmul    ;mm0:[? ? src32.p 4]
                                movd eax, mm0

                                ;pextrw eax, mm0, 1
                                ;shl eax, 8 ;byte ptr xshift FIXFIXFIX
                                ;pextrw esi, mm0, 0
                                ;lea eax, [eax+esi*4]

                                punpcklbw mm0, [eax+edx]
                                pmulhuw mm0, mm6
                                psrlw mm0, 6
                                packuswb mm0, mm0
                                movd [edi+ecx*4], mm0
                                paddd mm4, iwi
                                paddw mm6, mm7
                                add ecx, 1
                                jl short near_beg

                                movq iw, mm4
                                add ecx, xe3
                                mov x, ecx

                                pop edi
                                pop esi
                                }
                    }
                    else
                    {
                        _asm
                                {
                                push ebx
                                push esi
                                push edi

                                movq mm4, iw
                                mov edx, ttf
                                mov esi, ttp
                                add esi, edx

                                mov ecx, x
                                mov eax, xe3
                                mov edi, lptr
                                lea edi, [edi+eax*4]
                                sub ecx, eax
                                bilin_beg: pshufw mm0, mm4, 0xdd  ;mm0:[? ?    vi  ui]
                                pand mm0, qmask
                                pmaddwd mm0, qddmul    ;mm0:[? ? src32.p 4]
                                movd eax, mm0
                                movd mm0, [eax+edx]
                                movd mm2, [eax+esi]
                                lea ebx, [eax+4]       ;ui_temp = (ui+1)&(u_width-1)
                                and ebx, lmask0        ;
                                and eax, lmask1        ;
                                add eax, ebx           ;
                                movd mm1, [eax+edx]
                                movd mm3, [eax+esi]
                                pxor mm5, mm5
                                punpcklbw mm0, mm5
                                punpcklbw mm1, mm5
                                punpcklbw mm2, mm5
                                punpcklbw mm3, mm5

                                psubw mm1, mm0
                                psubw mm3, mm2
                                paddw mm1, mm1              ;
                                paddw mm3, mm3              ;

                                pshufw mm5, mm4, 0x00
                                psrlw mm5, 1                ;
                                pmulhw mm1, mm5
                                pmulhw mm3, mm5
                                paddw mm0, mm1
                                paddw mm2, mm3

                                pshufw mm5, mm4, 0xaa
                                psrlw mm5, 1                ;
                                psubw mm2, mm0
                                paddw mm2, mm2              ;
                                pmulhw mm2, mm5
                                paddw mm0, mm2

                                psllw mm0, 2                ;
                                pmulhuw mm0, mm6
                                packuswb mm0, mm0
                                movd [edi+ecx*4], mm0
                                paddd mm4, iwi
                                paddw mm6, mm7
                                add ecx, 1
                                jl short bilin_beg

                                movq iw, mm4
                                add ecx, xe3
                                mov x, ecx

                                pop edi
                                pop esi
                                pop ebx
                                }
                    }

#if 0
                    //fu = (iw[0]&65535);
                    //fv = (iw[1]&65535);
                    //b0 = ((u0[0]*(65535-fu) + u1[0]*fu)>>16);
                    //g0 = ((u0[1]*(65535-fu) + u1[1]*fu)>>16);
                    //r0 = ((u0[2]*(65535-fu) + u1[2]*fu)>>16);
                    //b1 = ((u2[0]*(65535-fu) + u3[0]*fu)>>16);
                    //g1 = ((u2[1]*(65535-fu) + u3[1]*fu)>>16);
                    //r1 = ((u2[2]*(65535-fu) + u3[2]*fu)>>16);
                    //r0 = ((   r0*(65535-fv) +    r1*fv)>>16);
                    //g0 = ((   g0*(65535-fv) +    g1*fv)>>16);
                    //b0 = ((   b0*(65535-fv) +    b1*fv)>>16);
                    __declspec(align(16)) const int sqmask0[4] = {-1,-1,-1,-1};
                    __declspec(align(16)) const int dqmask1[4] = {-1,-1, 0, 0};
                    ;xmm0:[ a3  r3  g3  b3  a1  r1  g1  b1]
                            ;xmm1:[ a2  r2  g2  b2  a0  r0  g0  b0]
                    movd xmm0, [e?]
                    movd xmm1, [e?]
                    movd xmm2, [e?]
                    movd xmm3, [e?]
                    punpckldq xmm1, xmm3 || unpcklps xmm1, xmm3
                    punpckldq xmm0, xmm2 || unpcklps xmm0, xmm2
                    punpcklbw xmm1, dqzero
                    punpcklbw xmm0, dqzero
                    pshuflw xmm2, xmm?, 0x??
                    movlhlps xmm2, xmm2      ;xmm2:[ fu  fu  fu  fu  fu  fu  fu  fu]
                    pmulhuw xmm0, xmm2
                    pxor xmm2, dqmask0       ;xmm2:[~fu ~fu ~fu ~fu ~fu ~fu ~fu ~fu]
                    pmulhuw xmm1, xmm2
                    paddw xmm0, xmm1         ;xmm0:[ a1  r1  g1  b1  a0  r0  g0  b0]
                    pshuflw xmm2, xmm?, 0x??
                    movlhps xmm2, xmm2       ;xmm1:[ fv  fv  fv  fv ~fv ~fv ~fv ~fv]
                    pxor xmm2, dqmask1
                    pmulhuw xmm0, xmm2
                    movhlps xmm1, xmm0
                    paddw xmm0, xmm1         ;xmm0:[  ?   ?   ?   ?  a   r   g   b ]

#endif

#endif
                    _asm
                            {
                            movq qs, mm6
                            emms
                            }
                } while (x < xe2);
#ifdef STANDALONE
                if (keystatus[0x2a]) lptr[xe2-1] = 0xffffff;
#endif
            }
        }

        //Inc x-steps
        for(i=pn3-1;i>=0;i--) rast[i].pos += rast[i].inc;
        for(l=plnumi-1;l>=0;l--)
            for(i=lpn3[l]-1;i>=0;i--)
                prast[l][i].pos += prast[l][i].inc;
    }
}
#if 0
!endif
#endif
