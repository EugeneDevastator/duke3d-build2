#ifndef SHADOWTEST2_H
#define SHADOWTEST2_H

#include "scenerender.h"
#include "monoclip.h"

// ================================================================================================
// CONSTANTS AND CONFIGURATION
// ================================================================================================

#define LIGHTMAX 256                    // Maximum number of light sources
#define LIGHASHSIZ 1024                 // Hash table size for light polygon matching
#define BFINTMAX 256                    // Maximum bunch front intersections
#define MAXVERTS 256                    // Maximum vertices per sector connection
#define LRASTMAX 8192                   // Maximum light raster entries
#define LFLATSTEPSIZ 3
#define FLATSTEPSIZ (1<<LFLATSTEPSIZ)
#define SCISDIST .001
extern int shadowtest2_numlights, shadowtest2_useshadows, shadowtest2_numcpu;
extern int shadowtest2_rendmode, eyepoln, glignum;
extern unsigned int *shadowtest2_sectgot;
// ================================================================================================
// LIGHT SYSTEM DATA STRUCTURES
// ================================================================================================

/** Light polygon data for shadow casting */
typedef struct
{
    int vert0, b2sect, b2wall, b2slab, b2hashn;
    triangle_strip_t tri_strip;
    bool has_triangulation;
} ligpol_t;
/** Light source definition and shadow polygon storage */
typedef struct {
    point3d p, f;                       // Position and forward direction vector
    int sect, sprilink;                 // Current sector and sprite link index
    float rgb[3], spotwid;              // RGB intensity and spotlight width (-1=omnidirectional)
    int flags;                          // Bit flags: &1=use shadows, &(1<<31)=moved this frame

    // Sector visibility tracking for this light
    unsigned int *sectgot, *sectgotmal; // Bit array of sectors illuminated by this light
    int sectgotn;                       // Number of sectors in bit array

    // Shadow polygon hash table and storage
    int *lighashead;                    // Hash table heads for polygon matching
    int lighasheadn;                    // Size of hash table
    ligpol_t *ligpol;                   // Light polygon match info array
    int ligpoln, ligpolmal;             // Count and allocated size of ligpol array
    point3d *ligpolv;                   // Vertices for light polygon geometry
    int ligpolvn, ligpolvmal;           // Count and allocated size of ligpolv array
} lightpos_t;


// ================================================================================================
// SOFTWARE RENDERING DATA STRUCTURES
// ================================================================================================

/** Polygon data for software rasterization queue */
typedef struct {
    int vert0;                          // Index into first vertex in eyepolv
    int b2sect, b2wall, b2slab;         // Build2 geometry references
    int b2hashn;                        // Hash chain for polygon matching
    int curcol, flags;                  // Color and rendering flags
    tile_t *tpic;                       // Texture tile pointer
    float ouvmat[9];                    // inverse perspective transformation
    point3d norm;                       // Surface normal vector
    int rdepth;
    int chain1_start; // NEW: index where chain 1 begins (relative to vert0)
    triangle_strip_t tri_strip;
    bool has_triangulation;
} eyepol_t;

// ================================================================================================
// GLOBAL VARIABLES
// ================================================================================================
extern int shadowtest2_updatelighting;
// Light system
extern lightpos_t shadowtest2_light[LIGHTMAX];  // Light source array
extern int shadowtest2_numlights;               // Current number of active lights
extern int shadowtest2_useshadows;              // Global shadow enable flag
extern int shadowtest2_numcpu;                  // Number of CPU threads to use
extern float shadowtest2_ambrgb[3];             // Ambient light RGB values

// Sector visibility tracking
extern unsigned int *shadowtest2_sectgot;       // Global sector visibility bit array
extern int shadowtest2_sectgotn;                // Size of global sector bit array

// Rendering mode control
extern int shadowtest2_rendmode;                // Current rendering mode (0-4)
extern eyepol_t *eyepol; // 4096 eyepol_t's = 192KB
extern point3d *eyepolv; //16384 point2d's  = 128KB
extern int eyepoln, glignum;
extern int eyepolmal, eyepolvn, eyepolvmal;
extern transform lastcamtr;
extern transform lastcamtr2;

extern bool captureframe;
// ================================================================================================
// POLYGONAL SCENE CLIPPING FUNCTIONS
// ================================================================================================
void normalize_transform(transform *tr);
/** Main sector scanning with near-plane clipping
 * @param sectnum Sector index to scan and add to bunch list
 */
