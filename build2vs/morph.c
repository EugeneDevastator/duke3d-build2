#if 0 //--------------------------------------------------------------------------------------------
Example library usage:

#define USEMORPH
#if USEMORPH
extern void morph_init (long, long, long, void (*)(cam_t *));
extern void morph_sleepuntilretrace ();
extern void morph_drawframe (tiltyp *, long, point3d *, point3d *, point3d *, point3d *, float, float, float);
extern void morph_uninit ();
#endif

	//User function must draw to surface: cc->c,cc->z
	//User function must write: cc->p,r,d,f,hx,hy,hz
void drawframe (cam_t *cc)
{
	*(long *)(cc->c.f) = 0xffffff; //draw your frame here!

		//User function must write these:
	cc->p = gps->ipos; cc->r = gps->irig; cc->d = gps->idow; cc->f = gps->ifor;
	cc->hx = gps->ghx; cc->hy = gps->ghy; cc->hz = gps->ghz;
}

#if USEMORPH
		//put inside doframe, before startdirectdraw
	static int morphinited = 0;
	if (!morphinited) { morphinited = 1; morph_init(xres,yres,drawframe); }
	morph_sleepuntilretrace();
#endif

#if USEMORPH
	//clear z!
	morph_drawframe((tiltyp *)&gdd,(long)zbmem,&ipos,&irig,&idow,&ifor,ghx,ghy,ghz);
#else
	cam.f = gdd.f;
	...
	drawframe(&cam);
#endif

#endif //-------------------------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>             //For refresh locking
#define DIRECTDRAW_VERSION 0x0700 //For refresh locking
#include <ddraw.h>                //For refresh locking
extern LPDIRECTDRAW7 lpdd;         //For refresh locking

#define MAXXDIM 2048
#define MAXYDIM 1536

#define USEREVMAP 1 //0=forward mapping (dots), 1=reverse mapping:faster but more artifacts
#define ZBUFTYPE 1 //0=float,1=long
#define LMORPHBOXSIZ 3 //3=default
#define MORPHBOXSIZ (1<<LMORPHBOXSIZ)
#define NUMPAGES 3

typedef struct tiltyp { long f, p, x, y, z; float shsc; tiltyp *lowermip; } tiltyp; //shsc=suggested height scale //Note: morph has no use for z,shsc
typedef struct { float x, y, z; } point3d;
typedef struct { tiltyp c, z; point3d p, r, d, f; float hx, hy, hz; } cam_t;

static volatile long numslowframes = 0, gotaframe = 0;
static tiltyp ocbuf[NUMPAGES], ozbuf[NUMPAGES];
static point3d oipos[NUMPAGES], oirig[NUMPAGES], oidow[NUMPAGES], oifor[NUMPAGES];
static float oghx[NUMPAGES], oghy[NUMPAGES], oghz[NUMPAGES];

unsigned long refreshz = 0;
static long totxres = 1024, totyres = 768;

	//if (a < 0) return(0); else if (a > b) return(b); else return(a);
static _inline long lbound0 (long a, long b) //b MUST be >= 0
{
	if ((unsigned long)a <= b) return(a);
	return((~(a>>31))&b);
}

static void (*morph_userframe)(cam_t *);

