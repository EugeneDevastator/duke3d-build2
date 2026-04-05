//
// Created by omnis on 11/13/2025.
//

#ifndef RAYLIB_LUA_IMGUI_EV_PROJECTION_H
#define RAYLIB_LUA_IMGUI_EV_PROJECTION_H

#include "event_types.h"

typedef struct projection projection_t;

typedef void (*apply_event_fn)(projection_t* self, const event_t* event);

struct projection {
    apply_event_fn apply_event;
    void* data;  // projection-specific state
};
#endif //RAYLIB_LUA_IMGUI_EV_PROJECTION_H