//
// Created by omnis on 11/25/2025.
//

#ifndef BUILD2_SCENERENDER_H
#define BUILD2_SCENERENDER_H
#include "mapcore.h"

typedef struct {
    //screen/camera state
    float ghx, ghy, ghz, zoom, ozoom;
    union {
struct { point3d ipos, irig, idow, ifor;};
        transform tr;
    };

    point3d npos, nrig, ndow, nfor; //for 2d/3d swap animation
    point3d grdc, grdu, grdv, grdn; //center,u,v,normal
    int cursect;
} player_transform;


#endif //BUILD2_SCENERENDER_H