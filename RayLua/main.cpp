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
#include "rlgl.h"
#include "rlImGui.h"
#include "imgui.h"


#include "FileWatcher.h"
#include <math.h>
#include <chrono>

#include "DumbRender.hpp"
//#include "MonoTest.hpp"
//#include "luabinder.hpp"
#include "DumbCore.hpp"
#include "DumbEdit.hpp"
#include "MonoTest.hpp"
#include "raymath.h"
#include "cmake-build-custom/_deps/raylib-src/src/external/glad.h"
#include "DukeGame/source/dukewrap.h"


extern "C" {
#include "Core/loaders.h"
#include "Core/artloader.h"

}

// Profiling variables
double luaRenderTime = 0.0;
double luaUITime = 0.0;
double drawingTime = 0.0;
bool renderOpaque = false;

typedef struct {
    unsigned int fbo;
    unsigned int colorTexture;
    unsigned int depthTexture;
    int width, height;
} CustomRenderTarget;

Shader lutShader = {0};
Texture2D lutTexture = {0};
float lutIntensity = 1.0f;
CustomRenderTarget finalTarget = {0};

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

int lutTextureLocation;
typedef struct {
    Vector3 position;
    Vector3 target;
    Vector3 up;
    float speed;
} FreeCamera;
void DrawImgui()
{
    rlImGuiBegin();
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 display_size = io.DisplaySize;

    // Create invisible window at bottom
    ImGui::SetNextWindowPos(ImVec2(10, display_size.y - 50), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background

    ImGui::Begin("##overlay", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Its time to write text and print spritenum!");
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

void VisualizeMapstate() {  //unused
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
    Camera2D camera2D = {0};
    camera2D.target = {0.0f, 0.0f};        // What the camera is looking at
    camera2D.offset = {512.0f, 384.0f};    // Camera offset (screen center)
    camera2D.rotation = 0.0f;              // Camera rotation
    camera2D.zoom = 1.0f;                  // Camera zoom
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

CustomRenderTarget CreateCustomRenderTarget(int width, int height, unsigned int sharedDepth) {
    CustomRenderTarget target = {0};
    target.width = width;
    target.height = height;

    // Generate framebuffer
    glGenFramebuffers(1, &target.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);

    // Create color texture
    glGenTextures(1, &target.colorTexture);
    glBindTexture(GL_TEXTURE_2D, target.colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.colorTexture, 0);

    // Use shared depth or create new one
    if (sharedDepth != 0) {
        target.depthTexture = sharedDepth;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sharedDepth, 0);
    } else {
        glGenTextures(1, &target.depthTexture);
        glBindTexture(GL_TEXTURE_2D, target.depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, target.depthTexture, 0);
    }

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        TraceLog(LOG_ERROR, "Framebuffer not complete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return target;
}

void BeginCustomRenderTarget(CustomRenderTarget target) {
    rlDrawRenderBatchActive();
    glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);
    glViewport(0, 0, target.width, target.height);
}

void EndCustomRenderTarget() {
    rlDrawRenderBatchActive();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, GetScreenWidth(), GetScreenHeight());
}

void UnloadCustomRenderTarget(CustomRenderTarget target) {
    glDeleteTextures(1, &target.colorTexture);
    glDeleteFramebuffers(1, &target.fbo);
    // Don't delete shared depth texture here
}

void InitLUTSystem() {
    // Load LUT shader
    lutShader = LoadShader(0, "Shaders/lut.frag");

    // Load LUT texture
    lutTexture = LoadTexture("Shaders/lut.png");
    SetTextureWrap(lutTexture, TEXTURE_WRAP_CLAMP);
    SetTextureFilter(lutTexture, TEXTURE_FILTER_POINT);
    // Set shader uniforms
    lutTextureLocation = GetShaderLocation(lutShader, "lutTexture");
    int lutIntensityLocation = GetShaderLocation(lutShader, "lutIntensity");

   // SetShaderValue(lutShader, lutIntensityLocation, &lutIntensity, SHADER_UNIFORM_FLOAT);
    SetShaderValueTexture(lutShader, lutTextureLocation, lutTexture);

    // Create final render target for post-processing
    finalTarget = CreateCustomRenderTarget(GetScreenWidth(), GetScreenHeight(), 0);
}

// Add this function to cleanup LUT resources
void CleanupLUTSystem() {
    UnloadShader(lutShader);
    UnloadTexture(lutTexture);
    UnloadCustomRenderTarget(finalTarget);
}
void DrawInfoUI() {
    rlImGuiBegin();

    // Get main viewport size
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;

    // Set window position to upper right
    ImVec2 window_pos = ImVec2(work_pos.x + work_size.x - 200.0f, work_pos.y + 10.0f);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);

    // Create window without title bar, resize, move, collapse
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_AlwaysAutoResize;

    ImGui::Begin("##info_panel", NULL, window_flags);
    ImGui::Text("press G to move");
    ImGui::Text("press Q to pickmove");
    ImGui::Text("press ` to discard");
    ImGui::End();

    rlImGuiEnd();
}
// Draw palette and texture preview on screen
void MainLoop()
{
    DisableCursor();

    DumbRender::Init();
    auto map = DumbRender::GetMap();
    DumbCore::Init(map);
    SetTargetFPS(60);
    DumbRender::LoadTexturesToGPU();
    InitEditor(map);
    // Initialize LUT system
    InitLUTSystem();

    // Create render targets with shared depth
    CustomRenderTarget albedoTarget = CreateCustomRenderTarget(GetScreenWidth(), GetScreenHeight(), 0);
    CustomRenderTarget lightTarget = CreateCustomRenderTarget(GetScreenWidth(), GetScreenHeight(), albedoTarget.depthTexture);
    CustomRenderTarget combinedTarget = CreateCustomRenderTarget(GetScreenWidth(), GetScreenHeight(), 0);
    int w = GetScreenWidth();
    int h = GetScreenHeight();
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        DumbCore::Update(deltaTime);
        EditorFrameMin();
        // Render albedo pass
        BeginCustomRenderTarget(albedoTarget);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        ClearBackground(BLACK);

        BeginMode3D(*DumbCore::GetCamera());
        DumbRender::ProcessKeys();
        DumbRender::DrawKenGeometry(GetScreenWidth(), GetScreenHeight(), DumbCore::GetCamera());
        DumbRender::DrawMapstateTex(*DumbCore::GetCamera());
        EndMode3D();
        EndCustomRenderTarget();

        // Render light pass
        BeginCustomRenderTarget(lightTarget);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        // batching improves perf significantly, so atlases are way to go
        DumbRender::DrawLightsPost3d(w,h,*DumbCore::GetCamera());
       // DumbRender::DrawPost3d(w,h,*DumbCore::GetCamera());
        glDepthMask(GL_TRUE);
        EndCustomRenderTarget();

        // Combine albedo and lights
        BeginCustomRenderTarget(combinedTarget);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        // Draw albedo
        DrawTextureRec({albedoTarget.colorTexture, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8},
                      {0, 0, (float)w, (float)-h}, {0, 0}, WHITE);

        // Multiply blend lights
        BeginBlendMode(RL_BLEND_ADDITIVE);

        DrawTextureRec({lightTarget.colorTexture, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8},
                      {0, 0, (float)w, (float)-h}, {0, 0}, WHITE);
        EndBlendMode();
        DrawFPS(10, 10);
        EndCustomRenderTarget();

        // Apply LUT to final result
        BeginCustomRenderTarget(finalTarget);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);


        BeginShaderMode(lutShader);
        SetShaderValueTexture(lutShader, lutTextureLocation, lutTexture);
        DrawTextureRec({combinedTarget.colorTexture, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8},
                      {0, 0, (float)w, (float)-h}, {0, 0}, WHITE);
        EndShaderMode();

        EndCustomRenderTarget();

        // Final draw to screen
        BeginDrawing();
        ClearBackground(BLACK);

        DrawTextureRec({finalTarget.colorTexture, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8},
                      {0, 0, (float)w, (float)-h}, {0, 0}, WHITE);
        DrawInfoUI();
        DisableCursor();
        EndDrawing();
    }

    // Cleanup
    glDeleteTextures(1, &albedoTarget.depthTexture);
    UnloadCustomRenderTarget(albedoTarget);
    UnloadCustomRenderTarget(lightTarget);
    UnloadCustomRenderTarget(combinedTarget);
    CleanupLUTSystem();
    DumbRender::CleanupMapstateTex();
}

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1024, 768, "Raylib + Lua + ImGui");
    SetTargetFPS(120);
    rlImGuiSetup(true);
   // LuaBinder::Init();
    SetImguiFonts();

   // FileWatcher watcher("script.lua");
    //LuaBinder::LoadScript();

    //VisualizeMapstate();
    MainLoop();
    //DumbRender::CleanupMapstateTex();
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
