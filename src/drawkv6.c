#if 0
objs=drawkv6.obj cwinmain.obj
opts=/c /TP /Ox /GFy /MT /nologo

!if "$(AS)" == "ml64"
objs=$(objs) drawkv6_64.obj
!endif

drawkv6.exe: $(objs); link $(objs) ole32.lib user32.lib gdi32.lib
	del drawkv6.obj
	del cwinmain.obj
drawkv6.obj:  drawkv6.c cwinmain.h ; cl $(opts) drawkv6.c /DSTANDALONE=1
drawkv6_64.obj: drawkv6_64.asm     ; ml64 /c drawkv6_64.asm
cwinmain.obj: cwinmain.c cwinmain.h; cl $(opts) cwinmain.c
!if 0
#endif

#include <basetsd.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#include <minmax.h>
#define MAX_PATH 260

#if defined(_WIN64)

#include <intrin.h>
#define rcpss(f) _mm_cvtss_f32(_mm_rcp_ss(_mm_set_ss(f)))
#define cvttss2si(f) _mm_cvtt_ss2si(_mm_set_ss(f))

#else

static __forceinline float rcpss (float f) { _asm rcpss xmm0, f _asm movd f, xmm0 _asm fld dword ptr f }
static __forceinline int cvttss2si (float f) { _asm cvttss2si eax, f }

#endif

#if defined(_WIN32)

#define LL(l) l##i64
#define ALIGN(i) __declspec(align(i))

#else

#define LL(l) l##ll
#define __int64 long long
#define ALIGN(i) __attribute__((aligned(i)))
static __inline float rcpss (float f) { float32x2_t f2 = {f,1.f}; f2 = vrecpe_f32(f2); return(vget_lane_f32(f2,0)); }
#define cvttss2si(f) lroundf(f)

#endif

#ifdef USEKZ
extern int kzopen (const char *);
extern int kzread (void *, int);
extern void kzclose ();
#define FILE void
#define fopen(filnam,st) (void *)kzopen(filnam)
#define fread(ptr,leng,one,fil) kzread(ptr,leng)
#define fclose(fil) kzclose()
#else
#include <stdio.h>
#endif

typedef struct { double x, y, z; } dpoint3d;
typedef struct { float x, y, z; } point3d;
#if (STANDALONE == 0)
typedef struct { INT_PTR f, p, x, y; } tiletype;
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "cwinmain.h"
#endif

#define MAXLIGHTS 256
typedef struct { int sect; point3d p; float rgb[3]; int useshadow; } drawkv6_lightpos_t;
drawkv6_lightpos_t drawkv6_light[MAXLIGHTS];
int drawkv6_numlights = -1;
float drawkv6_ambrgb[3] = {0.75,0.75,0.75};

#define PI 3.14159265358979323

static int gcpuid[8][4], gavxsupp = 0;

typedef struct
{
	float hx[8], hy[8], hz[8], rhzup20[8];
	short wmin[8], wmax[8];
	short ighyxyx[4], igyxyx[4]; //32-bit only!
	INT_PTR ddp, ddf, ddx, ddy, zbufoff;
	point3d p, r, d, f;
} drawkv6_frame_t;

#if defined(_WIN64)
extern "C"
{
void kcpuid (int a, int *s);
int drawkv6_xform_avx2 (float *nvx, float *nvy, float *nvz, float fx, float fy, float fz, int vis, int col, drawkv6_frame_t *frame);
}
#else
#if defined(_MSC_VER)

static _inline void kcpuid (int a, int *s)
{
	_asm
	{
		push ebx
		push esi
		mov eax, a
		cpuid
		mov esi, s
		mov dword ptr [esi+0], eax
		mov dword ptr [esi+4], ebx
		mov dword ptr [esi+8], ecx
		mov dword ptr [esi+12], edx
		pop esi
		pop ebx
	}
}

#else
static void kcpuid (int a, int *s) { s[0] = 0; s[1] = 0; s[2] = 0; s[3] = 0; }
#endif
#endif

#if defined(_WIN64)

static int umulshr32 (int a, int d) { return(((unsigned __int64)(unsigned int)/*yes, this ridiculous double casting is necessary!*/a*(unsigned int)d)>>32); }
static __forceinline int colscale15 (int col, __int64 mulshorts)
{
	__m128i x0, x1;
	x0 = _mm_cvtsi32_si128(col);       //movd xmm0, i0
	x0 = _mm_unpacklo_epi8(x0,x0);     //punpcklbw xmm0, xmm0
	x0 = _mm_srli_epi16(x0,7);         //psrlw xmm0, 7
	x1 = _mm_cvtsi64_si128(mulshorts); //movq xmm1, i1
	x0 = _mm_mulhi_epu16(x0,x1);       //pmulhuw xmm0, xmm1
	x0 = _mm_packus_epi16(x0,x0);      //packuswb xmm0, xmm0
	return(_mm_cvtsi128_si32(x0));     //movd eax, xmm0
}

#else

static __forceinline int umulshr32 (int a, int d)
{
	_asm
	{
		mov eax, a
		mul d
		mov eax, edx
	}
}

static __forceinline int colscale15 (int col, __int64 mulshorts)
{
	_asm
	{
		punpcklbw mm7, col
		psrlw mm7, 7
		pmulhuw mm7, mulshorts
		packuswb mm7, mm7
		movd eax, mm7
		emms
	}
}

#endif

typedef struct { int col; unsigned short z; unsigned char vis, dir; } kv6voxtype;
typedef struct kv6data_t
{
	int leng, xsiz, ysiz, zsiz;
	float xpiv, ypiv, zpiv;
	unsigned int numvoxs;
	kv6data_t *lowermip;
	kv6voxtype *vox;      //numvoxs*sizeof(kv6voxtype)
	unsigned int *xlen;   //xsiz*sizeof(int)
	unsigned short *ylen; //xsiz*ysiz*sizeof(short)
	void *datmalptr;
} kv6data_t;
typedef struct
{
	int hashnext;
	char filnam[MAX_PATH];
	kv6data_t *ptr;
} kv6nam2ptr_t;
static kv6nam2ptr_t *kv6nam2ptr = 0;
static int kv6nam2ptr_num = 0, kv6nam2ptr_mal = 0, kv6hashead[256];
ALIGN(16) static float bcf_ptx[8], bcf_pty[8], bcf_ptz[8];
ALIGN(8) static short ig0000[4] = {0,0,0,0};

static void boundcubefillz_c (drawkv6_frame_t *frame, int n, int col)
{
	INT_PTR p;
	float f;
	int i, x, y, z, x0, y0, x1, y1;

	x0 = frame->ddx; x1 = 0; y0 = frame->ddy; y1 = 0;
	for(i=n-1;i>=0;i--)
	{
		f = 1.f/bcf_ptz[i];
		x = (int)(bcf_ptx[i]*f + frame->hx[0]+.5); if (x < x0) x0 = x; if (x > x1) x1 = x;
		y = (int)(bcf_pty[i]*f + frame->hy[0]+.5); if (y < y0) y0 = y; if (y > y1) y1 = y;
	}
	if (x0 < 0) x0 = 0; if (x1 > frame->ddx) x1 = frame->ddx; if (x1 <= x0) return;
	if (y0 < 0) y0 = 0; if (y1 > frame->ddy) y1 = frame->ddy; if (y1 <= y0) return;

#if (USEINTZ != 0)
	i = cvttss2si(bcf_ptz[0]*frame->rhzup20[0]); bcf_ptz[0] = *(float *)&i;
#endif

	p = y0*frame->ddp + frame->ddf;
	x0 <<= 2; x1 <<= 2;
	for(y=y1-y0;y;y--,p+=frame->ddp)
		for(x=x0;x<x1;x+=4)
			if (*(int *)&bcf_ptz[0] < *(int *)(p+x+frame->zbufoff))
			{
				*(int *)(p+x+frame->zbufoff) = *(int *)&bcf_ptz[0];
				*(int *)(p+x) = col;
			}
}

