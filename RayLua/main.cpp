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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

extern "C" {
#include "Core/loaders.h"
#include "Core/artloader.h"

}

typedef struct {
    Texture2D* textures;
    int totalCount;
    int selected;
    int columns;
    float thumbnailSize;
    int startIndex;
    int tilesPerPage;
} TextureBrowser;

void DrawTextureBrowser(TextureBrowser* browser) {
    // Position on left side
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, GetScreenHeight()), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Texture Browser")) {
        ImGui::End();
        return;
    }

    // Handle scroll wheel for page navigation
    if (ImGui::IsWindowHovered()) {
        float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f) {
            int pageSize = browser->columns;
            browser->startIndex -= (int)(wheel * pageSize);

            // Clamp startIndex
            if (browser->startIndex < 0) browser->startIndex = 0;
            int maxStart = browser->totalCount - browser->tilesPerPage;
            if (maxStart < 0) maxStart = 0;
            if (browser->startIndex > maxStart) browser->startIndex = maxStart;
        }
    }

    // Controls
    ImGui::SliderInt("Columns", &browser->columns, 1, 6);
    ImGui::SliderFloat("Size", &browser->thumbnailSize, 32.0f, 128.0f);
    ImGui::SliderInt("Max Visible", &browser->tilesPerPage, 10, 200);

    // Navigation
    int maxStart = browser->totalCount - browser->tilesPerPage;
    if (maxStart < 0) maxStart = 0;
    if (ImGui::SliderInt("Start Index", &browser->startIndex, 0, maxStart)) {
        if (browser->startIndex < 0) browser->startIndex = 0;
        if (browser->startIndex > maxStart) browser->startIndex = maxStart;
    }

    int actualEnd = browser->startIndex + browser->tilesPerPage;
    if (actualEnd > browser->totalCount) actualEnd = browser->totalCount;

    ImGui::Text("Showing %d-%d of %d",
                browser->startIndex + 1,
                actualEnd,
                browser->totalCount);
    ImGui::Separator();

    if (browser->totalCount == 0) {
        ImGui::Text("No textures loaded");
        ImGui::End();
        return;
    }

    if (browser->textures == NULL) {
        ImGui::Text("Textures array is NULL");
        ImGui::End();
        return;
    }

    float padding = 4.0f;
    int endIndex = browser->startIndex + browser->tilesPerPage;
    if (endIndex > browser->totalCount) endIndex = browser->totalCount;

    for (int i = browser->startIndex; i < endIndex; i++) {
        ImGui::PushID(i);

        Texture2D tex = browser->textures[i];
        bool isValidTexture = !(tex.id == 0 || tex.width == 0 || tex.height == 0);

        // Always use square size
        ImVec2 buttonSize = ImVec2(browser->thumbnailSize, browser->thumbnailSize);

        bool isSelected = (browser->selected == i);

        if (isValidTexture) {
            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.8f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 0.9f, 1.0f));
            }

            if (ImGui::ImageButton("##thumb", (void*)(intptr_t)tex.id, buttonSize)) {
                browser->selected = i;
            }

            if (isSelected) {
                ImGui::PopStyleColor(2);
            }
        } else {
            // Invalid texture - draw disabled button
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

            ImGui::Button("NULL", buttonSize);

            ImGui::PopStyleColor(3);
        }

        if (ImGui::IsItemHovered()) {
            if (isValidTexture) {
                ImGui::SetTooltip("Texture %d\n%dx%d", i, tex.width, tex.height);
            } else {
                ImGui::SetTooltip("Texture %d\nNULL/Invalid", i);
            }
        }

        int relativeIndex = i - browser->startIndex;
        if ((relativeIndex + 1) % browser->columns != 0 && i < endIndex - 1) {
            ImGui::SameLine(0.0f, padding);
        }

        ImGui::PopID();
    }

    ImGui::End();
}

