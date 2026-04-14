// File: .../RayLua/Core/mapserial.c
#include "mapserial.h"
#include <stdlib.h>
#include <string.h>

// ================================================================
// CONVERSION HELPERS — all fixed, no literals scattered below
// ================================================================

// --- int64 <-> float (shift 16, ~0.015mm res) ---
static inline int64_t f_to_i64(float v)  { return (int64_t)(v * (float)(1 << SPACE_SCALE_SHIFT)); }
static inline float   i64_to_f(int64_t v){ return (float)v * SPACE_SCALE_F; }

// --- int32 <-> float (shift 10, ~0.977mm res) ---
static inline int32_t f_to_i32(float v)  { return (int32_t)(v * (float)(1 << SPACE32_SCALE_SHIFT)); }
static inline float   i32_to_f(int32_t v){ return (float)v * SPACE32_SCALE_F; }

// --- int16 <-> float (shift 10, same res as i32 but clamped range) ---
static inline int16_t f_to_i16(float v)  { return (int16_t)(v * (float)(1 << SPACE16_SCALE_SHIFT)); }
static inline float   i16_to_f(int16_t v){ return (float)v * SPACE16_SCALE_F; }

// --- UV float <-> int16 (shift 12, ~0.00024 res, good for 0..8 range) ---
#define UV_SHIFT 12
#define UV_SCALE_F (1.0f / (float)(1 << UV_SHIFT))
static inline int16_t f_to_uv16(float v) { return (int16_t)(v * (float)(1 << UV_SHIFT)); }
static inline float   uv16_to_f(int16_t v){ return (float)v * UV_SCALE_F; }

// --- point conversions ---
static inline point64_t p3_to_p64(point3d p) {
    point64_t r; r.x = f_to_i64(p.x); r.y = f_to_i64(p.y); r.z = f_to_i64(p.z); return r;
}
static inline point3d p64_to_p3(point64_t p) {
    point3d r; r.x = i64_to_f(p.x); r.y = i64_to_f(p.y); r.z = i64_to_f(p.z); return r;
}
static inline transform64_t tr_to_tr64(transform t) {
    transform64_t r;
    r.p = p3_to_p64(t.p); r.r = p3_to_p64(t.r);
    r.f = p3_to_p64(t.f); r.d = p3_to_p64(t.d);
    return r;
}
static inline transform tr64_to_tr(transform64_t t) {
    transform r;
    r.p = p64_to_p3(t.p); r.r = p64_to_p3(t.r);
    r.f = p64_to_p3(t.f); r.d = p64_to_p3(t.d);
    return r;
}

// --- uvform_t (float) <-> uvform128_t (int16, uv shift) ---
static inline uvform128_t uvform_to_128(uvform_t u) {
    uvform128_t r;
    r.scale.x = f_to_uv16(u.scale.x); r.scale.y = f_to_uv16(u.scale.y); r.scale.z = f_to_uv16(u.scale.z);
    r.pan.x   = f_to_uv16(u.pan.x);   r.pan.y   = f_to_uv16(u.pan.y);   r.pan.z   = f_to_uv16(u.pan.z);
    r.cropA.x = f_to_uv16(u.cropA.x); r.cropA.y = f_to_uv16(u.cropA.y); r.cropA.z = 0;
    r.cropB.x = f_to_uv16(u.cropB.x); r.cropB.y = f_to_uv16(u.cropB.y); r.cropB.z = 0;
    r.rot.x   = f_to_uv16(u.rot.x);   r.rot.y   = f_to_uv16(u.rot.y);   r.rot.z   = f_to_uv16(u.rot.z);
    r.scaling_mode  = u.scaling_mode;
    r.mapping_kind  = u.mapping_kind;
    r.tile_ordering = u.tile_ordering;
    return r;
}
static inline uvform_t uvform_from_128(uvform128_t u) {
    uvform_t r;
    r.scale.x = uv16_to_f(u.scale.x); r.scale.y = uv16_to_f(u.scale.y); r.scale.z = uv16_to_f(u.scale.z);
    r.pan.x   = uv16_to_f(u.pan.x);   r.pan.y   = uv16_to_f(u.pan.y);   r.pan.z   = uv16_to_f(u.pan.z);
    r.cropA.x = uv16_to_f(u.cropA.x); r.cropA.y = uv16_to_f(u.cropA.y); r.cropA.z = 0;
    r.cropB.x = uv16_to_f(u.cropB.x); r.cropB.y = uv16_to_f(u.cropB.y); r.cropB.z = 0;
    r.rot.x   = uv16_to_f(u.rot.x);   r.rot.y   = uv16_to_f(u.rot.y);   r.rot.z   = uv16_to_f(u.rot.z);
    r.scaling_mode  = u.scaling_mode;
    r.mapping_kind  = u.mapping_kind;
    r.tile_ordering = u.tile_ordering;
    return r;
}