static unsigned char bitsum[64] =
{
	0,1,1,2,1,2,2,3,
	1,2,2,3,2,3,3,4,
	1,2,2,3,2,3,3,4,
	2,3,3,4,3,4,4,5,
	1,2,2,3,2,3,3,4,
	2,3,3,4,3,4,4,5,
	2,3,3,4,3,4,4,5,
	3,4,4,5,4,5,5,6,
};
	//the 8 bits tell which 4 or 6 of the 8 corners of the cube (x=msb,z=lsb) must be considered for bounding
static unsigned char visboundcorns[43] =
{
		0,0x0f,0xf0,   0,0x55,0x5f,0xf5,   0,
	0xaa,0xaf,0xfa,   0,   0,   0,   0,   0,
	0x33,0x3f,0xf3,   0,0x77,0x7e,0xe7,   0,
	0xbb,0xbd,0xdb,   0,   0,   0,   0,   0,
	0xcc,0xcf,0xfc,   0,0xdd,0xdb,0xbd,   0,
	0xee,0xe7,0x7e,
};

#define GOLDRAT 0.3819660112501052 //Golden Ratio: 1 - 1/((sqrt(5)+1)/2)
typedef struct
{
	float fibx[45], fiby[45];
	float azval[20], zmulk, zaddk;
	int fib[47], aztop, npoints;
	point3d *p;  //For memory version :/
	int pcur;
} equivectyp;
static equivectyp equivec;

#if defined(_WIN64)

static void fcossin (float a, float *c, float *s) { (*c) = cos(a); (*s) = sin(a); }

#else

static _inline void fcossin (float a, float *c, float *s)
{
	_asm
	{
		fld a
		fsincos
		mov eax, c
		fstp dword ptr [eax]
		mov eax, s
		fstp dword ptr [eax]
	}
}

#endif

static void equiind2vec (int i, float *x, float *y, float *z)
{
	float r;
	(*z) = (float)i*equivec.zmulk + equivec.zaddk; r = sqrt(1.f - (*z)*(*z));
	fcossin((float)i*(GOLDRAT*PI*2),x,y); (*x) *= r; (*y) *= r;
}

#pragma warning(disable:4731)

