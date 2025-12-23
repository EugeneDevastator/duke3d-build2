//
// Created by omnis on 10/18/2025.
//
#ifndef BUILD2_MAPCORE_H
#define BUILD2_MAPCORE_H
#pragma once
#include <math.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "kplib.h"
#include "artloader.h"
#include "../interfaces/shared_types.h"
#ifndef PI
#define PI 3.141592653589793
#endif

extern long gnumtiles, gmaltiles, gtilehashead[1024];
extern char curmappath[MAX_PATH+1];
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

typedef struct { intptr_t f; int p, x, y; } tiletype;
typedef struct {
	// color, zbuf
	tiltyp c, z;
	union { transform tr; struct { point3d p, r, d, f; }; };
	point3d h;
	int cursect;
} cam_t;


#endif

typedef struct { int w, s; } vertlist_t;
typedef struct { float x, y, z, u, v; int n; } kgln_t;
typedef struct { double x, y, z; long n, filler; } genpoly_t;

typedef struct {
	point3d position_offset;
	point3d rotation_axis;
	float rotation_angle;
	float transform_matrix[16];  // 4x4 transformation matrix
	bool is_active;
	int entry_sprite_id;
	int target_sprite_id;
} portal_transform_t;

#define PORT_WALL 2
#define PORT_FLOR 1
#define PORT_CEIL 0
#define PORT_SPRI 3

typedef struct {
	// calculate only diff between sprites. ideally forward facing along normal.
	// target portal idx
	uint16_t destpn; // sprite in sector that defines portal transform - use position and forward vector for now.
	uint16_t surfid; // either wall, floor sprite etc. depends on kind;
	uint16_t anchorspri;
	uint16_t sect;
	uint16_t id;
	// 0 = ceil, 1= floor, 2 = wall, 3 = sprite itself.
	uint8_t kind;
} portal;


extern int portaln;
extern portal portals[100];


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
double ptpolydist2 (dpoint3d *pt, dpoint3d *pol, int n, dpoint3d *closest);

double roundcylminpath2 (double a0x, double a0y, double a1x, double a1y,
                         double b0x, double b0y, double b1x, double b1y);
long wallclippol (kgln_t *pol, kgln_t *npol);
int dupwall_imp (sect_t *s, int w);
long sect_isneighs_imp (int s0, int s1, mapstate_t* map);
double getslopez (sect_t *s, int i, double x, double y);
double getslopezpt(sect_t *s, int isflor, point2d pos);
double getwallz (sect_t *s, int isflor, int wid);
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

// returns next wall in the loop, not wall of the nextsec
static inline wall_t walnext(sect_t *sec, int wid) {
	return sec->wall[
		sec->wall[wid].n+wid
		];
}

float getzoftez(int tezflags, sect_t *mysec, int thiswall, point2d worldxy, mapstate_t *map);

void makewaluvs(sect_t *sect, int wid, mapstate_t *map);

void makesecuvs(sect_t *sect, mapstate_t *map);

static double distpoint2line2 (double x, double y, double x0, double y0, double x1, double y1)
{
	double f, g, dx, dy, nx, ny;
	dx = x1-x0; dy = y1-y0; nx = x-x0; ny = y-y0;
	f = nx*dx + ny*dy; if (f <=0.f) return(nx*nx + ny*ny); //behind point 0?
	g = dx*dx + dy*dy; if (f >=  g) return((x-x1)*(x-x1) + (y-y1)*(y-y1)); //behind point 1?
	f = nx*dy - ny*dx; return(f*f/g); //perpendicular distance to line
}

int polyspli (wall_t *owal, int on, wall_t **retwal, double kx, double ky, double ka);

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


int polbool_splitlinepoint (polbool_lin_t **lin, int *linmal, wall_t *wal, int n, wall_t *owal, int on);

	//   Collinear line priority:
	//1st sector (wal0): POLYBOOL_AND, POLYBOOL_SUB, POLYBOOL_OR
	//2nd sector (wal1): POLYBOOL_SUB2
	//if retwal is null, returns # walls without generating wall list
int polybool (wall_t *wal0, int on0, wall_t *wal1, int on1, wall_t **retwal, int *retn, int op);

int insidesect (double x, double y, wall_t *wal, int w);

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
static void getcentroid (wall_t *wal, int n, float *retcx, float *retcy);

static float getarea (wall_t *wal, int n);

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