//
// Created by Eugene
// Build2 helper methods
//

#ifndef RAYLIB_LUA_IMGUI_BUILDMATH_H
#define RAYLIB_LUA_IMGUI_BUILDMATH_H
#include "monoclip.h"


#define epsilon 0.0000001f
#define epsilond 0.000001

static const point3d BBRIGHT = {1, 0, 0};
static const point3d BBFORWARD = {0, 1, 0};
static const point3d BBDOWN = {0, 0, 1};
static const point3d BBPZERO = {0, 0, 0};
static const point3d BBPONE = {1, 1, 1};

#define BPXY(p3d) {p3d.x, p3d.y}
// Convention tr = transform, meaning data structure, not operation or anything else.
// transfromation is always written as 'transfrom'
#if 1 // ===================== SCALARS and UNARY ==================
static inline bool p3_issamexy(point3d a, point3d b) {
	return ((fabsf(a.x - b.x) + fabsf(a.y - b.y)) < epsilon);
}

static inline bool p3d_issamexy(dpoint3d a, dpoint3d b) {
	return ((fabs(a.x - b.x) + fabs(a.y - b.y)) < epsilond);
}

static inline float p3_length(point3d *p) {
	return sqrtf(p->x * p->x + p->y * p->y + p->z * p->z);
}

static inline float p3_length_squared(point3d *p) {
	return (p->x * p->x + p->y * p->y + p->z * p->z);
}

static inline void p3_rot90_cwz(point3d *a) {
	float t = a->x;
	a->x = a->y;
	a->y = -t;
}

static inline point3d p3_rotated_90_cwz(point3d a) {
	float t = a.x;
	a.x = a.y;
	a.y = -t;
	return a;
}

static inline point2d p3xy(point3d a) {
	point2d outp = {a.x, a.y};
	return outp;
}

static inline point2d p3dxy(dpoint3d a) {
	point2d outp = {(float) a.x, (float) a.y};
	return outp;
}

static inline void p3_scalar_div(point3d *pt, float diver) {
	pt->x /= diver;
	pt->y /= diver;
	pt->z /= diver;
}
#endif

#if 1 // =================== BINARY PT-PT operations ===================
static inline point3d p3_diff(point3d a, point3d b) {
	point3d p;
	p.x = a.x - b.x;
	p.y = a.y - b.y;
	p.z = a.z - b.z;
	return p;
}

static inline void p3_addto(point3d *awrite, const point3d b) {
	awrite->x += b.x;
	awrite->y += b.y;
	awrite->z += b.z;
}
static inline point3d p3_sum(const point3d a, const point3d b) {
	point3d p;
	p.x = a.x + b.x;
	p.y = a.y + b.y;
	p.z = a.z + b.z;
	return p;
}

static inline dpoint3d p3_todbl(const point3d b) {
	dpoint3d awrite;
	awrite.x = b.x;
	awrite.y = b.y;
	awrite.z = b.z;
	return awrite;
}

static inline dpoint3d p3d_sum(const dpoint3d a, const dpoint3d b) {
	dpoint3d p;
	p.x = a.x + b.x;
	p.y = a.y + b.y;
	p.z = a.z + b.z;
	return p;
}
static inline dpoint3d p3d_inv(const point3d a) {
	dpoint3d awrite;
	awrite.x = -a.x;
	awrite.y += -a.y;
	awrite.z += -a.z;
	return awrite;
}
static inline point3d p3_inv(const point3d a) {
	point3d awrite;
	awrite.x = -a.x;
	awrite.y = -a.y;
	awrite.z = -a.z;
	return awrite;
}
static void p3_scalar_mul(point3d *p, float s) {
	p->x *= s;
	p->y *= s;
	p->z *= s;
}

static point3d p3_scalar_mul_of(const point3d pt, float s) {
	point3d p = pt;
	p.x *= s;
	p.y *= s;
	p.z *= s;
	return p;
}

