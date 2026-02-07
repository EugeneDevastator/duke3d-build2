//
// Created by omnis on 1/7/2026.
//

#ifndef RAYLIB_LUA_IMGUI_DUMBEDIT_HPP
#define RAYLIB_LUA_IMGUI_DUMBEDIT_HPP
#include "Editor/uimodels.h"
static TextureBrowser *texbstate;
static Vector3 buildToRaylibPos(point3d buildcoord) {
	return {buildcoord.x, -buildcoord.z, buildcoord.y};
}

// Oh i know - the best option -
// Make a pick and carry mode.
// object or selection gets attached to the camera and follow it.
// scroll - scales.

// in default fly mode - scroll regulates how much objects you select and selection radius.

// in initial selection mode:
// ctrl - select similar - wall same tiles, sprites same tile
// shift - all walls of sec, floor/ceil, all sprites of sec.
// grave - target uv's for move rotate scale
// alt - // paint selection mode


// in modify mode:
// ctrl - auto snap to somewhere
// shift - slow down.
// grave - do increments

// in move mode wall/surfs:
// w - wall
// s - both ceil flor alternate between scaling up down or moveing together entire sector.
// s switches between this or next sector.
// c - ceil
// f - floor
// for sector selection always select sector AND any closest wall.

// ====== UV EDITING
// vbh = vertical basis horisontal = vou - for snapping to vertices and controling procedural behavior.

// looks like u for walls can be float between walls?
// another problem - slope wrapping, and essentially perspective uv projection.
// sort of scale y by x and x by y.
// so we kinda still do need 3 vector. + basis. so entire transform.
// perhaps trapezoid free transform will do?
// o u v w , but w depends on o and v, so we can store w as 2d vector in original uv space?
// we might also want w vewctor be dependent on u vector, for ex. animarted slopes
// pehaps in the end - best option is just have polymorphic unwrap as unions. would be easier on the brain, have currently simple and expand later.

extern "C" {
#include "buildmath.h"
#include "shadowtest2.h"
}

#define SEL_SPRI 1
#define SEL_SURF 1<<1
#define SEL_WAL 1<<2
#define SEL_SEC 1<<3
#define SEL_CHUNK 1<<4

#define SEL_UP 1<<5
#define SEL_DOWN 1<<6
#define SEL_MID 1<<7

#define SEL_NEAR 1<<8
#define SEL_FAR 1<<9

#define SEL_ALL 1<<10

uint16_t selmode = SEL_ALL;

#define ISGRABSPRI (grabfoc.spri >= 0)
#define ISGRABWAL (grabfoc.wal >= 0 && grabfoc.sec >= 0)
#define ISGRABCAP (grabfoc.wal < 0 && grabfoc.sec >= 0)
#define ISHOVERWAL (hoverfoc.wal >= 0 && hoverfoc.sec >= 0)
#define ISHOVERCAP (hoverfoc.wal < 0 && hoverfoc.sec >= 0)
#define HOVERSEC map->sect[hoverfoc.sec]
#define HOVERSPRI map->spri[hoverfoc.spri]
#define GRABSEC map->sect[grabfoc.sec]
#define GRABSPRI map->spri[grabfoc.spri]
#define HOVERWAL HOVERSEC.wall[hoverfoc.wal]
#define HOVERWAL2 HOVERSEC.wall[hoverfoc.wal2]

enum editorMode {
	Fly,
	Move,
	rotate
};

enum editorop {
	noop,
	accept,
	discard,
	back,
};

typedef struct estate {
	uint8_t id;

	void (*start)();

	void (*update)();

	void (*accept)();

	void (*discard)();
};
enum dateditop {
	dNone,
	dTiles,
	dColor,
	dValue,
};

typedef struct {
	dateditop op;
	bool haschanged;
	int32_t curval;
	int32_t curval2;
	char* text;
	Color color;
} datedit_t;


typedef struct {
	enum editorMode mode;
	bool iswasdEnabled;
	bool issnapEnabled;
	enum editorop op;
	bool hasOp;
	estate state;
} econtext;

typedef struct {
	long sect;
	int wal; // -1 = ceil -2 = fllor;
	point3d pos;
} loopt; // draw loop point

inline bool islptoncap(loopt p) { return p.wal >= 0 ? false : true; }


typedef struct {
	Vector3 camdelta;
	Vector3 targetDelta;
} viewmodel;

viewmodel editormodel = {};

int K_PICKGRAB = KEY_Q;
int K_LOOPDRAW = KEY_SPACE;
int K_PICKTILE = KEY_V;
int K_ACCEPT = KEY_SPACE;
int K_DISCARD = KEY_GRAVE;


// ------------ shared context

cam_t *cam;
econtext ctx;
econtext ctxprev;

typedef struct {
	signed int wal;
	signed int wal2;
	int onewall;
	int sec;
	int spri;
	int surf;
	point3d hitpos;
} focus_t;