TextureBrowser texb={0};
void InitTexBrowser() {
    texb.selected=2;
    texb.totalCount=1811;
    texb.columns=4;
    texb.startIndex = 0;
    texb.thumbnailSize=64;
    texb.tilesPerPage = 32; // Show 50 at a time
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
ImGuiViewport* viewport;


int file_exists(const char* path) {
    FILE* file = fopen(path, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

int has_extension(const char* path, const char* ext) {
    const char* dot = strrchr(path, '.');
    return dot && strcmp(dot, ext) == 0;
}

int check_tiles_art_exists(const char* dir_path) {
    char pattern[512];
    snprintf(pattern, sizeof(pattern), "%s/tiles*.art", dir_path);

    // Simple check for tiles000.art as minimum requirement
    char tiles_path[512];
    snprintf(tiles_path, sizeof(tiles_path), "%s/tiles000.art", dir_path);
    return file_exists(tiles_path);
}

void extract_directory(const char* filepath, char* dir_path, size_t dir_size) {
    strncpy(dir_path, filepath, dir_size - 1);
    dir_path[dir_size - 1] = '\0';

    char* last_slash = strrchr(dir_path, '/');
    char* last_backslash = strrchr(dir_path, '\\');
    char* last_sep = (last_slash > last_backslash) ? last_slash : last_backslash;

    if (last_sep) {
        *last_sep = '\0';
    } else {
        strcpy(dir_path, ".");
    }
}

bool loadifvalid() {
    if (__argc < 2) {
        printf("Error: No map file path provided\n");
        return false;
    }

    const char* map_path = __argv[1];

    // Check if path points to .map file
    if (!has_extension(map_path, ".map")) {
        printf("Error: File must have .map extension\n");
        return false;
    }

    if (!file_exists(map_path)) {
        printf("Error: Map file does not exist: %s\n", map_path);
        return false;
    }

    // Extract directory from map path
    char dir_path[512];
    extract_directory(map_path, dir_path, sizeof(dir_path));

    // Check for palette.dat
    char palette_path[512];
    snprintf(palette_path, sizeof(palette_path), "%s/palette.dat", dir_path);
    if (!file_exists(palette_path)) {
        printf("Error: palette.dat not found in %s\n", dir_path);
        return false;
    }

    // Check for lookup.dat not needed actually!
   //char lookup_path[512];
   //snprintf(lookup_path, sizeof(lookup_path), "%s/lookup.dat", dir_path);
   //if (!file_exists(lookup_path)) {
   //    printf("Error: lookup.dat not found in %s\n", dir_path);
   //    return false;
   //}

    // Check for at least one tiles*.art file
    if (!check_tiles_art_exists(dir_path)) {
        printf("Error: No tiles*.art files found in %s\n", dir_path);
        return false;
    }

    DumbRender::Init(map_path);
    return true;
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
    DumbRender::Init("c:/Eugene/Games/build2/e3l3,map");

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
    texb.textures = DumbRender::RuntimeTextures();
    //DrawTextureBrowser(&texb);
    ImGui::Begin("##info_panel", NULL, window_flags);
    ImGui::Text("Q = pick & move");
    ImGui::Text("` = discard");
    ImGui::Text("L = sprite to light");
    ImGui::Text("C = pick Color");

    if (ISGRABSPRI && GRABSPRI.flags&SPRITE_B2_IS_LIGHT) {
        ImGui::Text("IsLIGHT.");
    }
if (ISHOVERWAL) {
    ImGui::Text("Nsw:%i %i",HOVERWAL.ns, HOVERWAL.nw);
    ImGui::Text("NCsw:%i %i",HOVERWAL.nschain, HOVERWAL.nwchain);
    ImGui::Text("flags: %i %d", HOVERWAL.surf.asc, HOVERWAL.surf.alpha);// need show as uint32_t binary with zeros.
}

    ImGui::End();
}

bool showPicker = false;
typedef struct {
    unsigned char r, g, b;
    signed short luminance;
} ColorData;

ColorData currentColor = {255, 255, 255, 4000};

void DrawPicker() {
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;

    ImVec2 window_pos = ImVec2(work_pos.x + work_size.x * 0.5f - 150.0f, work_pos.y + work_size.y * 0.5f - 100.0f);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(400.0f, 400.0f), ImGuiCond_Always);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

    if (ImGui::Begin("Color Picker", &showPicker, window_flags)) {
        float rgb[3] = {currentColor.r / 255.0f, currentColor.g / 255.0f, currentColor.b / 255.0f};

        // Main color picker with only hue bar and saturation/value square
        ImGui::BeginGroup();
        if (ImGui::ColorPicker3("##picker", rgb,
            ImGuiColorEditFlags_PickerHueBar |
            ImGuiColorEditFlags_NoSidePreview |
            ImGuiColorEditFlags_NoInputs |
            ImGuiColorEditFlags_NoAlpha)) {
            currentColor.r = (unsigned char)(rgb[0] * 255.0f);
            currentColor.g = (unsigned char)(rgb[1] * 255.0f);
            currentColor.b = (unsigned char)(rgb[2] * 255.0f);
            }
        ImGui::EndGroup();

        // Vertical luminance bar on the right
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Text("Lum");
        int lum = currentColor.luminance;
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 10.0f);
        if (ImGui::VSliderInt("##luminance", ImVec2(20.0f, 150.0f), &lum, -32767, 32767)) {
            currentColor.luminance = (signed short)lum;
        }
        ImGui::PopStyleVar();
        ImGui::EndGroup();

        ImGui::Spacing();
        if (ImGui::Button("Close")) {
            showPicker = false;
        }
        ImGui::End();
    }
}

/* priority orderring list
 * 0. make ap exit
1. hdr lights
2. sprites from portals
3. render atest and sprite ordering
4. extrusion ops
5. floor ceil wall move
6. linux build
7. 4. light+portals
8. 5. duke game fix
9. 6. release polish
10. 7. floor ceil ports mono fix
11. 8. palforms

-- draw original wall on portal failures.
*/

// Draw palette and texture preview on screen
void MainLoop()
{
    InitTexBrowser();
    //    if (!loadifvalid())
 //       return;
    //DumbRender::Init("c:/Eugene/Games/build2/Content/GAL_002/E1L1.MAP ");
    DumbRender::Init("c:/Eugene/Games/build2/e2l2.map");
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
    showPicker = false;
    DisableCursor();
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

#if !IS_DUKE_INCLUDED
        Editor_DoRaycasts(&localb2cam);
        EditorFrameMin(*DumbCore::GetCamera());
#endif
        // Render albedo pass
        BeginCustomRenderTarget(albedoTarget);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        ClearBackground(BLACK);
        if (!showPicker) DumbCore::Update(deltaTime);

        BeginMode3D(*DumbCore::GetCamera());

        if (!showPicker) { DumbRender::ProcessKeys(); }
        DumbRender::DrawKenGeometry(GetScreenWidth(), GetScreenHeight(), DumbCore::GetCamera());
        DumbRender::DrawMapstateTex(*DumbCore::GetCamera());
        DrawGizmos();
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
        // draw frame;
        DrawTextureRec({finalTarget.colorTexture, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8},
                      {0, 0, (float)w, (float)-h}, {0, 0}, WHITE);
        if (IsKeyPressed(KEY_ESCAPE))
            DisableCursor();

#if !IS_DUKE_INCLUDED
        // IMGUI SECTION
        viewport = ImGui::GetMainViewport();
        rlImGuiBegin();
        DrawInfoUI();
        if (showPicker) {
            DrawPicker();
            SetColorum(currentColor.r, currentColor.g, currentColor.b, currentColor.luminance);
        }
        if (IsKeyPressed(KEY_L)) {
            SetColorum(currentColor.r, currentColor.g, currentColor.b, currentColor.luminance);
        }
        if (IsKeyPressed(KEY_C)) {
            showPicker = !showPicker;
            if (showPicker) {
                EnableCursor();
            } else {
                DisableCursor();
            }
        }

        rlImGuiEnd();
#endif

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
    SetExitKey(KEY_NULL);
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
        if (IsKeyPressed(KEY_ESCAPE) && IsKeyPressed(KEY_LEFT_SHIFT))
            break;

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
