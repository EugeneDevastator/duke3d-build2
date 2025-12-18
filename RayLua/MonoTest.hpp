//
// Created by omnis on 12/1/2025.
//

#ifndef RAYLIB_LUA_IMGUI_MONOTEST_H
#define RAYLIB_LUA_IMGUI_MONOTEST_H
#include "raylib.h"

#include <vector>
#include <cmath>
extern "C"{
#include "Core/monoclip.h"
#include "Core/mapcore.h"
}
struct Polygon3D {
    std::vector<Vector3> vertices;
};
static bdrawctx b={};
static std::vector<std::vector<Vector3>>* userdata = nullptr;
class MonoClip3D {
public:
    static std::vector<std::vector<Vector3>> ClipPolygons2D(
    const std::vector<Vector3>& subject,
    const std::vector<Vector3>& clip,
    int operation) {

        static bool initialized = false;
        if (!initialized) {
            mono_initonce();
            initialized = true;
        }

        std::vector<std::vector<Vector3>> result;
        bdrawctx b = {0};
        userdata = &result;

        // Convert to dpoint3d with Z=0 for 2D operation
        int subj_h0 = -1, subj_h1 = -1;
        if (subject.size() >= 3) {
            dpoint3d* subj_points = new dpoint3d[subject.size()];
            for (size_t i = 0; i < subject.size(); i++) {
                subj_points[i] = {subject[i].x, subject[i].y, subject[i].z};
            }
            mono_genfromloop(&subj_h0, &subj_h1, subj_points, subject.size());
            delete[] subj_points;
        }

        int clip_h0 = -1, clip_h1 = -1;
        if (clip.size() >= 3) {
            dpoint3d* clip_points = new dpoint3d[clip.size()];
            for (size_t i = 0; i < clip.size(); i++) {
                clip_points[i] = {clip[i].x, clip[i].y,  clip[i].z};
            }
            mono_genfromloop(&clip_h0, &clip_h1, clip_points, clip.size());
            delete[] clip_points;
        }

        // Perform boolean operation
        mono_bool(subj_h0, subj_h1, clip_h0, clip_h1, operation, &b, output_callback_2d);

        // Cleanup
        if (subj_h0 >= 0) mono_deloop(subj_h0);
        if (subj_h1 >= 0) mono_deloop(subj_h1);
        if (clip_h0 >= 0) mono_deloop(clip_h0);
        if (clip_h1 >= 0) mono_deloop(clip_h1);

        return result;
    }
    static std::vector<std::vector<Vector3>> SelfClip(const std::vector<Vector3>& subject) {

        static bool initialized = false;
        if (!initialized) {
            mono_initonce();
            initialized = true;
        }

        std::vector<std::vector<Vector3>> result;
        bdrawctx b = {0};
        userdata = &result;

        // Convert to dpoint3d with Z=0 for 2D operation
        int subj_h0 = -1, subj_h1 = -1;
        if (subject.size() >= 3) {
            dpoint3d* subj_points = new dpoint3d[subject.size()];
            for (size_t i = 0; i < subject.size(); i++) {
                subj_points[i] = {subject[i].x, subject[i].y, subject[i].z};
            }
            mono_genfromloop(&subj_h0, &subj_h1, subj_points, subject.size());
            delete[] subj_points;
        }


        // Perform boolean operation
       //mono_max(subj_h0,subj_h1,1,1);
        mono_clipself(subj_h0, subj_h1, &b, output_callback_2d);

        // Cleanup
        if (subj_h0 >= 0) mono_deloop(subj_h0);
        if (subj_h1 >= 0) mono_deloop(subj_h1);

        return result;
    }
private:

    // looks like it will be called multiple times with each resulting polygon,
    static void output_callback_2d(int h0, int h1, bdrawctx* b) {
        if (h0 < 0) return;

        std::vector<std::vector<Vector3>>* result = static_cast<std::vector<std::vector<Vector3>>*>(userdata);

        // Add polygon from h0
        std::vector<Vector3> polygon;
        int i = h1;
        i=h0;
        do {
            polygon.push_back({(float)mp[i].x, (float)mp[i].y, 0});
            i = mp[i].n;
        } while (i != h0);
        mono_deloop(h0);
        std::vector<Vector3> rchain;
        i = h1;
        if (h1 >= 0 && h1 != h0) {
            i = h1;
            do {
                rchain.push_back({(float)mp[i].x, (float)mp[i].y, 0});
                i = mp[i].n;
            } while (i != h1);
        }
        mono_deloop(h1);

        for (int r = rchain.size()-1; r >= 0; r--) {
            polygon.push_back(rchain[r]);
        }
        // Add polygon from h1 if different

        result->push_back(polygon);
    }
};



void PerformPolygonClipping2D() {
    std::vector<Vector3> square = {
        {-1.0f, -1.0f,1.0f/3},
        { 1.0f, -1.0f,1.0f/3},
        { 1.0f,  1.0f,0.0f},
        {-1.0f,  1.0f,0.0f}
    };

    std::vector<Vector3> triangle = {
        { 0.0f, -0.5f,0.0f},
        { 0.5f,  1.5f,-1.0f/3},
        {-0.5f,  0.5f,1.0f}
    };
    std::vector<Vector3> cave = {
        { 0.0f, 0.0f,0.1f},

           { 0.9f, -0.2f,0.1f},
           { 0.7f, -0.4f,0.1f},

        { 0.0f, -1.0f,0.1f},
            { 0.9f, -0.4f,0.1f},
        { 1.0f, -1.0f,0.1f},
        { 2.0f, -0.5f,0.1f},
        { 1.5f, 0.7f,0.1f},
        { 1.2f, -0.7f,0.1f},
        { 1.0f, 0.0f,0.1f},

    };
    // Perform subtraction (cut hole)
    std::vector<std::vector<Vector3>> result;
   // result = MonoClip3D::SelfClip(cave);
    result= MonoClip3D::ClipPolygons2D(square, triangle, MONO_BOOL_SUB_REV);
    //result
    for (size_t i = 0; i < cave.size(); i++) {
        size_t next = (i + 1) % cave.size();
        DrawLine3D({cave[i].x, cave[i].y, cave[i].z}, {cave[next].x, cave[next].y, cave[next].z}, BLUE);
    }

    // Draw original polygons
    for (size_t i = 0; i < square.size(); i++) {
        size_t next = (i + 1) % square.size();
        DrawLine3D({square[i].x, square[i].y, square[i].z}, {square[next].x, square[next].y, square[next].z}, BLUE);
    }

     for (size_t i = 0; i < triangle.size(); i++) {
         size_t next = (i + 1) % triangle.size();
         DrawLine3D({triangle[i].x, triangle[i].y, triangle[i].z}, {triangle[next].x, triangle[next].y, triangle[next].z}, RED);
     }

    // Draw all result polygons
    Color colors[] = {GREEN, YELLOW, ORANGE, PURPLE};
    for (size_t p = 0; p < result.size(); p++) {
        Color color = colors[p % 4];
        const auto& polygon = result[p];
        for (size_t i = 0; i < polygon.size(); i++) {
            size_t next = (i + 1) % polygon.size();
            DrawLine3D({polygon[i].x, polygon[i].y, polygon[i].z+p*0.2f}, {polygon[next].x, polygon[next].y, polygon[next].z+p*0.2f}, color);
        }
    }
}
void RunVisualization() {
    // Setup camera
    Camera3D camera = { 0 };
    camera.position = { 3.0f, 3.0f, 3.0f };
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    while (!WindowShouldClose()) {
        // Update camera
        UpdateCamera(&camera, CAMERA_ORBITAL);

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(camera);
        PerformPolygonClipping2D();
        EndMode3D();

        // Draw UI info
        DrawText("Use mouse to orbit camera", 10, 10, 20, WHITE);
        DrawText("Original: BLUE, Cut shape: RED, Result: GREEN", 10, 40, 20, WHITE);

        EndDrawing();
    }
}
#endif //RAYLIB_LUA_IMGUI_MONOTEST_H