bool hasgrab = false;
point3d *wcursor;
focus_t hoverfoc;
focus_t bufferfoc;
focus_t grabfoc;
datedit_t datstate;
// -------- RL Drawing funcs
const float vertside = 0.3f;
const float vertholeNorm = 0.85f;
const Color vertColor = {64, 255, 64, 255};
static Camera3D cam3d;

void drawVert(Vector3 pos) {
	float holeside = vertside * vertholeNorm;
	float half_vert = vertside * 0.5f;
	float half_hole = holeside * 0.5f;

	rlSetTexture(0);
	rlColor4ub(255, 255, 255, 255);

	rlDisableDepthTest();
	rlDisableDepthMask();
	// Get camera forward vector to align billboard
	Vector3 forward = Vector3Subtract(cam3d.target, cam3d.position);
	forward = Vector3Normalize(forward);

	// Calculate right and up vectors for billboard orientation
	Vector3 up = {0, 1, 0};
	Vector3 right = Vector3CrossProduct(up, forward);
	right = Vector3Normalize(right);
	up = Vector3CrossProduct(forward, right);

	// Scale vectors
	Vector3 rightScaled = Vector3Scale(right, half_vert);
	Vector3 upScaled = Vector3Scale(up, half_vert);
	Vector3 rightHole = Vector3Scale(right, half_hole);
	Vector3 upHole = Vector3Scale(up, half_hole);

	rlBegin(RL_QUADS);
	rlColor4ub(vertColor.r, vertColor.g, vertColor.b, vertColor.a);

	// Top strip
	Vector3 tl = Vector3Add(Vector3Subtract(pos, rightScaled), upScaled);
	Vector3 tr = Vector3Add(Vector3Add(pos, rightScaled), upScaled);
	Vector3 br = Vector3Add(Vector3Add(pos, rightScaled), upHole);
	Vector3 bl = Vector3Add(Vector3Subtract(pos, rightScaled), upHole);

	rlVertex3f(tl.x, tl.y, tl.z);
	rlVertex3f(tr.x, tr.y, tr.z);
	rlVertex3f(br.x, br.y, br.z);
	rlVertex3f(bl.x, bl.y, bl.z);

	// Bottom strip
	tl = Vector3Add(Vector3Subtract(pos, rightScaled), Vector3Scale(upHole, -1));
	tr = Vector3Add(Vector3Add(pos, rightScaled), Vector3Scale(upHole, -1));
	br = Vector3Add(Vector3Add(pos, rightScaled), Vector3Scale(upScaled, -1));
	bl = Vector3Add(Vector3Subtract(pos, rightScaled), Vector3Scale(upScaled, -1));

	rlVertex3f(tl.x, tl.y, tl.z);
	rlVertex3f(tr.x, tr.y, tr.z);
	rlVertex3f(br.x, br.y, br.z);
	rlVertex3f(bl.x, bl.y, bl.z);

	// Left strip
	tl = Vector3Add(Vector3Subtract(pos, rightScaled), upHole);
	tr = Vector3Add(Vector3Subtract(pos, rightHole), upHole);
	br = Vector3Add(Vector3Subtract(pos, rightHole), Vector3Scale(upHole, -1));
	bl = Vector3Add(Vector3Subtract(pos, rightScaled), Vector3Scale(upHole, -1));

	rlVertex3f(tl.x, tl.y, tl.z);
	rlVertex3f(tr.x, tr.y, tr.z);
	rlVertex3f(br.x, br.y, br.z);
	rlVertex3f(bl.x, bl.y, bl.z);

	// Right strip
	tl = Vector3Add(Vector3Add(pos, rightHole), upHole);
	tr = Vector3Add(Vector3Add(pos, rightScaled), upHole);
	br = Vector3Add(Vector3Add(pos, rightScaled), Vector3Scale(upHole, -1));
	bl = Vector3Add(Vector3Add(pos, rightHole), Vector3Scale(upHole, -1));

	rlVertex3f(tl.x, tl.y, tl.z);
	rlVertex3f(tr.x, tr.y, tr.z);
	rlVertex3f(br.x, br.y, br.z);
	rlVertex3f(bl.x, bl.y, bl.z);

	rlEnd();
}

void drawVert(int sec, int w) {
	if (sec >= 0 && w >= 0) {
		wall_t *wal = &map->sect[sec].wall[w];
		Vector3 pos = {wal->x, -1 * (float) getwallz(&map->sect[sec], 1, w), wal->y};
		drawVert(pos);
	}
}

void drawCylBoard(Vector3 origin, Vector3 localUp, float length, float width) {
}