// ================================================================
// SURF: runtime surf_t <-> storage surf_store
// surf_store uses dataheader + mat_surf (which has uvform128_t inside)
// ================================================================
static void surf_to_store(surf_store *dst, const surf_t *src) {
    memset(dst, 0, sizeof(surf_store));
    dst->head.id     = src->id;
    dst->head.lotag  = src->lotag;
    dst->head.hitag  = src->hitag;
    dst->head.typeflags = (uint8_t)(src->flags & 0xFF);
    dst->head.cmdtag = (int32_t)(src->flags >> 8);

    dst->mat.galnum  = src->galnum;
    dst->mat.tilnum  = src->tilnum;
    dst->mat.pal     = src->pal;
    dst->mat.procuv  = src->uvgen;
    dst->mat.uvform  = uvform_to_128(src->uvform);
    dst->mat.geoflags= src->geoflags;

    // color: asc/rsc/gsc/bsc -> color_hdr_t
    // asc/rsc/gsc/bsc are uint16 where 4096 = no change (1.0)
    // store directly in color r,g,b,a as int16 (same scale)
    dst->mat.color.r = (int16_t)src->rsc;
    dst->mat.color.g = (int16_t)src->gsc;
    dst->mat.color.b = (int16_t)src->bsc;
    dst->mat.color.a = (int16_t)src->asc;
    // alpha: store in mantissa (0..1 -> 0..65535)
    dst->mat.color.mantissa = (uint16_t)(src->alpha * 65535.0f);
    dst->mat.color.exp      = 0;

    dst->mat.blend_kind  = (uint8_t)src->frMode;
    dst->mat.optic_flags = 0;
}

static void store_to_surf(surf_t *dst, const surf_store *src) {
    memset(dst, 0, sizeof(surf_t));
    dst->id     = src->head.id;
    dst->lotag  = (short)src->head.lotag;
    dst->hitag  = (short)src->head.hitag;
    dst->flags  = (uint32_t)src->head.typeflags | ((uint32_t)src->head.cmdtag << 8);

    dst->galnum = src->mat.galnum;
    dst->tilnum = src->mat.tilnum;
    dst->pal    = src->mat.pal;
    dst->uvgen  = src->mat.procuv;
    dst->uvform = uvform_from_128(src->mat.uvform);
    dst->geoflags = (uint8_t)src->mat.geoflags;

    dst->rsc    = (unsigned short)src->mat.color.r;
    dst->gsc    = (unsigned short)src->mat.color.g;
    dst->bsc    = (unsigned short)src->mat.color.b;
    dst->asc    = (unsigned short)src->mat.color.a;
    dst->alpha  = (float)src->mat.color.mantissa / 65535.0f;

    dst->frMode = (enum fragRenderMode)src->mat.blend_kind;
}

