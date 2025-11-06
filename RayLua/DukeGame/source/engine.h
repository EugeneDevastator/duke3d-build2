// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
#ifndef KEN_ENGINE_H
#define KEN_ENGINE_H
#pragma once
#define SUPERBUILD

#define ENGINE
//#include <i86.h>
#include "build.h"
#include "cache1d.h"
#include "duke3d.h"
#include "pragmas.h"
#include "funct.h"
extern sectortype sector[MAXSECTORS];

//#include "ves2.h"

#define MAXCLIPNUM 512
#define MAXPERMS 512
#define MAXTILEFILES 256
#define MAXYSAVES ((MAXXDIM*MAXSPRITES)>>7)
#define MAXNODESPERLINE 42   //Warning: This depends on MAXYSAVES & MAXYDIM!
#define MAXWALLSB 2048
#define MAXCLIPDIST 1024


#ifdef SUPERBUILD
	//MUST CALL LOADVOXEL THIS WAY BECAUSE WATCOM STINKS!
void loadvoxel(long voxindex);
void kloadvoxel(long voxindex);


	//These variables need to be copied into BUILD
#define MAXXSIZ 128
#define MAXYSIZ 128
#define MAXZSIZ 200
#define MAXVOXELS 512
#define MAXVOXMIPS 5

#endif

static char kensmessage[128];


#define FASTPALGRIDSIZ 8

typedef struct { long x1, y1, x2, y2; } linetype;

typedef struct
{
	long sx, sy, z;
	short a, picnum;
	signed char dashade;
	char dapalnum, dastat, pagesleft;
	long cx1, cy1, cx2, cy2;
} permfifotype;

// Pure C version of nsqrtasm (requires lookup tables)
uint32_t nsqrtasm(uint32_t value);

// Pure C version of msqrtasm
uint32_t msqrtasm(uint32_t value);

// Assuming reciptable is defined elsewhere as:
// extern int32_t reciptable[2048];

int32_t krecipasm(int32_t input);

void setgotpic(uint32_t index);



void drawrooms(long daposx, long daposy, long daposz,
			 short daang, long dahoriz, short dacursectnum);

void scansector (short sectnum);

int wallfront (long l1, long l2);

int32_t spritewallfront (spritetype *s, long w);

int bunchfront (long b1, long b2);

void drawalls (long bunch);

void prepwall(long z, walltype *wal);

void ceilscan(long x1, long x2, long sectnum);

void florscan(long x1, long x2, long sectnum);

void wallscan(long x1, long x2, short* uwal, short* dwal, long* swal, long* lwal);

void maskwallscan(long x1, long x2, short *uwal, short *dwal, long *swal, long *lwal);

void transmaskvline(long x);

void transmaskvline2(long x);

void transmaskwallscan(long x1, long x2);

int loadboard(char *filename, long *daposx, long *daposy, long *daposz,
		  short *daang, short *dacursectnum);

int saveboard(char *filename, long *daposx, long *daposy, long *daposz,
		  short *daang, short *dacursectnum);

void loadtables();

void loadpalette();

static char screenalloctype = 255;
int setgamemode(char davidoption, long daxdim, long daydim);


void hline(long xr, long yp);

void slowhline(long xr, long yp);

void initengine();

void uninitengine();

void nextpage();


void loadtile(short tilenume);

long allocatepermanenttile(short tilenume, long xsiz, long ysiz);

int loadpics(char* filename);

#ifdef SUPERBUILD
void qloadkvx(long voxindex, char* filename);
#endif

int clipinsidebox(long x, long y, short wallnum, long walldist);

int clipinsideboxline(long x, long y, long x1, long y1, long x2, long y2, long walldist);

long readpixel16(long p);

int screencapture(char *filename, char inverseit);

int inside (long x, long y, short sectnum);

long getangle(long xvect, long yvect);

long ksqrt(long num);

//long krecip(long num);

void initksqrt();

void copytilepiece(long tilenume1, long sx1, long sy1, long xsiz, long ysiz,
			  long tilenume2, long sx2, long sy2);

int drawmasks();

void drawmaskwall(short damaskwallcnt);

void drawsprite(long snum);

#ifdef SUPERBUILD
void drawvox(long dasprx, long daspry, long dasprz, long dasprang,
			 long daxscale, long dayscale, char daindex,
			 signed char dashade, char dapal, long* daumost, long* dadmost);
#endif

void ceilspritescan (long x1, long x2);

void ceilspritehline(long x2, long y);

//int setsprite(short spritenum, long newx, long newy, long newz);

long animateoffs(short tilenum, short fakevar);

void initspritelists();

int insertsprite(short sectnum, short statnum);

int insertspritesect(short sectnum);

int insertspritestat(short statnum);

int deletesprite(short spritenum);

int deletespritesect(short deleteme);

int deletespritestat (short deleteme);

int changespritesect(short spritenum, short newsectnum);

int changespritestat(short spritenum, short newstatnum);

short nextsectorneighborz(short sectnum, long thez, short topbottom, short direction);

