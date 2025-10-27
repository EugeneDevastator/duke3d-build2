//
// Created by omnis on 10/18/2025.
//
#ifndef BUILD2_MAPCORE_H
#define BUILD2_MAPCORE_H
#pragma once
#include <math.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#include "kplib.h"
#include "artloader.h"
#ifndef PI
#define PI 3.141592653589793
#endif
typedef struct { float x, y; } point2d;
extern long gnumtiles, gmaltiles, gtilehashead[1024];
static char curmappath[MAX_PATH+1] = "";
long get_gnumtiles(void);
long get_gmaltiles(void);
long* get_gtilehashead(void);


#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


#ifndef KEN_DRAWPOLY_H
//typedef struct tiltyp {
//	long f, p, x, y, z;
//	float shsc;
//	struct tiltyp *lowermip;
//} tiltyp; //shsc=suggested height scale
typedef struct { float x, y, z; } point3d;
typedef struct { double x, y, z; } dpoint3d; 	//Note: pol doesn't support loops as dpoint3d's!

typedef struct { intptr_t f; int p, x, y; } tiletype;
typedef struct { tiltyp c, z; point3d p, r, d, f, h; } cam_t;

#endif

typedef struct { int w, s; } vertlist_t;
typedef struct { float x, y, z, u, v; int n; } kgln_t;
typedef struct { double x, y, z; long n, filler; } genpoly_t;


	//Map format:
typedef struct
{
	long tilnum, tilanm/*???*/;

	//Bit0:Blocking, Bit2:RelativeAlignment, Bit5:1Way, Bit16:IsParallax, Bit17:IsSkybox
	union { long flags; struct { char _f1, _f2, _f3, pal; }; }; // temporary pal storage
	union { long tag; struct { short lotag, hitag; }; };

	point2d uv[3];
	unsigned short asc, rsc, gsc, bsc; //4096 is no change
} surf_t;

typedef struct
{
	float x, y;
	long n, ns, nw; //n:rel. wall ind.; ns & nw : nextsect & nextwall_of_sect
	long owner; //for dragging while editing, other effects during game
	long surfn;
	surf_t surf, *xsurf; //additional malloced surfs when (surfn > 1)
} wall_t;

typedef struct
{
	point3d p, r, d, f;      //Position, orientation (right, down, forward)
	point3d v, av;           //Position velocity, Angular velocity (direction=axis, magnitude=vel)
	float fat, mas, moi;     //Physics (moi=moment of inertia)
	long tilnum;             //Model file. Ex:"TILES000.ART|64","CARDBOARD.PNG","CACO.KV6","HAND.KCM","IMP.MD3"
	unsigned short asc, rsc, gsc, bsc; //Color scales. 4096 is no change
	long owner;
	union { long tag; struct { short lotag, hitag; }; };
	long tim, otim;          //Time (in milliseconds) for animation

	//Bit0:Blocking, Bit2:1WayOtherSide, Bit5,Bit4:Face/Wall/Floor/.., Bit6:1side, Bit16:IsLight, Bit17-19:SpotAx(1-6), Bit20-29:SpotWid, Bit31:Invisible
	union { long flags; struct { char _f1, _f2, _f3, pal; }; }; // temporary pal storage

	long sect, sectn, sectp; //Current sector / doubly-linked list of indices
} spri_t;

typedef struct
{
	float minx, miny, maxx, maxy; //bounding box
	float z[2];      //ceil&flor height
	point2d grad[2]; //ceil&flor grad. grad.x = norm.x/norm.z, grad.y = norm.y/norm.z
	surf_t surf[2];  //ceil&flor texture info
	wall_t *wall;
	long n, nmax;    //n:numwalls, nmax:walls malloced (nmax >= n)
	long headspri;   //head sprite index (-1 if none)
	long foglev;
	long owner;      //for dragging while editing, other effects during game
} sect_t;
//--------------------------------------------------------------------------------------------------
typedef struct
{
	point3d startpos, startrig, startdow, startfor;
	int numsects, malsects; sect_t *sect;
	int numspris, malspris; spri_t *spri;
	int blankheadspri;

#define MAXLIGHTS 256
	int light_spri[MAXLIGHTS], light_sprinum;
} mapstate_t;