// ================================================================
// WALL SERIAL — flat struct, no floats
// ================================================================
// wall_store from mapform has surf_store surf0 + x,y as int64 + nxsurfs
// We need: n, ns, nw, nschain, nwchain, surfn, geoflags, dataid, tguid,
//          mflags, tags, tempflags, xsurf[3]
// Pack extras into a wrapper:
#pragma pack(push,1)
typedef struct {
    wall_store  base;       // surf0, x, y, nxsurfs, dataflex
    surf_store  xsurf[3];
    uint32_t    dataid;
    uint32_t    tguid;
    int32_t     n, ns, nw;
    int32_t     nschain, nwchain;
    uint8_t     surfn;
    uint8_t     geoflags;
    int16_t     mflags[4];
    int32_t     tags[16];
    int8_t      tempflags;
} wall_serial_t;

typedef struct {
    // caps[2] each has surf_store + slopehint + gradxy_zpos + nxsurfs
    // We store caps inline then extra surfs after
    surf_store  cap_surf[2];
    surf_store  cap_xsurf[2][3]; // up to 3 extra per cap
    slopehint_t cap_slope[2];
    point64_t   cap_grad[2];     // gradxy_zpos
    uint8_t     cap_nxsurfs[2];

    // sect header fields
    bb_uid_t    id;
    uint32_t    dataid;
    uint32_t    tguid;
    uint16_t    areaid;
    int64_t     z[2];
    int32_t     n, nmax;
    int32_t     headspri;
    float       minx, miny, maxx, maxy; // bounding box only, ok as float
    int32_t     tags[TAG_COUNT_PER_SECT];
    uint16_t    mflags[4];
    int16_t     scriptid, lotag, hitag;
    int32_t     destpn[2];
    uint8_t     hintw1, hintw2, hintmode;
    uint16_t    originwall;
} sect_serial_t;

typedef struct {
    transform64_t tr;
    uint32_t      packed_tile_data;
    uint32_t      rflags_raw;
    uvform128_t   uv;
    point16_t     anchor; // uv shift
    int32_t       sect;
    int16_t       lotag, hitag;
    int64_t       fat;
    int32_t       mas_raw;  // float bits
    int32_t       moi_raw;  // float bits
    uint16_t      clipmask;
    uint16_t      gameflags;
    uint16_t      signalmask;
    int32_t       flags;
    int32_t       tags[16];
    uint16_t      walcon;
} spri_serial_t;
#pragma pack(pop)

// ================================================================
// WALL
// ================================================================
static void wall_to_serial(wall_serial_t *dst, const wall_t *src) {
    memset(dst, 0, sizeof(wall_serial_t));
    surf_to_store(&dst->base.surf0, &src->surf);
    dst->base.x       = f_to_i64(src->x);
    dst->base.y       = f_to_i64(src->y);
    dst->base.nxsurfs = src->surfn > 1 ? src->surfn - 1 : 0;
    for (int i = 0; i < 3; i++) surf_to_store(&dst->xsurf[i], &src->xsurf[i]);
    dst->dataid   = src->dataid;
    dst->tguid    = src->tguid;
    dst->n        = src->n;
    dst->ns       = src->ns;
    dst->nw       = src->nw;
    dst->nschain  = src->nschain;
    dst->nwchain  = src->nwchain;
    dst->surfn    = src->surfn;
    dst->geoflags = src->geoflags;
    memcpy(dst->mflags, src->mflags, sizeof(dst->mflags));
    memcpy(dst->tags,   src->tags,   sizeof(dst->tags));
    dst->tempflags = src->tempflags;
}

static void serial_to_wall(wall_t *dst, const wall_serial_t *src) {
    memset(dst, 0, sizeof(wall_t));
    store_to_surf(&dst->surf, &src->base.surf0);
    dst->x        = i64_to_f(src->base.x);
    dst->y        = i64_to_f(src->base.y);
    for (int i = 0; i < 3; i++) store_to_surf(&dst->xsurf[i], &src->xsurf[i]);
    dst->dataid   = src->dataid;
    dst->tguid    = src->tguid;
    dst->n        = src->n;
    dst->ns       = src->ns;
    dst->nw       = src->nw;
    dst->nschain  = src->nschain;
    dst->nwchain  = src->nwchain;
    dst->surfn    = src->surfn;
    dst->geoflags = src->geoflags;
    dst->owner    = -1;
    memcpy(dst->mflags, src->mflags, sizeof(dst->mflags));
    memcpy(dst->tags,   src->tags,   sizeof(dst->tags));
    dst->tempflags = src->tempflags;
}

