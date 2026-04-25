// File: .../RayLua/Core/mapserial.h
#ifndef MAPSERIAL_H
#define MAPSERIAL_H

#include "mapform_b2.h"
#include <stdio.h>

int map_save_b2(const char *path, mapstate_t *map);
int map_load_b2(const char *path, mapstate_t *map);

#endif
