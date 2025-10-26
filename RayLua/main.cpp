//#include "kplib.h"

#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"


#include "FileWatcher.h"
#include <math.h>
#include <chrono>

#include "raymath.h"
#include "external/miniaudio.h"
extern "C" {
#include "Core/loaders.h"
#include "Core/artloader.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

static mapstate_t *map;
struct TransparentRect {
    float x, y, width, height;
    Color color;
};
void MapTest()
{
    map = static_cast<mapstate_t*>(malloc(sizeof(mapstate_t)));
    memset(map,0,sizeof(mapstate_t));
    initcrc32();

    gnumtiles = 0; memset(gtilehashead,-1,sizeof(gtilehashead));
    gmaltiles = 256;
    gtile = (tile_t *)malloc(gmaltiles*sizeof(tile_t)); if (!gtile) return;
    //memset(gtile,0,gmaltiles*sizeof(tile_t)); //FIX

    map->numsects = 0;
    map->malsects = 256;
    map->sect = static_cast<sect_t*>(malloc(map->malsects * sizeof(sect_t))); if (!map->sect) return;
    memset(map->sect,0,map->malsects*sizeof(sect_t));

    map->numspris = 0;
    map->malspris = 256;
    map->spri = static_cast<spri_t*>(malloc(map->malspris * sizeof(spri_t))); if (!map->spri) return;
    memset(map->spri,0,map->malspris*sizeof(spri_t));
    map->blankheadspri = -1;

    map->blankheadspri = -1;
    for(int i=0;i<map->malspris;i++)
    {
        map->spri[i].sectn = map->blankheadspri;
        map->spri[i].sectp = -1;
        map->spri[i].sect = -1;
        if (map->blankheadspri >= 0) map->spri[map->blankheadspri].sectp = i;
        map->blankheadspri = i;
    }


   loadmap_imp((char*)"c:/Eugene/Games/build2/E2L7.MAP",map);

int a =1;
}

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


typedef struct {
    Vector3 position;
    Vector3 target;
    Vector3 up;
    float speed;
} FreeCamera;

void DrawMapstate(mapstate_t* map) {
    // Draw sectors
    for (int s = 0; s < map->numsects; s++) {
        sect_t* sect = &map->sect[s];

        // Draw walls as lines
        for (int w = 0; w < sect->n; w++) {
            wall_t* wall = &sect->wall[w];
            wall_t* nextwall = &sect->wall[(w + 1) % sect->n];

            Vector3 start = {wall->x, sect->z[0], wall->y};
            Vector3 end = {nextwall->x, sect->z[0], nextwall->y};

            // Floor outline
            DrawLine3D(start, end, WHITE);

            // Ceiling outline
            start.y = sect->z[1];
            end.y = sect->z[1];
            DrawLine3D(start, end, GRAY);

            // Vertical wall lines
            Vector3 bottom = {wall->x, sect->z[0], wall->y};
            Vector3 top = {wall->x, sect->z[1], wall->y};
            DrawLine3D(bottom, top, LIGHTGRAY);
        }
    }

    // Draw sprites as simple cubes
    for (int i = 0; i < map->numspris; i++) {
        spri_t* spr = &map->spri[i];
        Vector3 pos = {spr->p.x, spr->p.z, spr->p.y};
        float s = 0.1f;
        DrawCubeWires(pos, s, s, s, DARKBLUE);
    }
}
void DrawImgui()
{
    rlImGuiBegin();
    ImGui::Begin("Profiling");
    ImGui::Text("Lets f..n go!");
    ImGui::End();
    rlImGuiEnd();
}
void UpdateFreeCamera(FreeCamera* cam, float deltaTime) {
    float speed = cam->speed * deltaTime;

    Vector3 forward = Vector3Normalize(Vector3Subtract(cam->target, cam->position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, cam->up));

    // WASD movement
    if (IsKeyDown(KEY_W)) {
        cam->position = Vector3Add(cam->position, Vector3Scale(forward, speed));
        cam->target = Vector3Add(cam->target, Vector3Scale(forward, speed));
    }
    if (IsKeyDown(KEY_S)) {
        cam->position = Vector3Subtract(cam->position, Vector3Scale(forward, speed));
        cam->target = Vector3Subtract(cam->target, Vector3Scale(forward, speed));
    }
    if (IsKeyDown(KEY_A)) {
        cam->position = Vector3Subtract(cam->position, Vector3Scale(right, speed));
        cam->target = Vector3Subtract(cam->target, Vector3Scale(right, speed));
    }
    if (IsKeyDown(KEY_D)) {
        cam->position = Vector3Add(cam->position, Vector3Scale(right, speed));
        cam->target = Vector3Add(cam->target, Vector3Scale(right, speed));
    }

    // Mouse look
    Vector2 mouseDelta = GetMouseDelta();
    if (mouseDelta.x != 0 || mouseDelta.y != 0) {
        float sensitivity = 0.003f;

        // Horizontal rotation
        Vector3 targetOffset = Vector3Subtract(cam->target, cam->position);
        targetOffset = Vector3RotateByAxisAngle(targetOffset, cam->up, -mouseDelta.x * sensitivity);

        // Vertical rotation
        targetOffset = Vector3RotateByAxisAngle(targetOffset, right, -mouseDelta.y * sensitivity);

        cam->target = Vector3Add(cam->position, targetOffset);
    }
}

void VisualizeMapstate(mapstate_t* map) {
    InitWindow(1024, 768, "Mapstate Visualizer");
    SetTargetFPS(60);


    FreeCamera cam = {0};
    cam.position = {map->startpos.x, map->startpos.y, map->startpos.z};
    cam.target = Vector3Add(cam.position, {map->startfor.x, map->startfor.y, map->startfor.z});
    cam.up ={0, 1, 0};
    cam.speed = 50.0f;

    Camera3D camera = {0};

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        UpdateFreeCamera(&cam, deltaTime);

        camera.position = cam.position;
        camera.target = cam.target;
        camera.up = cam.up;
        camera.fovy = 60.0f;
        camera.projection = CAMERA_PERSPECTIVE;

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(camera);
        DrawMapstate(map);
        EndMode3D();
        DrawImgui();
        DisableCursor();
        DrawText("WASD: Move, Mouse: Look", 10, 10, 20, WHITE);
        DrawFPS(10, 40);

        EndDrawing();
    }

    CloseWindow();
}

