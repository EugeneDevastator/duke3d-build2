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
#include "DumbCore.hpp"
#include "cmake-build-custom/_deps/raylib-src/src/external/glad.h"


 extern "C" {
#include "loaders.h"
#include "mapcore.h"
#include "monoclip.h"
#include "shadowtest2.h"
#include "buildmath.h"
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
static bool drawWalls = false;
static bool drawSpris = true;
static bool drawCeils = false;
static player_transform plr;
static Shader uvShader ;
static Shader lightShader ;
static int lightPosLoc;
static int lightRangeLoc;
static bool syncam = true;
static int cureyepoly = 0;
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
        uvShader = LoadShader("uv_vis_shader.vs", "uv_vis_shader.fs");

        lightShader = LoadShader("light.vs", "light.fs");

        lightPosLoc = GetShaderLocation(lightShader, "lightPosition");
        lightRangeLoc = GetShaderLocation(lightShader, "lightRange");


        strcpy_s(rootpath, "c:/Eugene/Games/build2/");
        LoadPal(rootpath);
        LoadMapAndTiles();
        shadowtest2_numlights=0;
        //init lights
        for(int i=0;i<map->numspris;i++)
        {
            map->spri[i].owner = -1;

            //Insert lights
            if (map->spri[i].tilnum == 126)
            {
                map->spri[i].flags |= SPRITE_B2_IS_LIGHT;
                if (map->light_sprinum < MAXLIGHTS)
                    map->light_spri[map->light_sprinum++] = i;
            }
        }
// init portals; walls test
        for (int i = 0; i < map->numsects; i++) { // wall ports
            for (int wn = 0; wn < map->sect[i].n; wn++) {
                map->sect[i].wall[wn].tags[1] = -1;
                if (map->sect[i].wall[wn].surf.pal == 30) {
                    portal &p = portals[portaln];
                    p.id = map->sect[i].wall[wn].surf.lotag;
                    p.sect = i;
                    p.anchorspri = map->sect[i].headspri;
                    p.surfid = wn;
                    p.kind = PORT_WALL;

                    map->sect[i].wall[wn].tags[1] = portaln;
                    p.destpn = map->sect[i].wall[wn].surf.hitag;
                    portaln++;
                }
            }
        }

        for (int i = 0; i < map->numsects; i++) { // floor ceil ports
            map->sect[i].tags[1] = -1;
            if (map->sect[i].surf[1].pal == 30 || map->sect[i].surf[0].pal == 30) {
                portal &p = portals[portaln];
                p.id = map->sect[i].surf[1].lotag;
                p.sect = i;
                p.anchorspri = map->sect[i].headspri;

                //p.surfid = map->sect[i].surf[1].lotag; // hak to determine ceil or floor in map lotag1==floor.
                int sid1 = abs(map->spri[p.anchorspri].p.z - map->sect[i].z[1]);
                int sid2 = abs(map->spri[p.anchorspri].p.z - map->sect[i].z[0]);
                p.surfid = sid1 < sid2;
                map->spri[p.anchorspri].p.z = map->sect[i].z[p.surfid]+1; // resolve flor ceil in future
                p.kind = p.surfid;
                spri_t *spr = &map->spri[p.anchorspri];
              //  point3d newr = spr->tr.r;
                point3d newd = spr->tr.r;
                point3d newr = spr->tr.d;
                vmulscal(&newd,-1.0f);
                spr->tr.d = newd;
                spr->tr.r = newr;
              //  vmulscal(&spr->tr.f,-1.0f);
                normalize_transform(&spr->tr);
                p.destpn = map->sect[i].surf[1].hitag;
                map->sect[i].tags[1] = portaln;
                portaln++;
            }
        }

        for (int i = 0; i < portaln; i++) { // portal post pass
            int target_tag = portals[i].destpn; // currently stores expected hitag
            portals[i].destpn = -1; // mark as unresolved
            spri_t *spr = &map->spri[portals[i].anchorspri];
            normalize_transform(&spr->tr);

            // Find portal with matching lowtag
            for (int j = 0; j < portaln; j++) {
                if (i == j) continue; // skip self, disable for mirror
                int id = portals[j].id;
                if (id == target_tag) {
                    portals[i].destpn = j;
                    break;
                }
            }

            // Optional: warn about unresolved portals
            if (portals[i].destpn == -1) {
                // Portal destination not found
                printf("Warning: Portal %d with target lotag %d has no matching hitag\n", i, target_tag);
            }
        }
        // auto paltex = ConvertPalToTexture();
        // tile_t* pic = static_cast<tile_t*>(malloc(sizeof(tile_t)));
        // strcpy_s(pic->filnam, "TILES000.art|1");
        // auto tex = ConvertPicToTexture(pic);
    }
    static void LoadTexturesToGPU() {
        GenerateTextures();
        InitMapstateTex();
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
        plr.ipos = map->startpos;
        plr.ifor = map->startfor;
        plr.irig = map->startrig;
        plr.idow = map->startdow;
        plr.cursect = map->startsectn;

        plr.grdc.x = 0; plr.grdc.y = 0; plr.grdc.z = 0; //center
        plr.grdu.x = 1; plr.grdu.y = 0; plr.grdu.z = 0;
        plr.grdv.x = 0; plr.grdv.y = 1; plr.grdv.z = 0;
        plr.grdn.x = 0; plr.grdn.y = 0; plr.grdn.z = 1; //normal

        plr.ghx = 800/2; //NOTE: Do not replace with variables - static init needed for sync
        plr.ghy = 600/2;
        plr.ghz = plr.ghx;
        plr.zoom = plr.ozoom = 1.f;
        shadowtest2_init();

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

        if (false)
        for (int s = 0; s < map->numsects; s++)
        {
            sect_t* sect = &map->sect[s];

            // Generate floor and ceiling meshes
            for (int isFloor = 0; isFloor < 2; isFloor++)
            {
               // if (!drawCeils && (isFloor == 0))
               //     continue;
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
                        triMesh.texcoords[w * 2] = wall->x * sect->surf[isFloor].uv[1].x + wall->y * sect->surf[isFloor]
                            .uv[2].x + sect->surf[isFloor].uv[0].x;
                        triMesh.texcoords[w * 2 + 1] = wall->x * sect->surf[isFloor].uv[1].y + wall->y * sect->surf[
                            isFloor].uv[2].y + sect->surf[isFloor].uv[0].y;
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
    static void DrawEyePoly(float sw, float sh, player_transform *playr, cam_t *cam) {
        dpoint3d dp, dr, dd, df;
        long i, j, k, l, m, flashlight1st;
        dp.x = 0.0;
        dp.y = 0.0;
        dp.z = 0.0;
        dr.x = 1.0;
        dr.y = 0.0;
        dr.z = 0.0;
        dd.x = 0.0;
        dd.y = 1.0;
        dd.z = 0.0;
        df.x = 0.0;
        df.y = 0.0;
        df.z = 1.0;
        cam->h.x = playr->ghx;
        cam->h.y = playr->ghy;
        cam->h.z = playr->ghz;


        // cam.c = cc->c; cam.z = cc->z;

        cam->c.x = sw;
        cam->c.y = sh;
        cam->z.x = sw;
        cam->z.y = sh;

        if (false)//((!useLights) || (gdps->cursect < 0))
        {
            cam->r.x = 1.f; cam->r.y = 0.f; cam->r.z = 0.f;
            cam->d.x = 0.f; cam->d.y = 1.f; cam->d.z = 0.f;
            cam->f.x = 0.f; cam->f.y = 0.f; cam->f.z = 1.f;
            cam->p.x = 0.f; cam->p.y = 0.f; cam->p.z = 0.f;
            //	drawkv6_numlights = -1;
            //	drawview(&cam,gdps,0);
        }
        else {
            cam->r.x = playr->irig.x; cam->r.y = playr->irig.y; cam->r.z = playr->irig.z;
            cam->d.x = playr->idow.x; cam->d.y = playr->idow.y; cam->d.z = playr->idow.z;
            cam->f.x = playr->ifor.x; cam->f.y = playr->ifor.y; cam->f.z = playr->ifor.z;
            cam->p.x = playr->ipos.x; cam->p.y = playr->ipos.y; cam->p.z = playr->ipos.z;
            cam->cursect = playr->cursect;
        }

 // Main render scope
			shadowtest2_useshadows = 1;//b2opts.shadows;
			shadowtest2_numlights = 0;
			for(i=map->light_sprinum-1;i>=0;i--)
			{
				if (((unsigned)map->light_spri[i] < (unsigned)map->malspris)
				    && (map->spri[map->light_spri[i]].sect >= 0)
				    && (shadowtest2_numlights < MAXLIGHTS))
				{
					shadowtest2_light[shadowtest2_numlights].sect   = map->spri[map->light_spri[i]].sect;
					shadowtest2_light[shadowtest2_numlights].p      = map->spri[map->light_spri[i]].p;
				    shadowtest2_light[shadowtest2_numlights].p.x += sin(GetTime()+shadowtest2_light[shadowtest2_numlights].p.y)*3;
					k = ((map->spri[map->light_spri[i]].flags>>17)&7);
					if (!k) { shadowtest2_light[shadowtest2_numlights].spotwid = -1.0; }
					else
					{
						m = ((map->spri[map->light_spri[i]].flags>>20)&1023); if (!m) continue;
						shadowtest2_light[shadowtest2_numlights].spotwid = cos(m*PI/1024.0); //FIX:use lut
						switch(k)
						{
							case 1: case 2: shadowtest2_light[shadowtest2_numlights].f = map->spri[map->light_spri[i]].d; break;
							case 3: case 4: shadowtest2_light[shadowtest2_numlights].f = map->spri[map->light_spri[i]].f; break;
							case 5: case 6: shadowtest2_light[shadowtest2_numlights].f = map->spri[map->light_spri[i]].r; break;
						}
						if (!(k&1)) { shadowtest2_light[shadowtest2_numlights].f.x *= -1; shadowtest2_light[shadowtest2_numlights].f.y *= -1; shadowtest2_light[shadowtest2_numlights].f.z *= -1; }
					}
					shadowtest2_light[shadowtest2_numlights].rgb[0] = map->spri[map->light_spri[i]].bsc/8192.f; //gsc/8192   map->spri[map->light_spri[i]].fat;
					shadowtest2_light[shadowtest2_numlights].rgb[1] = map->spri[map->light_spri[i]].gsc/8192.f;
					shadowtest2_light[shadowtest2_numlights].rgb[2] = map->spri[map->light_spri[i]].rsc/8192.f;
					shadowtest2_light[shadowtest2_numlights].flags  = 1;
					shadowtest2_numlights++;
				}
			}

        //---
        shadowtest2_rendmode = 2;
        reset_context();
        draw_hsr_polymost(cam, map,0);
        shadowtest2_rendmode = 4;

       // shadowtest2_numlights =1;
       // if (shadowtest2_updatelighting) //FIXFIX
        {
            cam_t ncam;
            ncam = *cam;
           // shadowtest2_updatelighting = 0; //FIXFIX
            shadowtest2_ligpolreset(-1);
            for(glignum=0;glignum<shadowtest2_numlights;glignum++)
            {
                ncam.p = shadowtest2_light[glignum].p;
                reset_context();
                ncam.cursect = shadowtest2_light[glignum].sect;
                draw_hsr_polymost(&ncam,map,0);
            }
        }
        shadowtest2_setcam(cam);
        cam->p = playr->ipos; cam->r = playr->irig; cam->d = playr->idow; cam->f = playr->ifor;
        cam->h.x = playr->ghx; cam->h.y = playr->ghy; cam->h.z = playr->ghz;
    }
    static void DrawTriangleFan3D(const Vector3 *points, int pointCount, Color color)
    {
        if (pointCount >= 3)
        {
            //rlSetTexture(GetShapesTexture().id);
            rlBegin(RL_QUADS);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 1; i < pointCount - 1; i++)
            {
               rlVertex3f(points[0].x, points[0].y,points[0].z);
               rlVertex3f(points[i].x, points[i].y, points[i].z);
                rlVertex3f(points[i + 1].x, points[i + 1].y, points[i + 1].z);
                rlVertex3f(points[i + 1].x, points[i + 1].y, points[i + 1].z);
            }
            rlEnd();
            //rlSetTexture(0);
        }
    }
    // Monotone polygon triangulation - O(n), no allocations needed
    static bool draw_eyepol_wspace(float sw, float sh, int i, int &v0, int &vertCount) {
        v0 = eyepol[i].vert0;
        int v1 = eyepol[i + 1].vert0;
        vertCount = v1 - v0;
        if (vertCount < 3) return true;
       // if (!eyepol[i].has_triangulation) return true;

        rlDrawRenderBatchActive();
        rlDisableBackfaceCulling();

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-0.5f, 1.0f);
        rlDisableDepthMask();

        Vector3* pts = static_cast<Vector3 *>(malloc(vertCount * sizeof(Vector3)));

        for (int j = 0; j < vertCount; j++) {
            point3d p = eyepolv[v0 + j];
            pts[j] = {p.x, -p.z, p.y};
        }
        unsigned char r = ((i)/5.0f)*255;
        rlColor4ub(r,128,3,40);
        //DrawTriangleFan3D(pts,vertCount,{r,128,3,30});
        TriangAndDraw3D(pts,vertCount);
        rlDrawRenderBatchActive();


        free(pts);

        glDisable(GL_POLYGON_OFFSET_FILL);
        return false;
    }


    static void DrawPost3d(float sw, float sh, Camera3D camsrc) {
        // Vector2 v1 = {0, 0};
        // Vector2 v2 = {sw, sh};
        // Vector2 v3 = {sw / 2, sh};
        Color transparentWhite = {255, 255, 255, 128};
        ClearBackground({50,50,60,255});  // Set your desired color

        cam_t b2cam;
        if (syncam) {
            plr.ipos = {camsrc.position.x, camsrc.position.z, -camsrc.position.y};
            updatesect_imp(plr.ipos.x,plr.ipos.y,plr.ipos.z, &plr.cursect, map);

            Vector3 forward = Vector3Normalize(Vector3Subtract(camsrc.target, camsrc.position));
            Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camsrc.up));
            Vector3 up = Vector3CrossProduct(right, forward); // Recalculate orthogonal up

            // Convert to Build2 coordinate system (x->x, y->z, z->-y)
            plr.ifor.x = forward.x;  plr.ifor.y = forward.z;  plr.ifor.z = -forward.y;
            plr.irig.x = right.x;    plr.irig.y = right.z;    plr.irig.z = -right.y;
            plr.idow.x = -up.x;       plr.idow.y = -up.z;       plr.idow.z = up.y;


            b2cam.p = plr.ipos;
            b2cam.f = plr.ifor;
            b2cam.r = plr.irig;
            b2cam.d = plr.idow;
        }

        if (IsKeyPressed(KEY_U))
            syncam = !syncam;

        DrawEyePoly(sw, sh, &plr, &b2cam); // ken render

        // Eyepol polys
        bool draweye = true;
        bool drawlines = true;
        bool drawlights = false;

        rlDisableBackfaceCulling();
        BeginMode3D(camsrc);


        if (draweye && (!(!eyepol || !eyepolv || eyepoln <= 0)))
            {
            for (int i = 0; i < eyepoln; i++) {
                // eypolis
                int v0;
                int vertCount;
                BeginBlendMode(BLEND_ADDITIVE);
                if (IsKeyPressed(KEY_J)) {
                    cureyepoly++;
                    if (cureyepoly > eyepoln)
                        cureyepoly = 0;
                }
                if (cureyepoly == 0 | cureyepoly == i) {
                    if (draw_eyepol_wspace(sw, sh, i, v0, vertCount))
                        continue;
                } else continue;

                if (drawlines)
                {
                    rlBegin(RL_LINES);
                    rlDisableDepthMask();
                    rlDisableBackfaceCulling();
                    glEnable(GL_POLYGON_OFFSET_FILL);
                    glPolygonOffset(-1.0f, 1.0f);
                    rlColor4f(0, 1, 1, 1);
                    for (int j = 0; j < vertCount; j++) {
                        rlColor4f(1, i/10.0f, 0, 1);
                       //int idx[] = {v0, v0 + j, v0 + j + 1};
                       //for (int k = 0; k < 3; k++) {
                          //  Vector3 pt = {eyepolv[idx[k]].x, eyepolv[idx[k]].y,eyepolv[idx[k]].z};
                            Vector3 pt = {eyepolv[v0+j].x, eyepolv[v0+j].y,eyepolv[v0+j].z};
                            rlVertex3f(pt.x,-pt.z, pt.y);
                       // }
                    }
                    Vector3 pt = {eyepolv[v0].x, eyepolv[v0].y,eyepolv[v0].z};
                    rlVertex3f(pt.x,-pt.z, pt.y);

                    rlEnd();
                    glDisable(GL_POLYGON_OFFSET_FILL);
                    rlEnableDepthMask();
                } // eyepol lines for each poly
                EndBlendMode();
            }
        }
        EndMode3D();
        //if (!lightpos_t || !eyepolv || eyepoln <= 0) return;}

        if (drawlights) { // Light polys

            BeginMode3D(camsrc);
            rlDisableBackfaceCulling();
            rlDisableDepthMask();
            BeginBlendMode(BLEND_ADDITIVE);

            BeginShaderMode(lightShader);

            int lightRange = 10;

            for(int lightIndex = 0; lightIndex < shadowtest2_numlights; lightIndex++) {
                lightpos_t* lght = &shadowtest2_light[lightIndex];
                Vector3 lightpos = {lght->p.x,-lght->p.z,lght->p.y};
                SetShaderValue(lightShader, lightPosLoc, &lightpos, SHADER_UNIFORM_VEC3);
                SetShaderValue(lightShader, lightRangeLoc, &lightRange, SHADER_UNIFORM_FLOAT);

                for (int i = 0; i < lght->ligpoln; i++) {
                    int v0 = lght->ligpol[i].vert0;
                    int v1 = lght->ligpol[i + 1].vert0;
                    int vertCount = v1 - v0;
                    if (vertCount < 3) continue;

                    //   BeginShaderMode(uvShader);
                    rlBegin(RL_TRIANGLES);

                    //rlSetTexture(0);
                    for (int j = 1; j < vertCount - 1; j++) {
                        int idx[] = {v0, v0 + j, v0 + j + 1};
                        for (int k = 0; k < 3; k++) {
                            Vector3 ptb2 = {lght->ligpolv[idx[k]].x, lght->ligpolv[idx[k]].y, lght->ligpolv[idx[k]].z};
                            Vector3 pt = {ptb2.x,-ptb2.z, ptb2.y};

                            rlColor4f(lightIndex, (1-lightIndex), 0, 1);
                            rlNormal3f(0,1,0);
                            rlTexCoord2f(0,0.5);
                            rlVertex3f(pt.x, pt.y, pt.z);
                        }
                    }
                   rlDrawRenderBatchActive();


                    rlEnd();

                }
            }
            EndShaderMode();
            EndBlendMode();
            glDisable(GL_POLYGON_OFFSET_FILL);
            rlEnableDepthMask();
            EndMode3D();
        }
    }



    // Updated wall rendering with segments
    static void DrawMapstateTex(Camera3D rlcam)
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
        if (drawWalls)
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
                if (wall->ns == -1 || wall->ns >= map->malsects)
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
                    int lowtile, masktile, hitile = wall->surf.tilnum;
                    if (wall->surfn == 3)
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
                            float dh = nextTopLeftZ - nextTopRightZ;
                            Texture2D upperTex = runtimeTextures[hitile];
                            rlSetTexture(upperTex.id);
                            rlBegin(RL_QUADS);
                            rlColor4ub(255, 255, 255, 255);

                            rlTexCoord2f(0.0f, 1.0f * wall->surf.uv[2].y * (upperDy + selfDy));
                            rlVertex3f(bottom_left.x, bottom_left.y, bottom_left.z);

                            rlTexCoord2f(wall->surf.uv[1].x * dx, wall->surf.uv[2].y * (upperDy + dh));
                            rlVertex3f(bottom_right.x, bottom_right.y, bottom_right.z);

                            rlTexCoord2f(1.0f * wall->surf.uv[1].x * dx, 0.0f);
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
                            float dh = nextBottomLeftZ - nextBottomRightZ;
                            Texture2D lowerTex = runtimeTextures[lowtile];

                            rlSetTexture(lowerTex.id);
                            rlBegin(RL_QUADS);

                            rlColor4ub(255, 255, 255, 255);
                            // CW starting from upper left
                            rlTexCoord2f(0.0f, 1.0f * wall->surf.uv[2].y * (largeDy + selfDy));
                            rlVertex3f(thisTopLeft.x, thisTopLeft.y, thisTopLeft.z);

                            rlTexCoord2f(wall->surf.uv[1].x * dx, wall->surf.uv[2].y * (largeDy + dh));
                            rlVertex3f(thisTopRight.x, thisTopRight.y, thisTopRight.z);

                            rlTexCoord2f(1.0f * wall->surf.uv[1].x * dx, 0.0f);
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
        //rlEnableBackfaceCulling();
        // Draw sprites (unchanged)
        for (int i = 0; i < map->numspris; i++)
        {
            spri_t* spr = &map->spri[i];
            if (spr->tilnum >= 0 ) // sprites
            {
                if (spr->tilnum >= gnumtiles_i)
                    spr->tilnum = gnumtiles_i - 10;

                Texture2D spriteTex = runtimeTextures[spr->tilnum];
                Vector3 rg = {spr->r.x, -spr->r.z, spr->r.y};
                Vector3 dw = {spr->d.x, -spr->d.z, spr->d.y};
                Vector3 frw = {spr->f.x, -spr->f.z, spr->f.y};
                Vector3 pos = {spr->p.x, -spr->p.z, spr->p.y};
                pos += frw * 0.1; // bias agains fighting
                Vector3 a = pos + rg + dw;
                Vector3 b = pos + rg - dw;
                Vector3 c = pos - rg - dw;
                Vector3 d = pos - rg + dw;
                // Debug vectors
                DrawLine3D(pos, Vector3Add(pos, frw), BLUE); // Forward vector
                DrawLine3D(pos, Vector3Add(pos, rg), RED); // Right vector
                DrawLine3D(pos, Vector3Add(pos, dw), GREEN); // Down vector

                if (spr->flags & 32) // spr->flags |= SPRITE_B2_FLAT_POLY;
                {
                    rlDisableDepthMask();
                    rlSetTexture(spriteTex.id);
                    rlBegin(RL_QUADS);
                    rlColor4ub(255, 255, 255, 255); // todo update transp.

                    rlTexCoord2f(0.0f, 1.0f);
                    rlVertex3V(b);
                    rlTexCoord2f(1.0f, 1.0f);
                    rlVertex3V(c);
                    rlTexCoord2f(1.0f, 0.0f);
                    rlVertex3V(d);
                    rlTexCoord2f(0.0f, 0.0f);
                    rlVertex3V(a);

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
                    Vector3 pos = {spr->p.x, -spr->p.z, spr->p.y};
                    Rectangle source = {0.0f, 0.0f, (float)spriteTex.width, (float)spriteTex.height};
                    xs *= 2;
                    ys *= 2;

                    DrawBillboardRec(rlcam, spriteTex, source, pos, {xs * xflip, ys * yflip}, WHITE);
                }
            }
        }
        DrawTransform(&lastcamtr);
        DrawTransform(&lastcamtr2);
        dpoint3d testp = {lastcamtr2.p.x + lastcamtr2.d.x,lastcamtr2.p.y+ lastcamtr2.d.y,lastcamtr2.p.z+ lastcamtr2.d.z};

      // wccw_transform(&testp,&lastcamtr,&orcamm_tr);
      // DrawB2Point(&testp);
    }
    static void DrawTransform(transform *tr) {
        Vector3 rg = {tr->r.x, -tr->r.z, tr->r.y};
        Vector3 dw = {tr->d.x, -tr->d.z, tr->d.y};
        Vector3 frw = {tr->f.x, -tr->f.z, tr->f.y};
        Vector3 pos = {tr->p.x, -tr->p.z, tr->p.y};
        DrawLine3D(pos, Vector3Add(pos, frw), BLUE); // Forward vector
        DrawLine3D(pos, Vector3Add(pos, rg), RED); // Right vector
        DrawLine3D(pos, Vector3Add(pos, dw), GREEN); // Down vector
    }
    static void DrawB2Point(dpoint3d *pt) {
        Vector3 vecpt = {(float)pt->x,(float)pt->z*-1,(float)pt->y};
        DrawPoint3D(vecpt, {255,255,255,255});
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
    static void TriangAndDraw3D(Vector3 *points, int count)
    {
        if (count < 3 || !points) return;

        // Convert Vector3 points to 2D for triangulation
        float* polyVertices = (float*)malloc(count * 2 * sizeof(float));
        for (int i = 0; i < count; i++)
        {
            polyVertices[i * 2] = points[i].x;
            polyVertices[i * 2 + 1] = points[i].z; // Use Z as Y for 2D triangulation
        }

        // Prepare triangulation output
        int maxTriangles = count - 2;
        unsigned short* indices = (unsigned short*)malloc(maxTriangles * 3 * sizeof(unsigned short));

        // Triangulate the polygon
        int triangleCount = DumbRender::TriangulatePolygon(polyVertices, count, indices);

        // Draw triangulated mesh
        rlBegin(RL_TRIANGLES);


        for (int tri = 0; tri < triangleCount; tri++)
        {
            for (int vert = 0; vert < 3; vert++)
            {
                int idx = indices[tri * 3 + vert];
                Vector3 pt = points[idx];
                rlVertex3f(pt.x, pt.y, pt.z);
            }
        }

        rlEnd();

        // Cleanup
        free(polyVertices);
        free(indices);
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
        loadmap_imp((char*)"c:/Eugene/Games/build2/prt3.MAP", map);
    }
};

#endif //RAYLIB_LUA_IMGUI_DUMBRENDER_H
