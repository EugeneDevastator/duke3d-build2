#include "monoclip.h"
#include "shadowtest2.h"

#include <stdbool.h>

#include "scenerender.h"

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#define PI 3.14159265358979323
#pragma warning(disable:4731)

#define USESSE2 0
#define USENEWLIGHT 1 //FIXFIXFIX
#define USEGAMMAHACK 1 //FIXFIXFIX
int renderinterp = 1;
int compact2d = 0;
bool captureframe = false;
transform lastcamtr = {};
transform lastcamtr2 = {};
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
void logstep(const char* fmt, ...) {
	if (!captureframe)
		return;
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	printf("\n");
}


// World point to camera space. Valid.
static void world_to_cam(double wx, double wy, double wz,
						 cam_t *ctin,
						 double *cx, double *cy, double *cz) {
	double dx = wx - ctin->p.x;
	double dy = wy - ctin->p.y;
	double dz = wz - ctin->p.z;

	*cx = dx * ctin->r. x + dy * ctin->r.y + dz * ctin->r.z;
	*cy = dx * ctin->d.x + dy * ctin->d.y + dz * ctin->d.z;
	*cz = dx * ctin->f.x + dy * ctin->f.y + dz * ctin->f.z;
}
// Validated - works correctly.
static void wccw_transform(dpoint3d *pinout, cam_t *ctin, cam_t *ctout) {
	// World -> camera space (using ctin)
	double dx = pinout->x - ctin->p.x;
	double dy = pinout->y - ctin->p.y;
	double dz = pinout->z - ctin->p.z;

	double cx = dx * ctin->r. x + dy * ctin->r.y + dz * ctin->r.z;
	double cy = dx * ctin->d.x + dy * ctin->d.y + dz * ctin->d.z;
	double cz = dx * ctin->f.x + dy * ctin->f.y + dz * ctin->f.z;

	// Camera space -> world (using ctout)
	pinout->x = cx * ctout->r.x + cy * ctout->d.x + cz * ctout->f.x + ctout->p.x;
	pinout->y = cx * ctout->r.y + cy * ctout->d.y + cz * ctout->f.y + ctout->p.y;
	pinout->z = cx * ctout->r.z + cy * ctout->d.z + cz * ctout->f.z + ctout->p.z;
}

// Convert point from mp (mono polygon) space to world coordinates
// mp.x, mp.y are in half-plane rotated space; mp.z is NOT used (depth from gouvmat)
// REPLACE the existing mp_to_world with this simpler version:

// Convert point from mp (mono polygon) space to world coordinates
// After drawpol_befclip changes, mp.x/mp.y ARE screen coordinates
static void mp_to_world(double sx, double sy, bunchgrp *b,
						double *wx, double *wy, double *wz,
						cam_t *cam) {
	double denom = (b->gouvmat[0] * sx + b->gouvmat[3] * sy + b->gouvmat[6]) * cam->h.z;

	if (fabs(denom) < 1e-10) {
		*wx = cam->p.x;
		*wy = cam->p.y;
		*wz = cam->p.z;
		return;
	}

	double depth = 1.0 / denom;
	double dx = sx - cam->h.x;
	double dy = sy - cam->h.y;

	*wx = (dx * cam->r.x + dy * cam->d.x + cam->h.z * cam->f.x) * depth + cam->p.x;
	*wy = (dx * cam->r.y + dy * cam->d.y + cam->h.z * cam->f.y) * depth + cam->p.y;
	*wz = (dx * cam->r.z + dy * cam->d.z + cam->h.z * cam->f.z) * depth + cam->p.z;
}

//--------------------------------------------------------------------------------------------------
static tiletype gdd;
int shadowtest2_rendmode = 1;
extern int drawpoly_numcpu;
int shadowtest2_updatelighting = 1;

	//Sorting

unsigned int *shadowtest2_sectgot = 0; //WARNING:code uses x86-32 bit shift trick!
static unsigned int *shadowtest2_sectgotmal = 0;
static int shadowtest2_sectgotn = 0;
#define CLIP_PORTAL_FLAG 8
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

static int prepbunch(int id, bunchverts_t *twal, bunchgrp *b) {
	cam_t gcam = b->cam;
	wall_t *wal;
	double f, x, y, x0, y0, x1, y1;
	int i, n;

	wal = curMap->sect[b->bunch[id].sec].wall;
	i = b->bunch[id].wal0;
	twal[0].i = i;
	x0 = wal[i].x;
	y0 = wal[i].y;
	i += wal[i].n;
	x1 = wal[i].x;
	y1 = wal[i].y;
	f = b->bunch[id].fra0;
	twal[0].x = (x1 - x0) * f + x0;
	twal[0].y = (y1 - y0) * f + y0;
	if ((b->bunch[id].wal0 == b->bunch[id].wal1) && (b->bunch[id].fra0 < b->bunch[id].fra1)) {
		//Hack for left side clip
		f = b->bunch[id].fra1;
		twal[1].x = (x1 - x0) * f + x0;
		twal[1].y = (y1 - y0) * f + y0;
		return (1);
	}
	twal[1].x = x1;
	twal[1].y = y1;
	n = 1;
	while (i != b->bunch[id].wal1) {
		twal[n].i = i;
		n++;
		i += wal[i].n;
		twal[n].x = wal[i].x;
		twal[n].y = wal[i].y;
	}
	if (b->bunch[id].fra1 > 0.0) {
		x = wal[i].x;
		y = wal[i].y;
		f = b->bunch[id].fra1;
		twal[n].i = i;
		n++;
		i += wal[i].n;
		twal[n].x = (wal[i].x - x) * f + x;
		twal[n].y = (wal[i].y - y) * f + y;
	}
	return (n);
}