static _inline void dcossin (double a, double *c, double *s)
{
	_asm
	{
		fld a
		fsincos
		mov eax, c
		fstp qword ptr [eax]
		mov eax, s
		fstp qword ptr [eax]
	}
}
//General point-polygon distance function. Single loops only. For multi, use genpoly_t and .n for next
static double ptpolydist2 (dpoint3d *pt, dpoint3d *pol, int n, dpoint3d *closest)
{
	double d, f, g, dmin, dx, dy, dz, nx, ny, nz, x0, y0, x1, y1;
	int i, j, k, maxnormaxis;

		//test inside poly
	nx = ny = nz = 0.0;
	for(i=n-2;i>0;i--) //Find normal
	{
		nx += (pol[i].y-pol[0].y)*(pol[i+1].z-pol[0].z) - (pol[i].z-pol[0].z)*(pol[i+1].y-pol[0].y);
		ny += (pol[i].z-pol[0].z)*(pol[i+1].x-pol[0].x) - (pol[i].x-pol[0].x)*(pol[i+1].z-pol[0].z);
		nz += (pol[i].x-pol[0].x)*(pol[i+1].y-pol[0].y) - (pol[i].y-pol[0].y)*(pol[i+1].x-pol[0].x);
	}
	f = nx*nx + ny*ny + nz*nz;
	if (f > 0.0) //Plane must have area
	{
		d = ((pol[0].x-pt->x)*nx + (pol[0].y-pt->y)*ny + (pol[0].z-pt->z)*nz); g = d/f;
		dx = nx*g + pt->x;
		dy = ny*g + pt->y;
		dz = nz*g + pt->z;
		if ((fabs(nx) > fabs(ny)) && (fabs(nx) > fabs(nz))) maxnormaxis = 0;
		else if (fabs(ny) > fabs(nz)) maxnormaxis = 1; else maxnormaxis = 2;
		for(i=n-1,j=k=0;j<n;i=j,j++)
		{
			if (maxnormaxis > 0) { x0 = pol[i].x - dx; x1 = pol[j].x - dx; }
								 else { x0 = pol[i].y - dy; x1 = pol[j].y - dy; }
			if (maxnormaxis > 1) { y0 = pol[i].y - dy; y1 = pol[j].y - dy; }
								 else { y0 = pol[i].z - dz; y1 = pol[j].z - dz; }
			if (y0*y1 < 0.0)
			{
				if (x0*x1 >= 0.0) { if (x0 < 0.0) k++; }
				else if ((x0*y1 - x1*y0)*y1 < 0.0) k++;
			}
		}
		if (k&1) { closest->x = dx; closest->y = dy; closest->z = dz; return(d*g); }
	}

	dmin = 1e32;
	for(i=n-1,j=0;j<n;i=j,j++)
	{
		nx = pt->x - pol[i].x; dx = pol[j].x - pol[i].x;
		ny = pt->y - pol[i].y; dy = pol[j].y - pol[i].y;
		nz = pt->z - pol[i].z; dz = pol[j].z - pol[i].z;

			//Edge
		f = nx*dx + ny*dy + nz*dz;
		if (f <= 0.0)
		{
				//Vertex
			d = nx*nx + ny*ny + nz*nz;
			if (d < dmin) { dmin = d; closest->x = pol[i].x; closest->y = pol[i].y; closest->z = pol[i].z; }
			continue;
		}
		g = dx*dx + dy*dy + dz*dz; if (f >= g) continue;
		f /= g;
		nx = dx*f + pol[i].x;
		ny = dy*f + pol[i].y;
		nz = dz*f + pol[i].z;
		d = (pt->x-nx)*(pt->x-nx) + (pt->y-ny)*(pt->y-ny) + (pt->z-nz)*(pt->z-nz);
		if (d < dmin) { dmin = d; closest->x = nx; closest->y = ny; closest->z = nz; }
	}
	return(dmin);
}
double roundcylminpath2 (double a0x, double a0y, double a1x, double a1y,
								 double b0x, double b0y, double b1x, double b1y);
long wallclippol (kgln_t *pol, kgln_t *npol);
int dupwall_imp (sect_t *s, int w);
long sect_isneighs_imp (int s0, int s1, mapstate_t* map);
double getslopez (sect_t *s, int i, double x, double y);
int wallprev (sect_t *s, int w);
int getwalls_imp (int s, int w, vertlist_t *ver, int maxverts, mapstate_t* map);
int getverts_imp (int s, int w, vertlist_t *ver, int maxverts, mapstate_t* map);
long insspri_imp (int sect, float x, float y, float z, mapstate_t *map);
void delspri_imp (int i, mapstate_t *map);
void changesprisect_imp (int i, int nsect, mapstate_t *map);
//Clip wall slopes. Returns loop ordered poly (0, 3, or 4 points)
//pol[0]   pol[1]
//pol[3]   pol[2]
static long wallclip (dpoint3d *pol, dpoint3d npol[4])
{
	double f, dz0, dz1;

	dz0 = pol[3].z-pol[0].z; dz1 = pol[2].z-pol[1].z;
	if (dz0 >= 0.0) //Include null case for collision
	{
		npol[0] = pol[0];
		if (dz1 >= 0.0) //Include null case for collision
		{
			npol[1] = pol[1];
			npol[2] = pol[2];
			npol[3] = pol[3];
			return(4);
		}
		else
		{
			f = dz0/(dz0-dz1);
			npol[1].x = (pol[1].x-pol[0].x)*f + pol[0].x;
			npol[1].y = (pol[1].y-pol[0].y)*f + pol[0].y;
			npol[1].z = (pol[1].z-pol[0].z)*f + pol[0].z;
			npol[2] = pol[3];
			return(3);
		}
	}
	if (dz1 < 0.0) return(0); //Include null case for collision
	f = dz0/(dz0-dz1);
	npol[0].x = (pol[1].x-pol[0].x)*f + pol[0].x;
	npol[0].y = (pol[1].y-pol[0].y)*f + pol[0].y;
	npol[0].z = (pol[1].z-pol[0].z)*f + pol[0].z;
	npol[1] = pol[1];
	npol[2] = pol[2];
	return(3);
}