// ================================================================
// SECT
// ================================================================
static void sect_to_serial(sect_serial_t *dst, const sect_t *src) {
    memset(dst, 0, sizeof(sect_serial_t));
    dst->id      = src->id;
    dst->dataid  = src->dataid;
    dst->tguid   = src->tguid;
    dst->areaid  = src->areaid;
    dst->z[0]    = f_to_i64(src->z[0]);
    dst->z[1]    = f_to_i64(src->z[1]);
    dst->n       = src->n;
    dst->nmax    = src->nmax;
    dst->headspri= src->headspri;
    dst->minx    = src->minx; dst->miny = src->miny;
    dst->maxx    = src->maxx; dst->maxy = src->maxy;
    memcpy(dst->tags,   src->tags,   sizeof(dst->tags));
    memcpy(dst->mflags, src->mflags, sizeof(dst->mflags));
    dst->scriptid = src->scriptid;
    dst->lotag    = src->lotag;
    dst->hitag    = src->hitag;
    dst->destpn[0]= src->destpn[0];
    dst->destpn[1]= src->destpn[1];
    dst->hintw1   = src->hintw1;
    dst->hintw2   = src->hintw2;
    dst->hintmode = src->hintmode;

    // caps: surf[0]=ceil, surf[1]=floor
    for (int c = 0; c < 2; c++) {
        surf_to_store(&dst->cap_surf[c], &src->surf[c]);
        // grad stored as point64 in cap_t.gradxy_zpos
        dst->cap_grad[c].x = f_to_i64(src->grad[c].x);
        dst->cap_grad[c].y = f_to_i64(src->grad[c].y);
        dst->cap_grad[c].z = 0;
        dst->cap_nxsurfs[c] = 0; // no extra cap surfs in runtime currently
    }
}

static void serial_to_sect(sect_t *dst, const sect_serial_t *src) {
    memset(dst, 0, sizeof(sect_t));
    dst->id      = src->id;
    dst->dataid  = src->dataid;
    dst->tguid   = src->tguid;
    dst->areaid  = src->areaid;
    dst->z[0]    = i64_to_f(src->z[0]);
    dst->z[1]    = i64_to_f(src->z[1]);
    dst->n       = src->n;
    dst->nmax    = src->nmax;
    dst->headspri= src->headspri;
    dst->minx    = src->minx; dst->miny = src->miny;
    dst->maxx    = src->maxx; dst->maxy = src->maxy;
    dst->owner   = -1;
    memcpy(dst->tags,   src->tags,   sizeof(dst->tags));
    memcpy(dst->mflags, src->mflags, sizeof(dst->mflags));
    dst->scriptid = src->scriptid;
    dst->lotag    = src->lotag;
    dst->hitag    = src->hitag;
    dst->destpn[0]= src->destpn[0];
    dst->destpn[1]= src->destpn[1];
    dst->hintw1   = src->hintw1;
    dst->hintw2   = src->hintw2;
    dst->hintmode = src->hintmode;

    for (int c = 0; c < 2; c++) {
        store_to_surf(&dst->surf[c], &src->cap_surf[c]);
        dst->grad[c].x = i64_to_f(src->cap_grad[c].x);
        dst->grad[c].y = i64_to_f(src->cap_grad[c].y);
    }
}

