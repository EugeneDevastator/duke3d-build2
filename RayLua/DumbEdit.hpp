//
// Created by omnis on 1/7/2026.
//

#ifndef RAYLIB_LUA_IMGUI_DUMBEDIT_HPP
#define RAYLIB_LUA_IMGUI_DUMBEDIT_HPP
#include "buildmath.h"
#include "DumbRender.hpp"
#include "raylib.h"

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


extern "C"{
#include "shadowtest2.h"
}
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

econtext ctx;
econtext ctxprev;
int savedfocus;

// ------------------ PICKGRAB
transform savedtr;
transform trdiff;
void PickgrabDiscard() {
	map->spri[savedfocus].tr= savedtr;
}
void PickgrabAccept() {

	int s = map->spri[savedfocus].sect;
	updatesect_p(map->spri[savedfocus].p, &s, map);
	changesprisect_imp(savedfocus, s, map);
	savedfocus = -1; // jsut do noting now.
	ctx.mode = Fly;
}
void PickgrabUpdate() {
	Vector2 dmov = GetMouseDelta();
	float scrol = GetMouseWheelMove();
	addto(&trdiff.p, scaled(down,scrol*0.2f));
	map->spri[savedfocus].tr = local_to_world_transform_p(trdiff, &plr.tri);
	if (IsKeyPressed(K_PICKGRAB)) {
		PickgrabAccept();
		savedfocus = -1;
		ctx.op = accept;
	}
}
void PickgrabStart() {
	savedfocus = focusedSprite;
	savedtr = map->spri[focusedSprite].tr;
	trdiff =  world_to_local_transform_p(map->spri[focusedSprite].tr, &plr.tri);
}
// ----------------------- MOVE OPER ---------------------
point3d savedpos;

void MoveObjContextUpdate() {
Vector2 dmov = GetMouseDelta();
	float scrol = GetMouseWheelMove();
	map->spri[focusedSprite].p.x+=dmov.x*0.5f;
	map->spri[focusedSprite].p.y+=dmov.y*0.5f;
	map->spri[focusedSprite].p.z+=scrol*0.5f;
}
void MoveObjContextDiscard() {
	map->spri[focusedSprite].p = savedpos;
}
void MoveObjContextAccept() {
	savedfocus = -1; // jsut do noting now.
}
void MoveObjContextStart() {
	savedfocus = focusedSprite;
	savedpos = map->spri[focusedSprite].p;
}

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

void EditorFrameMin() {
	ctx.state.update();

	if (focusedSprite >= 0) {
		if (IsKeyPressed(K_PICKGRAB)) {
			ctx.state = PickGrabState;
			ctx.state.start();
		}
	}


	if (IsKeyPressed(K_DISCARD)) {
		ctx.state.discard();
		ctx.state = Empty;
	}
	if (IsKeyPressed(K_ACCEPT) || ctx.op == accept) {
		ctx.state.accept();
		ctx.state = Empty;
		ctx.op = noop;
	}
}
void EditorFrame() {
	if (ctx.mode == Fly) {
		if (IsKeyPressed(KEY_G)) {
			if (focusedSprite >=0) {
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