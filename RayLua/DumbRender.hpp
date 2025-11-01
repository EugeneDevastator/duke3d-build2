//
// Created by omnis on 10/27/2025.
//
/*
*
The X-axis runs horizontally, with positive values extending to the right.
The Y-axis runs vertically, with positive values extending upwards.
In 3D, the Z-axis extends out of the screen, with positive values going forward.

Forward Direction in raylib

In raylib, the forward direction is typically defined as the positive Z-axis in 3D space. This means:

Moving forward increases the Z-coordinate.
The forward direction can be visualized as moving away from the camera or viewer.

*/

#ifndef RAYLIB_LUA_IMGUI_DUMBRENDER_H
#define RAYLIB_LUA_IMGUI_DUMBRENDER_H

extern "C" {
#include "loaders.h"
#include "mapcore.h"
}


#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

struct WallSegment
{
    float z[4]; // bottom-left, bottom-right, top-right, top-left
    bool isVisible;
    int adjacentSector;
};

typedef struct
{
    int s;
    float z;
} vert;

typedef struct
{
    float* vertices;
    float* texcoords;
    unsigned short* indices;
    int vertexCount;
    int triangleCount;
    bool isValid;
    Mesh rendermesh;
} TriangulatedMesh;

typedef struct
{
    TriangulatedMesh mesh;
    int textureIndex;
    bool isValid;
} FloorMeshData;

static FloorMeshData* floorMeshes = NULL;
static int numFloorMeshes = 0;
static Texture2D* runtimeTextures;
static mapstate_t* map;
static long gnumtiles_i, gmaltiles_i, gtilehashead_i[1024];

class DumbRender
{
public:
    static mapstate_t* GetMap()
    {
        return map;
    }

    static void Init()
    {
        char rootpath[256];
        strcpy_s(rootpath, "c:/Eugene/Games/build2/");
        LoadPal(rootpath);
        LoadMapAndTiles();
        GenerateTextures();
        InitMapstateTex();
        // auto paltex = ConvertPalToTexture();
        // tile_t* pic = static_cast<tile_t*>(malloc(sizeof(tile_t)));
        // strcpy_s(pic->filnam, "TILES000.art|1");
        // auto tex = ConvertPicToTexture(pic);
    }

    // Calculates the Z height at point (x,y) on a sloped surface
    // Uses the sector's gradient and base Z value
    // original, correct code
    double getslopez(sect_t* s, int i, double x, double y)
    {
        wall_t* wal = s->wall;
        // Calculate Z using plane equation: gradient dot (point - reference) + base_z
        return ((wal[0].x - x) * s->grad[i].x + (wal[0].y - y) * s->grad[i].y + s->z[i]);
    }

    // Calculate Z-coordinate at point using sector slope
    static float GetSlopeZ(sect_t* sect, int isFloor, float x, float y)
    {
        //   float baseZ = sect->z[isFloor];
        //   point2d* grad = &sect->grad[isFloor];

        //   // Use first wall as reference point
        //   if (sect->n > 0) {
        //       wall_t* refWall = &sect->wall[0];
        //       float dx = x - refWall->x;
        //       float dy = y - refWall->y;
        //       return baseZ + dx * grad->x + dy * grad->y;
        //   }
        //   return baseZ;
        wall_t* wal = sect->wall;
        // Calculate Z using plane equation: gradient dot (point - reference) + base_z
        return -((wal[0].x - x) * sect->grad[isFloor].x + (wal[0].y - y) * sect->grad[isFloor].y + sect->z[isFloor]);
    }

