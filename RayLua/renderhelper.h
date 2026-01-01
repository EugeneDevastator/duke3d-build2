//
// Created by omnis on 12/29/2025.
//

#ifndef RAYLIB_LUA_IMGUI_RENDERHELPER_H
#define RAYLIB_LUA_IMGUI_RENDERHELPER_H
#include "external/glad.h"

static inline void EnableDepthOffset(float mul) {
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(mul, 1.0f);
}

static inline void DisableDepthOffset() {
	glDisable(GL_POLYGON_OFFSET_FILL);
}

#endif //RAYLIB_LUA_IMGUI_RENDERHELPER_H