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
#include <algorithm>
#include <vector>
#include <string>
#include <filesystem>
#include <system_error>

#include "DumbRender.hpp"
//#include "MonoTest.hpp"
//#include "luabinder.hpp"
// Depends
//#include "DumbCore.hpp"
#include "DumbEdit.hpp"
#include "MonoTest.hpp"
#include "raymath.h"
#if IS_DUKE_INCLUDED
#include "DukeGame/source/dukewrap.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include "Editor/uimodels.h"

// Implements
#include "Editor/ieditorhudview.h"


extern "C" {
#include "Core/loaders.h"
#include "Core/artloader.h"

}

static int g_argc = 0;
static char **g_argv = nullptr;
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
cam_t globCam={0};

typedef struct {
    char map_path[512];
    char install_dir[512];
    char gallery0_dir[512];
    char gallery1_dir[512];
    char status[1024];
} StartupConfig;

int file_exists(const char* path) {
    FILE* file = fopen(path, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

int str_equals_ignore_case(const char* a, const char* b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

int str_starts_with_ignore_case(const char* str, const char* prefix) {
    while (*prefix) {
        if (!*str) return 0;
        if (tolower((unsigned char)*str) != tolower((unsigned char)*prefix)) return 0;
        str++;
        prefix++;
    }
    return 1;
}

int str_ends_with_ignore_case(const char* str, const char* suffix) {
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len) return 0;
    return str_equals_ignore_case(str + str_len - suffix_len, suffix);
}

void ensure_directory_separator(char* path, size_t path_size) {
    size_t len;

    if (!path[0]) return;
    len = strlen(path);
    if (len == 0) return;
    if (path[len - 1] == '/' || path[len - 1] == '\\') return;
    if (len + 1 >= path_size) return;

    path[len] = '/';
    path[len + 1] = '\0';
}

int directory_contains_named_file_ignore_case(const char* dir_path, const char* expected_name) {
    DIR* dir = opendir(dir_path);
    if (!dir) return 0;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (str_equals_ignore_case(entry->d_name, expected_name)) {
            closedir(dir);
            return 1;
        }
    }

    closedir(dir);
    return 0;
}

int has_extension(const char* path, const char* ext) {
    const char* dot = strrchr(path, '.');
    return dot && str_equals_ignore_case(dot, ext);
}

int check_tiles_art_exists(const char* dir_path) {
    DIR* dir = opendir(dir_path);
    if (!dir) return 0;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (str_starts_with_ignore_case(entry->d_name, "tiles") &&
            str_ends_with_ignore_case(entry->d_name, ".art")) {
            closedir(dir);
            return 1;
        }
    }

    closedir(dir);
    return 0;
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

void set_status(StartupConfig* cfg, const char* message) {
    snprintf(cfg->status, sizeof(cfg->status), "%s", message);
}

std::string to_lower_copy(const std::string& value) {
    std::string out = value;
    std::transform(out.begin(), out.end(), out.begin(),
        [](unsigned char c) { return (char)std::tolower(c); });
    return out;
}

bool is_map_filename(const std::string& name) {
    return name.size() >= 4 && to_lower_copy(name.substr(name.size() - 4)) == ".map";
}

bool resolve_path_case_insensitive(const std::filesystem::path& input_path, std::filesystem::path* resolved_path) {
    if (resolved_path == nullptr || input_path.empty()) return false;

    std::error_code ec;
    if (std::filesystem::exists(input_path, ec)) {
        *resolved_path = input_path;
        return true;
    }

    std::filesystem::path current;
    std::filesystem::path normalized = input_path.lexically_normal();
    if (normalized.is_absolute()) {
        current = normalized.root_path();
    } else {
        current = std::filesystem::current_path(ec);
        if (ec) return false;
    }

    bool consumed_root_name = false;
    bool consumed_root_dir = false;

    for (const auto& part : normalized) {
        std::string part_str = part.string();
        if (part_str.empty()) continue;
        if (!consumed_root_name && normalized.has_root_name() && part == normalized.root_name()) {
            current = normalized.root_name();
            consumed_root_name = true;
            continue;
        }
        if (!consumed_root_dir && normalized.has_root_directory() && part == normalized.root_directory()) {
            if (current.empty()) current = part;
            consumed_root_dir = true;
            continue;
        }
        if (part_str == ".") continue;
        if (part_str == "..") {
            current = current.parent_path();
            continue;
        }

        std::filesystem::path exact = current / part;
        if (std::filesystem::exists(exact, ec)) {
            current = exact;
            continue;
        }

        bool found = false;
        std::string wanted = to_lower_copy(part_str);
        for (const auto& entry : std::filesystem::directory_iterator(current, ec)) {
            if (ec) break;
            if (to_lower_copy(entry.path().filename().string()) == wanted) {
                current = entry.path();
                found = true;
                break;
            }
        }

        if (!found) return false;
    }

    *resolved_path = current;
    return true;
}

bool is_valid_primary_asset_dir(const char* dir_path) {
    if (!dir_path[0]) return false;
    return directory_contains_named_file_ignore_case(dir_path, "palette.dat") &&
           check_tiles_art_exists(dir_path);
}

void try_set_path(char* dst, size_t dst_size, const std::filesystem::path& path) {
    std::string normalized = path.lexically_normal().string();
    snprintf(dst, dst_size, "%s", normalized.c_str());
}

bool find_first_map_in_tree(const std::filesystem::path& root, std::filesystem::path* found_map) {
    if (found_map == nullptr) return false;

    std::error_code ec;
    if (!std::filesystem::exists(root, ec) || !std::filesystem::is_directory(root, ec)) return false;

    for (auto it = std::filesystem::recursive_directory_iterator(
             root, std::filesystem::directory_options::skip_permission_denied, ec);
         it != std::filesystem::recursive_directory_iterator(); ++it) {
        if (ec) break;
        if (it.depth() > 3) {
            it.disable_recursion_pending();
            continue;
        }
        if (!it->is_regular_file(ec)) continue;
        if (is_map_filename(it->path().filename().string())) {
            *found_map = it->path();
            return true;
        }
    }
    return false;
}

bool find_primary_assets_in_tree(const std::filesystem::path& root, std::filesystem::path* found_dir) {
    if (found_dir == nullptr) return false;

    std::error_code ec;
    if (!std::filesystem::exists(root, ec) || !std::filesystem::is_directory(root, ec)) return false;
    if (is_valid_primary_asset_dir(root.string().c_str())) {
        *found_dir = root;
        return true;
    }

    for (auto it = std::filesystem::recursive_directory_iterator(
             root, std::filesystem::directory_options::skip_permission_denied, ec);
         it != std::filesystem::recursive_directory_iterator(); ++it) {
        if (ec) break;
        if (it.depth() > 3) {
            it.disable_recursion_pending();
            continue;
        }
        if (!it->is_directory(ec)) continue;
        if (is_valid_primary_asset_dir(it->path().string().c_str())) {
            *found_dir = it->path();
            return true;
        }
    }
    return false;
}

void try_autodetect_map(StartupConfig* cfg) {
    std::filesystem::path resolved;
    if (cfg->map_path[0] && resolve_path_case_insensitive(cfg->map_path, &resolved) &&
        std::filesystem::is_regular_file(resolved)) {
        try_set_path(cfg->map_path, sizeof(cfg->map_path), resolved);
        return;
    }

    std::vector<std::filesystem::path> roots;
    std::error_code ec;
    roots.emplace_back(std::filesystem::current_path(ec));
    if (g_argv != nullptr && g_argv[0] != nullptr && g_argv[0][0]) {
        roots.emplace_back(std::filesystem::absolute(g_argv[0]).parent_path());
    }

    const char* dir_candidates[] = { cfg->install_dir, cfg->gallery0_dir, cfg->gallery1_dir };
    for (const char* candidate : dir_candidates) {
        if (!candidate[0]) continue;
        if (resolve_path_case_insensitive(candidate, &resolved)) {
            if (std::filesystem::is_regular_file(resolved)) resolved = resolved.parent_path();
            roots.emplace_back(resolved);
        }
    }

    for (const std::filesystem::path& root : roots) {
        std::filesystem::path found_map;
        if (find_first_map_in_tree(root, &found_map)) {
            try_set_path(cfg->map_path, sizeof(cfg->map_path), found_map);
            return;
        }
    }
}

void try_autodetect_primary_assets(StartupConfig* cfg) {
    std::filesystem::path resolved;
    if (cfg->gallery0_dir[0] && resolve_path_case_insensitive(cfg->gallery0_dir, &resolved) &&
        is_valid_primary_asset_dir(resolved.string().c_str())) {
        try_set_path(cfg->gallery0_dir, sizeof(cfg->gallery0_dir), resolved);
        return;
    }

    std::vector<std::filesystem::path> roots;
    std::error_code ec;
    roots.emplace_back(std::filesystem::current_path(ec));

    if (g_argv != nullptr && g_argv[0] != nullptr && g_argv[0][0]) {
        std::filesystem::path exe_path = std::filesystem::absolute(g_argv[0]);
        roots.emplace_back(exe_path.parent_path());
    }

    const char* map_candidates[] = { cfg->map_path, cfg->install_dir };
    for (const char* candidate : map_candidates) {
        if (!candidate[0]) continue;
        std::filesystem::path base(candidate);
        if (resolve_path_case_insensitive(base, &resolved)) {
            if (std::filesystem::is_regular_file(resolved)) {
                roots.emplace_back(resolved.parent_path());
            } else {
                roots.emplace_back(resolved);
            }
        }
    }

    for (const std::filesystem::path& root : roots) {
        std::filesystem::path found_dir;
        if (find_primary_assets_in_tree(root, &found_dir)) {
            try_set_path(cfg->gallery0_dir, sizeof(cfg->gallery0_dir), found_dir);
            return;
        }
    }
}

void autofill_from_map(StartupConfig* cfg) {
    if (!cfg->map_path[0]) return;
    char dir_path[512];
    extract_directory(cfg->map_path, dir_path, sizeof(dir_path));
    snprintf(cfg->gallery0_dir, sizeof(cfg->gallery0_dir), "%s", dir_path);
    snprintf(cfg->gallery1_dir, sizeof(cfg->gallery1_dir), "%s/Content/GAL_002_SW/", dir_path);
    if (!cfg->install_dir[0]) {
        snprintf(cfg->install_dir, sizeof(cfg->install_dir), "%s", dir_path);
    }
}

void autofill_from_install(StartupConfig* cfg) {
    if (!cfg->install_dir[0]) return;
    if (!cfg->gallery0_dir[0]) {
        snprintf(cfg->gallery0_dir, sizeof(cfg->gallery0_dir), "%s", cfg->install_dir);
    }
    if (!cfg->gallery1_dir[0]) {
        snprintf(cfg->gallery1_dir, sizeof(cfg->gallery1_dir), "%s/Content/GAL_002_SW/", cfg->install_dir);
    }
}

bool validate_startup_config(StartupConfig* cfg, bool verbose) {
    try_autodetect_map(cfg);
    try_autodetect_primary_assets(cfg);

    ensure_directory_separator(cfg->install_dir, sizeof(cfg->install_dir));
    ensure_directory_separator(cfg->gallery0_dir, sizeof(cfg->gallery0_dir));
    ensure_directory_separator(cfg->gallery1_dir, sizeof(cfg->gallery1_dir));

    if (!cfg->map_path[0]) {
        if (verbose) set_status(cfg, "Map path is empty.");
        return false;
    }

    if (!has_extension(cfg->map_path, ".map")) {
        if (verbose) set_status(cfg, "Map file must end with .map");
        return false;
    }

    if (!file_exists(cfg->map_path)) {
        if (verbose) set_status(cfg, "Map file does not exist.");
        return false;
    }

    if (!cfg->gallery0_dir[0]) {
        if (verbose) set_status(cfg, "Primary asset directory is empty.");
        return false;
    }

    if (!directory_contains_named_file_ignore_case(cfg->gallery0_dir, "palette.dat")) {
        if (verbose) set_status(cfg, "palette.dat was not found in the primary asset directory.");
        return false;
    }

    if (!check_tiles_art_exists(cfg->gallery0_dir)) {
        if (verbose) set_status(cfg, "No TILES*.ART was found in the primary asset directory.");
        return false;
    }

    if (verbose) {
        if (cfg->gallery1_dir[0] && check_tiles_art_exists(cfg->gallery1_dir))
            set_status(cfg, "Paths look valid. Secondary gallery found.");
        else
            set_status(cfg, "Paths look valid. Secondary gallery missing, continuing with primary gallery only.");
    }
    return true;
}

bool apply_startup_config(StartupConfig* cfg) {
    if (!validate_startup_config(cfg, true)) return false;

    loadgal(0, cfg->gallery0_dir);
    if (cfg->gallery1_dir[0] && check_tiles_art_exists(cfg->gallery1_dir)) {
        loadgal(1, cfg->gallery1_dir);
    }
    DumbRender::Init(cfg->map_path);
    return true;
}

void init_startup_config_from_cli(StartupConfig* cfg) {
    memset(cfg, 0, sizeof(*cfg));
    set_status(cfg, "Provide a map and asset folders, then click Start RayGame.");
    if (g_argc >= 2) {
        snprintf(cfg->map_path, sizeof(cfg->map_path), "%s", g_argv[1]);
        autofill_from_map(cfg);
        autofill_from_install(cfg);
        validate_startup_config(cfg, true);
    }
}

bool prompt_for_startup_config(StartupConfig* cfg) {
    if (g_argc >= 2 && validate_startup_config(cfg, true)) {
        return apply_startup_config(cfg);
    }

    while (!WindowShouldClose()) {
        bool should_start = false;

        BeginDrawing();
        ClearBackground(Color{18, 22, 30, 255});

        rlImGuiBegin();
        ImGui::SetNextWindowPos(ImVec2(40, 40), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(860, 520), ImGuiCond_Always);

        if (ImGui::Begin("RayGame Startup", nullptr,
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize)) {

            ImGui::TextWrapped("RayGame needs a map plus Build/Duke art assets. Set the paths here to start the application.");
            ImGui::Separator();

            ImGui::InputText("Map (.map)", cfg->map_path, sizeof(cfg->map_path));
            ImGui::InputText("Install Dir", cfg->install_dir, sizeof(cfg->install_dir));
            ImGui::InputText("Primary Assets", cfg->gallery0_dir, sizeof(cfg->gallery0_dir));
            ImGui::InputText("Secondary Gallery", cfg->gallery1_dir, sizeof(cfg->gallery1_dir));

            if (ImGui::Button("Use Map Dir")) {
                autofill_from_map(cfg);
                validate_startup_config(cfg, true);
            }
            ImGui::SameLine();
            if (ImGui::Button("Use Install Dir")) {
                autofill_from_install(cfg);
                validate_startup_config(cfg, true);
            }
            ImGui::SameLine();
            if (ImGui::Button("Validate")) {
                validate_startup_config(cfg, true);
            }
            ImGui::SameLine();
            if (ImGui::Button("Start RayGame")) {
                if (apply_startup_config(cfg)) {
                    should_start = true;
                }
            }

            ImGui::Spacing();
            ImGui::TextWrapped("Expected in Primary Assets: palette.dat and TILES*.ART");
            ImGui::TextWrapped("Optional secondary gallery: Content/GAL_002_SW/");
            ImGui::Spacing();
            ImGui::TextWrapped("Status: %s", cfg->status);
        }

        ImGui::End();
        rlImGuiEnd();
        EndDrawing();

        if (should_start) {
            return true;
        }
    }

    return false;
}


void SetImguiFonts()
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    const char* font_candidates[] = {
#if defined(__linux__)
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/liberation/LiberationSans-Regular.ttf",
#endif
        "C:\\Windows\\Fonts\\segoeui.ttf"
    };

    ImFont* font = nullptr;
    for (size_t i = 0; i < sizeof(font_candidates) / sizeof(font_candidates[0]); ++i) {
        if (file_exists(font_candidates[i])) {
            font = io.Fonts->AddFontFromFileTTF(font_candidates[i], 28.0f);
            if (font != nullptr) break;
        }
    }

    if (font == nullptr) {
        ImFontConfig config;
        config.SizePixels = 18.0f;
        config.PixelSnapH = true;
        io.Fonts->AddFontDefault(&config);
        printf("Info: Using ImGui default font fallback\n");
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

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_AlwaysAutoResize;

    // First pass: begin with a temporary position, let it size itself
    // Then reposition so it stays top-right without clipping
    ImGui::SetNextWindowPos(ImVec2(work_pos.x + work_size.x - 10.0f, work_pos.y + 10.0f),
                            ImGuiCond_Always, ImVec2(1.0f, 0.0f)); // pivot top-right

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
    ImGui::Text("S:%i w:%i w.n:%i \nNs:%i Nw:%i",mdl.hover.sec, mdl.hover.wal, HOVERWAL.n, HOVERWAL.ns, HOVERWAL.nw);
    ImGui::Text("NCsw:%i %i",HOVERWAL.nschain, HOVERWAL.nwchain);
    ImGui::Text("flags: %i %d", HOVERWAL.surf.asc, HOVERWAL.surf.alpha);// need show as uint32_t binary with zeros.
}
    if (ISHOVERSPRI) {
        int til = map->spri[mdl.hover.spri].tilnum;
        int gal = map->spri[mdl.hover.spri].galnum;
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

    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 work_pos  = vp->WorkPos;
    ImVec2 work_size = vp->WorkSize;

    // Draw directly on foreground drawlist - no window, no clipping
    ImDrawList* dl = ImGui::GetForegroundDrawList();

    ImVec2 text_size = ImGui::CalcTextSize(info_message);

    float x = work_pos.x + (work_size.x - text_size.x) * 0.5f;
    float y = work_pos.y + 20.0f;

    // Shadow for bold effect
    dl->AddText(ImVec2(x + 1.0f, y + 1.0f), IM_COL32(0, 0, 0, 200), info_message);
    dl->AddText(ImVec2(x - 1.0f, y + 1.0f), IM_COL32(0, 0, 0, 200), info_message);
    dl->AddText(ImVec2(x + 1.0f, y - 1.0f), IM_COL32(0, 0, 0, 200), info_message);
    dl->AddText(ImVec2(x - 1.0f, y - 1.0f), IM_COL32(0, 0, 0, 200), info_message);
    // Main text
    dl->AddText(ImVec2(x, y), IM_COL32(255, 255, 255, 255), info_message);
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
Camera3D rlcam;
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
    StartupConfig startup = {0};
    init_startup_config_from_cli(&startup);
    if (!prompt_for_startup_config(&startup)) return;
    DumbRender::LoadTexturesToGPU();
    auto map = DumbRender::GetMap();
    //DumbCore::Init(map);
    globCam.tr.p = map->startpos;
    globCam.tr.r = map->startrig;
    globCam.tr.d = map->startdow;
    globCam.tr.f = map->startfor;
    globCam.cursect = map->startsectn;
    updatesect_imp(globCam.tr.p.x, globCam.tr.p.y, globCam.tr.p.z, &globCam.cursect, map);
    SetTargetFPS(120);

    InitEditor(map, &globCam);
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
    int debugViewMode = 0; // 0=final, 1=combined, 2=albedo, 3=light
    DisableCursor();
    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_ENTER))
        {
            if (IsWindowFullscreen())
            {
                ToggleFullscreen();
                SetWindowSize(w, h);
            }
            else
            {
                int mon = GetCurrentMonitor();
                SetWindowSize(GetMonitorWidth(mon), GetMonitorHeight(mon));
                ToggleFullscreen();
            }
        }
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
        Editor_DoRaycasts(&globCam);
        EditorUpdate();