// inverse sub
static inline point3d p3_make_vector(point3d origin, point3d endpoint) {
	point3d p;
	p.x = endpoint.x - origin.x;
	p.y = endpoint.y - origin.y;
	p.z = endpoint.z - origin.z;
	return p;
}

static inline float p3_distance(point3d p1, point3d p2) {
	float dx = p1.x - p2.x;
	float dy = p1.y - p2.y;
	float dz = p1.z - p2.z;
	return (dx * dx + dy * dy + dz * dz);
}

static inline float p2_distance(point2d p1, point2d p2) {
	float dx = p1.x - p2.x;
	float dy = p1.y - p2.y;
	return (dx * dx + dy * dy);
}

static inline point3d p3sum(point3d a, point3d b) {
	point3d r;
	r.x = a.x + b.x;
	r.y = a.y + b.y;
	r.z = a.z + b.z;
	return r;
}


static inline float p3d_dot(dpoint3d a, dpoint3d b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline float p3_dot(point3d a, point3d b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline dpoint3d p3d_cross(dpoint3d a, dpoint3d b) {
	dpoint3d result;
	result.x = a.y * b.z - a.z * b.y;
	result.y = a.z * b.x - a.x * b.z;
	result.z = a.x * b.y - a.y * b.x;
	return result;
}

static inline point3d p3_cross(point3d a, point3d b) {
	point3d result;
	result.x = a.y * b.z - a.z * b.y;
	result.y = a.z * b.x - a.x * b.z;
	result.z = a.x * b.y - a.y * b.x;
	return result;
}

static inline dpoint3d p3d_normalized(dpoint3d v) {
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

static inline point3d p3_normalized(point3d v) {
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

#endif

#if 1 // ===================== TRANSFORMS ==========================

static inline bool tr_is_flipped(transform *tr) {
	// Calculate determinant of the 3x3 rotation matrix
	// det = r·(d×f)
	point3d cross = p3_cross(tr->d, tr->f);
	float det = p3_dot(tr->r, cross);
	return det < 0.0f;
}

static inline transform tr_xyplanar_from_forward(point3d forwardvec) {
	forwardvec.z = 0;
	point3d newf = p3_normalized(forwardvec);
	transform tr;
	tr.f = newf;
	tr.r = p3_rotated_90_cwz(newf);
	tr.d = BBDOWN;
	tr.p = BBPZERO;
	return tr;
}

static inline void tr_normalize(transform *tr) {
	// Normalize all axes
	float flen = p3_length(&tr->f);
	if (flen > 0.0001f) p3_scalar_div(&tr->f, flen);

	flen = p3_length(&tr->r);
	if (flen > 0.0001f) p3_scalar_div(&tr->r, flen);

	flen = p3_length(&tr->d);
	if (flen > 0.0001f) p3_scalar_div(&tr->d, flen);
}

static inline point3d p3_local_to_world(point3d local_pos, transform *tr) {
	point3d world;
	world.x = tr->p.x + local_pos.x * tr->r.x + local_pos.y * tr->d.x + local_pos.z * tr->f.x;
	world.y = tr->p.y + local_pos.x * tr->r.y + local_pos.y * tr->d.y + local_pos.z * tr->f.y;
	world.z = tr->p.z + local_pos.x * tr->r.z + local_pos.y * tr->d.z + local_pos.z * tr->f.z;

	return world;
}

static inline dpoint3d p3d_local_to_world(dpoint3d local_pos, transform *tr) {
	dpoint3d world;
	world.x = tr->p.x + local_pos.x * tr->r.x + local_pos.y * tr->d.x + local_pos.z * tr->f.x;
	world.y = tr->p.y + local_pos.x * tr->r.y + local_pos.y * tr->d.y + local_pos.z * tr->f.y;
	world.z = tr->p.z + local_pos.x * tr->r.z + local_pos.y * tr->d.z + local_pos.z * tr->f.z;

	return world;
}

static inline point3d p3_world_to_local(point3d world_pos, transform *tr) {
	float dx = world_pos.x - tr->p.x;
	float dy = world_pos.y - tr->p.y;
	float dz = world_pos.z - tr->p.z;

	point3d local;
	local.x = dx * tr->r.x + dy * tr->r.y + dz * tr->r.z;
	local.y = dx * tr->d.x + dy * tr->d.y + dz * tr->d.z;
	local.z = dx * tr->f.x + dy * tr->f.y + dz * tr->f.z;

	return local;
}

static inline dpoint3d p3d_world_to_local(dpoint3d world_pos, transform *tr) {
	float dx = world_pos.x - tr->p.x;
	float dy = world_pos.y - tr->p.y;
	float dz = world_pos.z - tr->p.z;

	dpoint3d local;
	local.x = dx * tr->r.x + dy * tr->r.y + dz * tr->r.z;
	local.y = dx * tr->d.x + dy * tr->d.y + dz * tr->d.z;
	local.z = dx * tr->f.x + dy * tr->f.y + dz * tr->f.z;

	return local;
}

static inline point3d p3d_local_to_world_vector(point3d local_vec, transform *space) {
	point3d world;
	world.x = local_vec.x * space->r.x + local_vec.y * space->d.x + local_vec.z * space->f.x;
	world.y = local_vec.x * space->r.y + local_vec.y * space->d.y + local_vec.z * space->f.y;
	world.z = local_vec.x * space->r.z + local_vec.y * space->d.z + local_vec.z * space->f.z;

	return world;
}

static inline point3d p3_world_to_local_vec(point3d world_vec, transform *space) {
	point3d local;
	local.x = world_vec.x * space->r.x + world_vec.y * space->r.y + world_vec.z * space->r.z; // right
	local.y = world_vec.x * space->d.x + world_vec.y * space->d.y + world_vec.z * space->d.z; // forward
	local.z = world_vec.x * space->f.x + world_vec.y * space->f.y + world_vec.z * space->f.z; // down

	return local;
}

static inline dpoint3d p3d_world_to_local_vec(dpoint3d world_vec, transform *space) {
	dpoint3d local;
	local.x = world_vec.x * space->r.x + world_vec.y * space->r.y + world_vec.z * space->r.z; // right
	local.y = world_vec.x * space->d.x + world_vec.y * space->d.y + world_vec.z * space->d.z; // forward
	local.z = world_vec.x * space->f.x + world_vec.y * space->f.y + world_vec.z * space->f.z; // down

	return local;
}
#endif
#if 1 // ====================== TRANSFORMS ** 2 =============================

static inline void tr_transform_wccw(dpoint3d *pinout, transform *ctin, transform *ctout) {
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


static inline transform tr_world_to_local(const transform transformed, transform *space) {
	// World -> camera space (using ctin)
	transform ret;
	ret.p = p3_world_to_local(transformed.p, space);
	ret.r = p3_world_to_local_vec(transformed.r, space);
	ret.d = p3_world_to_local_vec(transformed.d, space);
	ret.f = p3_world_to_local_vec(transformed.f, space);
	return ret;
}

static inline transform tr_local_to_world(const transform tin, transform *tfrom) {
	// World -> camera space (using ctin)
	transform ret;
	ret.p = p3_local_to_world(tin.p, tfrom);
	ret.r = p3d_local_to_world_vector(tin.r, tfrom);
	ret.d = p3d_local_to_world_vector(tin.d, tfrom);
	ret.f = p3d_local_to_world_vector(tin.f, tfrom);
	return ret;
}

static inline void p3_transform_wccw(point3d *pinout, transform *camspace, transform *outspace) {
	// World -> camera space (using camspace)
	double dx = pinout->x - camspace->p.x;
	double dy = pinout->y - camspace->p.y;
	double dz = pinout->z - camspace->p.z;

	double cx = dx * camspace->r.x + dy * camspace->r.y + dz * camspace->r.z;
	double cy = dx * camspace->d.x + dy * camspace->d.y + dz * camspace->d.z;
	double cz = dx * camspace->f.x + dy * camspace->f.y + dz * camspace->f.z;

	// Camera space -> world (using outspace)
	pinout->x = cx * outspace->r.x + cy * outspace->d.x + cz * outspace->f.x + outspace->p.x;
	pinout->y = cx * outspace->r.y + cy * outspace->d.y + cz * outspace->f.y + outspace->p.y;
	pinout->z = cx * outspace->r.z + cy * outspace->d.z + cz * outspace->f.z + outspace->p.z;
}

static inline void p3d_transform_wccw(dpoint3d *pinout, cam_t *camspace, cam_t *outspace) {
	tr_transform_wccw(pinout, &camspace->tr, &outspace->tr);
}


// is not power of two
static inline bool isnpot(int n) {
	return n <= 0 || (n & (n - 1)) != 0;
}

static inline void transform_wccw_vec(dpoint3d *dir, cam_t *ctin, cam_t *ctout) {
	// Transform direction vector (no translation)
	double cx = dir->x * ctin->r.x + dir->y * ctin->r.y + dir->z * ctin->r.z;
	double cy = dir->x * ctin->d.x + dir->y * ctin->d.y + dir->z * ctin->d.z;
	double cz = dir->x * ctin->f.x + dir->y * ctin->f.y + dir->z * ctin->f.z;

	dir->x = cx * ctout->r.x + cy * ctout->d.x + cz * ctout->f.x;
	dir->y = cx * ctout->r.y + cy * ctout->d.y + cz * ctout->f.y;
	dir->z = cx * ctout->r.z + cy * ctout->d.z + cz * ctout->f.z;
}

static inline void p3_transform_wccw_vec(point3d *dir, transform *camspace, transform *outspace) {
	// Transform direction vector (no translation)
	double cx = dir->x * camspace->r.x + dir->y * camspace->r.y + dir->z * camspace->r.z;
	double cy = dir->x * camspace->d.x + dir->y * camspace->d.y + dir->z * camspace->d.z;
	double cz = dir->x * camspace->f.x + dir->y * camspace->f.y + dir->z * camspace->f.z;

	dir->x = cx * outspace->r.x + cy * outspace->d.x + cz * outspace->f.x;
	dir->y = cx * outspace->r.y + cy * outspace->d.y + cz * outspace->f.y;
	dir->z = cx * outspace->r.z + cy * outspace->d.z + cz * outspace->f.z;
}

static inline void wccw_transform_full(transform *tr, transform *camspace, transform *outspace) {
	p3_transform_wccw(&tr->p, camspace, outspace);
	p3_transform_wccw_vec(&tr->f, camspace, outspace);
	p3_transform_wccw_vec(&tr->r, camspace, outspace);
	p3_transform_wccw_vec(&tr->d, camspace, outspace);
}
#endif

#if 1 // =============== MONO PLANE CONVERSIONS ====================
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

#endif
#if 1 // ======================== MISC OPS ===========================
// dont delete for now.
static inline void portal_xform_world_at_z(double *x, double *y, double ref_z, bdrawctx *b) {
	dpoint3d p;
	p.x = *x;
	p.y = *y;
	p.z = ref_z;
	p3d_transform_wccw(&p, &b->cam, &b->orcam);
	*x = p.x;
	*y = p.y;
}

static inline dpoint3d p3d_normal_of_triangle(dpoint3d p0, dpoint3d p1, dpoint3d p2) {
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

#endif
#if 1 // ======================= QUATERNIONS ======================


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
		b.x = -b.x;
		b.y = -b.y;
		b.z = -b.z;
		b.w = -b.w;
		dot = -dot;
	}

	if (dot > 0.9995f) {
		quat result;
		result.x = a.x + t * (b.x - a.x);
		result.y = a.y + t * (b.y - a.y);
		result.z = a.z + t * (b.z - a.z);
		result.w = a.w + t * (b.w - a.w);

		float len = sqrtf(result.x * result.x + result.y * result.y + result.z * result.z + result.w * result.w);
		result.x /= len;
		result.y /= len;
		result.z /= len;
		result.w /= len;
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

static inline void quat_to_tr(quat q, transform *tr) {
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

static inline quat quat_from_tr(transform *tr) {
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

static inline void tr_quat_rotate_on_axis(transform *tr, point3d axis, float angleDeg) {
	point3d normAxis = p3_normalized(axis);
	quat rotation = quat_from_axis_angle(normAxis, angleDeg);

	tr->r = quat_rotate_point(rotation, tr->r);
	tr->d = quat_rotate_point(rotation, tr->d);
	tr->f = quat_rotate_point(rotation, tr->f);
}

static inline void tr_quat_rotate_axis_and_point(transform *tr, point3d axis, point3d axisorigin, float angleDeg) {
	point3d normAxis = p3_normalized(axis);
	quat rotation = quat_from_axis_angle(normAxis, angleDeg);

	point3d offset = p3_diff(tr->p, axisorigin);
	point3d rotatedOffset = quat_rotate_point(rotation, offset);
	tr->p = p3sum(axisorigin, rotatedOffset);

	tr->r = quat_rotate_point(rotation, tr->r);
	tr->d = quat_rotate_point(rotation, tr->d);
	tr->f = quat_rotate_point(rotation, tr->f);
}

static inline void tr_quat_slerp_rotate(transform *trsubj, transform *trto, float normt) {
	quat qsubj = quat_from_tr(trsubj);
	quat qto = quat_from_tr(trto);
	quat result = quat_slerp(qsubj, qto, normt);

	// Preserve original axis lengths
	float rlen = p3_length(&trsubj->r);
	float dlen = p3_length(&trsubj->d);
	float flen = p3_length(&trsubj->f);

	quat_to_tr(result, trsubj);

	p3_scalar_div(&trsubj->r, p3_length(&trsubj->r));
	p3_scalar_div(&trsubj->d, p3_length(&trsubj->d));
	p3_scalar_div(&trsubj->f, p3_length(&trsubj->f));

	trsubj->r.x *= rlen;
	trsubj->r.y *= rlen;
	trsubj->r.z *= rlen;
	trsubj->d.x *= dlen;
	trsubj->d.y *= dlen;
	trsubj->d.z *= dlen;
	trsubj->f.x *= flen;
	trsubj->f.y *= flen;
	trsubj->f.z *= flen;
}

static inline void tr_quat_look_atp3(transform *trsubj, point3d to, float normt) {
	point3d dir = p3_diff(to, trsubj->p);
	float len = p3_length(&dir);
	if (len < epsilon) return;

	p3_scalar_div(&dir, len);

	point3d up = {0.0f, 0.0f, 1.0f};
	point3d right = p3_cross(up, dir);
	float rightLen = p3_length(&right);
	if (rightLen < epsilon) {
		up.x = 1.0f;
		up.y = 0.0f;
		up.z = 0.0f;
		right = p3_cross(up, dir);
		rightLen = p3_length(&right);
	}
	p3_scalar_div(&right, rightLen);
	up = p3_cross(dir, right);

	// Preserve original axis lengths
	float rlen = p3_length(&trsubj->r);
	float dlen = p3_length(&trsubj->d);
	float flen = p3_length(&trsubj->f);

	transform target = *trsubj;
	target.f.x = dir.x * flen;
	target.f.y = dir.y * flen;
	target.f.z = dir.z * flen;
	target.r.x = right.x * rlen;
	target.r.y = right.y * rlen;
	target.r.z = right.z * rlen;
	target.d.x = up.x * dlen;
	target.d.y = up.y * dlen;
	target.d.z = up.z * dlen;

	tr_quat_slerp_rotate(trsubj, &target, normt);
}

#endif


#endif //RAYLIB_LUA_IMGUI_BUILDMATH_H
