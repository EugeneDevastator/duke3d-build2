//
// Created by omnis on 2/9/2026.
//

#ifndef RAYLIB_LUA_IMGUI_IHUDVIEW_H
#define RAYLIB_LUA_IMGUI_IHUDVIEW_H
#include "../DumbEdit.hpp"

// INTERFACE FOR EDITOR HUD
typedef struct EditorView {
	void (*Start)(void);
	void (*Update)(void);
	void (*Accept)(void);
	void (*Discard)(void);
	void (*DrawImgui)(void);
	void (*DrawGizmos)(void);
	void (*DrawInScene)(void);
} EditorView;


typedef struct {
	signed int wal;
	signed int wal2;
	signed int walprev;
	int onewall;
	int sec;
	int spri;
	int surf;
	point3d hitpos;
} focus_t;

typedef struct {
	long sect;
	int wal; // -1 = ceil -2 = fllor;
	point3d pos;
} looploc_t; // draw loop point

typedef struct {
	transform cursor[2];
	focus_t hover;
	focus_t grab;
	looploc_t loopts[100];
	point2d loopts_p2d[100];
	int loopn = 0;
} EditorModel;

void EditorHudDrawTopInfo(const char* message);

#endif //RAYLIB_LUA_IMGUI_IHUDVIEW_H
