//
// Created by omnis on 11/21/2025.
//
#define LIGHTMAX 256 //FIX:make dynamic!
#include "softrender.h"
const uint64_t font6x8[]=  //256 DOS chars, from: DOSAPP.FON (tab blank)
{
	0x3E00000000000000,0x6F6B3E003E455145,0x1C3E7C3E1C003E6B,0x3000183C7E3C1800,0x7E5C180030367F36,0x000018180000185C,0x0000FFFFE7E7FFFF,0xDBDBC3FF00000000,
	0x0E364A483000FFC3,0x6000062979290600,0x0A7E600004023F70,0x2A1C361C2A003F35,0x0800081C3E7F0000,0x7F361400007F3E1C,0x005F005F00001436,0x22007F017F090600,
	0x606060002259554D,0x14B6FFB614000060,0x100004067F060400,0x3E08080010307F30,0x08083E1C0800081C,0x0800404040407800,0x3F3C3000083E083E,0x030F3F0F0300303C,
	0x0000000000000000,0x0003070000065F06,0x247E247E24000307,0x630000126A2B2400,0x5649360063640813,0x0000030700005020,0x00000000413E0000,0x1C3E080000003E41,
	0x08083E080800083E,0x0800000060E00000,0x6060000008080808,0x0204081020000000,0x00003E4549513E00,0x4951620000407F42,0x3649494922004649,0x2F00107F12141800,
	0x494A3C0031494949,0x0305097101003049,0x0600364949493600,0x6C6C00001E294949,0x00006CEC00000000,0x2400004122140800,0x2241000024242424,0x0609590102000814,
	0x7E001E555D413E00,0x49497F007E111111,0x224141413E003649,0x7F003E4141417F00,0x09097F0041494949,0x7A4949413E000109,0x00007F0808087F00,0x4040300000417F41,
	0x412214087F003F40,0x7F00404040407F00,0x04027F007F020402,0x3E4141413E007F08,0x3E00060909097F00,0x09097F005E215141,0x3249494926006619,0x3F0001017F010100,
	0x40201F003F404040,0x3F403C403F001F20,0x0700631408146300,0x4549710007087008,0x0041417F00000043,0x0000201008040200,0x01020400007F4141,0x8080808080800402,
	0x2000000007030000,0x44447F0078545454,0x2844444438003844,0x38007F4444443800,0x097E080008545454,0x7CA4A4A418000009,0x0000007804047F00,0x8480400000407D00,
	0x004428107F00007D,0x7C0000407F000000,0x04047C0078041804,0x3844444438000078,0x380038444444FC00,0x44784400FC444444,0x2054545408000804,0x3C000024443E0400,
	0x40201C00007C2040,0x3C6030603C001C20,0x9C00006C10106C00,0x54546400003C60A0,0x0041413E0800004C,0x0000000077000000,0x02010200083E4141,0x3C2623263C000001,
	0x3D001221E1A11E00,0x54543800007D2040,0x7855555520000955,0x2000785554552000,0x5557200078545555,0x1422E2A21C007857,0x3800085555553800,0x5555380008555455,
	0x00417C0100000854,0x0000004279020000,0x2429700000407C01,0x782F252F78007029,0x3400455554547C00,0x7F097E0058547C54,0x0039454538004949,0x3900003944453800,
	0x21413C0000384445,0x007C20413D00007D,0x3D00003D60A19C00,0x40413C00003D4242,0x002466241800003D,0x29006249493E4800,0x16097F00292A7C2A,0x02097E8840001078,
	0x0000785555542000,0x4544380000417D00,0x007D21403C000039,0x7A0000710A097A00,0x5555080000792211,0x004E51514E005E55,0x3C0020404D483000,0x0404040404040404,
	0x506A4C0817001C04,0x0000782A34081700,0x0014080000307D30,0x0814000814001408,0x55AA114411441144,0xEEBBEEBB55AA55AA,0x0000FF000000EEBB,0x0A0A0000FF080808,
	0xFF00FF080000FF0A,0x0000F808F8080000,0xFB0A0000FE0A0A0A,0xFF00FF000000FF00,0x0000FE02FA0A0000,0x0F0800000F080B0A,0x0F0A0A0A00000F08,0x0000F80808080000,
	0x080808080F000000,0xF808080808080F08,0x0808FF0000000808,0x0808080808080808,0xFF0000000808FF08,0x0808FF00FF000A0A,0xFE000A0A0B080F00,0x0B080B0A0A0AFA02,
	0x0A0AFA02FA0A0A0A,0x0A0A0A0AFB00FF00,0xFB00FB0A0A0A0A0A,0x0A0A0B0A0A0A0A0A,0x0A0A08080F080F08,0xF808F8080A0AFA0A,0x08080F080F000808,0x00000A0A0F000000,
	0xF808F8000A0AFE00,0x0808FF00FF080808,0x08080A0AFB0A0A0A,0xF800000000000F08,0xFFFFFFFFFFFF0808,0xFFFFF0F0F0F0F0F0,0xFF000000000000FF,0x0F0F0F0F0F0FFFFF,
	0xFE00241824241800,0x01017F0000344A4A,0x027E027E02000003,0x1800006349556300,0x2020FC00041C2424,0x000478040800001C,0x3E00085577550800,0x02724C00003E4949,
	0x0030595522004C72,0x1800182418241800,0x2A2A1C0018247E24,0x003C02023C00002A,0x0000002A2A2A2A00,0x4A4A510000242E24,0x00514A4A44000044,0x20000402FC000000,
	0x2A08080000003F40,0x0012241224000808,0x0000000609090600,0x0008000000001818,0x02023E4030000000,0x0900000E010E0100,0x3C3C3C0000000A0D,0x000000000000003C,
};