#define BUNCHNEAR 1e-7
static void scansector(int sectnum, bunchgrp *b)
{
    cam_t *gcam = &b->cam;
    sect_t *sec;
    wall_t *wal;
    double f, dx0, dy0, dx1, dy1, f0, f1;
    int i, j, k, ie, obunchn;

    if (sectnum < 0) return;
    b->sectgot[sectnum >> 5] |= (1 << sectnum);

    sec = &curMap->sect[sectnum];
    wal = sec->wall;

    // Use camera forward XY components for near-plane test
    // (walls are vertical, so only XY matters for front/back test)
    double fwdx = gcam->f.x;
    double fwdy = gcam->f.y;
    double fwdlen = sqrt(fwdx * fwdx + fwdy * fwdy);
    if (fwdlen > 1e-10) {
        fwdx /= fwdlen;
        fwdy /= fwdlen;
    } else {
        // Camera looking straight up/down - use right vector perpendicular
        fwdx = gcam->r.y;
        fwdy = -gcam->r.x;
    }

    obunchn = b->bunchn;
    for (i = 0, ie = sec->n; i < ie; i++) {
        j = wal[i].n + i;
        dx0 = wal[i].x - gcam->p.x;
        dy0 = wal[i].y - gcam->p.y;
        dx1 = wal[j].x - gcam->p.x;
        dy1 = wal[j].y - gcam->p.y;

        // Back-face cull (unchanged - this is correct)
        if (dy1 * dx0 <= dx1 * dy0) goto docont;

        // Near plane clip using camera forward direction
        f0 = dx0 * fwdx + dy0 * fwdy;
        f1 = dx1 * fwdx + dy1 * fwdy;

        if (f0 <= BUNCHNEAR) {
            if (f1 <= BUNCHNEAR) goto docont;
            f0 = (BUNCHNEAR - f0) / (f1 - f0);
            f1 = 1.0;
            if (f0 >= f1) goto docont;
        } else if (f1 <= BUNCHNEAR) {
            f1 = (BUNCHNEAR - f0) / (f1 - f0);
            f0 = 0.0;
            if (f0 >= f1) goto docont;
        } else {
            f0 = 0.0;
            f1 = 1.0;
        }

        // Add to bunch list (unchanged)
        k = b->bunch[b->bunchn - 1].wal1;
        if ((b->bunchn > obunchn) && (wal[k].n + k == i) && (b->bunch[b->bunchn - 1].fra1 == 1.0)) {
            b->bunch[b->bunchn - 1].wal1 = i;
            b->bunch[b->bunchn - 1].fra1 = f1;
        } else {
            if (b->bunchn >= b->bunchmal) {
                b->bunchmal <<= 1;
                b->bunch = (bunch_t *)realloc(b->bunch, b->bunchmal * sizeof(b->bunch[0]));
            }
            b->bunch[b->bunchn].wal0 = i;
            b->bunch[b->bunchn].fra0 = f0;
            b->bunch[b->bunchn].wal1 = i;
            b->bunch[b->bunchn].fra1 = f1;
            b->bunch[b->bunchn].sec = sectnum;
            b->bunchn++;
        }
    docont:;
        if (j < i) obunchn = b->bunchn;
    }
}

static void xformbac(double rx, double ry, double rz, dpoint3d *o, bunchgrp *b)
{
	cam_t *cam = &b->orcam;
	// Camera space to world direction (transpose of rotation)
	o->x = rx * cam->r.x + ry * cam->d.x + rz * cam->f.x;
	o->y = rx * cam->r.y + ry * cam->d.y + rz * cam->f.y;
	o->z = rx * cam->r.z + ry * cam->d.z + rz * cam->f.z;
}

eyepol_t *eyepol = 0; // 4096 eyepol_t's = 192KB
point3d *eyepolv = 0; //16384 point2d's  = 128KB
int eyepoln = 0, glignum = 0;
int eyepolmal = 0, eyepolvn = 0, eyepolvmal = 0;

