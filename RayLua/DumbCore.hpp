#ifndef DUMBCORE_HPP
#define DUMBCORE_HPP

// todo: get map loaded from console
// add pal 6
// stub to select one sprite, move around xy and rotate. - ask ai to gen the code.
// make quats.
// focus - automatic - R enter rotation - space confirm return

#include "raylib.h"
#include "raymath.h"



extern "C" {
#include "shadowtest2.h"
#include "monodebug.h"
#include "mapcore.h"
#include "physics.h"
#include "loaders.h"
#include "b2rlmath.h"
#include "interfaces/engineapi.h"
#if IS_DUKE_INCLUDED
#include "DukeGame/source/dukewrap.h"
#include "DukeGame/source/game.h"
#endif
}

class DumbCore {

private:
    struct FreeCamera {
        float speed;
    };

    static FreeCamera cam;
    static Camera3D camera;
    static bool initialized;
    static int cursec;


public:
    static mapstate_t *map;
    static point3d b2pos;
    static void Init(mapstate_t *loadedMap) {
        if (initialized) return;
        map = loadedMap;
        b2pos = map->startpos;
        camera.position = buildToRaylib(b2pos);
        updatesect_imp(camera.position.x, -camera.position.z, camera.position.y, &cursec, map);

        camera.target = {0.0f, 0.0f, 0.0f};
        camera.up = {0.0f, 1.0f, 0.0f};
        cam.speed = 10.0f;

        camera.position = camera.position;
        camera.target = camera.target;
        camera.up = camera.up;
        camera.fovy = 90.0f;
        camera.projection = CAMERA_PERSPECTIVE;

        initialized = true;
        InitEngineApi(map);
#if IS_DUKE_INCLUDED
        InitDukeWrapper(&engine);
        InitDuke();
#endif
    }

    static void Update(float deltaTime) {
        if (!initialized) return;

#if IS_DUKE_INCLUDED
        UpdateViaDuke(deltaTime);
#else
        UpdateFreeCamera(deltaTime);
#endif
        HandleInteraction();

        camera.position = camera.position;
        camera.target = camera.target;
        camera.up = camera.up;
    }

    static Camera3D* GetCamera() {
        return &camera;
    }

    static void SetCameraPosition(Vector3 pos) {
        camera.position = pos;
        camera.target = Vector3Add(pos, {0, 0, -1});
    }

private:
    static point3d RaylibToBuild(Vector3 vec) {
        return {vec.x, vec.z, -vec.y};
    }

