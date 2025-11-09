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


#define addclipline(dax1, day1, dax2, day2, daoval)      \
{                                                        \
clipit[clipnum].x1 = dax1; clipit[clipnum].y1 = day1; \
clipit[clipnum].x2 = dax2; clipit[clipnum].y2 = day2; \
clipobjectval[clipnum] = daoval;                      \
clipnum++;                                            \
}
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


static char screenalloctype = 255;

#define FASTPALGRIDSIZ 8

typedef struct
{
	long x1, y1, x2, y2;
} linetype;

typedef struct
{
	long sx, sy, z;
	short a, picnum;
	signed char dashade;
	char dapalnum, dastat, pagesleft;
	long cx1, cy1, cx2, cy2;
} permfifotype;

static long setviewcnt = 0;
static long bakvidoption[4];
static long bakframeplace[4], bakxsiz[4], bakysiz[4];
static long bakwindowx1[4], bakwindowy1[4];
static long bakwindowx2[4], bakwindowy2[4];
// Pure C version of nsqrtasm (requires lookup tables)
uint32_t nsqrtasm(uint32_t value);

// Pure C version of msqrtasm
uint32_t msqrtasm(uint32_t value);

// Assuming reciptable is defined elsewhere as:
// extern int32_t reciptable[2048];
// ----------------- internal init and helpers
static int32_t krecipasm(int32_t input);
static void setgotpic(uint32_t index);
static int wallfront(long l1, long l2);
static int32_t spritewallfront(spritetype* s, long w);
static void loadtables();
static void initspritelists();
static int clipinsideboxline(long x, long y, long x1, long y1, long x2, long y2, long walldist);
static int insertspritesect(short sectnum);
static int insertspritestat(short statnum);
static int deletespritesect(short deleteme);
static int deletespritestat(short deleteme);
static int lintersect(long x1, long y1, long z1, long x2, long y2, long z2, long x3,
			   long y3, long x4, long y4, long* intx, long* inty, long* intz);
static short lastwall(short point);
static int rintersect(long x1, long y1, long z1, long vx, long vy, long vz, long x3,
			   long y3, long x4, long y4, long* intx, long* inty, long* intz);
int krand();
static void dorotatesprite(long sx, long sy, long z, short a, short picnum, signed char dashade, char dapalnum, char dastat,
					long cx1, long cy1, long cx2, long cy2);
static long sectorofwall(short theline);
void alignflorslope(short dasect, long x, long y, long z);
static long getpalookup(long davis, long dashade);
static long loopnumofsector(short sectnum, short wallnum);

// ----------------- public to remove --------------
void initengine();
void uninitengine();
void nextpage();
void loadpalette();
int screencapture(char* filename, char inverseit);

// ---------------- public to retain --------
long getangle(long xvect, long yvect);
long ksqrt(long num);
void initksqrt();
void initmouse();
void getmousevalues(short* mousx, short* mousy, short* bstatus);

// ------------------ old render funcs
void drawrooms(long daposx, long daposy, long daposz, short daang, long dahoriz, short dacursectnum);
static void scansector(short sectnum);
static int bunchfront(long b1, long b2);
static void drawalls(long bunch);
static void prepwall(long z, walltype* wal);
static void ceilscan(long x1, long x2, long sectnum);
static void florscan(long x1, long x2, long sectnum);
static void wallscan(long x1, long x2, short* uwal, short* dwal, long* swal, long* lwal);
static void maskwallscan(long x1, long x2, short* uwal, short* dwal, long* swal, long* lwal);
static void transmaskvline(long x);
static void transmaskvline2(long x);
static void transmaskwallscan(long x1, long x2);
static void hline(long xr, long yp);
static void slowhline(long xr, long yp);
static long allocatepermanenttile(short tilenume, long xsiz, long ysiz);
static long readpixel16(long p);
int drawmasks();
static void drawmaskwall(short damaskwallcnt);
static void drawsprite(long snum);
void drawvox(long dasprx, long daspry, long dasprz, long dasprang, long daxscale, long dayscale, char daindex,
			 signed char dashade, char dapal, long* daumost, long* dadmost);