int lbound0(int a, int b) {
    if ((unsigned int)a <= b) return(a);
    return((~(a>>31))&b);
}

int uptil1(unsigned int *lptr, int z) {
    //   //This line does the same thing (but slow & brute force)
    //while ((z > 0) && (!(lptr[(z-1)>>5]&(1<<(z-1))))) z--; return(z);
    int i;
    if (!z) return(0); //Prevent possible crash
    i = (lptr[(z-1)>>5]&((1<<z)-1)); z &= ~31;
    while (!i)
    {
        z -= 32; if (z < 0) return(0);
        i = lptr[z>>5];
    }
    return(bsr(i)+z+1);
}

void print6x8(tiltyp *ldd, int ox, int y, int fcol, int bcol, const char *fmt, ...) {
    va_list arglist;
    char st[1024], *c, *v;
    int i, j, ie, x, *lp, *lpx;

    if (!fmt) return;
    va_start(arglist,fmt);
    if (_vsnprintf((char *)&st,sizeof(st)-1,fmt,arglist)) st[sizeof(st)-1] = 0;
    va_end(arglist);

    lp = (int *)(y*ldd->p+ldd->f);
    for(j=1;j<256;y++,lp=(int *)(((intptr_t)lp)+ldd->p),j+=j)
        if ((unsigned)y < (unsigned)ldd->y)
            for(c=st,x=ox;*c;c++,x+=6)
            {
                v = (char *)(((int)*c)*6 + (intptr_t)font6x8); lpx = &lp[x];
                for(i=max(-x,0),ie=min(ldd->x-x,6);i<ie;i++) { if (v[i]&j) lpx[i] = fcol; else if (bcol >= 0) lpx[i] = bcol; }
                if ((*c) == 9) { if (bcol >= 0) { for(i=max(-x,6),ie=min(ldd->x-x,18);i<ie;i++) lpx[i] = bcol; } x += 2*6; }
            }
}

void drawpix(tiltyp *ldd, int x, int y, int col) {
    if (((unsigned)x < (unsigned)ldd->x) && ((unsigned)y < (unsigned)ldd->y))
        *(int *)(ldd->p*y + (x<<2) + ldd->f) = col;
}

void drawline2d(tiltyp *ldd, float x0, float y0, float x1, float y1, int col) {
    float f;
    int i, x, y, dx, dy, ipx[2], ipy[2];

    if (x0 <      0) { if (x1 <      0) return; y0 = (     0-x0)*(y1-y0)/(x1-x0)+y0; x0 =      0; }
    else if (x0 > ldd->x) { if (x1 > ldd->x) return; y0 = (ldd->x-x0)*(y1-y0)/(x1-x0)+y0; x0 = ldd->x; }
    if (y0 <      0) { if (y1 <      0) return; x0 = (     0-y0)*(x1-x0)/(y1-y0)+x0; y0 =      0; }
    else if (y0 > ldd->y) { if (y1 > ldd->y) return; x0 = (ldd->y-y0)*(x1-x0)/(y1-y0)+x0; y0 = ldd->y; }
    if (x1 <      0) {                          y1 = (     0-x1)*(y1-y0)/(x1-x0)+y1; x1 =      0; }
    else if (x1 > ldd->x) {                          y1 = (ldd->x-x1)*(y1-y0)/(x1-x0)+y1; x1 = ldd->x; }
    if (y1 <      0) {                          x1 = (     0-y1)*(x1-x0)/(y1-y0)+x1; y1 =      0; }
    else if (y1 > ldd->y) {                          x1 = (ldd->y-y1)*(x1-x0)/(y1-y0)+x1; y1 = ldd->y; }

    x1 -= x0; y1 -= y0;
    i = (int)max(fabs(x1),fabs(y1)); if (!(i&0x7fffffff)) return;
    f = 65536.0/(float)i;
    ipx[0] = (int)(x0*65536.0); ipx[1] = (int)(x1*f);
    ipy[0] = (int)(y0*65536.0); ipy[1] = (int)(y1*f);
    for(;i>0;i--)
    {
        x = (ipx[0]>>16); y = (ipy[0]>>16);
        if (((unsigned)x < (unsigned)ldd->x) && ((unsigned)y < (unsigned)ldd->y))
            *(int *)(ldd->p*y + (x<<2) + ldd->f) = col;
        ipx[0] += ipx[1]; ipy[0] += ipy[1];
    }
}

