#ifndef DUMBCORE_HPP
#define DUMBCORE_HPP

#include "raylib.h"
#include "raymath.h"
extern "C" {
    #include "mapcore.h"
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

public:
    static mapstate_t *map;
    static void Init(mapstate_t* loadedMap) {
        if (initialized) return;
        map = loadedMap;

        cam.position = {map->startpos.x, -map->startpos.y, map->startpos.z};
        cam.target = {0.0f, 0.0f, 0.0f};
        cam.up = {0.0f, 1.0f, 0.0f};
        cam.speed = 50.0f;
        
        camera.position = cam.position;
        camera.target = cam.target;
        camera.up = cam.up;
        camera.fovy = 60.0f;
        camera.projection = CAMERA_PERSPECTIVE;
        
        initialized = true;
    }

    static void Update(float deltaTime) {
        if (!initialized) return;
        
        UpdateFreeCamera(deltaTime);
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
    static void UpdateFreeCamera(float deltaTime) {
        float speed = cam.speed * deltaTime;

        Vector3 forward = Vector3Normalize(Vector3Subtract(cam.target, cam.position));
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, cam.up));

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
mapstate_t* DumbCore::map = nullptr;
#endif // DUMBCORE_HPP
