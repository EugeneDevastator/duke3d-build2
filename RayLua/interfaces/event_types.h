//
// Created by omnis on 11/13/2025.
//

#ifndef RAYLIB_LUA_IMGUI_EVENT_TYPES_H
#define RAYLIB_LUA_IMGUI_EVENT_TYPES_H
#include <stdint.h>
typedef enum {
    EVENT_SPRITE_SET_POS = 1,
    EVENT_SPRITE_DELETE = 2
} event_type_t;

typedef struct {
    event_type_t type;
    uint8_t payload[32];
    uint64_t timestamp;
} event_t;

// Event payload structs
typedef struct {
    uint32_t sprite_id;
    float pos_x;
    float pos_y;
} sprite_pos_payload_t;

typedef struct {
    uint32_t sprite_id;
} sprite_delete_payload_t;

#endif //RAYLIB_LUA_IMGUI_EVENT_TYPES_H