//
// Created by omnis on 11/8/2025.
//

#ifndef BUILD2_PHYSICS_H
#define BUILD2_PHYSICS_H
#include "mapcore.h"

// Raycasting flags and modes

#define RHIT_SPRITES		 (1<<0)
#define RHIT_WALLS  		 (1<<1)
#define RHIT_CAPS   		 (1<<2)
#define RHIT_ACTORS 		 (1<<3)
#define RHIT_PROPS  		 (1<<4)
#define RHIT_INVISIBLE		 (1<<5)
#define RHIT_REDWALLS 		 (1<<6) // if on will stop at standard wall portals
#define RHIT_PORTALS		 (1<<7) // for new wccw style portals

#define RHIT_ALLNORMAL RHIT_SPRITES | RHIT_WALLS | RHIT_CAPS



typedef struct {
	double gammaval; //1.0=no change, useful range={0.0..~4.0)
	//----------------------- DATA coming from BUILD2.C -----------------------

	//Clipmove hit point info (use this after calling clipmove):
	double clipmaxcr; //clipmove always calls findmaxcr even with no movement
	dpoint3d cliphit[3];
	long cliphitnum, clipsect[3], clipwall[3];

	//----------------------- DATA provided to BUILD2.C -----------------------

	double fattestsprite; //For sprite collision: when to cross sectors
} clipdata;

extern clipdata build2;
int hitscan_b2 (point3d *p0, point3d *pv, point3d *viewright,point3d *viewdown, float vscale, int cursect, int *hitsect, int *hitwall, point3d *hit, mapstate_t* map);
int raycast(point3d *p0, point3d *pv, float vscale, int cursect, int *hitsect, int *hitwall, int *hitsprite, int *hitsurf,
            point3d *hit, uint32_t scanflags, mapstate_t *map);

void collmove(dpoint3d *p, int *cursect, dpoint3d *v, double cr, long doslide, mapstate_t *map);

void collmove_p(point3d *p, int *cursect, point3d *v, double cr, long doslide, mapstate_t *map);

double findmaxcr(dpoint3d *p0, int cursect, double mindist, dpoint3d *hit, mapstate_t *map);


//Find maximum clip radius (distance to closest point of any visible polygon)

//Note: pol doesn't support loops as dpoint3d's!
//(flags&1): collide both sides of plane
double sphpolydist(dpoint3d *p0, dpoint3d *v0, double cr, dpoint3d *pol, int n, int flags, dpoint3d *hit);

double sphtracewall(dpoint3d *p0, dpoint3d *v0, double cr, int s, int w, int s0, int cf0, int s1, int cf1,
                    dpoint3d *hit, mapstate_t *map);

double sphtracerec(dpoint3d *p0, dpoint3d *v0, dpoint3d *hit, int *cursect, double cr, mapstate_t *map);

//Returns: 1 if no obstacles, 0 if hit something
long sphtrace(dpoint3d *p0, //start pt
              dpoint3d *v0, //move vector
              dpoint3d *hit, //pt causing collision
              int *cursect,
              double cr, mapstate_t *gst);

//int hitscan (point3d *p0, point3d *pv, float vscale, int cursect, int *hitsect, int *hitwall, point3d *hit, mapstate_t* map);

#endif //BUILD2_PHYSICS_H