static point3d univec[256];
ALIGN(16) static float bcfaddx[43][8], bcfaddy[43][8], bcfaddz[43][8];
void drawkv6 (drawkv6_frame_t *frame, kv6data_t *kv6,
				  float posx, float posy, float posz,
				  float rigx, float rigy, float rigz,
				  float dowx, float dowy, float dowz,
				  float forx, float fory, float forz, int curcol, float shadefac)
{
	ALIGN(16) float bcf_nix[4], bcf_niy[4], bcf_niz[4], bcf_pr0[4], bcf_pr1[4];
	ALIGN(16) unsigned short rgbmul[256][4];
#if !defined(_WIN64)
	point3d nipos, nirig, nidow, nifor;
#endif
	point3d pr2, pra[8], mat[4], opos;
	float f, f2, fx, fy, fz, minprz;
	int i, k, x, y, z, col, vis0, vis1, vis, grdx, grdy, grdz;
	kv6voxtype *kvoxptr, *kvoxptre;
	unsigned short *usptr;
	unsigned char *ucptr;

	if (!kv6) return;

		//Quick & dirty estimation of distance
	f2 = (posx-frame->p.x)*frame->f.x + (posy-frame->p.y)*frame->f.y + (posz-frame->p.z)*frame->f.z; //perp. distance
	f2 /= sqrt(rigx*rigx + rigy*rigy + rigz*rigz + dowx*dowx + dowy*dowy + dowz*dowz + forx*forx + fory*fory + forz*forz); //voxel size in world units
	f2 *= frame->rhzup20[0]/524288.0; //MIP FACTOR; FIXFIXFIX:need ability to change externally!
	for(f=1.0;(kv6->lowermip) && (f2 >= f);kv6=kv6->lowermip,f+=f);

	opos.x = posx; opos.y = posy; opos.z = posz;

	rigx *= f; rigy *= f; rigz *= f;
	dowx *= f; dowy *= f; dowz *= f;
	forx *= f; fory *= f; forz *= f;
		//do pivot offset here
	posx -= (kv6->xpiv*rigx + kv6->zpiv*dowx + kv6->ypiv*forx);
	posy -= (kv6->xpiv*rigy + kv6->zpiv*dowy + kv6->ypiv*fory);
	posz -= (kv6->xpiv*rigz + kv6->zpiv*dowz + kv6->ypiv*forz);


	mat[3].x = (float)posx; mat[3].y = (float)posy; mat[3].z = (float)posz;
	mat[0].x = (float)rigx; mat[0].y = (float)rigy; mat[0].z = (float)rigz;
	mat[1].x = (float)dowx; mat[1].y = (float)dowy; mat[1].z = (float)dowz;
	mat[2].x = (float)forx; mat[2].y = (float)fory; mat[2].z = (float)forz;

		//transform world to screen coords
	for(i=0;i<4;i++)
	{
		fx = mat[i].x; fy = mat[i].y; fz = mat[i].z;
		if (i == 3) { fx -= frame->p.x; fy -= frame->p.y; fz -= frame->p.z; }
		mat[i].x = fx*frame->r.x + fy*frame->r.y + fz*frame->r.z;
		mat[i].y = fx*frame->d.x + fy*frame->d.y + fz*frame->d.z;
		mat[i].z = fx*frame->f.x + fy*frame->f.y + fz*frame->f.z;
	}

		//clip near Z plane
	f = mat[3].z;
	if (mat[0].z > 0.f) f += (float)kv6->xsiz*mat[0].z;
	if (mat[1].z > 0.f) f += (float)kv6->zsiz*mat[1].z;
	if (mat[2].z > 0.f) f += (float)kv6->ysiz*mat[2].z;
	if (f <= 0.f) return;

		//clip left edge
	f  =     mat[3].x*frame->hz[0] + mat[3].z*frame->hx[0];
	f += max(mat[0].x*frame->hz[0] + mat[0].z*frame->hx[0],0.f) * (float)kv6->xsiz;
	f += max(mat[1].x*frame->hz[0] + mat[1].z*frame->hx[0],0.f) * (float)kv6->zsiz;
	f += max(mat[2].x*frame->hz[0] + mat[2].z*frame->hx[0],0.f) * (float)kv6->ysiz;
	if (f <= 0.f) return;

		//clip right edge
	f  =     mat[3].x*frame->hz[0] + mat[3].z*(frame->hx[0]-frame->ddx);
	f += min(mat[0].x*frame->hz[0] + mat[0].z*(frame->hx[0]-frame->ddx),0.f) * (float)kv6->xsiz;
	f += min(mat[1].x*frame->hz[0] + mat[1].z*(frame->hx[0]-frame->ddx),0.f) * (float)kv6->zsiz;
	f += min(mat[2].x*frame->hz[0] + mat[2].z*(frame->hx[0]-frame->ddx),0.f) * (float)kv6->ysiz;
	if (f >= 0.f) return;

		//clip top edge
	f  =     mat[3].y*frame->hz[0] + mat[3].z*frame->hy[0];
	f += max(mat[0].y*frame->hz[0] + mat[0].z*frame->hy[0],0.f) * (float)kv6->xsiz;
	f += max(mat[1].y*frame->hz[0] + mat[1].z*frame->hy[0],0.f) * (float)kv6->zsiz;
	f += max(mat[2].y*frame->hz[0] + mat[2].z*frame->hy[0],0.f) * (float)kv6->ysiz;
	if (f <= 0.f) return;

		//clip bot edge
	f  =     mat[3].y*frame->hz[0] + mat[3].z*(frame->hy[0]-frame->ddy);
	f += min(mat[0].y*frame->hz[0] + mat[0].z*(frame->hy[0]-frame->ddy),0.f) * (float)kv6->xsiz;
	f += min(mat[1].y*frame->hz[0] + mat[1].z*(frame->hy[0]-frame->ddy),0.f) * (float)kv6->zsiz;
	f += min(mat[2].y*frame->hz[0] + mat[2].z*(frame->hy[0]-frame->ddy),0.f) * (float)kv6->ysiz;
	if (f >= 0.f) return;


		//use Cramer's rule to project origin to voxel grid coords
		//mat[0].x*fx + mat[1].x*fy + mat[2].x*fz = -mat[3].x
		//mat[0].y*fx + mat[1].y*fy + mat[2].y*fz = -mat[3].y
		//mat[0].z*fx + mat[1].z*fy + mat[2].z*fz = -mat[3].z
	f = (mat[1].y*mat[2].z - mat[2].y*mat[1].z)*mat[0].x +
		 (mat[2].y*mat[0].z - mat[0].y*mat[2].z)*mat[1].x +
		 (mat[0].y*mat[1].z - mat[1].y*mat[0].z)*mat[2].x;
	if (f != 0) f = 1.0/f;
	grdx = cvttss2si(((mat[2].y*mat[1].z - mat[1].y*mat[2].z)*mat[3].x +
							(mat[3].y*mat[2].z - mat[2].y*mat[3].z)*mat[1].x +
							(mat[1].y*mat[3].z - mat[3].y*mat[1].z)*mat[2].x)*f);
	grdy = cvttss2si(((mat[2].y*mat[3].z - mat[3].y*mat[2].z)*mat[0].x +
							(mat[0].y*mat[2].z - mat[2].y*mat[0].z)*mat[3].x +
							(mat[3].y*mat[0].z - mat[0].y*mat[3].z)*mat[2].x)*f);
	grdz = cvttss2si(((mat[3].y*mat[1].z - mat[1].y*mat[3].z)*mat[0].x +
							(mat[0].y*mat[3].z - mat[3].y*mat[0].z)*mat[1].x +
							(mat[1].y*mat[0].z - mat[0].y*mat[1].z)*mat[3].x)*f);

	if (drawkv6_numlights < 0)
	{
		fx = (float)(rigx+rigy+rigz); //Hack for shading
		fy = (float)(forx+fory+forz);
		fz = (float)(dowx+dowy+dowz);
		f = fx*fx + fy*fy + fz*fz; if (f != 0.f) f = shadefac*2.0/sqrt(f);
		fx *= f; fy *= f; fz *= f;
		for(i=256-1;i>=0;i--)
		{
			k = cvttss2si(univec[i].x*fx + univec[i].y*fy + univec[i].z*fz)+256;
			rgbmul[i][0] = (unsigned short)min(((curcol    )&255)*k,65535);
			rgbmul[i][1] = (unsigned short)min(((curcol>> 8)&255)*k,65535);
			rgbmul[i][2] = (unsigned short)min(((curcol>>16)&255)*k,65535);
		}
	}
	else
	{
		point3d *ligvec;

			//FIX: doesn't support non-orthogonal matrix
		f = 1.f/sqrt(rigx*rigx + rigy*rigy + rigz*rigz); rigx *= f; rigy *= f; rigz *= f;
		f = 1.f/sqrt(dowx*dowx + dowy*dowy + dowz*dowz); dowx *= f; dowy *= f; dowz *= f;
		f = 1.f/sqrt(forx*forx + fory*fory + forz*forz); forx *= f; fory *= f; forz *= f;

		ligvec = (point3d *)_alloca(drawkv6_numlights*sizeof(point3d));
		for(k=drawkv6_numlights-1;k>=0;k--)
		{
			float nfx, nfy, nfz;
#if (USEINTZ)
			fx = drawkv6_light[k].p.x - opos.x/256.0;
			fy = drawkv6_light[k].p.y - opos.y/256.0;
			fz = drawkv6_light[k].p.z - opos.z/256.0;
#else
			fx = drawkv6_light[k].p.x - opos.x;
			fy = drawkv6_light[k].p.y - opos.y;
			fz = drawkv6_light[k].p.z - opos.z;
#endif
			nfx = fx*rigx + fy*rigy + fz*rigz;
			nfy = fx*dowx + fy*dowy + fz*dowz;
			nfz = fx*forx + fy*fory + fz*forz;
			f2 = nfx*nfx + nfy*nfy + nfz*nfz; if (f2 < .0001f) break;
			f2 = 1024.0/f2;
			ligvec[k].x = nfx*f2;
			ligvec[k].y = nfy*f2;
			ligvec[k].z = nfz*f2;
		}
		if (k >= 0)
		{
			for(i=256-1;i>=0;i--)
			{
				rgbmul[i][0] = (unsigned short)min(((curcol    )&255)*65535,65535);
				rgbmul[i][1] = (unsigned short)min(((curcol>> 8)&255)*65535,65535);
				rgbmul[i][2] = (unsigned short)min(((curcol>>16)&255)*65535,65535);
			}
		}
		else
		{
			for(i=256-1;i>=0;i--)
			{
				float colr, colg, colb;
				colb = drawkv6_ambrgb[0];//*1.0;
				colg = drawkv6_ambrgb[1];//*1.0;
				colr = drawkv6_ambrgb[2];//*1.0;
				for(k=drawkv6_numlights-1;k>=0;k--)
				{
					f2 = ligvec[k].x*univec[i].x + ligvec[k].y*univec[i].z + ligvec[k].z*univec[i].y; if (f2 >= 0.f) continue;
					colb -= drawkv6_light[k].rgb[0]*f2;
					colg -= drawkv6_light[k].rgb[1]*f2;
					colr -= drawkv6_light[k].rgb[2]*f2;
				}
				rgbmul[i][0] = (unsigned short)min(((curcol    )&255)*((int)colb),65535);
				rgbmul[i][1] = (unsigned short)min(((curcol>> 8)&255)*((int)colg),65535);
				rgbmul[i][2] = (unsigned short)min(((curcol>>16)&255)*((int)colr),65535);
			}
		}
	}

#if !defined(_WIN64)
	nirig.x = mat[0].x*frame->hz[0]; nirig.y = mat[0].y*frame->hz[0]; nirig.z = mat[0].z;
	nidow.x = mat[1].x*frame->hz[0]; nidow.y = mat[1].y*frame->hz[0]; nidow.z = mat[1].z;
	nifor.x = mat[2].x*frame->hz[0]; nifor.y = mat[2].y*frame->hz[0]; nifor.z = mat[2].z;
	nipos.x = mat[3].x*frame->hz[0]; nipos.y = mat[3].y*frame->hz[0]; nipos.z = mat[3].z;

	pra[0].x = 0; pra[0].y = 0; pra[0].z = 0;
							 pra[  1].x = pra[0].x+nifor.x; pra[  1].y = pra[0].y+nifor.y; pra[  1].z = pra[0].z+nifor.z;
	for(i=0;i<2;i++) { pra[i+2].x = pra[i].x+nidow.x; pra[i+2].y = pra[i].y+nidow.y; pra[i+2].z = pra[i].z+nidow.z; }
	for(i=0;i<4;i++) { pra[i+4].x = pra[i].x+nirig.x; pra[i+4].y = pra[i].y+nirig.y; pra[i+4].z = pra[i].z+nirig.z; }

	for(vis=0;vis<43;vis++)
	{
		i = visboundcorns[vis]; if (!i) continue;
		k = 0;
		if (i&  1) { bcfaddx[vis][k] = pra[0].x; bcfaddy[vis][k] = pra[0].y; bcfaddz[vis][k] = pra[0].z; k++; }
		if (i&  2) { bcfaddx[vis][k] = pra[1].x; bcfaddy[vis][k] = pra[1].y; bcfaddz[vis][k] = pra[1].z; k++; }
		if (i&  4) { bcfaddx[vis][k] = pra[2].x; bcfaddy[vis][k] = pra[2].y; bcfaddz[vis][k] = pra[2].z; k++; }
		if (i&  8) { bcfaddx[vis][k] = pra[3].x; bcfaddy[vis][k] = pra[3].y; bcfaddz[vis][k] = pra[3].z; k++; }
		if (i& 16) { bcfaddx[vis][k] = pra[4].x; bcfaddy[vis][k] = pra[4].y; bcfaddz[vis][k] = pra[4].z; k++; }
		if (i& 32) { bcfaddx[vis][k] = pra[5].x; bcfaddy[vis][k] = pra[5].y; bcfaddz[vis][k] = pra[5].z; k++; }
		if (i& 64) { bcfaddx[vis][k] = pra[6].x; bcfaddy[vis][k] = pra[6].y; bcfaddz[vis][k] = pra[6].z; k++; }
		if (i&128) { bcfaddx[vis][k] = pra[7].x; bcfaddy[vis][k] = pra[7].y; bcfaddz[vis][k] = pra[7].z; k++; }
	}

	minprz = .004f;
	if (*(int *)&nirig.z < 0) minprz -= nirig.z;
	if (*(int *)&nidow.z < 0) minprz -= nidow.z;
	if (*(int *)&nifor.z < 0) minprz -= nifor.z;

	bcf_nix[0] = nirig.x; bcf_nix[1] = nirig.y; bcf_nix[2] = nirig.z;
	bcf_niy[0] = nidow.x; bcf_niy[1] = nidow.y; bcf_niy[2] = nidow.z;
	bcf_niz[0] = nifor.x; bcf_niz[1] = nifor.y; bcf_niz[2] = nifor.z;
	bcf_pr0[0] = nipos.x; bcf_pr0[1] = nipos.y; bcf_pr0[2] = nipos.z;
	bcf_pr1[0] = bcf_pr0[0]; bcf_pr1[1] = bcf_pr0[1]; bcf_pr1[2] = bcf_pr0[2];

	i = 0; kvoxptr = kv6->vox;
	if (!(gcpuid[1][3]&(1<<25))) //not SSE
	{
		for(x=0;x<kv6->xsiz;x++)
		{
			vis0 = 0;
				  if (grdx < x) vis0 = 1;
			else if (grdx > x) vis0 = 2;
			for(z=0;z<kv6->ysiz;z++,i++)
			{
				vis1 = vis0;
					  if (grdz < z) vis1 |= 4;
				else if (grdz > z) vis1 |= 8;
				for(kvoxptre=&kvoxptr[kv6->ylen[i]];kvoxptr<kvoxptre;kvoxptr++)
				{
					vis = vis1; y = kvoxptr->z;
						  if (grdy < y) vis |= 16;
					else if (grdy > y) vis |= 32;
					vis &= kvoxptr->vis; if (!vis) continue;

					col = kvoxptr->col;
					usptr = rgbmul[kvoxptr->dir];
					ucptr = (unsigned char *)&col;
					ucptr[0] = (unsigned char)min((((int)ucptr[0])*usptr[0])>>15,255);
					ucptr[1] = (unsigned char)min((((int)ucptr[1])*usptr[1])>>15,255);
					ucptr[2] = (unsigned char)min((((int)ucptr[2])*usptr[2])>>15,255);

					pr2.z = (float)y*bcf_niy[2] + bcf_pr1[2]; if (*(int *)&pr2.z < *(int *)&minprz) continue;
					pr2.x = (float)y*bcf_niy[0] + bcf_pr1[0];
					pr2.y = (float)y*bcf_niy[1] + bcf_pr1[1];

					bcf_ptx[0] = pr2.x+bcfaddx[vis][0]; bcf_ptx[1] = pr2.x+bcfaddx[vis][1];
					bcf_ptx[2] = pr2.x+bcfaddx[vis][2]; bcf_ptx[3] = pr2.x+bcfaddx[vis][3];
					bcf_pty[0] = pr2.y+bcfaddy[vis][0]; bcf_pty[1] = pr2.y+bcfaddy[vis][1];
					bcf_pty[2] = pr2.y+bcfaddy[vis][2]; bcf_pty[3] = pr2.y+bcfaddy[vis][3];
					bcf_ptz[0] = pr2.z+bcfaddz[vis][0]; bcf_ptz[1] = pr2.z+bcfaddz[vis][1];
					bcf_ptz[2] = pr2.z+bcfaddz[vis][2]; bcf_ptz[3] = pr2.z+bcfaddz[vis][3];
					if (bitsum[vis] != 1)
					{
						bcf_ptx[4] = pr2.x+bcfaddx[vis][4]; bcf_ptx[5] = pr2.x+bcfaddx[vis][5];
						bcf_pty[4] = pr2.y+bcfaddy[vis][4]; bcf_pty[5] = pr2.y+bcfaddy[vis][5];
						bcf_ptz[4] = pr2.z+bcfaddz[vis][4]; bcf_ptz[5] = pr2.z+bcfaddz[vis][5];
						k = 6;
					} else k = 4;
					boundcubefillz_c(frame,k,col);
				}
				bcf_pr1[0] += bcf_niz[0]; bcf_pr1[1] += bcf_niz[1]; bcf_pr1[2] += bcf_niz[2];
			}
			bcf_pr0[0] += bcf_nix[0]; bcf_pr0[1] += bcf_nix[1]; bcf_pr0[2] += bcf_nix[2];
			bcf_pr1[0] = bcf_pr0[0]; bcf_pr1[1] = bcf_pr0[1]; bcf_pr1[2] = bcf_pr0[2];
		}
	}
	else
	{
		for(x=0;x<kv6->xsiz;x++)
		{
			vis0 = 0;
				  if (grdx < x) vis0 = 1;
			else if (grdx > x) vis0 = 2;
			for(z=0;z<kv6->ysiz;z++,i++)
			{
				vis1 = vis0;
					  if (grdz < z) vis1 |= 4;
				else if (grdz > z) vis1 |= 8;
				kvoxptre = &kvoxptr[kv6->ylen[i]];
				_asm
				{
					push ebx
					push esi ;Must push because eax/ecx/edx can be destroyed in function
					push edi
					movd mm5, frame
					mov esi, kvoxptr
					cmp esi, kvoxptre
					jge short dk6_endit2
dk6_begit:        movzx edx, word ptr [esi+4] ;kvoxptr->z
						cvtsi2ss xmm0, edx

						sub edx, grdy
						jz short dk6_visy0
							shr edx, 31
							add edx, 1
							shl edx, 4
dk6_visy0:        add edx, vis1
						movzx ecx, byte ptr [esi+6] ;kvoxptr->vis
						and edx, ecx
						jz short dk6_endit

						shufps xmm0, xmm0, 0
						mulps xmm0, bcf_niy
						addps xmm0, bcf_pr1
						movhlps xmm2, xmm0
						comiss xmm2, minprz
						jb short dk6_endit

						mov ecx, edx ;edx=vis
						shl ecx, 5

						shufps xmm2, xmm2, 0
						movaps xmm1, xmm0
						shufps xmm0, xmm0, 0
						shufps xmm1, xmm1, 0x55
						movaps xmm5, xmm2
						movaps xmm3, xmm0
						movaps xmm4, xmm1
						addps xmm2, bcfaddz[ecx]
						addps xmm0, bcfaddx[ecx]
						addps xmm1, bcfaddy[ecx]

						rcpps xmm6, xmm2
						mulps xmm0, xmm6
						mulps xmm1, xmm6
#if (USEINTZ)
						movd eax, mm5 ;frame
						mulss xmm2, [eax+drawkv6_frame_t.rhzup20]
						cvtss2si edi, xmm2
#endif
						movaps xmm6, xmm0    ;xmm6: fx3 fx2 fx1 fx0
						unpcklps xmm0, xmm1  ;xmm0: fy1 fx1 fy0 fx0
						unpckhps xmm6, xmm1  ;xmm6: fy3 fx3 fy2 fx2
						movaps xmm1, xmm6    ;xmm1: fy3 fx3 fy2 fx2
						maxps xmm1, xmm0     ;xmm1: My1 Mx1 My0 Mx0
						minps xmm0, xmm6     ;xmm0: my1 mx1 my0 mx0

						cmp byte ptr bitsum[edx], 1
						je short dk6_skp
							addps xmm5, bcfaddz[ecx+16]
							addps xmm3, bcfaddx[ecx+16]
							addps xmm4, bcfaddy[ecx+16]
							rcpps xmm5, xmm5
							mulps xmm3, xmm5
							mulps xmm4, xmm5
							unpcklps xmm3, xmm4  ;xmm3: fy5 fx5 fy4 fx4
							maxps xmm1, xmm3
							minps xmm0, xmm3
dk6_skp:
						movhlps xmm3, xmm1  ;xmm3: ??? ??? My1 Mx1
						movhlps xmm6, xmm0  ;xmm6: ??? ??? my1 mx1
						maxps xmm1, xmm3    ;xmm1: ??? ??? My  Mx
						minps xmm0, xmm6    ;xmm0: ??? ??? my  mx
						cvttps2pi mm1, xmm1 ;mm1: y1 x1
						cvttps2pi mm0, xmm0 ;mm0: y0 x0

						packssdw mm0, mm1   ;mm0:[y1 x1 y0 x0]
						movd edx, mm5 ;frame
						paddsw mm0, [edx+drawkv6_frame_t.ighyxyx]
						pminsw mm0, [edx+drawkv6_frame_t.igyxyx]
						pmaxsw mm0, ig0000

						movd ecx, mm0
						punpckhdq mm0, mm0
						movd eax, mm0
						movzx edx, cx ;x0
						movzx ebx, ax ;x1
						shr ecx, 16   ;y0
						shr eax, 16   ;y1

						cmp ebx, edx
						jbe short dk6_endit
						cmp eax, ecx
						jbe short dk6_endit

						shl edx, 2
						shl ebx, 2

						sub edx, ebx
						movd mm6, edx ;x0 = ((x0-x1)<<2);
						movd edx, mm5 ;frame
						imul ecx, [edx+drawkv6_frame_t.ddp] ;ebx = ecx*frame->ddp + frame->ddf + (x1<<2);
						imul eax, [edx+drawkv6_frame_t.ddp] ;eax = eax*frame->ddp + frame->ddf;
						add ecx, [edx+drawkv6_frame_t.ddf]
						add eax, [edx+drawkv6_frame_t.ddf]
						add ebx, ecx

						punpcklbw mm7, [esi] ;kvoxptr->col
						psrlw mm7, 7
						movzx ecx, byte ptr [esi+7] ;kvoxptr->dir
						pmulhuw mm7, rgbmul[ecx*8]
						packuswb mm7, mm7

						mov ecx, [edx+drawkv6_frame_t.zbufoff]
						add ecx, ebx
dk6_begy:            movd edx, mm6
dk6_begx:
#if (USEINTZ)
								cmp edi, [ecx+edx]
								jge short dk6_skpx
									mov [ecx+edx], edi
#else
								comiss xmm2, [ecx+edx]
								ja short dk6_skpx
									movss [ecx+edx], xmm2
#endif
									movd [ebx+edx], mm7
dk6_skpx:               add edx, 4
								jl short dk6_begx

							movd edx, mm5 ;frame
							add ebx, [edx+drawkv6_frame_t.ddp]
							add ecx, [edx+drawkv6_frame_t.ddp]
							cmp ebx, eax
							jle short dk6_begy
dk6_endit:        add esi, 8
						cmp esi, kvoxptre
						jl short dk6_begit
					mov kvoxptr, esi
dk6_endit2:    pop edi
					pop esi
					pop ebx

					movaps xmm0, bcf_pr1
					addps xmm0, bcf_niz
					movaps bcf_pr1, xmm0
				}
			}
			_asm
			{
				movaps xmm0, bcf_pr0
				addps xmm0, bcf_nix
				movaps bcf_pr0, xmm0
				movaps bcf_pr1, xmm0
			}
		}
		_asm emms
	}
#else

#if defined(_WIN64)
	ALIGN(32) float nvx[8], nvy[8], nvz[8];
#endif
	//static const int vislut[64] = //indices 0..42 used; all valid results give 4 or 6 corners
	//{
	//      0,0x55,0xaa,   0,0x33,0x77,0xbb,   0,
	//   0xcc,0xdd,0xee,   0,   0,   0,   0,   0,
	//   0x0f,0x5f,0xaf,   0,0x3f,0x7e,0xbd,   0,
	//   0xcf,0xdb,0xe7,   0,   0,   0,   0,   0,
	//   0xf0,0xf5,0xfa,   0,0xf3,0xe7,0xdb,   0,
	//   0xfc,0xbd,0x7e,   0,   0,   0,   0,   0,
	//      0,   0,   0,   0,   0,   0,   0,   0,
	//      0,   0,   0,   0,   0,   0,   0,   0,
	//};
	static const int vislut2[64] = //indices 0..42 used; all valid results give 4 or 6 corners
	{
		0xfffffff,0xfff6420,0xfff7531,0xfffffff,0xfff5410,0xf654210,0xf754310,0xfffffff,
		0xfff7632,0xf764320,0xf765321,0xfffffff,0xfffffff,0xfffffff,0xfffffff,0xfffffff,
		0xfff3210,0xf643210,0xf753210,0xfffffff,0xf543210,0xf654321,0xf754320,0xfffffff,
		0xf763210,0xf764310,0xf765210,0xfffffff,0xfffffff,0xfffffff,0xfffffff,0xfffffff,
		0xfff7654,0xf765420,0xf765431,0xfffffff,0xf765410,0xf765210,0xf764310,0xfffffff,
		0xf765432,0xf754320,0xf654321,0xfffffff,0xfffffff,0xfffffff,0xfffffff,0xfffffff,
		0xfffffff,0xfffffff,0xfffffff,0xfffffff,0xfffffff,0xfffffff,0xfffffff,0xfffffff,
		0xfffffff,0xfffffff,0xfffffff,0xfffffff,0xfffffff,0xfffffff,0xfffffff,0xfffffff,
	};
	point3d nv[8];
	float g, nvminz;
	int yy, zz, ke, x0, y0, x1, y1, ix, iy, nx0, nx1, ext[8], *wptr, *wptre, *zptr;

	nvminz = 1e32;
	for(i=8-1;i>=0;i--)
	{
		fx = 0.f; fy = 0.f; fz = 0.f;
		if (i&1) { fx += mat[0].x; fy += mat[0].y; fz += mat[0].z; }
		if (i&2) { fx += mat[2].x; fy += mat[2].y; fz += mat[2].z; }
		if (i&4) { fx += mat[1].x; fy += mat[1].y; fz += mat[1].z; }
		nv[i].x = fx; nv[i].y = fy; nv[i].z = fz; nvminz = min(nvminz,fz);
#if defined(_WIN64)
		nvx[i] = fx; nvy[i] = fy; nvz[i] = fz;
#endif
	}
	nvminz = 1e-1-nvminz;

	bcf_nix[0] = mat[0].x; bcf_nix[1] = mat[0].y; bcf_nix[2] = mat[0].z;
	bcf_niy[0] = mat[1].x; bcf_niy[1] = mat[1].y; bcf_niy[2] = mat[1].z;
	bcf_niz[0] = mat[2].x; bcf_niz[1] = mat[2].y; bcf_niz[2] = mat[2].z;
	bcf_pr0[0] = mat[3].x; bcf_pr0[1] = mat[3].y; bcf_pr0[2] = mat[3].z;
	bcf_pr1[0] = bcf_pr0[0]; bcf_pr1[1] = bcf_pr0[1]; bcf_pr1[2] = bcf_pr0[2];

	i = 0; kvoxptr = kv6->vox;
	for(x=0;x<kv6->xsiz;x++)
	{
		vis0 = 0;
			  if (grdx < x) vis0 = 1;
		else if (grdx > x) vis0 = 2;
		for(z=0;z<kv6->ysiz;z++,i++)
		{
			vis1 = vis0;
				  if (grdz < z) vis1 |= 4;
			else if (grdz > z) vis1 |= 8;

			for(kvoxptre=&kvoxptr[kv6->ylen[i]];kvoxptr<kvoxptre;kvoxptr++)
			{
				vis = vis1; y = kvoxptr->z;
					  if (grdy < y) vis |= 16;
				else if (grdy > y) vis |= 32;
				vis &= kvoxptr->vis; if (!vis) continue; //Backface-cull

#if !defined(_WIN64)
				fx = (float)y*bcf_niy[0] + bcf_pr1[0];
				fy = (float)y*bcf_niy[1] + bcf_pr1[1];
				fz = (float)y*bcf_niy[2] + bcf_pr1[2]; if (fz < nvminz) continue;

					//Find bounding rectangle, /*bounding diamond*/, bounding slope 2's
				x0 = 0x7fffffff; y0 = 0x7fffffff; x1 = 0x80000000; y1 = 0x80000000;
				ext[0] = 0x7fffffff; ext[1] = 0x80000000; ext[2] = 0x7fffffff; ext[3] = 0x80000000;
				ext[4] = 0x7fffffff; ext[5] = 0x80000000; ext[6] = 0x7fffffff; ext[7] = 0x80000000;

				for(vis=vislut2[vis];1;vis>>=4) //Visits 4 or 6 corners
				{
					k = (vis&15); if (k == 15) break;

						//Transform/project/bbox
					g = rcpss(nv[k].z+fz)*frame->hz[0];
					ix = cvttss2si((nv[k].x+fx)*g + frame->hx[0]);
					iy = cvttss2si((nv[k].y+fy)*g + frame->hy[0]);

					x0 = min(x0,ix); x1 = max(x1,ix);
					y0 = min(y0,iy); y1 = max(y1,iy);
					k = ix*2+iy; ext[0] = min(ext[0],k); ext[1] = max(ext[1],k);
					k = iy*2+ix; ext[2] = min(ext[2],k); ext[3] = max(ext[3],k);
					k = ix*2-iy; ext[4] = min(ext[4],k); ext[5] = max(ext[5],k);
					k = ix-iy*2; ext[6] = min(ext[6],k); ext[7] = max(ext[7],k);
				}
				x0 = max(x0,0); x1 = min(x1,frame->ddx);
				y0 = max(y0,0); y1 = min(y1,frame->ddy);
				ext[2] *= 2; ext[3] *= 2; ext[6] *= 2; ext[7] *= 2;

				g = nv[7].z*.5f + fz;
#if (USEINTZ != 0)
				zz = cvttss2si(g*frame->rhzup20[0]);
#else
				zz = *(int *)&g;
#endif
				col = colscale15(kvoxptr->col,*(__int64 *)&rgbmul[kvoxptr->dir][0]);

					//Z-buffer&draw
				for(yy=y0;yy<y1;yy++)
				{
					nx0 = max(max(ext[2]-yy*4,ext[6]+yy*4),max(ext[0]-yy,ext[4]+yy))>>1; nx0 = max(nx0,x0);
					nx1 = min(min(ext[3]-yy*4,ext[7]+yy*4),min(ext[1]-yy,ext[5]+yy))>>1; nx1 = min(nx1,x1);

					wptr = (int *)(yy*frame->ddp + frame->ddf); wptre = &wptr[nx1]; wptr += nx0;
					for(;wptr<wptre;wptr++)
					{
						if (zz >= *(int *)((INT_PTR)wptr+frame->zbufoff)) continue;
						*(int *)((INT_PTR)wptr+frame->zbufoff) = zz;
						wptr[0] = col;
					}
				}
#else
				fx = (float)y*bcf_niy[0] + bcf_pr1[0];
				fy = (float)y*bcf_niy[1] + bcf_pr1[1];
				fz = (float)y*bcf_niy[2] + bcf_pr1[2]; if (fz < nvminz) continue;
				col = colscale15(kvoxptr->col,*(__int64 *)&rgbmul[kvoxptr->dir][0]);
				drawkv6_xform_avx2(nvx,nvy,nvz,fx,fy,fz,vis,col,frame);
#endif
			}
			bcf_pr1[0] += bcf_niz[0]; bcf_pr1[1] += bcf_niz[1]; bcf_pr1[2] += bcf_niz[2];
		}
		bcf_pr0[0] += bcf_nix[0]; bcf_pr0[1] += bcf_nix[1]; bcf_pr0[2] += bcf_nix[2];
		bcf_pr1[0] = bcf_pr0[0]; bcf_pr1[1] = bcf_pr0[1]; bcf_pr1[2] = bcf_pr0[2];
	}
#endif
}