// ================================================================
// SPRI
// ================================================================
static void spri_to_serial(spri_serial_t *dst, const spri_t *src) {
    memset(dst, 0, sizeof(spri_serial_t));
    dst->tr.p = p3_to_p64(src->p);
    dst->tr.r = p3_to_p64(src->r);
    dst->tr.d = p3_to_p64(src->d);
    dst->tr.f = p3_to_p64(src->f);
    dst->packed_tile_data = src->packed_tile_data;
    memcpy(&dst->rflags_raw, &src->view.rflags, 4);
    dst->uv     = uvform_to_128(src->view.uv);
    dst->anchor.x = f_to_uv16(src->view.anchor.x);
    dst->anchor.y = f_to_uv16(src->view.anchor.y);
    dst->anchor.z = f_to_uv16(src->view.anchor.z);
    dst->sect     = (int32_t)src->sect;
    dst->lotag    = src->lotag;
    dst->hitag    = src->hitag;
    dst->fat      = f_to_i64(src->phys.fat);
    // store float bits exactly — no precision loss
    memcpy(&dst->mas_raw, &src->phys.mas, 4);
    memcpy(&dst->moi_raw, &src->phys.moi, 4);
    dst->clipmask   = src->phys.clipmask;
    dst->gameflags  = src->gameflags;
    dst->signalmask = src->signalmask;
    dst->flags      = (int32_t)src->flags;
    memcpy(dst->tags, src->tags, sizeof(dst->tags));
    dst->walcon     = src->walcon;
}

static void serial_to_spri(spri_t *dst, const spri_serial_t *src) {
    memset(dst, 0, sizeof(spri_t));
    dst->p = p64_to_p3(src->tr.p);
    dst->r = p64_to_p3(src->tr.r);
    dst->d = p64_to_p3(src->tr.d);
    dst->f = p64_to_p3(src->tr.f);
    dst->packed_tile_data = src->packed_tile_data;
    memcpy(&dst->view.rflags, &src->rflags_raw, 4);
    dst->view.uv     = uvform_from_128(src->uv);
    dst->view.anchor.x = uv16_to_f(src->anchor.x);
    dst->view.anchor.y = uv16_to_f(src->anchor.y);
    dst->view.anchor.z = uv16_to_f(src->anchor.z);
    dst->sect          = src->sect;
    dst->lotag         = src->lotag;
    dst->hitag         = src->hitag;
    dst->phys.fat      = i64_to_f(src->fat);
    memcpy(&dst->phys.mas, &src->mas_raw, 4);
    memcpy(&dst->phys.moi, &src->moi_raw, 4);
    dst->phys.clipmask = src->clipmask;
    dst->gameflags     = src->gameflags;
    dst->signalmask    = src->signalmask;
    dst->flags         = src->flags;
    memcpy(dst->tags, src->tags, sizeof(dst->tags));
    dst->walcon        = src->walcon;
    dst->owner         = -1;
    dst->sectn         = -1;
    dst->sectp         = -1;
}

// ================================================================
// IO HELPERS
// ================================================================
#define MAP_B2_MAGIC    "BE20"
#define MAP_B2_VER_MAJOR 1
#define MAP_B2_VER_MINOR 0

static int fw(FILE *f, const void *d, size_t n) { return fwrite(d, 1, n, f) == n; }
static int fr(FILE *f,       void *d, size_t n) { return fread (d, 1, n, f) == n; }

// ================================================================
// SAVE
// ================================================================
int map_save_b2(const char *path, mapstate_t *map) {
    FILE *f = fopen(path, "wb");
    if (!f) return 0;

    map_b2_store_header_t hdr;
    memset(&hdr, 0, sizeof(hdr));
    memcpy(hdr.magic, MAP_B2_MAGIC, 4);
    hdr.ver_major = MAP_B2_VER_MAJOR;
    hdr.ver_minor = MAP_B2_VER_MINOR;
    hdr.n_sects   = (uint64_t)map->numsects;
    hdr.n_sprites = (uint64_t)map->numspris;
    hdr.n_chunks  = 0;
    hdr.start.p   = p3_to_p64(map->startpos);
    hdr.start.r   = p3_to_p64(map->startrig);
    hdr.start.d   = p3_to_p64(map->startdow);
    hdr.start.f   = p3_to_p64(map->startfor);

    if (!fw(f, &hdr, sizeof(hdr))) goto fail;

    {
        sect_serial_t ss;
        wall_serial_t ws;
        for (int i = 0; i < map->numsects; i++) {
            sect_t *sec = &map->sect[i];
            sect_to_serial(&ss, sec);
            if (!fw(f, &ss, sizeof(ss))) goto fail;
            for (int j = 0; j < sec->n; j++) {
                wall_to_serial(&ws, &sec->wall[j]);
                if (!fw(f, &ws, sizeof(ws))) goto fail;
            }
        }
    }

    {
        spri_serial_t sps;
        for (int i = 0; i < map->numspris; i++) {
            spri_to_serial(&sps, &map->spri[i]);
            if (!fw(f, &sps, sizeof(sps))) goto fail;
        }
    }

    fclose(f);
    return 1;
fail:
    fclose(f);
    return 0;
}