void scansector(int sectnum, bunchgrp *b);

/** Prepares wall segments for bunch processing
 * @param b Bunch index to process
 * @param twal Output array for wall vertices (must be sector.n+1 size)
 * @return Number of vertices generated
 */
int prepbunch(int id, bunchverts_t *twal, bunchgrp* b);

/** Clips polygons to viewing frustum before rendering
 * @param tag Current portal tag
 * @param newtag New portal tag (-1 for final render)
 * @param plothead0 First polygon loop head
 * @param plothead1 Second polygon loop head
 * @param flags Clipping flags: &1=do and, &2=do sub, &4=reverse cut for sub
 */
int drawpol_befclip(int tag1, int newtag1, int sec, int newsec,
                            int plothead0, int plothead1, int flags, bunchgrp *b);
/** Main HSR (Hidden Surface Removal) function handling both clipping and rendering
 * @param cc Camera parameters
 * @param lgs Game state with geometry data
 * @param lps Player state with rendering settings
 * @param cursect Current sector index
 */
void reset_context();
void draw_hsr_polymost(cam_t *cc, mapstate_t *map, int dummy);
void draw_hsr_ctx (mapstate_t *lgs, bunchgrp *newctx);
// ================================================================================================
// POLYGONAL SHADOW CREATION FUNCTIONS
// ================================================================================================
void draw_hsr_enter_portal(mapstate_t* map, int endportaln, bunchgrp *b, int plothead0, int plothead1);

/** Creates shadow polygon lists for light sources
 * @param rethead0 First polygon loop head from clipping
 * @param rethead1 Second polygon loop head from clipping
 */
void ligpoltagfunc(int rethead0, int rethead1, bunchgrp *b);

/** Resets light polygon data structures
 * @param ind Light index to reset (-1 for all lights)
 */
void shadowtest2_ligpolreset(int ind);

/** Prepares light ramping calculations for shadows
 * @param ouvmat UV mapping matrix for surface
 * @param norm Surface normal vector
 * @param lig Light source index
 * @param hl Output light interpolation data
 */
void prepligramp(float *ouvmat, point3d *norm, int lig, void *hl);

/** Main shadow polygon rendering with light calculations
 * @param ei Eye polygon index to render with lighting
 */
void drawpollig(int ei);

/** Checks if current view intersects light's sector list
 * @param lignum Light source index to check
 * @return Non-zero if intersection exists
 */
int shadowtest2_isgotsectintersect(int lignum);

// ================================================================================================
// SOFTWARE RENDER ONLY FUNCTIONS
// ================================================================================================

/** Software polygon rasterization (thread-safe)
 * @param ind Eye polygon index to rasterize
 */
void eyepol_drawfunc(int ind);

/** Adds polygons to software render queue
 * @param rethead0 First polygon loop head
 * @param rethead1 Second polygon loop head
 */
void drawtagfunc(int rethead0, int rethead1, bunchgrp * b);

/** Software skybox rendering
 * @param rethead0 First polygon loop head
 * @param rethead1 Second polygon loop head
 */
void skytagfunc(int rethead0, int rethead1, bunchgrp * b);

// Texture coordinate generation functions
//void gentex_wall(void *npol2, void *sur);       // Wall texture mapping
//void gentex_ceilflor(void *sec, void *wal, void *sur, int isflor); // Ceiling/floor texture mapping
//void gentex_sky(void *sur);                     // Sky texture mapping

// ================================================================================================
// UTILITY AND MANAGEMENT FUNCTIONS
// ================================================================================================

/** Portal traversal function used in both clipping and rendering phases
 * @param rethead0 First polygon loop head
 * @param rethead1 Second polygon loop head
 */
void changetagfunc(int rethead0, int rethead1, bunchgrp* b);

/** Processes wall segments, handles both clipping and rendering setup
 * @param bid Bunch index to process
 */
static void drawalls(mapstate_t *map, int s, int *walls, int wallcount, bunchgrp *b);

/** Renders sprites with lighting if available */
void drawsprites();

/** Sets camera parameters for shadow system
 * @param ncam New camera parameters
 */
void shadowtest2_setcam(cam_t *ncam);

/** Removes a light source and frees its memory
 * @param i Light index to delete
 */
void shadowtest2_dellight(int i);

/** Initialize shadow system */
void shadowtest2_init();

/** Cleanup shadow system resources */
void shadowtest2_uninit();

#endif // SHADOWTEST2_H