int cansee(long x1, long y1, long z1, short sect1, long x2, long y2, long z2, short sect2);

int hitscan(long xs, long ys, long zs, short sectnum, long vx, long vy, long vz, short* hitsect, short* hitwall,
	short* hitsprite, long* hitx, long* hity, long* hitz, unsigned long cliptype);

int inthitscan(long xs, long ys, long zs, short sectnum, long vx, long vy, long vz,
		short *hitsect, short *hitwall, short *hitsprite,
		long *hitx, long *hity, long *hitz, unsigned long cliptype);

int neartag (long xs, long ys, long zs, short sectnum, short ange, short *neartagsector, short *neartagwall, short *neartagsprite, long *neartaghitdist, long neartagrange, char tagsearch);

int lintersect(long x1, long y1, long z1, long x2, long y2, long z2, long x3,
		   long y3, long x4, long y4, long *intx, long *inty, long *intz);

int rintersect(long x1, long y1, long z1, long vx, long vy, long vz, long x3,
		   long y3, long x4, long y4, long *intx, long *inty, long *intz);

void dragpoint(short pointhighlight, long dax, long day);

short lastwall(short point);

#define addclipline(dax1, day1, dax2, day2, daoval)      \
{                                                        \
	clipit[clipnum].x1 = dax1; clipit[clipnum].y1 = day1; \
	clipit[clipnum].x2 = dax2; clipit[clipnum].y2 = day2; \
	clipobjectval[clipnum] = daoval;                      \
	clipnum++;                                            \
}                                                        \


long clipmove(long* x, long* y, long* z, short* sectnum,
			  long xvect, long yvect,
			  long walldist, long ceildist, long flordist, unsigned long cliptype);

void keepaway(long* x, long* y, long w);

long raytrace(long x3, long y3, long* x4, long* y4);

long pushmove(long* x, long* y, long* z, short* sectnum,
			  long walldist, long ceildist, long flordist, unsigned long cliptype);

void updatesector(long x, long y, short* sectnum);

void rotatepoint(long xpivot, long ypivot, long x, long y, short daang, long* x2, long* y2);

void initmouse();

void getmousevalues(short* mousx, short* mousy, short* bstatus);

void printscreeninterrupt();

void drawline256(long x1, long y1, long x2, long y2, char col);

void drawline16(long x1, long y1, long x2, long y2, char col);

void printext256(long xpos, long ypos, short col, short backcol, char name[82], char fontsize);

int krand();

void getzrange(long x, long y, long z, short sectnum,
			   long* ceilz, long* ceilhit, long* florz, long* florhit,
			   long walldist, unsigned long cliptype);

int setview(long x1, long y1, long x2, long y2);

void setaspect(long daxrange, long daaspect);

void dosetaspect();

void flushperms();

void rotatesprite(long sx, long sy, long z, short a, short picnum, signed char dashade, char dapalnum, char dastat,
				  long cx1, long cy1, long cx2, long cy2);

void dorotatesprite(long sx, long sy, long z, short a, short picnum, signed char dashade, char dapalnum, char dastat,
					long cx1, long cy1, long cx2, long cy2);

//Assume npoints=4 with polygon on &rx1,&ry1
long clippoly4(long cx1, long cy1, long cx2, long cy2);

void makepalookup(long palnum, char* remapbuf, signed char r, signed char g, signed char b, char dastat);

void initfastcolorlookup(long rscale, long gscale, long bscale);

long getclosestcol(long r, long g, long b);

void setbrightness(char dabrightness, char *dapal);

void drawmapview(long dax, long day, long zoome, short ang);

long clippoly (long npoints, long clipstat);

void fillpolygon(long npoints);

void clearview(long dacol);

void clearallviews(long dacol);
/*
plotpixel(long x, long y, char col)
{
	drawpixel(ylookup[y]+x+frameplace,(long)col);
}

char getpixel(long x, long y)
{
	return(readpixel(ylookup[y]+x+frameplace));
}
*/
	//MUST USE RESTOREFORDRAWROOMS AFTER DRAWING
static long setviewcnt = 0;
static long bakvidoption[4];
static long bakframeplace[4], bakxsiz[4], bakysiz[4];
static long bakwindowx1[4], bakwindowy1[4];
static long bakwindowx2[4], bakwindowy2[4];

void setviewtotile(short tilenume, long xsiz, long ysiz);

void setviewback();

void squarerotatetile(short tilenume);

void preparemirror(long dax, long day, long daz, short daang, long dahoriz, short dawall, short dasector, long* tposx,
				   long* tposy, short* tang);

void completemirror();

long sectorofwall(short theline);

long getceilzofslope(short sectnum, long dax, long day);

long getflorzofslope(short sectnum, long dax, long day);

void getzsofslope(short sectnum, long dax, long day, long* ceilz, long* florz);

void alignflorslope(short dasect, long x, long y, long z);

long getpalookup(long davis, long dashade);

extern long loopnumofsector(short sectnum, short wallnum);


#endif
