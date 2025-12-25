#include "monoclip.h"
#include "shadowtest2.h"

#include <stdbool.h>

#include "scenerender.h"

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>

#include "buildmath.h"
#include "monodebug.h"
#define PI 3.14159265358979323
#pragma warning(disable:4731)
#define EXLOGS 0
#define USESSE2 0
#define USENEWLIGHT 1 //FIXFIXFIX
#define USEGAMMAHACK 1 //FIXFIXFIX
int renderinterp = 1;
int compact2d = 0;
/*
Monopoly uses xy as screen coords and z as depth
Engine uses z as right, y as forward, z as down
Portals: we move camera to supposed place instead of transforming entire world.
for all porals same clipping plane is used, so mono state is shared, and should not reset
word has coordinates right down forward, which correspond to x z y
z grows down. and down vector is xyz=0,0,1;
mono plane has xy as screen coords and z as some sort of depth, but better not use it at all.
eyepols are generated from mono space AND plane equation stored in gouvmat.
*/
//------ UTILS -------
#define DP_AND_SUB 3
#define DP_AND_SUBREV 7
#define DP_NO_SCANSECT 8
#define DP_NO_PROJECT 16

#define MAX_PORTAL_DEPTH 2



void bdrawctx_clear(bdrawctx *b) {
	if (!b) return;

	// Clear bunch context allocations
	if (b->bunch) {
		free(b->bunch);
		b->bunch = NULL;
	}

	if (b->bunchgot) {
		free(b->bunchgot);
		b->bunchgot = NULL;
	}

	if (b->bunchgrid) {
		free(b->bunchgrid);
		b->bunchgrid = NULL;
	}

	// Clear sector allocations
	if (b->sectgot) {
		free(b->sectgot);
		b->sectgot = NULL;
	}

	if (b->sectgotmal) free((void *)b->sectgotmal);

	// Reset all counters and flags
	b->bunchn = 0;
	b->bunchmal = 0;
	b->bfintn = 0;
	b->sectgotn = 0;
	b->entrysec = 0;
	b->has_mono_out = false;
	b->has_portal_clip = false;
	b->recursion_depth = 0;
	b->testignorewall = 0;
	b->ignorekind = 0;
	b->testignoresec = 0;
	b->planecuts = 0;
	b->currenthalfplane = 0;
	b->gligsect = 0;
	b->gligwall = 0;
	b->gligslab = 0;
	b->gflags = 0;
	b->gnewtag = 0;
	b->gdoscansector = 0;
	b->gnewtagsect = 0;
	b->chead[0] = 0;
	b->chead[1] = 0;

	// Zero out lookup tables and matrices
	memset(b->bfintlut, 0, sizeof(b->bfintlut));
	memset(b->xformmat, 0, sizeof(b->xformmat));
	memset(b->oxformmat, 0, sizeof(b->oxformmat));
	memset(b->gouvmat, 0, sizeof(b->gouvmat));

	b->xformmatc = 0.0;
	b->xformmats = 0.0;
	b->oxformmatc = 0.0;
	b->oxformmats = 0.0;
}

static inline dpoint3d portal_xform_world_fullr(double x, double y, double z, bdrawctx *b) {
	dpoint3d p;
	p.x = x;
	p.y = y;
	p.z = z;
	wccw_transform(&p, &b->cam, &b->orcam);
//loops[loopnum] = p;
//loopuse[loopnum] = true;
//loopnum++;
	return p;
}
static inline void portal_xform_world_full(double *x, double *y, double *z, bdrawctx *b) {
	dpoint3d p;
	p.x = *x;
	p.y = *y;
	p.z = *z;
	wccw_transform(&p, &b->cam, &b->orcam);
	*x = p.x;
	*y = p.y;
	*z = p.z;
}

static inline void portal_xform_world_fullp(dpoint3d *inp, bdrawctx *b) {
	wccw_transform(inp, &b->cam, &b->orcam);
//loops[loopnum] = *inp;
//loopuse[loopnum] = true;
//loopnum++;

}

int intersect_traps_mono3d(double x0, double y0, double x1, double y1, double z0, double z4, double z5, double z1,
                         double z2, double z6, double z7, double z3, int *rh0, int *rh1, bdrawctx* b) {
    double fx, fy, fz;
    int i, j, h0, h1;
#define PXF(aa,bb,cc) portal_xform_world_fullr(aa,bb,cc,b)
    //0123,0213,0231,2013,2031,2301
	//LOOPEND
    if (z0 < z2) {
        if (z1 < z2) i = 0;
        else i = (z1 >= z3) + 1;
    } else {
        if (z3 < z0) i = 5;
        else i = (z3 < z1) + 3;
    }
    if (z4 < z6) {
        if (z5 < z6) j = 0;
        else j = (z5 >= z7) + 1;
    } else {
        if (z7 < z4) j = 5;
        else j = (z7 < z5) + 3;
    }

    h0 = -1;
    h1 = -1;
    if ((i == 0) || (i == 5)) {
        if (i != j) {
            if (i == 5) mono_intersamexy(x0, y0, x1, y1, z0, z4, z3, z7, &fx, &fy, &fz);
            else mono_intersamexy(x0, y0, x1, y1, z1, z5, z2, z6, &fx, &fy, &fz);

        	h0 = mono_insp(h0, PXF(fx, fy, fz));
            h1 = mono_insp(h1, PXF(fx, fy, fz));
        }
    } else {
        if (i > 2) h0 = mono_insp(h0, PXF(x0, y0, z0));
        else h0 = mono_insp(h0, PXF(x0, y0, z2));
        if (i & 1) h1 = mono_insp(h1, PXF(x0, y0, z1));
        else h1 = mono_insp(h1, PXF(x0, y0, z3));
    }
    if (i != j) {
        if ((i < 3) != (j < 3)) {
            mono_intersamexy(x0, y0, x1, y1, z0, z4, z2, z6, &fx, &fy, &fz);
            h0 = mono_insp(h0, PXF(fx, fy, fz));
        }
        if (((i ^ 1) < 3) != ((j ^ 1) < 3)) {
            mono_intersamexy(x0, y0, x1, y1, z1, z5, z3, z7, &fx, &fy, &fz);
            h1 = mono_insp(h1, PXF(fx, fy, fz));
        }
    }
    if ((j == 0) || (j == 5)) {
        if (i != j) {
            if (j == 5) mono_intersamexy(x0, y0, x1, y1, z0, z4, z3, z7, &fx, &fy, &fz);
            else mono_intersamexy(x0, y0, x1, y1, z1, z5, z2, z6, &fx, &fy, &fz);
            h0 = mono_insp(h0, PXF(fx, fy, fz));
            h1 = mono_insp(h1, PXF(fx, fy, fz));
        }
    } else {
        if (j > 2) h0 = mono_insp(h0, PXF(x1, y1, z4));
        else h0 = mono_insp(h0, PXF(x1, y1, z6));
        if (j & 1) h1 = mono_insp(h1, PXF(x1, y1, z5));
        else h1 = mono_insp(h1, PXF(x1, y1, z7));
    }

    if ((h0 | h1) < 0) return (0);
    (*rh0) = mp[h0].n;
    (*rh1) = mp[h1].n;
    return (1);
}
int intersect_traps_mono_points3d(dpoint3d p0, dpoint3d p1, dpoint3d trap1[4], dpoint3d trap2[4], int *rh0, int *rh1, bdrawctx* b) {
	return intersect_traps_mono3d(
		p0.x, p0.y, p1.x, p1.y,
		trap1[0].z, trap1[1].z, trap1[2].z, trap1[3].z,
		trap2[0].z, trap2[1].z, trap2[2].z, trap2[3].z,
		rh0, rh1,b
	);
}
int shadowtest2_backface_cull = 0;  // Toggle backface culling
int shadowtest2_distance_cull = 0;  // Toggle distance-based culling
int shadowtest2_debug_walls = 1;    // Verbose wall logging
int shadowtest2_debug_block_selfportals = 1;    // Verbose wall logging

