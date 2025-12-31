//
// Created by omnis on 12/31/2025.
//

#ifndef RAYLIB_LUA_IMGUI_B2RLMATH_H
#define RAYLIB_LUA_IMGUI_B2RLMATH_H
#include "raylib.h"
#include "interfaces/shared_types.h"
static inline Vector3 buildToRaylib(point3d buildcoord)
{
	return {buildcoord.x, -buildcoord.z, buildcoord.y};
}
static inline point3d raylibtobuild(Vector3 rlcoord)
{
	return {rlcoord.x, rlcoord.z, -rlcoord.y};
}
// In-place conversion - modifies original
static inline void toRaylibInPlace(point3d *buildcoord)
{
	float temp_y = buildcoord->y;
	buildcoord->y = -buildcoord->z;
	buildcoord->z = temp_y;
}
static inline void camfromb2(Camera3D* rlcam, transform* b2tr){
	rlcam->position = buildToRaylib(b2tr->p);
	Vector3 forward = buildToRaylib(b2tr->f);
	Vector3 up = buildToRaylib(b2tr->d); // Recalculate orthogonal up
	rlcam->up = Vector3Scale(up,-1);
	rlcam->target = rlcam->position + forward;
}
static inline void camfromrl(transform* b2tr, Camera3D* rlcam){
	b2tr->p = raylibtobuild(rlcam->position);

	Vector3 forward = Vector3Normalize(Vector3Subtract(rlcam->target, rlcam->position));
	Vector3 up = Vector3Normalize(rlcam->up);
	Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, up));

	// Recalculate orthogonal up vector
	up = Vector3CrossProduct(right, forward);

	b2tr->f = raylibtobuild(forward);
	b2tr->r = raylibtobuild(right);
	b2tr->d = raylibtobuild(Vector3Scale(up, -1.0f));
}

#endif //RAYLIB_LUA_IMGUI_B2RLMATH_H