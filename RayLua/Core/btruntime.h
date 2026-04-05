//
// Created by omnis on 12/1/2025.
//

#ifndef RAYLIB_LUA_IMGUI_BTRUNTIME_H
#define RAYLIB_LUA_IMGUI_BTRUNTIME_H
#include "mapcore.h"
// stub for engine runtime
typedef struct {
    float anchor;
    float viewtype;
    int orisp; // origin sprite
    int ch[]; // children views
} sview;

typedef struct {
    view *views;
    int viewn;
    int viewmal;

    // dblinked list for views.
    int *vroot; // head in ken's code.
    int *vnext;
    int *vrev;

    int pview; // player view;

    mapstate_t* base_map; // original unmodified map;
    mapstate_t* run_map; // runtime copy for modifications;
} runstate;


#endif //RAYLIB_LUA_IMGUI_BTRUNTIME_H