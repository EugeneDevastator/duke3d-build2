/*
*       BUILD2 Art Loading moduykle by Ken Silverman (http://advsys.net/ken)
 *      This file has been modified from Ken Silverman's original release
 *
 *       Things changd:
 *       - moved into separate module
 *       - long -> int32_t for 4-byte pixel ops (64-bit compat)
 */
#include "artloader.h"

#include <string.h>
#include "mapcore.h"
#include "kplib.h"
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

	for(int i = 0; i < 256; i++) {
		gal->gammlut[i] = pow(((double)i) * (1.0/256.0), 1.0) * 256.0;
	}

	char tbuf[MAX_PATH*2];
	sprintf(tbuf, "%spalette.dat", basepath);

	if (kzopen(tbuf)) {
		kzread(gal->globalpal, 768);
		*(int32_t*)&gal->globalpal[256][0] = 0^0xff000000;

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
		kzclose();
		gal->gotpal = 1;
	}
}

unsigned char globalpal[256][4];
#if defined(_MSC_VER)
void divconst_setdenom_intr (long *twolongstate, long denom)
{
	_asm
	{
		mov eax, denom
		mov edx, 2
		cmp eax, edx
		jae short dc_l1
		neg eax
		jmp short dc_l2
dc_l1:lea ecx, [eax+eax]
		div ecx
dc_l2:mov ecx, twolongstate
		mov [ecx+4], eax ;dc_mul
		mul dword ptr denom
		xor eax, eax
		cmp edx, 1
		setc al
		mov [ecx], eax ;dc_add
	}
}
_inline long divconst (long *twolongstate, long numer)
{
	_asm
	{
		mov eax, numer
		mov edx, twolongstate
		add eax, [edx] ;dc_add
		mul dword ptr [edx+4] ;dc_mul
		mov eax, edx
	}
}
#else
static long divconst_denom = 1;
void divconst_setdenom_intr (long *twolongstate, long denom)
{ twolongstate[0] = divconst_denom; divconst_denom = denom; }
long divconst (long *twolongstate, long numer) { return(numer/twolongstate[0]); }
#endif

void scaletex_boxsum_intr (tiltyp *rt, tiltyp *wt)
{
	long i, rx, ry, rxf, ryf, nxm1, nym1, col0[4], col1[4], col2, ds, divstate[4][2];
	long x, y, xx, yy, lwx, lwy;
	int32_t *lptr;
	unsigned char *ucptr, *ucptr2;

	lwx = bsr(wt->x);
	lwy = bsr(wt->y);

	divconst_setdenom_intr(&divstate[0][0],1);
	divconst_setdenom_intr(&divstate[1][0],rt->x);
	divconst_setdenom_intr(&divstate[2][0],rt->y);
	divconst_setdenom_intr(&divstate[3][0],rt->x*rt->y);
	nxm1 = wt->x-1; nym1 = wt->y-1;
	for(y=nym1,yy=nym1*rt->y;y>=0;y--,yy-=rt->y)
	{
		ry = (yy>>lwy); ryf = ((yy+rt->y)&nym1); ucptr2 = (unsigned char *)(ry*rt->p + rt->f);
		lptr = (int32_t *)(y*wt->p + wt->f);
		for(x=nxm1,xx=nxm1*rt->x;x>=0;x--,xx-=rt->x)
		{
			rx = (xx>>lwx); rxf = ((xx+rt->x)&nxm1); ucptr = &ucptr2[rx<<2];
			for(i=0;i<4;i++) col0[i] = ucptr[i];
			ds = 0;
			if (rxf < rt->x) { ds = 1; for(i=0;i<4;i++) col0[i] = (ucptr[i+4]-col0[i])*rxf + col0[i]*rt->x; }
			if (ryf < rt->y)
			{
				ds += 2;
				for(i=0;i<4;i++) col1[i] = ucptr[rt->p+i];
				if (rxf < rt->x) { for(i=0;i<4;i++) col1[i] = (ucptr[rt->p+4+i]-col1[i])*rxf + col1[i]*rt->x; }
				for(i=0;i<4;i++) col0[i] = (col1[i]-col0[i])*ryf + col0[i]*rt->y;
			}
			for(i=0,col2=0;i<4;i++) col2 += (divconst(&divstate[ds][0],col0[i])<<(i<<3));
			lptr[x] = col2;
		}
	}
}

void fixtex4grou_intr (tiltyp *tt)
{
	memcpy((void *)(tt->p*tt->y + tt->f),(void *)tt->f,tt->p+4);
}