//--------------------------------------------------------------------------------------------------
static tiletype gdd;
int shadowtest2_rendmode = 1;
extern int drawpoly_numcpu;
int shadowtest2_updatelighting = 1;

	//Sorting

unsigned int *shadowtest2_sectgot = 0; //WARNING:code uses x86-32 bit shift trick!
static unsigned int *shadowtest2_sectgotmal = 0;
static int shadowtest2_sectgotn = 0;

//Translation & rotation
static mapstate_t *curMap;
static player_transform *gps;
//static point3d b->gnadd;
//static double b->xformmat[9], b->xformmatc, b->xformmats;

	//Texture mapping parameters
static tile_t *gtpic;

static int gcurcol, gtilenum;
static int taginc= 30000;
#define LIGHTMAX 256 //FIX:make dynamic!
lightpos_t shadowtest2_light[LIGHTMAX];
static lightpos_t *glp;
int shadowtest2_numlights = 0, shadowtest2_useshadows = 1, shadowtest2_numcpu = 0;
float shadowtest2_ambrgb[3] = {32.0,32.0,32.0};
__declspec(align(16)) static float g_qamb[4]; //holder for SSE to avoid degenerates
static point3d slightpos[LIGHTMAX], slightdir[LIGHTMAX];

static float spotwid[LIGHTMAX];


eyepol_t *eyepol = 0; // 4096 eyepol_t's = 192KB
vert3d_t *eyepolv = 0; //16384 point2d's  = 128KB
int eyepoln = 0, glignum = 0;
int eyepolmal = 0, eyepolvn = 0, eyepolvmal = 0;
#define LIGHASHSIZ 1024
static int ligpolmaxvert = 0;
#define lighash(sect,wall,slab) ((((slab)<<6)+((sect)<<4)+(wall))&(LIGHASHSIZ-1))
//--------------------------------------------------------------------------------------------------
extern void htrun (void (*dacallfunc)(int), int v0, int v1, int danumcpu);
extern double distpoint2line2 (double x, double y, double x0, double y0, double x1, double y1);

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

static int prepbunch (int id, bunchverts_t *twal, bdrawctx *b)
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
static int bunchfront (int b0, int b1, int fixsplitnow, bdrawctx *b)
{
	cam_t gcam = b->cam;
	bunchverts_t *twal[2];
	wall_t *wal;
	double d, a[2], x0, y0, x1, y1, x2, y2, x3, y3, t0, t1, t2, t3;
	double x, y, ix, iy, tix, tiy, x10, y10, x23, y23, x20, y20, otx0, oty0, otx1, oty1, tx0, ty0, tx1, ty1, u, t, d0, d1;
	int i, j, twaln[2], oj, ind[2], sid, cnt, gotsid, startsid, obfintn;

	if (b0 == b1) return(0);

	twal[0] = (bunchverts_t *)_alloca((curMap->sect[b->bunch[b0].sec].n+curMap->sect[b->bunch[b1].sec].n+2)*sizeof(bunchverts_t));
	twaln[0] = prepbunch(b0,twal[0],b); twal[1] = &twal[0][twaln[0]+1];
	twaln[1] = prepbunch(b1,twal[1],b);

		//Offset vertices (BUNCHNEAR of scansector() already puts them safely in front)
	for (j = 2 - 1; j >= 0; j--) for (i = twaln[j]; i >= 0; i--) {
		twal[j][i].x -= gcam.p.x;
		twal[j][i].y -= gcam.p.y;
	}

	if (twal[0][0].y*twal[1][twaln[1]].x >= twal[1][twaln[1]].y*twal[0][0].x) return(0); //no overlap (whole bunch)
	if (twal[1][0].y*twal[0][twaln[0]].x >= twal[0][twaln[0]].y*twal[1][0].x) return(0); //no overlap (whole bunch)

	a[0] = 0; a[1] = 0;
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
}
// problem is before mono insertion (see drawpol,.)
static void scansector (int sectnum, bdrawctx* b)
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

		double zzz = getwallz(sec, 1, i);
		dpoint3d wp = {wal[i].x, wal[i].y, zzz};

		dx0 = wal[i].x - gcam.p.x;
		dy0 = wal[i].y - gcam.p.y;
		dx1 = wal[j].x - gcam.p.x;
		dy1 = wal[j].y - gcam.p.y;
		if (dy1 * dx0 <= dx1 * dy0) goto docont; //Back-face cull

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

static void xformprep (double hang, bdrawctx *b)
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

static void xformbac (double rx, double ry, double rz, dpoint3d *o, bdrawctx *b)
{
	float mul = b->ismirrored ? -1 : 1; // for flipped world
	o->x = rx*b->xformmat[0] + ry*b->xformmat[3] + rz*b->xformmat[6];
	o->y = rx*b->xformmat[1] + ry*b->xformmat[4] + rz*b->xformmat[7];
	o->z = rx*b->xformmat[2] + ry*b->xformmat[5] + rz*b->xformmat[8];
}
// Helper function to check turn direction
float cross_product (int a, int b, int c) {
	point3d pa = eyepolv[a].wpos;
	point3d pb = eyepolv[b].wpos;
	point3d pc = eyepolv[c].wpos;
	return (pb.x - pa.x) * (pc.y - pa.y) - (pb.y - pa.y) * (pc.x - pa.x);
};

