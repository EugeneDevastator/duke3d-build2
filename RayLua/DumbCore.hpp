#ifndef DUMBCORE_HPP
#define DUMBCORE_HPP

#include "raylib.h"
#include "raymath.h"


extern "C" {
#include "mapcore.h"
#include "physics.h"
#include "loaders.h"
#include "interfaces/engineapi.h"
#if IS_DUKE_INCLUDED
#include "DukeGame/source/dukewrap.h"
#include "DukeGame/source/game.h"
#endif
}


class DumbCore {
private:
    struct FreeCamera {
        Vector3 position;
        Vector3 target;
        Vector3 up;
        float speed;
    };

    static FreeCamera cam;
    static Camera3D camera;
    static bool initialized;
    static int cursec;
    static point3d b2pos;

public:
    static mapstate_t *map;

    static void Init(mapstate_t *loadedMap) {
        if (initialized) return;
        map = loadedMap;
        b2pos = map->startpos;
        point3d pos = buildToRaylib(b2pos);
        cam.position = {pos.x, pos.y, pos.z};
        updatesect_imp(cam.position.x, -cam.position.z, cam.position.y, &cursec, map);

        cam.target = {0.0f, 0.0f, 0.0f};
        cam.up = {0.0f, 1.0f, 0.0f};
        cam.speed = 10.0f;

        camera.position = cam.position;
        camera.target = cam.target;
        camera.up = cam.up;
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

        camera.position = cam.position;
        camera.target = cam.target;
        camera.up = cam.up;
    }

    static Camera3D GetCamera() {
        return camera;
    }

    static void SetCameraPosition(Vector3 pos) {
        cam.position = pos;
        cam.target = Vector3Add(pos, {0, 0, -1});
    }

private:
    static point3d RaylibToBuild(Vector3 vec) {
        return {vec.x, vec.z, -vec.y};
    }

    static void UpdateFreeCamera(float deltaTime) {
        point3d camposb2 = {b2pos.x, b2pos.y, b2pos.z};
        Vector3 startpos = cam.position;
        float speed = cam.speed * deltaTime;

        Vector3 forward = Vector3Normalize(Vector3Subtract(cam.target, cam.position));
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, cam.up));

        // pos in build2


        // WASD movement
        if (IsKeyDown(KEY_W)) {
            cam.position = Vector3Add(cam.position, Vector3Scale(forward, speed));
            cam.target = Vector3Add(cam.target, Vector3Scale(forward, speed));
        }
        if (IsKeyDown(KEY_S)) {
            cam.position = Vector3Subtract(cam.position, Vector3Scale(forward, speed));
            cam.target = Vector3Subtract(cam.target, Vector3Scale(forward, speed));
        }
        if (IsKeyDown(KEY_A)) {
            cam.position = Vector3Subtract(cam.position, Vector3Scale(right, speed));
            cam.target = Vector3Subtract(cam.target, Vector3Scale(right, speed));
        }
        if (IsKeyDown(KEY_D)) {
            cam.position = Vector3Add(cam.position, Vector3Scale(right, speed));
            cam.target = Vector3Add(cam.target, Vector3Scale(right, speed));
        }

        //those funcs still use internal build coords.
        point3d movevec = RaylibToBuild(cam.position - startpos);
        point3d mv = {movevec.x, movevec.y, movevec.z};
        collmove_p(&camposb2, &cursec, &mv, 0.25, 1, map);
        updatesect_imp(camposb2.x, camposb2.y, camposb2.z, &cursec, map);

        b2pos = {camposb2.x, camposb2.y, camposb2.z};

        cam.position.x = b2pos.x;
        cam.position.y = -b2pos.z;
        cam.position.z = b2pos.y;
        // Mouse look
        Vector2 mouseDelta = GetMouseDelta();
        if (mouseDelta.x != 0 || mouseDelta.y != 0) {
            float sensitivity = 0.003f;

            Vector3 targetOffset = Vector3Subtract(cam.target, cam.position);
            targetOffset = Vector3RotateByAxisAngle(targetOffset, cam.up, -mouseDelta.x * sensitivity);
            targetOffset = Vector3RotateByAxisAngle(targetOffset, right, -mouseDelta.y * sensitivity);

            cam.target = Vector3Add(cam.position, targetOffset);
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
        engine.Inputs[ACT_AIM_DOWN] = mdelta.y < 0 ? 1 : 0;
        engine.Inputs[ACT_AIM_UP] = mdelta.y > 0 ? 1 : 0;
        ForwardEngineUpdate(deltaTime);
        point3d ppos = GetPlayerPos();
        point3d frw = GetPlayerFrw();
        cam.position.x = ppos.x;
        cam.position.y = -ppos.z;
        cam.position.z = ppos.y;
        cam.target = Vector3Add(cam.position, {frw.x, -frw.z, frw.y});
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
point3d DumbCore::b2pos = {0, 0, 0};
mapstate_t *DumbCore::map = nullptr;
#endif // DUMBCORE_HPP