void kpzload4grou_intr (const char *filnam, tiltyp *tt, float shsc, int flags)
{
	tiltyp pow2t;
	char *buf;
	long x, y, nx, ny, lnx, lny, leng, xorval;
	int32_t *lptr;

	tt->f = 0; tt->lowermip = 0;

	if (!kzopen(filnam)) return;
	leng = kzfilelength();
	buf = (char *)malloc(leng+4); if (!buf) { kzclose(); return; }
	kzread(buf,leng);
	kzclose();

	kpgetdim(buf,leng,(int *)&tt->x,(int *)&tt->y);

	if (tt->x <= 1) lnx = 0; else lnx = bsr(tt->x-1)+1;
	if (tt->y <= 1) lny = 0; else lny = bsr(tt->y-1)+1;
	nx = (1<<lnx); ny = (1<<lny);

	tt->p = (nx<<2); tt->f = (intptr_t)malloc((ny+1)*tt->p + 4);
	if (!tt->f) { free(buf); return; }
	if (kprender(buf,leng,tt->f,tt->p,nx,ny,0,0) < 0)
	{ free(buf); free((void *)tt->f); tt->f = 0; return; }
	free(buf);

	if ((tt->x != nx) || (tt->y != ny))
	{
		pow2t.f = tt->f; pow2t.p = tt->p; pow2t.x = nx; pow2t.y = ny;
		scaletex_boxsum_intr((tiltyp *)tt,&pow2t);
		tt->x = nx; tt->y = ny;
	}

	if (flags&3)
	{
		xorval = 0;
		if (flags&1) xorval ^= 0x80000000;
		if (flags&2) xorval ^= 0xff000000;
		for(y=tt->y-1;y>=0;y--)
		{
			lptr = (int32_t *)(y*tt->p + tt->f);
			for(x=tt->x-1;x>=0;x--) lptr[x] ^= xorval;
		}
	}

	fixtex4grou_intr(tt);
	tt->shsc = shsc;
}

void CleanTiles(){
}
void loadpic_raw(tile_t *tpic, char* rootpath, int gal_idx) {
    long i, j, x, y, filnum, tilenum, loctile0, loctile1;
    short *sxsiz, *sysiz;
    unsigned char *uptr;
    char tbuf[MAX_PATH*2];

    if (gal_idx < 0 || gal_idx >= 16) return;

    gallery* gal = &g_gals[gal_idx];
    tiltyp *pic = &tpic->tt;

    if (pic->f && pic->f != (intptr_t)nullpic) {
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
            pic->f = (intptr_t)nullpic;
            pic->x = 4;
            pic->y = 4;
            pic->p = (pic->x<<2);
            pic->lowermip = 0;
            return;
        }

        filnum = 0;
        do {
            sprintf(tbuf, "%sTILES%03d.ART", rootpath, filnum);
            if (!kzopen(tbuf)) {
                filnum = -1;
                break;
            }

            kzread(tbuf,16);
            if (*(int32_t *)&tbuf[0] != 1) { filnum = -1; break; }
            loctile0 = *(int32_t *)&tbuf[8];
            loctile1 = (*(int32_t *)&tbuf[12])-loctile0+1;
            i = tilenum-loctile0;
            if ((unsigned)i < (unsigned)loctile1) { tilenum = i; break; }
            filnum++;
            kzclose();
        } while (1);

        if (filnum >= 0) {
            sxsiz = (short *)_alloca(loctile1<<2);
            sysiz = &sxsiz[loctile1];
            kzread(sxsiz,loctile1<<2);
            kzseek(kztell() + (loctile1<<2), SEEK_SET);

            for(i=0,j=16+(loctile1<<3);i<tilenum;i++) j += ((long)sxsiz[i])*((long)sysiz[i]);

            kzseek(j,SEEK_SET);

            pic->p = (pic->x<<2);
            pic->f = (intptr_t)malloc((pic->y+1)*pic->p+4);

            memset((void*)pic->f, 0, (pic->y+1)*pic->p+4);

            for(x=0;x<pic->x;x++) {
                uptr = (unsigned char *)_alloca(pic->y);
                kzread(uptr,pic->y);

                for(y=0;y<pic->y;y++) {
                    int32_t *pixel_ptr = (int32_t*)(pic->f + y*pic->p + (x<<2));
                    *pixel_ptr = *(int32_t *)&gal->globalpal[(long)uptr[y]][0];
                }
            }
            kzclose();

            fixtex4grou_intr((tiltyp *)pic);
            pic->lowermip = 0;
        }
    } else {
        tiltyp gtt;
        kpzload4grou_intr(tbuf,&gtt,1.0,2);
        pic->f = gtt.f; pic->p = gtt.p; pic->x = gtt.x; pic->y = gtt.y; pic->lowermip = gtt.lowermip;
    }

    if (!pic->f) {
        pic->f = (intptr_t)nullpic;
        pic->x = 4;
        pic->y = 4;
        pic->p = (pic->x<<2);
        pic->lowermip = 0;
    }
}