static void drawtagfunc_ws(int rethead0, int rethead1, bdrawctx *b)
{

// ouput is x-monotone, left to right.
	float f,fx,fy, g, *fptr;
	int i, j, k, h, rethead[2];
	cam_t cam = b->cam;
	double* xform = b->xformmat;
	point3d add = b->gnadd;
	if ((rethead0|rethead1) < 0) { mono_deloop(rethead1); mono_deloop(rethead0); return; }
	rethead[0] = rethead0; rethead[1] = rethead1;
	int *indices = NULL;
	int index_count = 0;
	int index_capacity = 0;
	int chain_starts[2];
	int chain_lengths[2] = {0, 0};

	for(h=0;h<2;h++) // h is head
	{

		i = rethead[h];
		chain_starts[h] = eyepolvn;
		do
		{
			if (eyepolvn >= eyepolvmal)
			{
				eyepolvmal = max(eyepolvmal<<1,16384);
				eyepolv = (vert3d_t *)realloc(eyepolv,eyepolvmal*sizeof(vert3d_t));
			}
			eyepolv[eyepolvn].wpos = (point3d){mp[i].pos.x,mp[i].pos.y,mp[i].pos.z};
			eyepolvn++;
			chain_lengths[h]++;
			i = mp[i].n;
		} while (i != rethead[h]);
		mono_deloop(rethead[h]);

	}
	// TRIANGULATION
	// Allocate indices array
// Stack for monotone triangulation


int total_vertices = chain_lengths[0] + chain_lengths[1];
	int* stack = (int*)malloc(total_vertices * sizeof(int));
	int stack_top = -1;
int triangle_count = total_vertices - 2;
index_capacity = triangle_count * 3;
indices = (int*)malloc(index_capacity * sizeof(int));
index_count = 0;

if (total_vertices <= 3) {
    free(stack);
    return;
}

// Merge chains into sorted order by x-coordinate
int* sorted_vertices = (int*)malloc(total_vertices * sizeof(int));
int* chain_id = (int*)malloc(total_vertices * sizeof(int)); // 0 for left chain, 1 for right chain

int left_idx = 0, right_idx = 0, merge_idx = 0;

// Merge the two chains
while (left_idx < chain_lengths[0] && right_idx < chain_lengths[1]) {
    float left_x = eyepolv[chain_starts[0] + left_idx].x;
    float right_x = eyepolv[chain_starts[1] + right_idx].x;

    if (left_x <= right_x) {
        sorted_vertices[merge_idx] = chain_starts[0] + left_idx;
        chain_id[merge_idx] = 0;
        left_idx++;
    } else {
        sorted_vertices[merge_idx] = chain_starts[1] + right_idx;
        chain_id[merge_idx] = 1;
        right_idx++;
    }
    merge_idx++;
}

// Add remaining vertices
while (left_idx < chain_lengths[0]) {
    sorted_vertices[merge_idx] = chain_starts[0] + left_idx;
    chain_id[merge_idx] = 0;
    left_idx++;
    merge_idx++;
}
while (right_idx < chain_lengths[1]) {
    sorted_vertices[merge_idx] = chain_starts[1] + right_idx;
    chain_id[merge_idx] = 1;
    right_idx++;
    merge_idx++;
}

// Cross product for turn test


// Initialize stack with first two vertices
stack[++stack_top] = sorted_vertices[0];
stack[++stack_top] = sorted_vertices[1];

// Process remaining vertices
for (int i = 2; i < total_vertices; i++) {
    int current_vertex = sorted_vertices[i];
    int current_chain = chain_id[i];
    int top_chain = chain_id[stack_top];

    if (current_chain != top_chain) {
        // Different chains - triangulate with all stack vertices except the last
        while (stack_top > 0) {
            int v1 = stack[stack_top - 1];
            int v2 = stack[stack_top];

            // Check orientation for proper winding
            float cross = cross_product(v1, v2, current_vertex);
            if (cross > 0) { // CCW orientation
                indices[index_count++] = v1;
                indices[index_count++] = v2;
                indices[index_count++] = current_vertex;
            } else {
                indices[index_count++] = v1;
                indices[index_count++] = current_vertex;
                indices[index_count++] = v2;
            }
            stack_top--;
        }

        // Keep last vertex and add current
        stack[++stack_top] = current_vertex;
    } else {
        // Same chain - check for valid triangles
        while (stack_top > 0) {
            int v1 = stack[stack_top - 1];
            int v2 = stack[stack_top];

            // Check if we can form a valid triangle
            float cross = cross_product(v1, v2, current_vertex);

            // For monotone polygon, we need to check if the triangle is inside
            bool valid_triangle = false;
            if (current_chain == 0) { // Left chain
                valid_triangle = (cross < 0); // CW turn for left chain
            } else { // Right chain
                valid_triangle = (cross > 0); // CCW turn for right chain
            }

            if (valid_triangle) {
                // Add triangle with proper winding
                if (cross > 0) {
                    indices[index_count++] = v1;
                    indices[index_count++] = v2;
                    indices[index_count++] = current_vertex;
                } else {
                    indices[index_count++] = v1;
                    indices[index_count++] = current_vertex;
                    indices[index_count++] = v2;
                }
                stack_top--; // Remove the middle vertex
            } else {
                break; // No more valid triangles
            }
        }

        // Add current vertex to stack
        stack[++stack_top] = current_vertex;
    }
}

free(stack);
free(sorted_vertices);
free(chain_id);


	// Store triangulation in eyepol structure
	if (eyepoln+1 >= eyepolmal)
	{
		eyepolmal = max(eyepolmal<<1, 4096);
		eyepol = (eyepol_t *)realloc(eyepol, eyepolmal*sizeof(eyepol_t));
		eyepol[0].vert0 = 0;
	}
	eyepol[eyepoln].tilnum = gtilenum;
	// setup uvs
	if (b->gisflor < 2) {
		eyepol[eyepoln].worlduvs = curMap->sect[b->gligsect].surf[b->gisflor].uvcoords;
		eyepol[eyepoln].uvform = curMap->sect[b->gligsect].surf[b->gisflor].uvform;
	//	eyepol[eyepoln].tilnum = curMap->sect[b->gligsect].surf[b->gisflor].tilnum;
	} else { // walls
		eyepol[eyepoln].worlduvs = curMap->sect[b->gligsect].wall[b->gligwall].xsurf[b->gligslab].uvcoords;
		eyepol[eyepoln].uvform = curMap->sect[b->gligsect].wall[b->gligwall].xsurf[b->gligslab].uvform;
		eyepol[eyepoln].tilnum = curMap->sect[b->gligsect].wall[b->gligwall].xsurf[b->gligslab].tilnum;
		//xsurf[b->gligslab % 3].uvcoords;
	}

	eyepol[eyepoln].slabid = b->gligslab; // 0 -top, 1 - mid, 2-bot.

	// transform verts to WS
	for (int pn= chain_starts[0]; pn<eyepolvn;pn++) {

		f = cam.h.z/(eyepolv[pn].x*xform[6] + eyepolv[pn].y*xform[7] + add.z);
		fx        =  (eyepolv[pn].x*xform[0] + eyepolv[pn].y*xform[1] + add.x)*f + cam.h.x;
		fy        =  (eyepolv[pn].x*xform[3] + eyepolv[pn].y*xform[4] +add.y)*f + cam.h.y;

		f = 1.0/((b->gouvmat[0]*fx + b->gouvmat[3]*fy + b->gouvmat[6])*cam.h.z);

		float retx = ((fx-cam.h.x)*cam.r.x + (fy-cam.h.y)*cam.d.x + (cam.h.z)*cam.f.x)*f + cam.p.x;
		float rety = ((fx-cam.h.x)*cam.r.y + (fy-cam.h.y)*cam.d.y + (cam.h.z)*cam.f.y)*f + cam.p.y;
		float retz = ((fx-cam.h.x)*cam.r.z + (fy-cam.h.y)*cam.d.z + (cam.h.z)*cam.f.z)*f + cam.p.z;
		dpoint3d ret = {retx,rety,retz};
		eyepolv[pn].uvpos = ret;
		// get it in space of really moved cam, and return back to original space.
		// vector transforms are working vell outside of mono plane.
		wccw_transform(&ret, &b->movedcam, &b->orcam);
		eyepolv[pn].wpos = (point3d){ret.x,ret.y,ret.z};
	}
	eyepol[eyepoln].c1 = chain_starts[0];
	eyepol[eyepoln].c2 = chain_starts[1];
	eyepol[eyepoln].l1 = chain_lengths[0];
	eyepol[eyepoln].l2 = chain_lengths[1];


	eyepol[eyepoln].vert0 = chain_starts[0];
	eyepol[eyepoln].indices = indices;
	eyepol[eyepoln].nid = index_count;
	memcpy((void *)eyepol[eyepoln].ouvmat, (void *)b->gouvmat, sizeof(b->gouvmat[0])*9);
	eyepol[eyepoln].tpic = gtpic;
	eyepol[eyepoln].curcol = gcurcol;
	eyepol[eyepoln].flags = (b->gflags != 0);
	eyepol[eyepoln].b2sect = b->gligsect;
	eyepol[eyepoln].b2wall = b->gligwall;
	eyepol[eyepoln].b2slab = b->gligslab;
	memcpy((void *)&eyepol[eyepoln].norm, (void *)&b->gnorm, sizeof(b->gnorm));
	eyepol[eyepoln].rdepth = b->recursion_depth;
	eyepoln++;
	eyepol[eyepoln].vert0 = eyepolvn;

    logstep("produce eyepol, depth:%d",b->recursion_depth);
}