//Split complex polygon by line. Returns complex polygon.
// owal[],on: input wall list
//    retwal: output wall list (malloced inside polyspli if not null)
//(kx,ky,ka): valid half-space test (x*kx + y*ky + ka > 0)
//   returns: # output walls


static double distpoint2line2 (double x, double y, double x0, double y0, double x1, double y1)
{
	double f, g, dx, dy, nx, ny;
	dx = x1-x0; dy = y1-y0; nx = x-x0; ny = y-y0;
	f = nx*dx + ny*dy; if (f <=0.f) return(nx*nx + ny*ny); //behind point 0?
	g = dx*dx + dy*dy; if (f >=  g) return((x-x1)*(x-x1) + (y-y1)*(y-y1)); //behind point 1?
	f = nx*dy - ny*dx; return(f*f/g); //perpendicular distance to line
}

static int polyspli (wall_t *owal, int on, wall_t **retwal, double kx, double ky, double ka)
{
	typedef struct { float x, y; long n, i; } polspli_t;
	polspli_t *tpal;
	wall_t *twal;
	double t, ti, tj;
	int i, j, k, n, oi, n2, on2, startn, *spi[2], spin[2];
	char *got;

	tpal = (polspli_t *)_alloca(on*2*sizeof(polspli_t)); if (!tpal) return(0);
	spi[0] = (int *)_alloca(((on+1)>>1)*sizeof(wall_t)); if (!spi[0]) return(0);
	spi[1] = (int *)_alloca(((on+1)>>1)*sizeof(wall_t)); if (!spi[1]) return(0);

		//Clip poly to line, noting intersections to spi[]
	n = 0; startn = 0; spin[0] = 0; spin[1] = 0;
	for(i=0;i<on;i++)
	{
		j = owal[i].n+i;
		ti = owal[i].x*kx + owal[i].y*ky + ka;
		tj = owal[j].x*kx + owal[j].y*ky + ka;
		if (ti > 0.0)
		{
			tpal[n].x = owal[i].x; tpal[n].y = owal[i].y;
			tpal[n].n = 1; tpal[n].i = i; n++;
		}
		if ((ti > 0.0) != (tj > 0.0))
		{
			t = ti/(ti-tj); k = (!(ti > 0)); spi[k][spin[k]] = n; spin[k]++;
			tpal[n].x = (owal[j].x-owal[i].x)*t + owal[i].x;
			tpal[n].y = (owal[j].y-owal[i].y)*t + owal[i].y;
			tpal[n].n = 1; tpal[n].i = i; n++;
		}
		if ((j < i) && (n-startn >= 3)) { tpal[n-1].n = startn-n+1; startn = n; }
	}
	if (n < 3) return(0);
	if (!retwal) return(n); //Hack to return # walls only

		//Sort intersections..
	for(k=2-1;k>=0;k--)
		for(j=1;j<spin[k];j++)
			for(i=0;i<j;i++)
				if ((tpal[spi[k][i]].x-tpal[spi[k][j]].x)*ky >
					 (tpal[spi[k][i]].y-tpal[spi[k][j]].y)*kx)
					{ t = spi[k][i]; spi[k][i] = spi[k][j]; spi[k][j] = t; }

		//Re-map intersections..
	for(i=0;i<spin[0];i++) tpal[spi[0][i]].n = spi[1][i]-spi[0][i];

	got = (char *)_alloca(n*sizeof(got[0])); if (!got) return(0);
	(*retwal) = (wall_t *)malloc(n*sizeof(wall_t)); if (!(*retwal)) return(0);

		//De-spaghettify loops (put in sequential order)
	for(i=0;i<n;i++) got[i] = 0;
	n2 = 0; i = 0;
	while (1)
	{
		on2 = n2; oi = i;
		do
		{
			(*retwal)[n2].x = tpal[i].x; (*retwal)[n2].y = tpal[i].y; (*retwal)[n2].n = 1;
			(*retwal)[n2].ns = -1; (*retwal)[n2].nw = -1;
			twal = &owal[tpal[i].i];
			(*retwal)[n2].owner = twal->owner;
			(*retwal)[n2].surf  = twal->surf; (*retwal)[n].surf.flags &= ~0x20;/*annoying hack to disable 1-way walls*/
			(*retwal)[n2].surfn = twal->surfn;
			(*retwal)[n2].xsurf = twal->xsurf;
			n2++;

			got[i] = 1; i += tpal[i].n;
		} while (i != oi);
		(*retwal)[n2-1].n = on2-(n2-1);
		while (got[i]) { i++; if (i >= n) return(n2); }
	}
}

	//Split wall list, slope vs. slope (helper function)