    // Updated floor mesh generation with slopes
    static void InitMapstateTex(void)
    {
        if (floorMeshes)
        {
            for (int i = 0; i < numFloorMeshes; i++)
            {
                if (floorMeshes[i].isValid)
                {
                    UnloadMesh(floorMeshes[i].mesh.rendermesh);
                    free(floorMeshes[i].mesh.vertices);
                    free(floorMeshes[i].mesh.texcoords);
                    free(floorMeshes[i].mesh.indices);
                }
            }
            free(floorMeshes);
        }

        numFloorMeshes = map->numsects * 2; // floor + ceiling
        floorMeshes = (FloorMeshData*)calloc(numFloorMeshes, sizeof(FloorMeshData));

        for (int s = 0; s < map->numsects; s++)
        {
            sect_t* sect = &map->sect[s];

            // Generate floor and ceiling meshes
            for (int isFloor = 0; isFloor < 2; isFloor++)
            {
                int meshIdx = s * 2 + isFloor;
                FloorMeshData* meshData = &floorMeshes[meshIdx];

                meshData->isValid = false;

                if (sect->n >= 3 && sect->surf[isFloor].tilnum >= 0 && sect->surf[isFloor].tilnum < get_gnumtiles())
                {
                    TriangulatedMesh triMesh = {0};
                    triMesh.vertexCount = sect->n;
                    triMesh.triangleCount = sect->n - 2;

                    triMesh.vertices = (float*)malloc(triMesh.vertexCount * 3 * sizeof(float));
                    triMesh.texcoords = (float*)malloc(triMesh.vertexCount * 2 * sizeof(float));
                    triMesh.indices = (unsigned short*)malloc(triMesh.triangleCount * 3 * sizeof(unsigned short));

                    // Fill vertices with slope calculations
                    for (int w = 0; w < sect->n; w++)
                    {
                        const wall_t* wall = &sect->wall[w];
                        float z = GetSlopeZ(sect, isFloor, wall->x, wall->y);

                        triMesh.vertices[w * 3] = wall->x;
                        triMesh.vertices[w * 3 + 1] = z;
                        triMesh.vertices[w * 3 + 2] = wall->y;
                        // uvs are matrix transform.. converting pos to uv.
                        // [u]   [uv[1].x  uv[2].x  uv[0].x] [wall->x]
                        // [v] = [uv[1].y  uv[2].y  uv[0].y] [wall->y]
                        // [1]   [   0        0        1   ] [   1   ]
                        triMesh.texcoords[w * 2] =  wall->x * sect->surf[isFloor].uv[1].x + wall->y * sect->surf[isFloor].uv[2].x+ sect->surf[isFloor].uv[0].x;
                        triMesh.texcoords[w * 2 + 1] =  wall->x * sect->surf[isFloor].uv[1].y + wall->y * sect->surf[isFloor].uv[2].y+ sect->surf[isFloor].uv[0].y;
                    }

                    // Triangulate
                    float* polyVertices = (float*)malloc(sect->n * 2 * sizeof(float));
                    for (int w = 0; w < sect->n; w++)
                    {
                        polyVertices[w * 2] = sect->wall[w].x;
                        polyVertices[w * 2 + 1] = sect->wall[w].y;
                    }

                    triMesh.triangleCount = TriangulatePolygon(polyVertices, sect->n, triMesh.indices);
                    triMesh.isValid = true;

                    // Create Raylib mesh
                    Mesh mesh = {0};
                    mesh.vertexCount = triMesh.vertexCount;
                    mesh.triangleCount = triMesh.triangleCount;
                    mesh.vertices = triMesh.vertices;
                    mesh.texcoords = triMesh.texcoords;
                    mesh.indices = triMesh.indices;

                    UploadMesh(&mesh, false);

                    meshData->mesh.rendermesh = mesh;
                    meshData->mesh = triMesh;
                    meshData->textureIndex = sect->surf[isFloor].tilnum;
                    meshData->isValid = true;

                    free(polyVertices);
                }
            }
        }
    }