void drawCylBoard(Vector3 origin, Vector3 endpoint, float width) {
	rlSetTexture(0);
	rlDisableDepthTest();
	rlDisableDepthMask();

	// Calculate line axis
	Vector3 axis = Vector3Subtract(endpoint, origin);
	float length = Vector3Length(axis);
	if (length < 0.001f) return;
	axis = Vector3Normalize(axis);

	// Get direction from line midpoint to camera
	Vector3 midpoint = Vector3Scale(Vector3Add(origin, endpoint), 0.5f);
	Vector3 toCamera = Vector3Subtract(cam3d.position, midpoint);

	// Project toCamera onto plane perpendicular to axis
	float dot = Vector3DotProduct(toCamera, axis);
	Vector3 toCameraProjected = Vector3Subtract(toCamera, Vector3Scale(axis, dot));
	float projLen = Vector3Length(toCameraProjected);

	Vector3 right;
	Vector3 facing;

	if (projLen < 0.001f) {
		Vector3 up = {0, 1, 0};
		if (fabsf(Vector3DotProduct(axis, up)) > 0.99f) {
			up = {1, 0, 0};
		}
		right = Vector3CrossProduct(axis, up);
		right = Vector3Normalize(right);
		facing = Vector3CrossProduct(right, axis);
	} else {
		facing = Vector3Normalize(toCameraProjected);
		right = Vector3CrossProduct(axis, facing);
		right = Vector3Normalize(right);
	}

	// Build transformation matrix (column-major for raylib)
	// Local space: X = right, Y = axis (along line), Z = facing (toward camera)
	Matrix transform = {
		right.x, axis.x, facing.x, origin.x,
		right.y, axis.y, facing.y, origin.y,
		right.z, axis.z, facing.z, origin.z,
		0, 0, 0, 1
	};

	rlPushMatrix();
	rlMultMatrixf(MatrixToFloat(transform));

	float hw = width * 0.5f;

	rlBegin(RL_QUADS);

	// Main quad (front face, XY plane at Z=0, facing +Z toward camera)
	rlVertex3f(-hw, 0, 0);
	rlVertex3f(hw, 0, 0);
	rlVertex3f(hw, length, 0);
	rlVertex3f(-hw, length, 0);

	// Origin end cap (at Y=0, XZ plane, extends along +Z)
	rlVertex3f(-hw, 0, 0);
	rlVertex3f(-hw, 0, hw);
	rlVertex3f(hw, 0, hw);
	rlVertex3f(hw, 0, 0);

	// Endpoint end cap (at Y=length, XZ plane, extends along +Z)
	rlVertex3f(hw, length, 0);
	rlVertex3f(hw, length, hw);
	rlVertex3f(-hw, length, hw);
	rlVertex3f(-hw, length, 0);

	rlEnd();

	rlPopMatrix();
}

// ------------------ PICKGRAB
transform savedtr;
point3d localp1;
point3d localp2;
transform trdiff;
wall_idx verts[256];
wall_idx verts2[256];
int totalverts;
int totalverts2;
point2d secondwalldif;
bool pg_graballverts = true;
bool usehitZ = false;
float savedHeight;

void PickgrabDiscard() {
	if (grabfoc.spri >= 0) {
		map->spri[grabfoc.spri].tr = savedtr;
	} else if (ISGRABWAL) {
		map->sect[grabfoc.sec].wall[grabfoc.wal].x = savedtr.p.x;
		map->sect[grabfoc.sec].wall[grabfoc.wal].y = savedtr.p.y;
		for (int i = 0; i < totalverts; ++i) {
			map->sect[verts[i].s].wall[verts[i].w].x = savedtr.p.x;
			map->sect[verts[i].s].wall[verts[i].w].y = savedtr.p.y;
			// move nextwall
			//map->sect[grabfoc.sec].wall[grabfoc.wal2].x=tp2.x;
			//map->sect[grabfoc.sec].wall[grabfoc.wal2].y=tp2.y;
			//for (int i = 0; i < totalverts2; ++i) {
			//	map->sect[verts2[i].s].wall[verts2[i].w].x = tp2.x;
			//	map->sect[verts2[i].s].wall[verts2[i].w].y = tp2.y;
			//}
		}
	}
}

void PickgrabAccept() {
	if (grabfoc.spri >= 0) {
		int s = map->spri[grabfoc.spri].sect;
		updatesect_p(map->spri[grabfoc.spri].p, &s, map);
		changesprisect_imp(grabfoc.spri, s, map);
	} else if (grabfoc.wal >= 0 && grabfoc.sec >= 0) {
		// need to update sprites here.
		//		map->sect[grabfoc.sec].wall[grabfoc.wal].x=savedtr.p.x;
		//	map->sect[grabfoc.sec].wall[grabfoc.wal].y=savedtr.p.y;
		checksprisect_imp(-1, map);
	}

	grabfoc.spri = -1; // jsut do noting now.
	ctx.mode = Fly;
}