static int polyspli2_imp (wall_t *owal, int on, wall_t **retwal, long sec0, long isflor0,
																		long sec1, long isflor1, long dir, mapstate_t *map)
{
	double nx0, ny0, ox0, oy0, oz0, nx1, ny1, ox1, oy1, oz1;
	sect_t *sec = map->sect;

		//(x-sec[i].wal[0].x)*sec[i].grad[0].x + (y-sec[i].wal[0].y)*sec[i].grad[0].y + (z-sec[i].z[0])*1 > 0 //ceil i
		//(x-sec[i].wal[0].x)*sec[i].grad[1].x + (y-sec[i].wal[0].y)*sec[i].grad[1].y + (z-sec[i].z[1])*1 < 0 //flor i
		//(x-sec[j].wal[0].x)*sec[j].grad[0].x + (y-sec[j].wal[0].y)*sec[j].grad[0].y + (z-sec[j].z[0])*1 > 0 //ceil j
		//(x-sec[j].wal[0].x)*sec[j].grad[1].x + (y-sec[j].wal[0].y)*sec[j].grad[1].y + (z-sec[j].z[1])*1 < 0 //flor j
		//
		//(x-ox0)*nx0 + (y-oy0)*ny0 + (z-oz0) > 0
		//(x-ox1)*nx1 + (y-oy1)*ny1 + (z-oz1) < 0
		//
		//x*(nx0-nx1) + y*(ny0-ny1) + ox1*nx1 + oy1*ny1 + oz1 - ox0*nx0 - oy0*ny0 - oz0 = 0
	nx0 = sec[sec0].grad[isflor0].x; ny0 = sec[sec0].grad[isflor0].y; ox0 = sec[sec0].wall[0].x; oy0 = sec[sec0].wall[0].y; oz0 = sec[sec0].z[isflor0];
	nx1 = sec[sec1].grad[isflor1].x; ny1 = sec[sec1].grad[isflor1].y; ox1 = sec[sec1].wall[0].x; oy1 = sec[sec1].wall[0].y; oz1 = sec[sec1].z[isflor1];
	if (!dir) return(polyspli(owal,on,retwal,nx0-nx1,ny0-ny1,ox1*nx1 + oy1*ny1 + oz1 - ox0*nx0 - oy0*ny0 - oz0));
		  else return(polyspli(owal,on,retwal,nx1-nx0,ny1-ny0,ox0*nx0 + oy0*ny0 + oz0 - ox1*nx1 - oy1*ny1 - oz1));
}

enum { POLYBOOL_AND,POLYBOOL_SUB,POLYBOOL_SUBR,POLYBOOL_OR,/*POLYBOOL_XOR,*/POLYBOOL_END };
typedef struct { double x0, y0, x1, y1; int i; } polbool_lin_t; //FIXFIX:try float?

//returns 1 if (x,y) is on border; else 0
static int onpoly (double x, double y, wall_t *wal, int n)
{
	int i, j;

	for(i=n-1;i>=0;i--) //FIX:visit bbox(x,y,x,y){wal,n}
	{
		if ((x == wal[i].x) && (y == wal[i].y)) return(1);
		j = wal[i].n+i;
		if ((wal[i].x < x) != (x < wal[j].x)) continue;
		if ((wal[i].y < y) != (y < wal[j].y)) continue;
		if ((wal[j].x-x)*(wal[i].y-y) == (wal[i].x-x)*(wal[j].y-y)) return(1);
	}
	return(0);
}

