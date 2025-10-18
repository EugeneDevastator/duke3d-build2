main.c
#include "raylib.h"
#include "rlImGui.h"
#include <luajit-2.1/lua.h>
#include <luajit-2.1/lualib.h>
#include <luajit-2.1/lauxlib.h>

// Expose functions to Lua
int lua_DrawRectangle(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int w = lua_tointeger(L, 3);
    int h = lua_tointeger(L, 4);
    DrawRectangle(x, y, w, h, RED);
    return 0;
}

int lua_GetMouseX(lua_State* L) {
    lua_pushinteger(L, GetMouseX());
    return 1;
}

int lua_GetMouseY(lua_State* L) {
    lua_pushinteger(L, GetMouseY());
    return 1;
}

int lua_ImGuiBegin(lua_State* L) {
    const char* title = lua_tostring(L, 1);
    ImGui::Begin(title);
    return 0;
}

int lua_ImGuiEnd(lua_State* L) {
    ImGui::End();
    return 0;
}

int lua_ImGuiText(lua_State* L) {
    const char* text = lua_tostring(L, 1);
    ImGui::Text("%s", text);
    return 0;
}

int main() {
    InitWindow(800, 600, "Raylib + Lua + ImGui");
    SetTargetFPS(60);
    rlImGuiSetup(true);
    
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    
    // Register functions
    lua_register(L, "DrawRectangle", lua_DrawRectangle);
    lua_register(L, "GetMouseX", lua_GetMouseX);
    lua_register(L, "GetMouseY", lua_GetMouseY);
    lua_register(L, "ImGuiBegin", lua_ImGuiBegin);
    lua_register(L, "ImGuiEnd", lua_ImGuiEnd);
    lua_register(L, "ImGuiText", lua_ImGuiText);
    
    luaL_dofile(L, "script.lua");
    
    while (!WindowShouldClose()) {
        // Reload script on R key
        if (IsKeyPressed(KEY_R)) {
            luaL_dofile(L, "script.lua");
        }
        
        BeginDrawing();
        ClearBackground(DARKGRAY);
        
        // Call Lua render
        lua_getglobal(L, "Render");
        lua_pcall(L, 0, 0, 0);
        
        // ImGui
        rlImGuiBegin();
        lua_getglobal(L, "RenderUI");
        lua_pcall(L, 0, 0, 0);
        rlImGuiEnd();
        
        EndDrawing();
    }
    
    rlImGuiShutdown();
    lua_close(L);
    CloseWindow();
    return 0;
}