void PickgrabUpdate() {
	Vector2 dmov = GetMouseDelta();
	float scrol = GetMouseWheelMove();
	addto(&trdiff.p, scaled(BBDOWN, scrol * 0.2f));
	addto(&localp2, scaled(BBDOWN, scrol * 0.2f));

	if (ISGRABSPRI) {
		map->spri[grabfoc.spri].tr = local_to_world_transform_p(trdiff, &cam->tr);
		int s = map->spri[grabfoc.spri].sect;
		if (hoverfoc.sec >= 0) {
			GRABSPRI.walcon = (signed char)hoverfoc.wal;
		}

		updatesect_p(map->spri[grabfoc.spri].p, &s, map);
		changesprisect_imp(grabfoc.spri, s, map);
	} else if (ISGRABCAP) {
		float newz = local_to_world_point(trdiff.p, &cam->tr).z;
		int isflor = grabfoc.wal + 2;
		GRABSEC.z[isflor] = newz;
		if (IsKeyDown(KEY_LEFT_SHIFT)) {
			GRABSEC.z[1 - isflor] = newz + savedHeight * ((1 - isflor) * 2 - 1);
		}
	} else if (ISGRABWAL) {
		transform tmp;
		point3d tp2;

		// Wall dragging along floor/ceiling plane
		//if (hoverfoc.wal < 0 && hoverfoc.sec >= 0)
		{
			// Project camera ray onto horizontal plane

			// Calculate intersection of camera ray with horizontal plane at target_z
			point3d ray_start = cam->p;
			point3d ray_dir = cam->f;
			float target_z;

			if (IsKeyPressed(KEY_F))
				usehitZ = !usehitZ;
			if (usehitZ)
				target_z = grabfoc.hitpos.z; // floor : ceiling
			else
				target_z = map->sect[grabfoc.sec].z[ray_dir.z > 0]; // floor : ceiling

			if (fabsf(ray_dir.z) > 0.001f) {
				float t = (target_z - ray_start.z) / ray_dir.z;
				tmp.p.x = ray_start.x + ray_dir.x * t;
				tmp.p.y = ray_start.y + ray_dir.y * t;
			} else {
				// Ray parallel to plane, use mouse delta for movement
				point3d right = cam->tr.r;
				point3d down = cam->tr.d;
				tmp.p.x = savedtr.p.x + dmov.x * 0.1f * right.x + dmov.y * -0.1f * down.x;
				tmp.p.y = savedtr.p.y + dmov.x * 0.1f * right.y + dmov.y * -0.1f * down.y;
			}

			tp2.x = tmp.p.x + localp2.x;
			tp2.y = tmp.p.y + localp2.y;
		}
		//else {
		//    tmp = local_to_world_transform_p(trdiff, &cam->tr);
		//    tp2 = local_to_world_point(localp2, &cam->tr);
		//}

		map->sect[grabfoc.sec].wall[grabfoc.wal].x = tmp.p.x;
		map->sect[grabfoc.sec].wall[grabfoc.wal].y = tmp.p.y;

		if (pg_graballverts)
			for (int i = 0; i < totalverts; ++i) {
				map->sect[verts[i].s].wall[verts[i].w].x = tmp.p.x;
				map->sect[verts[i].s].wall[verts[i].w].y = tmp.p.y;
			}

		// Move ahead-wall
		if (selmode & SEL_SURF) {
			map->sect[grabfoc.sec].wall[grabfoc.wal2].x = tp2.x;
			map->sect[grabfoc.sec].wall[grabfoc.wal2].y = tp2.y;
			for (int i = 0; i < totalverts2; ++i) {
				map->sect[verts2[i].s].wall[verts2[i].w].x = tp2.x;
				map->sect[verts2[i].s].wall[verts2[i].w].y = tp2.y;
			}
		}
	}

	if (IsKeyPressed(K_PICKGRAB)) {
		PickgrabAccept();
		grabfoc.spri = -1;
		ctx.op = accept;
	}
}

void PickgrabStart() {
	if (grabfoc.spri >= 0) {
		savedtr = map->spri[grabfoc.spri].tr;
	} else if (ISGRABCAP) {
		savedtr = cam->tr;
		savedtr.p = grabfoc.hitpos;
		savedHeight = GRABSEC.z[1] - GRABSEC.z[0];
	} else if (grabfoc.wal >= 0 && grabfoc.sec >= 0) {
		bool grabboth = false;
		if (!grabboth) // grab both walls
		{
			savedtr = cam->tr;
			grabfoc.wal = hoverfoc.onewall;
		}
		//for red walls we'd need to grab verts of all adjacent walls.
		// something in build2 was there for it.
		savedtr = cam->tr;
		savedtr.p.x = map->sect[grabfoc.sec].wall[grabfoc.wal].x;
		savedtr.p.y = map->sect[grabfoc.sec].wall[grabfoc.wal].y;

		totalverts = getwallsofvert(grabfoc.sec, grabfoc.wal, verts, 256, map);

		grabfoc.wal2 = map->sect[grabfoc.sec].wall[grabfoc.wal].n + grabfoc.wal;
		point2d wpos = getwall({grabfoc.wal2, grabfoc.sec}, map)->pos;
		point3d wpos3d = {wpos.x, wpos.y, 0};
		localp2 = world_to_local_point(wpos3d, &cam->tr);
		totalverts2 = getwallsofvert(grabfoc.sec, grabfoc.wal2, verts2, 256, map);
	}
	trdiff = world_to_local_transform_p(savedtr, &cam->tr);
}

