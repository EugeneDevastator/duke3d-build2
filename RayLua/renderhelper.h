//
// Created by omnis on 12/29/2025.
//

#ifndef RAYLIB_LUA_IMGUI_RENDERHELPER_H
#define RAYLIB_LUA_IMGUI_RENDERHELPER_H
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GL/gl.h>
#include <GL/glext.h>

static inline void EnableDepthOffset(float mul) {
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(mul, 1.0f);
}

static inline void DisableDepthOffset() {
	glDisable(GL_POLYGON_OFFSET_FILL);
}

#endif //RAYLIB_LUA_IMGUI_RENDERHELPER_H
