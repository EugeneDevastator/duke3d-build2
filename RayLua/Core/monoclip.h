 // MONOCLIP - Monotone Polygon Clipping Library (by Ken Silverman)
//
// This module implements efficient 2D/3D polygon clipping operations using monotone polygon
// decomposition. It provides boolean operations (AND, SUB, SUB_REV) on polygons and handles
// polygon intersection, union, and difference calculations.
//
// Core data structures:
// - mp_t: Doubly-linked list nodes representing polygon vertices with x,y,z coordinates
// - mph_t: Polygon headers containing head pointers and tags for polygon management
//
// The library uses a memory pool system for efficient vertex allocation/deallocation
// and maintains monotone polygon properties for optimal clipping performance.

#ifndef BUILD2_MONOCLIP_H
#define BUILD2_MONOCLIP_H

#include "mapcore.h"

#define MONO_BOOL_AND 0 // result is intersection
#define MONO_BOOL_SUB 1
#define MONO_BOOL_SUB_REV 2
typedef struct {
	int *indices;
	int count;
	int capacity;
} triangle_strip_t;


//Mono Polygon Head
typedef struct {
	int head[2], tag;
} mph_t;

//Mono Polygon (vertex data)
typedef struct {
	union {
		dpoint3d pos;
		struct {double x, y, z;};
	};

	int n, p;  // next, previous indices for doubly-linked list
} mp_t;

// ================================================================================================
// POLYGONAL SCENE CLIPPING DATA STRUCTURES
// ================================================================================================
#define BFINTMAX 256
/** Wall segment bunch for front-to-back sorting */
typedef struct {
	int sec, wal0, wal1;                // Sector index and wall range
	double fra0, fra1;                  // Parametric fractions for wall clipping
} bunch_t;

/** Bunch intersection data for polygon splitting */
typedef struct {
	int bun, sid;                       // Bunch index and intersection side (1:/, 2:\)
	int wal;                            // Wall index on sector
	double fra;                         // Intersection point ratio [0.0..1.0]
} bfint_t;

/** Temporary vertex structure for bunch processing
* 	//for twal, it is always safe to allocate sector's number of walls + 1 indices
typedef struct { int i; double x, y; } bunchverts_t; //temp structure holding verts of bunch - easier to work with

 */
typedef struct {
	int i;                              // Original wall index
	double x, y;                        // Clipped coordinates
} bunchverts_t;


#define MAX_PORTAL_VERTS 32

void mono_triangulate_strip(int hd0, int hd1, triangle_strip_t *strip);
void strip_init(triangle_strip_t *strip);
void strip_free(triangle_strip_t *strip);
void strip_add(triangle_strip_t *strip, int index);

typedef struct {
	int sec, wall;
	float dist;
} wall_job_t;
typedef struct {
	// ---------- bunch context
	bunch_t *bunch;
	unsigned int *bunchgot;
	unsigned char *bunchgrid;
	int bunchn, bunchmal;
	bfint_t bfint[BFINTMAX];
	int bfintn, bfintlut[BFINTMAX+1];
	unsigned int *sectgot, *sectgotmal;        // Visited sectors per level
	int sectgotn;
	int entrysec;
	// mono context
	bool has_mono_out; // Whether portal clipping is active
	// transform context
	cam_t cam;                    // Camera per recursion level
 	cam_t orcam; // one true camera, read only.
	double xformmat[9], xformmatc, xformmats;
	double oxformmat[9], oxformmatc, oxformmats;
	point3d gnadd, ognadd;
	point3d gnorm;
	float gouvmat[9]; // 0 3 6 - store plane equation to convert back from mp.
	int gligsect, gligwall, gligslab, gflags;
	int gnewtag, gdoscansector, gnewtagsect;
	// n-portals context
	bool has_portal_clip; // Whether portal clipping is active
	int recursion_depth;
	int tagoffset;
	int testignorewall;
	int ignorekind;
	int testignoresec;
	int planecuts;
	int currenthalfplane;
	int chead[2];
} bdrawctx;

// mono vertex chains
extern mp_t *mp;
// head indices pointing to mp
extern mph_t *mph;
extern int mphnum, mphmal;
extern int mpempty, mpmal;
// Memory management - ensures mph array has sufficient capacity
void mono_mph_check (int num);