static void skytagfunc (int rethead0, int rethead1, bdrawctx* b){}

/*
	Purpose: Generates shadow polygon lists for lighting
	Converts screen-space polygons back to 3D world coordinates
	Stores shadow-casting geometry in ligpol[] arrays per light source
	Used during shadow map generation phase (mode 4)
	Creates hash table for fast polygon lookup by sector/wall/slab
 */
static void ligpoltagfunc (int rethead0, int rethead1, bdrawctx *b)
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
static void drawtag_debug(int rethead0, int rethead1, bdrawctx *b)
{

	float f,fx,fy, g, *fptr;
	int i, j, k, h, rethead[2];
	cam_t cam = b->cam;
	double* xform = b->xformmat;
	point3d add = b->gnadd;
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
				eyepolv = (vert3d_t *)realloc(eyepolv,eyepolvmal*sizeof(vert3d_t));
			}
			f = cam.h.z/(mp[i].x*xform[6] + mp[i].y*xform[7] + add.z);
			fx        =  (mp[i].x*xform[0] + mp[i].y*xform[1] + add.x)*f + cam.h.x;
			fy        =  (mp[i].x*xform[3] + mp[i].y*xform[4] +add.y)*f + cam.h.y;

			f = 1.0/((b->gouvmat[0]*fx + b->gouvmat[3]*fy + b->gouvmat[6])*cam.h.z);

			float retx = ((fx-cam.h.x)*cam.r.x + (fy-cam.h.y)*cam.d.x + (cam.h.z)*cam.f.x)*f + cam.p.x;
			float rety = ((fx-cam.h.x)*cam.r.y + (fy-cam.h.y)*cam.d.y + (cam.h.z)*cam.f.y)*f + cam.p.y;
			float retz = ((fx-cam.h.x)*cam.r.z + (fy-cam.h.y)*cam.d.z + (cam.h.z)*cam.f.z)*f + cam.p.z;
			dpoint3d ret = {retx,rety,retz};
			if (b->recursion_depth>1) {
			//	LOOPADD(ret)
			}
			wccw_transform(&ret, &b->cam, &b->orcam);
			eyepolv[eyepolvn].wpos = (point3d){ret.x,ret.y,ret.z};

			eyepolvn++;

			if (!h) i = mp[i].n;
		} while (i != rethead[h]);
	}

	if (eyepoln+1 >= eyepolmal)
	{
		eyepolmal = max(eyepolmal<<1, 4096);
		eyepol = (eyepol_t *)realloc(eyepol, eyepolmal*sizeof(eyepol_t));
		eyepol[0].vert0 = 0;
	}

	memcpy((void *)eyepol[eyepoln].ouvmat, (void *)b->gouvmat, sizeof(b->gouvmat[0])*9);

	eyepol[eyepoln].tpic = gtpic;
	eyepol[eyepoln].curcol = gcurcol;
	eyepol[eyepoln].flags = (b->gflags != 0);
	eyepol[eyepoln].b2sect = b->gnewtag;
	eyepol[eyepoln].b2wall = b->gligwall;
	eyepol[eyepoln].b2slab = b->gligslab;
	memcpy((void *)&eyepol[eyepoln].norm, (void *)&b->gnorm, sizeof(b->gnorm));
	eyepoln++;
	eyepol[eyepoln].vert0 = eyepolvn;
	eyepol[eyepoln].rdepth = b->recursion_depth;
	logstep("produce eyepol, depth:%d",b->recursion_depth);
}

static void changetagfunc (int rethead0, int rethead1, bdrawctx *b)
{
	if ((rethead0|rethead1) < 0) return;
	int mapsect = b->gnewtagsect;
	if ((b->gdoscansector)
		&& (!(b->sectgot[mapsect>>5]&(1<<mapsect))))
		scansector(mapsect,b);

	mono_mph_check(mphnum);
	mph[mphnum].head[0] = rethead0;
	mph[mphnum].head[1] = rethead1;
	mph[mphnum].tag = b->gnewtag;
	if (b->has_portal_clip)
		mono_dbg_capture_mph(mphnum, "clip in potal");
	mphnum++;
	//if (b->recursion_depth >=2)
	//	drawtag_debug(rethead0,rethead0,b);
	logstep("changetag: newMtag:%d, new mphnum:%d",b->gnewtag,mphnum);
}
	//flags&1: do and
	//flags&2: do sub
	//flags&4: reverse cut for sub
