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
// Depends
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
#include "Editor/uimodels.h"

// Implements
#include "Editor/ieditorhudview.h"


extern "C" {
#include "Core/loaders.h"
#include "Core/artloader.h"

}
// make parallax
// for floors and walls - if own flor is paralax - tag floor trap and wall trap as ns portal.
//

void DrawTextureBrowser(TextureBrowser* browser) {
    if (!browser->shown)
        return;
    int totalGals = 2;
    float dxmul = 0.4f;
    float dymul = 0.4f;
    static bool useRepeat = true;
    static int galSelected[2] = {0, 0};
    static int galStartRow[2] = {0, 0};
    static bool needsResize = false;

    browser->totalCount = g_gals[browser->galnum].gnumtiles;

    // Calculate tiles per page from rows and columns
    int tilesPerPage = browser->maxVisibleRows * browser->columns;

    // Calculate start index from start row
    int startIndex = browser->startRow * browser->columns;

    // Calculate window size based on current parameters
    float padding = 4.0f;
    float windowPadding = ImGui::GetStyle().WindowPadding.x;

    // Calculate content height for settings area
    float settingsHeight = 0.0f;
    if (browser->showSettings) {
        settingsHeight = 6 * ImGui::GetFrameHeight() + 5 * ImGui::GetStyle().ItemSpacing.y; // 6 controls
    }

    float headerHeight = ImGui::GetFrameHeight() + // Gallery slider
                        ImGui::GetFrameHeight() + // Settings button
                        ImGui::GetFrameHeight() + // Info text
                        ImGui::GetStyle().SeparatorTextPadding.y + // Separator
                        settingsHeight +
                        4 * ImGui::GetStyle().ItemSpacing.y; // Spacing between elements

    float neededWidth = browser->columns * browser->thumbnailSize +
                       (browser->columns - 1) * padding +
                       2 * windowPadding;

    float neededHeight = headerHeight +
                        browser->maxVisibleRows * (browser->thumbnailSize + padding) - padding +
                        2 * windowPadding;

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(neededWidth, neededHeight), ImGuiCond_Always);

    // Disable resizing but allow moving
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;

    if (!ImGui::Begin("Texture Browser", NULL, windowFlags)) {
        ImGui::End();
        return;
    }

    // Movement operations - simple deltas
    int galDelta = 0;
    int selDelta = 0;
    int viewRowDelta = 0;
    bool moveSelection = false;
    bool moveView = false;
    bool settingsChanged = false;

    // Gallery selection
    if (ImGui::SliderInt("Gallery", &browser->galnum, 0, totalGals - 1)) {
        galDelta = 0; // Force state switch
    }

    // Handle shift + A/D for gallery switching
    bool shiftHeld = ImGui::GetIO().KeyShift;
    if (shiftHeld) {
        if (ImGui::IsKeyPressed(ImGuiKey_A)) galDelta = -1;
        if (ImGui::IsKeyPressed(ImGuiKey_D)) galDelta = 1;
    }

    // Settings
    if (ImGui::Button(browser->showSettings ? "Hide Settings" : "Show Settings")) {
        browser->showSettings = !browser->showSettings;
        needsResize = true;
    }

    if (browser->showSettings) {
        if (ImGui::SliderInt("Columns", &browser->columns, 1, 6)) {
            settingsChanged = true;
            needsResize = true;
        }
        if (ImGui::SliderFloat("Size", &browser->thumbnailSize, 32.0f, 128.0f)) {
            settingsChanged = true;
            needsResize = true;
        }
        if (ImGui::SliderInt("Max Visible Rows", &browser->maxVisibleRows, 2, 20)) {
            settingsChanged = true;
            needsResize = true;
        }
        ImGui::Checkbox("Use Repeat", &useRepeat);

        int totalRows = (browser->totalCount + browser->columns - 1) / browser->columns;
        int maxStartRow = totalRows - browser->maxVisibleRows;
        if (maxStartRow < 0) maxStartRow = 0;
        if (ImGui::SliderInt("Start Row", &browser->startRow, 0, maxStartRow)) {
            galStartRow[browser->galnum] = browser->startRow;
        }
    }

    // Resize window if needed
    if (needsResize) {
        float newSettingsHeight = 0.0f;
        if (browser->showSettings) {
            newSettingsHeight = 6 * ImGui::GetFrameHeight() + 5 * ImGui::GetStyle().ItemSpacing.y;
        }

        float newHeaderHeight = ImGui::GetFrameHeight() +
                               ImGui::GetFrameHeight() +
                               ImGui::GetFrameHeight() +
                               ImGui::GetStyle().SeparatorTextPadding.y +
                               newSettingsHeight +
                               4 * ImGui::GetStyle().ItemSpacing.y;

        float newWidth = browser->columns * browser->thumbnailSize +
                        (browser->columns - 1) * padding +
                        2 * windowPadding;

        float newHeight = newHeaderHeight +
                         browser->maxVisibleRows * (browser->thumbnailSize + padding) - padding +
                         2 * windowPadding;

        ImGui::SetWindowSize(ImVec2(newWidth, newHeight));
        needsResize = false;
    }

    // Handle shift + mouse delta for 2D selection movement
    if (shiftHeld) {
        ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
        static float accumulatedX = 0.0f;
        static float accumulatedY = 0.0f;

        accumulatedX += mouseDelta.x * dxmul;
        accumulatedY += mouseDelta.y * dymul;

        float threshold = 10.0f;
        int moveX = 0, moveY = 0;

        if (accumulatedX > threshold) { moveX = 1; accumulatedX = 0.0f; }
        else if (accumulatedX < -threshold) { moveX = -1; accumulatedX = 0.0f; }

        if (accumulatedY > threshold) { moveY = 1; accumulatedY = 0.0f; }
        else if (accumulatedY < -threshold) { moveY = -1; accumulatedY = 0.0f; }

        if (moveX != 0 || moveY != 0) {
            int currentRow = browser->selected / browser->columns;
            int currentCol = browser->selected % browser->columns;

            currentCol += moveX;
            currentRow += moveY;
            currentCol = Clamp(currentCol,0,browser->columns-1);
            selDelta = (currentRow * browser->columns + currentCol) - browser->selected;
            moveSelection = true;
        }
    }

    // Handle scroll wheel
    float wheel = ImGui::GetIO().MouseWheel;
    if (wheel != 0.0f) {
        int scrollDirection = wheel > 0 ? -1 : 1;
        bool ctrlHeld = ImGui::GetIO().KeyCtrl;

        if (shiftHeld) {
            int scrollRows = (browser->maxVisibleRows + 2) / 3;
            if (scrollRows < 1) scrollRows = 1;

            viewRowDelta = scrollDirection * scrollRows;
            selDelta = scrollDirection * scrollRows * browser->columns;
            moveView = true;
            moveSelection = true;
        } else if (ctrlHeld) {
            viewRowDelta = scrollDirection * browser->maxVisibleRows;
            selDelta = browser->startRow * browser->columns + viewRowDelta * browser->columns - browser->selected;
            moveView = true;
            moveSelection = true;
        } else {
            int viewStartIndex = browser->startRow * browser->columns;
            int viewEndIndex = viewStartIndex + tilesPerPage - 1;
            bool atTopEdge = (browser->selected < viewStartIndex + browser->columns);
            bool atBottomEdge = (browser->selected > viewEndIndex - browser->columns);

            if ((scrollDirection == -1 && atTopEdge) || (scrollDirection == 1 && atBottomEdge)) {
                selDelta = scrollDirection * browser->columns;
            } else {
                selDelta = scrollDirection;
            }
            moveSelection = true;
        }
    }

    // Apply gallery change
    if (galDelta != 0) {
        galSelected[browser->galnum] = browser->selected;
        galStartRow[browser->galnum] = browser->startRow;

        browser->galnum += galDelta;
        if (browser->galnum < 0) browser->galnum = totalGals - 1;
        if (browser->galnum >= totalGals) browser->galnum = 0;

        browser->totalCount = g_gals[browser->galnum].gnumtiles;
        browser->selected = galSelected[browser->galnum];
        browser->startRow = galStartRow[browser->galnum];
    }

    // Apply movements
    if (moveSelection) {
        browser->selected += selDelta;
    }
    if (moveView) {
        browser->startRow += viewRowDelta;
    }

    // Single clamping pass
    if (browser->selected < 0) browser->selected = 0;
    if (browser->selected >= browser->totalCount) browser->selected = browser->totalCount - 1;

    int totalRows = (browser->totalCount + browser->columns - 1) / browser->columns;
    int maxStartRow = totalRows - browser->maxVisibleRows;
    if (maxStartRow < 0) maxStartRow = 0;
    if (browser->startRow < 0) browser->startRow = 0;
    if (browser->startRow > maxStartRow) browser->startRow = maxStartRow;

    // Adjust view to keep selection visible
    int selectedRow = browser->selected / browser->columns;
    int viewStartRow = browser->startRow;
    int viewEndRow = browser->startRow + browser->maxVisibleRows - 1;

    if (selectedRow < viewStartRow) {
        browser->startRow = selectedRow;
    } else if (selectedRow > viewEndRow) {
        browser->startRow = selectedRow - browser->maxVisibleRows + 1;
        if (browser->startRow > maxStartRow) browser->startRow = maxStartRow;
    }

    // Update stored states once
    galSelected[browser->galnum] = browser->selected;
    galStartRow[browser->galnum] = browser->startRow;

    // Recalculate start index for display
    startIndex = browser->startRow * browser->columns;

    // Display info
    int actualEnd = startIndex + tilesPerPage;
    if (actualEnd > browser->totalCount) actualEnd = browser->totalCount;

    int currentPage = startIndex / tilesPerPage;
    int totalPages = (browser->totalCount + tilesPerPage - 1) / tilesPerPage;

    ImGui::Text("Gallery %d | Page %d/%d | Showing %d-%d of %d | Selected: %d",
                browser->galnum, currentPage + 1, totalPages,
                startIndex + 1, actualEnd, browser->totalCount, browser->selected);
    ImGui::Separator();

    if (browser->totalCount == 0) {
        ImGui::Text("No textures loaded");
        ImGui::End();
        return;
    }

    if (DumbRender::galtextures[browser->galnum] == NULL) {
        ImGui::Text("Textures array is NULL");
        ImGui::End();
        return;
    }

    // Render tiles
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

    int endIndex = startIndex + tilesPerPage;
    if (endIndex > browser->totalCount) endIndex = browser->totalCount;

    for (int i = startIndex; i < endIndex; i++) {
        ImGui::PushID(i);

        Texture2D tex = DumbRender::galtextures[browser->galnum][i];
        bool isValidTexture = !(tex.id == 0 || tex.width == 0 || tex.height == 0);
        bool isSelected = (browser->selected == i);

        ImVec2 buttonSize = ImVec2(browser->thumbnailSize, browser->thumbnailSize);

        if (isSelected) {
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddRect(
                ImVec2(pos.x - 2, pos.y - 2),
                ImVec2(pos.x + buttonSize.x + 2, pos.y + buttonSize.y + 2),
                IM_COL32(255, 255, 255, 255), 0.0f, 0, 2.0f
            );
        }

        bool clicked = false;

        if (isValidTexture) {
            clicked = ImGui::InvisibleButton("##thumb", buttonSize);

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 btnMin = ImGui::GetItemRectMin();
            ImVec2 btnMax = ImGui::GetItemRectMax();

            bool hovered = ImGui::IsItemHovered();
            bool active = ImGui::IsItemActive();
            ImU32 bgCol = ImGui::GetColorU32(
                active  ? ImGuiCol_ButtonActive :
                hovered ? ImGuiCol_ButtonHovered :
                          ImGuiCol_Button
            );
            drawList->AddRectFilled(btnMin, btnMax, bgCol);

            float texW = (float)tex.width;
            float texH = (float)tex.height;
            ImVec2 image_topleft, image_botright;

            if (texW > texH) {
                float eside = 0.5f * (1.0f - texH / texW);
                image_topleft  = ImVec2(0.0f, eside);
                image_botright = ImVec2(1.0f, 1.0f - eside);
            } else {
                float eside = 0.5f * (1.0f - texW / texH);
                image_topleft  = ImVec2(eside, 0.0f);
                image_botright = ImVec2(1.0f - eside, 1.0f);
            }

            ImVec2 imgMin = ImVec2(
                btnMin.x + image_topleft.x * buttonSize.x,
                btnMin.y + image_topleft.y * buttonSize.y
            );
            ImVec2 imgMax = ImVec2(
                btnMin.x + image_botright.x * buttonSize.x,
                btnMin.y + image_botright.y * buttonSize.y
            );

            drawList->AddImage(
                (intptr_t)tex.id,
                imgMin, imgMax,
                ImVec2(0, 0), ImVec2(1, 1)
            );
        }else {
            clicked = ImGui::InvisibleButton("##thumb", buttonSize);

            ImVec2 pos = ImGui::GetItemRectMin();
            ImVec2 endPos = ImGui::GetItemRectMax();
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(pos, endPos, IM_COL32(50, 50, 50, 255));

            ImVec2 textSize = ImGui::CalcTextSize("NULL");
            ImVec2 textPos = ImVec2(
                pos.x + (buttonSize.x - textSize.x) * 0.5f,
                pos.y + (buttonSize.y - textSize.y) * 0.5f
            );
            drawList->AddText(textPos, IM_COL32(150, 150, 150, 255), "NULL");
        }

        int relativeIndex = i - startIndex;
        if ((relativeIndex + 1) % browser->columns != 0 && i < endIndex - 1) {
            ImGui::SameLine(0.0f, padding);
        }

        ImGui::PopID();
    }

    ImGui::PopStyleVar();
    ImGui::End();
}