    // Updated wall rendering with segments
    static void DrawMapstateTex(Camera3D cam)
    {
        rlDrawRenderBatchActive();
        rlDisableBackfaceCulling();

        // Draw floors and ceilings with slopes
        for (int s = 0; s < map->numsects; s++)
        {
            for (int isFloor = 0; isFloor < 2; isFloor++)
            {
                int meshIdx = s * 2 + isFloor;
                FloorMeshData* meshData = &floorMeshes[meshIdx];

                if (meshData->isValid)
                {
                    const Texture2D tex = runtimeTextures[meshData->textureIndex];

                    rlBegin(RL_TRIANGLES);
                    rlSetTexture(tex.id);
                    rlDisableBackfaceCulling();

                    for (int tri = 0; tri < meshData->mesh.triangleCount; tri++)
                    {
                        for (int vert = 0; vert < 3; vert++)
                        {
                            int idx = meshData->mesh.indices[tri * 3 + vert];
                            rlColor4ub(255, 255, 255, 255);
                            rlTexCoord2f(
                                meshData->mesh.texcoords[idx * 2],
                                meshData->mesh.texcoords[idx * 2 + 1]
                            );
                            rlVertex3f(
                                meshData->mesh.vertices[idx * 3],
                                meshData->mesh.vertices[idx * 3 + 1],
                                meshData->mesh.vertices[idx * 3 + 2]
                            );
                        }
                    }

                    rlDrawRenderBatchActive();
                    rlSetTexture(0);
                    rlEnd();
                }
            }
        }

        // draw walls
        rlDisableBackfaceCulling();
        for (int s = 0; s < map->numsects; s++)
        {
            sect_t* sect = &map->sect[s];

            for (int w = 0; w < sect->n; w++)
            {
                wall_t* wall = &sect->wall[w];
                wall_t* nextwall = &sect->wall[(w + 1) % sect->n];

                // Get wall heights with slope support
                float bottomLeftZ = GetSlopeZ(sect, 1, wall->x, wall->y); // floor at wall start
                float bottomRightZ = GetSlopeZ(sect, 1, nextwall->x, nextwall->y); // floor at wall end
                float topLeftZ = GetSlopeZ(sect, 0, wall->x, wall->y); // ceiling at wall start
                float topRightZ = GetSlopeZ(sect, 0, nextwall->x, nextwall->y); // ceiling at wall end

                // raylib friendly coords
                Vector3 bottomLeft = {wall->x, bottomLeftZ, wall->y};
                Vector3 bottomRight = {nextwall->x, bottomRightZ, nextwall->y};
                Vector3 topLeft = {wall->x, topLeftZ, wall->y};
                Vector3 topRight = {nextwall->x, topRightZ, nextwall->y};

                int texIndex = wall->surf.tilnum;
                if (wall->xsurf && wall->surfn > 1)
                    texIndex = wall->xsurf[0].tilnum;

                float dx = sqrt((nextwall->x - wall->x) * (nextwall->x - wall->x) +
                    (nextwall->y - wall->y) * (nextwall->y - wall->y));
                float dy = sect->z[0] - sect->z[1];
                rlEnableDepthMask();
                rlDisableBackfaceCulling();
                if (wall->ns == -1)
                {
                    // Solid wall - draw full wall
                    if (texIndex >= 0 && texIndex < get_gnumtiles())
                    {
                        float dy = topLeftZ - bottomLeftZ; // Use actual height difference

                        Texture2D wallTex = runtimeTextures[texIndex];
                        rlSetTexture(wallTex.id);
                        rlBegin(RL_QUADS);
                        rlColor4ub(255, 255, 255, 255);

                        rlTexCoord2f(0.0f, 1.0f * wall->surf.uv[2].y * dy);
                        rlVertex3f(bottomLeft.x, bottomLeft.y, bottomLeft.z);
                        rlTexCoord2f(1.0f * wall->surf.uv[1].x * dx, 1.0f * wall->surf.uv[2].y * dy);
                        rlVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);
                        rlTexCoord2f(1.0f * wall->surf.uv[1].x * dx, 0.0f);
                        rlVertex3f(topRight.x, topRight.y, topRight.z);
                        rlTexCoord2f(0.0f, 0.0f);
                        rlVertex3f(topLeft.x, topLeft.y, topLeft.z);

                        rlEnd();
                        rlSetTexture(0);
                    }
                }
                else // rendering red wall of this sector.
                {
                    // Portal wall - draw upper and lower parts
                    sect_t* nextSect = &map->sect[wall->ns];
                    int lowtile,masktile,hitile = wall->surf.tilnum;
                    if (wall->surfn==3)
                    {
                        int lowtileind = wall->surf.flags & 2 ? 2 : 0;
                        lowtile = wall->xsurf[lowtileind].tilnum;
                        masktile = wall->xsurf[1].tilnum;
                    }

                    // Get heights of adjacent sector with slopes
                    float nextBottomLeftZ = GetSlopeZ(nextSect, 1, wall->x, wall->y);
                    float nextBottomRightZ = GetSlopeZ(nextSect, 1, nextwall->x, nextwall->y);
                    float nextTopLeftZ = GetSlopeZ(nextSect, 0, wall->x, wall->y);
                    float nextTopRightZ = GetSlopeZ(nextSect, 0, nextwall->x, nextwall->y);

                    // Draw upper wall (between current ceiling and next ceiling)
                    if (topLeftZ > nextTopLeftZ || topRightZ > nextTopRightZ)
                    {
                        Vector3 bottom_left = {wall->x, nextTopLeftZ, wall->y};
                        Vector3 bottom_right = {nextwall->x, nextTopRightZ, nextwall->y};

                        if (hitile >= 0 && hitile < get_gnumtiles())
                        {
                            float upperDy = topLeftZ - nextTopLeftZ; // affects other paart
                            float selfDy = topLeftZ - topRightZ; // affects upper part
                            float dh = nextTopLeftZ -nextTopRightZ;
                            Texture2D upperTex = runtimeTextures[hitile];
                            rlSetTexture(upperTex.id);
                            rlBegin(RL_QUADS);
                            rlColor4ub(255, 255, 255, 255);

                            rlTexCoord2f(0.0f, 1.0f * wall->surf.uv[2].y *(upperDy+selfDy));
                            rlVertex3f(bottom_left.x, bottom_left.y, bottom_left.z);

                            rlTexCoord2f(wall->surf.uv[1].x * dx, wall->surf.uv[2].y*(upperDy + dh));
                            rlVertex3f(bottom_right.x, bottom_right.y, bottom_right.z);

                            rlTexCoord2f(1.0f  * wall->surf.uv[1].x * dx, 0.0f);
                            rlVertex3f(topRight.x, topRight.y, topRight.z);

                            rlTexCoord2f(0.0f, 0.0);
                            rlVertex3f(topLeft.x, topLeft.y, topLeft.z);
                            rlEnd();
                            rlSetTexture(0);
                        }
                    }

                    // Draw lower wall (between current floor and next floor)

                    if (bottomLeftZ < nextBottomLeftZ || bottomRightZ < nextBottomRightZ)
                    {

                        Vector3 thisTopLeft = {wall->x, nextBottomLeftZ, wall->y};
                        Vector3 thisTopRight = {nextwall->x, nextBottomRightZ, nextwall->y};

                        if (lowtile >= 0 && lowtile < get_gnumtiles())
                        {
                            float largeDy = bottomLeftZ - nextBottomLeftZ; // affects other paart
                            float selfDy = bottomLeftZ - bottomRightZ; // affects upper part
                            float dh = nextBottomLeftZ -nextBottomRightZ;
                            Texture2D lowerTex = runtimeTextures[lowtile];

                            rlSetTexture(lowerTex.id);
                            rlBegin(RL_QUADS);

                            rlColor4ub(255, 255, 255, 255);
// CW starting from upper left
                            rlTexCoord2f(0.0f, 1.0f * wall->surf.uv[2].y *(largeDy+selfDy));
                            rlVertex3f(thisTopLeft.x, thisTopLeft.y, thisTopLeft.z);

                            rlTexCoord2f(wall->surf.uv[1].x * dx, wall->surf.uv[2].y*(largeDy + dh));
                            rlVertex3f(thisTopRight.x, thisTopRight.y, thisTopRight.z);

                            rlTexCoord2f(1.0f  * wall->surf.uv[1].x * dx, 0.0f);
                            rlVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);

                            rlTexCoord2f(0.0f, 0.0);
                            rlVertex3f(bottomLeft.x, bottomLeft.y, bottomLeft.z);
                            rlEnd();
                            rlSetTexture(0);
                        }
                    }
                    // need separate transparent pass for it.
                    // Draw middle wall (masked/transparent texture between sectors)
                    // todo - make transparent queue
                    if (wall->xsurf[1].asc > 0 && masktile < get_gnumtiles())
                    {
                            rlEnableBackfaceCulling();
                            rlDisableDepthMask();
                            Vector3 midBottomLeft = {wall->x, max(bottomLeftZ, nextBottomLeftZ), wall->y};
                            Vector3 midBottomRight = {nextwall->x, max(bottomRightZ, nextBottomRightZ), nextwall->y};
                            Vector3 midTopLeft = {wall->x, min(topLeftZ, nextTopLeftZ), wall->y};
                            Vector3 midTopRight = {nextwall->x, min(topRightZ, nextTopRightZ), nextwall->y};

                            float midDy = min(topLeftZ, nextTopLeftZ) - max(bottomLeftZ, nextBottomLeftZ);

                            Texture2D midTex = runtimeTextures[masktile];
                            rlSetTexture(midTex.id);
                            rlBegin(RL_QUADS);
                            rlColor4ub(255, 255, 255, 128); // todo update transp.

                            rlTexCoord2f(0.0f, 1.0f * midDy);
                            rlVertex3f(midBottomLeft.x, midBottomLeft.y, midBottomLeft.z);
                            rlTexCoord2f(1.0f * dx, 1.0f * midDy);
                            rlVertex3f(midBottomRight.x, midBottomRight.y, midBottomRight.z);
                            rlTexCoord2f(1.0f * dx, 0.0f);
                            rlVertex3f(midTopRight.x, midTopRight.y, midTopRight.z);
                            rlTexCoord2f(0.0f, 0.0f);
                            rlVertex3f(midTopLeft.x, midTopLeft.y, midTopLeft.z);

                            rlEnd();
                            rlSetTexture(0);
                            rlEnableDepthMask();

                    }
                }
            }
        }
        rlEnableDepthMask();
        rlDisableBackfaceCulling();
        // Draw sprites (unchanged)
        for (int i = 0; i < map->numspris; i++)
        {
            spri_t* spr = &map->spri[i];
            if (spr->tilnum >= 0 && spr->tilnum < gnumtiles_i) // sprites
            {
                Texture2D spriteTex = runtimeTextures[spr->tilnum];
                Vector3 rg = {spr->r.x, spr->r.y, spr->r.z};
                Vector3 rg9 = {spr->r.y, spr->r.x, spr->r.z};
                Vector3 dw = {spr->d.x, spr->d.y, spr->d.z};
                Vector3 frw = {spr->f.x, spr->f.y, spr->f.z};
                Vector3 pos = {spr->p.x, spr->p.y, spr->p.z};
                Vector3 a = pos+rg+frw;
                Vector3 b = pos+rg-frw;
                Vector3 c = pos-rg-frw;
                Vector3 d = pos-rg+frw;
                if (spr->flags & 32) // spr->flags |= SPRITE_B2_FLAT_POLY;
                {
                    rlDisableDepthMask();
                    rlSetTexture(spriteTex.id);
                    rlBegin(RL_QUADS);
                    rlColor4ub(255, 255, 255, 255); // todo update transp.

                    rlTexCoord2f(0.0f, 1.0f);
                    rlVertex3V(a);
                    rlTexCoord2f(1.0f , 1.0f);
                    rlVertex3V(b);
                    rlTexCoord2f(1.0f , 0.0f);
                    rlVertex3V(c);
                    rlTexCoord2f(0.0f, 0.0f);
                    rlVertex3V(d);

                    rlEnd();
                    rlSetTexture(0);
                    rlEnableDepthMask();
                }
                else
                {

                    auto xs = Vector3Length(rg);
                    auto ys = Vector3Length(dw);
                    int xflip = spr->flags & 4 ? -1 : 1;
                    int yflip = spr->flags & 8 ? -1 : 1;
                    Vector3 pos = {spr->p.x - xs, spr->p.y , spr->p.z};
                    Rectangle source = {0.0f, 0.0f, (float)spriteTex.width, (float)spriteTex.height};
                    xs *= 2;
                    ys *= 2;

                    DrawBillboardRec(cam, spriteTex, source, pos, {xs*xflip, ys*yflip}, WHITE);
                }
            }
        }
    }