void drawkv6_init (void)
{
	int i;

	gcpuid[0][0] = 0;
	for(i=0;(i<8) && (i <= gcpuid[0][0]);i++) { kcpuid(i,&gcpuid[i][0]); }
	gavxsupp = (((gcpuid[1][2]&((1<<12)/*FMA*/ + (1<<28)/*AVX*/)) == (1<<12) + (1<<28)) && (gcpuid[7][1]&(1<<5))/*AVX2*/);

		//for drawkv6()
	equivec.zmulk = 2.0/255.0; equivec.zaddk = equivec.zmulk*.5 - 1.0;
	for(i=0;i<255;i++) equiind2vec(i,&univec[i].x,&univec[i].y,&univec[i].z);
	univec[255].x = 0; univec[255].y = 0; univec[255].z = 0;

	drawkv6_numlights = -1;

	memset(kv6hashead,-1,sizeof(kv6hashead));
}

void drawkv6_setup (drawkv6_frame_t *frame, tiletype *dd, INT_PTR lzbufoff,
						  point3d *lipos, point3d *lirig, point3d *lidow, point3d *lifor,
						  float hx, float hy, float hz)
{
	int i;

	frame->zbufoff = lzbufoff;
	frame->ddp = dd->p;
	frame->ddf = dd->f;
	frame->ddx = dd->x;
	frame->ddy = dd->y;
	frame->p = *lipos;
	frame->r = *lirig;
	frame->d = *lidow;
	frame->f = *lifor;
	for(i=8-1;i>=0;i--)
	{
		frame->hx[i] = hx; frame->hy[i] = hy; frame->hz[i] = hz;
#if defined(_WIN64)
		frame->wmax[i] = -32768; frame->wmin[i] = 32767;
#endif
	}
#if defined(_WIN64)
	frame->wmax[0] = 0; frame->wmin[0] = (short)dd->x;
	frame->wmax[1] = 0; frame->wmin[1] = (short)dd->y;
#endif
	frame->rhzup20[0] = 1048576.0/hz; for(i=8-1;i>0;i--) frame->rhzup20[i] = frame->rhzup20[0];

	frame->ighyxyx[0] = hx; frame->ighyxyx[2] = hx; frame->ighyxyx[1] = hy; frame->ighyxyx[3] = hy;
	frame->igyxyx[0] = dd->x; frame->igyxyx[2] = dd->x; frame->igyxyx[1] = dd->y; frame->igyxyx[3] = dd->y;
}

