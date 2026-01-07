//
// Created by omnis on 1/7/2026.
//

#ifndef RAYLIB_LUA_IMGUI_DUMBEDIT_HPP
#define RAYLIB_LUA_IMGUI_DUMBEDIT_HPP
#include "rayl.h"
#include "raylib.h"
// in initial selection mode:
// ctrl - select similar - wall same tiles, sprites same tile
// shift - all walls of sec, floor/ceil, all sprites of sec.
// grave - paint selection mode

// in modify mode:
// ctrl - auto snap to somewhere
// shift - slow down.
// grave - do increments


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


static mapstate_t *map;
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
} viewmodel editormodel = {};

econtext ctx;
econtext ctxprev;


// ----------------------- MOVE OPER ---------------------
point3d savedpos;
int savedfocus;
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
const estate Empty = { 0, empty, empty, empty, empty};
// ----------------- MAIN
void InitEditor(mapstate_t *m) {
	map = m;
	ctx.mode = Fly;
	ctx.state = FlyState;
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

	ctx.update();
}
#endif //RAYLIB_LUA_IMGUI_DUMBEDIT_HPP