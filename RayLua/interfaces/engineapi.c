//
// Created by omnis on 11/10/2025.
//
#include "engineapi.h"

#include <stdlib.h>

#include "mapcore.h"
#include "source/game.h"
static mapstate_t *mapref;
char *inputs;
float px,py,pz = 0;
float pfx,pfy,pfz = 0;
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

static void InsertSprite(int sectid, float x, float y, float z) {
    long i = insspri_imp(sectid,x,y,z,mapref);
    mapref->spri[i].tilnum=1;
}

static void DelSprite(int id) {
    delspri_imp(id,mapref);
}

void SetSpritePicNum(int id, int picnum) {
    mapref->spri[id].tilnum;
}

point3d GetPlayerPos() {
    return (point3d){px,py,pz};
}
point3d GetPlayerFrw() {
    return (point3d){pfx,pfy,pfz};
}
void SetPlayerForward(float x, float y, float z) {
    pfx = x;
    pfy = y;
    pfz = z;
}
void InitEngineApi(mapstate_t *map) {
    mapref = map;
    inputs = (char *) calloc(30, sizeof(char));
    engine.Inputs = inputs;
    engine.SetSpritePos = SetSpritePos;
    engine.SetFloorZ = SetFloorZ;
    engine.SetCeilZ = SetCeilZ;
    engine.SetPlayerPos = SetPlayerPos;
    engine.GetLoadedMap = GetLoadedMap;
    engine.RegisterUpdate = RegisterUpdate;
    engine.InsertSprite = InsertSprite;
    engine.DeleteSprite = DelSprite;
    engine.SetPlayerForward = SetPlayerForward;
}
