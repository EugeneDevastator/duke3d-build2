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

    /* +4 avoids bug in KPLIB overrunning */
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

/* -----------------------------------------------------------------------
   ART loading helpers: operate on already-loaded file data in memory
   ----------------------------------------------------------------------- */

/* Walk ART data in memory, return pointer to pixel data for tilenum.
   Returns NULL on failure. Fills out sx/sy. */
static const unsigned char* art_find_tile(
    const unsigned char *artdata, int artsize,
    int tilenum_global, int loctile0, int loctile1,
    int *out_sx, int *out_sy)
{
    int local = tilenum_global - loctile0;
    if (local < 0 || local >= loctile1) return NULL;

    const short *sxsiz = (const short *)(artdata + 16);
    const short *sysiz = sxsiz + loctile1;
    /* skip picanm: loctile1 * 4 bytes after sysiz */
    int pixel_base = 16 + (loctile1 * 4); /* sxsiz+sysiz = loctile1*2*2 */
    /* actually: 16 + loctile1*sizeof(short) + loctile1*sizeof(short) + loctile1*sizeof(picanm) */
    /* = 16 + loctile1*2 + loctile1*2 + loctile1*4 = 16 + loctile1*8 */
    pixel_base = 16 + (loctile1 << 3);

    int offset = pixel_base;
    for(int i = 0; i < local; i++)
        offset += (int)sxsiz[i] * (int)sysiz[i];

    *out_sx = sxsiz[local];
    *out_sy = sysiz[local];
    if (*out_sx <= 0 || *out_sy <= 0) return NULL;
    if (offset + (*out_sx) * (*out_sy) > artsize) return NULL;
    return artdata + offset;
}

void loadpic_raw(tile_t *tpic, char* rootpath, int gal_idx) {
    long i, j, filnum, tilenum, loctile0, loctile1;
    char tbuf[MAX_PATH*2];

    if (gal_idx < 0 || gal_idx >= 16) return;

    gallery* gal = &g_gals[gal_idx];
    tiltyp *pic = &tpic->tt;

    if (pic->f && pic->f != (long)nullpic) {
        free((void *)pic->f);
        pic->f = 0;
    }

    strcpy(tbuf, tpic->filnam);

    for(i=j=0;tbuf[i];i++) if (tbuf[i] == '|') j = i;
    if (!j) { tilenum = 0; } else { tilenum = atol(&tbuf[j+1]); tbuf[j] = 0; i = j; }

    if ((i >= 5) && (!stricmp(&tbuf[i-4],".ART"))) {
        pic->x = g_gals[gal_idx].sizex[tilenum];
        pic->y = g_gals[gal_idx].sizey[tilenum];

        if (pic->x <= 0 || pic->y <= 0) {
            pic->f = (long)nullpic; pic->x = 4; pic->y = 4;
            pic->p = (pic->x<<2); pic->lowermip = 0;
            return;
        }

        filnum = 0;
        do {
            sprintf(tbuf, "%sTILES%03d.ART", rootpath, filnum);

            int artsize = 0;
            unsigned char *artdata = LoadFileData(tbuf, &artsize);
            if (!artdata) { filnum = -1; break; }
            if (artsize < 16 || *(long*)artdata != 1) { UnloadFileData(artdata); filnum = -1; break; }

            loctile0 = *(long*)(artdata + 8);
            loctile1 = (*(long*)(artdata + 12)) - loctile0 + 1;
            i = tilenum - loctile0;

            if ((unsigned)i < (unsigned)loctile1) {
                /* found the right ART file */
                int sx, sy;
                const unsigned char *pixels = art_find_tile(artdata, artsize, tilenum, loctile0, loctile1, &sx, &sy);

                if (pixels && sx > 0 && sy > 0) {
                    pic->p = (pic->x<<2);
                    pic->f = (uint32_t)malloc((pic->y+1)*pic->p+4);
                    memset((void*)pic->f, 0, (pic->y+1)*pic->p+4);

                    for(int x=0;x<pic->x;x++) {
                        for(int y=0;y<pic->y;y++) {
                            long *pixel_ptr = (long*)(pic->f + y*pic->p + (x<<2));
                            *pixel_ptr = *(long*)&gal->globalpal[(long)pixels[x*pic->y + y]][0];
                        }
                    }
                }
                UnloadFileData(artdata);
                break;
            }
            filnum++;
            UnloadFileData(artdata);
        } while (1);

        if (filnum < 0 || !pic->f) {
            pic->f = (long)nullpic; pic->x = 4; pic->y = 4;
            pic->p = (pic->x<<2); pic->lowermip = 0;
            return;
        }

        fixtex4grou_intr((tiltyp *)pic);
        pic->lowermip = 0;
    } else {
        tiltyp gtt;
        kpzload4grou_intr(tbuf,&gtt,1.0,2);
        pic->f = gtt.f; pic->p = gtt.p; pic->x = gtt.x; pic->y = gtt.y; pic->lowermip = gtt.lowermip;
    }

    if (!pic->f) {
        pic->f = (long)nullpic; pic->x = 4; pic->y = 4;
        pic->p = (pic->x<<2); pic->lowermip = 0;
    }
}

