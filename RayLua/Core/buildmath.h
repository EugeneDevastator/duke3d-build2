//
// Created by Eugene
// Build2 helper methods
//

#ifndef RAYLIB_LUA_IMGUI_BUILDMATH_H
#define RAYLIB_LUA_IMGUI_BUILDMATH_H
#include "monoclip.h"


#define epsilon 0.0000001f
#define epsilond 0.000001

static const point3d BBRIGHT = {1,0,0};
static const point3d BBFORWARD = {0,1,0};
static const point3d BBDOWN = {0,0,1};

#define BPXY(p3d) {p3d.x, p3d.y}


static inline float vlen(point3d *p) {
    return sqrtf(p->x * p->x + p->y * p->y + p->z * p->z);
}
static inline float vlensquared(point3d *p) {
    return (p->x * p->x + p->y * p->y + p->z * p->z);
}
static inline float bmathdistsqrp3d(point3d p1, point3d p2) {
    float dx = p1.x-p2.x;
    float dy = p1.y-p2.y;
    float dz = p1.z-p2.z;
    return (dx*dx + dy*dy + dz*dz);
}
static inline float bmathdistsqrp2d(point2d p1, point2d p2) {
    float dx = p1.x-p2.x;
    float dy = p1.y-p2.y;
    return (dx*dx + dy*dy);
}
/*
*typedef struct {
point3d p, r, d, f; // pos right down forward.
} transform;
*/

static inline point3d subtract(point3d a, point3d b) {
    point3d p;
    p.x = a.x - b.x;
    p.y = a.y - b.y;
    p.z = a.z - b.z;
    return p;
}
static inline void addto(point3d *a, const point3d b) {
    a->x += b.x;
    a->y += b.y;
    a->z += b.z;
}
static inline point3d sump3(point3d a, point3d b) {
point3d r;
    r.x=a.x+b.x;
    r.y=a.y+b.y;
    r.z=a.z+b.z;
    return r;
}