// ----------------------- draw loop OPER ---------------------
loopt loopts[100];
point2d loopts_p3d[100];
int loopn = 0;

void LoopDrawUpdate() {
	if (IsKeyPressed(KEY_THREE) && loopn == 1) {
		long s = mapspriteadd(loopts[0].sect, loopts[0].pos, map);
		map->spri[s].tilnum = 1;
		ctx.op = discard;
	}
	if (IsKeyPressed(KEY_SPACE)) {
		//add
		if (hoverfoc.sec >= 0) {
			loopts[loopn].sect = hoverfoc.sec;
			loopts[loopn].wal = hoverfoc.wal;
		} else {
			loopts[loopn].sect = -1;
			loopts[loopn].wal = 0;
		}
		loopts[loopn].pos = hoverfoc.hitpos;
		loopn++;
	}
	if (IsKeyPressed(KEY_R)) {
		// del last;
		loopn = max(0, loopn-1);
	}

	if (IsKeyPressed(KEY_T)) {
		for (int i = 0; i < loopn; ++i) {
			loopts_p3d[i] = {loopts[i].pos.x, loopts[i].pos.y};
		}
		appendwall_loop(&map->sect[hoverfoc.sec], loopn, loopts_p3d);
		loopn = 0;
		ctx.op = accept;
	}

	// so walls are just i as t is, nw can look like this: 1 1 -2 1 1 1 -3 essentially two loops 3 and 4 verts.
	// inner loop is counter clockwise.
	// outer is clockwize.
}

void LoopDrawDiscard() {
	loopn = 0;
	K_ACCEPT = KEY_SPACE;
}

void LoopDrawAccept() {
	loopn = 0;
	K_ACCEPT = KEY_SPACE;
}

void LoopDrawStart() {
	loopn = 0;
	K_ACCEPT = KEY_E;
	LoopDrawUpdate();
}

//----------------------- tile editing
void TilsedStart() {

}

void TilsedUpdate() {
	if (IsKeyDown(KEY_TAB)) {
		if (hoverfoc.spri >= 0) {
			datstate.curval = HOVERSPRI.tilnum;
			datstate.curval2 = HOVERSPRI.galnum;
		} else if (ISHOVERWAL) {
			datstate.curval = HOVERWAL.xsurf[hoverfoc.surf].tilnum;
			datstate.curval2 = HOVERWAL.xsurf[hoverfoc.surf].galnum;
		} else if (ISHOVERCAP) {
			datstate.curval = HOVERSEC.surf[hoverfoc.surf].tilnum;
			datstate.curval2 = HOVERSEC.surf[hoverfoc.surf].galnum;
		}

		texbstate->selected = datstate.curval;
		texbstate->galnum = datstate.curval2;
	}
	if (IsKeyPressed(KEY_E)) {
		datstate.curval = texbstate->selected;
		datstate.curval2 = texbstate->galnum;
		if (hoverfoc.spri >= 0) {
			HOVERSPRI.tilnum = datstate.curval;
			HOVERSPRI.galnum = datstate.curval2;
		} else if (ISHOVERWAL) {
			HOVERWAL.xsurf[hoverfoc.surf].tilnum = datstate.curval;
			HOVERWAL.xsurf[hoverfoc.surf].galnum = datstate.curval2;
		} else if (ISHOVERCAP) {
			HOVERSEC.surf[hoverfoc.surf].tilnum = datstate.curval;
			HOVERSEC.surf[hoverfoc.surf].galnum = datstate.curval2;
		}
	}
}
void TilsedAccept() {

}
void TilsedDiscard() {

}



// ----------------------- MOVE OPER ---------------------
point3d savedpos;

void MoveObjContextUpdate() {
	Vector2 dmov = GetMouseDelta();
	float scrol = GetMouseWheelMove();
	map->spri[grabfoc.spri].p.x += dmov.x * 0.5f;
	map->spri[grabfoc.spri].p.y += dmov.y * 0.5f;
	map->spri[grabfoc.spri].p.z += scrol * 0.5f;
}

void MoveObjContextDiscard() {
	map->spri[grabfoc.spri].p = savedpos;
}

void MoveObjContextAccept() {
	grabfoc.spri = -1; // jsut do noting now.
}

void MoveObjContextStart() {
	grabfoc.spri = grabfoc.spri;
	savedpos = map->spri[grabfoc.spri].p;
}


// ------------------------ all operators-----------------
void empty() {
}

