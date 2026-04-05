//
// Created by omnis on 11/13/2025.
//
#include "event_types.h"
#include "eventstore.h"
#include "ev_projection.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

static event_store_t g_store = {0};

static uint64_t get_timestamp(void) {
    return (uint64_t)time(NULL);
}

void eventstore_init(void) {
    g_store.capacity = 1000;
    g_store.events = malloc(sizeof(event_t) * g_store.capacity);
    g_store.count = 0;

    g_store.projection_capacity = 10;
    g_store.projections = malloc(sizeof(projection_t*) * g_store.projection_capacity);
    g_store.projection_count = 0;
}

void eventstore_add_projection(projection_t* projection) {
    if (g_store.projection_count < g_store.projection_capacity) {
        g_store.projections[g_store.projection_count++] = projection;
    }
}

void eventstore_add_event(const event_t* event) {
    if (g_store.count < g_store.capacity) {
        g_store.events[g_store.count] = *event;
        g_store.events[g_store.count].timestamp = get_timestamp();

        // Apply to all projections polymorphically
        for (size_t i = 0; i < g_store.projection_count; i++) {
            projection_t* proj = g_store.projections[i];
            proj->apply_event(proj, &g_store.events[g_store.count]);
        }

        g_store.count++;
    }
}

void eventstore_replay_all(void) {
    for (size_t i = 0; i < g_store.projection_count; i++) {
        projection_t* proj = g_store.projections[i];
        for (size_t j = 0; j < g_store.count; j++) {
            proj->apply_event(proj, &g_store.events[j]);
        }
    }
}

// Helper functions
event_t eventstore_create_sprite_pos_event(int sprite_id, float x, float y) {
    event_t event = {0};
    event.type = EVENT_SPRITE_SET_POS;

    sprite_pos_payload_t payload = {sprite_id, x, y};
    memcpy(event.payload, &payload, sizeof(payload));

    return event;
}

event_t eventstore_create_sprite_delete_event(int sprite_id) {
    event_t event = {0};
    event.type = EVENT_SPRITE_DELETE;

    sprite_delete_payload_t payload = {sprite_id};
    memcpy(event.payload, &payload, sizeof(payload));

    return event;
}