static void rlVertex3V(Vector3 v)
    {
        rlVertex3f(v.x, v.y, v.z);
    }
 static bool IsPointInTriangle(float px, float py, float ax, float ay, float bx, float by, float cx, float cy)
{
    float denom = (by - cy) * (ax - cx) + (cx - bx) * (ay - cy);
    if (fabs(denom) < 1e-10f) return false;

    float a = ((by - cy) * (px - cx) + (cx - bx) * (py - cy)) / denom;
    float b = ((cy - ay) * (px - cx) + (ax - cx) * (py - cy)) / denom;
    float c = 1.0f - a - b;

    return a >= 0 && b >= 0 && c >= 0;
}

static float CrossProduct2D(float ax, float ay, float bx, float by, float cx, float cy)
{
    return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}

static bool IsEar(float* vertices, int* vertexList, int remainingVertices, int earIndex)
{
    if (remainingVertices < 3) return false;

    int prev = (earIndex - 1 + remainingVertices) % remainingVertices;
    int curr = earIndex;
    int next = (earIndex + 1) % remainingVertices;

    int v0 = vertexList[prev];
    int v1 = vertexList[curr];
    int v2 = vertexList[next];

    float ax = vertices[v0 * 2];
    float ay = vertices[v0 * 2 + 1];
    float bx = vertices[v1 * 2];
    float by = vertices[v1 * 2 + 1];
    float cx = vertices[v2 * 2];
    float cy = vertices[v2 * 2 + 1];

    // Check if triangle is convex (counter-clockwise)
    if (CrossProduct2D(ax, ay, bx, by, cx, cy) <= 0)
        return false;

    // Check if any other vertex is inside this triangle
    for (int i = 0; i < remainingVertices; i++)
    {
        if (i == prev || i == curr || i == next) continue;

        int vi = vertexList[i];
        float px = vertices[vi * 2];
        float py = vertices[vi * 2 + 1];

        if (IsPointInTriangle(px, py, ax, ay, bx, by, cx, cy))
            return false;
    }

    return true;
}