void drawkv6_setup (drawkv6_frame_t *frame, tiletype *dd, INT_PTR lzbufoff,
						  dpoint3d *lipos, dpoint3d *lirig, dpoint3d *lidow, dpoint3d *lifor,
						  float hx, float hy, float hz)
{
	point3d fipos, firig, fidow, fifor;
	fipos.x = lipos->x; fipos.y = lipos->y; fipos.z = lipos->z;
	firig.x = lirig->x; firig.y = lirig->y; firig.z = lirig->z;
	fidow.x = lidow->x; fidow.y = lidow->y; fidow.z = lidow->z;
	fifor.x = lifor->x; fifor.y = lifor->y; fifor.z = lifor->z;
	drawkv6_setup(frame,dd,lzbufoff,&fipos,&firig,&fidow,&fifor,hx,hy,hz);
}

static kv6data_t *genmipkv6 (kv6data_t *kv6)
{
	static unsigned int umulmip[9] = {0,4294967295,2147483648,1431655765,1073741824,858993459,715827882,613566756,536870912};
	kv6data_t *nkv6;
	kv6voxtype *v0[2], *vs[4], *ve[4], *voxptr;
	unsigned short *xyptr, *xyi2, *sxyi2;
	int i, j, x, y, z, xs, ys, zs, xysiz, n, oxn, oxyn, *xptr;
	int xx, yy, zz, r, g, b, vis, npix, sxyi2i, darand = 0;
	char vecbuf[8];

	if ((!kv6) || (kv6->lowermip)) return(0);

	xs = ((kv6->xsiz+1)>>1); ys = ((kv6->ysiz+1)>>1); zs = ((kv6->zsiz+1)>>1);
	if ((xs < 2) || (ys < 2) || (zs < 2)) return(0);
	xysiz = ((((xs*ys)<<1)+3)&~3);
	i = sizeof(kv6data_t) + (xs<<2) + xysiz + kv6->numvoxs*sizeof(kv6voxtype);
	nkv6 = (kv6data_t *)malloc(i);
	if (!nkv6) return(0);

	nkv6->xsiz = xs;
	nkv6->ysiz = ys;
	nkv6->zsiz = zs;
	nkv6->xpiv = kv6->xpiv*.5;
	nkv6->ypiv = kv6->ypiv*.5;
	nkv6->zpiv = kv6->zpiv*.5;
	nkv6->lowermip = 0;

	xptr = (int *)(((INT_PTR)nkv6) + sizeof(kv6data_t));
	xyptr = (unsigned short *)(((INT_PTR)xptr) + (xs<<2));
	voxptr = (kv6voxtype *)(((INT_PTR)xyptr) + xysiz);
	n = 0;

	v0[0] = kv6->vox; sxyi2 = kv6->ylen; sxyi2i = (kv6->ysiz<<1);
	for(x=0;x<xs;x++)
	{
		v0[1] = v0[0]+kv6->xlen[x<<1];

			//vs: start pointer of each of the 4 columns
			//ve: end pointer of each of the 4 columns
		vs[0] = v0[0]; vs[2] = v0[1];

		xyi2 = sxyi2; sxyi2 += sxyi2i;

		oxn = n;
		for(y=0;y<ys;y++)
		{
			oxyn = n;

			ve[0] = vs[1] = vs[0]+xyi2[0];
			if ((x<<1)+1 < kv6->xsiz) { ve[2] = vs[3] = vs[2]+xyi2[kv6->ysiz]; }
			if ((y<<1)+1 < kv6->ysiz)
			{
				ve[1] = vs[1]+xyi2[1];
				if ((x<<1)+1 < kv6->xsiz) ve[3] = vs[3]+xyi2[kv6->ysiz+1];
			}
			xyi2 += 2;

			while (1)
			{
				z = 0x7fffffff;
				for(i=3;i>=0;i--)
					if ((vs[i] < ve[i]) && (vs[i]->z < z)) z = vs[i]->z;
				if (z == 0x7fffffff) break;

				z |= 1;

				r = 0; g = 0; /*b = 0;*/ vis = 0; npix = 0;
				for(i=3;i>=0;i--)
					for(zz=z-1;zz<=z;zz++)
					{
						if ((vs[i] >= ve[i]) || (vs[i]->z > zz)) continue;
						r += (vs[i]->col&0xff00ff); //MMX-style trick!
						g += (vs[i]->col&  0xff00);
						//b += (vs[i]->col&    0xff);
						vis |= vs[i]->vis;
						vecbuf[npix] = vs[i]->dir;
						npix++; vs[i]++;
					}

				if (npix)
				{
					if (n >= kv6->numvoxs) { free(nkv6); return(0); } //Don't let it crash!

					i = umulmip[npix]; j = (npix>>1);
					voxptr[n].col = (umulshr32(r+(j<<16),i)&0xff0000) +
										 (umulshr32(g+(j<< 8),i)&  0xff00) +
										 (umulshr32((r&0xfff)+ j     ,i));
					voxptr[n].z = (z>>1);
					voxptr[n].vis = vis;
					voxptr[n].dir = vecbuf[umulshr32(darand,npix)]; darand += i;
					n++;
				}
			}
			xyptr[0] = n-oxyn; xyptr++;
			vs[0] = ve[1]; vs[2] = ve[3];
		}
		xptr[x] = n-oxn;
		if ((x<<1)+1 >= kv6->xsiz) break; //Avoid read page fault
		v0[0] = v0[1]+kv6->xlen[(x<<1)+1];
	}

	nkv6->leng = sizeof(kv6data_t) + (xs<<2) + xysiz + n*sizeof(kv6voxtype);
	nkv6 = (kv6data_t *)realloc(nkv6,nkv6->leng); if (!nkv6) return(0);
	nkv6->xlen = (unsigned int *)(((INT_PTR)nkv6) + sizeof(kv6data_t));
	nkv6->ylen = (unsigned short *)(((INT_PTR)nkv6->xlen) + (xs<<2));
	nkv6->vox = (kv6voxtype *)(((INT_PTR)nkv6->ylen) + xysiz);
	nkv6->datmalptr = 0; //mips are all in 1 block - no need to free separate data block
	nkv6->numvoxs = n;
	kv6->lowermip = nkv6;
	return(nkv6);
}