// Initialize memory pools and data structures (call once at startup)
void mono_initonce ();

// Insert vertex into heads pool
int  mono_ins2d (int i, double nx, double ny);
// Insert vertex into heads pool
int  mono_ins (int i, double nx, double ny, double nz);  // 3D version
// Insert vertex into heads pool
int mono_insp(int i, dpoint3d p);

// Insert vertex into heads pool
void mono_del (int i);

// Delete entire polygon loop starting at vertex i
void mono_deloop (int i);

#ifdef STANDALONE
// Calculate polygon centroid and area using shoelace formula
void mono_centroid_addlin (int i0, int i1, double *cx, double *cy, double *area);
double mono_centroid (int hd0, int hd1, double *retcx, double *retcy);
double mono_area (int hd0, int hd1);  // Returns signed area of polygon
#endif

// Generate monotone polygon pair from input vertex loop
void mono_genfromloop (int *plothead0, int *plothead1, dpoint3d *tp, int n);

//Similar to general intersection, except:
	//  1. Assumes: x0=x2,y0=y2 and x1=x3,y1=y3
	//  2. No error checking
// 3D line intersection with same x,y endpoints but different z values
void mono_intersamexy (double x0, double y0, double x1, double y1, double z0, double z1, double z2, double z3, double *ix, double *iy, double *iz);

//(x0,y0) (x1,y1)
	//   z0-----z1
	//    |     |
	//   z3-----z2
	//               NOTE:These z indices are parm order, not parm name!
	//   z4-----z5
	//    |     |
	//   z7-----z6
	//See WALLCLIP.KC for derivation
	// Intersect two trapezoids in 3D space, returns monotone polygon intersection
int intersect_traps_mono (double x0, double y0, double x1, double y1,
											double z0, double z4, double z5, double z1,
											double z2, double z6, double z7, double z3, int *rh0, int *rh1);
int intersect_traps_mono_points(dpoint3d p0, dpoint3d p1, dpoint3d trap1[4], dpoint3d trap2[4], int *rh0, int *rh1);
// Find maximum/minimum envelope of two monotone polygons
// maxsid: -1=top envelope, +1=bottom envelope; mode: 0=intersection, 1=union
int mono_max (int hd0, int hd1, int maxsid, int mode);

// Clip two monotone polygons against each other, calls output function for results
int mono_clipself (int hd0, int hd1, bdrawctx* b, void (*mono_output)(int h0, int h1,bdrawctx* b));

#ifdef STANDALONE
	//May be useful for splitting walls off of bunch processing
	// Clip polygon to x-range [x0,x1], useful for wall splitting
int mono_clipends (int hd, double x0, double x1);
#endif

// Returns 1 on success, 0 on failure
// ho0 points to first joined chain: hd0 → iy[0] → iy[2] → hd2
// ho1 points to second joined chain: hd1 → iy[1] → iy[3] → hd3

int mono_join (int hd0, int hd1, int hd2, int hd3, int *ho0, int *ho1);

//AND on A,B: Callback gets intersection pieces (A ∩ B)
//SUB on A,B: Callback gets pieces of A outside B (A - B)
//SUB_REVERSE on A,B: Callback gets pieces of A outside B with reverse winding?.
// Perform boolean operation on two polygon pairs (AND, SUB, SUB_REV)
// re
void mono_bool (int subjh0, int subjh1, int cutter0, int cutter1, int boolop, bdrawctx* b, void (*mono_output)(int h0, int h1,bdrawctx* b));
// Generate triangle strip vertices directly from monotone polygon
int mono_generate_eyepol(int hd0, int hd1, point3d **out_verts1,  point3d **out_verts2, int *out_count1, int *out_count2);
// adds mono to mph directly

// registers loop into mono heads with tag
int mph_appendloop(int *outh1, int *outh2, dpoint3d *tp, int n, int newtag);
// removes mph and points from mph list.
int mph_remove(int delid);
int mph_append( int h1, int h2, int tag);


int mpcheck(int h1,int h2);
int mphremoveontag(int tag);
int mphremoveaboveincl(int tag_including);
void monocopy(int h1, int h2, int *hout1, int *hout2);

#endif //BUILD2_MONOCLIP_H
