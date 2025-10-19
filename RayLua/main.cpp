#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"
#include "FileWatcher.h"
#include <vector>
#include <chrono>

#include "external/miniaudio.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

struct TransparentRect {
    float x, y, width, height;
    Color color;
};

std::vector<TransparentRect> transparentRects;

// Profiling variables
double luaRenderTime = 0.0;
double luaUITime = 0.0;
double drawingTime = 0.0;
bool renderOpaque = false;

int lua_DrawRectangle(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int w = lua_tointeger(L, 3);
    int h = lua_tointeger(L, 4);
    DrawRectangle(x, y, w, h, RED);
    return 0;
}

int lua_SpawnTransparentRect(lua_State* L) {
    float x = lua_tonumber(L, 1);
    float y = lua_tonumber(L, 2);
    float w = lua_tonumber(L, 3);
    float h = lua_tonumber(L, 4);
    int r = lua_tointeger(L, 5);
    int g = lua_tointeger(L, 6);
    int b = lua_tointeger(L, 7);
    int a = lua_tointeger(L, 8);

    TransparentRect rect = {x, y, w, h, {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a}};
    transparentRects.push_back(rect);

    lua_pushinteger(L, transparentRects.size() - 1);
    return 1;
}

int lua_SetRectPosition(lua_State* L) {
    int index = lua_tointeger(L, 1);
    float x = lua_tonumber(L, 2);
    float y = lua_tonumber(L, 3);

    if (index >= 0 && index < transparentRects.size()) {
        transparentRects[index].x = x;
        transparentRects[index].y = y;
    }
    return 0;
}

int lua_SetRectSize(lua_State* L) {
    int index = lua_tointeger(L, 1);
    float w = lua_tonumber(L, 2);
    float h = lua_tonumber(L, 3);

    if (index >= 0 && index < transparentRects.size()) {
        transparentRects[index].width = w;
        transparentRects[index].height = h;
    }
    return 0;
}

int lua_SetRectColor(lua_State* L) {
    int index = lua_tointeger(L, 1);
    int r = lua_tointeger(L, 2);
    int g = lua_tointeger(L, 3);
    int b = lua_tointeger(L, 4);
    int a = lua_tointeger(L, 5);

    if (index >= 0 && index < transparentRects.size()) {
        transparentRects[index].color = {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a};
    }
    return 0;
}

int lua_DeleteRect(lua_State* L) {
    int index = lua_tointeger(L, 1);

    if (index >= 0 && index < transparentRects.size()) {
        transparentRects.erase(transparentRects.begin() + index);
    }
    return 0;
}

int lua_ClearAllRects(lua_State* L) {
    transparentRects.clear();
    return 0;
}

int lua_GetRectCount(lua_State* L) {
    lua_pushinteger(L, transparentRects.size());
    return 1;
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
    ImGui::Begin(title,NULL, 0);
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

int lua_GetKeyPressed(lua_State* L) {
    int key = lua_tointeger(L, 1);
    lua_pushboolean(L, IsKeyPressed(key));
    return 1;
}

int lua_GetTime(lua_State* L) {
    lua_pushnumber(L, GetTime());
    return 1;
}

void LoadScript(lua_State* L) {
    if (luaL_dofile(L, "script.lua") != LUA_OK) {
        printf("Error loading script.lua: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);

        luaL_dostring(L, R"(
        function Render()
            DrawRectangle(100, 100, 200, 150)
        end

        function RenderUI()
            ImGuiBegin("Test Window")
            ImGuiText("Hello from Lua!")
            ImGuiText("Mouse: " .. GetMouseX() .. ", " .. GetMouseY())
            ImGuiText("Rects: " .. GetRectCount())
            ImGuiEnd()
        end
        )");
    } else {
        printf("Script reloaded successfully\n");
    }
}

void SetImguiFonts()
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 28.0f);
    if (font == nullptr) {
        ImFontConfig config;
        config.SizePixels = 18.0f;
        config.PixelSnapH = true;
        io.Fonts->AddFontDefault(&config);
    }
    io.Fonts->Build();
}

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(800, 600, "Raylib + Lua + ImGui");
    SetTargetFPS(120);
    rlImGuiSetup(true);

    SetImguiFonts();

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    lua_register(L, "DrawRectangle", lua_DrawRectangle);
    lua_register(L, "SpawnTransparentRect", lua_SpawnTransparentRect);
    lua_register(L, "SetRectPosition", lua_SetRectPosition);
    lua_register(L, "SetRectSize", lua_SetRectSize);
    lua_register(L, "SetRectColor", lua_SetRectColor);
    lua_register(L, "DeleteRect", lua_DeleteRect);
    lua_register(L, "ClearAllRects", lua_ClearAllRects);
    lua_register(L, "GetRectCount", lua_GetRectCount);
    lua_register(L, "GetMouseX", lua_GetMouseX);
    lua_register(L, "GetMouseY", lua_GetMouseY);
    lua_register(L, "ImGuiBegin", lua_ImGuiBegin);
    lua_register(L, "ImGuiEnd", lua_ImGuiEnd);
    lua_register(L, "ImGuiText", lua_ImGuiText);
    lua_register(L, "GetKeyPressed", lua_GetKeyPressed);
    lua_register(L, "GetTime", lua_GetTime);

    FileWatcher watcher("script.lua");
    LoadScript(L);

    while (!WindowShouldClose()) {
        if (watcher.HasChanged()) {
            LoadScript(L);
        }

        if (IsKeyPressed(KEY_R)) {
            LoadScript(L);
        }

        auto frameStart = std::chrono::high_resolution_clock::now();

        BeginDrawing();
        ClearBackground(DARKGRAY);

        // Drawing phase timing
        auto drawStart = std::chrono::high_resolution_clock::now();

        for (const auto& rect : transparentRects) {
            Color drawColor = rect.color;
            if (renderOpaque) {
                drawColor.a = 255;
            }
            DrawRectangle(rect.x, rect.y, rect.width, rect.height, drawColor);
        }


        // Lua Render timing
        auto luaRenderStart = std::chrono::high_resolution_clock::now();
        lua_getglobal(L, "Render");
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            printf("Render error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
        auto luaRenderEnd = std::chrono::high_resolution_clock::now();
        luaRenderTime = std::chrono::duration<double, std::milli>(luaRenderEnd - luaRenderStart).count();

        // UI phase timing
        rlImGuiBegin();
        auto luaUIStart = std::chrono::high_resolution_clock::now();
        lua_getglobal(L, "RenderUI");
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            printf("RenderUI error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
        auto luaUIEnd = std::chrono::high_resolution_clock::now();
        luaUITime = std::chrono::duration<double, std::milli>(luaUIEnd - luaUIStart).count();

        // Profiling window
        ImGui::Begin("Profiling");
        ImGui::Text("Lua Render: %.3f ms", luaRenderTime);
        ImGui::Text("Lua UI: %.3f ms", luaUITime);
        ImGui::Text("Drawing: %.3f ms", drawingTime);
        ImGui::Text("Total Lua: %.3f ms", luaRenderTime + luaUITime);
        ImGui::Checkbox("Render Opaque", &renderOpaque);
        ImGui::End();

        rlImGuiEnd();

        auto drawEnd = std::chrono::high_resolution_clock::now();
        drawingTime = std::chrono::duration<double, std::milli>(drawEnd - drawStart).count();

        DrawFPS(10, 10);

        EndDrawing();
    }

    rlImGuiShutdown();
    lua_close(L);
    CloseWindow();
    return 0;
}
