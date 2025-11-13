//
// Created by omnis on 11/13/2025.
//

#ifndef RAYLIB_LUA_IMGUI_EVENTSTORE_H
#define RAYLIB_LUA_IMGUI_EVENTSTORE_H
#include "event_types.h"
#include "ev_projection.h"

typedef struct {
    event_t* events;
    size_t count;
    size_t capacity;
    projection_t** projections;
    size_t projection_count;
    size_t projection_capacity;
} event_store_t;

void eventstore_init(void);
void eventstore_add_projection(projection_t* projection);
void eventstore_add_event(const event_t* event);
void eventstore_replay_all(void);

// Helper functions to create events
event_t eventstore_create_sprite_pos_event(int sprite_id, float x, float y);
event_t eventstore_create_sprite_delete_event(int sprite_id);

#endif //RAYLIB_LUA_IMGUI_EVENTSTORE_H