/*
 * BUILD2 Art Loading module by Ken Silverman (http://advsys.net/ken)
 * Modified: kz* file I/O replaced with raylib LoadFileData
 */
#include "artloader.h"
#include <string.h>
#include <math.h>
#include "mapcore.h"
#include "raylib.h"

gallery g_gals[16] = {0};
int galcount = 0;
tile_t* gtile;
tile_t* getGtile(int i){return &gtile[i];}

unsigned char* getGalleryColor(int gal_idx, int color_idx) {
    if (gal_idx < 0 || gal_idx >= 16) return NULL;
    return (unsigned char*)g_gals[gal_idx].globalpal[color_idx];
}
unsigned char* getColor(int idx) {
    return getGalleryColor(0, idx);
}

void LoadGalleryPal(int gal_idx, const char* basepath) {
    if (gal_idx < 0 || gal_idx >= 16) return;
    galcount++;
    gallery* gal = &g_gals[gal_idx];
    if (gal->gotpal) return;

    for(int i = 0; i < 256; i++)
        gal->gammlut[i] = pow(((double)i) * (1.0/256.0), 1.0) * 256.0;

    char tbuf[MAX_PATH*2];
    sprintf(tbuf, "%spalette.dat", basepath);

    int dataSize = 0;
    unsigned char *data = LoadFileData(tbuf, &dataSize);
    if (data && dataSize >= 768) {
        memcpy(gal->globalpal, data, 768);
        *(long*)&gal->globalpal[256][0] = 0^0xff000000;

        for(int i = 255-1; i >= 0; i--) {
            gal->globalpal[i][3] = 255;
            gal->globalpal[i][2] = gal->gammlut[gal->globalpal[0][i*3+2]<<2];
            gal->globalpal[i][1] = gal->gammlut[gal->globalpal[0][i*3+1]<<2];
            gal->globalpal[i][0] = gal->gammlut[gal->globalpal[0][i*3  ]<<2];

            unsigned char uch = gal->globalpal[i][0];
            gal->globalpal[i][0] = gal->globalpal[i][2];
            gal->globalpal[i][2] = uch;
        }
        gal->globalpal[255][3] = 0;
        gal->gotpal = 1;
    }
    if (data) UnloadFileData(data);
}

unsigned char globalpal[256][4];

void scaletex_boxsum_intr(tiltyp *rt, tiltyp *wt)
{
}

void fixtex4grou_intr(tiltyp *tt)
{
    memcpy((void *)(tt->p*tt->y + tt->f),(void *)tt->f,tt->p+4);
}

void kpzload4grou_intr(const char *filnam, tiltyp *tt, float shsc, int flags)
{
    tiltyp pow2t;
    long x, y, nx, ny, lnx, lny, xorval, *lptr;

    tt->f = 0; tt->lowermip = 0;

    int leng = 0;
    unsigned char *buf = LoadFileData(filnam, &leng);
    if (!buf || leng == 0) return;

    char *kbuf = (char *)malloc(leng+4);
    if (!kbuf) { UnloadFileData(buf); return; }
    memcpy(kbuf, buf, leng);
    UnloadFileData(buf);

    kpgetdim(kbuf, leng, (int *)&tt->x, (int *)&tt->y);

    if (tt->x <= 1) lnx = 0; else lnx = bsr(tt->x-1)+1;
    if (tt->y <= 1) lny = 0; else lny = bsr(tt->y-1)+1;
    nx = (1<<lnx); ny = (1<<lny);

    tt->p = (nx<<2); tt->f = (long)malloc((ny+1)*tt->p + 4);
    if (!tt->f) { free(kbuf); return; }
    if (kprender(kbuf, leng, tt->f, tt->p, nx, ny, 0, 0) < 0)
    { free(kbuf); free((void *)tt->f); tt->f = 0; return; }
    free(kbuf);

    if ((tt->x != nx) || (tt->y != ny))
    {
        pow2t.f = tt->f; pow2t.p = tt->p; pow2t.x = nx; pow2t.y = ny;
        scaletex_boxsum_intr((tiltyp *)tt, &pow2t);
        tt->x = nx; tt->y = ny;
    }

    if (flags&3)
    {
        xorval = 0;
        if (flags&1) xorval ^= 0x80000000;
        if (flags&2) xorval ^= 0xff000000;
        for(y=tt->y-1;y>=0;y--)
        {
            lptr = (long *)(y*tt->p + tt->f);
            for(x=tt->x-1;x>=0;x--) lptr[x] ^= xorval;
        }
    }

    fixtex4grou_intr(tt);
    tt->shsc = shsc;
}