//returns 1 if (x,y) is inside (undefined on border); else 0
static int inpoly (double x, double y, wall_t *wal, int n)
{
	int i, j, c;

	c = 0;
	for(i=n-1;i>=0;i--) //FIX:visit bbox(x,y,x,y){wal,n}
	{
		j = wal[i].n+i; if ((wal[i].y < y) != (y <= wal[j].y)) continue;
		if (((wal[j].x-wal[i].x)*(y-wal[i].y) <
			  (wal[j].y-wal[i].y)*(x-wal[i].x)) != (wal[j].y < wal[i].y)) c ^= 1;
	}
	return(c);
}


static int polbool_splitlinepoint (polbool_lin_t **lin, int *linmal, wall_t *wal, int n, wall_t *owal, int on)
{
	double x0, y0, x1, y1, ix, iy;
	int i, j;

	if ((*linmal) < n) { (*linmal) = fmax(n,256); (*lin) = (polbool_lin_t *)realloc(*lin,(*linmal)*sizeof(polbool_lin_t)); }

	for(i=0;i<n;i++)
	{
		(*lin)[i].x0 = wal[i].x; (*lin)[i].y0 = wal[i].y; j = wal[i].n+i;
		(*lin)[i].x1 = wal[j].x; (*lin)[i].y1 = wal[j].y; (*lin)[i].i = i;
	}
	for(i=0;i<n;i++)
	{
		x0 = (*lin)[i].x0; y0 = (*lin)[i].y0; x1 = (*lin)[i].x1; y1 = (*lin)[i].y1;
		for(j=0;j<on;j++) //FIX:visit bbox(x0,y0,x1,y1){owal,on}
		{
			ix = owal[j].x; iy = owal[j].y;

			//if in middle of line segment..
			if ((x0 < ix) != (ix < x1)) continue;
			if ((y0 < iy) != (iy < y1)) continue;
			if ((x1-ix)*(y0-iy) != (x0-ix)*(y1-iy)) continue;
			if ((ix == x0) && (iy == y0)) continue;
			if ((ix == x1) && (iy == y1)) continue;

			if (n >= (*linmal)) { (*linmal) = max((*linmal)*2,n+1); (*lin) = (polbool_lin_t *)realloc(*lin,(*linmal)*sizeof(polbool_lin_t)); }

			(*lin)[i].x1 = ix; (*lin)[i].y1 = iy;
			(*lin)[n].x0 = ix; (*lin)[n].y0 = iy;
			(*lin)[n].x1 = x1; (*lin)[n].y1 = y1; (*lin)[n].i = (*lin)[i].i; n++;
			x1 = ix; y1 = iy;
		}
	}
	return(n);
}


	// ��Ŀ
	// �A��Ŀ
	// ����B�
	//   ����
	//   Collinear line priority:
	//1st sector (wal0): POLYBOOL_AND, POLYBOOL_SUB, POLYBOOL_OR
	//2nd sector (wal1): POLYBOOL_SUB2
	//if retwal is null, returns # walls without generating wall list
