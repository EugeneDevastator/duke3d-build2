//
// Created by omnis on 11/10/2025.
//
#include "engineapi.h"

#include <stdlib.h>
static mapstate_t *mapref;
char *inputs;

void (*targetupdate)(float t);


void GetKeysThisFrame(char *writearr) {
    writearr = inputs;
}

void SetPlayerPos(float x, float y, float z) {
    px = x;
    py = y;
    pz = z;
}

void SetSpritePos(int i, float x, float y, float z) {
    mapref->spri[i].p.x = x;
    mapref->spri[i].p.y = y;
    mapref->spri[i].p.z = z;
}

void SetFloorZ(int i, float z) {
    mapref->sect[i].z[1] = z;
}

void SetCeilZ(int i, float z) {
    mapref->sect[i].z[0] = z;
}

mapstate_t *GetLoadedMap() {
    return mapref;
}

void RegisterUpdate(void (*UpdateFunc)(float t)) {
    targetupdate = UpdateFunc;
}

void ForwardEngineUpdate(float dt) {
    targetupdate(dt);
}

void InitEngineApi(mapstate_t *map) {
    mapref = map;
    inputs = (char *) calloc(10, sizeof(char));
    engine.Inputs = inputs;
    engine.SetSpritePos = SetSpritePos;
    engine.SetFloorZ = SetFloorZ;
    engine.SetCeilZ = SetCeilZ;
    engine.SetPlayerPos = SetPlayerPos;
}
