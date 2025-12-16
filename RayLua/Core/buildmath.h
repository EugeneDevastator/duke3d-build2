//
// Created by omnis on 12/8/2025.
//

#ifndef RAYLIB_LUA_IMGUI_BUILDMATH_H
#define RAYLIB_LUA_IMGUI_BUILDMATH_H
#include "mapcore.h"
#include "monoclip.h"
//
// Created by omnis on 12/8/2025.
//

#include "buildmath.h"

static inline float vlen(point3d *p) {
    return sqrtf(p->x * p->x + p->y * p->y + p->z * p->z);
}

static inline void scalardivv(point3d *pt, float diver) {
    pt->x /= diver; pt->y /= diver; pt->z /= diver;
}

static inline void normalize_transform(transform *tr) {
    // Normalize all axes
    float flen = vlen(&tr->f);
    if (flen > 0.0001f) scalardivv(&tr->f, flen);

    flen = vlen(&tr->r);
    if (flen > 0.0001f) scalardivv(&tr->r, flen);

    flen = vlen(&tr->d);
    if (flen > 0.0001f) scalardivv(&tr->d, flen);
}

static inline point3d local_to_world_point(point3d local_pos, transform *tr) {
    point3d world;
    world.x = tr->p.x + local_pos.x * tr->r.x + local_pos.y * tr->d.x + local_pos.z * tr->f.x;
    world.y = tr->p.y + local_pos.x * tr->r.y + local_pos.y * tr->d.y + local_pos.z * tr->f.y;
    world.z = tr->p.z + local_pos.x * tr->r.z + local_pos.y * tr->d.z + local_pos.z * tr->f.z;

    return world;
}

static inline point3d world_to_local_point(point3d world_pos, transform *tr) {
    float dx = world_pos.x - tr->p.x;
    float dy = world_pos.y - tr->p.y;
    float dz = world_pos.z - tr->p.z;

    point3d local;
    local.x = dx * tr->r.x + dy * tr->r.y + dz * tr->r.z;
    local.y = dx * tr->d.x + dy * tr->d.y + dz * tr->d.z;
    local.z = dx * tr->f.x + dy * tr->f.y + dz * tr->f.z;

    return local;
}
static inline dpoint3d world_to_local_pointd(dpoint3d world_pos, transform *tr) {
    float dx = world_pos.x - tr->p.x;
    float dy = world_pos.y - tr->p.y;
    float dz = world_pos.z - tr->p.z;

    dpoint3d local;
    local.x = dx * tr->r.x + dy * tr->r.y + dz * tr->r.z;
    local.y = dx * tr->d.x + dy * tr->d.y + dz * tr->d.z;
    local.z = dx * tr->f.x + dy * tr->f.y + dz * tr->f.z;

    return local;
}
static inline point3d local_to_world_vec(point3d local_vec, transform *tr) {
    point3d world;
    world.x = local_vec.x * tr->r.x + local_vec.y * tr->d.x + local_vec.z * tr->f.x;
    world.y = local_vec.x * tr->r.y + local_vec.y * tr->d.y + local_vec.z * tr->f.y;
    world.z = local_vec.x * tr->r.z + local_vec.y * tr->d.z + local_vec.z * tr->f.z;

    return world;
}

static inline point3d world_to_local_vec(point3d world_vec, transform *tr) {
    point3d local;
    local.x = world_vec.x * tr->r.x + world_vec.y * tr->r.y + world_vec.z * tr->r.z;  // right
    local.y = world_vec.x * tr->d.x + world_vec.y * tr->d.y + world_vec.z * tr->d.z;  // forward
    local.z = world_vec.x * tr->f.x + world_vec.y * tr->f.y + world_vec.z * tr->f.z;  // down

    return local;
}
static inline dpoint3d world_to_local_vecd(dpoint3d world_vec, transform *tr) {
    dpoint3d local;
    local.x = world_vec.x * tr->r.x + world_vec.y * tr->r.y + world_vec.z * tr->r.z;  // right
    local.y = world_vec.x * tr->d.x + world_vec.y * tr->d.y + world_vec.z * tr->d.z;  // forward
    local.z = world_vec.x * tr->f.x + world_vec.y * tr->f.y + world_vec.z * tr->f.z;  // down

    return local;
}
static inline void world_to_cam(double wx, double wy, double wz, cam_t *ctin, double *cx, double *cy, double *cz) {
    double dx = wx - ctin->p.x;
    double dy = wy - ctin->p.y;
    double dz = wz - ctin->p.z;

    *cx = dx * ctin->r.x + dy * ctin->r.y + dz * ctin->r.z;
    *cy = dx * ctin->d.x + dy * ctin->d.y + dz * ctin->d.z;
    *cz = dx * ctin->f.x + dy * ctin->f.y + dz * ctin->f.z;
}
static inline void wccw_transform_tr(dpoint3d *pinout, transform *ctin, transform *ctout) {
    // World -> camera space (using ctin)
    double dx = pinout->x - ctin->p.x;
    double dy = pinout->y - ctin->p.y;
    double dz = pinout->z - ctin->p.z;

    double cx = dx * ctin->r.x + dy * ctin->r.y + dz * ctin->r.z;
    double cy = dx * ctin->d.x + dy * ctin->d.y + dz * ctin->d.z;
    double cz = dx * ctin->f.x + dy * ctin->f.y + dz * ctin->f.z;

    // Camera space -> world (using ctout)
    pinout->x = cx * ctout->r.x + cy * ctout->d.x + cz * ctout->f.x + ctout->p.x;
    pinout->y = cx * ctout->r.y + cy * ctout->d.y + cz * ctout->f.y + ctout->p.y;
    pinout->z = cx * ctout->r.z + cy * ctout->d.z + cz * ctout->f.z + ctout->p.z;
}
static inline void wccw_transform(dpoint3d *pinout, cam_t *ctin, cam_t *ctout) {
    wccw_transform_tr(pinout, &ctin->tr,&ctout->tr);
}