static void drawtagfunc_ws(int rethead0, int rethead1, bunchgrp *b)
{
	cam_t *usecam = &b->orcam;  // Changed from cam_transform_t to cam_t*
	int i, h, rethead[2];

	if ((rethead0|rethead1) < 0) { mono_deloop(rethead1); mono_deloop(rethead0); return; }
	rethead[0] = rethead0; rethead[1] = rethead1;

	for(h=0; h<2; h++)
	{
		i = rethead[h];
		do
		{
			if (h) i = mp[i].p;

			if (eyepolvn >= eyepolvmal)
			{
				eyepolvmal = max(eyepolvmal<<1, 16384);
				eyepolv = (point3d *)realloc(eyepolv, eyepolvmal*sizeof(point3d));
			}

			double wx, wy, wz;
			mp_to_world(mp[i].x, mp[i].y, b, &wx, &wy, &wz, usecam);

			eyepolv[eyepolvn].x = (float)wx;
			eyepolv[eyepolvn].y = (float)wy;
			eyepolv[eyepolvn].z = (float)wz;
			eyepolvn++;

			if (!h) i = mp[i].n;
		} while (i != rethead[h]);
		mono_deloop(rethead[h]);
	}

	// Rest unchanged...
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
	eyepol[eyepoln].b2sect = b->gligsect;
	eyepol[eyepoln].b2wall = b->gligwall;
	eyepol[eyepoln].b2slab = b->gligslab;
	memcpy((void *)&eyepol[eyepoln].norm, (void *)&b->gnorm, sizeof(b->gnorm));
	eyepoln++;
	eyepol[eyepoln].vert0 = eyepolvn;
	eyepol[eyepoln].rdepth = b->recursion_depth;
	logstep("produce eyepol, depth:%d",b->recursion_depth);
}

/*
	Purpose: Renders skybox faces as background
	Generates 6 cube faces for skybox rendering
	Clips each face against view frustum
	Projects cube vertices to screen space
	Calls drawtagfunc for each visible skybox face
	Uses special texture mapping flags (b->gflags = 10-15 for different cube faces)
*/

static void skytagfunc (int rethead0, int rethead1, bunchgrp* b){}
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
static void ligpoltagfunc(int rethead0, int rethead1, bunchgrp *b)
{
    cam_t *gcam = &b->cam;
    float f;
    int i, j, rethead[2];

    if ((rethead0 | rethead1) < 0) { mono_deloop(rethead1); mono_deloop(rethead0); return; }

    rethead[0] = rethead0;
    rethead[1] = rethead1;

    for (j = 0; j < 2; j++)
    {
        i = rethead[j];
        do
        {
            if (j) i = mp[i].p;

            if (glp->ligpolvn >= glp->ligpolvmal)
            {
                glp->ligpolvmal = max(glp->ligpolvmal << 1, 1024);
                glp->ligpolv = (point3d *)realloc(glp->ligpolv, glp->ligpolvmal * sizeof(point3d));
            }

            // mp[i].x, mp[i].y are screen coordinates
            // Use same unproject logic as mp_to_world
            double sx = mp[i].x;
            double sy = mp[i].y;

            double denom = (b->gouvmat[0] * sx + b->gouvmat[3] * sy + b->gouvmat[6]) * gcam->h.z;
            if (fabs(denom) < 1e-10) denom = 1e-10;
            f = 1.0f / denom;

            double dx = sx - gcam->h.x;
            double dy = sy - gcam->h.y;

            glp->ligpolv[glp->ligpolvn].x = (dx * gcam->r.x + dy * gcam->d.x + gcam->h.z * gcam->f.x) * f + gcam->p.x;
            glp->ligpolv[glp->ligpolvn].y = (dx * gcam->r.y + dy * gcam->d.y + gcam->h.z * gcam->f.y) * f + gcam->p.y;
            glp->ligpolv[glp->ligpolvn].z = (dx * gcam->r.z + dy * gcam->d.z + gcam->h.z * gcam->f.z) * f + gcam->p.z;

            glp->ligpolvn++;
            if (!j) i = mp[i].n;
        } while (i != rethead[j]);
        mono_deloop(rethead[j]);
    }

    // Rest unchanged...
    if (glp->ligpoln + 1 >= glp->ligpolmal)
    {
        glp->ligpolmal = max(glp->ligpolmal << 1, 256);
        glp->ligpol = (ligpol_t *)realloc(glp->ligpol, glp->ligpolmal * sizeof(ligpol_t));
        glp->ligpol[0].vert0 = 0;
    }
    glp->ligpol[glp->ligpoln].b2sect = b->gligsect;
    glp->ligpol[glp->ligpoln].b2wall = b->gligwall;
    glp->ligpol[glp->ligpoln].b2slab = b->gligslab;
    i = lighash(b->gligsect, b->gligwall, b->gligslab);
    glp->ligpol[glp->ligpoln].b2hashn = glp->lighashead[i];
    glp->lighashead[i] = glp->ligpoln;
    ligpolmaxvert = max(ligpolmaxvert, glp->ligpolvn - glp->ligpol[glp->ligpoln].vert0);
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
	int mapsect = b->gnewsec;
	if ((b->needsecscan)
		&& (!(b->sectgot[mapsect>>5]&(1<<mapsect))))
		scansector(mapsect,b);

	mph_check(mphnum);
	mph[mphnum].head[0] = rethead0;
	mph[mphnum].head[1] = rethead1;
	mph[mphnum].tag = b->gnewtag;
	mphnum++;
	logstep("changetag: doscan?:%d, newMtag:%d, new mphnum:%d",b->needsecscan,b->gnewtag,mphnum);
}
	//flags&1: do and
	//flags&2: do sub
	//flags&4: reverse cut for sub
// Replace the existing drawpol_befclip with this version
// Full 3D camera transform - handles pitch/roll correctly for portals