void drawline3d(cam_t *lcam, float x0, float y0, float z0, float x1, float y1, float z1, int col) {
#define SCISDIST .001
    double ox, oy, oz, r;

    ox = x0-lcam->p.x; oy = y0-lcam->p.y; oz = z0-lcam->p.z;
    x0 = ox*lcam->r.x + oy*lcam->r.y + oz*lcam->r.z;
    y0 = ox*lcam->d.x + oy*lcam->d.y + oz*lcam->d.z;
    z0 = ox*lcam->f.x + oy*lcam->f.y + oz*lcam->f.z;
    ox = x1-lcam->p.x; oy = y1-lcam->p.y; oz = z1-lcam->p.z;
    x1 = ox*lcam->r.x + oy*lcam->r.y + oz*lcam->r.z;
    y1 = ox*lcam->d.x + oy*lcam->d.y + oz*lcam->d.z;
    z1 = ox*lcam->f.x + oy*lcam->f.y + oz*lcam->f.z;

    if (z0 < SCISDIST)
    {
        if (z1 < SCISDIST) return;
        r = (SCISDIST-z0)/(z1-z0); z0 = SCISDIST;
        x0 += (x1-x0)*r; y0 += (y1-y0)*r;
    }
    else if (z1 < SCISDIST)
    {
        r = (SCISDIST-z1)/(z1-z0); z1 = SCISDIST;
        x1 += (x1-x0)*r; y1 += (y1-y0)*r;
    }

    ox = lcam->h.z/z0;
    oy = lcam->h.z/z1;
    drawline2d(&lcam->c,x0*ox+lcam->h.x,y0*ox+lcam->h.y,x1*oy+lcam->h.x,y1*oy+lcam->h.y,col);
}

void drawpolsol(cam_t *lcam, point2d *pt, int pn, int lignum) {
    typedef struct { int i0, i1; float pos, inc; } rast_t;
    rast_t *rast, rtmp;
    int i, j, k, x, y, ix0, ix1, iy0, iy1, pn2, pn3, pn4, ymin, ymax;

    rast = (rast_t *)_alloca(pn*sizeof(rast_t));

    ymin = 0x7fffffff; ymax = 0x80000000; pn2 = 0; j = -1; iy1 = 0;
    for(i=0;i<pn;i++)
    {
        if (i != j)                  iy0 = (int)min(max(ceil(pt[i].y),0),lcam->c.y); else iy0 = iy1;
        j = i+1; if (j >= pn) j = 0; iy1 = (int)min(max(ceil(pt[j].y),0),lcam->c.y); if (iy0 == iy1) continue;
        if (iy0 < iy1) { rast[pn2].i0 = i; rast[pn2].i1 = j; if (iy0 < ymin) ymin = iy0; }
        else { rast[pn2].i0 = j; rast[pn2].i1 = i; if (iy0 > ymax) ymax = iy0; }
        pn2++;
    }

    //Shell sort top y's
    for(k=(pn2>>1);k;k>>=1)
        for(i=0;i<pn2-k;i++)
            for(j=i;j>=0;j-=k)
            {
                if (pt[rast[j].i0].y <= pt[rast[j+k].i0].y) break;
                rtmp = rast[j]; rast[j] = rast[j+k]; rast[j+k] = rtmp;
            }

    pn3 = 0; pn4 = 0;
    for(y=ymin;y<ymax;y++)
    {
        for(i=pn3-1;i>=0;i--)
        {
            if ((float)y >= pt[rast[i].i1].y)
            {     //Delete line segments
                pn3--;
                for(j=i;j<pn3;j++) rast[j] = rast[j+1];
            }
            else if (rast[i+1].pos < rast[i].pos)
            {     //Refresh sort (needed for degenerate poly/intersections)
                rtmp = rast[i];
                for(j=i+1;(j < pn3) && (rast[j].pos < rtmp.pos);j++) rast[j-1] = rast[j];
                rast[j-1] = rtmp;
            }
        }

        //Insert line segments
        while ((pn4 < pn2) && ((float)y >= pt[rast[pn4].i0].y))
        {
            rtmp.i0 = rast[pn4].i0; rtmp.i1 = rast[pn4].i1;
            rtmp.inc = (pt[rtmp.i1].x - pt[rtmp.i0].x)/(pt[rtmp.i1].y - pt[rtmp.i0].y);
            rtmp.pos = ((float)y - pt[rtmp.i0].y)*rtmp.inc + pt[rtmp.i0].x;

            for(j=pn3;(j > 0) && (rast[j-1].pos > rtmp.pos);j--) rast[j] = rast[j-1];
            rast[j] = rtmp;

            pn3++; pn4++;
        }

        //Draw hlines xor style
        for(i=0;i<pn3;i+=2)
        {
            ix0 = (int)min(max(rast[i  ].pos,0.f),(float)lcam->c.x);
            ix1 = (int)min(max(rast[i+1].pos,0.f),(float)lcam->c.x);
            if ((y&1) == (lignum&1))
            {
                if ((y&2) == (lignum&2)) x = ((ix0+3)&~3);
                else x = ((ix0+5)&~3)-2;
                for(;x<ix1;x+=4) drawpix(&lcam->c,x,y,0xffffff);
            }
        }
        //Inc x-steps
        for(i=pn3-1;i>=0;i--) rast[i].pos += rast[i].inc;
    }
}