void CleanTiles(){ }

/* Decode all tiles from one already-loaded ART buffer into gal->gtile[].
   loctile0/loctile1 are the tile range from the ART header.
   arttiles is the total allocated tile count (bounds check). */
static void decode_art_tiles(
    gallery *gal,
    const unsigned char *artdata, int artsize,
    int loctile0, int loctile1,
    int arttiles)
{
    int count = loctile1 - loctile0;
    const short *sxsiz = (const short *)(artdata + 16);
    const short *sysiz = sxsiz + count;
    /* pixel data starts after: 16 + count*2 (sx) + count*2 (sy) + count*4 (picanm) = 16 + count*8 */
    int pixel_offset = 16 + (count << 3);

    for (int i = 0; i < count; i++) {
        int tilenum = loctile0 + i;
        if (tilenum >= arttiles) break;

        int sx = sxsiz[i];
        int sy = sysiz[i];

        tiltyp *pic = &gal->gtile[tilenum].tt;

        /* skip zero-size tiles — nullpic assigned later in loadgal */
        if (sx <= 0 || sy <= 0) {
            pixel_offset += 0; /* no pixels */
            continue;
        }

        int pixel_count = sx * sy;
        if (pixel_offset + pixel_count > artsize) break; /* corrupt */

        const unsigned char *pixels = artdata + pixel_offset;
        pixel_offset += pixel_count;

        pic->x = sx;
        pic->y = sy;
        pic->p = (sx << 2);
        pic->f = (long)malloc((sy + 1) * pic->p + 4);
        if (!pic->f) continue;
        memset((void*)pic->f, 0, (sy + 1) * pic->p + 4);

        for (int x = 0; x < sx; x++) {
            for (int y = 0; y < sy; y++) {
                long *dst = (long*)(pic->f + y * pic->p + (x << 2));
                *dst = *(long*)&gal->globalpal[(long)pixels[x * sy + y]][0];
            }
        }

        fixtex4grou_intr(pic);
        pic->lowermip = 0;
    }
}

void setgammlut(double gammval)
{
    long i;
    gammval = 1.0/gammval;
    for(i=0;i<256;i++) gammlut[i] = pow(((double)i)*(1.0/256.0),gammval)*256.0;
}

void LoadPal(const char* basepath) {
    char tbuf[MAX_PATH*2];
    int i;
    unsigned char uch;

    if (gotpal) return;
    setgammlut(1.0);

    sprintf(tbuf, "%spalette.dat", basepath);

    int dataSize = 0;
    unsigned char *data = LoadFileData(tbuf, &dataSize);
    if (data && dataSize >= 768) {
        memcpy(globalpal, data, 768);
        *(long *)&globalpal[256][0] = 0^0xff000000;
        for(i=255-1;i>=0;i--) {
            globalpal[i][3] = 255;
            globalpal[i][2] = gammlut[globalpal[0][i*3+2]<<2];
            globalpal[i][1] = gammlut[globalpal[0][i*3+1]<<2];
            globalpal[i][0] = gammlut[globalpal[0][i*3  ]<<2];
            uch = globalpal[i][0]; globalpal[i][0] = globalpal[i][2]; globalpal[i][2] = uch;
        }
        globalpal[255][3] = 0;
        gotpal = 1;
    }
    if (data) UnloadFileData(data);
}

void galfreetextures(int gal_idx) {
    if (gal_idx < 0 || gal_idx >= 16) return;
    gallery* gal = &g_gals[gal_idx];
    if (gal->gtile) {
        for (int i = 0; i < gal->gnumtiles; i++) {
            if (gal->gtile[i].tt.f && gal->gtile[i].tt.f != (long)nullpic) {
                free((void*)gal->gtile[i].tt.f);
                gal->gtile[i].tt.f = 0;
            }
        }
    }
}

void galfree(int gal_idx) {
    if (gal_idx < 0 || gal_idx >= 16) return;
    gallery* gal = &g_gals[gal_idx];
    galfreetextures(gal_idx);
    if (gal->picanm_data) { free(gal->picanm_data); gal->picanm_data = NULL; }
}