static void drawpol_befclip(int tag1, int newtag1, int sec, int newsec,
                            int plothead0, int plothead1, int flags, bunchgrp *b)
{
    int mtag = tag1;
    int tagsect = sec;
    int mnewtag = newtag1;
    b->gnewsec = newsec;
    cam_t *orcam = &b->orcam;

    #define BSCISDIST 0.000001
    void (*mono_output)(int h0, int h1, bunchgrp *b);
    dpoint3d *otp, *tp;
    double f;
    int i, j, k, l, h, on, n, plothead[2], omph0, omph1, i0, i1;

    if ((plothead0 | plothead1) < 0) return;
    plothead[0] = plothead0;
    plothead[1] = plothead1;

    // Count vertices
    n = 2;
    for (h = 0; h < 2; h++)
        for (i = mp[plothead[h]].n; i != plothead[h]; i = mp[i].n)
            n++;

    otp = (dpoint3d *)_alloca(n * sizeof(dpoint3d));
    tp = (dpoint3d *)_alloca(n * sizeof(dpoint3d) * 2);

    // Transform world coordinates to camera space using FULL 3D rotation
    // mp[] contains world-space coordinates (after wccw_transform in callers)
    on = 0;
    for (h = 0; h < 2; h++) {
        i = plothead[h];
        do {
            if (h) i = mp[i].p;

            // World position from mono polygon
            double wx = mp[i].x;
            double wy = mp[i].y;
            double wz = mp[i].z;

            // Vector from camera to point
            double dx = wx - orcam->p.x;
            double dy = wy - orcam->p.y;
            double dz = wz - orcam->p.z;

            // Full 3D rotation to camera space
            // r = right vector (screen X axis)
            // d = down vector (screen Y axis)
            // f = forward vector (depth axis)
            otp[on].x = dx * orcam->r.x + dy * orcam->r.y + dz * orcam->r.z;
            otp[on].y = dx * orcam->d.x + dy * orcam->d.y + dz * orcam->d.z;
            otp[on].z = dx * orcam->f.x + dy * orcam->f.y + dz * orcam->f.z;
            on++;

            if (!h) i = mp[i].n;
        } while (i != plothead[h]);
        mono_deloop(plothead[h]);
    }

    // Clip against near plane (perpendicular to camera forward vector)
    // This is now correct for any camera orientation including pitch/roll
    n = 0;
    for (i = on - 1, j = 0; j < on; i = j, j++) {
        if (otp[i].z >= BSCISDIST) {
            tp[n] = otp[i];
            n++;
        }
        if ((otp[i].z >= BSCISDIST) != (otp[j].z >= BSCISDIST)) {
            f = (BSCISDIST - otp[j].z) / (otp[i].z - otp[j].z);
            tp[n].x = (otp[i].x - otp[j].x) * f + otp[j].x;
            tp[n].y = (otp[i].y - otp[j].y) * f + otp[j].y;
            tp[n].z = BSCISDIST;
            n++;
        }
    }
    if (n < 3) return;

    // Project from camera space to screen space
    for (i = 0; i < n; i++) {
        f = orcam->h.z / tp[i].z;
        tp[i].x = tp[i].x * f + orcam->h.x;
        tp[i].y = tp[i].y * f + orcam->h.y;
    }

    // Generate mono polygon from projected vertices
    mono_genfromloop(&plothead[0], &plothead[1], tp, n);
    if ((plothead[0] | plothead[1]) < 0) {
        mono_deloop(plothead[0]);
        mono_deloop(plothead[1]);
        return;
    }

    // === AND operation (flags & 1) ===
    if (flags & 1) {
        if (mnewtag >= 0) {
            b->gnewsec = newsec;
            b->gnewtag = mnewtag;
            omph0 = mphnum;
            b->needsecscan = !(flags & CLIP_PORTAL_FLAG);

            for (i = mphnum - 1; i >= 0; i--)
                if (mph[i].tag == mtag)
                    mono_bool(mph[i].head[0], mph[i].head[1],
                              plothead[0], plothead[1],
                              MONO_BOOL_AND, b, changetagfunc);

            // Join adjacent monos with same tag
            for (l = omph0; l < mphnum; l++) {
                mph[omph0] = mph[l];
                k = omph0;
                omph0++;
                for (j = omph0 - 1; j >= 0; j--) {
                    if (mph[j].tag != b->gnewtag) continue;
                    if (!mono_join(mph[j].head[0], mph[j].head[1],
                                   mph[k].head[0], mph[k].head[1], &i0, &i1)) continue;
                    for (i = 1; i >= 0; i--) {
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
        } else {
            // newtag == -1: final output (solid surface)
            if (shadowtest2_rendmode == 4)
                mono_output = ligpoltagfunc;
            else
                mono_output = drawtagfunc_ws;

            for (i = mphnum - 1; i >= 0; i--)
                if (mph[i].tag == mtag)
                    mono_bool(mph[i].head[0], mph[i].head[1],
                              plothead[0], plothead[1],
                              MONO_BOOL_AND, b, mono_output);
        }
    }

    // === SUB operation (flags & 2) ===
    if (flags & 2) {
        j = (flags & 4) ? MONO_BOOL_SUB_REV : MONO_BOOL_SUB;

        b->gnewtag = mtag;
        b->gnewsec = tagsect;
        b->needsecscan = 0;
        omph0 = mphnum;
        omph1 = mphnum;

        for (i = mphnum - 1; i >= 0; i--) {
            if (mph[i].tag != mtag) continue;
            mono_bool(mph[i].head[0], mph[i].head[1],
                      plothead[0], plothead[1], j, b, changetagfunc);
            mono_deloop(mph[i].head[1]);
            mono_deloop(mph[i].head[0]);
            omph0--;
            mph[i] = mph[omph0];
        }

        // Join new entries
        for (l = omph1; l < mphnum; l++) {
            mph[omph0] = mph[l];
            k = omph0;
            omph0++;
            for (j = omph0 - 1; j >= 0; j--) {
                if (mph[j].tag != b->gnewtag) continue;
                if (!mono_join(mph[j].head[0], mph[j].head[1],
                               mph[k].head[0], mph[k].head[1], &i0, &i1)) continue;
                for (i = 1; i >= 0; i--) {
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
// create plane EQ using GCAM
static void gentransform_ceilflor(sect_t *sec, wall_t *wal, int isflor, bunchgrp *b)
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
static void gentransform_wall (kgln_t *npol2, surf_t *sur, bunchgrp *b) {
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
}

/*
the mono engine produces camera-space polygons that are clipped to not overlap.
The plothead[0] and plothead[1] contain monotone polygon pairs representing
the final visible geometry ready for 2D projection.
The b parameter is a bunch index - this function processes one "bunch" (visible sector group) at a time. The traversal logic is in the caller that:
*/// Transform world vertex through portal using wccw_transform
// For infinity Z: transform XY at reference plane, preserve Z sign/magnitude
static void portal_xform_world_at_z(double *x, double *y, double ref_z, bunchgrp *b) {
	dpoint3d p;
	p.x = *x;
	p.y = *y;
	p.z = ref_z;
	wccw_transform(&p, &b->cam, &b->orcam);
	*x = p.x;
	*y = p.y;
}

static void portal_xform_world_full(double *x, double *y, double *z, bunchgrp *b) {
	dpoint3d p;
	p.x = *x;
	p.y = *y;
	p.z = *z;
	wccw_transform(&p, &b->cam, &b->orcam);
	*x = p.x;
	*y = p.y;
	*z = p.z;
}
static void drawalls (int bid, mapstate_t* map, bunchgrp* b)
{
	int portal_draw = 3|8;
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

	twal = (bunchverts_t *)_alloca((curMap->sect[b->bunch[bid].sec].n+1)*sizeof(bunchverts_t));
	twaln = prepbunch(bid,twal,b);
	b->gligsect = s; b->gligslab = 0;

	// === BUNCH MANAGEMENT: DELETE CURRENT BUNCH FROM GRID ===
	b->bunchn--;
	bool noportals = b->recursion_depth >= MAX_PORTAL_DEPTH;
	// === DRAW CEILINGS & FLOORS ===
	for(isflor=0;isflor<2;isflor++) // floor ceil
	{
		// here, when we draw sector of the exit portal we get glitches when it would draw a triangle with point below the camera resulting in triangle spanning entire vertical of the screen

		     // Back-face culling: skip if camera is on wrong side of surface
			// need to get original slope, as if camera was in origin.
	//if (!b->has_portal_clip)
		bool skipport = (b->has_portal_clip
				&& s == b->testignoresec
				&& isflor == b->testignorewall);

		// Back-face cull, but NOT for exit portal surface (we need to draw it solid)
		float surfpos = getslopez(&sec[s], isflor, b->cam.p.x, b->cam.p.y);
		if (!skipport && (b->cam.p.z >= surfpos) == isflor)
			continue;
		gentransform_ceilflor(&sec[s], wal, isflor, b);

		// Setup surface properties (height, gradient, color)
		fz = sec[s].z[isflor]; grad = &sec[s].grad[isflor];

		// Calculate surface normal vector
		b->gnorm.x = grad->x;
		b->gnorm.y = grad->y;
		b->gnorm.z = 1.f;
		if (isflor) {
			b->gnorm.x = -b->gnorm.x;
			b->gnorm.y = -b->gnorm.y;
			b->gnorm.z = -b->gnorm.z;
		}
		f = 1.0 / sqrt(b->gnorm.x * b->gnorm.x + b->gnorm.y * b->gnorm.y + 1);
		b->gnorm.x *= f;
		b->gnorm.y *= f;
		b->gnorm.z *= f;

			//plane point: (wal[0].x,wal[0].y,fz)
			//plane norm: <grad->x,grad->y,1>
			//
			//   (wal[i].x-wal[0].x)*grad->x +
			//   (wal[i].y-wal[0].y)*grad->y +
			//   (?       -      fz)*      1 = 0
		// Build polygon for ceiling/floor using plane equation:
		plothead[0] = -1; plothead[1] = -1;
		// claude, look at that section, explain what is going into mono_ins, in what coordinate system, and how can i modify it
		// with wccw transform to transform world around. think - how would i draw rotated world, if camera was stationary.
		// First loop - infinity points
		for (ww = twaln; ww >= 0; ww -= twaln) {
			double xw = twal[ww].x;
			double yw = twal[ww].y;
			double zw = b->gnorm.z * -1e32;  // keep this as output Z

			// Calculate surface Z at this XY for consistent transform
			double surface_z = (wal[0].x - xw) * grad->x + (wal[0].y - yw) * grad->y + fz;
			portal_xform_world_at_z(&xw, &yw, surface_z, b);

			plothead[isflor] = mono_ins(plothead[isflor], xw, yw, zw);
		}

		// Second loop - real surface points
		i = isflor ^ 1;
		for (ww = 0; ww <= twaln; ww++) {
			double xw = twal[ww].x;
			double yw = twal[ww].y;
			double zw = (wal[0].x - xw) * grad->x + (wal[0].y - yw) * grad->y + fz;

			portal_xform_world_full(&xw, &yw, &zw, b);

			plothead[i] = mono_ins(plothead[i], xw, yw, zw);
		}
		plothead[i] = mp[plothead[i]].n;

		// Setup texture and rendering flags
		sur = &sec[s].surf[isflor];
		gtpic = &gtile[sur->tilnum];
		//if (!gtpic->tt.f) loadpic(gtpic);
		if (sec[s].surf[isflor].flags & (1 << 17)) { b->gflags = 2; } //skybox ceil/flor

		b->gligwall = isflor - 2;

		int myport = sec[s].tags[1];
		if (myport>=0)
			int asd=1;

		bool isportal = myport>=0 && portals[myport].destpn >=0 && portals[myport].surfid == isflor;
		if (isportal && !noportals && !skipport) {
			int endpn = portals[myport].destpn;
			int endsec = portals[myport].sect;
			// Only SUB (flags&2), NOT AND (flags&1) - child handles the content
			// This prevents parent creating mph entries that collide with child's tags

			logstep("entering portal floor?, %d, sec:%d, cur depth:%d", isflor, s, b->recursion_depth);

			int ci = taginc *b->recursion_depth;
			drawpol_befclip(s+ci,portals[endpn].sect+ci+taginc, s,portals[endpn].sect ,plothead[0],plothead[1],   ((isflor<<2)+3)|CLIP_PORTAL_FLAG, b);
			draw_hsr_enter_portal(map, myport, b, plothead[0], plothead[1]);
		}

		else {
			logstep("drawing solid - floor?, %d, sec:%d, cur depth:%d", isflor, s, b->recursion_depth);

			drawpol_befclip(s+taginc*b->recursion_depth,-1,s,-1,plothead[0],plothead[1],(isflor<<2)+3,b);
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
			f = 1e-7;
			dpoint3d trap1[4] = {{pol[0].x, pol[0].y, pol[0].z-f}, {pol[1].x, pol[1].y, pol[1].z-f},
								{pol[2].x, pol[2].y, pol[2].z+f}, {pol[3].x, pol[3].y, pol[3].z+f}};
			dpoint3d trap2[4] = {{pol[0].x, pol[0].y, opolz[0]-f}, {pol[1].x, pol[1].y, opolz[1]-f},
								{pol[2].x, pol[2].y, opolz[2]+f}, {pol[3].x, pol[3].y, opolz[3]+f}};
			for (int i=0;i<4;i++) {
				wccw_transform(&trap1[i],&b->cam,&b->orcam);
				wccw_transform(&trap2[i],&b->cam,&b->orcam);
			}

			// Use copies to preserve original pol[] for subsequent loop iterations
			dpoint3d pol0_xf = pol[0];
			dpoint3d pol1_xf = pol[1];
			wccw_transform(&pol1_xf, &b->cam,&b->orcam);
			wccw_transform(&pol0_xf, &b->cam,&b->orcam);

			if (!intersect_traps_mono_points(pol0_xf, pol1_xf, trap1, trap2, &plothead[0], &plothead[1]))
				continue;

			// Render wall segment if visible
			//	gtpic = &gtile[sur->tilnum]; if (!gtpic->tt.f) loadpic(gtpic);
			//if (sur->flags & (1 << 17))
			//{ b->gflags = 2; } //skybox ceil/flor
			//else
			if ((!(m & 1)) || (wal[w].surf.flags & (1 << 5))) //Draw wall here //(1<<5): 1-way
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
					gentransform_wall(npol2, sur, b);

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
			bool skipport= (b->has_portal_clip
				&& s==b->testignoresec
				&& w == b->testignorewall);

			if (!skipport && !noportals && myport >= 0 && portals[myport].destpn >= 0 && portals[myport].kind == PORT_WALL) {
				int endpn = portals[myport].destpn;
				logstep("entering wall port,wal:%d, sec:%d, cur depth:%d, curhalf:%d",w, s, b->recursion_depth,0);
				// Only SUB, not AND
				int drawflags = 2; // 2 is sub;
				int ci = taginc*b->recursion_depth;
				drawpol_befclip(s+ci,portals[endpn].sect+ci+taginc, s,portals[endpn].sect ,plothead[0],plothead[1],   (((m > vn) << 2) + 3)|CLIP_PORTAL_FLAG, b);
				//drawpol_befclip(s, -1, -1, -1,plothead[0],plothead[1],  drawflags, b);

				draw_hsr_enter_portal(map, myport, b, plothead[0], plothead[1]);
			} else {
				if (ns >= 0)
					logstep("drawing nextsecw - sec-wall:%d-%d, ns:%d  sec:%d, cur depth:%d, curhalf:%d",
							s, w, ns, s, b->recursion_depth, 0);
				else
					logstep("drawing solid - sec-wall:%d-%d, ns:%d  sec:%d, cur depth:%d, curhalf:%d",
							s, w, ns, s, b->recursion_depth, 0);

				int inc = taginc * b->recursion_depth;
				int newtag = (ns >= 0) ? (ns + inc) : -1;  // FIX: preserve -1 for solid walls
				drawpol_befclip(s + inc, newtag, s, ns, plothead[0], plothead[1], ((m > vn) << 2) + 3, b);
			}
		}

	}
}
/*
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
	logstep("frame start");
	draw_hsr_ctx(map,&bs);

}

void draw_hsr_ctx (mapstate_t *lgs, bunchgrp *newctx) {
	if (!newctx) {
		return;
		if (captureframe) printf("discarding due to null ctx");
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
	b->bunchn=0;
	b->bunchmal=0;

	//b->curportal=0;
	cam_t gcam = b->cam;
	cam_t oricam = b->orcam;
	logstep("entered hsr_ctx, halfplane:%d",0);
	wall_t *wal;
	spri_t *spr;
	dpoint3d dpos, drig, ddow, dfor;
	dpoint3d fp, bord[4], bord2[8];
	double f, d;
	unsigned int *uptr;
	int i, j, k, n, s, w, closest, col, didcut, halfplane;
	int loopsrun = 0;
	if (shadowtest2_rendmode == 4)
	{
		glp = &shadowtest2_light[glignum];
	//	if ((!(glp->flags&1)) || (!shadowtest2_useshadows)) return;
	}

	curMap = lgs;

	//if ((lgs->numsects <= 0) || ((unsigned)gcam.cursect >= (unsigned)lgs->numsects))
	//{
	////if (shadowtest2_rendmode != 4) eyepoln = 0; //Prevents drawpollig() from crashing
	////	return;
	//}
	if (!b->bunchmal)
	{
		b->bunchmal = 64;
		b->bunch     = (bunch_t       *)malloc(b->bunchmal*sizeof(b->bunch[0]));
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

{
    int halfplane = 0;

    logstep("HALFPLANE removed - single pass, depth: %d", b->recursion_depth);

		if (shadowtest2_rendmode == 4) {
			// gcam.r.x = 1; gcam.d.x = 0; gcam.f.x = 0;
			// gcam.r.y = 0; gcam.d.y = 0; gcam.f.y = -gcam.r.x;
			// gcam.r.z = 0; gcam.d.z = 1; gcam.f.z = 0;
			cam_t *cam = &b->orcam;
			xformbac(-65536.0, -65536.0, 1.0, &bord2[0], b);
			xformbac(+65536.0, -65536.0, 1.0, &bord2[1], b);
			xformbac(+65536.0, +65536.0, 1.0, &bord2[2], b);
			xformbac(-65536.0, +65536.0, 1.0, &bord2[3], b);
			n = 4;
			didcut = 1;
		}
		// In draw_hsr_ctx, replace the viewport setup:
		else if (!b->has_portal_clip) {
			// Screen-independent viewport - use huge bounds
			double huge = 1e7;
			bord2[0].x = -huge;
			bord2[0].y = -huge;
			bord2[1].x = +huge;
			bord2[1].y = -huge;
			bord2[2].x = +huge;
			bord2[2].y = +huge;
			bord2[3].x = -huge;
			bord2[3].y = +huge;
			n = 4;
			b->planecuts = n;
			didcut = 0;

			memset8(b->sectgot, 0, (lgs->numsects + 31) >> 3);

			for (i = mphnum - 1; i >= 0; i--) {
				mono_deloop(mph[i].head[1]);
				mono_deloop(mph[i].head[0]);
			}

			mono_genfromloop(&mph[0].head[0], &mph[0].head[1], bord2, n);
			mph[0].tag = gcam.cursect;
			mphnum = 1;
		} else {
			// Portal case
			n = b->planecuts;
			didcut = 1;
			memset8(b->sectgot, 0, (lgs->numsects + 31) >> 3);
		}

		b->bunchn = 0;
		scansector(gcam.cursect, b);

		while (b->bunchn) {
			closest = b->bunchn - 1;
			logstep("bunch draw close: %d, depth:%d", closest, b->recursion_depth);
			drawalls(closest, lgs, b);
		}

		if (shadowtest2_rendmode == 4)
			uptr = glp->sectgot;
		else
			uptr = shadowtest2_sectgot;

		memcpy(uptr, b->sectgot, (lgs->numsects + 31) >> 3);
	}
}

// 1. Normalize transform (makes vectors unit length and orthogonal)
static float vlen(point3d *p) {
    return sqrtf(p->x * p->x + p->y * p->y + p->z * p->z);
}

static void scalardivv(point3d *pt, float diver) {
    pt->x /= diver; pt->y /= diver; pt->z /= diver;
}

void normalize_transform(transform *tr) {
    // Normalize all axes
    float flen = vlen(&tr->f);
    if (flen > 0.0001f) scalardivv(&tr->f, flen);

    flen = vlen(&tr->r);
    if (flen > 0.0001f) scalardivv(&tr->r, flen);

    flen = vlen(&tr->d);
    if (flen > 0.0001f) scalardivv(&tr->d, flen);
}
static point3d world_to_local_vec(point3d world_vec, transform *tr) {
	point3d local;
	local.x = world_vec.x * tr->r.x + world_vec.y * tr->r.y + world_vec.z * tr->r.z;  // right
	local.y = world_vec.x * tr->f.x + world_vec.y * tr->f.y + world_vec.z * tr->f.z;  // forward
	local.z = world_vec.x * tr->d.x + world_vec.y * tr->d.y + world_vec.z * tr->d.z;  // down

	return local;
}

static point3d local_to_world_point(point3d local_pos, transform *tr) {
	point3d world;
	world.x = tr->p.x + local_pos.x * tr->r.x + local_pos.y * tr->f.x + local_pos.z * tr->d.x;
	world.y = tr->p.y + local_pos.x * tr->r.y + local_pos.y * tr->f.y + local_pos.z * tr->d.y;
	world.z = tr->p.z + local_pos.x * tr->r.z + local_pos.y * tr->f.z + local_pos.z * tr->d.z;

	return world;
}

static point3d world_to_local_point(point3d world_pos, transform *tr) {
	float dx = world_pos.x - tr->p.x;
	float dy = world_pos.y - tr->p.y;
	float dz = world_pos.z - tr->p.z;

	point3d local;
	local.x = dx * tr->r.x + dy * tr->r.y + dz * tr->r.z;
	local.y = dx * tr->f.x + dy * tr->f.y + dz * tr->f.z;
	local.z = dx * tr->d.x + dy * tr->d.y + dz * tr->d.z;

	return local;
}

static point3d local_to_world_vec(point3d local_vec, transform *tr) {
	point3d world;
	world.x = local_vec.x * tr->r.x + local_vec.y * tr->f.x + local_vec.z * tr->d.x;
	world.y = local_vec.x * tr->r.y + local_vec.y * tr->f.y + local_vec.z * tr->d.y;
	world.z = local_vec.x * tr->r.z + local_vec.y * tr->f.z + local_vec.z * tr->d.z;

    return world;
}
static void draw_hsr_enter_portal(mapstate_t* map, int myport, bunchgrp *parentctx,
                                   int plothead0, int plothead1)
{
// on entering portal we must rotate world verts, as if current camera was stationary.
	// hopefully ouvmat traqnsforms yield camera space coords.
	// we also should NOT modify xformmat beacuse of clipping planes shit.
	// clipping planes are ONLY 2 ever, for first cam.

    if (parentctx->recursion_depth >= MAX_PORTAL_DEPTH) {
		logstep("portal entrance aborted due to max depth.");
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

    // Normalize transforms to ensure orthonormality
    normalize_transform(&ent.tr);
    normalize_transform(&tgs.tr);
    normalize_transform(&ncam.tr);

	point3d cam_local_pos = world_to_local_point(ncam.p, &ent.tr);
	point3d cam_local_r = world_to_local_vec(ncam.r, &ent.tr);
	point3d cam_local_d = world_to_local_vec(ncam.d, &ent.tr);
	point3d cam_local_f = world_to_local_vec(ncam.f, &ent.tr);
	//point3d cam_local_h = world_to_local_point(ncam.h, &ent.tr);


    // Step 2: Apply that same relative transform from the target portal's perspective
    // Since entry.forward points IN and target.forward points OUT (already opposite),
    // we just transform directly without any flips
    ncam.p = local_to_world_point(cam_local_pos, &tgs.tr);
    ncam.r = local_to_world_vec(cam_local_r, &tgs.tr);
    ncam.d = local_to_world_vec(cam_local_d, &tgs.tr);
    ncam.f = local_to_world_vec(cam_local_f, &tgs.tr);

    ncam.cursect = portals[endp].sect;

    bunchgrp newctx = {};
	newctx.prevsec = portals[myport].sect;
	newctx.newsec = portals[endp].sect;
	newctx.recursion_depth = parentctx->recursion_depth + 1;
	//if (newctx.recursion_depth >0 )
	//	printf("ncam fw %f,%f,%f \r", ncam.f.x,ncam.f.y,ncam.f.z);
    newctx.cam = ncam;
    newctx.orcam = parentctx->orcam;
    newctx.has_portal_clip = true;
    newctx.portal_clip[0] = plothead0;
    newctx.portal_clip[1] = plothead1;
    newctx.sectgotn = 0;
    newctx.sectgot = 0;
    newctx.sectgotmal = 0;
    newctx.bunchn = 0;
    newctx.bunchmal = 0;
    newctx.testignorewall = ignw;
    newctx.testignoresec = igns;
    newctx.gnewsec = -1;
    newctx.gnewtag = -1;
    newctx.planecuts = parentctx->planecuts;
	// Copy parent's clipping matrices (for mono clipping)
	//point3d test = {1, 2, 3};  -- this is ok.
	//point3d w = local_to_world_vec(test, &ncam.tr);
	//point3d back = world_to_local_vec(w, &ncam.tr);
	//printf("test rt: %f %f %f",back.x,back.y,back.z);
	lastcamtr = newctx.orcam.tr;
	lastcamtr2 = newctx.cam.tr;

    draw_hsr_ctx(map, &newctx);
	logstep("Finished portal %d, mphnum:%d", myport, mphnum);
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