#if USEREVMAP
typedef struct { float x, y; } point2d;
typedef struct { long x, y; } lpoint2d;
static point2d boxf[(MAXXDIM/MORPHBOXSIZ+1)*(MAXYDIM/MORPHBOXSIZ+1)];
static lpoint2d boxr[(MAXXDIM/MORPHBOXSIZ+1)*(MAXYDIM/MORPHBOXSIZ+1)];
static lpoint2d boxp[MAXXDIM], boxi[MAXXDIM];
static long *readylookup[MAXYDIM];
static point3d verx[MAXXDIM/MORPHBOXSIZ];
#endif
void morph_drawframe (tiltyp *tt, long nzf, point3d *np, point3d *nr, point3d *nd, point3d *nf, float nhx, float nhy, float nhz)
{
	point3d npos, nrig, ndow, nfor, oldp, oldr, oldd, oldf;
	float f, d, dis, fx, fy, fz, gx, gy, ohx, ohy, ohz;
	long i, j, bxp, x, y, p, sx, sy, col, *colptr, gddxm1, gddym1, ncf, ocf, ozf, zboff;
#if (ZBUFTYPE == 0)
	float *disptr;
#else
	float dismul;
	long *disptr;
	dismul = 1.0/((float)(1<<28));
#endif

	if (!gotaframe) return;
	i = numslowframes-1; if (i < 0) i += NUMPAGES;
	ocf = ocbuf[i].f; ozf = ozbuf[i].f;
	oldp = oipos[i]; oldr = oirig[i]; oldd = oidow[i]; oldf = oifor[i];
	ohx = oghx[i]; ohy = oghy[i]; ohz = oghz[i];
	ncf = tt->f;

	nrig.x = nr->x*oldr.x + nr->y*oldr.y + nr->z*oldr.z;
	nrig.y = nd->x*oldr.x + nd->y*oldr.y + nd->z*oldr.z;
	nrig.z = nf->x*oldr.x + nf->y*oldr.y + nf->z*oldr.z;
	ndow.x = nr->x*oldd.x + nr->y*oldd.y + nr->z*oldd.z;
	ndow.y = nd->x*oldd.x + nd->y*oldd.y + nd->z*oldd.z;
	ndow.z = nf->x*oldd.x + nf->y*oldd.y + nf->z*oldd.z;
	nfor.x = nr->x*oldf.x + nr->y*oldf.y + nr->z*oldf.z;
	nfor.y = nd->x*oldf.x + nd->y*oldf.y + nd->z*oldf.z;
	nfor.z = nf->x*oldf.x + nf->y*oldf.y + nf->z*oldf.z;
	fx = oldp.x-np->x; fy = oldp.y-np->y; fz = oldp.z-np->z;
	npos.x = nr->x*fx + nr->y*fy + nr->z*fz;
	npos.y = nd->x*fx + nd->y*fy + nd->z*fz;
	npos.z = nf->x*fx + nf->y*fy + nf->z*fz;
	nfor.x = -nrig.x*ohx - ndow.x*ohy + nfor.x*ohz;
	nfor.y = -nrig.y*ohx - ndow.y*ohy + nfor.y*ohz;
	nfor.z = -nrig.z*ohx - ndow.z*ohy + nfor.z*ohz;
	gddxm1 = tt->x-1;
	gddym1 = tt->y-1;
	i = 0;
#if USEREVMAP
	bxp = tt->x/MORPHBOXSIZ+1;
	for(y=0;y<tt->y;y+=MORPHBOXSIZ)
#else
	zboff = nzf-tt->f;
	for(y=0;y<tt->y;y++)
#endif
	{
		colptr = (long *)(y*tt->p + ocf);
#if (ZBUFTYPE == 0)
		disptr = (float *)(y*tt->p + ozf);
#else
		disptr = (long *)(y*tt->p + ozf);
#endif
#if USEREVMAP
		i = (y/MORPHBOXSIZ)*bxp;
		for(x=0;x<tt->x;x+=MORPHBOXSIZ,i++)
#else
		for(x=0;x<tt->x;x++,i++)
#endif
		{
#if (ZBUFTYPE == 0)
			d = (nrig.z*(float)x + nfor.z)*disptr[x] + npos.z; if (*(long *)&d <= 0.0) continue;
#else
			d = (nrig.z*(float)x + nfor.z)*((float)disptr[x])*dismul + npos.z; if (*(long *)&d <= 0.0) continue;
#endif
#if 0
			f = nhz/d;
#else
			_asm
			{
				rcpss xmm0, d
				mulss xmm0, nhz
				movss f, xmm0
			}
#endif
#if (ZBUFTYPE == 0)
			fx = ((nrig.x*(float)x + nfor.x)*disptr[x] + npos.x)*f + nhx;
			fy = ((nrig.y*(float)x + nfor.y)*disptr[x] + npos.y)*f + nhy;
#else
			fx = ((nrig.x*(float)x + nfor.x)*((float)disptr[x])*dismul + npos.x)*f + nhx;
			fy = ((nrig.y*(float)x + nfor.y)*((float)disptr[x])*dismul + npos.y)*f + nhy;
#endif
#if (USEREVMAP == 0)
			sx = (long)fx; if ((unsigned long)sx >= (unsigned long)gddxm1) continue;
			sy = (long)fy; if ((unsigned long)sy >= (unsigned long)gddym1) continue;
			col = colptr[x]; p = sy*tt->p + (sx<<2) + tt->f;
			if (*(long *)&d < *(long *)(zboff+p  )) { *(long *)(zboff+p  ) = *(long *)&d; *(long *)(p  ) = col; }
			if (*(long *)&d < *(long *)(zboff+p+4)) { *(long *)(zboff+p+4) = *(long *)&d; *(long *)(p+4) = col; }
			p += tt->p;
			if (*(long *)&d < *(long *)(zboff+p  )) { *(long *)(zboff+p  ) = *(long *)&d; *(long *)(p  ) = col; }
			if (*(long *)&d < *(long *)(zboff+p+4)) { *(long *)(zboff+p+4) = *(long *)&d; *(long *)(p+4) = col; }
#else
			boxf[i].x = fx;
			boxf[i].y = fy;
#endif
		}
#if USEREVMAP
		nfor.x += ndow.x*MORPHBOXSIZ; nfor.y += ndow.y*MORPHBOXSIZ; nfor.z += ndow.z*MORPHBOXSIZ;
#else
		nfor.x += ndow.x; nfor.y += ndow.y; nfor.z += ndow.z;
#endif
	}

#if USEREVMAP

		//Convert forward-mapped (bilinear) grid lut
		//     to reverse mapped (bilinear) grid lut
	{
	float x0, y0, x1, y1, xrat, yrat, vy, nsx, nsy;
	long sx, sxe, sy, sye, ifx, ify, igx, igy, *lptr, xx;
	lpoint2d *lxptr, *lyptr;

		//Mark unwritten parts
	//for(sy=0,i=0;sy<tt->y;sy+=MORPHBOXSIZ)
	//   for(sx=0;sx<tt->x;sx+=MORPHBOXSIZ,i++) boxr[i].y = 0; //0x80000000;

	for(x=0;x<(tt->x>>LMORPHBOXSIZ);x++) verx[x].z = -1;

	for(y=0;y<tt->y;y+=MORPHBOXSIZ)
		for(x=MORPHBOXSIZ;x<tt->x;x+=MORPHBOXSIZ)
		{
			i = (y>>LMORPHBOXSIZ)*bxp+(x>>LMORPHBOXSIZ);
			x0 = boxf[i-1].x; x1 = boxf[i].x;
			y0 = boxf[i-1].y; y1 = boxf[i].y;

			sx  = x0/(float)MORPHBOXSIZ; sx  = lbound0(sx ,tt->x>>LMORPHBOXSIZ);
			sxe = x1/(float)MORPHBOXSIZ; sxe = lbound0(sxe,tt->x>>LMORPHBOXSIZ);
			if (sx != sxe)
			{
				f = 1.0/(x1-x0);
				do
				{
					if (sx < sxe) sx++;

					xrat = (sx*MORPHBOXSIZ - x0)*f;

					vy = (y1-y0)*xrat + y0;
					if (vy > verx[sx].z)
					{
						if (verx[sx].z < 0) verx[sx].z = vy;
						sye = (long)(min(vy,tt->y) / (float)MORPHBOXSIZ);
						sy  = (long)(verx[sx].z    / (float)MORPHBOXSIZ);
						nsx = (xrat-1)*MORPHBOXSIZ + x;
						nsy = y;
						while (sy < sye)
						{
							sy++;
							yrat = (sy*MORPHBOXSIZ - verx[sx].z) / (vy - verx[sx].z);
							i = sy*bxp+sx;
							boxr[i].x = (long)(((nsx - verx[sx].x)*yrat + nsx)*65536.0);
							boxr[i].y = (long)(((nsy - verx[sx].y)*yrat + nsy)*65536.0);
							boxr[i].x = lbound0(boxr[i].x,(tt->x<<16)-1);
							boxr[i].y = lbound0(boxr[i].y,(tt->y<<16)-1);
						}
						verx[sx].x = nsx;
						verx[sx].y = nsy;
						verx[sx].z = vy;
					}

					if (sx > sxe) sx--;
				} while (sx != sxe);
			}
		}

#if 0
		//Original brute-force loop
	for(sy=0;sy<tt->y;sy++)
	{
		lyptr = &boxr[(sy>>LBOXSIZ)*bxp];
		lptr = (long *)(sy*tt->p + tt->f);
		for(sx=0;sx<tt->x;sx++)
		{
			lxptr = &lyptr[(sx>>LBOXSIZ)];
			i = (sx&(BOXSIZ-1));
			ifx = (((lxptr[    1].x-lxptr[  0].x)*i)>>LBOXSIZ) + lxptr[  0].x;
			ify = (((lxptr[    1].y-lxptr[  0].y)*i)>>LBOXSIZ) + lxptr[  0].y;
			igx = (((lxptr[bxp+1].x-lxptr[bxp].x)*i)>>LBOXSIZ) + lxptr[bxp].x;
			igy = (((lxptr[bxp+1].y-lxptr[bxp].y)*i)>>LBOXSIZ) + lxptr[bxp].y;
			i = (sy&(BOXSIZ-1));
			x = ((((igx-ifx)*i)>>LBOXSIZ) + ifx)>>16;
			y = ((((igy-ify)*i)>>LBOXSIZ) + ify)>>16;
			lptr[sx] = *(long *)(y*tt->p + (x<<2) + ocf);
		}
	}
#else
	for(i=0,j=ocf;i<tt->y;i++) { readylookup[i] = (long *)j; j += tt->p; }
	for(sy=0;sy<tt->y;sy++)
	{
		if (!(sy&(MORPHBOXSIZ-1)))
		{
			lyptr = &boxr[(sy>>LMORPHBOXSIZ)*bxp];
			if (!sy)
			{
				x = lyptr[0].x; y = lyptr[0].y; lxptr = lyptr+1;
				for(xx=0;xx<tt->x;xx+=MORPHBOXSIZ)
				{
					ifx = (lxptr[0].x-x)>>LMORPHBOXSIZ;
					ify = (lxptr[0].y-y)>>LMORPHBOXSIZ; lxptr++;
					for(sx=xx,sxe=min(xx+MORPHBOXSIZ,tt->x);sx<sxe;sx++)
					{
						boxp[sx].x = x; x += ifx;
						boxp[sx].y = y; y += ify;
					}
				}
			}
			lyptr += bxp;
			x = lyptr[0].x; y = lyptr[0].y; lxptr = lyptr+1;
			for(xx=0;xx<tt->x;xx+=MORPHBOXSIZ)
			{
				ifx = (lxptr[0].x-x)>>LMORPHBOXSIZ;
				ify = (lxptr[0].y-y)>>LMORPHBOXSIZ; lxptr++;
				for(sx=xx,sxe=min(xx+MORPHBOXSIZ,tt->x);sx<sxe;sx++)
				{
					boxi[sx].x = ((x - boxp[sx].x)>>LMORPHBOXSIZ); x += ifx;
					boxi[sx].y = ((y - boxp[sx].y)>>LMORPHBOXSIZ); y += ify;
				}
			}
		}

		lptr = (long *)(sy*tt->p + tt->f);
		for(sx=0;sx<tt->x;sx++)
		{
			lptr[sx] = readylookup[boxp[sx].y>>16][boxp[sx].x>>16];
			boxp[sx].x += boxi[sx].x; boxp[sx].y += boxi[sx].y;
		}
	}
#endif
	}
#endif
}

