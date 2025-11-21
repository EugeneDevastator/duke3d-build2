//
// Created by omnis on 11/21/2025.
//

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
	int n, p;
} mp_t;

extern mp_t *mp;
extern mph_t *mph;
extern int mphnum, mphmal;

void mph_check (int mphnum); //ensures mph[0 .. mphnum] is safe to access;
void mono_initonce ();
int  mono_ins (int i, double nx, double ny); //if (i < 0) start_list else inserts point after ;
int  mono_ins (int i, double nx, double ny, double nz);
void mono_del (int i);
void mono_deloop (int i);

#ifdef STANDALONE
	//Find centroid of polygon (copied from Build2, which is from TAGPNT2.BAS 09/14/2006)
void mono_centroid_addlin (int i0, int i1, double *cx, double *cy, double *area);

double mono_centroid (int hd0, int hd1, double *retcx, double *retcy);

double mono_area (int hd0, int hd1);
#endif

void mono_genfromloop (int *plothead0, int *plothead1, dpoint3d *tp, int n);

//Similar to general intersection, except:
	//  1. Assumes: x0=x2,y0=y2 and x1=x3,y1=y3
	//  2. No error checking
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
int intersect_traps_mono (double x0, double y0, double x1, double y1,
											double z0, double z4, double z5, double z1,
											double z2, double z6, double z7, double z3, int *rh0, int *rh1);

int mono_max (int hd0, int hd1, int maxsid, int mode) //maxsid:-1=top,+1=bot, mode:0:inner, 1:first
;

int mono_clipself (int hd0, int hd1, void (*mono_output)(int h0, int h1));

#ifdef STANDALONE
	//May be useful for splitting walls off of bunch processing
int mono_clipends (int hd, double x0, double x1);
#endif

int mono_join (int hd0, int hd1, int hd2, int hd3, int *ho0, int *ho1);


void mono_bool (int hr0, int hr1, int hw0, int hw1, int boolop, void (*mono_output)(int h0, int h1));

#endif //BUILD2_MONOCLIP_H
