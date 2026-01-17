//
// Created by omnis on 1/7/2026.
//

#ifndef RAYLIB_LUA_IMGUI_DUMBEDIT_HPP
#define RAYLIB_LUA_IMGUI_DUMBEDIT_HPP
    static Vector3 buildToRaylibPos(point3d buildcoord)
    {
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

extern "C"{
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

#define ISGRABSPRI (grabfoc.spri >= 0)
#define ISGRABWAL (grabfoc.wal >= 0 && grabfoc.sec >= 0)

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


typedef struct {
	enum editorMode mode;
	bool iswasdEnabled;
	bool issnapEnabled;
	enum editorop op;
	bool hasOp;
	estate state;
} econtext;

typedef struct {
	Vector3 camdelta;
	Vector3 targetDelta;
} viewmodel;
viewmodel editormodel = {};

int K_PICKGRAB = KEY_Q;
int K_ACCEPT = KEY_SPACE;
int K_DISCARD = KEY_GRAVE;


// ------------ shared context

cam_t *cam;
econtext ctx;
econtext ctxprev;
int savedfocus;

typedef struct {
	int wal;
	int wal2;
	int sec;
	int spri;
	point3d hitpos;
} focus;

bool hasgrab = false;
point3d *wcursor;
focus hoverfoc;
focus grabfoc;

// ------------------ PICKGRAB
transform savedtr;
transform trdiff;
wall_idx verts[256];
int totalverts;

void PickgrabDiscard() {
	if (grabfoc.spri>=0) {
		map->spri[savedfocus].tr= savedtr;
	}
	else
		if (ISGRABWAL) {
			map->sect[grabfoc.sec].wall[grabfoc.wal].x=savedtr.p.x;
			map->sect[grabfoc.sec].wall[grabfoc.wal].y=savedtr.p.y;
			for (int i = 0; i < totalverts; ++i) {
				map->sect[verts[i].s].wall[verts[i].w].x = savedtr.p.x;
				map->sect[verts[i].s].wall[verts[i].w].y = savedtr.p.y;
			}
		}
}
void PickgrabAccept() {
	if (grabfoc.spri>=0) {
		int s = map->spri[savedfocus].sect;
		updatesect_p(map->spri[savedfocus].p, &s, map);
		changesprisect_imp(savedfocus, s, map);
	}
	else if (grabfoc.wal>=0 && grabfoc.sec >=0) {
		// need to update sprites here.
//		map->sect[grabfoc.sec].wall[grabfoc.wal].x=savedtr.p.x;
	//	map->sect[grabfoc.sec].wall[grabfoc.wal].y=savedtr.p.y;
	}
	savedfocus = -1; // jsut do noting now.
	ctx.mode = Fly;
}
void PickgrabUpdate() {
	Vector2 dmov = GetMouseDelta();
	float scrol = GetMouseWheelMove();
	addto(&trdiff.p, scaled(down,scrol*0.2f));
	if (grabfoc.spri>=0) {
		map->spri[savedfocus].tr = local_to_world_transform_p(trdiff, &cam->tr);
		int s = map->spri[savedfocus].sect;
		updatesect_p(map->spri[savedfocus].p, &s, map);
		changesprisect_imp(savedfocus, s, map);
	}
	else if (grabfoc.wal>=0 && grabfoc.sec >=0) {
		transform tmp = local_to_world_transform_p(trdiff, &cam->tr);
		map->sect[grabfoc.sec].wall[grabfoc.wal].x=tmp.p.x;
		map->sect[grabfoc.sec].wall[grabfoc.wal].y=tmp.p.y;
		for (int i = 0; i < totalverts; ++i) {
			map->sect[verts[i].s].wall[verts[i].w].x = tmp.p.x;
			map->sect[verts[i].s].wall[verts[i].w].y = tmp.p.y;
		}
	}

	if (IsKeyPressed(K_PICKGRAB)) {
		PickgrabAccept();
		savedfocus = -1;
		ctx.op = accept;
	}
}
void PickgrabStart() {
	if (grabfoc.spri>=0) {
		savedtr = map->spri[grabfoc.spri].tr;
	}
	else
		if (grabfoc.wal>=0 && grabfoc.sec >=0) {
			//for red walls we'd need to grab verts of all adjacent walls.
			// something in build2 was there for it.
			savedtr = cam->tr;
			savedtr.p.x = map->sect[grabfoc.sec].wall[grabfoc.wal].x;
			savedtr.p.y = map->sect[grabfoc.sec].wall[grabfoc.wal].y;
			totalverts = getwallsofvert(grabfoc.sec,grabfoc.wal,verts,256,map);
		}
	trdiff =  world_to_local_transform_p(savedtr, &cam->tr);
}
// ----------------------- MOVE OPER ---------------------
point3d savedpos;

void MoveObjContextUpdate() {
Vector2 dmov = GetMouseDelta();
	float scrol = GetMouseWheelMove();
	map->spri[grabfoc.spri].p.x+=dmov.x*0.5f;
	map->spri[grabfoc.spri].p.y+=dmov.y*0.5f;
	map->spri[grabfoc.spri].p.z+=scrol*0.5f;
}
void MoveObjContextDiscard() {
	map->spri[grabfoc.spri].p = savedpos;
}
void MoveObjContextAccept() {
	savedfocus = -1; // jsut do noting now.
}
void MoveObjContextStart() {
	savedfocus = grabfoc.spri;
	savedpos = map->spri[grabfoc.spri].p;
}


// ------------------------ all operators-----------------
void empty(){}
const estate RotateState = { 3,MoveObjContextStart, MoveObjContextUpdate, MoveObjContextAccept, MoveObjContextDiscard};
const estate MoveState = { 2,MoveObjContextStart, MoveObjContextUpdate, MoveObjContextAccept, MoveObjContextDiscard};
const estate FlyState = { 1,MoveObjContextStart, MoveObjContextUpdate, MoveObjContextAccept, MoveObjContextDiscard};
const estate PickGrabState = { 4,PickgrabStart, PickgrabUpdate, PickgrabAccept, PickgrabDiscard};
const estate Empty = { 0, empty, empty, empty, empty};
// ----------------- MAIN
void InitEditor(mapstate_t *m) {
	map = m;
	ctx.mode = Fly;
	ctx.state = Empty;
}

void SetColorum(uint8_t r,uint8_t g,uint8_t b, int16_t lum) {
	if (grabfoc.spri >=0) {
		map->spri[grabfoc.spri].view.color.x=r/255.0F;
		map->spri[grabfoc.spri].view.color.y=g/255.0F;
		map->spri[grabfoc.spri].view.color.z=b/255.0F;
		map->spri[grabfoc.spri].view.lum=lum;
	}
}

void Editor_DoRaycasts(cam_t *cc) {
	int isec=0;
	int iwal=0;
	int ispri=0;
	cam = cc;
	//this raycast works.
	raycast(&cc->p,&cc->f,1e32,cc->cursect,&hoverfoc.sec,&hoverfoc.wal,&hoverfoc.spri,&hoverfoc.hitpos,map);
}

void EditorFrameMin() {
// process raycasts;

	ctx.state.update();
	if (IsKeyPressed(K_PICKGRAB)) {
		hasgrab = true;
		grabfoc = hoverfoc;
		ctx.state = PickGrabState;
		ctx.state.start();
	}

	if (grabfoc.spri >= 0) {
		if (IsKeyPressed(KEY_L)) {
			map->spri[grabfoc.spri].flags ^= SPRITE_B2_IS_LIGHT;
			bool wasdel = false;
			for (int j = map->light_sprinum - 1; j >= 0; j--) {
				if (map->light_spri[j] == grabfoc.spri) {
					map->light_spri[j] = map->light_spri[--map->light_sprinum];
					wasdel = true;
				}
			}

			if (!wasdel && map->light_sprinum < MAXLIGHTS) {
				map->spri[grabfoc.spri].view.lum=255;
				map->light_spri[map->light_sprinum++] = grabfoc.spri;
			}
		}
	}

	if (IsKeyPressed(K_DISCARD) || IsKeyPressed(KEY_ESCAPE)) {
		ctx.state.discard();
		ctx.state = Empty;
	}
	if (IsKeyPressed(K_ACCEPT) || ctx.op == accept) {
		ctx.state.accept();
		ctx.state = Empty;
		ctx.op = noop;
	}
}
void DrawGizmos(){
	float mv = GetMouseWheelMove();
	if (grabfoc.spri >=0) {
		transform* sptr = &map->spri[grabfoc.spri].tr;
		qrotaxis(sptr, sptr->r, mv);

		Vector3 pos = buildToRaylibPos(sptr->p);
		Vector3 fw = buildToRaylibPos(sptr->r);
		Vector3 rg = buildToRaylibPos(sptr->d);
		Vector3 dw = buildToRaylibPos(sptr->f);
		float l = Vector3Length(rg);
		Vector3 bbmin = pos + Vector3{l,l,l};
		Vector3 bbmax = pos - Vector3{l,l,l};
		DrawBoundingBox({bbmax,bbmin}, LIME);
		//   addto(&map->spri[focusedSprite].tr.p,scaled(right,mv));
		DrawPoint3D(buildToRaylibPos(hoverfoc.hitpos), RED);
	}
}
void EditorFrame() {
	if (ctx.mode == Fly) {
		if (IsKeyPressed(KEY_G)) {
			if (grabfoc.spri >=0) {
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
	if (ctxprev.mode != ctx.mode){}

	//ctx.update();
}
#endif //RAYLIB_LUA_IMGUI_DUMBEDIT_HPP