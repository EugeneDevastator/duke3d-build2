//#include "kplib.h"
// notes for porting:
// raylib bridge.h : has drawDukeSprite(..) and doUpdate(dt)
// raylib bridge .cpp
// dukewrapper.h
// call duke.init
// duke init calls bridge
// call bridge.update or smth to update.
// duke is built behind initializer wrapper
// bridge is also build behind call forwarding.


#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"


#include "FileWatcher.h"
#include <math.h>
#include <chrono>

#include "DumbRender.hpp"
//#include "luabinder.hpp"
#include "DumbCore.hpp"
#include "raymath.h"


extern "C" {
#include "Core/loaders.h"
#include "Core/artloader.h"

}

// Profiling variables
double luaRenderTime = 0.0;
double luaUITime = 0.0;
double drawingTime = 0.0;
bool renderOpaque = false;


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

void VisualizeMapstate() {
    DumbRender::Init();

    auto map = DumbRender::GetMap();
    //InitWindow(1024, 768, "Mapstate Visualizer");
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

        DumbRender::DrawMapstateTex(camera);
     //   DumbRender::DrawMapstateLines();
        EndMode3D();
       // DumbRender::DrawPaletteAndTexture();

//DumbRender::TestRenderTextures();

        DrawImgui();
        DisableCursor();
        DrawText("WASD: Move, Mouse: Look", 10, 10, 20, WHITE);
        DrawFPS(10, 40);

        EndDrawing();
    }

    CloseWindow();
}

// Draw palette and texture preview on screen
void MainLoop()
{
    DumbRender::Init();
    auto map = DumbRender::GetMap();
    DumbCore::Init(map);
    //InitWindow(1024, 768, "Mapstate Visualizer");
    SetTargetFPS(60);
    DumbRender::LoadTexturesToGPU();
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        DumbCore::Update(deltaTime);

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(DumbCore::GetCamera());


        DumbRender::DrawMapstateTex(DumbCore::GetCamera());
        //   DumbRender::DrawMapstateLines();
        EndMode3D();
        // DumbRender::DrawPaletteAndTexture();

        //DumbRender::TestRenderTextures();

        DrawImgui();
        DisableCursor();
        DrawText("WASD: Move, Mouse: Look", 10, 10, 20, WHITE);
        DrawFPS(10, 40);

        EndDrawing();
    }
}

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(800, 600, "Raylib + Lua + ImGui");
    SetTargetFPS(120);
    rlImGuiSetup(true);
   // LuaBinder::Init();
    SetImguiFonts();

   // FileWatcher watcher("script.lua");
    //LuaBinder::LoadScript();

    //VisualizeMapstate();
    MainLoop();
    DumbRender::CleanupMapstateTex();
    return 0;
    //MapTest();

    while (!WindowShouldClose()) {
       // if (watcher.HasChanged()) {
          //  LuaBinder::LoadScript();
       // }

        if (IsKeyPressed(KEY_R)) {
          //  LuaBinder::LoadScript();
        }
        // Add the E key check here
        if (IsKeyPressed(KEY_E)) {
            system("notepad script.lua");
        }

        auto frameStart = std::chrono::high_resolution_clock::now();

        BeginDrawing();
        ClearBackground(DARKGRAY);



        // Drawing phase timing
        auto drawStart = std::chrono::high_resolution_clock::now();

     //   for (const auto& rect : transparentRects) {
     //       Color drawColor = rect.color;
     //       if (renderOpaque) {
     //           drawColor.a = 255;
     //       }
     //       DrawRectangle(rect.x, rect.y, rect.width, rect.height, drawColor);
     //   }

        // Lua Render timing
        auto luaRenderStart = std::chrono::high_resolution_clock::now();
      //  LuaBinder::DoSceneUpdate();
        auto luaRenderEnd = std::chrono::high_resolution_clock::now();
        luaRenderTime = std::chrono::duration<double, std::milli>(luaRenderEnd - luaRenderStart).count();


        // UI phase timing
        rlImGuiBegin();

        auto luaUIStart = std::chrono::high_resolution_clock::now();
     //   LuaBinder::DoUpdate();
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
      //  DrawPaletteAndTexture(paltex,tex,600,600);
        EndDrawing();
    }

    rlImGuiShutdown();
 //!  lua_close(L);
    CloseWindow();
    return 0;
}