static int TriangulatePolygon(float* vertices, int vertexCount, unsigned short* indices)
{
    if (vertexCount < 3) return 0;

    int triangleCount = 0;
    int* vertexList = (int*)malloc(vertexCount * sizeof(int));

    // Initialize vertex list
    for (int i = 0; i < vertexCount; i++)
    {
        vertexList[i] = i;
    }

    int remainingVertices = vertexCount;
    int attempts = 0;
    int maxAttempts = remainingVertices * 2;

    while (remainingVertices > 2 && attempts < maxAttempts)
    {
        bool foundEar = false;

        for (int i = 0; i < remainingVertices; i++)
        {
            if (IsEar(vertices, vertexList, remainingVertices, i))
            {
                // Found an ear, create triangle
                int prev = (i - 1 + remainingVertices) % remainingVertices;
                int curr = i;
                int next = (i + 1) % remainingVertices;

                indices[triangleCount * 3] = vertexList[prev];
                indices[triangleCount * 3 + 1] = vertexList[curr];
                indices[triangleCount * 3 + 2] = vertexList[next];

                triangleCount++;

                // Remove the ear vertex
                for (int j = i; j < remainingVertices - 1; j++)
                {
                    vertexList[j] = vertexList[j + 1];
                }
                remainingVertices--;

                foundEar = true;
                attempts = 0;
                break;
            }
        }

        if (!foundEar)
        {
            attempts++;
            // If no ear found, try fallback fan triangulation
            if (attempts >= maxAttempts && remainingVertices > 2)
            {
                for (int i = 1; i < remainingVertices - 1; i++)
                {
                    indices[triangleCount * 3] = vertexList[0];
                    indices[triangleCount * 3 + 1] = vertexList[i];
                    indices[triangleCount * 3 + 2] = vertexList[i + 1];
                    triangleCount++;
                }
                break;
            }
        }
    }

    free(vertexList);
    return triangleCount;
}


    // Call once when map loads
    static void InitMapstateTexOld(void)
    {
        if (floorMeshes)
        {
            for (int i = 0; i < numFloorMeshes; i++)
            {
                if (floorMeshes[i].isValid)
                {
                    UnloadMesh(floorMeshes[i].mesh.rendermesh);
                    free(floorMeshes[i].mesh.vertices);
                    free(floorMeshes[i].mesh.texcoords);
                    free(floorMeshes[i].mesh.indices);
                }
            }
            free(floorMeshes);
        }

        numFloorMeshes = map->numsects;
        floorMeshes = (FloorMeshData*)calloc(numFloorMeshes, sizeof(FloorMeshData));

        for (int s = 0; s < map->numsects; s++)
        {
            sect_t* sect = &map->sect[s];
            FloorMeshData* floorData = &floorMeshes[s];

            floorData->isValid = false;

            if (sect->n >= 3 && sect->surf[1].tilnum >= 0 && sect->surf[1].tilnum < get_gnumtiles())
            {
                // Prepare vertices for triangulation
                float* polyVertices = (float*)malloc(sect->n * 2 * sizeof(float));

                for (int w = 0; w < sect->n; w++)
                {
                    polyVertices[w * 2] = sect->wall[w].x;
                    polyVertices[w * 2 + 1] = sect->wall[w].y;
                }

                // Create triangulated mesh
                TriangulatedMesh triMesh = {0};
                triMesh.vertexCount = sect->n;
                triMesh.triangleCount = sect->n - 2;

                triMesh.vertices = (float*)malloc(triMesh.vertexCount * 3 * sizeof(float));
                triMesh.texcoords = (float*)malloc(triMesh.vertexCount * 2 * sizeof(float));
                triMesh.indices = (unsigned short*)malloc(triMesh.triangleCount * 3 * sizeof(unsigned short));

                float z = sect->z[0];

                // Fill 3D vertices and UVs
                for (int w = 0; w < sect->n; w++)
                {
                    const wall_t* wall = &sect->wall[w];
                    triMesh.vertices[w * 3] = wall->x;
                    triMesh.vertices[w * 3 + 1] = z;
                    triMesh.vertices[w * 3 + 2] = wall->y;

                    triMesh.texcoords[w * 2] = 0.2f * wall->x * sect->surf->uv[1].x;
                    triMesh.texcoords[w * 2 + 1] = 0.2f * wall->y * sect->surf->uv[2].y;
                }

                // Triangulate polygon
                triMesh.triangleCount = TriangulatePolygon(polyVertices, sect->n, triMesh.indices);
                triMesh.isValid = true;

                // Convert to Raylib mesh
                Mesh floorMesh = {0};
                floorMesh.vertexCount = triMesh.vertexCount;
                floorMesh.triangleCount = triMesh.triangleCount;
                floorMesh.vertices = triMesh.vertices;
                floorMesh.texcoords = triMesh.texcoords;
                floorMesh.indices = triMesh.indices;

                UploadMesh(&floorMesh, false);

                floorData->mesh.rendermesh = floorMesh;
                floorData->mesh = triMesh;
                floorData->textureIndex = sect->surf[1].tilnum;
                floorData->isValid = true;

                free(polyVertices);
            }
        }
    }

    // Call every frame
    static void DrawMapstateTexOld(Camera3D cam)
    {
        rlDrawRenderBatchActive();
        rlDisableBackfaceCulling();
        // Draw pre-built floor meshes (UNLIT)
        for (int s = 0; s < numFloorMeshes; s++)
        {
            FloorMeshData* floorData = &floorMeshes[s];
            if (floorData->isValid)
            {
                const Texture2D floorTex = runtimeTextures[floorData->textureIndex];

                // Draw triangles from mesh data
                if (false)
                {
                    // Create material with unlit shader
                    Material mat = LoadMaterialDefault();

                    // Load basic unlit shader (or use default basic shader)
                    Shader unlitShader = LoadShader(0, 0); // Uses default vertex/fragment shaders
                    mat.shader = unlitShader;

                    SetMaterialTexture(&mat, MATERIAL_MAP_DIFFUSE, floorTex);

                    DrawMesh(floorData->mesh.rendermesh, mat, MatrixTranslate(0, 0, 0));

                    UnloadShader(unlitShader);
                    UnloadMaterial(mat);
                }
                else
                {
                    // rlCheckRenderBatchLimit(floorData->mesh.vertexCount * 12);
                    rlBegin(RL_TRIANGLES); // in triangels it resets texture hence reorder
                    rlSetTexture(floorTex.id);
                    rlDisableBackfaceCulling();
                    for (int tri = 0; tri < floorData->mesh.triangleCount; tri++)
                    {
                        // rlBegin(RL_TRIANGLES);  // without those 2 lines last vertex ofr each floor pinches at 0,0,0 with black color
                        // rlSetTexture(floorTex.id);    //
                        for (int vert = 0; vert < 3; vert++)
                        {
                            int idxa = floorData->mesh.indices[tri * 3 + vert];
                            rlColor4ub(255, 255, 255, 255);
                            rlTexCoord2f(
                                floorData->mesh.texcoords[idxa * 2],
                                floorData->mesh.texcoords[idxa * 2 + 1]
                            );
                            // rlNormal3f(0,1,0); // irrelevant
                            rlVertex3f(
                                floorData->mesh.vertices[idxa * 3], // xpos
                                floorData->mesh.vertices[idxa * 3 + 1], //ypos
                                floorData->mesh.vertices[idxa * 3 + 2] //zpos
                            );
                        }
                    }
                    rlDrawRenderBatchActive();
                    rlSetTexture(0);
                    rlEnd();
                }
            }
        }

        // Draw walls (unchanged - already efficient)
        for (int s = 0; s < map->numsects; s++)
        {
            sect_t* sect = &map->sect[s];

            for (int w = 0; w < sect->n; w++)
            {
                wall_t* wall = &sect->wall[w];
                wall_t* nextwall = &sect->wall[(w + 1) % sect->n];

                Vector3 bottomLeft = {wall->x, sect->z[0], wall->y};
                Vector3 bottomRight = {nextwall->x, sect->z[0], nextwall->y};
                Vector3 topLeft = {wall->x, sect->z[1], wall->y};
                Vector3 topRight = {nextwall->x, sect->z[1], nextwall->y};

                int texIndex = wall->surf.tilnum;
                if (wall->xsurf && wall->surfn > 1)
                    texIndex = wall->xsurf[0].tilnum;
                auto dx = sqrt(
                    (nextwall->x - wall->x) * (nextwall->x - wall->x) + (nextwall->y - wall->y) * (nextwall->y - wall->
                        y));
                auto dy = sect->z[0] - sect->z[1];

                if (wall->ns == -1) //dont draw red walls for now.
                    if (texIndex >= 0 && texIndex < get_gnumtiles())
                    {
                        Texture2D wallTex = runtimeTextures[texIndex];
                        rlSetTexture(wallTex.id);
                        rlBegin(RL_QUADS);
                        rlColor4ub(255, 255, 255, 255);
                        rlTexCoord2f(0.0f, 1.0f * wall->surf.uv[2].y * dy);
                        rlVertex3f(bottomLeft.x, bottomLeft.y, bottomLeft.z);
                        rlTexCoord2f(1.0f * wall->surf.uv[1].x * dx, 1.0f * wall->surf.uv[2].y * dy);
                        rlVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);
                        rlTexCoord2f(1.0f * wall->surf.uv[1].x * dx, 0.0f);
                        rlVertex3f(topRight.x, topRight.y, topRight.z);
                        rlTexCoord2f(0.0f, 0.0f);
                        rlVertex3f(topLeft.x, topLeft.y, topLeft.z);
                        rlEnd();
                        rlSetTexture(0);
                    }
            }
        }

        // Draw sprites (unchanged - already efficient)
        for (int i = 0; i < map->numspris; i++)
        {
            spri_t* spr = &map->spri[i];
            if (spr->tilnum >= 0 && spr->tilnum < gnumtiles_i)
            {
                Texture2D spriteTex = runtimeTextures[spr->tilnum];
                Vector3 rg = {spr->r.x, spr->r.y, spr->r.z};
                Vector3 dw = {spr->d.x, spr->d.y, spr->d.z};
                auto xs = Vector3Length(rg);
                auto ys = Vector3Length(dw);
                Vector3 pos = {spr->p.x - xs, spr->p.z - ys, spr->p.y};
                Rectangle source = {0.0f, 0.0f, (float)spriteTex.width, (float)spriteTex.height};
                xs *= 2;
                ys *= 2;

                DrawBillboardRec(cam, spriteTex, source, pos, {xs, ys}, WHITE);
            }
        }
    }

    // Call when map unloads
    static void CleanupMapstateTex(void)
    {
        if (floorMeshes)
        {
            for (int i = 0; i < numFloorMeshes; i++)
            {
                if (floorMeshes[i].isValid)
                {
                    UnloadMesh(floorMeshes[i].mesh.rendermesh);
                }
            }
            free(floorMeshes);
            floorMeshes = NULL;
            numFloorMeshes = 0;
        }
    }


    static void DrawMapstateLines()
    {
        // how to get texture indexes.
        // map->sect->surf[0].tilnum // index of texture in array runtimetextures[]
        // map->spri->tilnum
        // map->sect->wall->xsurf[0].tilnum
        // Draw sectors
        for (int s = 0; s < map->numsects; s++)
        {
            sect_t* sect = &map->sect[s];

            // Draw walls as lines
            for (int w = 0; w < sect->n; w++)
            {
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
        for (int i = 0; i < map->numspris; i++)
        {
            spri_t* spr = &map->spri[i];
            Vector3 pos = {spr->p.x, spr->p.z, spr->p.y};
            float s = 0.1f;
            DrawCubeWires(pos, s, s, s, DARKBLUE);
        }
    }

    static void TestRenderTextures() // all end up gobbled data
    {
        // Clear background
        //  BeginDrawing();
        //  ClearBackground(BLACK);

        // Test texture indices 1, 2, 3
        for (int i = 1; i <= 3; i++)
        {
            Texture2D tex = runtimeTextures[i];

            // Skip invalid textures
            if (tex.id == 0) continue;

            // Position quads side by side
            float x = 100.0f + (i - 1) * 150.0f;
            float y = 100.0f;
            float size = 100.0f;

            // Draw textured quad
            DrawTexture(tex, (int)x, (int)y, WHITE);

            // Draw texture info
            DrawText(TextFormat("Tex %d: %dx%d ID:%d", i, tex.width, tex.height, tex.id),
                     (int)x, (int)y + tex.height + 5, 10, WHITE);
        }

        //   EndDrawing();
    }

    static Texture2D ConvertPalToTexture()
    {
        Image palImage = {0};
        palImage.width = 16;
        palImage.height = 16;
        palImage.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        palImage.mipmaps = 1;
        palImage.data = malloc(16 * 16 * 4);

        auto* pixels = static_cast<unsigned char*>(palImage.data);

        // Debug the actual memory

        int i = 6;
        printf("Direct memory read: %d %d %d %d\n",
               getColor(i + 0), getColor(i + 1), getColor(i + 2), getColor(i + 3));

        for (int y = 0; y < 16; y++)
        {
            for (int x = 0; x < 16; x++)
            {
                int colorIndex = y * 16 + x;
                int pixelIndex = (y * 16 + x) * 4;

                // Direct memory access instead of array indexing
                pixels[pixelIndex + 0] = getColor(colorIndex)[2]; // R
                pixels[pixelIndex + 1] = getColor(colorIndex)[1]; // G
                pixels[pixelIndex + 2] = getColor(colorIndex)[0]; // B
                pixels[pixelIndex + 3] = 255; // A
            }
        }

        Texture2D texture = LoadTextureFromImage(palImage);
        UnloadImage(palImage);
        return texture;
    }

    // converts INDEXED pics only!
    static Texture2D ConvertPicToTexture(tile_t* tpic)
    {
        if (!tpic || !tpic->tt.f)
            return {0};

        tiltyp* pic = &tpic->tt;

        Image picImage = {0};
        int x = max(4, pic->x);
        int y = max(4, pic->y);
        picImage.width = x;
        picImage.height = y;
        picImage.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        picImage.mipmaps = 1;

        picImage.data = malloc(x * y * 4);
        auto* pixels = static_cast<unsigned char*>(picImage.data);

        // pic->f points to RGBA data, pic->p is stride in bytes
        for (int y = 0; y < pic->y; y++)
        {
            unsigned char* srcRow = (unsigned char*)(pic->f + y * pic->p);
            for (int x = 0; x < pic->x; x++)
            {
                int srcIndex = x * 4; // 4 bytes per pixel in source, even tho we need only byte 1 as index.
                // i guess Ken used it for rgba textures too, since build2 can do them.
                int dstIndex = (y * pic->x + x) * 4;

                // Source is already RGBA, just copy and potentially reorder
                // tried replacing with those, and for a split second it is orange, but then falls back to green and still debugs everywhere
                //    pixels[dstIndex + 0] = 255;
                //    pixels[dstIndex + 1] = 122;
                //    pixels[dstIndex + 2] = 44;

                pixels[dstIndex + 0] = srcRow[srcIndex + 2]; // R (from B)
                pixels[dstIndex + 1] = srcRow[srcIndex + 1]; // G
                pixels[dstIndex + 2] = srcRow[srcIndex + 0]; // B (from R)
                pixels[dstIndex + 3] = srcRow[srcIndex + 3]; // A
            }
        }

        Texture2D texture = LoadTextureFromImage(picImage);
        UnloadImage(picImage);
        return texture;
    }

    static void DrawPaletteAndTexture()
    {
        DrawPaletteAndTexture(runtimeTextures[6], runtimeTextures[10], 660, 660);
    }

    static void DrawPaletteAndTexture(Texture2D palTexture, Texture2D picTexture, int screenWidth, int screenHeight)
    {
        //  BeginDrawing();
        //  ClearBackground(DARKGRAY);

        // Draw palette in top-left corner
        if (palTexture.id > 0)
        {
            DrawTextureEx(palTexture, {10, 10}, 0.0f, 8.0f, WHITE);
            DrawText("PALETTE", 10, 150, 20, WHITE);
        }

        // Draw texture in center-right area
        if (picTexture.id > 0)
        {
            float scale = 2.0f;
            int maxSize = 400;

            // Scale texture to fit preview area
            if (picTexture.width > maxSize || picTexture.height > maxSize)
            {
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

private:
    static void GenerateTextures()
    {
        gnumtiles_i = get_gnumtiles();
        gmaltiles_i = get_gmaltiles();
        // static long gnumtiles, gmaltiles, gtilehashead[1024];
        // static *long get_gtilehashead() { return gtilehashead; } // in outer file
        long* source = get_gtilehashead();
        memcpy(gtilehashead_i, source, sizeof(long) * 1024);

        runtimeTextures = static_cast<Texture2D*>(malloc(sizeof(Texture2D) * gnumtiles_i));
        int end = gnumtiles_i;
        for (int i = 0; i < end; ++i)
        {
            runtimeTextures[i] = ConvertPicToTexture(getGtile(i)); // returns Texture2D
        }
        int a = 1;
    }

    static void LoadMapAndTiles()
    {
        map = static_cast<mapstate_t*>(malloc(sizeof(mapstate_t)));
        memset(map, 0, sizeof(mapstate_t));
        initcrc32();

        map->numsects = 0;
        map->malsects = 256;
        map->sect = static_cast<sect_t*>(malloc(map->malsects * sizeof(sect_t)));
        if (!map->sect) return;
        memset(map->sect, 0, map->malsects * sizeof(sect_t));

        map->numspris = 0;
        map->malspris = 256;
        map->spri = static_cast<spri_t*>(malloc(map->malspris * sizeof(spri_t)));
        if (!map->spri) return;
        memset(map->spri, 0, map->malspris * sizeof(spri_t));
        map->blankheadspri = -1;

        map->blankheadspri = -1;
        for (int i = 0; i < map->malspris; i++)
        {
            map->spri[i].sectn = map->blankheadspri;
            map->spri[i].sectp = -1;
            map->spri[i].sect = -1;
            if (map->blankheadspri >= 0) map->spri[map->blankheadspri].sectp = i;
            map->blankheadspri = i;
        }
        loadmap_imp((char*)"c:/Eugene/Games/build2/E2l4.MAP", map);
    }
};


#endif //RAYLIB_LUA_IMGUI_DUMBRENDER_H
