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

#define MONO_BOOL_AND 0
#define MONO_BOOL_SUB 1
#define MONO_BOOL_SUB_REV 2

//Mono Polygon Head
typedef struct {
	int head[2], tag;
} mph_t;

//Mono Polygon (vertex data)
typedef struct {
	double x, y, z;
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
typedef struct {
	float m[9];
	point3d p,h;
}cam_transform_t;
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

#define MAX_PORTAL_DEPTH 1
#define MAX_PORTAL_VERTS 32


typedef struct {
	bunch_t *bunch;
	unsigned int *bunchgot;
	unsigned char *bunchgrid;
	int bunchn, bunchmal;

	bfint_t bfint[BFINTMAX];
	int bfintn, bfintlut[BFINTMAX+1];
// other context stuff
	cam_t cam;                    // Camera per recursion level
 	cam_t orcam; // one true camera, read only.
	double xformmat[9], xformmatc, xformmats;
	double xformmatc_g, xformmats_g;
	point3d gnadd;
	unsigned int *sectgot, *sectgotmal;        // Visited sectors per level
	int sectgotn;
	int portal_clip[2];  // Current portal clipping region
	bool has_portal_clip; // Whether portal clipping is active
	int recursion_depth;
	float gouvmat[9]; // 0 3 6 - store plane equation to convert back from mp.
	int gligsect, gligwall, gligslab, gflags;
	int gnewtag, needsecscan, gnewtagsect;
	point3d gnorm;
	int testignorewall;
	int testignoresec;
	int currenthalfplane;
	int planecuts;

	cam_transform_t ct;     // NEW: clean transform for current camera
	cam_transform_t ct_or;  // NEW: clean transform for original camera (for eyepol output)

} bunchgrp;

extern mp_t *mp;
extern mph_t *mph;
extern int mphnum, mphmal;
extern int mpempty, mpmal;
// Memory management - ensures mph array has sufficient capacity
void mph_check (int mphnum);

// Initialize memory pools and data structures (call once at startup)
void mono_initonce ();

// Insert vertex into polygon chain - if i<0 starts new loop, else inserts after vertex i
int  mono_ins2d (int i, double nx, double ny);
int  mono_ins (int i, double nx, double ny, double nz);  // 3D version

// Remove vertex from polygon chain and return to free pool
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
void intersamexy (double x0, double y0, double x1, double y1, double z0, double z1, double z2, double z3, double *ix, double *iy, double *iz);

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

// Find maximum/minimum envelope of two monotone polygons
// maxsid: -1=top envelope, +1=bottom envelope; mode: 0=intersection, 1=union
int mono_max (int hd0, int hd1, int maxsid, int mode);

// Clip two monotone polygons against each other, calls output function for results
int mono_clipself (int hd0, int hd1, bunchgrp* b, void (*mono_output)(int h0, int h1,bunchgrp* b));

#ifdef STANDALONE
	//May be useful for splitting walls off of bunch processing
	// Clip polygon to x-range [x0,x1], useful for wall splitting
int mono_clipends (int hd, double x0, double x1);
#endif

// Join four polygon segments into two output polygons
int mono_join (int hd0, int hd1, int hd2, int hd3, int *ho0, int *ho1);

// Perform boolean operation on two polygon pairs (AND, SUB, SUB_REV)
void mono_bool (int hr0, int hr1, int hw0, int hw1, int boolop, bunchgrp* b, void (*mono_output)(int h0, int h1,bunchgrp* b));

#endif //BUILD2_MONOCLIP_H