static int polybool (wall_t *wal0, int on0, wall_t *wal1, int on1, wall_t **retwal, int *retn, int op)
{
	static polbool_lin_t *lin0 = 0, *lin1 = 0;
	static int lin0mal = 0, lin1mal = 0;
	wall_t *twal;
	double d, t, u, ax, ay, x0, y0, x1, y1, x2, y2, x3, y3, x10, y10, x23, y23, x20, y20, ix, iy;
	int i, j, n, n0, n1, oldn0, oi, on;

	if (retwal) { (*retwal) = 0; (*retn) = 0; }
	if ((unsigned)op >= POLYBOOL_END) return(0);

	if (op == POLYBOOL_SUBR)
	{
		twal = wal0; wal0 = wal1; wal1 = twal;
		n = on0; on0 = on1; on1 = n;
	}

		//Line vs. Point
	n0 = polbool_splitlinepoint(&lin0,&lin0mal,wal0,on0,wal1,on1);
	n1 = polbool_splitlinepoint(&lin1,&lin1mal,wal1,on1,wal0,on0);

		//Line vs. Line..
	for(i=0;i<n0;i++)
	{
		x0 = lin0[i].x0; y0 = lin0[i].y0; x1 = lin0[i].x1; y1 = lin0[i].y1;
		for(j=n1-1;j>=0;j--) //FIX:visit hash&bbox(x0,y0,x1,y1){lin1,n1}
		{
			x2 = lin1[j].x0; y2 = lin1[j].y0; x3 = lin1[j].x1; y3 = lin1[j].y1;
			if ((x2 == x0) && (y2 == y0)) continue; //This extra check is necessary
			if ((x2 == x1) && (y2 == y1)) continue; //because intersect() calculates
			if ((x3 == x0) && (y3 == y0)) continue; //d/t/u temp products differently
			if ((x3 == x1) && (y3 == y1)) continue; //giving inconsistent results :/

				//intersect() inline
				//(x1-x0)*t + (x2-x3)*u = (x2-x0)
				//(y1-y0)*t + (y2-y3)*u = (y2-y0)
			x10 = x1-x0; x23 = x2-x3; x20 = x2-x0;
			y10 = y1-y0; y23 = y2-y3; y20 = y2-y0;
			d =  x10*y23 - y10*x23;    if (d == 0.0) continue; d = 1.0/d;
			t = (x20*y23 - y20*x23)*d; if ((t <= 0.0) || (t >= 1.0)) continue;
			u = (x10*y20 - y10*x20)*d; if ((u <= 0.0) || (u >= 1.0)) continue;
			ix = x10*t + x0; iy = y10*t + y0;

			if (n0 >= lin0mal) { lin0mal = max(lin0mal*2,n0+1); lin0 = (polbool_lin_t *)realloc(lin0,lin0mal*sizeof(polbool_lin_t)); }
			if (n1 >= lin1mal) { lin1mal = max(lin1mal*2,n1+1); lin1 = (polbool_lin_t *)realloc(lin1,lin1mal*sizeof(polbool_lin_t)); }

			lin0[n0].x0 = ix; lin0[n0].x1 = lin0[i].x1; lin0[i].x1 = ix;
			lin0[n0].y0 = iy; lin0[n0].y1 = lin0[i].y1; lin0[i].y1 = iy;
			lin0[n0].i = lin0[i].i; n0++;

			lin1[n1].x0 = ix; lin1[n1].x1 = lin1[j].x1; lin1[j].x1 = ix;
			lin1[n1].y0 = iy; lin1[n1].y1 = lin1[j].y1; lin1[j].y1 = iy;
			lin1[n1].i = lin1[j].i; n1++;

			x1 = ix; y1 = iy;
		}
	}

	if ((op == POLYBOOL_SUB) || (op == POLYBOOL_SUBR))
	{
		for(i=n1-1;i>=0;i--)
		{
			d = lin1[i].x0; lin1[i].x0 = lin1[i].x1; lin1[i].x1 = d;
			d = lin1[i].y0; lin1[i].y0 = lin1[i].y1; lin1[i].y1 = d;
		}
	}

		//Delete non-output lines from lin0 soup..
	for(i=n0-1;i>=0;i--)
	{
		x0 = lin0[i].x0; x1 = lin0[i].x1; ax = (x0+x1)*.5;
		y0 = lin0[i].y0; y1 = lin0[i].y1; ay = (y0+y1)*.5;
		if (onpoly(ax,ay,wal1,on1)) //Make sure no exact matches on other loop
		{
			for(j=n1-1;j>=0;j--) //FIX:visit hash|bbox(x0,y0,x1,y1){lin1,n1}
				if ((x0 == lin1[j].x1) && (y0 == lin1[j].y1) &&
					 (x1 == lin1[j].x0) && (y1 == lin1[j].y0)) { n0--; lin0[i] = lin0[n0]; break; }
		}
		else if (inpoly(ax,ay,wal1,on1) == (op != POLYBOOL_AND)) { n0--; lin0[i] = lin0[n0]; }
	}

	if (n0+n1 >= lin0mal) { lin0mal = max(lin0mal*2,n0+n1); lin0 = (polbool_lin_t *)realloc(lin0,lin0mal*sizeof(polbool_lin_t)); }

		//Copy output lines from lin1 soup to lin0.. (delete unnecessary)
	oldn0 = n0;
	for(i=n1-1;i>=0;i--)
	{
		ax = (lin1[i].x0+lin1[i].x1)*.5;
		ay = (lin1[i].y0+lin1[i].y1)*.5;
		if (onpoly(ax,ay,wal0,on0)) continue;
		if (inpoly(ax,ay,wal0,on0) != (op != POLYBOOL_OR)) continue;
		lin0[n0] = lin1[i]; n0++; //copy to lin0/n0
	}
	if (n0 < 3) return(0);
	if (!retwal) return(n0); //Hack to return # walls only

	if (op == POLYBOOL_SUBR) { for(i= 0;i<oldn0;i++) lin0[i].i += 0x80000000; }
							  else { for(i=oldn0;i<n0;i++) lin0[i].i += 0x80000000; }

	(*retwal) = (wall_t *)malloc(n0*sizeof(wall_t)); if (!(*retwal)) return(0);

		//Convert soup to sector.. (re-loop)
	oi = 0; on = 0; n = 0;
	while (1)
	{
		j = oi; x1 = lin0[j].x0; y1 = lin0[j].y0;
		do
		{
			i = j;
			x0 = x1; x1 = lin0[i].x1;
			y0 = y1; y1 = lin0[i].y1;

			(*retwal)[n].x = x0; (*retwal)[n].y = y0; (*retwal)[n].n = 1;
			(*retwal)[n].ns = -1; (*retwal)[n].nw = -1;
			if (!(lin0[i].i&0x80000000)) twal = &wal0[lin0[i].i&0x7fffffff];
											else twal = &wal1[lin0[i].i&0x7fffffff];
			(*retwal)[n].owner = twal->owner;
			(*retwal)[n].surf  = twal->surf; (*retwal)[n].surf.flags &= ~0x20;/*annoying hack to disable 1-way walls*/
			(*retwal)[n].surfn = twal->surfn;
			(*retwal)[n].xsurf = twal->xsurf;
			lin0[i].i = -1; n++;

			for(j=n0-1;j>=0;j--) //FIX:visit hash|bbox(x1,y1,x1,y1){lin0,n0}
				if ((lin0[j].i != -1) && (lin0[j].x0 == x1) && (lin0[j].y0 == y1)) break;
		} while (j >= 0);
		if (n-on >= 3) { (*retwal)[n-1].n = on-n+1; on = n; } else { n = on; }
		do { oi++; if (oi >= n0) { (*retn) = n; return(1); } } while (lin0[oi].i == -1);
	}
}