int loadgal(int gal_idx, const char* path) {
    if (gal_idx < 0 || gal_idx >= 16) return 0;

    gallery* gal = &g_gals[gal_idx];

    galfreetextures(gal_idx);
    if (gal->gtile)  { free(gal->gtile);  gal->gtile  = NULL; }
    if (gal->sizex)  { free(gal->sizex);  gal->sizex  = NULL; }
    if (gal->sizey)  { free(gal->sizey);  gal->sizey  = NULL; }

    strcpy(gal->curmappath, path);
    int j = 0;
    for(int i = 0; gal->curmappath[i]; i++)
        if ((gal->curmappath[i] == '/') || (gal->curmappath[i] == '\\')) j = i + 1;
    gal->curmappath[j] = 0;

    LoadGalleryPal(gal_idx, gal->curmappath);
    gal->gotpal = 1;

    /* --- Pass 1: count total tiles across all ART files --- */
    int arttiles = 0;
    uint16_t *tilesizx = NULL, *tilesizy = NULL;
    picanm_t *picanm = NULL;
    char tbuf[MAX_PATH*2];

    for(int filnum = 0; ; filnum++) {
        sprintf(tbuf, "%sTILES%03d.ART", gal->curmappath, filnum);

        int artsize = 0;
        unsigned char *artdata = LoadFileData(tbuf, &artsize);
        if (!artdata) break;
        if (artsize < 16 || *(long*)artdata != 1) { UnloadFileData(artdata); break; }

        int loctile0 = *(long*)(artdata + 8);
        int loctile1 = (*(long*)(artdata + 12)) + 1;

        if ((loctile0 < 0) || (loctile1 <= arttiles) || (loctile0 >= loctile1)) {
            UnloadFileData(artdata); continue;
        }

        int old_arttiles = arttiles;
        arttiles = loctile1;

        tilesizx = (uint16_t*)realloc(tilesizx, arttiles * sizeof(uint16_t));
        tilesizy = (uint16_t*)realloc(tilesizy, arttiles * sizeof(uint16_t));
        picanm    = (picanm_t*)realloc(picanm,   arttiles * sizeof(picanm_t));

        for(int i = old_arttiles; i < arttiles; i++) {
            tilesizx[i] = 0; tilesizy[i] = 0; picanm[i].asint = 0;
        }

        int count = loctile1 - loctile0;
        const short *sx = (const short*)(artdata + 16);
        const short *sy = sx + count;
        const picanm_t *pa = (const picanm_t*)(sy + count);

        for(int i = 0; i < count; i++) {
            tilesizx[loctile0 + i] = sx[i];
            tilesizy[loctile0 + i] = sy[i];
            picanm[loctile0 + i]   = pa[i];
        }

        UnloadFileData(artdata);
    }

    if (!arttiles) {
        tilesizx = (uint16_t*)malloc(sizeof(uint16_t));
        tilesizy = (uint16_t*)malloc(sizeof(uint16_t));
        picanm   = (picanm_t*)malloc(sizeof(picanm_t));
        tilesizx[0] = tilesizy[0] = 2;
        picanm[0].asint = 0;
        arttiles = 1;
    }

    gal->sizex       = tilesizx;
    gal->sizey       = tilesizy;
    gal->picanm_data = picanm;
    gal->gnumtiles   = arttiles;
    gal->gmaltiles   = arttiles;

    gal->gtile = (tile_t*)malloc(arttiles * sizeof(tile_t));
    memset(gal->gtile, 0, arttiles * sizeof(tile_t));

    /* set filnam and zero tt for all tiles */
    for(int i = 0; i < arttiles; i++) {
        sprintf(gal->gtile[i].filnam, "tiles000.art|%d", i);
        gal->gtile[i].tt.f = 0;
    }

    /* --- Pass 2: one file open per ART, decode all tiles from it --- */
    for(int filnum = 0; ; filnum++) {
        sprintf(tbuf, "%sTILES%03d.ART", gal->curmappath, filnum);

        int artsize = 0;
        unsigned char *artdata = LoadFileData(tbuf, &artsize);
        if (!artdata) break;
        if (artsize < 16 || *(long*)artdata != 1) { UnloadFileData(artdata); break; }

        int loctile0 = *(long*)(artdata + 8);
        int loctile1 = (*(long*)(artdata + 12)) + 1;

        if (loctile0 >= 0 && loctile1 > loctile0)
            decode_art_tiles(gal, artdata, artsize, loctile0, loctile1, arttiles);

        UnloadFileData(artdata);
    }

    /* assign nullpic to any tile with no pixel data */
    for(int i = 0; i < arttiles; i++) {
        if (!gal->gtile[i].tt.f) {
            gal->gtile[i].tt.f = (long)nullpic;
            gal->gtile[i].tt.x = 4;
            gal->gtile[i].tt.y = 4;
            gal->gtile[i].tt.p = (4<<2);
            gal->gtile[i].tt.lowermip = 0;
        }
    }

    return 1;
}
