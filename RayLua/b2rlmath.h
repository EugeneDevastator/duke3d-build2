//
// Created by omnis on 12/31/2025.
//

#ifndef RAYLIB_LUA_IMGUI_B2RLMATH_H
#define RAYLIB_LUA_IMGUI_B2RLMATH_H
#include "raylib.h"
#include "interfaces/shared_types.h"
static inline Vector3 vec3_from_p3(point3d buildcoord)
{
	return {buildcoord.x, -buildcoord.z, buildcoord.y};
}
static inline point3d p3_from_vec3(Vector3 rlcoord)
{
	return {rlcoord.x, rlcoord.z, -rlcoord.y};
}

static inline void cam3d_from_tr(Camera3D* rlcam, transform* b2tr){
	rlcam->position = vec3_from_p3(b2tr->p);
	Vector3 forward = vec3_from_p3(b2tr->f);
	Vector3 up = vec3_from_p3(b2tr->d); // Recalculate orthogonal up
	rlcam->up = Vector3Scale(up,-1);
	rlcam->target = rlcam->position + forward;
}
static inline void tr_from_cam3d(transform* b2tr, Camera3D* rlcam){
	b2tr->p = p3_from_vec3(rlcam->position);

	Vector3 forward = Vector3Normalize(Vector3Subtract(rlcam->target, rlcam->position));
	Vector3 up = Vector3Normalize(rlcam->up);
	Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, up));

	// Recalculate orthogonal up vector
	up = Vector3CrossProduct(right, forward);

	b2tr->f = p3_from_vec3(forward);
	b2tr->r = p3_from_vec3(right);
	b2tr->d = p3_from_vec3(Vector3Scale(up, -1.0f));
}

#endif //RAYLIB_LUA_IMGUI_B2RLMATH_H