static int insidesect (double x, double y, wall_t *wal, int w)
{
	int v, c;

	c = 0;
	for(w--;w>=0;w--)
	{
		v = wal[w].n+w; if ((wal[w].y < y) != (y <= wal[v].y)) continue;
		if ((((double)wal[v].x-(double)wal[w].x)*(y-(double)wal[w].y) <
			  ((double)wal[v].y-(double)wal[w].y)*(x-(double)wal[w].x)) != (wal[v].y < wal[w].y)) c++;
	}
	return(c&1);
}

//Pass z as >1e30 to make updatesect ignore height return first sector containing (x,y)
static void updatesect_imp (float x, float y, float z, int *cursect, mapstate_t* map)
{
	sect_t *sec;
	long *gotsect;
	int i, s, w, ns, nw, allsec, cnt, *secfif, secfifw, secfifr;

	sec = map->sect;
	s = (*cursect);
	if ((unsigned)s >= (unsigned)map->numsects) //reference invalid; brute force search
	{
		for(s=map->numsects-1;s>=0;s--)
			if (insidesect(x,y,sec[s].wall,sec[s].n))
				if ((z > 1e30) || ((z >= getslopez(&sec[s],0,x,y)) && (z <= getslopez(&sec[s],1,x,y)))) break;
		(*cursect) = s; return;
	}

	if (insidesect(x,y,sec[s].wall,sec[s].n))
		if ((z > 1e30) || ((z >= getslopez(&sec[s],0,x,y)) && (z <= getslopez(&sec[s],1,x,y)))) return;

	w = (((map->numsects+31)>>5)<<2);
	gotsect = (long *)_alloca(w);
	memset(gotsect,0,w); gotsect[s>>5] |= (1<<s);
	secfif = (int *)_alloca(map->numsects*sizeof(secfif[0]));
	secfifw = secfifr = 0;

	(*cursect) = -1; allsec = map->numsects-1;
	for(cnt=map->numsects-1;cnt>0;cnt--)
	{
		for(w=sec[s].n-1;w>=0;w--)
		{
			ns = sec[s].wall[w].ns;
			nw = sec[s].wall[w].nw;
			while (((unsigned)ns < (unsigned)map->numsects) && (ns != s))
			{
				if (!(gotsect[ns>>5]&(1<<ns)))
				{
					gotsect[ns>>5] |= (1<<ns);
					secfif[secfifw] = ns; secfifw++;
				}
				i = ns;
				ns = sec[i].wall[nw].ns;
				nw = sec[i].wall[nw].nw;
			}
		}

		if (secfifr < secfifw)
		{ s = secfif[secfifr]; secfifr++; } //breadth-first
		else
		{     //fifo was empty.. must be some disjoint sectors
			while ((allsec >= 0) && (gotsect[allsec>>5]&(1<<allsec))) allsec--;
			s = allsec; if (s < 0) break;
			gotsect[s>>5] |= (1<<s);
		}

		if (insidesect(x,y,sec[s].wall,sec[s].n))
			if ((z > 1e30) || ((z >= getslopez(&sec[s],0,x,y)) && (z <= getslopez(&sec[s],1,x,y))))
			{ (*cursect) = s; return; }
	}
}