// ================================================================
// LOAD
// ================================================================
int map_load_b2(const char *path, mapstate_t *map) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;

    map_b2_store_header_t hdr;
    if (!fr(f, &hdr, sizeof(hdr)))                    goto fail;
    if (memcmp(hdr.magic, MAP_B2_MAGIC, 4) != 0)      goto fail;
    if (hdr.ver_major != MAP_B2_VER_MAJOR)             goto fail;

    map->startpos = p64_to_p3(hdr.start.p);
    map->startrig = p64_to_p3(hdr.start.r);
    map->startdow = p64_to_p3(hdr.start.d);
    map->startfor = p64_to_p3(hdr.start.f);

    {
        int ns  = (int)hdr.n_sects;
        int nsp = (int)hdr.n_sprites;

        map->numsects = ns;
        map->malsects = ns > 0 ? ns : 1;
        map->sect = (sect_t*)malloc(map->malsects * sizeof(sect_t));
        if (!map->sect) goto fail;
        memset(map->sect, 0, map->malsects * sizeof(sect_t));

        sect_serial_t ss;
        wall_serial_t ws;
        for (int i = 0; i < ns; i++) {
            if (!fr(f, &ss, sizeof(ss))) goto fail;
            serial_to_sect(&map->sect[i], &ss);
            int nw   = map->sect[i].n;
            int nwmax= map->sect[i].nmax;
            if (nwmax < nw) nwmax = nw;
            map->sect[i].wall = (wall_t*)malloc(nwmax * sizeof(wall_t));
            if (!map->sect[i].wall) goto fail;
            memset(map->sect[i].wall, 0, nwmax * sizeof(wall_t));
            for (int j = 0; j < nw; j++) {
                if (!fr(f, &ws, sizeof(ws))) goto fail;
                serial_to_wall(&map->sect[i].wall[j], &ws);
            }
        }

        map->numspris = nsp;
        map->malspris = nsp > 0 ? nsp : 1;
        map->spri = (spri_t*)malloc(map->malspris * sizeof(spri_t));
        if (!map->spri) goto fail;
        memset(map->spri, 0, map->malspris * sizeof(spri_t));

        spri_serial_t sps;
        for (int i = 0; i < nsp; i++) {
            if (!fr(f, &sps, sizeof(sps))) goto fail;
            serial_to_spri(&map->spri[i], &sps);
        }

        // rebuild sprite sector linked lists
        for (int i = 0; i < ns; i++) map->sect[i].headspri = -1;
        map->blankheadspri = -1;
        for (int i = 0; i < nsp; i++) {
            int s = map->spri[i].sect;
            if ((unsigned)s < (unsigned)ns) {
                map->spri[i].sectn = map->sect[s].headspri;
                map->spri[i].sectp = -1;
                if (map->sect[s].headspri >= 0)
                    map->spri[map->sect[s].headspri].sectp = i;
                map->sect[s].headspri = i;
            } else {
                map->spri[i].sectn = map->blankheadspri;
                map->spri[i].sectp = -1;
                map->blankheadspri = i;
            }
        }
    }

    fclose(f);
    return 1;
fail:
    fclose(f);
    return 0;
}
