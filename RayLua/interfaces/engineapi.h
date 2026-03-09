//
// Created by omnis on 11/10/2025.
//

#ifndef R_ENGINEAPI_H
#define R_ENGINEAPI_H

#include "mapform_b2.h"
#include "shared_types.h"
#define W_FRW 0
#define S_BACK 1
#define A_LEFT 2
#define D_RIGHT 3
#define E_USE 4
#define SPC_JUMP 5
#define CROUCH 6
#define MB_SHOOT 7
#define Q_TLEFT 8
#define R_TRIGHT 9
#define ACT_AIM_UP 10
#define ACT_AIM_DOWN 11

// wsad use-E jump-Space crouch-letfctrl
typedef struct {
    // void (*GetKeysThisFrame)(char *writearr);
    void (*SetPlayerPos)(float x, float y, float z);

    void (*SetSpritePos)(int i, float x, float y, float z);

    void (*SetFloorZ)(int i, float z);

    void (*SetCeilZ)(int i, float z);

    mapstate_t * (*GetLoadedMap)();

    void (*RegisterUpdate)(void (*UpdateFunc)(float t));
    void (*InsertSprite)(int sect, float x,float y,float z);
    void (*DeleteSprite)(int sid);
    void (*SetPlayerForward)(float x, float y, float z);
    void (*SetSpritePicNum)(int i, int picnum);
    char *Inputs;
// temp info
    int tilesizex[7000];
    int tilesizey[7000];
    uint32_t picanms[7000];
} engineapi_t;

engineapi_t engine;
point3d GetPlayerPos();
point3d GetPlayerFrw();
void InitEngineApi(mapstate_t *map);
void ForwardEngineUpdate(float dt);
#endif