    static void UpdateFreeCamera(float deltaTime) {
        point3d camposb2 = {b2pos.x, b2pos.y, b2pos.z};
        Vector3 startpos = camera.position;
        float speed = cam.speed * deltaTime;

        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
        captureframe = false;
        g_captureframe = false;

        if (IsKeyPressed(KEY_G)) {
            captureframe = true;
            g_captureframe = true;
            mono_dbg_clear();
        }

        // pos in build2
        if (IsKeyDown(KEY_Q) && IsKeyDown(KEY_LEFT_SHIFT)) {
            float rollSpeed = 2.0f * deltaTime; // Adjust roll speed as needed
            camera.up = Vector3RotateByAxisAngle(camera.up, forward, rollSpeed);
        }
        if (IsKeyDown(KEY_E)&& IsKeyDown(KEY_LEFT_SHIFT)) {
            float rollSpeed = 2.0f * deltaTime;
            camera.up = Vector3RotateByAxisAngle(camera.up, forward, -rollSpeed);
        }

        // Normalize up vector to prevent drift
        camera.up = Vector3Normalize(camera.up);

        // WASD movement
        if (IsKeyDown(KEY_W)) {
            camera.position = Vector3Add(camera.position, Vector3Scale(forward, speed));
            camera.target = Vector3Add(camera.target, Vector3Scale(forward, speed));
        }
        if (IsKeyDown(KEY_S)) {
            camera.position = Vector3Subtract(camera.position, Vector3Scale(forward, speed));
            camera.target = Vector3Subtract(camera.target, Vector3Scale(forward, speed));
        }
        if (IsKeyDown(KEY_A)) {
            camera.position = Vector3Subtract(camera.position, Vector3Scale(right, speed));
            camera.target = Vector3Subtract(camera.target, Vector3Scale(right, speed));
        }
        if (IsKeyDown(KEY_D)) {
            camera.position = Vector3Add(camera.position, Vector3Scale(right, speed));
            camera.target = Vector3Add(camera.target, Vector3Scale(right, speed));
        }

        //those funcs still use internal build coords.
        point3d movevec = RaylibToBuild(camera.position - startpos);
        point3d mv = {movevec.x, movevec.y, movevec.z};
        camposb2.x +=movevec.x;        camposb2.y+=movevec.y;        camposb2.z+=movevec.z;
      //  collmove_p(&camposb2, &cursec, &mv, 0.25, 1, map);
        updatesect_imp(camposb2.x, camposb2.y, camposb2.z, &cursec, map);

        b2pos = {camposb2.x, camposb2.y, camposb2.z};
        camera.position.x = b2pos.x;
        camera.position.y = -b2pos.z;
        camera.position.z = b2pos.y;
        Vector2 mouseDelta = GetMouseDelta();
        if (mouseDelta.x != 0 || mouseDelta.y != 0) {
            float sensitivity = 0.003f;

            Vector3 targetOffset = Vector3Subtract(camera.target, camera.position);

            // Yaw around WORLD up (0,1,0), not camera up
            Vector3 worldUp = {0, 1, 0};
            targetOffset = Vector3RotateByAxisAngle(targetOffset, worldUp, -mouseDelta.x * sensitivity);

            // Pitch around current right vector (this stays the same)
            Vector3 right = Vector3Normalize(Vector3CrossProduct(targetOffset, worldUp));
            targetOffset = Vector3RotateByAxisAngle(targetOffset, right, -mouseDelta.y * sensitivity);

            camera.target = Vector3Add(camera.position, targetOffset);

            // Update camera up to match new orientation
            Vector3 forward = Vector3Normalize(targetOffset);
            camera.up = Vector3Normalize(Vector3CrossProduct(right, forward));
        }
    }
#if IS_DUKE_INCLUDED
    static void UpdateViaDuke(float deltaTime) {

        // WASD movement
        if (IsKeyDown(KEY_W))
            engine.Inputs[W_FRW] = 1;
        else
            engine.Inputs[W_FRW] = 0;

        engine.Inputs[S_BACK] = IsKeyDown(KEY_S) ? 1 : 0;
        engine.Inputs[A_LEFT] = IsKeyDown(KEY_A) ? 1 : 0;
        engine.Inputs[D_RIGHT] = IsKeyDown(KEY_D) ? 1 : 0;
        engine.Inputs[SPC_JUMP] = IsKeyDown(KEY_SPACE) ? 1 : 0;
        engine.Inputs[E_USE] = IsKeyDown(KEY_E) ? 1 : 0;
        engine.Inputs[CROUCH] = IsKeyDown(KEY_LEFT_CONTROL) ? 1 : 0;
        engine.Inputs[MB_SHOOT] = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? 1 : 0;
        Vector2 mdelta = GetMouseDelta();
        engine.Inputs[Q_TLEFT] = mdelta.x<0 ? 1 : 0;
        engine.Inputs[R_TRIGHT] = mdelta.x>0 ? 1 : 0;
        engine.Inputs[ACT_AIM_DOWN] = mdelta.y > 0 ? 1 : 0; // inverted
        engine.Inputs[ACT_AIM_UP] = mdelta.y < 0 ? 1 : 0;
        ForwardEngineUpdate(deltaTime);
        point3d ppos = GetPlayerPos();
        point3d frw = GetPlayerFrw();
        camera.position.x = ppos.x;
        camera.position.y = -ppos.z;
        camera.position.z = ppos.y;
        camera.target = Vector3Add(camera.position, {frw.x, -frw.z, frw.y});
    }
#endif
    static void HandleInteraction() {
        if (IsKeyPressed(KEY_E)) {
            OnInteract();
        }
    }

    static void OnInteract() {
        // Stub for interaction - override this for actual functionality
        // For now, just print to console or do nothing
    }
};

// Static member definitions
DumbCore::FreeCamera DumbCore::cam = {};
Camera3D DumbCore::camera = {};
bool DumbCore::initialized = false;
int DumbCore::cursec = false;

mapstate_t *DumbCore::map = nullptr;
point3d DumbCore::b2pos;
#endif // DUMBCORE_HPP