kv6data_t *drawkv6_get (char *filnam)
{
	kv6data_t *kv6, *tkv6;
	int i, l0, l1, l2, hashind;
	unsigned char ch;
	FILE *fil;

		//Calculate hash index (which hopefully is uniformly random :)
	for(i=0,hashind=0;filnam[i];i++)
	{
		ch = filnam[i]; if ((ch >= 'a') && (ch <= 'z')) ch -= 32;
		if (ch == '/') ch = '\\';
		hashind = ((int)ch) - hashind*3;
	}
	hashind &= (sizeof(kv6hashead)/sizeof(kv6hashead[0])-1);

		//Find if string is already in hash...
	for(i=kv6hashead[hashind];i>=0;i=kv6nam2ptr[i].hashnext)
		if (!strcmp(filnam,kv6nam2ptr[i].filnam)) return(kv6nam2ptr[i].ptr);

	fil = fopen(filnam,"rb"); if (!fil) return(0);
	if (kv6nam2ptr_num >= kv6nam2ptr_mal)
	{
		kv6nam2ptr_mal = max(kv6nam2ptr_mal<<1,256);
		kv6nam2ptr = (kv6nam2ptr_t *)realloc(kv6nam2ptr,kv6nam2ptr_mal*sizeof(kv6nam2ptr_t));
		if (!kv6nam2ptr) return(0);
	}
	kv6 = (kv6data_t *)malloc(sizeof(kv6data_t));
	kv6nam2ptr[kv6nam2ptr_num].hashnext = kv6hashead[hashind]; kv6hashead[hashind] = kv6nam2ptr_num;
	strcpy(kv6nam2ptr[kv6nam2ptr_num].filnam,filnam);
	kv6nam2ptr[kv6nam2ptr_num].ptr = kv6;
	kv6nam2ptr_num++;

	fread(kv6,8<<2,1,fil); if (kv6->leng != 0x6c78764b /*FIXFIXFIXFIX:LSWAPIB*/) { fclose(fil); free(kv6); kv6nam2ptr[kv6nam2ptr_num-1].ptr = 0; return(0); } //Kvxl
	l0 = kv6->numvoxs*sizeof(kv6voxtype);
	l1 = kv6->xsiz*sizeof(int);
	l2 = kv6->xsiz*kv6->ysiz*sizeof(short);
	kv6->lowermip = 0;
	kv6->vox = (kv6voxtype *)malloc(l0+l1+l2); if (!kv6->vox) { fclose(fil); free(kv6); kv6nam2ptr[kv6nam2ptr_num-1].ptr = 0; return(0); }
	kv6->datmalptr = kv6->vox;
	kv6->xlen = (unsigned int *)(((INT_PTR)kv6->vox) + l0);
	kv6->ylen = (unsigned short *)(((INT_PTR)kv6->xlen) + l1);
	fread(kv6->vox,l0+l1+l2,1,fil);
	fclose(fil);

		//Generate all lower mip-maps here:
	for(tkv6=kv6;tkv6=genmipkv6(tkv6););

	return(kv6);
}