//------------------------------------
#include <process.h>
static HANDLE drawslowhand;
static volatile long drawslowstate = -1;
static void __cdecl drawslowfunc (void *_)
{
	cam_t cm;
	long i;

	while (drawslowstate <= 0)
	{
		i = numslowframes;
		cm.c = ocbuf[i];
		cm.z = ozbuf[i];
		morph_userframe(&cm);
		oipos[i] = cm.p;
		oirig[i] = cm.r;
		oidow[i] = cm.d;
		oifor[i] = cm.f;
		oghx[i] = cm.hx;
		oghy[i] = cm.hy;
		oghz[i] = cm.hz;
		i++; if (i >= NUMPAGES) i = 0;
		numslowframes = i; gotaframe = 1;
	}
	drawslowstate = 2;
	_endthread();
}
//------------------------------------

void morph_uninit ()
{
	long i;

	timeEndPeriod(1);
	for(drawslowstate=1;drawslowstate==1;Sleep(1));

	for(i=NUMPAGES-1;i>=0;i--)
	{
		if (ozbuf[i].f) { free((void *)ozbuf[i].f); ozbuf[i].f = 0; }
		if (ocbuf[i].f) { free((void *)ocbuf[i].f); ocbuf[i].f = 0; }
	}
}

void morph_init (long xsiz, long ysiz, long dafullsc, void (*userframe)(cam_t *))
{
	long i;

	morph_userframe = userframe;

	for(i=0;i<NUMPAGES;i++)
	{
		ocbuf[i].x = xsiz; ocbuf[i].y = ysiz; ocbuf[i].p = (xsiz<<2); ocbuf[i].f = (long)malloc(ocbuf[i].p*ocbuf[i].y);
		ozbuf[i].x = xsiz; ozbuf[i].y = ysiz; ozbuf[i].p = (xsiz<<2); ozbuf[i].f = (long)malloc(ozbuf[i].p*ozbuf[i].y);
	}

	timeBeginPeriod(1);
	if (drawslowstate < 0) { drawslowstate = 0; _beginthread(drawslowfunc,0,0); }

	if (!refreshz)
	{
		lpdd->GetMonitorFrequency(&refreshz);
		if (refreshz < 60) refreshz = 60; //60,72,75,85,100,...
	}

	if (!dafullsc)
	{
		totxres = GetSystemMetrics(SM_CXSCREEN);
		totyres = GetSystemMetrics(SM_CYSCREEN);
	}
	else { totxres = xsiz; totyres = ysiz; }
}

void morph_sleepuntilretrace ()
{
	unsigned long u;
	if (lpdd->GetScanLine(&u) == DD_OK) //DD_OK,DDERR_VERTICALBLANKINPROGRESS
		Sleep(min(max((totyres-18-u)*1000/(refreshz*totyres),0),20));
}