static void ceilspritescan(long x1, long x2);
static void copytilepiece(long tilenume1, long sx1, long sy1, long xsiz, long ysiz, long tilenume2, long sx2, long sy2);
static void ceilspritehline(long x2, long y);
static long animateoffs(short tilenum, short fakevar);
static void printscreeninterrupt();
void drawline256(long x1, long y1, long x2, long y2, char col);
void drawline16(long x1, long y1, long x2, long y2, char col);
void printext256(long xpos, long ypos, short col, short backcol, char name[82], char fontsize);
int setview(long x1, long y1, long x2, long y2);
void setaspect(long daxrange, long daaspect);
static void dosetaspect();
void flushperms();
void qloadkvx(long voxindex, char* filename);
static void initfastcolorlookup(long rscale, long gscale, long bscale);
static long getclosestcol(long r, long g, long b);
void setbrightness(char dabrightness, char* dapal);
void drawmapview(long dax, long day, long zoome, short ang);
static void fillpolygon(long npoints);
void clearview(long dacol);
void clearallviews(long dacol);
void setviewtotile(short tilenume, long xsiz, long ysiz);
void setviewback();
void squarerotatetile(short tilenume);
//void preparemirror(long dax, long day, long daz, short daang, long dahoriz, short dawall, short dasector, long* tposx,long* tposy, short* tang);
//void completemirror();
static long clippoly4(long cx1, long cy1, long cx2, long cy2);
static long clippoly(long npoints, long clipstat);


// ------------ funcs to port ---------------
// load map and output starting pos. should be good.
int loadboard(char* filename, long* daposx, long* daposy, long* daposz, short* daang, short* dacursectnum);
int saveboard(char* filename, long* daposx, long* daposy, long* daposz, short* daang, short* dacursectnum);
int setgamemode(char davidoption, long daxdim, long daydim);
void loadtile(short tilenume);
int loadpics(char* filename);
int clipinsidebox(long x, long y, short wallnum, long walldist);
int inside(long x, long y, short sectnum);
int insertsprite(short sectnum, short statnum);
int deletesprite(short spritenum);
int changespritesect(short spritenum, short newsectnum);
int changespritestat(short spritenum, short newstatnum);
// will be internalized
//short nextsectorneighborz(short sectnum, long thez, short topbottom, short direction);
int cansee(long x1, long y1, long z1, short sect1, long x2, long y2, long z2, short sect2);
int hitscan(long xs, long ys, long zs, short sectnum, long vx, long vy, long vz, short* hitsect, short* hitwall,short* hitsprite, long* hitx, long* hity, long* hitz, unsigned long cliptype);
int inthitscan(long xs, long ys, long zs, short sectnum, long vx, long vy, long vz, short* hitsect, short* hitwall, short* hitsprite, long* hitx, long* hity, long* hitz, unsigned long cliptype);
int neartag(long xs, long ys, long zs, short sectnum, short ange, short* neartagsector, short* neartagwall,
			short* neartagsprite, long* neartaghitdist, long neartagrange, char tagsearch);
void dragpoint(short pointhighlight, long dax, long day);
long clipmove(long* x, long* y, long* z, short* sectnum,
			  long xvect, long yvect,
			  long walldist, long ceildist, long flordist, unsigned long cliptype);
// check
void keepaway(long* x, long* y, long w);

long raytrace(long x3, long y3, long* x4, long* y4);
long pushmove(long* x, long* y, long* z, short* sectnum,
			  long walldist, long ceildist, long flordist, unsigned long cliptype);
// gets valid sector at position. assumes that most of the time it is already in sectnum, otherwise - scan nearby, and then scan all
void updatesector(long x, long y, short* sectnum);
void rotatepoint(long xpivot, long ypivot, long x, long y, short daang, long* x2, long* y2);
void getzrange(long x, long y, long z, short sectnum,
			   long* ceilz, long* ceilhit, long* florz, long* florhit,
			   long walldist, unsigned long cliptype);
void rotatesprite(long sx, long sy, long z, short a, short picnum, signed char dashade, char dapalnum, char dastat,
				  long cx1, long cy1, long cx2, long cy2);
void makepalookup(long palnum, char* remapbuf, signed char r, signed char g, signed char b, char dastat);
long getceilzofslope(short sectnum, long dax, long day);
long getflorzofslope(short sectnum, long dax, long day);
void getzsofslope(short sectnum, long dax, long day, long* ceilz, long* florz);



//---------------- ported

int setsprite(short spritenum, long newx, long newy, long newz);

//----------------------


//Assume npoints=4 with polygon on &rx1,&ry1





#endif