TextureBrowser texb={0};
void InitTexBrowser() {
    texb.selected=2;
    texb.totalCount=1811;
    texb.columns=7;
    texb.startRow = 0;
    texb.thumbnailSize=84;
    texb.tilesPerPage = 63; // Show 50 at a time
    texb.maxVisibleRows = 9; // Show 50 at a time
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

    glGenFramebuffers(1, &target.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);

    // Create HDR color texture (16-bit float)
    glGenTextures(1, &target.colorTexture);
    glBindTexture(GL_TEXTURE_2D, target.colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.colorTexture, 0);

    // Depth buffer remains the same
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

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        TraceLog(LOG_ERROR, "HDR Framebuffer not complete!");
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
    DrawTextureBrowser(&texb);
    ImGui::Begin("##info_panel", NULL, window_flags);
    ImGui::Text("Q = pick & move");
    ImGui::Text("` = discard");
    ImGui::Text("L = sprite to light");
    ImGui::Text("C = pick Color");
    ImGui::Text("1234 selmodes");
    ImGui::Text("V tile pick");
    ImGui::Text("G geo ops.");
    ImGui::Text("K - temp extrude");


    if (ISGRABSPRI && GRABSPRI.flags&SPRITE_B2_IS_LIGHT) {
        ImGui::Text("IsLIGHT.");
    }
if (ISHOVERWAL) {
    ImGui::Text("S:%i w:%i w.n:%i \nNs:%i Nw:%i",hoverfoc.sec, hoverfoc.wal, HOVERWAL.n, HOVERWAL.ns, HOVERWAL.nw);
    ImGui::Text("NCsw:%i %i",HOVERWAL.nschain, HOVERWAL.nwchain);
    ImGui::Text("flags: %i %d", HOVERWAL.surf.asc, HOVERWAL.surf.alpha);// need show as uint32_t binary with zeros.
}
    if (ISHOVERSPRI) {
        int til = map->spri[hoverfoc.spri].tilnum;
        int gal = map->spri[hoverfoc.spri].galnum;
        ImGui::Text("T: %i, \n xof,yof: %i,%i", til, g_gals[gal].picanm_data[til].x_center_offset,g_gals[gal].picanm_data[til].y_center_offset);
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

// --------------------- infos
static char info_message[256] = {0};
static float info_timer = 0.0f;
static const float INFO_DISPLAY_TIME = 3.0f;

void EditorHudDrawTopInfo(const char* message) {
    strncpy(info_message, message, sizeof(info_message) - 1);
    info_message[sizeof(info_message) - 1] = '\0';
    info_timer = INFO_DISPLAY_TIME;
}

void DrawInfoMessage() {
    if (info_timer <= 0.0f) return;

    info_timer -= ImGui::GetIO().DeltaTime;

    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;

    // Center horizontally at top
    ImVec2 window_pos = ImVec2(work_pos.x + work_size.x * 0.5f, work_pos.y + 20.0f);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, ImVec2(0.5f, 0.0f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                            ImGuiWindowFlags_NoResize |
                            ImGuiWindowFlags_NoMove |
                            ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_AlwaysAutoResize |
                            ImGuiWindowFlags_NoBackground |
                            ImGuiWindowFlags_NoInputs;

    ImGui::Begin("##top_info", NULL, flags);
    ImGui::Text("%s", info_message);
    ImGui::End();
}

// ------------------------------------

/* priority orderring list
 * 0. make ap exit
1. hdr lights
2. sprites from portals
3. render atest and sprite ordering
6. linux build
7. 4. light+portals
8. 5. duke game fix
9. 6. release polish
10. 7. floor ceil ports mono fix
11. 8. palforms

-- draw original wall on portal failure handling.

sprites:
1. draw geo to albedo
2. also collect sprites to draw with portalxforms.
2. draw geo lights as alpha test, using geo zbuf.
3. draw sprites to albedo using a-test for all
4. draw sprite lights using zbuf from albedo.

draw sprites as polys inside light pass.
sort them together as walls, or just by poss.
1. collect sprites in camera pass
2. draw only those sprites in light passes.
3. sort them together with walls.
3. dont emit light polys if sect was not in camera pass


*/
void RecreateRenderTargets(CustomRenderTarget* albedo, CustomRenderTarget* light, CustomRenderTarget* combined, CustomRenderTarget* final) {
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    // Cleanup old targets
    glDeleteTextures(1, &albedo->depthTexture);
    UnloadCustomRenderTarget(*albedo);
    UnloadCustomRenderTarget(*light);
    UnloadCustomRenderTarget(*combined);
    UnloadCustomRenderTarget(*final);

    // Create new targets with current screen size
    *albedo = CreateCustomRenderTarget(w, h, 0);
    *light = CreateCustomRenderTarget(w, h, albedo->depthTexture);
    *combined = CreateCustomRenderTarget(w, h, 0);
    *final = CreateCustomRenderTarget(w, h, 0);
}
// Draw palette and texture preview on screen
void MainLoop() {
    Shader multiplyShader = LoadShader(NULL, "Shaders/lightmul.frag");
    // Get uniform locations
    int albedoLocation = GetShaderLocation(multiplyShader, "albedoTexture");
    int lightLocation = GetShaderLocation(multiplyShader, "lightTexture");
    InitTexBrowser();
    EditorSetTileState(&texb);
    //    if (!loadifvalid())
    //       return;
    //DumbRender::Init("c:/Eugene/Games/build2/Content/GAL_002/E1L1.MAP ");
    loadgal(0, "c:/Eugene/Games/build2/");
    loadgal(1, "c:/Eugene/Games/build2/Content/GAL_002_SW/");
    DumbRender::LoadTexturesToGPU();
    DumbRender::Init("c:/Eugene/Games/build2/e1l1.map");
    auto map = DumbRender::GetMap();
    DumbCore::Init(map);
    SetTargetFPS(60);

    InitEditor(map);
    // Initialize LUT system
    InitLUTSystem();

    // Create render targets with shared depth
    CustomRenderTarget albedoTarget = CreateCustomRenderTarget(GetScreenWidth(), GetScreenHeight(), 0);
    CustomRenderTarget lightTarget = CreateCustomRenderTarget(GetScreenWidth(), GetScreenHeight(),
                                                              albedoTarget.depthTexture);
    CustomRenderTarget combinedTarget = CreateCustomRenderTarget(GetScreenWidth(), GetScreenHeight(), 0);
    int w = GetScreenWidth();
    int h = GetScreenHeight();
    showPicker = false;
    DisableCursor();
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        // Check for window resize
        int currentW = GetScreenWidth();
        int currentH = GetScreenHeight();

        if (currentW != w || currentH != h) {
            w = currentW;
            h = currentH;
            RecreateRenderTargets(&albedoTarget, &lightTarget, &combinedTarget, &finalTarget);
        }
#if !IS_DUKE_INCLUDED
        Editor_DoRaycasts(&localb2cam);
        EditorUpdate(*DumbCore::GetCamera());
#endif
        // Render albedo pass
        BeginCustomRenderTarget(albedoTarget);
        {
            // Albedo Geometry
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
        }
        EndCustomRenderTarget(); //END ALBEDO

        BeginCustomRenderTarget(lightTarget);
        {
            // Render light pass (HDR accumulation)
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Clear to black for light accumulation
            glClear(GL_COLOR_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);

            // Enable additive blending for light accumulation
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);

            DumbRender::DrawLightsPost3d(w, h, *DumbCore::GetCamera());

            glDisable(GL_BLEND);
            glDepthMask(GL_TRUE);
        }
        EndCustomRenderTarget(); // END LIGHT


        BeginCustomRenderTarget(combinedTarget);
        {
            // combine Lights wit albedo
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            BeginShaderMode(multiplyShader);
            SetShaderValueTexture(multiplyShader, GetShaderLocation(multiplyShader, "albedoTexture"),
                                  {albedoTarget.colorTexture, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R16G16B16A16});
            SetShaderValueTexture(multiplyShader, GetShaderLocation(multiplyShader, "lightTexture"),
                                  {lightTarget.colorTexture, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R16G16B16A16});

            // Use DrawTextureRec instead of DrawRectangle for proper UV mapping
            DrawTextureRec({albedoTarget.colorTexture, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R16G16B16A16},
                           {0, 0, (float) w, (float) -h}, {0, 0}, WHITE);
            EndShaderMode();
        }
        EndCustomRenderTarget(); // END COMBINED


        BeginDrawing();
        {
            // 2d ops and pp
            ClearBackground(BLACK);

            // Draw final texture with proper Y-flip for OpenGL framebuffer
            DrawTextureRec({finalTarget.colorTexture, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8},
                           {0, 0, (float) w, (float) -h}, {0, 0}, WHITE); // Keep the -h for Y-flip

            // Also fix the LUT pass:
            BeginCustomRenderTarget(finalTarget);
            {
                // LUT, POSTPROCESSINg
                glClear(GL_COLOR_BUFFER_BIT);
                glDisable(GL_DEPTH_TEST);

                BeginShaderMode(lutShader);
                SetShaderValueTexture(lutShader, lutTextureLocation, lutTexture);
                DrawTextureRec({combinedTarget.colorTexture, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R16G16B16A16},
                               {0, 0, (float) w, (float) -h}, {0, 0}, WHITE); // Keep Y-flip here too
                EndShaderMode();
            }
            EndCustomRenderTarget();
            if (IsKeyPressed(KEY_ESCAPE))
                DisableCursor();

#if !IS_DUKE_INCLUDED

            // Restore complete GL state for ImGui
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, w, h);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Standard alpha blending
            glBlendEquation(GL_FUNC_ADD);
            DrawFPS(10, 10);
            // IMGUI SECTION
            viewport = ImGui::GetMainViewport();
            rlImGuiBegin();
            DrawInfoUI();
            DrawInfoMessage();
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
        }

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


      //  DrawPaletteAndTexture(paltex,tex,600,600);
        EndDrawing();
    }

    rlImGuiShutdown();
 //!  lua_close(L);
    CloseWindow();
    return 0;
}

/* mul and additrive buffers.
Create two light targets
CustomRenderTarget lightMultiplyTarget = CreateCustomRenderTarget(w, h, albedoTarget.depthTexture);
CustomRenderTarget lightAdditiveTarget = CreateCustomRenderTarget(w, h, albedoTarget.depthTexture);

// Render multiply lights
BeginCustomRenderTarget(lightMultiplyTarget);
glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Clear to white for multiply
glClear(GL_COLOR_BUFFER_BIT);
glEnable(GL_BLEND);
glBlendFunc(GL_DST_COLOR, GL_ZERO); // Multiply blend
DumbRender::DrawLightsMultiply(w, h, *DumbCore::GetCamera());
glDisable(GL_BLEND);
EndCustomRenderTarget();

// Render additive lights
BeginCustomRenderTarget(lightAdditiveTarget);
glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Clear to black for additive
glClear(GL_COLOR_BUFFER_BIT);
glEnable(GL_BLEND);
glBlendFunc(GL_ONE, GL_ONE); // Additive blend
DumbRender::DrawLightsAdditive(w, h, *DumbCore::GetCamera());
glDisable(GL_BLEND);
EndCustomRenderTarget();
*/