#define MAKESTATE(id, name) const estate name##State = { id, name##Start, name##Update, name##Accept, name##Discard };
const estate RotateState = {3, MoveObjContextStart, MoveObjContextUpdate, MoveObjContextAccept, MoveObjContextDiscard};
const estate MoveState = {2, MoveObjContextStart, MoveObjContextUpdate, MoveObjContextAccept, MoveObjContextDiscard};
const estate FlyState = {1, MoveObjContextStart, MoveObjContextUpdate, MoveObjContextAccept, MoveObjContextDiscard};
MAKESTATE(5, LoopDraw);
MAKESTATE(4, Pickgrab);
MAKESTATE(6, Tilsed);

const estate Empty = {0, empty, empty, empty, empty};
// ----------------- MAIN
void InitEditor(mapstate_t *m) {
	map = m;
	ctx.mode = Fly;
	ctx.state = Empty;
}

void SetColorum(uint8_t r, uint8_t g, uint8_t b, int16_t lum) {
	if (hoverfoc.spri >= 0) {
		map->spri[hoverfoc.spri].view.color.x = r / 255.0F;
		map->spri[hoverfoc.spri].view.color.y = g / 255.0F;
		map->spri[hoverfoc.spri].view.color.z = b / 255.0F;
		map->spri[hoverfoc.spri].view.lum = lum;
	}
}



void EditorSetTileState(TextureBrowser *ts) {
	texbstate = ts;
}
void Editor_DoRaycasts(cam_t *cc) {
	int isec = 0;
	int iwal = 0;
	int ispri = 0;
	cam = cc;
	//ctx.state.discard();// to restore original map state for raycast.
	raycast(&cc->p, &cc->f, 1e32, cc->cursect, &hoverfoc.sec, &hoverfoc.wal, &hoverfoc.spri,&hoverfoc.surf, &hoverfoc.hitpos, RHIT_REDWALLS, map);
	hoverfoc.wal2=-1;
	if (ISHOVERWAL) {
		hoverfoc.wal2 = mapwallnextid(hoverfoc.sec,hoverfoc.wal,map);
		float z1 = getwallz(&map->sect[hoverfoc.sec], 1, hoverfoc.wal);
		float z2 = getwallz(&map->sect[hoverfoc.sec], 1, hoverfoc.wal2);
		float d1 = bmathdistsqrp2d({HOVERWAL.x, HOVERWAL.y}, BPXY(hoverfoc.hitpos));
		float d2 = bmathdistsqrp2d({HOVERWAL2.x, HOVERWAL2.y}, BPXY(hoverfoc.hitpos));
		hoverfoc.onewall = hoverfoc.wal;
		if (d2 < d1) {
			hoverfoc.onewall = hoverfoc.wal2;
		}
	}
}