void drawpolsol(cam_t *lcam, point3d *vt, int num, int lignum) {
#define SCISDIST .001
    point3d *vt2;
    point2d *pt;
    float f, g, ox, oy, oz;
    int i, j, pn;

    vt2 = (point3d *)_alloca(num  *sizeof(point3d));
    pt  = (point2d *)_alloca(num*2*sizeof(point2d));

    for(i=0;i<num;i++)
    {
        ox = vt[i].x-lcam->p.x; oy = vt[i].y-lcam->p.y; oz = vt[i].z-lcam->p.z;
        vt2[i].x = ox*lcam->r.x + oy*lcam->r.y + oz*lcam->r.z;
        vt2[i].y = ox*lcam->d.x + oy*lcam->d.y + oz*lcam->d.z;
        vt2[i].z = ox*lcam->f.x + oy*lcam->f.y + oz*lcam->f.z;
    }

    pn = 0;
    for(i=num-1,j=0;j<num;i=j,j++)
    {
        if (vt2[i].z >= SCISDIST)
        {
            f = lcam->h.z/vt2[i].z;
            pt[pn].x = vt2[i].x*f + lcam->h.x;
            pt[pn].y = vt2[i].y*f + lcam->h.y;
            pn++;
        }
        if ((vt2[i].z >= SCISDIST) != (vt2[j].z >= SCISDIST))
        {
            f = (SCISDIST-vt2[i].z)/(vt2[j].z-vt2[i].z); g = lcam->h.z/SCISDIST;
            pt[pn].x = ((vt2[j].x-vt2[i].x)*f + vt2[i].x)*g + lcam->h.x;
            pt[pn].y = ((vt2[j].y-vt2[i].y)*f + vt2[i].y)*g + lcam->h.y;
            pn++;
        }
    }
    if (pn >= 3) drawpolsol(lcam,pt,pn,lignum);
}
#ifndef _MSC_VER
static _inline int argb_interp (int c0, int c1, int mul15)
{
    unsigned char *u0, *u1;
    u0 = (unsigned char *)&c0;
    u1 = (unsigned char *)&c1;
    u0[0] += (unsigned char)((((int)u1[0]-(int)u0[0])*mul15)>>15);
    u0[1] += (unsigned char)((((int)u1[1]-(int)u0[1])*mul15)>>15);
    u0[2] += (unsigned char)((((int)u1[2]-(int)u0[2])*mul15)>>15);
    u0[3] += (unsigned char)((((int)u1[3]-(int)u0[3])*mul15)>>15);
    return(c0);
}
#else
int argb_interp(int c0, int c1, int mul15) {
    _asm
            {
            punpcklbw mm0, c0
            punpcklbw mm1, c1
            psrlw mm0, 8
            psrlw mm1, 8
            psubw mm1, mm0
            paddw mm1, mm1
            pshufw mm2, mul15, 0
            pmulhw mm1, mm2
            paddw mm1, mm0
            packuswb mm1, mm1
            movd eax, mm1
            emms
            }
}
#endif

