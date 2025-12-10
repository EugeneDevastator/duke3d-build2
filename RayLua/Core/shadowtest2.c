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
static inline void portal_xform_world_full(double *x, double *y, double *z, bdrawctx *b) {
	dpoint3d p;
	p.x = *x;
	p.y = *y;
	p.z = *z;
	wccw_transform(&p, &b->cam, &b->orcam);
	loops[loopnum] = p;
	loopuse[loopnum] = true;
	loopnum++;
	*x = p.x;
	*y = p.y;
	*z = p.z;
}
static inline void portal_xform_world_fullp(dpoint3d *inp, bdrawctx *b) {
	wccw_transform(inp, &b->cam, &b->orcam);
	loops[loopnum] = *inp;
	loopuse[loopnum] = true;
	loopnum++;

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

eyepol_t *eyepol = 0; // 4096 eyepol_t's = 192KB
point3d *eyepolv = 0; //16384 point2d's  = 128KB
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

// Simple sort comparator
static int wall_dist_cmp(const void *a, const void *b) {
    float da = ((wall_job_t*)a)->dist;
    float db = ((wall_job_t*)b)->dist;
    return (da > db) - (da < db);
}

#define BUNCHNEAR 1e-7

static void xformbac(double rx, double ry, double rz, dpoint3d *o, bdrawctx *b)
{
	cam_t *cam = &b->orcam;
	// Camera space to world direction (transpose of rotation)
	o->x = rx * cam->r.x + ry * cam->d.x + rz * cam->f.x;
	o->y = rx * cam->r.y + ry * cam->d.y + rz * cam->f.y;
	o->z = rx * cam->r.z + ry * cam->d.z + rz * cam->f.z;
}

static void drawtagfunc_ws(int rethead0, int rethead1, bdrawctx *b)
{
	cam_t *usecam = &b->orcam;
	int i, h, rethead[2];

	if ((rethead0|rethead1) < 0) { mono_deloop(rethead1); mono_deloop(rethead0); return; }
	rethead[0] = rethead0; rethead[1] = rethead1;

	int start_vert = eyepolvn;
	int chain0_end = 0;  // Will mark where chain 0 ends

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

	// Allocate eyepol entry
	if (eyepoln+1 >= eyepolmal)
	{
		eyepolmal = max(eyepolmal<<1, 4096);
		eyepol = (eyepol_t *)realloc(eyepol, eyepolmal*sizeof(eyepol_t));
		eyepol[0].vert0 = 0;
	}

	memcpy((void *)eyepol[eyepoln].ouvmat, (void *)b->gouvmat, sizeof(b->gouvmat[0])*9);
	//eyepol[eyepoln].chain1_start = chain0_end;  // Store the split
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
}

static void skytagfunc (int rethead0, int rethead1, bdrawctx* b){}

/*
	Purpose: Generates shadow polygon lists for lighting
	Converts screen-space polygons back to 3D world coordinates
	Stores shadow-casting geometry in ligpol[] arrays per light source
	Used during shadow map generation phase (mode 4)
	Creates hash table for fast polygon lookup by sector/wall/slab
 */
static void ligpoltagfunc(int rethead0, int rethead1, bdrawctx *b)
{
	return;// - revert this
    cam_t *gcam = &b->cam;
    float f;
    int i, j, rethead[2];

    if ((rethead0 | rethead1) < 0) { mono_deloop(rethead1); mono_deloop(rethead0); return; }

    rethead[0] = rethead0;
    rethead[1] = rethead1;
	//mono_triangulate_strip(rethead0, rethead1, &glp->ligpol[glp->ligpoln].tri_strip);
	glp->ligpol[glp->ligpoln].has_triangulation = true;
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
static void changetagfunc (int rethead0, int rethead1, bdrawctx *b)
{
	if ((rethead0|rethead1) < 0) {
		mono_deloop(rethead0);
		mono_deloop(rethead1);
		return;
	}

	mono_mph_check(mphnum);
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

// Change return type from void to int
// Returns: 1 if AND produced visible output, 0 if not
static int drawpol_befclip(int tag1, int newtag1, int sec, int newsec,
                            int plothead0, int plothead1, int flags, bdrawctx *b)
{
    int mtag = tag1;
    int tagsect = sec;
    int mnewtag = newtag1;
    b->gnewsec = newsec;
    cam_t *orcam = &b->orcam;

    int produced_output = 0;  // NEW: track if we produced anything

    #define BSCISDIST 0.000001
    void (*mono_output)(int h0, int h1, bdrawctx *b);
    dpoint3d *otp, *tp;
    double f;
    int i, j, k, l, h, on, n, plothead[2], omph0, omph1, i0, i1;

    if ((plothead0 | plothead1) < 0) return 0;
    plothead[0] = plothead0;
    plothead[1] = plothead1;

    // Count vertices
    n = 2;
    for (h = 0; h < 2; h++)
        for (i = mp[plothead[h]].n; i != plothead[h]; i = mp[i].n)
            n++;

    otp = (dpoint3d *)_alloca(n * sizeof(dpoint3d));
    tp = (dpoint3d *)_alloca(n * sizeof(dpoint3d) * 2);

    // Transform world coordinates to camera space
    on = 0;
    for (h = 0; h < 2; h++) {
        i = plothead[h];
        do {
            if (h) i = mp[i].p;

            double wx = mp[i].x;
            double wy = mp[i].y;
            double wz = mp[i].z;

            double dx = wx - orcam->p.x;
            double dy = wy - orcam->p.y;
            double dz = wz - orcam->p.z;


            otp[on].x = dx * orcam->r.x + dy * orcam->r.y + dz * orcam->r.z;
            otp[on].y = dx * orcam->d.x + dy * orcam->d.y + dz * orcam->d.z;
            otp[on].z = dx * orcam->f.x + dy * orcam->f.y + dz * orcam->f.z;

            on++;

            if (!h) i = mp[i].n;
        } while (i != plothead[h]);
        mono_deloop(plothead[h]);
    }

    // Clip against near plane
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
    if (n < 3) return 0;

    // Project to screen space
    for (i = 0; i < n; i++) {
        f = orcam->h.z / tp[i].z;
        tp[i].x = tp[i].x * f + orcam->h.x;
        tp[i].y = tp[i].y * f + orcam->h.y;

    }

    mono_genfromloop(&plothead[0], &plothead[1], tp, n);
    if ((plothead[0] | plothead[1]) < 0) {
        mono_deloop(plothead[0]);
        mono_deloop(plothead[1]);
        return 0;
    }

    // === AND operation (flags & 1) ===
    if (flags & 1) {
        if (mnewtag >= 0) {
            b->gnewsec = newsec;
            b->gnewtag = mnewtag;
            omph0 = mphnum;
            b->needsecscan = !(flags & CLIP_PORTAL_FLAG);

            int before_mphnum = mphnum;
            for (i = mphnum - 1; i >= 0; i--)
                if (mph[i].tag == mtag)
                    mono_bool(mph[i].head[0], mph[i].head[1],
                              plothead[0], plothead[1],
                              MONO_BOOL_AND, b, changetagfunc);

            // NEW: Check if AND produced any new regions
            if (mphnum > before_mphnum)
                produced_output = 1;

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

            int before_eyepoln = eyepoln;
            for (i = mphnum - 1; i >= 0; i--)
                if (mph[i].tag == mtag)
                    mono_bool(mph[i].head[0], mph[i].head[1],
                              plothead[0], plothead[1],
                              MONO_BOOL_AND, b, mono_output);

            // NEW: Check if AND produced any eyepols
            if (eyepoln > before_eyepoln)
                produced_output = 1;
        }
    }

    // === SUB operation (flags & 2) - ONLY if AND produced output ===
    if ((flags & 2) && produced_output) {
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

    return 1;//produced_output;
}
// create plane EQ using GCAM
static void gentransform_ceilflor(sect_t *sec, wall_t *wal, int isflor, bdrawctx *b)
{
	cam_t *cam = &b->orcam;
	float gx = sec->grad[isflor].x;
	float gy = sec->grad[isflor].y;
	dpoint3d gvec={gx,gy,1};
	dpoint3d sp={wal[0].x,wal[0].y,sec->z[isflor]};
	wccw_transform_dir(&gvec,&b->cam,&b->orcam);
	wccw_transform(&sp,&b->cam,&b->orcam);

	// Transform plane normal (gx, gy, 1) to camera space
	//float nx = cam->r.x * gx + cam->r.y * gy + cam->r.z;
	//float ny = cam->d.x * gx + cam->d.y * gy + cam->d.z;
	//float nz = cam->f.x * gx + cam->f.y * gy + cam->f.z;
	dpoint3d npos = world_to_local_vecd(gvec,&b->orcam.tr);

	// Camera-space plane constant
	float D_c = gvec.x * (sp.x - cam->p.x)
			  + gvec.y * (sp.y - cam->p.y)
			  + gvec.z * (sp.z - cam->p.z);

	// Scale includes h.z for screen-space depth formula
	float scale = 1.0f / (D_c * cam->h.z);
	b->gouvmat[0] = npos.x * scale;
	b->gouvmat[3] = npos.y * scale;
	b->gouvmat[6] = npos.z / D_c - b->gouvmat[0] * cam->h.x - b->gouvmat[3] * cam->h.y;
}

// create plane EQ using GCAM
static void gentransform_wall (dpoint3d *npol2, int n, surf_t *sur, bdrawctx *b) {
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

// New function: draw sector floor/ceiling using FULL sector polygon
static void draw_sector_surface(int sectnum, int isflor, mapstate_t *map, bdrawctx *b)
{
    sect_t *sec = &map->sect[sectnum];
    wall_t *wal = sec->wall;
    point2d *grad = &sec->grad[isflor];
    double fz = sec->z[isflor];
    int plothead[2] = {-1, -1};
    int i, n = sec->n;
	loopuse[loopnum] = false;
	loopnum++;
    if (n < 3) return;  // Need at least 3 vertices
	int myport = sec->tags[1];
	bool noportals = b->recursion_depth >= MAX_PORTAL_DEPTH;
	bool isportal = myport >= 0
				&& portals[myport].destpn >= 0
				&& portals[myport].kind == isflor;
    bool skipport = shadowtest2_debug_block_selfportals
                    && b->has_portal_clip
                    && isportal
                    && sectnum == b->testignoresec
                    && portals[myport].kind == b->ignorekind
                    && isflor == b->testignorewall;
    if (skipport)
    	return;
     // FIXED: Check surfid
    // Back-face cull
    float surfpos = getslopez(sec, isflor, b->cam.p.x, b->cam.p.y);
		if ((b->cam.p.z >= surfpos) == isflor) return;

    // Setup transform and normal
    gentransform_ceilflor(sec, wal, isflor, b);

    b->gnorm.x = grad->x;
    b->gnorm.y = grad->y;
    b->gnorm.z = 1.f;
    if (isflor) {
        b->gnorm.x = -b->gnorm.x;
        b->gnorm.y = -b->gnorm.y;
        b->gnorm.z = -b->gnorm.z;
    }
    double f = 1.0 / sqrt(b->gnorm.x * b->gnorm.x + b->gnorm.y * b->gnorm.y + 1);
    b->gnorm.x *= f;
    b->gnorm.y *= f;
    b->gnorm.z *= f;

    b->gligsect = sectnum;
    b->gligwall = isflor - 2;
    b->gligslab = 0;

    // Build vertex array from ALL sector walls
    dpoint3d *verts = (dpoint3d *)_alloca(n * sizeof(dpoint3d));

    for (i = 0; i < n; i++) {
        double xw = wal[i].x;
        double yw = wal[i].y;
        double zw = (wal[0].x - xw) * grad->x + (wal[0].y - yw) * grad->y + fz;

        // Transform through portal if needed
        portal_xform_world_full(&xw, &yw, &zw, b);

        verts[i].x = xw;
        verts[i].y = yw;
        verts[i].z = zw;
    }

    // Use mono_genfromloop to create proper monotone polygon structure
    // It expects the vertices in the correct winding order
    // Floor: one order, Ceiling: reverse order
    if (!isflor) {
        // Ceiling - reverse the vertex order
        for (i = 0; i < n / 2; i++) {
            dpoint3d tmp = verts[i];
            verts[i] = verts[n - 1 - i];
            verts[n - 1 - i] = tmp;
        }
    }

    mono_genfromloop(&plothead[0], &plothead[1], verts, n);

    if ((plothead[0] | plothead[1]) < 0) {
        mono_deloop(plothead[0]);
        mono_deloop(plothead[1]);
        return;
    }

    // Check for portal



	int mytag = sectnum + taginc * b->recursion_depth;

    if (isportal && !noportals && !skipport) {
        int endpn = portals[myport].destpn;
        int nexttag = portals[endpn].sect + taginc * (b->recursion_depth + 1);

	    logstep("entering portal surf %d, sec:%d, depth:%d", isflor, sectnum, b->recursion_depth);

	    int visible = drawpol_befclip(mytag, nexttag,
	                                  sectnum, portals[endpn].sect,
	                                  plothead[0], plothead[1],
	                                  ((!isflor << 2) + 3) | CLIP_PORTAL_FLAG, b);
	    if (visible) {
		    draw_hsr_enter_portal(map, myport, b);
	    }
    	else {
    		mono_deloop(plothead[0]);
    		mono_deloop(plothead[1]);
    	}
    } else {
	    logstep("drawing solid surf %d, sec:%d, depth:%d", isflor, sectnum, b->recursion_depth);
	    bool visible = drawpol_befclip(mytag, -1,
	                    sectnum, -1,
	                    plothead[0], plothead[1],
	                    (isflor << 2) + 3, b);
    	if (!visible) {
    		mono_deloop(plothead[0]);
    		mono_deloop(plothead[1]);
    	}
    }
}
void shadowtest2_set_culling(int backface, int distance, int debug) {
	shadowtest2_backface_cull = backface;
	shadowtest2_distance_cull = distance;
	shadowtest2_debug_walls = debug;

	printf("Culling: backface=%d, distance=%d, debug=%d\n", backface, distance, debug);
}
static int get_wall_global_id(int sec, int wall, mapstate_t *map) {
	int id = 0;
	for (int i = 0; i < sec; i++) {
		id += map->sect[i].n;
	}
	return id + wall;
}
// Simplified sector wall adder
static void add_sector_walls(int sectnum, bdrawctx *b) {
	sect_t *sec = &curMap->sect[sectnum];
	wall_t *wal = sec->wall;

	for (int i = 0; i < sec->n; i++) {
		int j = wal[i].n + i;

		// Vectors from camera to wall endpoints
		double dx0 = wal[i].x - b->cam.p.x;
		double dy0 = wal[i].y - b->cam.p.y;
		double dx1 = wal[j].x - b->cam.p.x;
		double dy1 = wal[j].y - b->cam.p.y;

		// 2D cross product: positive if camera sees front (CCW winding)
		double cross = dx0 * dy1 - dx1 * dy0;

		if (shadowtest2_backface_cull && cross <= 0 && wal[i].ns <0) {
			if (shadowtest2_debug_walls) {
				logstep("  wall %d: CULLED (backface) white wall, cross=%.2f", i, cross);
			}
			continue;
		}

		// Midpoint distance for sorting
		double mx = (wal[i].x + wal[j].x) * 0.5 - b->cam.p.x;
		double my = (wal[i].y + wal[j].y) * 0.5 - b->cam.p.y;

		int wall_id = get_wall_global_id(sectnum, i, curMap);
		if (b->visited_walls[wall_id]) continue;
		b->visited_walls[wall_id] = true;

		if (b->jobcount >= b->jobcap) {
			b->jobcap = b->jobcap ? b->jobcap * 2 : 256;
			b->jobs = (wall_job_t *)realloc(b->jobs, b->jobcap * sizeof(wall_job_t));
		}

		b->jobs[b->jobcount].sec = sectnum;
		b->jobs[b->jobcount].wall = i;
		b->jobs[b->jobcount].dist = mx*mx + my*my;
		b->jobcount++;

		if (shadowtest2_debug_walls) {
			logstep("  wall %d: ADDED, dist=%.2f, cross=%.2f, depth=%d, sec:%d", i, sqrt(mx*mx + my*my), cross, b->recursion_depth, sectnum);
		}
	}

	qsort(b->jobs, b->jobcount, sizeof(wall_job_t), wall_dist_cmp);
}
static void draw_walls(mapstate_t *map, int s, int *walls, int wallcount, bdrawctx *b) {
#define MAXVERTS 256
	vertlist_t verts[MAXVERTS];
	dpoint3d pol[4];
	double opolz[4];
	dpoint3d npol2[3];
	sect_t *sec = curMap->sect;
	wall_t *wal = sec[s].wall;
	surf_t *sur;
	double f, dx;
	int m, vn, w, nw, ns;

    b->gligsect = s;
    b->gligslab = 0;
    bool noportals = b->recursion_depth >= MAX_PORTAL_DEPTH;

    // Loop through provided walls instead of bunch
    for (int ww = 0; ww < sec[s].n; ww++)
    {

    	int myport = wal[ww].tags[1];
    	bool isportal = myport >= 0 && portals[myport].destpn >= 0 && portals[myport].kind == PORT_WALL;
    	if (b->has_portal_clip && isportal)
    		int a =1;
    	bool skipport = isportal &&
			shadowtest2_debug_block_selfportals
		&& b->has_portal_clip
		&& s == b->testignoresec
		&& w == b->testignorewall
		&& portals[myport].kind == b->ignorekind;
		if (skipport)
{
			logstep("skipped portal at wall %d at sec %d",ww,s);
			continue; // drop this wall as it is not there.
			}

    	loopuse[loopnum] = false;
    	loopnum++;
        w=ww;
        vn = getwalls_imp(s, ww, verts, MAXVERTS, curMap);
        nw = wal[w].n + w;
        sur = &wal[w].surf;

        dx = sqrt((wal[nw].x - wal[w].x) * (wal[nw].x - wal[w].x) +
                  (wal[nw].y - wal[w].y) * (wal[nw].y - wal[w].y));

        b->gnorm.x = wal[w].y - wal[nw].y;
        b->gnorm.y = wal[nw].x - wal[w].x;
        b->gnorm.z = 0;
        f = 1.0 / sqrt(b->gnorm.x * b->gnorm.x + b->gnorm.y * b->gnorm.y);
        b->gnorm.x *= f;
        b->gnorm.y *= f;

        // Base wall quad - use full wall, no bunch clipping
        pol[0].x = wal[w].x;   pol[0].y = wal[w].y;
        pol[1].x = wal[nw].x;  pol[1].y = wal[nw].y;
        pol[0].z = getslopez(&sec[s], 0, pol[0].x, pol[0].y);
        pol[1].z = getslopez(&sec[s], 0, pol[1].x, pol[1].y);
        pol[2].x = pol[1].x; pol[2].y = pol[1].y;
        pol[2].z = getslopez(&sec[s], 1, pol[2].x, pol[2].y);
        pol[3].x = pol[0].x; pol[3].y = pol[0].y;
        pol[3].z = getslopez(&sec[s], 1, pol[3].x, pol[3].y);

        opolz[3] = pol[0].z;
        opolz[2] = pol[1].z;

        // Process slabs (unchanged from here)
        for (m = 0; m <= (vn << 1); m++)
        {


            int plothead[2];

            opolz[0] = opolz[3];
            opolz[1] = opolz[2];
            if (m == (vn << 1)) {
                opolz[2] = pol[2].z;
                opolz[3] = pol[3].z;
            } else {
                opolz[2] = getslopez(&sec[verts[m >> 1].s], m & 1, pol[2].x, pol[2].y);
                opolz[3] = getslopez(&sec[verts[m >> 1].s], m & 1, pol[3].x, pol[3].y);
            }

            if ((max(pol[0].z, opolz[0]) >= min(pol[3].z, opolz[3]) - 1e-4) &&
                (max(pol[1].z, opolz[1]) >= min(pol[2].z, opolz[2]) - 1e-4))
                continue;

            f = 1e-7;
            dpoint3d trap1[4] = {
                {pol[0].x, pol[0].y, pol[0].z - f}, {pol[1].x, pol[1].y, pol[1].z - f},
                {pol[2].x, pol[2].y, pol[2].z + f}, {pol[3].x, pol[3].y, pol[3].z + f}
            };
            dpoint3d trap2[4] = {
                {pol[0].x, pol[0].y, opolz[0] - f}, {pol[1].x, pol[1].y, opolz[1] - f},
                {pol[2].x, pol[2].y, opolz[2] + f}, {pol[3].x, pol[3].y, opolz[3] + f}
            };

            for (int i = 0; i < 4; i++) {
                portal_xform_world_fullp(&trap1[i], b);
                portal_xform_world_fullp(&trap2[i], b);
            }

            dpoint3d pol0_xf = pol[0];
            dpoint3d pol1_xf = pol[1];
            portal_xform_world_fullp(&pol1_xf,b);
            portal_xform_world_fullp(&pol0_xf,b);

            if (!intersect_traps_mono_points(pol0_xf, pol1_xf, trap1, trap2, &plothead[0], &plothead[1])) {
			//mono_deloop(plothead[0]);
			//mono_deloop(plothead[1]);
            	continue;
            }

            if ((!(m & 1)) || (wal[w].surf.flags & (1 << 5)))
            {
                npol2[0].x = wal[w].x;  npol2[0].y = wal[w].y;
                npol2[0].z = getslopez(&sec[s], 0, wal[w].x, wal[w].y);
                npol2[1].x = wal[nw].x; npol2[1].y = wal[nw].y;
                npol2[1].z = npol2[0].z;
                npol2[2].x = wal[w].x;  npol2[2].y = wal[w].y;
                npol2[2].z = npol2[0].z + 1.f;

                if (!(sur->flags & 4)) f = sec[s].z[0];
                else if (!vn) f = sec[s].z[1];
                else if (!m) f = sec[verts[0].s].z[0];
                else f = sec[verts[(m - 1) >> 1].s].z[0];

              // npol2[0].u = sur->uv[0].x;
              // npol2[0].v = sur->uv[2].y * (npol2[0].z - f) + sur->uv[0].y;
              // npol2[1].u = sur->uv[1].x * dx + npol2[0].u;
              // npol2[1].v = sur->uv[1].y * dx + npol2[0].v;
              // npol2[2].u = sur->uv[2].x + npol2[0].u;
              // npol2[2].v = sur->uv[2].y + npol2[0].v;
                b->gflags = 0;
                gentransform_wall(npol2, 3, sur, b);

                b->gligwall = w;
                b->gligslab = m;
                ns = -1;
                logstep("  slab %d: solid wall", m);
            } else {
                ns = verts[m >> 1].s;
                logstep("  slab %d: opening to sector %d", m, ns);
            }


			int mytag = s + taginc * b->recursion_depth;

            if (!skipport && !noportals && isportal) {
            	int endpn = portals[myport].destpn;
            	int nexttag = portals[endpn].sect + taginc * (b->recursion_depth + 1);

            	int visible = drawpol_befclip(mytag, nexttag,
								s, portals[endpn].sect,
								plothead[0], plothead[1],
								(((m > vn) << 2) + 3) | CLIP_PORTAL_FLAG, b);
            	if (visible) {
            		logstep("wall %d: m=%d, VISIBLE, ns=%d", w, m, ns);
            	} else {
            		logstep("wall %d: m=%d, not visible, skipping", w, m);
            	}
                draw_hsr_enter_portal(map, myport, b);
            } else {
            	int inc = taginc * b->recursion_depth;
            	int newtag;

            	// FIX: if ns == s (same sector), treat as solid wall
            	if (ns >= 0 && ns != s) {
            		newtag = ns + inc;
            	} else {
            		newtag = -1;  // Solid wall - will produce eyepol
            	}
            	logstep("wall %d: m=%d, ns=%d, s=%d, newtag=%d", w, m, ns, s, newtag);
            	int visible = drawpol_befclip(mytag, newtag, s, ns, plothead[0], plothead[1],
							   ((m > vn) << 2) + 3, b);
            	if (visible) {
            		logstep("wall %d: m=%d, VISIBLE, ns=%d", w, m, ns);
            	} else {
            		mono_deloop(plothead[0]);
            		mono_deloop(plothead[1]);
            		logstep("wall %d: m=%d, not visible, skipping", w, m);
            	}
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


// Simplified sector wall adder

void draw_hsr_polymost(cam_t *cc, mapstate_t *map, int dummy){
	bdrawctx bs;
	loopnum=0;
	bs.cam = *cc;
	bs.orcam = *cc;
	bs.recursion_depth = 0;
	bs.has_portal_clip = false;
	logstep("frame start");
	draw_hsr_ctx(map,&bs);

}
int lastcamsect = 0;
void draw_hsr_ctx (mapstate_t *map, bdrawctx *newctx) {
	if (!newctx) {
		return;
		if (captureframe) printf("discarding due to null ctx");
	}
	bdrawctx *b;
	b = newctx;
	int total_walls = 0;
	for (int i = 0; i < map->numsects; i++) {
		total_walls += map->sect[i].n;
	}
	b->visited_walls = (bool *) calloc(total_walls, 1);
	b->visited_sectors = (bool *) calloc(map->numsects, 1); // NEW
	b->jobcount = 0;


	int recursiveDepth = newctx->recursion_depth;
	b = newctx;
	b->sectgotn = 0;
	b->sectgot = 0;
	b->sectgotmal = 0;
	b->visited_walls = (bool *)calloc(total_walls, 1);
	b->jobcount = 0;
	b->jobcap = 16;
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

	curMap = map;

	//if ((map->numsects <= 0) || ((unsigned)gcam.cursect >= (unsigned)map->numsects))
	//{
	////if (shadowtest2_rendmode != 4) eyepoln = 0; //Prevents drawpollig() from crashing
	////	return;
	//}
	if (map->numsects > b->sectgotn)
	{
		if (b->sectgotmal) free((void *)b->sectgotmal);
		b->sectgotn = ((map->numsects+127)&~127);
		b->sectgotmal = (unsigned int *)malloc((b->sectgotn>>3)+16); //NOTE:malloc doesn't guarantee 16-byte alignment!
		b->sectgot = (unsigned int *)((((intptr_t)b->sectgotmal)+15)&~15);
	}
	if ((shadowtest2_rendmode != 4) && (map->numsects > shadowtest2_sectgotn))
	{
		if (shadowtest2_sectgotmal) free((void *)shadowtest2_sectgotmal);
		shadowtest2_sectgotn = ((map->numsects+127)&~127);
		shadowtest2_sectgotmal = (unsigned int *)malloc((shadowtest2_sectgotn>>3)+16); //NOTE:malloc doesn't guarantee 16-byte alignment!
		shadowtest2_sectgot = (unsigned int *)((((intptr_t)shadowtest2_sectgotmal)+15)&~15);
	}
	if (!mphmal)
		mono_initonce();

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
		if (map->numsects > glp->sectgotn)
		{
			if (glp->sectgotmal) free((void *)glp->sectgotmal);
			glp->sectgotn = ((map->numsects+127)&~127);
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
    if (b->orcam.cursect == -1)
	    b->orcam.cursect = lastcamsect;
    else
	    lastcamsect = b->orcam.cursect;
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
			    double far_dist = 1e6;
    cam_t *cam = &b->orcam;

    // Frustum corners in world space
			float mc = 900; far_dist = 900;
    xformbac(-mc, -mc, far_dist, &bord2[0], b);
    xformbac(+mc, -mc, far_dist, &bord2[1], b);
    xformbac(+mc, +mc, far_dist, &bord2[2], b);
    xformbac(-mc, +mc, far_dist, &bord2[3], b);
    n = 4;
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

			memset8(b->sectgot, 0, (map->numsects + 31) >> 3);

			for (i = mphnum - 1; i >= 0; i--) {
				mono_deloop(mph[i].head[1]);
				mono_deloop(mph[i].head[0]);
			}

			mono_genfromloop(&mph[0].head[0], &mph[0].head[1], bord2, n);
			mph[0].tag = gcam.cursect + taginc * b->recursion_depth;
			mphnum = 1;
		} else {
			// Portal case
			n = b->planecuts;
			didcut = 1;
			memset8(b->sectgot, 0, (map->numsects + 31) >> 3);
		}

    // Track which sectors have had their floors/ceilings drawn
		// Track which sectors have had their floors/ceilings drawn
		unsigned int *sectdrawn = (unsigned int *)_alloca(((map->numsects + 31) >> 3) + 4);
		memset(sectdrawn, 0, (map->numsects + 31) >> 3);

		b->jobcount = 0;
		b->jobs = malloc(sizeof(wall_job_t)*b->jobcap);
		//add_sector_walls(b->cam.cursect, b);
		//qsort(b->jobs, b->jobcount, sizeof(wall_job_t), wall_dist_cmp);

		// Group walls by sector for efficient getwalls_imp batching
		int current_sec = -1;
		int sec_walls[256];  // Temp array for walls in current sector
		int sec_wall_count = 0;
		int *nextsecs = calloc(55,sizeof(int));
		int nextsecsn=1;
		nextsecs[0]= b->cam.cursect;

		for (int gs=0;gs<=map->malsects;gs++) {
			int acumsec[55];
			int nss=0;
			for (int ni=0;ni<nextsecsn;ni++) { // breadth loop;
				int nxs = (nextsecs[ni]);
				if (!b->visited_sectors[nxs]) { // loop for one sector
					draw_walls(map, nxs, sec_walls, sec_wall_count, b);
					gs++;
					b->visited_sectors[nxs]=true;
					draw_sector_surface(nxs, 0, map, b); // Floor
					draw_sector_surface(nxs, 1, map, b); // Ceiling
					// get all neighbors;
					for (int wi=0;wi<map->sect[nxs].n;wi++) {
						int ns = map->sect[nxs].wall[wi].ns;
						if ( ns >= 0)
							if (!b->visited_sectors[ns]) {
								acumsec[nss]=ns;
								nss++;
							}
					}
				}
			}
			nextsecs = acumsec;
			nextsecsn=nss;
			if (nss==0)
				break;
		}
		if (false)//old one;
		{
			for (int ji = 0; ji < b->jobcount; ji++) {
				wall_job_t *job = &b->jobs[ji];

				if (job->sec != current_sec) {
					if (sec_wall_count > 0) {
						draw_walls(map, current_sec, sec_walls, sec_wall_count, b);
						b->visited_sectors[current_sec] = true; // NEW: Mark sector
						sec_wall_count = 0;
					}
					current_sec = job->sec;
				}

				sec_walls[sec_wall_count++] = job->wall;

				// FIX: Use .ns field, not tags[0]
				int ns = map->sect[job->sec].wall[job->wall].ns;
				if (ns >= 0 && ns != job->sec) {
					add_sector_walls(ns, b);
					qsort(b->jobs, b->jobcount, sizeof(wall_job_t), wall_dist_cmp);
				}
			}

    	// Flush last batch
    	if (sec_wall_count > 0) {
    		draw_walls(map, current_sec, sec_walls, sec_wall_count, b);
    		b->visited_sectors[current_sec] = true; // NEW: Mark last sector
    	}

    	// Draw surfaces
    	for (i = 0; i < map->numsects; i++) {
    		if (!b->visited_sectors[i]) continue; // NEW: Simple check
    		draw_sector_surface(i, 0, map, b); // Floor
    		draw_sector_surface(i, 1, map, b); // Ceiling
    	}
		}

		free(b->visited_walls);
		free(b->visited_sectors); // NEW: Cleanup



		// Draw any remaining sectors that were scanned but not reached via bunches
		//for (i = 0; i < map->numsects; i++) {
		//	if (!(b->sectgot[i >> 5] & (1 << i))) continue;
		//	if (sectdrawn[i >> 5] & (1 << i)) continue;
		//	draw_sector_surface(i, 0, map, b);
		//	draw_sector_surface(i, 1, map, b);
		//}

		if (shadowtest2_rendmode == 4)
			uptr = glp->sectgot;
		else
			uptr = shadowtest2_sectgot;

		memcpy(uptr, b->sectgot, (map->numsects + 31) >> 3);
	}
}
static void draw_hsr_enter_portal(mapstate_t* map, int myport, bdrawctx *parentctx)
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
    int ignkind = portals[endp].kind;

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

    bdrawctx newctx = {};
	newctx.prevsec = portals[myport].sect;
	newctx.newsec = portals[endp].sect;
	newctx.recursion_depth = parentctx->recursion_depth + 1;
	//if (newctx.recursion_depth >0 )
	//	printf("ncam fw %f,%f,%f \r", ncam.f.x,ncam.f.y,ncam.f.z);
    newctx.cam = ncam;
    newctx.orcam = parentctx->orcam;
    newctx.has_portal_clip = true;

    newctx.sectgotn = 0;
    newctx.sectgot = 0;
    newctx.sectgotmal = 0;
newctx.testignorewall = ignw;
    newctx.testignoresec = igns;
    newctx.ignorekind = ignkind;
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