static inline void rot90cwz(point3d *a) {
    float t = a->x;
    a->x = a->y;
    a->y = -t;
}
static void vscalar(point3d *p, float s) {
    p->x*=s;
    p->y*=s;
    p->z*=s;
}
static point3d scaled(const point3d pt, float s) {
point3d p = pt;
    p.x*=s;
    p.y*=s;
    p.z*=s;
    return p;
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
static inline bool issamexy(point3d a, point3d b) {
    return ((fabsf(a.x - b.x) + fabsf(a.y-b.y)) < epsilon);
}
static inline bool issamexyd(dpoint3d a, dpoint3d b) {
    return ((fabs(a.x - b.x) + fabs(a.y-b.y)) < epsilond);
}
static inline point3d local_to_world_point(point3d local_pos, transform *tr) {
    point3d world;
    world.x = tr->p.x + local_pos.x * tr->r.x + local_pos.y * tr->d.x + local_pos.z * tr->f.x;
    world.y = tr->p.y + local_pos.x * tr->r.y + local_pos.y * tr->d.y + local_pos.z * tr->f.y;
    world.z = tr->p.z + local_pos.x * tr->r.z + local_pos.y * tr->d.z + local_pos.z * tr->f.z;

    return world;
}static inline dpoint3d local_to_world_dpoint(dpoint3d local_pos, transform *tr) {
    dpoint3d world;
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


static inline transform world_to_local_transform_p(const transform tin, transform *totr) {
    // World -> camera space (using ctin)
    transform ret;
    ret.p = world_to_local_point(tin.p, totr);
    ret.r = world_to_local_vec(tin.r, totr);
    ret.d = world_to_local_vec(tin.d, totr);
    ret.f = world_to_local_vec(tin.f, totr);
    return ret;
}
static inline transform local_to_world_transform_p(const transform tin, transform *tfrom) {
    // World -> camera space (using ctin)
    transform ret;
    ret.p = local_to_world_point(tin.p, tfrom);
    ret.r = local_to_world_vec(tin.r, tfrom);
    ret.d = local_to_world_vec(tin.d, tfrom);
    ret.f = local_to_world_vec(tin.f, tfrom);
    return ret;
}
static inline void wccw_transform_trp(point3d *pinout, transform *ctin, transform *ctout) {
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


// is not power of two
static inline bool isnpot(int n) {
    return n <= 0 || (n & (n - 1)) != 0;
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
static inline void wccw_transform_dirp(point3d *dir, transform *ctin, transform *ctout) {
    // Transform direction vector (no translation)
    double cx = dir->x * ctin->r.x + dir->y * ctin->r.y + dir->z * ctin->r.z;
    double cy = dir->x * ctin->d.x + dir->y * ctin->d.y + dir->z * ctin->d.z;
    double cz = dir->x * ctin->f.x + dir->y * ctin->f.y + dir->z * ctin->f.z;

    dir->x = cx * ctout->r.x + cy * ctout->d.x + cz * ctout->f.x;
    dir->y = cx * ctout->r.y + cy * ctout->d.y + cz * ctout->f.y;
    dir->z = cx * ctout->r.z + cy * ctout->d.z + cz * ctout->f.z;
}
static inline void wccw_transform_full(transform *tr, transform *ctin, transform *ctout) {
    wccw_transform_trp(&tr->p, ctin,ctout);
    wccw_transform_dirp(&tr->f,ctin,ctout);
    wccw_transform_dirp(&tr->r,ctin,ctout);
    wccw_transform_dirp(&tr->d,ctin,ctout);
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
// --------------------- quats
typedef struct {
    float x, y, z, w;
} quat;

static inline quat quat_identity() {
    quat q = {0.0f, 0.0f, 0.0f, 1.0f};
    return q;
}

static inline quat quat_from_axis_angle(point3d axis, float angleDeg) {
    float angleRad = angleDeg * 0.017453292519943295f; // deg to rad
    float halfAngle = angleRad * 0.5f;
    float s = sinf(halfAngle);

    quat q;
    q.x = axis.x * s;
    q.y = axis.y * s;
    q.z = axis.z * s;
    q.w = cosf(halfAngle);
    return q;
}

static inline quat quat_multiply(quat a, quat b) {
    quat result;
    result.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
    result.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    result.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
    result.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
    return result;
}

static inline point3d quat_rotate_point(quat q, point3d p) {
    float qx2 = q.x * 2.0f;
    float qy2 = q.y * 2.0f;
    float qz2 = q.z * 2.0f;
    float qxqx2 = q.x * qx2;
    float qyqy2 = q.y * qy2;
    float qzqz2 = q.z * qz2;
    float qxqy2 = q.x * qy2;
    float qxqz2 = q.x * qz2;
    float qyqz2 = q.y * qz2;
    float qwqx2 = q.w * qx2;
    float qwqy2 = q.w * qy2;
    float qwqz2 = q.w * qz2;

    point3d result;
    result.x = p.x * (1.0f - qyqy2 - qzqz2) + p.y * (qxqy2 - qwqz2) + p.z * (qxqz2 + qwqy2);
    result.y = p.x * (qxqy2 + qwqz2) + p.y * (1.0f - qxqx2 - qzqz2) + p.z * (qyqz2 - qwqx2);
    result.z = p.x * (qxqz2 - qwqy2) + p.y * (qyqz2 + qwqx2) + p.z * (1.0f - qxqx2 - qyqy2);
    return result;
}

static inline quat quat_slerp(quat a, quat b, float t) {
    float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

    if (dot < 0.0f) {
        b.x = -b.x; b.y = -b.y; b.z = -b.z; b.w = -b.w;
        dot = -dot;
    }

    if (dot > 0.9995f) {
        quat result;
        result.x = a.x + t * (b.x - a.x);
        result.y = a.y + t * (b.y - a.y);
        result.z = a.z + t * (b.z - a.z);
        result.w = a.w + t * (b.w - a.w);

        float len = sqrtf(result.x * result.x + result.y * result.y + result.z * result.z + result.w * result.w);
        result.x /= len; result.y /= len; result.z /= len; result.w /= len;
        return result;
    }

    float theta = acosf(dot);
    float sinTheta = sinf(theta);
    float wa = sinf((1.0f - t) * theta) / sinTheta;
    float wb = sinf(t * theta) / sinTheta;

    quat result;
    result.x = wa * a.x + wb * b.x;
    result.y = wa * a.y + wb * b.y;
    result.z = wa * a.z + wb * b.z;
    result.w = wa * a.w + wb * b.w;
    return result;
}

static inline void quat_to_transform(quat q, transform *tr) {
    float x2 = q.x * 2.0f;
    float y2 = q.y * 2.0f;
    float z2 = q.z * 2.0f;
    float xx2 = q.x * x2;
    float yy2 = q.y * y2;
    float zz2 = q.z * z2;
    float xy2 = q.x * y2;
    float xz2 = q.x * z2;
    float yz2 = q.y * z2;
    float wx2 = q.w * x2;
    float wy2 = q.w * y2;
    float wz2 = q.w * z2;

    tr->r.x = 1.0f - yy2 - zz2;
    tr->r.y = xy2 + wz2;
    tr->r.z = xz2 - wy2;

    tr->d.x = xy2 - wz2;
    tr->d.y = 1.0f - xx2 - zz2;
    tr->d.z = yz2 + wx2;

    tr->f.x = xz2 + wy2;
    tr->f.y = yz2 - wx2;
    tr->f.z = 1.0f - xx2 - yy2;
}

static inline quat quat_from_transform(transform *tr) {
    float trace = tr->r.x + tr->d.y + tr->f.z;
    quat q;

    if (trace > 0.0f) {
        float s = sqrtf(trace + 1.0f) * 2.0f;
        q.w = 0.25f * s;
        q.x = (tr->d.z - tr->f.y) / s;
        q.y = (tr->f.x - tr->r.z) / s;
        q.z = (tr->r.y - tr->d.x) / s;
    } else if (tr->r.x > tr->d.y && tr->r.x > tr->f.z) {
        float s = sqrtf(1.0f + tr->r.x - tr->d.y - tr->f.z) * 2.0f;
        q.w = (tr->d.z - tr->f.y) / s;
        q.x = 0.25f * s;
        q.y = (tr->d.x + tr->r.y) / s;
        q.z = (tr->f.x + tr->r.z) / s;
    } else if (tr->d.y > tr->f.z) {
        float s = sqrtf(1.0f + tr->d.y - tr->r.x - tr->f.z) * 2.0f;
        q.w = (tr->f.x - tr->r.z) / s;
        q.x = (tr->d.x + tr->r.y) / s;
        q.y = 0.25f * s;
        q.z = (tr->f.y + tr->d.z) / s;
    } else {
        float s = sqrtf(1.0f + tr->f.z - tr->r.x - tr->d.y) * 2.0f;
        q.w = (tr->r.y - tr->d.x) / s;
        q.x = (tr->f.x + tr->r.z) / s;
        q.y = (tr->f.y + tr->d.z) / s;
        q.z = 0.25f * s;
    }
    return q;
}
static inline void qrotaxis(transform *tr, point3d axis, float angleDeg) {
    point3d normAxis = normalizep3(axis);
    quat rotation = quat_from_axis_angle(normAxis, angleDeg);

    tr->r = quat_rotate_point(rotation, tr->r);
    tr->d = quat_rotate_point(rotation, tr->d);
    tr->f = quat_rotate_point(rotation, tr->f);
}

static inline void qrotwhole(transform *tr, point3d axis, point3d axisorigin, float angleDeg) {
    point3d normAxis = normalizep3(axis);
    quat rotation = quat_from_axis_angle(normAxis, angleDeg);

    point3d offset = subtract(tr->p, axisorigin);
    point3d rotatedOffset = quat_rotate_point(rotation, offset);
    tr->p = sump3(axisorigin, rotatedOffset);

    tr->r = quat_rotate_point(rotation, tr->r);
    tr->d = quat_rotate_point(rotation, tr->d);
    tr->f = quat_rotate_point(rotation, tr->f);
}

static inline void qrotslerp(transform *trsubj, transform *trto, float normt) {
    quat qsubj = quat_from_transform(trsubj);
    quat qto = quat_from_transform(trto);
    quat result = quat_slerp(qsubj, qto, normt);

    // Preserve original axis lengths
    float rlen = vlen(&trsubj->r);
    float dlen = vlen(&trsubj->d);
    float flen = vlen(&trsubj->f);

    quat_to_transform(result, trsubj);

    scalardivv(&trsubj->r, vlen(&trsubj->r));
    scalardivv(&trsubj->d, vlen(&trsubj->d));
    scalardivv(&trsubj->f, vlen(&trsubj->f));

    trsubj->r.x *= rlen; trsubj->r.y *= rlen; trsubj->r.z *= rlen;
    trsubj->d.x *= dlen; trsubj->d.y *= dlen; trsubj->d.z *= dlen;
    trsubj->f.x *= flen; trsubj->f.y *= flen; trsubj->f.z *= flen;
}

static inline void qrotlookat(transform *trsubj, point3d to, float normt) {
    point3d dir = subtract(to, trsubj->p);
    float len = vlen(&dir);
    if (len < epsilon) return;

    scalardivv(&dir, len);

    point3d up = {0.0f, 0.0f, 1.0f};
    point3d right = crossp3(up, dir);
    float rightLen = vlen(&right);
    if (rightLen < epsilon) {
        up.x = 1.0f; up.y = 0.0f; up.z = 0.0f;
        right = crossp3(up, dir);
        rightLen = vlen(&right);
    }
    scalardivv(&right, rightLen);
    up = crossp3(dir, right);

    // Preserve original axis lengths
    float rlen = vlen(&trsubj->r);
    float dlen = vlen(&trsubj->d);
    float flen = vlen(&trsubj->f);

    transform target = *trsubj;
    target.f.x = dir.x * flen; target.f.y = dir.y * flen; target.f.z = dir.z * flen;
    target.r.x = right.x * rlen; target.r.y = right.y * rlen; target.r.z = right.z * rlen;
    target.d.x = up.x * dlen; target.d.y = up.y * dlen; target.d.z = up.z * dlen;

    qrotslerp(trsubj, &target, normt);
}

#endif //RAYLIB_LUA_IMGUI_BUILDMATH_H
