// File: .../RayLua/Core/mapserial.h
#ifndef MAPSERIAL_H
#define MAPSERIAL_H

#include "mapform_b2.h"
#include <stdio.h>

int map_save_b2(const char *path, mapstate_t *map);
int map_load_b2(const char *path, mapstate_t *map);

#endif

/*
note, sept 2026:
make it chunked:
// -- fundamental markup, most stable ---
sector[] : nwalls, id
capform[] : z, xnorm, ynorm x2 maps to sector 2-1
walls[] : xy, chainwal //mapped to sector 1-1 by nwalls
walink[] : nlinks, (nsid, nwid) - maps to walls 1-1 // portal linkage
xform[] : transform, sectorid, ownid // xforms of the sprites with id. basic markup
// -- attached data, volataile ----
capdata[]
walldata[]
spritedata[] : spriteid, data..
sprlightdata[] : spriteid, data..
// -- arts
capart[] // floors and ceils of each sector
wallart[] //
spriteart[] //
*/

/* note sept 14 2026
solution to lights and portals.
1. need multiple MPH arrays. - because of translucent polys.
translucent ones are emitted for draw, but are considered as portals same time.

- render all light polys to world, match them to sectors.
- all lights are rendered into main world, possibly overlapping due to mirrors/portals.

loop looks like this:
for each sector:
	for each poly:
		- process as usual
		- emit opaque and translucent copies into separate MPH2 array
	- draw lightpolys for this sector, using AND(MPH2) to clip them.
	- reset MPH2

drawing lightpoly is:
- project from world to current camera
- clip with MPH2 
	-(cb) emitting multiple sub-polys. emit to render array. 	
*/