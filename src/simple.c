#if 0 //To compile, type: nmake simple.c
simple.exe:\
		  simple.obj build2.obj shadowtest2.obj drawpoly.obj drawcone.obj drawkv6.obj morph.obj kplib.obj winmain.obj
	link simple.obj build2.obj shadowtest2.obj drawpoly.obj drawcone.obj drawkv6.obj morph.obj kplib.obj winmain.obj\
	ddraw.lib dinput.lib dxguid.lib ole32.lib user32.lib gdi32.lib winmm.lib /opt:nowin98
	del simple.obj
simple.obj:      simple.c sysmain.h           ; cl /c /J /TP simple.c      /Ox /Ob2 /G6Fy /Gs /MT
build2.obj:      build2.c drawpoly.h sysmain.h; cl /c /J /TP build2.c               /G6Fy /Gs /MT
shadowtest2.obj: shadowtest2.c                ; cl /c /J /TP shadowtest2.c /Ox      /G6Fy /Gs /MT
drawpoly.obj:    drawpoly.c drawpoly.h        ; cl /c /J /TP drawpoly.c    /Ox      /G6Fy /Gs /MT /QIfist
drawcone.obj:    drawcone.c                   ; cl /c /J /TP drawcone.c    /Ox /Ob2 /G6Fy /Gs /MT /QIfist /DUSEINTZ
drawkv6.obj:     drawkv6.c                    ; cl /c /J /TP drawkv6.c     /Ox /Ob2 /G6Fy /Gs /MT /QIfist /DUSEINTZ /DUSEKZ
morph.obj:       morph.c                      ; cl /c /J /TP morph.c       /Ox      /G6Fy /Gs /MT /QIfist
kplib.obj:       kplib.c                      ; cl /c /J /TP kplib.c       /Ox /Ob2 /G6Fy /Gs /MT
winmain.obj:     winmain.cpp                  ; cl /c /J /TP winmain.cpp   /Ox /Ob2 /G6Fy /Gs /MT
!if 0
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "build2.h"
#include "sysmain.h"
extern void drawsph (double, double, double, double, int, double, int);

static tiletype gdd;
static double dtotclk, odtotclk;
static int cursect, bstatus, obstatus;
static dpoint3d ipos, irig, idow, ifor;

static void drawsph (double x, double y, double z, double r, int col)
{
	float nx, ny, nz;
	x -= ipos.x; y -= ipos.y; z -= ipos.z;
	nx = x*irig.x + y*irig.y + z*irig.z;
	ny = x*idow.x + y*idow.y + z*idow.z;
	nz = x*ifor.x + y*ifor.y + z*ifor.z;
	drawsph(nx*256,ny*256,nz*256,r*256,col,38.4,0);
}

void uninitapp () { build2_uninit(); }

long initapp (long argc, char **argv)
{
	xres = 800; yres = 600; colbits = 32; fullscreen = 0; prognam = "BUILD 2.0 by Ken Silverman";
	build2_init();
	build2_loadmap("doortest.map",&cursect,&ipos,&irig,&idow,&ifor);
	return(0);
}

void doframe ()
{
	static long lastspri = -1;
	dpoint3d vec, vadd;
	float f, fx, fy, dtim;
	int s, w, nsect, obstatus;

	readkeyboard(); if (ext_keystatus[1]) { quitloop(); return; }
	odtotclk = dtotclk; readklock(&dtotclk); dtim = dtotclk-odtotclk;
	obstatus = bstatus; readmouse(&fx,&fy,(long *)&bstatus);
	orthorotate(irig.z*.2,fy*.01,fx*.01,&irig,&idow,&ifor);
	f = (keystatus[0x36]+1)/((float)(keystatus[0x2a]+1)); f = f*f*dtim*2.0; //Control speed with shifts
	vec.x = (keystatus[0xcd]-keystatus[0xcb])*f; //Left&Right (Right-Left)
	vec.y = (keystatus[0x52]-keystatus[0x9d])*f; //Up&Down    (KP0-RCTRL)
	vec.z = (keystatus[0xc8]-keystatus[0xd0])*f; //Forw&Back  (Up-Down)
	vadd.x = irig.x*vec.x + idow.x*vec.y + ifor.x*vec.z;
	vadd.y = irig.y*vec.x + idow.y*vec.y + ifor.y*vec.z;
	vadd.z = irig.z*vec.x + idow.z*vec.y + ifor.z*vec.z;
	build2_hitmove(&cursect,&ipos,&vadd,0.25,1,0,0);

	if (bstatus)
	{
		vec.x = ipos.x; vadd.x = ifor.x*256.0;
		vec.y = ipos.y; vadd.y = ifor.y*256.0;
		vec.z = ipos.z; vadd.z = ifor.z*256.0;
		nsect = cursect;
		build2_hitmove(&nsect,&vec,&vadd,0.0,0,&s,&w);
		if (bstatus&1)
		{
			if ((unsigned)s < (unsigned)sst.numsects)
			{
					  if (w <       0) { surf_t *sur = &sst.sect[s].surf[w&1];    sur->rsc = rand(); sur->gsc = rand(); sur->bsc = rand(); }
				else if (w < (1<<30)) { surf_t *sur = &sst.sect[s].wall[w].surf; sur->rsc = rand(); sur->gsc = rand(); sur->bsc = rand(); }
				else                  { spri_t *spr = &sst.spri[w-(1<<30)];      spr->rsc = rand(); spr->gsc = rand(); spr->bsc = rand(); }
			}
		}
	}

	if ((bstatus&2) > (obstatus&2)) lastspri = insspri(s,vec.x,vec.y,vec.z);
	if (lastspri >= 0)
	{
		sst.spri[lastspri].p.x += sin(dtotclk*4.0)*.01;
		sst.sect[lastspri].z[1] += sin(dtotclk*4.0)*.01;
	}

	if (startdirectdraw((long *)&gdd.f,(long *)&gdd.p,(long *)&gdd.x,(long *)&gdd.y))
	{
		build2_render(&gdd,0,cursect,&ipos,&irig,&idow,&ifor,gdd.x*.5,gdd.y*.5,gdd.x*.5);
		if (bstatus) drawsph(vec.x,vec.y,vec.z,0.0625,0x404040);
		stopdirectdraw();
		nextpage();
	}
}

#if 0
!endif
#endif