void drawkv6_free (kv6data_t *kv6)
{
	if (kv6->lowermip) drawkv6_free(kv6->lowermip); //NOTE: dangerous - recursive!
	if (kv6->datmalptr) free(kv6->datmalptr);
	free((void *)kv6);
}

void drawkv6_freeall (void)
{
	kv6data_t *kv6;
	int i;

	for(i=kv6nam2ptr_num-1;i>=0;i--)
	{
		kv6 = kv6nam2ptr[i].ptr;
		if (kv6) drawkv6_free(kv6);
	}
	if (kv6nam2ptr) { free(kv6nam2ptr); kv6nam2ptr = 0; }
	kv6nam2ptr_num = kv6nam2ptr_mal = 0;
	memset(kv6hashead,-1,sizeof(kv6hashead));
}

#if (STANDALONE != 0)

//--------------------------------------------------------------------------------------------------
#if defined(_WIN32)
static void hidecurs (int makehid)
{
	static int ishid = 0;
	static RECT cursoldclip;

	makehid = (makehid != 0); if (makehid == ishid) return;

	if (makehid)
	{
		POINT p;
		RECT r;
		GetClipCursor(&cursoldclip);
		p.x = (xres>>1);
		p.y = (yres>>1);
		ClientToScreen(ghwnd,&p);
		r.left = p.x; r.right  = p.x+1;
		r.top  = p.y; r.bottom = p.y+1;
		ClipCursor(&r);
		ShowCursor(0);
	} else
	{
		ClipCursor(&cursoldclip);
		ShowCursor(1);
	}
	ishid = makehid;
}

