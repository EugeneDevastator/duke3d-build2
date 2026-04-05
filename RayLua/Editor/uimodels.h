//
// Created by omnis on 2/4/2026.
//

#ifndef RAYLIB_LUA_IMGUI_UIMODELS_H
#define RAYLIB_LUA_IMGUI_UIMODELS_H
typedef struct {
	bool shown;
	int totalCount;
	int selected;
	int columns;
	float thumbnailSize;
	int startRow;
	int tilesPerPage;
	int maxVisibleRows;
	int galnum;
	int edgeDirection;
	int showSettings;
} TextureBrowser;

#endif //RAYLIB_LUA_IMGUI_UIMODELS_H