void EditorFrameMin(const Camera3D rlcam) {
	// process raycasts;
	cam3d = rlcam;
	ctx.state.update();
	if (ctx.state.id == Empty.id) {
		if (IsKeyPressed(K_PICKGRAB)) {
			hasgrab = true;
			grabfoc = hoverfoc;
			ctx.state = PickgrabState;
			ctx.state.start();
		} else if (IsKeyPressed(K_LOOPDRAW)) {
			hasgrab = true;
			grabfoc = hoverfoc;
			ctx.state = LoopDrawState;
			ctx.state.start();
		} 	else if (IsKeyPressed(K_PICKTILE)) {
// dont lock up to the surf.
			// e = apply tile
			// r = read from hover. and refocus selected tile.
			datstate.op = dTiles;
			ctx.state = TilsedState;
			ctx.state.start();
		} else {
			datstate.op=dNone;
			if (IsKeyPressed(KEY_K)) { // EXTRUDE PROTOTYPE
				point2d p0 = HOVERWAL.pos;
				point2d p3 = HOVERWAL2.pos;
				float offsy = -(p3.x - p0.x);
				float offsx = (p3.y - p0.y);

				point2d p1 = p0;
				point2d p2 = p3;
				p1.x += offsx;
				p1.y += offsy;
				p2.x += offsx;
				p2.y += offsy;


				float capr = HOVERSEC.z[1] - HOVERSEC.z[0];
				float florz = HOVERSEC.z[1] - capr * 0.1;
				//axis grows down to floor.
				if (HOVERWAL.ns >= 0) {
					if (hoverfoc.hitpos.z < map->sect[HOVERWAL.ns].z[0]) {
						// upper seg.
						florz = map->sect[HOVERWAL.ns].z[0];
						capr = florz - HOVERSEC.z[0];
						florz -= capr * 0.1;
					} else // lower seg then;
					{
						// upper seg.
						florz = HOVERSEC.z[1];
						capr = florz - map->sect[HOVERWAL.ns].z[1];
						florz -= capr * 0.1;
					}
				}
				float height = capr * 0.2;
				point2d loop[4] = {p0, p1, p2, p3};
				int nsid = addsectfromloop(4, loop, florz, height, map);
				sect_t *sec = &map->sect[nsid];
				sec->wall[0].xsurf[0].tilnum = HOVERWAL.xsurf[0].tilnum;
				sec->wall[1].xsurf[0].tilnum = HOVERWAL.xsurf[0].tilnum;
				sec->wall[2].xsurf[0].tilnum = HOVERWAL.xsurf[0].tilnum;
				sec->wall[3].xsurf[0].tilnum = HOVERWAL.xsurf[0].tilnum;
				sec->grad[0] = HOVERSEC.grad[0];
				sec->grad[1] = HOVERSEC.grad[1];

				// seems that for SOS, new sectors point to wall and sector of firstly made sector hm.
				// SOS linking section
				// Enhanced SOS linking using chain approach
				if (HOVERWAL.ns < 0) {
					// First sector on this wall - simple case
					sec->wall[3].ns = hoverfoc.sec;
					sec->wall[3].nw = hoverfoc.wal;
					sec->wall[3].nschain = hoverfoc.sec;
					sec->wall[3].nwchain = hoverfoc.wal;
					sec->wall[3].surfn = 3;
					sec->wall[3].xsurf[1] = sec->wall[3].xsurf[0];
					sec->wall[3].xsurf[2] = sec->wall[3].xsurf[0];
					sec->wall[3].surf.flags = 4;

					// Update original wall to point to new sector
					HOVERWAL.ns = nsid;
					HOVERWAL.nw = 3;
					HOVERWAL.nschain = nsid;
					HOVERWAL.nwchain = 3;
					HOVERWAL.surfn = 3;
					HOVERWAL.surf.flags = 4;
					HOVERWAL.xsurf[1] = HOVERWAL.xsurf[0];
					HOVERWAL.xsurf[2] = HOVERWAL.xsurf[0];
				} else {
					// Multiple sectors case - upgrade existing chain first
					upgradewallportchain(hoverfoc.sec, hoverfoc.wal, map);

					// Get existing chain using new method
					vertlist_t existing_chain[32];
					int chain_count = getwalls_chain(hoverfoc.sec, hoverfoc.wal, existing_chain, 32, map);

					if (chain_count > 0) {
						// Find insertion point based on height
						float new_height = sec->z[0] + sec->z[1]; // floor + ceiling
						float fx = (HOVERWAL.pos.x + HOVERWAL2.pos.x) * 0.5f;
						float fy = (HOVERWAL.pos.y + HOVERWAL2.pos.y) * 0.5f;

						int insert_after = -1;
						for (int i = 0; i < chain_count; i++) {
							float chain_height = getslopez(&map->sect[existing_chain[i].s], 0, fx, fy) +
												 getslopez(&map->sect[existing_chain[i].s], 1, fx, fy);
							if (new_height > chain_height) {
								insert_after = i;
							} else {
								break;
							}
						}

						if (insert_after == -1) {
							// Insert at beginning of chain
							sec->wall[3].ns = existing_chain[0].s;
							sec->wall[3].nw = existing_chain[0].w;
							sec->wall[3].nschain = existing_chain[0].s;
							sec->wall[3].nwchain = existing_chain[0].w;

							// Update original wall to point to new sector
							HOVERWAL.ns = nsid;
							HOVERWAL.nw = 3;
							HOVERWAL.nschain = nsid;
							HOVERWAL.nwchain = 3;
						} else if (insert_after == chain_count - 1) {
							// Insert at end of chain
							wall_t *last_wall = &map->sect[existing_chain[insert_after].s].wall[existing_chain[
								insert_after].w];

							sec->wall[3].ns = last_wall->nschain;
							sec->wall[3].nw = last_wall->nwchain;
							sec->wall[3].nschain = last_wall->nschain;
							sec->wall[3].nwchain = last_wall->nwchain;

							// Update last wall to point to new sector
							last_wall->nschain = nsid;
							last_wall->nwchain = 3;
						} else {
							// Insert in middle of chain
							wall_t *prev_wall = &map->sect[existing_chain[insert_after].s].wall[existing_chain[
								insert_after].w];

							sec->wall[3].ns = prev_wall->nschain;
							sec->wall[3].nw = prev_wall->nwchain;
							sec->wall[3].nschain = prev_wall->nschain;
							sec->wall[3].nwchain = prev_wall->nwchain;

							// Update previous wall to point to new sector
							prev_wall->nschain = nsid;
							prev_wall->nwchain = 3;
						}
					}

					// Set surface properties
					sec->wall[3].surfn = 3;
					sec->wall[3].surf.flags = 4;
					sec->wall[3].xsurf[1] = sec->wall[3].xsurf[0];
					sec->wall[3].xsurf[2] = sec->wall[3].xsurf[0];

					// Update original wall surface properties
					HOVERWAL.surfn = 3;
					HOVERWAL.surf.flags = 4;
					HOVERWAL.xsurf[1] = HOVERWAL.xsurf[0];
					HOVERWAL.xsurf[2] = HOVERWAL.xsurf[0];
				}
			}
		}
	}
	// how to unpress the key here to prevent further intercept?
	if (hoverfoc.spri >= 0 || grabfoc.spri >= 0) {
		focus_t usefoc = (ctx.state.id == Empty.id) ? hoverfoc : grabfoc;
		if (IsKeyPressed(KEY_L)) {
			map->spri[usefoc.spri].flags ^= SPRITE_B2_IS_LIGHT;
			bool wasdel = false;
			for (int j = map->light_sprinum - 1; j >= 0; j--) {
				if (map->light_spri[j] == usefoc.spri) {
					map->light_spri[j] = map->light_spri[--map->light_sprinum];
					wasdel = true;
				}
			}

			if (!wasdel && map->light_sprinum < MAXLIGHTS) {
				map->spri[usefoc.spri].view.lum = 255;
				map->light_spri[map->light_sprinum++] = usefoc.spri;
			}
		}
	}
	if (ISHOVERWAL) {
		hoverfoc.wal2 = map->sect[hoverfoc.sec].wall[hoverfoc.wal].n + hoverfoc.wal;
		if (IsKeyPressed(KEY_INSERT)) {
			// split walls
			if (hoverfoc.wal >= 0) {
				splitwallat(hoverfoc.sec, hoverfoc.wal, hoverfoc.hitpos, map);
			}
		}
	}

	if (IsKeyPressed(K_DISCARD) || IsKeyPressed(KEY_ESCAPE) || ctx.op == discard) {
		ctx.state.discard();
		ctx.state = Empty;
		ctx.op = noop;
	}
	if (IsKeyPressed(K_ACCEPT) || ctx.op == accept) {
		ctx.state.accept();
		ctx.state = Empty;
		ctx.op = noop;
	}
}