//s: sector of sprites to check
//Pass -1 to check and compact all valid sprites
static void checksprisect_imp (int s, mapstate_t *map)
{
	sect_t *sec;
	spri_t *spr;
	int w, ns, nw, s0, s1;

	sec = map->sect;
	spr = map->spri;
#if 0
	//FIXFIX:Warning: this block has not been tested!
	if ((unsigned)s < (unsigned)map->numsects)
	{
		for(w=sec[s].headspri;w>=0;w=nw)
		{
			nw = spr[w].sectn;
			ns = spr[w].sect; updatesect_imp(spr[w].p.x,spr[w].p.y,spr[w].p.z,&ns,map);
			if (ns != spr[w].sect) changesprisect(w,ns);
		}
		return;
	}
#endif
	for(s=map->numsects-1;s>=0;s--) sec[s].headspri = -1;
	for(w=nw=0;w<map->numspris;w++)
	{
		ns = spr[w].sect; if (ns < 0) continue;
		updatesect_imp(spr[w].p.x,spr[w].p.y,spr[w].p.z,&ns,map);
		if ((unsigned)ns >= (unsigned)map->numsects) ns = spr[w].sect;

		spr[nw] = spr[w];
		spr[nw].sect = ns;
		spr[nw].sectn = sec[ns].headspri;
		spr[nw].sectp = -1;
		if (sec[ns].headspri >= 0) spr[sec[ns].headspri].sectp = nw;
		sec[ns].headspri = nw;
		nw++;
	}
#ifndef STANDALONE
	map->blankheadspri = -1;
	for(;nw<map->malspris;nw++)
	{
		map->spri[nw].sectn = map->blankheadspri;
		map->spri[nw].sectp = -1;
		map->spri[nw].sect = -1;
		if (map->blankheadspri >= 0) map->spri[map->blankheadspri].sectp = nw;
		map->blankheadspri = nw;
	}
#endif
}

//Find centroid of polygon (algo copied from TAGPNT2.BAS 09/14/2006)
static void getcentroid (wall_t *wal, int n, float *retcx, float *retcy)
{
	float r, cx, cy, x0, y0, x1, y1, area;
	int w0, w1;

		//Find centroid of polygon (works for all polygons! 06/21/1999)
	cx = cy = 0.f; area = 0.f;
	for(w0=0;w0<n;w0++)
	{
		x0 = wal[w0].x; y0 = wal[w0].y; w1 = wal[w0].n+w0;
		x1 = wal[w1].x; y1 = wal[w1].y;
		cx += ((x0+x1)*x0 + x1*x1)*(y1-y0);
		cy += ((y0+y1)*y0 + y1*y1)*(x0-x1);
		area += (x0+x1)*(y1-y0);
	}
	r = 1.0/(area*3.0); (*retcx) = cx*r; (*retcy) = cy*r; //area *= .5;
}

static float getarea (wall_t *wal, int n)
{
	float area;
	int w0, w1, w2;

		//Get area of polygon using pieces of pie (triangles)
		//(0,0),(x1,y1),(x2,y1) using z of cross product, multiply-optimized
	area = 0.f;
	for(w0=n-1;w0>=0;w0--)
	{
		w1 = wal[w0].n+w0; w2 = wal[w1].n+w1;
		area += (wal[w0].x-wal[w2].x)*wal[w1].y;
	}
	return(area*.5);
}

static void delwall_imp (sect_t *s, int w, mapstate_t* map)
{
	wall_t *wal;
	int i;

#if 0
	{ //debug only
		char snotbuf[1024];
		sprintf(snotbuf,"before delwall(wall %d) %d walls\n",w,s->n);
		for(i=0;i<s->n;i++) sprintf(&snotbuf[strlen(snotbuf)],"Wall %d: %f %f %d\n",i,wal[i].x,wal[i].y,wal[i].n);
		MessageBox(ghwnd,snotbuf,prognam,MB_OK);
	}
#endif

	if (!s->n) return;
	wal = s->wall;
	if (wal[w].n < 0) { wal[w-1].n = wal[w].n+1; }
					 else { for(i=w;wal[i].n>0;i++); wal[i].n++; }

	s->n--;
	for(i=w;i<s->n;i++) wal[i] = wal[i+1];

#if 0
	{ //debug only
		char snotbuf[1024];
		sprintf(snotbuf,"after delwall(wall %d) %d walls\n",w,s->n);
		for(i=0;i<s->n;i++) sprintf(&snotbuf[strlen(snotbuf)],"Wall %d: %f %f %d\n",i,wal[i].x,wal[i].y,wal[i].n);
		MessageBox(ghwnd,snotbuf,prognam,MB_OK);
	}
#endif
}

static void delsect_imp (int s, mapstate_t* map)
{
	sect_t *sec;
	int i;
	sec = map->sect;

	if (sec[s].wall) free(sec[s].wall);
	map->numsects--; sec[s] = sec[map->numsects];
	memset(&sec[map->numsects],0,sizeof(sect_t));
}

#endif //BUILD2_MAPCORE_H