Texture2D ConvertPalToTexture() {
    Image palImage = {0};
    palImage.width = 16;
    palImage.height = 16;
    palImage.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    palImage.mipmaps = 1;
    palImage.data = malloc(16 * 16 * 4);

    auto *pixels = static_cast<unsigned char*>(palImage.data);

    // Debug the actual memory

    int i = 6;
    printf("Direct memory read: %d %d %d %d\n",
           getColor(i+0), getColor(i+1),getColor(i+2),getColor(i+3));

    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            int colorIndex = y * 16 + x;
            int pixelIndex = (y * 16 + x) * 4;

            // Direct memory access instead of array indexing
            pixels[pixelIndex + 0] = getColor(colorIndex)[2]; // R
            pixels[pixelIndex + 1] = getColor(colorIndex)[1]; // G
            pixels[pixelIndex + 2] = getColor(colorIndex)[0]; // B
            pixels[pixelIndex + 3] = 255;        // A
        }
    }

    Texture2D texture = LoadTextureFromImage(palImage);
    UnloadImage(palImage);
    return texture;
}
// Convert PIC/ART tile to Raylib texture
Texture2D ConvertPicToTexture(tile_t *tpic) {
    if (!tpic || !tpic->tt.f) return {0};

    tiltyp *pic = &tpic->tt;

    Image picImage = {0};
    picImage.width = pic->x;
    picImage.height = pic->y;
    picImage.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    picImage.mipmaps = 1;

    // Allocate memory for image data
    picImage.data = malloc(pic->x * pic->y * 4);
    unsigned char *pixels = (unsigned char*)picImage.data;

    // Copy pixel data from pic format to RGBA
    for (int y = 0; y < pic->y; y++) {
        for (int x = 0; x < pic->x; x++) {
            int srcIndex = y * pic->p + (x << 2);
            int dstIndex = (y * pic->x + x) * 4;

            unsigned char *srcPixel = (unsigned char*)(pic->f+srcIndex);
            auto col = getColor(*srcPixel);
            pixels[dstIndex + 0] = col[2]; // R
            pixels[dstIndex + 1] = col[1]; // G
            pixels[dstIndex + 2] = col[0]; // B
            pixels[dstIndex + 3] = 255; // A
        }
    }

    Texture2D texture = LoadTextureFromImage(picImage);
    UnloadImage(picImage);
    return texture;
}

// Draw palette and texture preview on screen
void DrawPaletteAndTexture(Texture2D palTexture, Texture2D picTexture, int screenWidth, int screenHeight) {
  //  BeginDrawing();
  //  ClearBackground(DARKGRAY);

    // Draw palette in top-left corner
    if (palTexture.id > 0) {
        DrawTextureEx(palTexture, {10, 10}, 0.0f, 8.0f, WHITE);
        DrawText("PALETTE", 10, 150, 20, WHITE);
    }

    // Draw texture in center-right area
    if (picTexture.id > 0) {
        float scale = 1.0f;
        int maxSize = 400;

        // Scale texture to fit preview area
        if (picTexture.width > maxSize || picTexture.height > maxSize) {
            float scaleX = (float)maxSize / picTexture.width;
            float scaleY = (float)maxSize / picTexture.height;
            scale = (scaleX < scaleY) ? scaleX : scaleY;
        }

        Vector2 pos = {
            screenWidth - picTexture.width * scale - 20,
            (screenHeight - picTexture.height * scale) / 2
        };

        DrawTextureEx(picTexture, pos, 0.0f, scale, WHITE);

        // Draw texture info
        char info[256];
        sprintf(info, "SIZE: %dx%d", picTexture.width, picTexture.height);
        DrawText(info, (int)pos.x, (int)pos.y - 25, 20, WHITE);
    }

  //  EndDrawing();
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
    char rootpath[256];
    strcpy_s(rootpath, "c:/Eugene/Games/build2/");
    LoadPal(rootpath);
    auto paltex = ConvertPalToTexture();
    tile_t* pic = static_cast<tile_t*>(malloc(sizeof(tile_t)));
    strcpy_s(pic->filnam, "TILES000.art|1");
    loadpic(pic,rootpath);


    auto tex = ConvertPicToTexture(pic);

    //MapTest();

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


        //VisualizeMapstate(map);
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
        DrawPaletteAndTexture(paltex,tex,1000,1000);
        EndDrawing();
    }

    rlImGuiShutdown();
    lua_close(L);
    CloseWindow();
    return 0;
}