void DrawGizmos() {
	float mv = GetMouseWheelMove();
	if (hoverfoc.spri >= 0) {
		transform *sptr = &map->spri[hoverfoc.spri].tr;
		qrotaxis(sptr, sptr->r, mv);

		Vector3 pos = buildToRaylibPos(sptr->p);
		Vector3 fw = buildToRaylibPos(sptr->r);
		Vector3 rg = buildToRaylibPos(sptr->d);
		Vector3 dw = buildToRaylibPos(sptr->f);
		float l = Vector3Length(rg);
		Vector3 bbmin = pos + Vector3{l, l, l};
		Vector3 bbmax = pos - Vector3{l, l, l};
		DrawBoundingBox({bbmax, bbmin}, LIME);
		//   addto(&map->spri[focusedSprite].tr.p,scaled(right,mv));
	}
	focus_t usefoc;

	if (ISHOVERWAL || ctx.state.id != Empty.id) {
		if (ctx.state.id == Empty.id)
			usefoc = hoverfoc;
		else
			usefoc = grabfoc;
		drawVert(usefoc.sec, usefoc.wal);
		drawVert(usefoc.sec, usefoc.wal2);

		float z1 = getwallz(&map->sect[usefoc.sec], 0, usefoc.onewall);
		float z2 = getwallz(&map->sect[usefoc.sec], 1, usefoc.onewall);
		wall_t *w = &map->sect[usefoc.sec].wall[usefoc.onewall];
		//	wall_t *w2 = &map->sect[usefoc.sec].wall[usefoc.wal2];
		Vector3 rlp1 = {w->x, -z1 - 0.1f, w->y};
		Vector3 rlp2 = {w->x, -z2 + 0.1f, w->y};
		rlColor4ub(255, 128, 128, 255);
		drawCylBoard(rlp1, rlp2, 0.1f);
	}
	DrawPoint3D(buildToRaylibPos(hoverfoc.hitpos), RED);

	// Draw loops that are userdrawn:
	int n = 0;
	for (int i = 0; i < loopn; ++i) {
		n = (i + 1) % loopn;
		rlColor4ub(255, 255, 255, 255);
		drawVert(buildToRaylib(loopts[i].pos));
		rlColor4ub(230, 230, 230, 255);
		drawCylBoard(buildToRaylib(loopts[i].pos), buildToRaylib(loopts[n].pos), 0.01);
	}
}

void EditorFrame() {
	if (ctx.mode == Fly) {
		if (IsKeyPressed(KEY_G)) {
			if (grabfoc.spri >= 0) {
				ctx.mode = Move;
				ctx.state = MoveState;
			}
		}
		if (IsKeyPressed(KEY_R)) { ctx.mode = rotate; }
	}
	// intercept fly overlay
	if (IsMouseButtonDown(1)) {
		FlyState.update();
		return;
	}

	if (IsKeyPressed(KEY_SPACE)) {
		ctx.state.accept();

		ctx.mode = Fly;
	}
	if (IsKeyPressed(KEY_ESCAPE)) {
		ctx.state.discard();
		ctx.mode = Fly;
		// ctx.curstate =
	}
	// parse state change
	if (ctxprev.mode != ctx.mode) {
	}

	//ctx.update();
}
#endif //RAYLIB_LUA_IMGUI_DUMBEDIT_HPP
