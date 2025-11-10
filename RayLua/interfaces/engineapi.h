//
// Created by omnis on 11/10/2025.
//

#ifndef R_ENGINEAPI_H
#define R_ENGINEAPI_H
#include "shared_types.h"

// wsad use-E jump-Space crouch-letfctrl
typedef struct{
    void (*GetKeysThisFrame)(char *writearr);
    void (*SetPlayerPos)(float x, float y, float z);
    mapstate_t* (*GetLoadedMap)();
}engineapi_t;
    #endif