static inline void wccw_transform_dir(dpoint3d *dir, cam_t *ctin, cam_t *ctout) {
    // Transform direction vector (no translation)
    double cx = dir->x * ctin->r.x + dir->y * ctin->r.y + dir->z * ctin->r.z;
    double cy = dir->x * ctin->d.x + dir->y * ctin->d.y + dir->z * ctin->d.z;
    double cz = dir->x * ctin->f.x + dir->y * ctin->f.y + dir->z * ctin->f.z;

    dir->x = cx * ctout->r.x + cy * ctout->d.x + cz * ctout->f.x;
    dir->y = cx * ctout->r.y + cy * ctout->d.y + cz * ctout->f.y;
    dir->z = cx * ctout->r.z + cy * ctout->d.z + cz * ctout->f.z;
}

static inline void mp_to_world(double sx, double sy, bdrawctx *b, double *wx, double *wy, double *wz, cam_t *cam) {
    double denom = (b->gouvmat[0] * sx + b->gouvmat[3] * sy + b->gouvmat[6]) * cam->h.z;

    if (fabs(denom) < 1e-10) {
        *wx = cam->p.x;
        *wy = cam->p.y;
        *wz = cam->p.z;
        return;
    }

    double depth = 1.0 / denom;
    double dx = sx - cam->h.x;
    double dy = sy - cam->h.y;

    *wx = (dx * cam->r.x + dy * cam->d.x + cam->h.z * cam->f.x) * depth + cam->p.x;
    *wy = (dx * cam->r.y + dy * cam->d.y + cam->h.z * cam->f.y) * depth + cam->p.y;
    *wz = (dx * cam->r.z + dy * cam->d.z + cam->h.z * cam->f.z) * depth + cam->p.z;
}
static inline void portal_xform_world_at_z(double *x, double *y, double ref_z, bdrawctx *b) {
    dpoint3d p;
    p.x = *x;
    p.y = *y;
    p.z = ref_z;
    wccw_transform(&p, &b->cam, &b->orcam);
    *x = p.x;
    *y = p.y;
}

static inline dpoint3d gettrianglenorm(dpoint3d p0, dpoint3d p1, dpoint3d p2) {
    dpoint3d v1, v2, normal;

    // Calculate edge vectors
    v1.x = p1.x - p0.x;
    v1.y = p1.y - p0.y;
    v1.z = p1.z - p0.z;

    v2.x = p2.x - p0.x;
    v2.y = p2.y - p0.y;
    v2.z = p2.z - p0.z;

    // Cross product v1 × v2
    normal.x = v1.y * v2.z - v1.z * v2.y;
    normal.y = v1.z * v2.x - v1.x * v2.z;
    normal.z = v1.x * v2.y - v1.y * v2.x;

    // Normalize
    float length = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    if (length > 0.0f) {
        normal.x /= length;
        normal.y /= length;
        normal.z /= length;
    }

    return normal;
}

static inline float dotdp3(dpoint3d a, dpoint3d b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
static inline float dotp3(point3d a, point3d b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline dpoint3d crossdp3(dpoint3d a, dpoint3d b) {
    dpoint3d result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}
static inline point3d crossp3(point3d a, point3d b) {
    point3d result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}
static inline dpoint3d normalizedp3(dpoint3d v) {
    dpoint3d result;
    float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (length > 0.0f) {
        result.x = v.x / length;
        result.y = v.y / length;
        result.z = v.z / length;
    } else {
        result.x = result.y = result.z = 0.0f;
    }
    return result;
}
static inline point3d normalizep3(point3d v) {
    point3d result;
    float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (length > 0.0f) {
        result.x = v.x / length;
        result.y = v.y / length;
        result.z = v.z / length;
    } else {
        result.x = result.y = result.z = 0.0f;
    }
    return result;
}

static inline bool is_transform_flipped(transform* tr) {
    // Calculate determinant of the 3x3 rotation matrix
    // det = r·(d×f)
    point3d cross = crossp3(tr->d, tr->f);
    float det = dotp3(tr->r, cross);
    return det < 0.0f;
}
#endif //RAYLIB_LUA_IMGUI_BUILDMATH_H
