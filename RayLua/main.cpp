#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"
#include "FileWatcher.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

// Keep your existing Lua functions...
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

void LoadScript(lua_State* L) {
    if (luaL_dofile(L, "script.lua") != LUA_OK) {
        printf("Error loading script.lua: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1); // Remove error message

        // Create default functions
        luaL_dostring(L, R"(
        function Render()
            DrawRectangle(100, 100, 200, 150)
        end

        function RenderUI()
            ImGuiBegin("Test Window")
            ImGuiText("Hello from Lua!")
            ImGuiText("Mouse: " .. GetMouseX() .. ", " .. GetMouseY())
            ImGuiEnd()
        end
        )");
    } else {
        printf("Script reloaded successfully\n");
    }
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

    FileWatcher watcher("script.lua");
    LoadScript(L);

    while (!WindowShouldClose()) {
        // Auto-reload on file change
        if (watcher.HasChanged()) {
            LoadScript(L);
        }

        // Manual reload on R key
        if (IsKeyPressed(KEY_R)) {
            LoadScript(L);
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);

        // Call Lua render with error handling
        lua_getglobal(L, "Render");
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            printf("Render error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }

        // ImGui
        rlImGuiBegin();
        lua_getglobal(L, "RenderUI");
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            printf("RenderUI error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
        rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    lua_close(L);
    CloseWindow();
    return 0;
}