void loadpic(tile_t *tpic, char* rootpath, int gal_idx) {

    long i, j, x, y, filnum, tilenum, loctile0, loctile1, lnx, lny, nx, ny;
    short *sxsiz, *sysiz;
    unsigned char *uptr;
    char tbuf[MAX_PATH*2];

	if (gal_idx < 0 || gal_idx >= 16) return;

	gallery* gal = &g_gals[gal_idx];
	tiltyp *pic = &tpic->tt;

    if (pic->f && pic->f != (intptr_t)nullpic) {
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
            pic->f = (intptr_t)nullpic;
            pic->x = 4;
            pic->y = 4;
            pic->p = (pic->x<<2);
            pic->lowermip = 0;
            return;
        }

        if (pic->x <= 1) lnx = 0; else lnx = bsr(pic->x-1)+1;
        if (pic->y <= 1) lny = 0; else lny = bsr(pic->y-1)+1;
        nx = (1<<lnx); ny = (1<<lny);

        filnum = 0;
        do {
            sprintf(tbuf, "%sTILES%03d.ART", rootpath, filnum);
            if (!kzopen(tbuf)) {
                filnum = -1;
                break;
            }

            kzread(tbuf,16);
            if (*(int32_t *)&tbuf[0] != 1) { filnum = -1; break; }
            loctile0 = *(int32_t *)&tbuf[8];
            loctile1 = (*(int32_t *)&tbuf[12])-loctile0+1;
            i = tilenum-loctile0;
            if ((unsigned)i < (unsigned)loctile1) { tilenum = i; break; }
            filnum++;
            kzclose();
        } while (1);

        if (filnum >= 0) {
            sxsiz = (short *)_alloca(loctile1<<2);
            sysiz = &sxsiz[loctile1];
            kzread(sxsiz,loctile1<<2);
            kzseek(kztell() + (loctile1<<2), SEEK_SET);

            for(i=0,j=16+(loctile1<<3);i<tilenum;i++) j += ((long)sxsiz[i])*((long)sysiz[i]);

            kzseek(j,SEEK_SET);
            uptr = (unsigned char *)_alloca(pic->y);
            pic->p = (nx<<2);
            pic->f = (intptr_t)malloc((ny+1)*pic->p+4);

            for(x=0;x<pic->x;x++) {
                kzread(uptr,pic->y);
                intptr_t pi = pic->f + (x<<2);
                for(y=0;y<pic->y;y++,pi+=pic->p)
                    *(int32_t *)pi = *(int32_t *)&gal->globalpal[(long)uptr[y]][0];
            }
            kzclose();

            if ((pic->x != nx) || (pic->y != ny)) {
                tiltyp pow2t;
                pow2t.f = pic->f; pow2t.p = pic->p; pow2t.x = nx; pow2t.y = ny;
                scaletex_boxsum_intr((tiltyp *)pic,&pow2t);
                pic->x = nx; pic->y = ny;
            }

            fixtex4grou_intr((tiltyp *)pic);
            pic->lowermip = 0;
        }
    } else {
        tiltyp gtt;
        kpzload4grou_intr(tbuf,&gtt,1.0,2);
        pic->f = gtt.f; pic->p = gtt.p; pic->x = gtt.x; pic->y = gtt.y; pic->lowermip = gtt.lowermip;
    }

    if (!pic->f) {
        pic->f = (intptr_t)nullpic;
        pic->x = 4;
        pic->y = 4;
        pic->p = (pic->x<<2);
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
	int i, j;
	unsigned char uch;

	if (gotpal) return;
	setgammlut(1.0);

	sprintf(tbuf, "%spalette.dat", basepath);

	if (kzopen(tbuf)) {
		kzread(globalpal, 768);
		*(int32_t *)&globalpal[256][0] = 0^0xff000000;
		for(i=255-1;i>=0;i--) {
			globalpal[i][3] = 255;
			globalpal[i][2] = gammlut[globalpal[0][i*3+2]<<2];
			globalpal[i][1] = gammlut[globalpal[0][i*3+1]<<2];
			globalpal[i][0] = gammlut[globalpal[0][i*3  ]<<2];
			uch = globalpal[i][0]; globalpal[i][0] = globalpal[i][2]; globalpal[i][2] = uch;
		}
		globalpal[255][3] = 0;
		kzclose();
		gotpal = 1;
	}
}
void galfreetextures(int gal_idx) {
	if (gal_idx < 0 || gal_idx >= 16) return;

	gallery* gal = &g_gals[gal_idx];

	if (gal->gtile) {
		for (int i = 0; i < gal->gnumtiles; i++) {
			if (gal->gtile[i].tt.f && gal->gtile[i].tt.f != (intptr_t)nullpic) {
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
	if (gal->picanm_data) {
		free(gal->picanm_data);
		gal->picanm_data = NULL;
	}
}
int loadgal(int gal_idx, const char* path) {
    if (gal_idx < 0 || gal_idx >= 16) return 0;

    gallery* gal = &g_gals[gal_idx];
    double t0 = GetTime();

    galfreetextures(gal_idx);
    if (gal->gtile) {
        free(gal->gtile);
        gal->gtile = NULL;
    }
    if (gal->sizex) {
        free(gal->sizex);
        gal->sizex = NULL;
    }
    if (gal->sizey) {
        free(gal->sizey);
        gal->sizey = NULL;
    }

    strcpy(gal->curmappath, path);
    int j = 0;
    for(int i = 0; gal->curmappath[i]; i++) {
        if ((gal->curmappath[i] == '/') || (gal->curmappath[i] == '\\')) j = i + 1;
    }
    gal->curmappath[j] = 0;

	LoadGalleryPal(gal_idx, gal->curmappath);
    gal->gotpal = 1;

    int arttiles = 0;
    uint16_t *tilesizx = NULL, *tilesizy = NULL;
    picanm_t *picanm = NULL;
    char tbuf[MAX_PATH*2];

    for(int filnum = 0; ; filnum++) {
        sprintf(tbuf, "%sTILES%03d.ART", gal->curmappath, filnum);
        if (!kzopen(tbuf)) break;

        kzread(tbuf, 16);
        if (*(int32_t*)&tbuf[0] != 1) {
            kzclose();
            break;
        }

        int loctile0 = *(int32_t*)&tbuf[8];
        int loctile1 = (*(int32_t*)&tbuf[12]) + 1;

        if ((loctile0 < 0) || (loctile1 <= arttiles) || (loctile0 >= loctile1)) {
            kzclose();
            continue;
        }

        int old_arttiles = arttiles;
        arttiles = loctile1;

        tilesizx = (uint16_t*)realloc(tilesizx, arttiles * sizeof(uint16_t));
        tilesizy = (uint16_t*)realloc(tilesizy, arttiles * sizeof(uint16_t));
        picanm = (picanm_t*)realloc(picanm, arttiles * sizeof(picanm_t));

        for(int i = old_arttiles; i < arttiles; i++) {
            tilesizx[i] = 0;
            tilesizy[i] = 0;
            picanm[i].asint = 0;
        }

        kzread(&tilesizx[loctile0], (loctile1 - loctile0) * sizeof(uint16_t));
        kzread(&tilesizy[loctile0], (loctile1 - loctile0) * sizeof(uint16_t));
        kzread(&picanm[loctile0], (loctile1 - loctile0) * sizeof(picanm_t));

        kzclose();
    }

    if (!arttiles) {
        tilesizx = (uint16_t*)malloc(sizeof(uint16_t));
        tilesizy = (uint16_t*)malloc(sizeof(uint16_t));
        picanm = (picanm_t*)malloc(sizeof(picanm_t));
        tilesizx[0] = tilesizy[0] = 2;
        picanm[0].asint = 0;
        arttiles = 1;
    }

    gal->sizex = tilesizx;
    gal->sizey = tilesizy;
    gal->picanm_data = picanm;
    gal->gnumtiles = arttiles;
    gal->gmaltiles = arttiles;

    gal->gtile = (tile_t*)malloc(arttiles * sizeof(tile_t));
    memset(gal->gtile, 0, arttiles * sizeof(tile_t));

    for(int i = 0; i < arttiles; i++) {
        sprintf(gal->gtile[i].filnam, "tiles000.art|%d", i);
        gal->gtile[i].tt.f = 0;
    }

    for(int i = 0; i < arttiles; i++) {
        if (tilesizx[i] > 0 && tilesizy[i] > 0) {
            loadpic_raw(&gal->gtile[i], gal->curmappath, gal_idx);
        }
    }

    printf("loadgal(%d): %.3fs for %d tiles\n", gal_idx, GetTime() - t0, arttiles);

    return 1;
}
