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
walls[] : xy, nporals, nwallid[nportals], ownid //mapped to sector 1-1 by nwalls
xfrom[] : transform, sectorid, ownid // xforms of the sprites with id. basic markup
// -- attached data, volataile ----
capart[] // floors and ceils of each sector
wallart[] //
spritedata[] : spriteid, data..
sprlightdata[] : spriteid, data..

*/