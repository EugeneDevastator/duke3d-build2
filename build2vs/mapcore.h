//
// Created by omnis on 10/18/2025.
//
#include <math.h>
#include <windows.h>
#include <malloc.h>

#ifndef BUILD2_MAPCORE_H
#define BUILD2_MAPCORE_H
typedef struct { float x, y; } point2d;
#ifndef KEN_DRAWPOLY_H
typedef struct tiltyp {
	long f, p, x, y, z;
	float shsc;
	struct tiltyp *lowermip;
} tiltyp; //shsc=suggested height scale
typedef struct { float x, y, z; } point3d;
typedef struct { double x, y, z; } dpoint3d; 	//Note: pol doesn't support loops as dpoint3d's!

typedef struct { INT_PTR f; int p, x, y; } tiletype;
typedef struct { tiltyp c, z; point3d p, r, d, f, h; } cam_t;

#endif

typedef struct { int w, s; } vertlist_t;
typedef struct { float x, y, z, u, v; int n; } kgln_t;
typedef struct { double x, y, z; long n, filler; } genpoly_t;
typedef struct
{
    char filnam[240]; //Must fit packet header, sector&wall index, null terminator in 256 byte packet
    tiltyp tt; //union! if (!tt.p), it's a 3D model, tt.f points to raw model data, tt.x is type
    long namcrc32, hashnext;
} tile_t;

extern tile_t *gtile;
extern long gnumtiles, gmaltiles, gtilehashead[1024];
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
#endif //BUILD2_MAPCORE_H