void drawpoly_flat_threadsafe(tiltyp *tt, vertyp *pt, int pn, int rgbmul, float hsc, float *ouvmat, int rendflags, cam_t &gcam) {
    __declspec(align(16)) static const float dpqmulval[4] = {0,1,2,3}, dpqfours[4] = {4,4,4,4};
#define PR_USEFLOAT 0
#if (PR_USEFLOAT != 0)
    typedef struct { int i0, i1; float pos, inc; } rast_t;
#else
    typedef struct { int i0, i1, pos, inc; } rast_t; //FIX:overflows in BUILD2 easily :/
#endif
    rast_t *rast, rtmp;
    float f, d, u, v, vx, vy, di8, ui8, vi8, od;
    int i, j, k, iy0, iy1, pn2, pn3, pn4;
    __declspec(align(8)) int iw[2], iwi[2];
    intptr_t p, p2, padd, ttf, ttp;
    int id, idi, sx0, sx1, sy;
    int ttps, ymsk, xmsk, *isy;
    __int64 qddmul, qmask;
    int lmask0, lmask1;

    di8 = ouvmat[0]*FLATSTEPSIZ;
    ui8 = ouvmat[1]*FLATSTEPSIZ;
    vi8 = ouvmat[2]*FLATSTEPSIZ;
    rgbmul = ((rgbmul&0xffffff)|0x80000000);
    ttp = tt->p; ttf = tt->f; i = bsr(ttp);
    xmsk = (1<<bsr(tt->x))-1; xmsk <<= 2;
    ymsk = (1<<bsr(tt->y))-1; ymsk <<= i;
    ttps = 16-i;
    qddmul = (__int64)((ttp<<16)+4);
    qmask = (__int64)(((tt->y-1)<<16) + (tt->x-1));
    lmask0 = ((tt->x-1)<<2); lmask1 = ~lmask0;

    rast = (rast_t *)_alloca(pn*sizeof(rast_t));
#if 0
    isy = (int *)_alloca(pn*sizeof(isy[0]));
    for(i=pn-1;i>=0;i--) isy[i] = (int)min(max(ceil(pt[i].y),0.f),(float)gcam.c.y);
#else
    isy = (int *)((((intptr_t)_alloca(pn*sizeof(isy[0])+32))+15)&~15);
    _asm
            {
            mov eax, 0x5f80 ;round +inf
            mov i, eax
            ldmxcsr i

            push edi
            mov eax, pn
            mov edx, pt
            lea ecx, [eax*2+eax]
            lea ecx, [ecx*8+edx-20]
            mov edi, isy
            xorps xmm1, xmm1
            cvtsi2ss xmm2, gcam.c.y
            shufps xmm2, xmm2, 0

            test eax, 3
            jz short isybeg4
            isybeg1:
            movss xmm0, [ecx]
            maxss xmm0, xmm1
            minss xmm0, xmm2
            cvtss2si edx, xmm0
            mov [edi+eax*4-4], edx
            sub ecx, 24
            sub eax, 1
            jle short isyend
            test eax, 3
            jnz short isybeg1

            isybeg4:
            movss xmm7, [ecx   ]
            movss xmm6, [ecx-24]
            movss xmm5, [ecx-48]
            movss xmm4, [ecx-72]
            unpcklps xmm6, xmm7  ;xmm6:[  0    0  xmm7 xmm6]
            unpcklps xmm4, xmm5  ;xmm4:[  0    0  xmm5 xmm4]
            movlhps xmm4, xmm6   ;xmm4:[xmm7 xmm6 xmm5 xmm4]

            maxps xmm4, xmm1
            minps xmm4, xmm2
#if (USESSE2 != 0)
            cvtps2dq xmm4, xmm4
            movaps [edi+eax*4-16], xmm4
#else
            cvtps2pi mm0, xmm4
            movhlps xmm4, xmm4
            cvtps2pi mm1, xmm4
            movq [edi+eax*4-16], mm0
            movq [edi+eax*4-8], mm1
#endif
            sub ecx, 96
            sub eax, 4
            jg short isybeg4

            isyend:

            mov eax, 0x3f80 ;round -inf
            mov i, eax
            ldmxcsr i

            pop edi
#if (USESSE2 == 0)
            emms
#endif
            }
#endif
    pn2 = 0;
    for(i=0;i<pn;i++)
    {
        j = pt[i].n; iy0 = isy[i]; iy1 = isy[j]; if (iy0 == iy1) continue;
        if (iy0 < iy1) { rast[pn2].i0 = i; rast[pn2].i1 = j; }
        else { rast[pn2].i0 = j; rast[pn2].i1 = i; }
        pn2++;
    }
    if (pn2 < 2) return;

    //Shell sort top y's
    //for(k=(pn2>>1);k;k>>=1)
    for(k=1;k<pn2;k=k*2+1);
    for(k>>=1;k>0;k>>=1)
        for(i=0;i<pn2-k;i++)
            for(j=i;(j >= 0) && (isy[rast[j].i0] > isy[rast[j+k].i0]);j-=k)
            { rtmp = rast[j]; rast[j] = rast[j+k]; rast[j+k] = rtmp; }

    pn3 = 0; pn4 = 0;
    for(sy=isy[rast[0].i0];1;sy++)
    {
        for(i=pn3-1;i>=0;i--)
        {
            if (sy >= isy[rast[i].i1])
            {     //Delete line segments
                pn3--; if ((!pn3) && (pn4 >= pn2)) return;
                for(j=i;j<pn3;j++) rast[j] = rast[j+1];
            }
            else if (rast[i+1].pos < rast[i].pos)
            {     //Refresh sort (needed for degenerate poly/intersections)
                rtmp = rast[i];
                for(j=i+1;(j < pn3) && (rast[j].pos < rtmp.pos);j++) rast[j-1] = rast[j];
                rast[j-1] = rtmp;
            }
        }

        //Insert line segments
        while ((pn4 < pn2) && (sy >= isy[rast[pn4].i0]))
        {
            rtmp.i0 = rast[pn4].i0; rtmp.i1 = rast[pn4].i1;
#if (PR_USEFLOAT != 0)
            rtmp.inc = (pt[rtmp.i1].x - pt[rtmp.i0].x)/(pt[rtmp.i1].y - pt[rtmp.i0].y);
            rtmp.pos = ((float)sy - pt[rtmp.i0].y)*rtmp.inc + pt[rtmp.i0].x;
#else
            f = (pt[rtmp.i1].x - pt[rtmp.i0].x)/(pt[rtmp.i1].y - pt[rtmp.i0].y);
            rtmp.inc = ((int)(f*65536.0));
            f = ((float)sy - pt[rtmp.i0].y)*f + pt[rtmp.i0].x;
            rtmp.pos = ((int)(f*65536.0))+32768; //32768 matches old algo best //+65535;
#endif
            for(j=pn3;(j > 0) && (rast[j-1].pos > rtmp.pos);j--) rast[j] = rast[j-1];
            rast[j] = rtmp;

            pn3++; pn4++;
        }

        //Draw hlines xor style
        for(i=0;i<pn3;i+=2)
        {
#if (PR_USEFLOAT != 0)
            sx0 = (int)min(max(rast[i  ].pos,0.f),(float)gcam.c.x); rast[i  ].pos += rast[i  ].inc;
            sx1 = (int)min(max(rast[i+1].pos,0.f),(float)gcam.c.x); rast[i+1].pos += rast[i+1].inc;
#else
            sx0 = lbound0(rast[i  ].pos>>16,gcam.c.x); rast[i  ].pos += rast[i  ].inc;
            sx1 = lbound0(rast[i+1].pos>>16,gcam.c.x); rast[i+1].pos += rast[i+1].inc;
#endif
            if (sx1 <= sx0) continue;
            j = gcam.c.p*sy + (sx1<<2);

            vx = (float)sx0; vy = (float)sy;
            d = ouvmat[0]*vx + ouvmat[3]*vy + ouvmat[6]; f = 1.0/d;
            u = ouvmat[1]*vx + ouvmat[4]*vy + ouvmat[7];
            v = ouvmat[2]*vx + ouvmat[5]*vy + ouvmat[8];
            id    = (int)(  f);
            iw[0] = (int)(u*f);
            iw[1] = (int)(v*f);
            od = d;

            //Render Z's
            padd = gcam.z.f+j; p = ((sx0-sx1)<<2);
#ifndef _MSC_VER
            d = od+di8;
            do
            {
                f = 1.0/d; d += di8;
                idi = ((((int)f)-id)>>LFLATSTEPSIZ);
                p2 = min(p+(FLATSTEPSIZ<<2),0);
                do { *(int *)(padd+p) = id;/*FIX:USEINTZ only!*/ id += idi; p += 4; } while (p < p2);
            } while (p < 0);
#else
            _asm
                    {
                    mov eax, ouvmat
                    mov ecx, p
                    mov edx, padd
                    movss xmm0, od
                    movss xmm1, [eax]

                    add edx, ecx
                    test edx, 12
                    jz short zbufskp1
                    zbufbeg1: rcpss xmm2, xmm0
                    addss xmm0, xmm1
#if (USEINTZ)
                    cvttss2si eax, xmm2
                    mov [edx], eax
#else
                    movss [edx], xmm2
#endif
                    add edx, 4
                    add ecx, 4
                    jge short zbufend
                    test edx, 12
                    jnz short zbufbeg1
                    zbufskp1: sub edx, ecx

                    shufps xmm1, xmm1, 0
                    shufps xmm0, xmm0, 0
                    movaps xmm2, xmm1
                    mulps xmm2, dpqmulval ;{0,1,2,3}
                    mulps xmm1, dpqfours  ;{4,4,4,4}
                    addps xmm0, xmm2

                    add ecx, 16
                    jg short zbufend1

                    zbufbeg4: rcpps xmm2, xmm0
                    addps xmm0, xmm1
#if ((USESSE2 != 0) || (!USEINTZ))
#if (USEINTZ)
                    cvttps2dq xmm2, xmm2
#endif
                    movaps [edx+ecx-16], xmm2
#else
                    cvttps2pi mm0, xmm2
                    movhlps xmm2, xmm2
                    cvttps2pi mm1, xmm2
                    movq [edx+ecx-16], mm0
                    movq [edx+ecx-8], mm1
#endif
                    add ecx, 16
                    jle short zbufbeg4

                    zbufend1: sub ecx, 16
                    jz short zbufend
                    rcpps xmm2, xmm0

#if ((USESSE2 != 0) || (!USEINTZ))
#if (USEINTZ)
                    cvttps2dq xmm2, xmm2
#endif
                    zbufend2: movss [edx+ecx], xmm2
#else
                zbufend2: cvttss2si eax, xmm2
                    mov [edx+ecx], eax
#endif
                    shufps xmm2, xmm2, 0x39
                    add ecx, 4
                    jl short zbufend2
                    zbufend:
#if ((USESSE2 == 0) && (USEINTZ))
                    emms
#endif
                    }
#endif

            //Render colors
            padd = gcam.c.f+j; p = ((sx0-sx1)<<2);
            d = od+di8;
            do
            {
                f = 1.0/d; u += ui8; v += vi8; d += di8;
                iwi[0] = ((((int)(u*f))-iw[0])>>LFLATSTEPSIZ);
                iwi[1] = ((((int)(v*f))-iw[1])>>LFLATSTEPSIZ);
                p2 = min(p+(FLATSTEPSIZ<<2),0);

#if 0
                do //Nearest. Note:Brute force attempt at asm failed
                {
                    *(int *)(padd+p) = rgb_scale(*(int *)(((iw[1]>>ttps)&ymsk) + ((iw[0]>>14)&xmsk) + ttf),rgbmul);
                    //*(int *)(padd+p) = (*(int *)(padd+p+gcam.z.f-gcam.c.f)>>14); //debug zbuf
                    iw[0] += iwi[0]; iw[1] += iwi[1]; p += 4;
                } while (p < p2);
#elif 0
                _asm
                        {
                        punpcklbw mm6, rgbmul
                        psrlw mm6, 1
                        }
                if (!(rendflags&RENDFLAGS_INTERP))
                {
                    do
                    {
                        j = *(int *)(((iw[1]>>ttps)&ymsk) + ((iw[0]>>14)&xmsk) + ttf);
                        _asm
                                {
                                punpcklbw mm0, j
                                pmulhuw mm0, mm6
                                psrlw mm0, 6
                                packuswb mm0, mm0
                                mov eax, padd
                                mov edx, p
                                movd [eax+edx], mm0
                                }
                        iw[0] += iwi[0]; iw[1] += iwi[1]; p += 4;
                    } while (p < p2);
                }
                else
                {
                    do
                    {
                        int r0, g0, b0, r1, g1, b1;
                        unsigned char *u0, *u1, *u2, *u3;
                        j = ((iw[1]>>ttps)&ymsk) + ttf;
                        u0 = (unsigned char *)(j + ( (iw[0]>>14)   &xmsk));
                        u1 = (unsigned char *)(j + (((iw[0]>>14)+4)&xmsk));
                        u2 = (unsigned char *)(u0+ttp);
                        u3 = (unsigned char *)(u1+ttp); j = (iw[0]&65535);
                        b0 = ((((int)u1[0]-(int)u0[0])*j)>>16) + (int)u0[0];
                        g0 = ((((int)u1[1]-(int)u0[1])*j)>>16) + (int)u0[1];
                        r0 = ((((int)u1[2]-(int)u0[2])*j)>>16) + (int)u0[2];
                        b1 = ((((int)u3[0]-(int)u2[0])*j)>>16) + (int)u2[0];
                        g1 = ((((int)u3[1]-(int)u2[1])*j)>>16) + (int)u2[1];
                        r1 = ((((int)u3[2]-(int)u2[2])*j)>>16) + (int)u2[2]; j = (iw[1]&65535);
                        b0 += (((b1-b0)*j)>>16);
                        g0 += (((g1-g0)*j)>>16);
                        r0 += (((r1-r0)*j)>>16);
                        j = (r0<<16)+(g0<<8)+b0;
                        _asm
                                {
                                punpcklbw mm0, j
                                pmulhuw mm0, mm6
                                psrlw mm0, 6
                                packuswb mm0, mm0
                                mov eax, padd
                                mov edx, p
                                movd [eax+edx], mm0
                                }
                        iw[0] += iwi[0]; iw[1] += iwi[1]; p += 4;
                    } while (p < p2);
                }
                _asm emms
#else
                _asm
                        {
                        punpcklbw mm6, rgbmul
                        psrlw mm6, 1
                        }
                if (!(rendflags&RENDFLAGS_INTERP))
                {
                    _asm
                            {
                            push esi
                            push edi

                            movq mm4, iw
                            mov edx, ttf

                            mov ecx, p
                            mov eax, p2
                            mov edi, padd
                            lea edi, [edi+eax]
                            sub ecx, eax
                            near_beg: pshufw mm0, mm4, 0xdd  ;mm0:[? ?    vi  ui]
                            pand mm0, qmask
                            pmaddwd mm0, qddmul    ;mm0:[? ? src32.p 4]
                            movd eax, mm0
                            punpcklbw mm0, [eax+edx]
                            pmulhuw mm0, mm6
                            psrlw mm0, 6
                            packuswb mm0, mm0
                            movd [edi+ecx], mm0
                            paddd mm4, iwi
                            add ecx, 4
                            jl short near_beg

                            movq iw, mm4
                            add ecx, p2
                            mov p, ecx

                            pop edi
                            pop esi
                            }
                }
                else
                {
                    _asm
                            {
                            push ebx
                            push esi
                            push edi

                            movq mm4, iw
                            mov edx, ttf
                            mov esi, ttp
                            add esi, edx

                            mov ecx, p
                            mov eax, p2
                            mov edi, padd
                            lea edi, [edi+eax]
                            sub ecx, eax
                            bilin_beg: pshufw mm0, mm4, 0xdd  ;mm0:[? ?    vi  ui]
                            pand mm0, qmask
                            pmaddwd mm0, qddmul    ;mm0:[? ? src32.p 4]
                            movd eax, mm0
                            movd mm0, [eax+edx]
                            movd mm2, [eax+esi]
                            lea ebx, [eax+4]       ;ui_temp = (ui+1)&(u_width-1)
                            and ebx, lmask0        ;
                            and eax, lmask1        ;
                            add eax, ebx           ;
                            movd mm1, [eax+edx]
                            movd mm3, [eax+esi]
                            pxor mm5, mm5
                            punpcklbw mm0, mm5
                            punpcklbw mm1, mm5
                            punpcklbw mm2, mm5
                            punpcklbw mm3, mm5

                            psubw mm1, mm0
                            psubw mm3, mm2
                            paddw mm1, mm1
                            paddw mm3, mm3

                            pshufw mm5, mm4, 0x00
                            psrlw mm5, 1
                            pmulhw mm1, mm5
                            pmulhw mm3, mm5
                            paddw mm0, mm1
                            paddw mm2, mm3

                            pshufw mm5, mm4, 0xaa
                            psrlw mm5, 1
                            psubw mm2, mm0
                            paddw mm2, mm2
                            pmulhw mm2, mm5
                            paddw mm0, mm2

                            psllw mm0, 2
                            pmulhuw mm0, mm6
                            packuswb mm0, mm0
                            movd [edi+ecx], mm0
                            paddd mm4, iwi
                            add ecx, 4
                            jl short bilin_beg

                            movq iw, mm4
                            add ecx, p2
                            mov p, ecx

                            pop edi
                            pop esi
                            pop ebx
                            }
                }
                _asm emms
#endif
            } while (p < 0);
        }
    }
}