#endif
        // Render albedo pass
        BeginCustomRenderTarget(albedoTarget);
        {
            // Albedo Geometry
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            ClearBackground(BLACK);
            //if (!showPicker) DumbCore::Update(deltaTime);

            DumbRender::MoveCamB2(&globCam);
            cam3d_from_tr(&rlcam, globCam.tr);
            rlcam.fovy = 90.0f;
            rlcam.projection = CAMERA_PERSPECTIVE;
            BeginMode3D(rlcam);
            if (!showPicker) { DumbRender::ProcessKeys(); }
            DumbRender::DrawKenGeometry(GetScreenWidth(), GetScreenHeight(), &globCam);

            DumbRender::DrawMapstateTex(rlcam);
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

            DumbRender::DrawLightsPost3d(w, h,rlcam);

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
        EndCustomRenderTarget(); // END POSTPROC COMBINE

        BeginCustomRenderTarget(finalTarget);
        {
            // LUT/post-process pass writes the frame that will be presented.
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            BeginShaderMode(lutShader);
            SetShaderValueTexture(lutShader, lutTextureLocation, lutTexture);
            DrawTextureRec({combinedTarget.colorTexture, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R16G16B16A16},
                           {0, 0, (float) w, (float) -h}, {0, 0}, WHITE);
            EndShaderMode();
        }
        EndCustomRenderTarget();  // END OF LUT PASS.

        BeginDrawing();
        {
            // 2d ops and pp
            ClearBackground(BLACK);

            if (IsKeyPressed(KEY_F6)) {
                debugViewMode = (debugViewMode + 1) % 4;
            }

            Texture2D presentTexture = {finalTarget.colorTexture, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R16G16B16A16};
            if (debugViewMode == 1) {
                presentTexture = {combinedTarget.colorTexture, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R16G16B16A16};
            } else if (debugViewMode == 2) {
                presentTexture = {albedoTarget.colorTexture, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R16G16B16A16};
            } else if (debugViewMode == 3) {
                presentTexture = {lightTarget.colorTexture, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R16G16B16A16};
            }

            DrawTextureRec(presentTexture, {0, 0, (float) w, (float) -h}, {0, 0}, WHITE);
            if (debugViewMode == 0) DrawText("F6: final", 10, 30, 20, WHITE);
            if (debugViewMode == 1) DrawText("F6: combined", 10, 30, 20, YELLOW);
            if (debugViewMode == 2) DrawText("F6: albedo", 10, 30, 20, ORANGE);
            if (debugViewMode == 3) DrawText("F6: light", 10, 30, 20, GREEN);

            if (IsKeyPressed(KEY_ESCAPE))
                DisableCursor();

#if !IS_DUKE_INCLUDED
            {
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
                        mdl.camera_controls_enabled = false;
                        EnableCursor();
                    } else {
                        mdl.camera_controls_enabled = true;
                        DisableCursor();
                    }
                }

                rlImGuiEnd();
            } // END OF IMGUI PASS
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

int main(int argc, char **argv) {
    g_argc = argc;
    g_argv = argv;
    if (argv != nullptr && argv[0] != nullptr && argv[0][0]) {
        std::error_code ec;
        std::filesystem::path exe_dir = std::filesystem::absolute(argv[0], ec).parent_path();
        if (!ec && !exe_dir.empty()) {
            std::filesystem::current_path(exe_dir, ec);
        }
    }
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