// this takes pair and projects it onto screen plane with a cam.
// returns new heads.
static int projectonmono (int *plothead0, int *plothead1,  bdrawctx* b) {
	if (!mpcheck(*plothead0,*plothead1))
		return 0;
	cam_t gcam = b->cam;
	double* xform = b->xformmat;
	double xformc = b->xformmatc;
	double xforms = b->xformmats;
#define BSCISDIST 0.000001 //Reduces probability of glitch further
	//#define BSCISDIST 0.0001 //Gaps undetectable
	//#define BSCISDIST 0.1 //Huge gaps

	dpoint3d *otp, *tp;
	double f, ox, oy, oz;
	int i, j, k, l, h, on, n, plothead[2], imin, imax, i0, i1, omph0, omph1;

	plothead[0] = *plothead0;
	plothead[1] = *plothead1;

	n = 2;
	for (h = 0; h < 2; h++)
		for (i = mp[plothead[h]].n; i != plothead[h]; i = mp[i].n) {
		//	printf("%d, ",n);
			n++;
			if (n > 20) {
				printf ("fucked up mono");
				//mono_deloop(*plothead0);
				//mono_deloop(*plothead1);
				//return 0;
			}
		}
	otp = (dpoint3d *) _alloca(n * sizeof(dpoint3d));
	tp = (dpoint3d *) _alloca(n * sizeof(dpoint3d) * 2);

	//rotate, converting vmono to simple point3d loop
	on = 0;
	for(h=0;h<2;h++)
	{
		i = plothead[h];
		do
		{
			if (h) i = mp[i].p;
			if (b->recursion_depth==2) {
			//	LOOPADDP(mp[i])
			}
			ox = mp[i].x-gcam.p.x; oy = mp[i].y-gcam.p.y;
			//if (b->has_portal_clip)
			//	LOOPADD(mp[i].pos)
			otp[on].x = oy*xformc - ox*xforms;
			otp[on].y = mp[i].z-gcam.p.z;
			otp[on].z = ox*xformc + oy*xforms; on++;

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
	if (n < 3) {
		return 0;
	}

	//project & find x extents
	for(i=0;i<n;i++)
	{
		f = gcam.h.z/tp[i].z;
		tp[i].x = tp[i].x*f + gcam.h.x;
		tp[i].y = tp[i].y*f + gcam.h.y;

	}
//LOOPEND
	//generate vmon
		mono_genfromloop(&plothead[0], &plothead[1], tp, n);
	if ((plothead[0] | plothead[1]) < 0) {
		mono_deloop(plothead[0]);
		mono_deloop(plothead[1]);
		return 0;
	}
	*plothead0 = plothead[0];
	*plothead1 = plothead[1];
	return 1;
}
static int cliptonewregion(int fromtag, int newtag, int newsect, int h1,int h2, bool doscan, bdrawctx *b) {

	b->gdoscansector =  doscan;
	// intersect with same monos, and change tag for resulting pieces, creating new clip group
	logstep("bool AND, keep all, changetag, on tag %d -> %d", fromtag,newtag);
	for (int i = mphnum - 1; i >= 0; i--)
		if (mph[i].tag == fromtag) {
			mono_bool(
			   mph[i].head[0],
			   mph[i].head[1],
			   h1,
			   h2,
			   MONO_BOOL_AND,
			   b,
			   changetagfunc);
		}
}
static int drawpol_nosect(int overlaptag, int newtag, int *heads, int flags,bdrawctx* b) {
	flags = flags | DP_NO_SCANSECT;
	if (newtag < 0)
		return 0;
	return drawpol_befclip(overlaptag,newtag,-1,-1,heads[0],heads[1],flags,b);
}
static int drawpol_befclip (int fromtag, int newtag1, int fromsect, int newsect, int plothead0, int plothead1, int flags, bdrawctx* b)
{
#if EXLOGS
	printf("drawpol from:%d, to:%d, h1:%d, h2:%d, depth:%d \n",fromtag,newtag1,plothead0,plothead1,b->recursion_depth);
#endif
	if ((plothead0|plothead1) < 0) {
		mono_deloop(plothead0);
		mono_deloop(plothead1);
		return 0;
	}

	//LOOPEND
	int curtag = fromtag;
	int cursec = fromsect;
	int newtag = newtag1;
	logstep("drawpol tag:%d nwtag:%d\n",curtag , newtag);
	b->gnewtagsect = newsect;
	dpoint3d *otp, *tp;
	double f, ox, oy, oz;
	int i, j, k, l, h, on, n, plothead[2], imin, imax, i0, i1, omph0, omph1;

	void (*mono_output)(int h0, int h1, bdrawctx *b);
	plothead[0] = plothead0;
	plothead[1] = plothead1;
	int projok = 1;
	if (!(flags & DP_NO_PROJECT))
		projok = projectonmono(&plothead[0],&plothead[1],b);

	if (!projok)
		return 0;

	// -- plothead points to polygon clipped with camera plane.
	if (flags&1 || flags&8)
	{
		if (newtag >= 0) // produces new clipping group
		{
			b->gnewtagsect = newsect;
			b->gnewtag = newtag;
			omph0 = mphnum;
			b->gdoscansector = !(flags & DP_NO_SCANSECT);
			// intersect with same monos, and change tag for resulting piece?
			// cliptonewregion
			logstep("bool AND, keep all, changetag, on tag %d to %d", curtag, b->gnewtag);
			for (i = mphnum - 1; i >= 0; i--)
				if (mph[i].tag == curtag) {
					mono_bool(
					   mph[i].head[0],
					   mph[i].head[1],
					   plothead[0],
					   plothead[1],
					   MONO_BOOL_AND,
					   b,
					   changetagfunc);
				}
			{
				logstep ("Join and remove bases for tags, on upper res,  mhp[j]== %d, heads: [%d..%d]", b->gnewtag, omph0, mphnum-1);
				for (l = omph0; l < mphnum; l++) {
					logstep("set %d to %d",omph0, l);
					mph[omph0] = mph[l];
					k = omph0;
					omph0++;

					for (j = omph0 - 1; j >= 0; j--) //Join monos
					{
						if (mph[j].tag != b->gnewtag) continue;
						if (!mono_join(mph[j].head[0], mph[j].head[1], mph[k].head[0], mph[k].head[1], &i0,
						               &i1)) continue;
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
		}
		else { // do AND with current mono, draw result, and discard it in drawtag.
			if (shadowtest2_rendmode == 4)
				mono_output = ligpoltagfunc;
				//add to light list // this will process point lights. otherwize will only use plr light.
			else if (b->gflags < 2) mono_output = drawtagfunc_ws;
			else mono_output = drawtagfunc_ws; //calls drawtagfunc inside
			logstep ("Bool-AND for solids drawtag, againsst all heads, keep all, with mono N=%d, when tag==%d", mphnum-1,curtag);
			for (i = mphnum - 1; i >= 0; i--)
				if (mph[i].tag == curtag)
					mono_bool(mph[i].head[0], mph[i].head[1], plothead[0], plothead[1],MONO_BOOL_AND, b, mono_output);
		}
	}
	if (flags&2)  // this entire section will chip current off of others with same tag, detalizing clip group.
	{
		if (!(flags&4)) j = MONO_BOOL_SUB;
					  else j = MONO_BOOL_SUB_REV; // when floor.

		b->gnewtag = curtag;
		b->gnewtagsect = cursec;
		b->gdoscansector = 0; omph0 = mphnum; omph1 = mphnum;
		// cut this off result from initial areas
		//logstep("stored head o0 o1 before op %d", omph1);
		logstep("Bool cutting, changetag all heads N=%d, against mono, remove cutted bases, on tag == %d to %d", mphnum-1, curtag, b->gnewtag);
		for(i=mphnum-1;i>=0;i--)
		{
			if (mph[i].tag != curtag) continue;
			mono_bool(mph[i].head[0],mph[i].head[1],plothead[0],plothead[1],j,b,changetagfunc);
			mono_deloop(mph[i].head[1]);
			mono_deloop(mph[i].head[0]);

			omph0--; mph[i] = mph[omph0];
		}

			//valid mph's stored in 2 blocks: (0<=?<omph0), (omph1<=?<mphnum)
		// join leftovers of the original tag
			logstep("joining monos, on tag == %d", b->gnewtag);
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
logstep ("removing originally produced mono");
	mono_deloop(plothead[1]);
	mono_deloop(plothead[0]);
return 1;
}

static void gentransform_ceilflor(sect_t *sec, wall_t *wal, int isflor, bdrawctx *b)
{
	cam_t *cam = &b->cam;
	float gx = sec->grad[isflor].x;
	float gy = sec->grad[isflor].y;

	// Transform plane normal (gx, gy, 1) to camera space
	float nx = cam->r.x * gx + cam->r.y * gy + cam->r.z;
	float ny = cam->d.x * gx + cam->d.y * gy + cam->d.z;
	float nz = cam->f.x * gx + cam->f.y * gy + cam->f.z;

	// Camera-space plane constant
	float D_c = gx * (wal[0].x - cam->p.x)
			  + gy * (wal[0].y - cam->p.y)
			  + (sec->z[isflor] - cam->p.z);

	// Scale includes h.z for screen-space depth formula
	float scale = 1.0f / (D_c * cam->h.z);
	b->gouvmat[0] = nx * scale;
	b->gouvmat[3] = ny * scale;
	b->gouvmat[6] = nz / D_c - b->gouvmat[0] * cam->h.x - b->gouvmat[3] * cam->h.y;
}

// create plane EQ using GCAM
static void gentransform_wall (dpoint3d *npol2, surf_t *sur, bdrawctx *b) {
	cam_t usedcam = b->cam; // we can use camera hack to get plane equation in space of current cam, not necessart clipping cam.
	float f, g, ox, oy, oz, rdet, fk[24];
	int i;

	for(i=0;i<3;i++)
	{
		ox = npol2[i].x-usedcam.p.x; oy = npol2[i].y-usedcam.p.y; oz = npol2[i].z-usedcam.p.z;
		npol2[i].x = ox*usedcam.r.x + oy*usedcam.r.y + oz*usedcam.r.z;
		npol2[i].y = ox*usedcam.d.x + oy*usedcam.d.y + oz*usedcam.d.z;
		npol2[i].z = ox*usedcam.f.x + oy*usedcam.f.y + oz*usedcam.f.z;
	}

	fk[0] = npol2[0].z; fk[3] = npol2[0].x*usedcam.h.z + npol2[0].z*usedcam.h.x; fk[6] = npol2[0].y*usedcam.h.z + npol2[0].z*usedcam.h.y;
	fk[1] = npol2[1].z; fk[4] = npol2[1].x*usedcam.h.z + npol2[1].z*usedcam.h.x; fk[7] = npol2[1].y*usedcam.h.z + npol2[1].z*usedcam.h.y;
	fk[2] = npol2[2].z; fk[5] = npol2[2].x*usedcam.h.z + npol2[2].z*usedcam.h.x; fk[8] = npol2[2].y*usedcam.h.z + npol2[2].z*usedcam.h.y;
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

	rdet = 1.0/(fk[0]*fk[12] + fk[1]*fk[13] + fk[2]*fk[14]);

	g = rdet;

	b->gouvmat[0] *= g;
	b->gouvmat[3] *= g;
	b->gouvmat[6] *= g;

	if (renderinterp)
	{
		// idx 0 3 6 store plane eq?

	//	b->gouvmat[1] -= b->gouvmat[0]*32768.0; b->gouvmat[2] -= b->gouvmat[0]*32768.0;
	//	b->gouvmat[4] -= b->gouvmat[3]*32768.0; b->gouvmat[5] -= b->gouvmat[3]*32768.0;
	//	b->gouvmat[7] -= b->gouvmat[6]*32768.0; b->gouvmat[8] -= b->gouvmat[6]*32768.0;
	}
}

/*
the mono engine produces camera-space polygons that are clipped to not overlap.
The plothead[0] and plothead[1] contain monotone polygon pairs representing
the final visible geometry ready for 2D projection.
The b parameter is a bunch index - this function processes one "bunch" (visible sector group) at a time. The traversal logic is in the caller that:
*/

static void drawalls (int bid, mapstate_t* map, bdrawctx* b)
{
	gtilenum = 0;
	cam_t gcam = b->cam;
	// === VARIABLE DECLARATIONS ===
	//extern void loadpic (tile_t *);
	#define MAXVERTS 256 //FIX:timebomb: assumes there are never > 256 sectors connected at same vertex
	vertlist_t verts[MAXVERTS];
	bunchverts_t *twal;
	int twaln;
	dpoint3d pol[4], npol[6];
	dpoint3d npol2[3];
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

	twal = (bunchverts_t *)_alloca((curMap->sect[b->bunch[bid].sec].n+1)*sizeof(bunchverts_t));
	twaln = prepbunch(bid,twal,b);
	b->gligsect = s;
	b->gligslab = 0;

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
	bool noportals = b->recursion_depth >= MAX_PORTAL_DEPTH;
	for(isflor=0;isflor<2;isflor++) // floor ceil
	{
		b->gisflor = isflor;

			int myport = sec[s].tags[1]; // FLOOR PORTAL CHECK
			bool isportal = myport >= 0
							&& !noportals
			                && portals[myport].destpn >= 0
			                && portals[myport].surfid == isflor
			                && portals[myport].kind == isflor;
			bool skipport = shadowtest2_debug_block_selfportals
			                && b->has_portal_clip
			                && isportal
			                && s == b->testignoresec
			                && portals[myport].kind == b->ignorekind
			                && isflor == b->testignorewall;
			if (skipport)
				continue;
		gtilenum = sec[s].surf[isflor].tilnum;

		float surfpos = getslopez(&sec[s],isflor,b->cam.p.x,b->cam.p.y);
		if ((b->cam.p.z >= surfpos) == isflor) // ignore backfaces
				continue;
		fz = sec[s].z[isflor]; grad = &sec[s].grad[isflor];

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
		point3d locnorm = world_to_local_vec(b->gnorm, &b->cam.tr);
		for (ww = twaln; ww >= 0; ww -= twaln) plothead[isflor] = mono_ins(
			                                       plothead[isflor], twal[ww].x, twal[ww].y,
			                                       b->gnorm.z * -1e9);
		//do not replace w/single zenith point - ruins precision
		i = isflor ^ 1;
		for (ww = 0; ww <= twaln; ww++) {
			plothead[i] = mono_ins(plothead[i], twal[ww].x, twal[ww].y,
								  (wal[0].x - twal[ww].x) * grad->x + (
									  wal[0].y - twal[ww].y) * grad->y + fz);
		}

			plothead[i] = mp[plothead[i]].n;

		// Setup texture and rendering flags
		sur = &sec[s].surf[isflor];
		gtpic = &gtile[sur->tilnum];
		gtilenum = sur->tilnum;
		//if (!gtpic->tt.f) loadpic(gtpic);
		if (sec[s].surf[isflor].flags & (1 << 17)) { b->gflags = 2; } //skybox ceil/flor
		else if (sec[s].surf[isflor].flags & (1 << 16)) {  //parallaxing ceil/flor
			b->gflags = 1;
			//gentex_sky(sur, b);
		}
		else {
			b->gflags = 0;
		}
			gentransform_ceilflor(&sec[s], wal, isflor, b);

		b->gligwall = isflor - 2;
		// F L O O R S
		//
		int surflag = ((isflor<<2)+3);
		if (isportal && !noportals) {
			int endpn = portals[myport].destpn;
			int ttag = b->tagoffset + taginc + portals[endpn].sect;
			int portalpolyflags =  ((isflor<<2)+3) | DP_NO_SCANSECT;
			int portaltag = b->tagoffset + taginc -1;

		//	drawpol_befclip(s+b->tagoffset, portaltag, s, portals[endpn].sect,plothead[0],plothead[1], portalpolyflags , b);
			//int c1, c2;
			//monocopy(plothead[0],plothead[1], &c1,&c2);
			draw_hsr_enter_portal(map, myport, plothead[0],plothead[1],b);
		}

		else {
			drawpol_befclip(s+b->tagoffset,-1,s,-1,plothead[0],plothead[1],surflag,b);
		}
	}
	b->gisflor = 2;
	// === DRAW WALLS ===
	for(ww=0;ww<twaln;ww++)
	{

		// Get wall vertices and setup wall segment
		vn = getwalls_imp(s,twal[ww].i,verts,MAXVERTS,map);
		w = twal[ww].i; nw = wal[w].n+w;
		sur = &wal[w].xsurf[0];

		int myport = wal[w].tags[1]; // FLOOR PORTAL CHECK
		bool isportal = myport >= 0
						&& !noportals
						&& portals[myport].destpn >= 0
						&& portals[myport].kind == PORT_WALL;
						//&& portals[myport].surfid == w;
		bool skipport = shadowtest2_debug_block_selfportals
						&& b->has_portal_clip
						&& isportal
						&& s == b->testignoresec
						&& portals[myport].kind == b->ignorekind
						&& ww == b->testignorewall;
		if (skipport)
			continue;

		// Calculate wall length and setup color/normal
		dx = sqrt((wal[nw].x-wal[w].x)*(wal[nw].x-wal[w].x) + (wal[nw].y-wal[w].y)*(wal[nw].y-wal[w].y));
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
					gtilenum = sur->tilnum;
					//gtpic = &gtile[sur->tilnum];// if (!gtpic->tt.f) loadpic(gtpic);
				if (sur->flags & (1 << 17))	{ b->gflags = 2; } //skybox ceil/flor
				if (sur->flags & (1 << 16))  b->gflags = 1;
{
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
					if (!(sur->flags & 4)) f = sec[s].z[0]; // default is ceil align
					else if (!vn) f = sec[s].z[1]; //White walls don't have verts[]! and align is different.
					else if (!m) f = sec[verts[0].s].z[0]; //
					else f = sec[verts[(m - 1) >> 1].s].z[0];
					// Apply UV coordinates with proper scaling
					//npol2[0].u = sur->uv[0].x;
					//npol2[0].v = sur->uv[2].y * (npol2[0].z - f) + sur->uv[0].y;
					//npol2[1].u = sur->uv[1].x * dx + npol2[0].u;
					//npol2[1].v = sur->uv[1].y * dx + npol2[0].v;
					//npol2[2].u = sur->uv[2].x + npol2[0].u;
					//npol2[2].v = sur->uv[2].y + npol2[0].v;
					b->gflags = 0;
					gentransform_wall(npol2, sur, b);
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
			// W A L L S
			//
			myport = wal[w].tags[1];
			int surflag = ((m > vn) << 2) + 3;
			int newtag = ns == -1 ? -1 : ns+b->tagoffset;
			if (isportal) {
				int endp = portals[myport].destpn;
				int portalpolyflags = surflag | DP_NO_SCANSECT;
				int portaltag = +b->tagoffset + taginc -1;
				int endpn = portals[myport].destpn;
				int ttag = b->tagoffset + taginc + portals[endpn].sect;

			//	drawpol_befclip(s+b->tagoffset, portaltag,s,portals[endp].sect, plothead[0], plothead[1],  portalpolyflags, b);
			//int c1, c2;
			//monocopy(plothead[0],plothead[1], &c1,&c2);
				draw_hsr_enter_portal(map, myport, plothead[0],plothead[1],b);
			} else {
				// could be 7 or 3, .111 or .011
				logstep("Draw wal pol s:%d ns:%d tag:%d",s,ns,wal[w].surf.lotag);
				drawpol_befclip(s+b->tagoffset, newtag, s, ns, plothead[0], plothead[1], surflag, b);
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
int lastvalidsec=0;
void draw_hsr_polymost(cam_t *cc, mapstate_t *map, int dummy){
	bdrawctx bs;
	loopnum=0;
	//operstopn=-1;
	bs.cam = *cc;
	bs.movedcam = *cc;
	bs.orcam = *cc;
	bs.recursion_depth = 0;
	bs.has_portal_clip = false;
	bs.tagoffset=0;
	bs.ismirrored = false;
opercurr = 0;
	draw_hsr_polymost_ctx(map,&bs);

}

void draw_hsr_polymost_ctx (mapstate_t *lgs, bdrawctx *newctx) {
	if (!newctx) {
		return;
	}
	int recursiveDepth = newctx->recursion_depth;
	bdrawctx *b;
	b = newctx;
	b->sectgotn = 0;
	b->sectgot = 0;
	b->sectgotmal = 0;
	b->bunchgot=0;
	b->bunchn=0;
	b->bunchmal=0;
	b->bunchgrid =0;
	cam_t gcam = b->cam;

	if (gcam.cursect >=0)
		lastvalidsec = gcam.cursect;
	else
        gcam.cursect = lastvalidsec;

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

	//if ((lgs->numsects <= 0) || ((unsigned)cam.cursect >= (unsigned)lgs->numsects))
	//{
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


		//Hack to keep camera away from sector line; avoids clipping glitch in drawpol_befclip/changetagfunc
//wal = lgs->sect[cam.cursect].wall;
//for(i=lgs->sect[cam.cursect].n-1;i>=0;i--)
//{
//	#define WALHAK 1e-3
//	j = wal[i].n+i;
//	d = distpoint2line2(cam.p.x,cam.p.y,wal[i].x,wal[i].y,wal[j].x,wal[j].y); if (d >= WALHAK*WALHAK) continue;
//	fp.x = wal[j].x-wal[i].x;
//	fp.y = wal[j].y-wal[i].y;
//	f = (WALHAK - sqrt(d))/sqrt(fp.x*fp.x + fp.y*fp.y);
//	cam.p.x -= fp.y*f;
//	cam.p.y += fp.x*f;
//}

	if (shadowtest2_rendmode != 4)
	{
			//Horrible hacks for internal build2 global variables
		dpos.x = 0.0; dpos.y = 0.0; dpos.z = 0.0;
		drig.x = 1.0; drig.y = 0.0; drig.z = 0.0;
		ddow.x = 0.0; ddow.y = 1.0; ddow.z = 0.0;
		dfor.x = 0.0; dfor.y = 0.0; dfor.z = 1.0;
	//	drawpoly_setup(                           (tiletype *)&cam.c,cam.z.f-cam.c.f,&dpos,&drig,&ddow,&dfor,cam.h.x,cam.h.y,cam.h.z);
		//drawcone_setup(cputype,shadowtest2_numcpu,(tiletype *)&cam.c,cam.z.f-cam.c.f,&dpos,&drig,&ddow,&dfor,cam.h.x,cam.h.y,cam.h.z);
		// drawkv6_setup(&drawkv6_frame,            (tiletype *)&cam.c,cam.z.f-cam.c.f,&dpos,&drig,&ddow,&dfor,cam.h.x,cam.h.y,cam.h.z);

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
int wasclipped = 0;
	int passcomplete =0;
	for(int pass=0;pass<2;pass++) {

		if (!b->has_portal_clip) {
			b->currenthalfplane = pass;
			halfplane = pass;
		}
		else {
			halfplane = pass;
		}
		logstep("Pass start pass:%d, hfp:%d, depth:%d, camsec:%d", pass, halfplane, b->recursion_depth, b->cam.cursect);

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
		} else {
			xformprep(((double) halfplane) * PI, b);

			if (!b->has_portal_clip) {
				// store original if it is first context;
				b->oxformmatc = b->xformmatc;
				b->oxformmats = b->xformmats;
				b->ognadd = b->gnadd;
				memcpy(&b->oxformmat, &b->xformmat, sizeof(double) * 9);
			}
				// NEW CODE - Use much larger bounds:
				float large_bound = 1e6f;
				xformbac(-large_bound, -large_bound, gcam.h.z, &bord[0], b);
				xformbac(+large_bound, -large_bound, gcam.h.z, &bord[1], b);
				xformbac(+large_bound, +large_bound, gcam.h.z, &bord[2], b);
				xformbac(-large_bound, +large_bound, gcam.h.z, &bord[3], b);

				//Clip screen to front plane
				n = 0;
				didcut = 0;
				for (i = 4 - 1, j = 0; j < 4; i = j, j++) {
					if (bord[i].z >= SCISDIST) {
						bord2[n] = bord[i];
						n++;
					}
					if ((bord[i].z >= SCISDIST) != (bord[j].z >= SCISDIST)) {
						f = (SCISDIST - bord[i].z) / (bord[j].z - bord[i].z);
						bord2[n].x = (bord[j].x - bord[i].x) * f + bord[i].x;
						bord2[n].y = (bord[j].y - bord[i].y) * f + bord[i].y;
						bord2[n].z = (bord[j].z - bord[i].z) * f + bord[i].z;
						n++;
						didcut = 1;
					}
				}
				if (n < 3) {
					continue;
					printf("n<3 1");
				}

				for(j=0;j<n;j++)
				{
					f = gcam.h.z/bord2[j].z;
					bord2[j].x = bord2[j].x*f + gcam.h.x;
					bord2[j].y = bord2[j].y*f + gcam.h.y;
				}
		}  // need to draw reproject original opening unfortunately.

		memset8(b->sectgot,0,(lgs->numsects+31)>>3);
if (b->recursion_depth==2)
	int a =1;
		int passmphstart;
		if (!b->has_portal_clip) {
			//FIX! once means not each frame! (of course it doesn't hurt functionality)
			// Standard case: clear existing state and create new viewport
			for (i = mphnum - 1; i >= 0; i--) {
				mono_deloop(mph[i].head[1]);
				mono_deloop(mph[i].head[0]);
			}
			// maybe need second run for alternating mono?
			mono_genfromloop(&mph[0].head[0], &mph[0].head[1], bord2, n);
			mph[0].tag = gcam.cursect;
			mphnum = 1;
		} else {
			//drawpol_befclip(gcam.cursect-taginc, gcam.cursect, gcam.cursect,	b->chead[0],b->chead[1], 8|3 , b);
			int res=-1;
			mphremoveaboveincl(b->tagoffset); // clean anything above.
{
				if (n < 3) {

					printf("n<3 2");continue;
				}
				int bh1 = -1, bh2 = -1;

				mono_genfromloop(&bh1, &bh2, bord2, n);
				bool bordok = (mpcheck(bh1,bh2)); if (!bordok) {
					printf("bordok not");
					continue;
				}


				int portaltag = b->tagoffset - 1;
				int newtag = gcam.cursect + b->tagoffset;
				int whead[2]={-1,-1};
				int bordar[] = {bh1,bh2};
					if (!wasclipped) {
						bool wok = (mpcheck(b->chead[0], b->chead[1])); // invalid window
							if (!wok) {
								logstep("failed portal window chain");
								printf("window not");
								return;
							}
						for (int h = 0; h < 2; h++) {
							i = b->chead[h];
							do {
								if (h) i = mp[i].p;
								// must find previous in coords of new, may need previous camera, not orcam.
								wccw_transform(&mp[i].pos, &b->prevcam, &b->movedcam);
								if (!h) i = mp[i].n;
							} while (i != b->chead[h]);
						}
						monocopy(b->chead[0], b->chead[1],&whead[0],&whead[1]);
						// reproject original opening.
						// MOVE TO ENTER PORTAL, to reproject from previous cam.
						wasclipped = 1;
						}
					else { //
						whead[0] = b->chead[0];
						whead[1] = b->chead[1];
					}

					res = projectonmono(&whead[0], &whead[1], b);
					if (!res) {
						continue;
					}
					//do AND with board and add only clipped portion to MPH.
					b->gdoscansector=0;
					b->gnewtag=gcam.cursect + b->tagoffset;
					// and swap of indices is necessary.
				if (b->ismirrored)
					mono_bool(whead[1],whead[0],bh1,bh2,MONO_BOOL_AND,b,changetagfunc);
				else
					mono_bool(whead[0],whead[1],bh1,bh2,MONO_BOOL_AND,b,changetagfunc);

				//mono_dbg_capture_mph(mphnum - 1, "reprojected");
				//	mono_deloop(bh1);
				//	mono_deloop(bh2);
			}
		}

		b->bunchn = 0; scansector(gcam.cursect,b);
		while (b->bunchn)
		{
			memset(b->bunchgot,0,(b->bunchn+7)>>3);

			for(i=b->bunchn-1;i>0;i--) //assume: bunchgrid[(((j-1)*j)>>1)+i] = bunchfront(j,i,0); is valid iff:{i<j}
			{
				for (k = (((i - 1) * i) >> 1), j = 0; j < i; k += 1, j++)
					if (b->bunchgrid[k] & 2) goto nogood;
				for (k += j, j++; j < b->bunchn; k += j, j++)
					if (b->bunchgrid[k] & 1) goto nogood;
				break;
nogood:; }
			closest = i;

			drawalls(closest,lgs,b);
		}

		if (shadowtest2_rendmode == 4) uptr = glp->sectgot;
		else uptr = shadowtest2_sectgot;


		if (!pass) // write only after first pass.
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
	if (!b->has_portal_clip)
		if (!didcut) {
			logstep("break on no cut: pass:%d, hfp:%d, depth:%d, camsec:%d", pass, halfplane, b->recursion_depth, b->cam.cursect);
			break;
		}
		passcomplete=1;
	}
	if (b->has_portal_clip) {
		logstep("mph clean after passes");
		mphremoveaboveincl(b->tagoffset-1);
	}
}

static void draw_hsr_enter_portal(mapstate_t* map, int myport,  int head1, int head2,bdrawctx *parentctx)
{
	if (parentctx->recursion_depth == 1)
		lastcamtr = parentctx->orcam.tr;
	OPERLOG;
    if (parentctx->recursion_depth >= MAX_PORTAL_DEPTH) {
        return;
    }
	bdrawctx newctx = {};
    cam_t movcam = parentctx->movedcam;
    int endp = portals[myport].destpn;
    int entry = portals[myport].anchorspri;
    int tgtspi = portals[endp].anchorspri;
    int ignw = portals[endp].surfid;
    int igns = portals[endp].sect;

    spri_t tgs = map->spri[tgtspi];
    spri_t ent = map->spri[entry];

    // Normalize transforms to ensure orthonormality
    normalize_transform(&ent.tr);
    normalize_transform(&tgs.tr);

    // Step 1: Transform camera to entry portal's local space
    // This finds the camera's position and orientation RELATIVE to the entry portal
    point3d cam_local_pos = world_to_local_point(movcam.p, &ent.tr);
    point3d cam_local_r = world_to_local_vec(movcam.r, &ent.tr);
    point3d cam_local_d = world_to_local_vec(movcam.d, &ent.tr);
    point3d cam_local_f = world_to_local_vec(movcam.f, &ent.tr);

    // Step 2: Apply that same relative transform from the target portal's perspective
    // Since entry.forward points IN and target.forward points OUT (already opposite),
    // we just transform directly without any flips
    movcam.p = local_to_world_point(cam_local_pos, &tgs.tr);
    movcam.r = local_to_world_vec(cam_local_r, &tgs.tr);
    movcam.d = local_to_world_vec(cam_local_d, &tgs.tr);
    movcam.f = local_to_world_vec(cam_local_f, &tgs.tr);
	movcam.cursect = portals[endp].sect;
// to avoid winding problems with mono, we render with normalized camera
// then in draw eyepol we can just flip polygons as if camera was really flipped.
// the only thing important is board output, as orientation is preserved in movedcam.
	cam_t rencam = movcam;
	rencam.r = normalizep3(crossp3(movcam.d,movcam.f));

	bool portalflipped = is_transform_flipped(&tgs.tr) ^  is_transform_flipped(&ent.tr);
	// we need to know only about flip in current portal switch to flip or not to flip the opening.
	newctx.ismirrored = portalflipped;
	newctx.entrysec = portals[myport].sect;
	newctx.recursion_depth = parentctx->recursion_depth + 1;
	newctx.tagoffset = (newctx.recursion_depth)*taginc;
    newctx.cam = rencam;
    newctx.movedcam = movcam;
    newctx.prevcam = parentctx->movedcam;
    newctx.orcam = parentctx->orcam;
    newctx.has_portal_clip = true;
    newctx.sectgotn = 0;
    newctx.sectgot = 0;
    newctx.sectgotmal = 0;
    newctx.bunchgot = 0;
    newctx.bunchn = 0;
    newctx.bunchmal = 0;
    newctx.bunchgrid = 0;
    newctx.testignorewall = ignw;
    newctx.testignoresec = igns;
    newctx.ignorekind = portals[endp].kind;
    newctx.gnewtagsect = -1;
    newctx.gnewtag = -1;
    newctx.currenthalfplane = parentctx->currenthalfplane;
	// propagate original xforms
	newctx.oxformmatc = parentctx->oxformmatc;
	newctx.oxformmats = parentctx->oxformmats;
	newctx.ognadd = parentctx->ognadd;
	memcpy(&newctx.oxformmat, &parentctx->oxformmat, sizeof(double)*9);
	newctx.chead[0] = head1;
	newctx.chead[1] = head2;

    draw_hsr_polymost_ctx(map, &newctx);

	OPERLOG;
}


typedef struct { int sect; point3d p; float rgb[3]; int useshadow; } drawkv6_lightpos_t;
void drawsprites ()
{
}

void shadowtest2_setcam (cam_t *ncam)
{
	//cam = *ncam;
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