#else

static void hidecurs (int makehid) { }

#endif
//--------------------------------------------------------------------------------------------------

	//i7-5820K:
	//vc6_32:  4.25ms
	//vc9_32:  4.25ms
	//vc12_32: 3.48ms
	//vc9_64:  7.34ms
	//vc12_64: 6.29ms -> 5.25ms -> 4.05ms

static void rotate_vex (float ang, point3d *a, point3d *b) //Rotate vectors a & b around their common plane, by ang
{
	float f, c, s;
	c = cos(ang); s = sin(ang);
	f = a->x; a->x = f*c + b->x*s; b->x = b->x*c - f*s;
	f = a->y; a->y = f*c + b->y*s; b->y = b->y*c - f*s;
	f = a->z; a->z = f*c + b->z*s; b->z = b->z*c - f*s;
}

int WINAPI WinMain (HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	tiletype dd;
	ALIGN(32) drawkv6_frame_t kv6frame;
	kv6data_t *caco;
	point3d ipos, irig, idow, ifor;
	INT_PTR zbufoff;
	double tim = 0.0, otim, dtim, t0, t1, tottim = 0.0, avgdtim = 0.0;
	float f, g, px, py, pz, rx, ry, rz, dx, dy, dz, fx, fy, fz;
	int i, x, y, z, obstatus = 0, *zbuf = 0, zbufmal = 0, numframes = 0;

	xres = 1024; yres = 768; prognam = "DrawKV6 by Ken Silverman";
	if (!initapp(hinst)) return(1);

	hidecurs(1);

	ipos.x = 0.f; ipos.y = 0.f; ipos.z = -64.f;
	irig.x = 1.f; irig.y = 0.f; irig.z = 0.f;
	idow.x = 0.f; idow.y = 1.f; idow.z = 0.f;
	ifor.x = 0.f; ifor.y = 0.f; ifor.z = 1.f;

	drawkv6_init();
	caco = drawkv6_get("caco.kv6");
	//caco = drawkv6_get("c:/doc/ken/voxon/klab/doghouse.kv6");
	//caco = drawkv6_get("c:/doc/ken/voxon/kv6/stuart.kv6");

	while (!breath())
	{
		otim = tim; tim = klock(); dtim = tim-otim;

		if (startdirectdraw(&dd.f,&dd.p,&dd.x,&dd.y))
		{
			drawrectfill(&dd,0,0,dd.x,dd.y,0x808080);

			i = dd.p*dd.y;
			if (i > zbufmal) { zbufmal = i; zbuf = (int *)realloc(zbuf,zbufmal+256); }
				//zbuffer aligns its memory to the same pixel boundaries as the screen!
				//WARNING: Pentium 4's L2 cache has severe slowdowns when 65536-64 <= (zbufoff&65535) < 64
			zbufoff = (((((INT_PTR)zbuf)-dd.f-128)+255)&~255)+128;
			memset(zbuf,0x7f,i);

#if 0
			drawkv6_setup(&kv6frame,&dd,zbufoff,&ipos,&irig,&idow,&ifor,(float)dd.x*.5f,(float)dd.y*.5f,(float)dd.x*.5f);

			t0 = klock();
			for(x=-1;x<=1;x+=2)
				for(y=-1;y<=1;y+=2)
					for(z=-1;z<=1;z+=2)
					{
						float f, g, c0, s0, c1, s1, c2, s2;
						f = (float)(x+y+z)*.5f + tim;
						c0 = cos(f); s0 = sin(f);
						c1 = c0; s1 = s0;
						c2 = 1.0; s2 = 0.0;
						f = c0*c2; g = s0*s2; rx = g*s1 + f; dz = f*s1 + g;
						f = s0*c2; g = c0*s2; dx = f*s1 - g; rz = g*s1 - f;
						ry = s2*c1; dy = c2*c1; fx = -s0*c1; fy = s1; fz = -c0*c1;
						f = 0.4f;
						px = (float)x*16.f; py = (float)y*16.f; pz = (float)z*16.f;
						drawkv6(&kv6frame,caco, px,py,pz, rx*f,ry*f,rz*f, dx*f,dy*f,dz*f, fx*f,fy*f,fz*f, ((x*127+128)<<16)+((y*127+128)<<8)+(z*127+128),48.0);
					}
			t1 = klock();
#else
			rotate_vex(dmousx*.005f,&ifor,&irig); dmousx = 0;
			rotate_vex(dmousy*.005f,&ifor,&idow); dmousy = 0;
			rotate_vex(irig.y*-.1f,&irig,&idow);

			f = dtim*16.f;
			if (keystatus[0x2a]) f *= 1.f/16.f;
			if (keystatus[0x36]) f *= 16.f/1.f;
			fx = keystatus[0xcd]-keystatus[0xcb];
			fy = keystatus[0x52]-keystatus[0x9d];
			fz = keystatus[0xc8]-keystatus[0xd0];
			ipos.x += (fx*irig.x + fy*idow.x + fz*ifor.x)*f;
			ipos.y += (fx*irig.y + fy*idow.y + fz*ifor.y)*f;
			ipos.z += (fx*irig.z + fy*idow.z + fz*ifor.z)*f;

			drawkv6_setup(&kv6frame,&dd,zbufoff,&ipos,&irig,&idow,&ifor,(float)dd.x*.5f,(float)dd.y*.5f,(float)dd.x*.5f);

			t0 = klock();
			drawkv6(&kv6frame,caco, 0.f,0.f,0.f, -1.f,0.f,0.f, 0.f,1.f,0.f, 0.f,0.f,-1.f, 0x808080,48.0);
			t1 = klock();
#endif

			tottim += t1-t0; print6x8(&dd,0,0,0xffffff,0,"%.2fms",tottim*1000.0/(double)(numframes+1));
			//avgdtim += (t1-t0-avgdtim)*.05; print6x8(&dd,0,0,0xffffff,0,"%.2fms",avgdtim*1000.0);

			if (keystatus[0x2a]) { tottim = 0.0; numframes = 0; }

			stopdirectdraw();
			nextpage();
			numframes++;
		}

		obstatus = bstatus;
		if (keystatus[0x1]) { keystatus[0x1] = 0; quitloop(); }
	}
	drawkv6_freeall();
	uninitapp();
	return(0);
}

#endif

#if 0
!endif
#endif
