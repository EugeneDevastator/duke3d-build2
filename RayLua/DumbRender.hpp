//
// Created by omnis on 10/27/2025.
//

#ifndef RAYLIB_LUA_IMGUI_DUMBRENDER_H
#define RAYLIB_LUA_IMGUI_DUMBRENDER_H

extern "C" {
#include "loaders.h"
#include "mapcore.h"
}


#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

typedef struct
{
    Mesh mesh;
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


    // Call once when map loads
    static void InitMapstateTex(void)
    {
        if (floorMeshes)
        {
            // Cleanup existing meshes
            for (int i = 0; i < numFloorMeshes; i++)
            {
                if (floorMeshes[i].isValid)
                {
                    UnloadMesh(floorMeshes[i].mesh);
                }
            }
            free(floorMeshes);
        }

        numFloorMeshes = map->numsects;
        floorMeshes = (FloorMeshData*)calloc(numFloorMeshes, sizeof(FloorMeshData));

        // Pre-build all floor meshes
        for (int s = 0; s < map->numsects; s++)
        {
            sect_t* sect = &map->sect[s];
            FloorMeshData* floorData = &floorMeshes[s];

            floorData->isValid = false;

            if (sect->n >= 3 && sect->surf[1].tilnum >= 0 && sect->surf[1].tilnum < get_gnumtiles())
            {
                Mesh floorMesh = {0};
                floorMesh.vertexCount = sect->n;
                floorMesh.triangleCount = sect->n - 2;

                floorMesh.vertices = static_cast<float*>(malloc(floorMesh.vertexCount * 3 * sizeof(float)));
                floorMesh.texcoords = static_cast<float*>(malloc(floorMesh.vertexCount * 2 * sizeof(float)));
                floorMesh.indices = static_cast<unsigned short*>(malloc(floorMesh.triangleCount * 3 * sizeof(unsigned short)));
                float minX = FLT_MAX, maxX = -FLT_MAX;
                float minY = FLT_MAX, maxY = -FLT_MAX;

                for (int w = 0; w < sect->n; w++)
                {
                    wall_t* wall = &sect->wall[w];
                    if (wall->x < minX) minX = wall->x;
                    if (wall->x > maxX) maxX = wall->x;
                    if (wall->y < minY) minY = wall->y;
                    if (wall->y > maxY) maxY = wall->y;
                }

                float rangeX = maxX - minX;
                float rangeY = maxY - minY;
                // Fill vertices and UVs
                for (int w = 0; w < sect->n; w++)
                {
                    wall_t* wall = &sect->wall[w];
                    floorMesh.vertices[w * 3] = wall->x;
                    floorMesh.vertices[w * 3 + 1] = (float) sect->z[1];
                    floorMesh.vertices[w * 3 + 2] = wall->y;
                    floorMesh.texcoords[w * 2] = 0.2f * wall->x;
                    floorMesh.texcoords[w * 2 + 1] = 0.2f * wall->y;
                   // floorMesh.texcoords[w * 2] = ((int)wall->x % 100)/100.0f;
                   // floorMesh.texcoords[w * 2 + 1] = ((int)wall->y % 100)/100.0f;
                }

                // Fan triangulation
                for (int t = 0; t < floorMesh.triangleCount; t++)
                {
                    floorMesh.indices[t * 3] = 0;
                    floorMesh.indices[t * 3 + 1] = t + 1;
                    floorMesh.indices[t * 3 + 2] = t + 2;
                }

                UploadMesh(&floorMesh, false);

                floorData->mesh = floorMesh;
                floorData->textureIndex = sect->surf[1].tilnum;
                floorData->isValid = true;
            }
        }
    }

    // Call every frame
    static void DrawMapstateTex(Camera3D cam)
    {
        // Draw pre-built floor meshes (UNLIT)
        for (int s = 0; s < numFloorMeshes; s++)
        {
            FloorMeshData* floorData = &floorMeshes[s];
            if (floorData->isValid)
            {
                Texture2D floorTex = runtimeTextures[floorData->textureIndex];

                // Draw triangles from mesh data
                if (false)
                {
                    // Create material with unlit shader
                    Material mat = LoadMaterialDefault();

                    // Load basic unlit shader (or use default basic shader)
                    Shader unlitShader = LoadShader(0, 0); // Uses default vertex/fragment shaders
                    mat.shader = unlitShader;

                    SetMaterialTexture(&mat, MATERIAL_MAP_DIFFUSE, floorTex);

                    DrawMesh(floorData->mesh, mat, MatrixTranslate(0, 0, 0));

                    UnloadShader(unlitShader);
                    UnloadMaterial(mat);
                }
                else
                {
                    rlBegin(RL_TRIANGLES); // in triangels it resets texture hence reorder
                    rlSetTexture(floorTex.id);
                    rlColor4ub(255, 255, 255, 255);
                    for (int t = 0; t < floorData->mesh.triangleCount; t++)
                    {
                        for (int v = 0; v < 3; v++)
                        {
                            int idx = floorData->mesh.indices[t * 3 + v];
                            rlTexCoord2f(
                                floorData->mesh.texcoords[idx * 2],
                                floorData->mesh.texcoords[idx * 2 + 1]
                            );
                            rlVertex3f(
                                floorData->mesh.vertices[idx * 3],
                                floorData->mesh.vertices[idx * 3 + 1],
                                floorData->mesh.vertices[idx * 3 + 2]
                            );
                        }
                    }

                    rlEnd();
                    rlSetTexture(0);
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

                if (texIndex >= 0 && texIndex < get_gnumtiles())
                {
                    Texture2D wallTex = runtimeTextures[texIndex];

                    rlSetTexture(wallTex.id);
                    rlBegin(RL_QUADS);
                    rlColor4ub(255, 255, 255, 255);
                    rlTexCoord2f(0.0f, 1.0f);
                    rlVertex3f(bottomLeft.x, bottomLeft.y, bottomLeft.z);
                    rlTexCoord2f(1.0f, 1.0f);
                    rlVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);
                    rlTexCoord2f(1.0f, 0.0f);
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
                Vector3 pos = {spr->p.x, spr->p.z, spr->p.y};
                DrawBillboard(cam, spriteTex, pos, 1.0f, WHITE);
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
                    UnloadMesh(floorMeshes[i].mesh);
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
                pixels[dstIndex + 3] = 255; // A
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
            runtimeTextures[i] = ConvertPicToTexture(getGtile(i));
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
        loadmap_imp((char*)"c:/Eugene/Games/build2/E2L7.MAP", map);
    }
};


#endif //RAYLIB_LUA_IMGUI_DUMBRENDER_H