void loadpic(tile_t *tpic, char* rootpath, int gal_idx) {
    long i, j, filnum, tilenum, loctile0, loctile1, lnx, lny, nx, ny;
    char tbuf[MAX_PATH*2];

    if (gal_idx < 0 || gal_idx >= 16) return;

    gallery* gal = &g_gals[gal_idx];
    tiltyp *pic = &tpic->tt;

    if (pic->f && pic->f != (long)nullpic) {
        free((void *)pic->f);
        pic->f = 0;
    }

    strcpy(tbuf, tpic->filnam);

    for(i=j=0;tbuf[i];i++) if (tbuf[i] == '|') j = i;
    if (!j) { tilenum = 0; } else { tilenum = atol(&tbuf[j+1]); tbuf[j] = 0; i = j; }

    if ((i >= 5) && (!stricmp(&tbuf[i-4],".ART"))) {
        pic->x = g_gals[gal_idx].sizex[tilenum];
        pic->y = g_gals[gal_idx].sizey[tilenum];

        if (pic->x <= 0 || pic->y <= 0) {
            pic->f = (long)nullpic; pic->x = 4; pic->y = 4;
            pic->p = (pic->x<<2); pic->lowermip = 0;
            return;
        }

        if (pic->x <= 1) lnx = 0; else lnx = bsr(pic->x-1)+1;
        if (pic->y <= 1) lny = 0; else lny = bsr(pic->y-1)+1;
        nx = (1<<lnx); ny = (1<<lny);

        filnum = 0;
        do {
            sprintf(tbuf, "%sTILES%03d.ART", rootpath, filnum);

            int artsize = 0;
            unsigned char *artdata = LoadFileData(tbuf, &artsize);
            if (!artdata) { filnum = -1; break; }
            if (artsize < 16 || *(long*)artdata != 1) { UnloadFileData(artdata); filnum = -1; break; }

            loctile0 = *(long*)(artdata + 8);
            loctile1 = (*(long*)(artdata + 12)) - loctile0 + 1;
            i = tilenum - loctile0;

            if ((unsigned)i < (unsigned)loctile1) {
                int sx, sy;
                const unsigned char *pixels = art_find_tile(artdata, artsize, tilenum, loctile0, loctile1, &sx, &sy);

                if (pixels && sx > 0 && sy > 0) {
                    pic->p = (nx<<2);
                    pic->f = (uint32_t)malloc((ny+1)*pic->p+4);

                    for(int x=0;x<pic->x;x++) {
                        long base = (x<<2) + pic->f;
                        for(int y=0;y<pic->y;y++, base+=pic->p)
                            *(long*)base = *(long*)&gal->globalpal[(long)pixels[x*pic->y + y]][0];
                    }

                    if ((pic->x != nx) || (pic->y != ny)) {
                        tiltyp pow2t;
                        pow2t.f = pic->f; pow2t.p = pic->p; pow2t.x = nx; pow2t.y = ny;
                        scaletex_boxsum_intr((tiltyp *)pic, &pow2t);
                        pic->x = nx; pic->y = ny;
                    }
                }
                UnloadFileData(artdata);
                break;
            }
            filnum++;
            UnloadFileData(artdata);
        } while (1);

        if (filnum < 0 || !pic->f) {
            pic->f = (long)nullpic; pic->x = 4; pic->y = 4;
            pic->p = (pic->x<<2); pic->lowermip = 0;
            return;
        }

        fixtex4grou_intr((tiltyp *)pic);
        pic->lowermip = 0;
    } else {
        tiltyp gtt;
        kpzload4grou_intr(tbuf,&gtt,1.0,2);
        pic->f = gtt.f; pic->p = gtt.p; pic->x = gtt.x; pic->y = gtt.y; pic->lowermip = gtt.lowermip;
    }

    if (!pic->f) {
        pic->f = (long)nullpic; pic->x = 4; pic->y = 4;
        pic->p = (pic->x<<2); pic->lowermip = 0;
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

        /* read directly from in-memory artdata */
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

    for(int i = 0; i < arttiles; i++) {
        sprintf(gal->gtile[i].filnam, "tiles000.art|%d", i);
        gal->gtile[i].tt.f = 0;
    }

    for(int i = 0; i < arttiles; i++) {
        if (tilesizx[i] > 0 && tilesizy[i] > 0)
            loadpic_raw(&gal->gtile[i], gal->curmappath, gal_idx);
    }

    return 1;
}
