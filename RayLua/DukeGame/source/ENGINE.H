// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
#ifndef KEN_ENGINE_H
#define KEN_ENGINE_H
#define SUPERBUILD

#define ENGINE
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <conio.h>
//#include <i86.h>
#include "build.h"
#include "CACHE1D.H"
#include "duke3d.h"
#include "pragmas.h"

long stereowidth = 23040, stereopixelwidth = 28, ostereopixelwidth = -1;
volatile long stereomode = 0, visualpage, activepage, whiteband, blackband;
volatile char oa1, o3c2, ortca, ortcb, overtbits, laststereoint;

//#include "ves2.h"

#pragma intrinsic(min);
#pragma intrinsic(max);

#define MAXCLIPNUM 512
#define MAXPERMS 512
#define MAXTILEFILES 256
#define MAXYSAVES ((MAXXDIM*MAXSPRITES)>>7)
#define MAXNODESPERLINE 42   //Warning: This depends on MAXYSAVES & MAXYDIM!
#define MAXWALLSB 2048
#define MAXCLIPDIST 1024

	//MUST CALL MALLOC THIS WAY TO FORCE CALLS TO KMALLOC!

void *kkmalloc(size_t size);
#pragma aux kkmalloc =\
	"call kmalloc",\
	parm [eax]\

	//MUST CALL FREE THIS WAY TO FORCE CALLS TO KFREE!



#ifdef SUPERBUILD
	//MUST CALL LOADVOXEL THIS WAY BECAUSE WATCOM STINKS!
void loadvoxel(long voxindex) { }
void kloadvoxel(long voxindex);
#pragma aux kloadvoxel =\
	"call loadvoxel",\
	parm [eax]\

	//These variables need to be copied into BUILD
#define MAXXSIZ 128
#define MAXYSIZ 128
#define MAXZSIZ 200
#define MAXVOXELS 512
#define MAXVOXMIPS 5
long voxoff[MAXVOXELS][MAXVOXMIPS], voxlock[MAXVOXELS][MAXVOXMIPS];
static long ggxinc[MAXXSIZ+1], ggyinc[MAXXSIZ+1];
static long lowrecip[1024], nytooclose, nytoofar;
static unsigned long distrecip[16384];
#endif

//static char moustat = 0;

long transarea = 0, totalarea = 0, beforedrawrooms = 1;

static long oxdimen = -1, oviewingrange = -1, oxyaspect = -1;

static long curbrightness = 0;

	//Textured Map variables
static char globalpolytype;
static short *dotp1[MAXYDIM], *dotp2[MAXYDIM];

static unsigned char tempbuf[MAXWALLS];

long ebpbak, espbak;
long slopalookup[2048];

static char permanentlock = 255;
long artversion, mapversion;
char *pic = NULL;
char picsiz[MAXTILES], tilefilenum[MAXTILES];
long lastageclock;
long tilefileoffs[MAXTILES];

static long artsize = 0, cachesize = 0;

static short radarang[1280], radarang2[MAXXDIM];
static unsigned short sqrtable[4096], shlookup[4096+256];
char pow2char[8] = {1,2,4,8,16,32,64,128};
long pow2long[32] =
{
	1L,2L,4L,8L,
	16L,32L,64L,128L,
	256L,512L,1024L,2048L,
	4096L,8192L,16384L,32768L,
	65536L,131072L,262144L,524288L,
	1048576L,2097152L,4194304L,8388608L,
	16777216L,33554432L,67108864L,134217728L,
	268435456L,536870912L,1073741824L,2147483647L,
};
long reciptable[2048], fpuasm;

char britable[16][64];
char textfont[1024], smalltextfont[1024];

static char kensmessage[128];
#pragma aux getkensmessagecrc =\
	"xor eax, eax",\
	"mov ecx, 32",\
	"beg: mov edx, dword ptr [ebx+ecx*4-4]",\
	"ror edx, cl",\
	"adc eax, edx",\
	"bswap eax",\
	"loop short beg",\
	parm [ebx]\
	modify exact [eax ebx ecx edx]\

static long xb1[MAXWALLSB], yb1[MAXWALLSB], xb2[MAXWALLSB], yb2[MAXWALLSB];
static long rx1[MAXWALLSB], ry1[MAXWALLSB], rx2[MAXWALLSB], ry2[MAXWALLSB];
static short p2[MAXWALLSB], thesector[MAXWALLSB], thewall[MAXWALLSB];

static short bunchfirst[MAXWALLSB], bunchlast[MAXWALLSB];

static short smost[MAXYSAVES], smostcnt;
static short smoststart[MAXWALLSB];
static char smostwalltype[MAXWALLSB];
static long smostwall[MAXWALLSB], smostwallcnt = -1L;

static short maskwall[MAXWALLSB], maskwallcnt;
static long spritesx[MAXSPRITESONSCREEN];
static long spritesy[MAXSPRITESONSCREEN+1];
static long spritesz[MAXSPRITESONSCREEN];
static spritetype *tspriteptr[MAXSPRITESONSCREEN];

short umost[MAXXDIM], dmost[MAXXDIM];
static short bakumost[MAXXDIM], bakdmost[MAXXDIM];
short uplc[MAXXDIM], dplc[MAXXDIM];
static short uwall[MAXXDIM], dwall[MAXXDIM];
static long swplc[MAXXDIM], lplc[MAXXDIM];
static long swall[MAXXDIM], lwall[MAXXDIM+4];
long xdimen = -1, xdimenrecip, halfxdimen, xdimenscale, xdimscale;
long wx1, wy1, wx2, wy2, ydimen;
long viewoffset, frameoffset;

static long rxi[8], ryi[8], rzi[8], rxi2[8], ryi2[8], rzi2[8];
static long xsi[8], ysi[8], *horizlookup, *horizlookup2, horizycent;

long globalposx, globalposy, globalposz, globalhoriz;
short globalang, globalcursectnum;
long globalpal, cosglobalang, singlobalang;
long cosviewingrangeglobalang, sinviewingrangeglobalang;
char *globalpalwritten;
long globaluclip, globaldclip, globvis;
long globalvisibility, globalhisibility, globalpisibility, globalcisibility;
char globparaceilclip, globparaflorclip;

long xyaspect, viewingrangerecip;

long asm1, asm2, asm3, asm4;
long vplce[4], vince[4], palookupoffse[4], bufplce[4];
char globalxshift, globalyshift;
long globalxpanning, globalypanning, globalshade;
short globalpicnum, globalshiftval;
long globalzd, globalbufplc, globalyscale, globalorientation;
long globalx1, globaly1, globalx2, globaly2, globalx3, globaly3, globalzx;
long globalx, globaly, globalz;

static short sectorborder[256], sectorbordercnt;
static char tablesloaded = 0;
long pageoffset, ydim16, qsetmode = 0;
long startposx, startposy, startposz;
short startang, startsectnum;
short pointhighlight, linehighlight, highlightcnt;
static long lastx[MAXYDIM];
char *transluc = NULL, paletteloaded = 0;

#define FASTPALGRIDSIZ 8
static long rdist[129], gdist[129], bdist[129];
static char colhere[((FASTPALGRIDSIZ+2)*(FASTPALGRIDSIZ+2)*(FASTPALGRIDSIZ+2))>>3];
static char colhead[(FASTPALGRIDSIZ+2)*(FASTPALGRIDSIZ+2)*(FASTPALGRIDSIZ+2)];
static long colnext[256];
static char coldist[8] = {0,1,2,3,4,3,2,1};
static long colscan[27];

static short clipnum, hitwalls[4];
long hitscangoalx = (1<<29)-1, hitscangoaly = (1<<29)-1;

typedef struct { long x1, y1, x2, y2; } linetype;
static linetype clipit[MAXCLIPNUM];
static short clipsectorlist[MAXCLIPNUM], clipsectnum;
static short clipobjectval[MAXCLIPNUM];

typedef struct
{
	long sx, sy, z;
	short a, picnum;
	signed char dashade;
	char dapalnum, dastat, pagesleft;
	long cx1, cy1, cx2, cy2;
} permfifotype;
static permfifotype permfifo[MAXPERMS];
static long permhead = 0, permtail = 0;

short numscans, numhits, numbunches;
static short posfil, capturecount = 0, hitcnt;

static char pcxheader[128] =
{
	0xa,0x5,0x1,0x8,0x0,0x0,0x0,0x0,0x3f,0x1,0xc7,0x0,
	0x40,0x1,0xc8,0x0,0x0,0x0,0x0,0x8,0x8,0x8,0x10,0x10,
	0x10,0x18,0x18,0x18,0x20,0x20,0x20,0x28,0x28,0x28,0x30,0x30,
	0x30,0x38,0x38,0x38,0x40,0x40,0x40,0x48,0x48,0x48,0x50,0x50,
	0x50,0x58,0x58,0x58,0x60,0x60,0x60,0x68,0x68,0x68,0x70,0x70,
	0x70,0x78,0x78,0x78,0x0,0x1,0x40,0x1,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
};
static char vgapal16[48] =
{
	00,00,00,00,00,42,00,42,00,00,42,42,42,00,00,42,00,42,42,21,00,42,42,42,
	21,21,21,21,21,63,21,63,21,21,63,63,63,21,21,63,21,63,63,63,21,63,63,63,
};

short editstatus = 0;
short searchit;
long searchx = -1, searchy;                          //search input
short searchsector, searchwall, searchstat;     //search output

static char artfilename[20];
static long numtilefiles, artfil = -1, artfilnum, artfilplc;

static char inpreparemirror = 0;
static long mirrorsx1, mirrorsy1, mirrorsx2, mirrorsy2;

long totalclocklock;

extern long mmxoverlay();
#pragma aux mmxoverlay modify [eax ebx ecx edx];
extern long sethlinesizes(long,long,long);
#pragma aux sethlinesizes parm [eax][ebx][ecx];
extern long setpalookupaddress(char *);
#pragma aux setpalookupaddress parm [eax];
extern long setuphlineasm4(long,long);
#pragma aux setuphlineasm4 parm [eax][ebx];
extern long hlineasm4(long,long,long,long,long,long);
#pragma aux hlineasm4 parm [eax][ebx][ecx][edx][esi][edi];
extern long setuprhlineasm4(long,long,long,long,long,long);
#pragma aux setuprhlineasm4 parm [eax][ebx][ecx][edx][esi][edi];
extern long rhlineasm4(long,long,long,long,long,long);
#pragma aux rhlineasm4 parm [eax][ebx][ecx][edx][esi][edi];
extern long setuprmhlineasm4(long,long,long,long,long,long);
#pragma aux setuprmhlineasm4 parm [eax][ebx][ecx][edx][esi][edi];
extern long rmhlineasm4(long,long,long,long,long,long);
#pragma aux rmhlineasm4 parm [eax][ebx][ecx][edx][esi][edi];
extern long setupqrhlineasm4(long,long,long,long,long,long);
#pragma aux setupqrhlineasm4 parm [eax][ebx][ecx][edx][esi][edi];
extern long qrhlineasm4(long,long,long,long,long,long);
#pragma aux qrhlineasm4 parm [eax][ebx][ecx][edx][esi][edi];
extern long setvlinebpl(long);
#pragma aux setvlinebpl parm [eax];
extern long fixtransluscence(long);
#pragma aux fixtransluscence parm [eax];
extern long prevlineasm1(long,long,long,long,long,long);
#pragma aux prevlineasm1 parm [eax][ebx][ecx][edx][esi][edi];
extern long vlineasm1(long,long,long,long,long,long);
#pragma aux vlineasm1 parm [eax][ebx][ecx][edx][esi][edi];
extern long setuptvlineasm(long);
#pragma aux setuptvlineasm parm [eax];
extern long tvlineasm1(long,long,long,long,long,long);
#pragma aux tvlineasm1 parm [eax][ebx][ecx][edx][esi][edi];
extern long setuptvlineasm2(long,long,long);
#pragma aux setuptvlineasm2 parm [eax][ebx][ecx];
extern long tvlineasm2(long,long,long,long,long,long);
#pragma aux tvlineasm2 parm [eax][ebx][ecx][edx][esi][edi];
extern long mvlineasm1(long,long,long,long,long,long);
#pragma aux mvlineasm1 parm [eax][ebx][ecx][edx][esi][edi];
extern long setupvlineasm(long);
#pragma aux setupvlineasm parm [eax];
extern long vlineasm4(long,long);
#pragma aux vlineasm4 parm [ecx][edi] modify [eax ebx ecx edx esi edi];
extern long setupmvlineasm(long);
#pragma aux setupmvlineasm parm [eax];
extern long mvlineasm4(long,long);
#pragma aux mvlineasm4 parm [ecx][edi] modify [eax ebx ecx edx esi edi];
extern void setupspritevline(long,long,long,long,long,long);
#pragma aux setupspritevline parm [eax][ebx][ecx][edx][esi][edi];
extern void spritevline(long,long,long,long,long,long);
#pragma aux spritevline parm [eax][ebx][ecx][edx][esi][edi];
extern void msetupspritevline(long,long,long,long,long,long);
#pragma aux msetupspritevline parm [eax][ebx][ecx][edx][esi][edi];
extern void mspritevline(long,long,long,long,long,long);
#pragma aux mspritevline parm [eax][ebx][ecx][edx][esi][edi];
extern void tsetupspritevline(long,long,long,long,long,long);
#pragma aux tsetupspritevline parm [eax][ebx][ecx][edx][esi][edi];
extern void tspritevline(long,long,long,long,long,long);
#pragma aux tspritevline parm [eax][ebx][ecx][edx][esi][edi];
extern long mhline(long,long,long,long,long,long);
#pragma aux mhline parm [eax][ebx][ecx][edx][esi][edi];
extern long mhlineskipmodify(long,long,long,long,long,long);
#pragma aux mhlineskipmodify parm [eax][ebx][ecx][edx][esi][edi];
extern long msethlineshift(long,long);
#pragma aux msethlineshift parm [eax][ebx];
extern long thline(long,long,long,long,long,long);
#pragma aux thline parm [eax][ebx][ecx][edx][esi][edi];
extern long thlineskipmodify(long,long,long,long,long,long);
#pragma aux thlineskipmodify parm [eax][ebx][ecx][edx][esi][edi];
extern long tsethlineshift(long,long);
#pragma aux tsethlineshift parm [eax][ebx];
extern long setupslopevlin(long,long,long);
#pragma aux setupslopevlin parm [eax][ebx][ecx] modify [edx];
extern long slopevlin(long,long,long,long,long,long);
#pragma aux slopevlin parm [eax][ebx][ecx][edx][esi][edi];
extern long settransnormal();
#pragma aux settransnormal parm;
extern long settransreverse();
#pragma aux settransreverse parm;
extern long setupdrawslab(long,long);
#pragma aux setupdrawslab parm [eax][ebx];
extern long drawslab(long,long,long,long,long,long);
#pragma aux drawslab parm [eax][ebx][ecx][edx][esi][edi];

// Pure C version of nsqrtasm (requires lookup tables)
uint32_t nsqrtasm(uint32_t value) {
	uint32_t temp = value;
	uint16_t shift_info;

	if (value & 0xff000000) {
		temp >>= 24;
		shift_info = shlookup[temp + 4096]; // +4096 for offset 8192/2
	} else {
		temp >>= 12;
		shift_info = shlookup[temp];
	}

	value >>= (shift_info & 0xff);
	value = sqrtable[value];
	value >>= (shift_info >> 8);

	return value;
}

// Pure C version of msqrtasm
uint32_t msqrtasm(uint32_t value) {
	uint32_t result = 0x40000000;
	uint32_t bit = 0x20000000;

	while (bit != 0) {
		if (value >= result) {
			value -= result;
			result += bit << 2;
		}
		result -= bit;
		result >>= 1;
		bit >>= 2;
	}

	if (value >= result) {
		result++;
	}

	return result >> 1;
}

// Assuming reciptable is defined elsewhere as:
// extern int32_t reciptable[2048];

int32_t krecipasm(int32_t input) {
	union {
		float f;
		int32_t i;
	} fpuasm;

	// Convert integer to float and back to get IEEE 754 representation
	fpuasm.f = (float)input;

	// Extract the original input for further processing
	int32_t eax = fpuasm.i;
	int32_t ebx, ecx;

	// add eax, eax + sbb ebx, ebx (sets ebx to -1 if carry, 0 otherwise)
	uint64_t temp = (uint64_t)eax + (uint64_t)eax;
	ebx = (temp > 0xFFFFFFFF) ? -1 : 0;
	eax = (int32_t)temp;

	// mov ecx, eax
	ecx = eax;

	// and eax, 0x007ff000 + shr eax, 10
	eax = (eax & 0x007ff000) >> 10;

	// sub ecx, 0x3f800000 + shr ecx, 23
	ecx = (ecx - 0x3f800000) >> 23;

	// mov eax, dword ptr reciptable[eax]
	eax = reciptable[eax];

	// sar eax, cl (arithmetic right shift)
	eax = eax >> ecx;

	// xor eax, ebx
	eax = eax ^ ebx;

	return eax;
}

void setgotpic(uint32_t index) {
	if (walock[index] < 200) {
		walock[index] = 199;
	}

	uint32_t byte_index = index >> 3;
	uint32_t bit_mask = 1 << (index & 7);

	gotpic[byte_index] |= bit_mask;
}

#pragma aux getclipmask =\
	"sar eax, 31",\
	"add ebx, ebx",\
	"adc eax, eax",\
	"add ecx, ecx",\
	"adc eax, eax",\
	"add edx, edx",\
	"adc eax, eax",\
	"mov ebx, eax",\
	"shl ebx, 4",\
	"or al, 0xf0",\
	"xor eax, ebx",\
	parm [eax][ebx][ecx][edx]\
	modify exact [eax ebx ecx edx]\

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

long dommxoverlay = 1;
void initengine();

void uninitengine();

void nextpage();

char cachedebug = 0;
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

long krecip(long num);

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

int setsprite(short spritenum, long newx, long newy, long newz);

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

long clipmoveboxtracenum = 3;
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

/*
qsetmode640350()
{
	if (qsetmode != 350)
	{
		stereomode = 0;

		setvmode(0x10);

		pageoffset = 0;
		ydim16 = 350;
		koutpw(0x3d4,0xc+((pageoffset>>11)<<8));

		koutpw(0x3ce,0x0f00);  //set/reset
		koutpw(0x3ce,0x0f01);  //enable set/reset
		fillscreen16(0L,0L,640L*350L);
	}
	qsetmode = 350;
}

qsetmode640480()
{
	short i;

	if (qsetmode != 480)
	{
		stereomode = 0;

		setvmode(0x12);

		i = 479-144;
		koutpw(0x3d4,0x18+((i&255)<<8));             //line compare
		koutp(0x3d4,0x7); koutp(0x3d5,(kinp(0x3d5)&239)|((i&256)>>4));
		koutp(0x3d4,0x9); koutp(0x3d5,(kinp(0x3d5)&191)|((i&512)>>3));

		pageoffset = 92160;
		koutpw(0x3d4,0xc+((pageoffset>>11)<<8));

		koutpw(0x3ce,0x0f00);  //set/reset
		koutpw(0x3ce,0x0f01);  //enable set/reset
		fillscreen16(0L,8L,640L*144L);
		fillscreen16((640L*144L)>>3,0L,640L*336L);
		pageoffset = 92160; ydim16 = 336;
	}

	qsetmode = 480;
}

clear2dscreen()
{
	if (qsetmode == 350)
		fillscreen16(pageoffset>>3,0L,640L*350L);
	else if (qsetmode == 480)
	{
		if (ydim16 <= 336) fillscreen16(pageoffset>>3,0L,640L*336L);
						  else fillscreen16(pageoffset>>3,0L,640L*480L);
	}
}

draw2dgrid(long posxe, long posye, short ange, long zoome, short gride)
{
	long i, xp1, yp1, xp2, yp2, tempy, templong;
	char mask;

	if (gride > 0)
	{
		yp1 = 200-mulscale14(posye+131072,zoome);
		if (yp1 < 0) yp1 = 0;
		yp2 = 200-mulscale14(posye-131072,zoome);
		if (yp2 >= ydim16) yp2 = ydim16-1;

		if ((yp1 < ydim16) && (yp2 >= 0) && (yp2 >= yp1))
		{
			setcolor16(8);
			koutp(0x3ce,0x8);

			templong = ((yp1*640+pageoffset)>>3)+0xa0000;
			tempy = yp2-yp1+1;
			mask = 0;
			xp1 = 320-mulscale14(posxe+131072,zoome);

			for(i=-131072;i<=131072;i+=(2048>>gride))
			{
				xp2 = xp1;
				xp1 = 320-mulscale14(posxe-i,zoome);

				if (xp1 >= 640) break;
				if (xp1 >= 0)
				{
					if ((xp1|7) != (xp2|7))
					{
						koutp(0x3cf,mask);
						if (((xp2>>3) >= 0) && ((xp2>>3) < 80))
							vlin16first(templong+(xp2>>3),tempy);
						mask = 0;
					}
					mask |= pow2char[xp1&7^7];
				}
			}
			if ((i >= 131072) && (xp1 < 640))
				xp2 = xp1;
			if ((mask != 0) && ((xp2>>3) >= 0) && ((xp2>>3) < 80))
			{
				koutp(0x3cf,mask);
				vlin16first(templong+(xp2>>3),tempy);
			}
		}

		xp1 = mulscale14(posxe+131072,zoome);
		xp2 = mulscale14(posxe-131072,zoome);
		tempy = 0x80000000;
		for(i=-131072;i<=131072;i+=(2048>>gride))
		{
			yp1 = (((posye-i)*zoome)>>14);
			if (yp1 != tempy)
			{
				if ((yp1 > 200-ydim16) && (yp1 <= 200))
				{
					drawline16(320-xp1,200-yp1,320-xp2,200-yp1,8);
					tempy = yp1;
				}
			}
		}
	}
}

draw2dscreen(long posxe, long posye, short ange, long zoome, short gride)
{
	walltype *wal;
	long i, j, k, xp1, yp1, xp2, yp2, tempy, templong;
	char col, mask;

	if (qsetmode == 200) return;

	if (editstatus == 0)
	{
		faketimerhandler();
		clear2dscreen();

		faketimerhandler();
		draw2dgrid(posxe,posye,ange,zoome,gride);
	}

	faketimerhandler();
	for(i=numwalls-1,wal=&wall[i];i>=0;i--,wal--)
	{
		if (editstatus == 0)
		{
			if ((show2dwall[i>>3]&pow2char[i&7]) == 0) continue;
			j = wal->nextwall;
			if ((j >= 0) && (i > j))
				if ((show2dwall[j>>3]&pow2char[j&7]) > 0) continue;
		}
		else
		{
			j = wal->nextwall;
			if ((j >= 0) && (i > j)) continue;
		}

		if (j < 0)
		{
			col = 7;
			if (i == linehighlight) col += ((numframes&2)<<2);
		}
		else
		{
			col = 4;
			if ((wal->cstat&1) != 0) col = 5;
			if ((i == linehighlight) || ((linehighlight >= 0) && (i == wall[linehighlight].nextwall)))
				col += ((numframes&2)<<2);
		}

		xp1 = mulscale14(wal->x-posxe,zoome);
		yp1 = mulscale14(wal->y-posye,zoome);
		xp2 = mulscale14(wall[wal->point2].x-posxe,zoome);
		yp2 = mulscale14(wall[wal->point2].y-posye,zoome);

		if ((wal->cstat&64) > 0)
		{
			if (klabs(xp2-xp1) >= klabs(yp2-yp1))
			{
				drawline16(320+xp1,200+yp1+1,320+xp2,200+yp2+1,col);
				drawline16(320+xp1,200+yp1-1,320+xp2,200+yp2-1,col);
			}
			else
			{
				drawline16(320+xp1+1,200+yp1,320+xp2+1,200+yp2,col);
				drawline16(320+xp1-1,200+yp1,320+xp2-1,200+yp2,col);
			}
			col += 8;
		}
		drawline16(320+xp1,200+yp1,320+xp2,200+yp2,col);

		if ((zoome >= 256) && (editstatus == 1))
			if (((320+xp1) >= 2) && ((320+xp1) <= 637))
				if (((200+yp1) >= 2) && ((200+yp1) <= ydim16-3))
				{
					col = 2;
					if (i == pointhighlight) col += ((numframes&2)<<2);
					else if ((highlightcnt > 0) && (editstatus == 1))
					{
						if (show2dwall[i>>3]&pow2char[i&7])
							col += ((numframes&2)<<2);
					}

					templong = (mul5(200+yp1)<<7)+(320+xp1)+pageoffset;
					setcolor16((long)col);
					drawpixel16(templong-2-1280);
					drawpixel16(templong-1-1280);
					drawpixel16(templong+0-1280);
					drawpixel16(templong+1-1280);
					drawpixel16(templong+2-1280);

					drawpixel16(templong-2+1280);
					drawpixel16(templong-1+1280);
					drawpixel16(templong+0+1280);
					drawpixel16(templong+1+1280);
					drawpixel16(templong+2+1280);

					drawpixel16(templong-2-640);
					drawpixel16(templong-2+0);
					drawpixel16(templong-2+640);

					drawpixel16(templong+2-640);
					drawpixel16(templong+2+0);
					drawpixel16(templong+2+640);
				}
	}
	faketimerhandler();

	if ((zoome >= 256) || (editstatus == 0))
		for(i=0;i<numsectors;i++)
			for(j=headspritesect[i];j>=0;j=nextspritesect[j])
				if ((editstatus == 1) || (show2dsprite[j>>3]&pow2char[j&7]))
				{
					col = 3;
					if ((sprite[j].cstat&1) > 0) col = 5;
					if (editstatus == 1)
					{
						if (j+16384 == pointhighlight)
							col += ((numframes&2)<<2);
						else if ((highlightcnt > 0) && (editstatus == 1))
						{
							if (show2dsprite[j>>3]&pow2char[j&7])
								col += ((numframes&2)<<2);
						}
					}

					xp1 = mulscale14(sprite[j].x-posxe,zoome);
					yp1 = mulscale14(sprite[j].y-posye,zoome);
					if (((320+xp1) >= 2) && ((320+xp1) <= 637))
						if (((200+yp1) >= 2) && ((200+yp1) <= ydim16-3))
						{
							templong = (mul5(200+yp1)<<7)+(320+xp1)+pageoffset;
							setcolor16((long)col);
							drawpixel16(templong-1-1280);
							drawpixel16(templong+0-1280);
							drawpixel16(templong+1-1280);

							drawpixel16(templong-1+1280);
							drawpixel16(templong+0+1280);
							drawpixel16(templong+1+1280);

							drawpixel16(templong-2-640);
							drawpixel16(templong-2+0);
							drawpixel16(templong-2+640);

							drawpixel16(templong+2-640);
							drawpixel16(templong+2+0);
							drawpixel16(templong+2+640);

							drawpixel16(templong+1+640);
							drawpixel16(templong-1+640);
							drawpixel16(templong+1-640);
							drawpixel16(templong-1-640);

							xp2 = mulscale11(sintable[(sprite[j].ang+2560)&2047],zoome) / 768;
							yp2 = mulscale11(sintable[(sprite[j].ang+2048)&2047],zoome) / 768;

							if ((sprite[j].cstat&256) > 0)
							{
								if (((sprite[j].ang+256)&512) == 0)
								{
									drawline16(320+xp1,200+yp1-1,320+xp1+xp2,200+yp1+yp2-1,col);
									drawline16(320+xp1,200+yp1+1,320+xp1+xp2,200+yp1+yp2+1,col);
								}
								else
								{
									drawline16(320+xp1-1,200+yp1,320+xp1+xp2-1,200+yp1+yp2,col);
									drawline16(320+xp1+1,200+yp1,320+xp1+xp2+1,200+yp1+yp2,col);
								}
								col += 8;
							}
							drawline16(320+xp1,200+yp1,320+xp1+xp2,200+yp1+yp2,col);
						}
				}

	faketimerhandler();
	xp1 = mulscale11(sintable[(ange+2560)&2047],zoome) / 768; //Draw white arrow
	yp1 = mulscale11(sintable[(ange+2048)&2047],zoome) / 768;
	drawline16(320+xp1,200+yp1,320-xp1,200-yp1,15);
	drawline16(320+xp1,200+yp1,320+yp1,200-xp1,15);
	drawline16(320+xp1,200+yp1,320-yp1,200+xp1,15);
}

printext16(long xpos, long ypos, short col, short backcol, char name[82], char fontsize)
{
	long p, z, zz, charxsiz, daxpos;
	char ch, dat, mask, *fontptr;

	daxpos = xpos;

	koutp(0x3ce,0x5); koutp(0x3cf,(kinp(0x3cf)&(255-3))+2);
	koutp(0x3ce,0x8);

	if (fontsize == 1)
	{
		fontptr = smalltextfont;
		charxsiz = 4;
	}
	else
	{
		fontptr = textfont;
		charxsiz = 8;
	}

	z = 0;
	while (name[z] != 0)
	{
		ch = name[z];
		z++;

		mask = pow2char[8-(daxpos&7)]-1;
		p = ypos*80 + (daxpos>>3)+0xa0000;   //Do not make ylookup!

		if ((daxpos&7) == 0)
		{
			for(zz=0;zz<8;zz++)
			{
				if (backcol >= 0)
				{
					koutp(0x3cf,0xff);
					if (charxsiz == 4) koutp(0x3cf,0x7c);
					readpixel(p), drawpixel(p,(long)backcol);
				}
				koutp(0x3cf,fontptr[(((long)ch)<<3)+zz]);
				if (charxsiz == 4) koutp(0x3cf,0x7c&fontptr[(((long)ch)<<3)+zz]);
				readpixel(p), drawpixel(p,col);
				p += 80;
			}
		}
		else
		{
			for(zz=0;zz<8;zz++)
			{
				if (backcol >= 0)
				{
					if (charxsiz == 8)
					{
						koutp(0x3cf,mask);
						readpixel(p), drawpixel(p,backcol);
						koutp(0x3cf,~mask);
						readpixel(p+1), drawpixel(p+1,backcol);
					}
					else
					{
						koutp(0x3cf,0x7c>>(daxpos&7));
						readpixel(p), drawpixel(p,backcol);
						koutp(0x3cf,0x7c<<(8-(daxpos&7)));
						readpixel(p+1), drawpixel(p+1,backcol);
					}
				}
				dat = fontptr[(((long)ch)<<3)+zz];
				if (charxsiz == 8)
				{
					koutp(0x3cf,mask&(dat>>(daxpos&7)));
					readpixel(p), drawpixel(p,col);
					koutp(0x3cf,(~mask)&(dat<<(8-(daxpos&7))));
					readpixel(p+1), drawpixel(p+1,col);
				}
				else
				{
					koutp(0x3cf,(0x7c&dat)>>(daxpos&7));
					readpixel(p), drawpixel(p,col);
					koutp(0x3cf,(0x7c&dat)<<(8-(daxpos&7)));
					readpixel(p+1), drawpixel(p+1,col);
				}
				p += 80;    //Do not make bytesperline!
			}
		}

		daxpos += charxsiz;
	}
	koutp(0x3ce,0x5); koutp(0x3cf,(kinp(0x3cf)&(255-3))+0);
}
*/

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
/*
alignceilslope(short dasect, long x, long y, long z)
{
	long i, dax, day;
	walltype *wal;

	wal = &wall[sector[dasect].wallptr];
	dax = wall[wal->point2].x-wal->x;
	day = wall[wal->point2].y-wal->y;

	i = (y-wal->y)*dax - (x-wal->x)*day; if (i == 0) return;
	sector[dasect].ceilingheinum = scale((z-sector[dasect].ceilingz)<<8,
													 nsqrtasm(dax*dax+day*day),i);

	if (sector[dasect].ceilingheinum == 0) sector[dasect].ceilingstat &= ~2;
												 else sector[dasect].ceilingstat |= 2;
}
*/
void alignflorslope(short dasect, long x, long y, long z);
/*
owallmost(short *mostbuf, long w, long z)
{
	long bad, x, intx, inty, xcross, y, yy, yinc;
	long s1, s2, s3, s4, ix1, ix2, iy1, iy2, t, x1, x2;

	z <<= 7;
	s1 = mulscale20(globaluclip,yb1[w]); s2 = mulscale20(globaluclip,yb2[w]);
	s3 = mulscale20(globaldclip,yb1[w]); s4 = mulscale20(globaldclip,yb2[w]);
	bad = (z<s1)+((z<s2)<<1)+((z>s3)<<2)+((z>s4)<<3);

	ix1 = xb1[w]; iy1 = yb1[w];
	ix2 = xb2[w]; iy2 = yb2[w];

	if ((bad&3) == 3)
	{
		clearbufbyte(&mostbuf[ix1],(ix2-ix1+1)*sizeof(mostbuf[0]),0L);
		return(bad);
	}

	if ((bad&12) == 12)
	{
		clearbufbyte(&mostbuf[ix1],(ix2-ix1+1)*sizeof(mostbuf[0]),ydimen+(ydimen<<16));
		return(bad);
	}

	if (bad&3)
	{
		t = divscale30(z-s1,s2-s1);
		inty = yb1[w] + mulscale30(yb2[w]-yb1[w],t);
		xcross = xb1[w] + scale(mulscale30(yb2[w],t),xb2[w]-xb1[w],inty);

		if ((bad&3) == 2)
		{
			if (xb1[w] <= xcross) { iy2 = inty; ix2 = xcross; }
			clearbufbyte(&mostbuf[xcross+1],(xb2[w]-xcross)*sizeof(mostbuf[0]),0L);
		}
		else
		{
			if (xcross <= xb2[w]) { iy1 = inty; ix1 = xcross; }
			clearbufbyte(&mostbuf[xb1[w]],(xcross-xb1[w]+1)*sizeof(mostbuf[0]),0L);
		}
	}

	if (bad&12)
	{
		t = divscale30(z-s3,s4-s3);
		inty = yb1[w] + mulscale30(yb2[w]-yb1[w],t);
		xcross = xb1[w] + scale(mulscale30(yb2[w],t),xb2[w]-xb1[w],inty);

		if ((bad&12) == 8)
		{
			if (xb1[w] <= xcross) { iy2 = inty; ix2 = xcross; }
			clearbufbyte(&mostbuf[xcross+1],(xb2[w]-xcross)*sizeof(mostbuf[0]),ydimen+(ydimen<<16));
		}
		else
		{
			if (xcross <= xb2[w]) { iy1 = inty; ix1 = xcross; }
			clearbufbyte(&mostbuf[xb1[w]],(xcross-xb1[w]+1)*sizeof(mostbuf[0]),ydimen+(ydimen<<16));
		}
	}

	y = (scale(z,xdimenscale,iy1)<<4);
	yinc = ((scale(z,xdimenscale,iy2)<<4)-y) / (ix2-ix1+1);
	qinterpolatedown16short(&mostbuf[ix1],ix2-ix1+1,y+(globalhoriz<<16),yinc);

	if (mostbuf[ix1] < 0) mostbuf[ix1] = 0;
	if (mostbuf[ix1] > ydimen) mostbuf[ix1] = ydimen;
	if (mostbuf[ix2] < 0) mostbuf[ix2] = 0;
	if (mostbuf[ix2] > ydimen) mostbuf[ix2] = ydimen;

	return(bad);
}

wallmost(short *mostbuf, long w, long sectnum, char dastat)
{
	long bad, i, j, t, x, y, z, intx, inty, intz, xcross, yinc, fw;
	long x1, y1, z1, x2, y2, z2, xv, yv, dx, dy, dasqr, oz1, oz2;
	long s1, s2, s3, s4, ix1, ix2, iy1, iy2;
	char datempbuf[256];

	if (dastat == 0)
	{
		z = sector[sectnum].ceilingz-globalposz;
		if ((sector[sectnum].ceilingstat&2) == 0) return(owallmost(mostbuf,w,z));
	}
	else
	{
		z = sector[sectnum].floorz-globalposz;
		if ((sector[sectnum].floorstat&2) == 0) return(owallmost(mostbuf,w,z));
	}

	i = thewall[w];
	if (i == sector[sectnum].wallptr) return(owallmost(mostbuf,w,z));

	x1 = wall[i].x; x2 = wall[wall[i].point2].x-x1;
	y1 = wall[i].y; y2 = wall[wall[i].point2].y-y1;

	fw = sector[sectnum].wallptr; i = wall[fw].point2;
	dx = wall[i].x-wall[fw].x; dy = wall[i].y-wall[fw].y;
	dasqr = krecipasm(nsqrtasm(dx*dx+dy*dy));

	if (xb1[w] == 0)
		{ xv = cosglobalang+sinviewingrangeglobalang; yv = singlobalang-cosviewingrangeglobalang; }
	else
		{ xv = x1-globalposx; yv = y1-globalposy; }
	i = xv*(y1-globalposy)-yv*(x1-globalposx); j = yv*x2-xv*y2;
	if (klabs(j) > klabs(i>>3)) i = divscale28(i,j);
	if (dastat == 0)
	{
		t = mulscale15(sector[sectnum].ceilingheinum,dasqr);
		z1 = sector[sectnum].ceilingz;
	}
	else
	{
		t = mulscale15(sector[sectnum].floorheinum,dasqr);
		z1 = sector[sectnum].floorz;
	}
	z1 = dmulscale24(dx*t,mulscale20(y2,i)+((y1-wall[fw].y)<<8),
						 -dy*t,mulscale20(x2,i)+((x1-wall[fw].x)<<8))+((z1-globalposz)<<7);


	if (xb2[w] == xdimen-1)
		{ xv = cosglobalang-sinviewingrangeglobalang; yv = singlobalang+cosviewingrangeglobalang; }
	else
		{ xv = (x2+x1)-globalposx; yv = (y2+y1)-globalposy; }
	i = xv*(y1-globalposy)-yv*(x1-globalposx); j = yv*x2-xv*y2;
	if (klabs(j) > klabs(i>>3)) i = divscale28(i,j);
	if (dastat == 0)
	{
		t = mulscale15(sector[sectnum].ceilingheinum,dasqr);
		z2 = sector[sectnum].ceilingz;
	}
	else
	{
		t = mulscale15(sector[sectnum].floorheinum,dasqr);
		z2 = sector[sectnum].floorz;
	}
	z2 = dmulscale24(dx*t,mulscale20(y2,i)+((y1-wall[fw].y)<<8),
						 -dy*t,mulscale20(x2,i)+((x1-wall[fw].x)<<8))+((z2-globalposz)<<7);


	s1 = mulscale20(globaluclip,yb1[w]); s2 = mulscale20(globaluclip,yb2[w]);
	s3 = mulscale20(globaldclip,yb1[w]); s4 = mulscale20(globaldclip,yb2[w]);
	bad = (z1<s1)+((z2<s2)<<1)+((z1>s3)<<2)+((z2>s4)<<3);

	ix1 = xb1[w]; ix2 = xb2[w];
	iy1 = yb1[w]; iy2 = yb2[w];
	oz1 = z1; oz2 = z2;

	if ((bad&3) == 3)
	{
		clearbufbyte(&mostbuf[ix1],(ix2-ix1+1)*sizeof(mostbuf[0]),0L);
		return(bad);
	}

	if ((bad&12) == 12)
	{
		clearbufbyte(&mostbuf[ix1],(ix2-ix1+1)*sizeof(mostbuf[0]),ydimen+(ydimen<<16));
		return(bad);
	}

	if (bad&3)
	{
			//inty = intz / (globaluclip>>16)
		t = divscale30(oz1-s1,s2-s1+oz1-oz2);
		inty = yb1[w] + mulscale30(yb2[w]-yb1[w],t);
		intz = oz1 + mulscale30(oz2-oz1,t);
		xcross = xb1[w] + scale(mulscale30(yb2[w],t),xb2[w]-xb1[w],inty);

		//t = divscale30((x1<<4)-xcross*yb1[w],xcross*(yb2[w]-yb1[w])-((x2-x1)<<4));
		//inty = yb1[w] + mulscale30(yb2[w]-yb1[w],t);
		//intz = z1 + mulscale30(z2-z1,t);

		if ((bad&3) == 2)
		{
			if (xb1[w] <= xcross) { z2 = intz; iy2 = inty; ix2 = xcross; }
			clearbufbyte(&mostbuf[xcross+1],(xb2[w]-xcross)*sizeof(mostbuf[0]),0L);
		}
		else
		{
			if (xcross <= xb2[w]) { z1 = intz; iy1 = inty; ix1 = xcross; }
			clearbufbyte(&mostbuf[xb1[w]],(xcross-xb1[w]+1)*sizeof(mostbuf[0]),0L);
		}
	}

	if (bad&12)
	{
			//inty = intz / (globaldclip>>16)
		t = divscale30(oz1-s3,s4-s3+oz1-oz2);
		inty = yb1[w] + mulscale30(yb2[w]-yb1[w],t);
		intz = oz1 + mulscale30(oz2-oz1,t);
		xcross = xb1[w] + scale(mulscale30(yb2[w],t),xb2[w]-xb1[w],inty);

		//t = divscale30((x1<<4)-xcross*yb1[w],xcross*(yb2[w]-yb1[w])-((x2-x1)<<4));
		//inty = yb1[w] + mulscale30(yb2[w]-yb1[w],t);
		//intz = z1 + mulscale30(z2-z1,t);

		if ((bad&12) == 8)
		{
			if (xb1[w] <= xcross) { z2 = intz; iy2 = inty; ix2 = xcross; }
			clearbufbyte(&mostbuf[xcross+1],(xb2[w]-xcross)*sizeof(mostbuf[0]),ydimen+(ydimen<<16));
		}
		else
		{
			if (xcross <= xb2[w]) { z1 = intz; iy1 = inty; ix1 = xcross; }
			clearbufbyte(&mostbuf[xb1[w]],(xcross-xb1[w]+1)*sizeof(mostbuf[0]),ydimen+(ydimen<<16));
		}
	}

	y = (scale(z1,xdimenscale,iy1)<<4);
	yinc = ((scale(z2,xdimenscale,iy2)<<4)-y) / (ix2-ix1+1);
	qinterpolatedown16short((long)&mostbuf[ix1],ix2-ix1+1,y+(globalhoriz<<16),yinc);

	if (mostbuf[ix1] < 0) mostbuf[ix1] = 0;
	if (mostbuf[ix1] > ydimen) mostbuf[ix1] = ydimen;
	if (mostbuf[ix2] < 0) mostbuf[ix2] = 0;
	if (mostbuf[ix2] > ydimen) mostbuf[ix2] = ydimen;

	return(bad);
}

#define BITSOFPRECISION 3  //Don't forget to change this in A.ASM also!
grouscan (long dax1, long dax2, long sectnum, char dastat)
{
	long i, j, k, l, m, n, x, y, dx, dy, wx, wy, x1, y1, x2, y2, daz;
	long daslope, dasqr;
	long dashade, shoffs, shinc, m1, m2, *mptr1, *mptr2, *nptr1, *nptr2;
	walltype *wal;
	sectortype *sec;

	sec = &sector[sectnum];

	if (dastat == 0)
	{
		if (globalposz <= getceilzofslope(sectnum,globalposx,globalposy))
			return;  //Back-face culling
		globalorientation = sec->ceilingstat;
		globalpicnum = sec->ceilingpicnum;
		globalshade = sec->ceilingshade;
		globalpal = sec->ceilingpal;
		daslope = sec->ceilingheinum;
		daz = sec->ceilingz;
	}
	else
	{
		if (globalposz >= getflorzofslope(sectnum,globalposx,globalposy))
			return;  //Back-face culling
		globalorientation = sec->floorstat;
		globalpicnum = sec->floorpicnum;
		globalshade = sec->floorshade;
		globalpal = sec->floorpal;
		daslope = sec->floorheinum;
		daz = sec->floorz;
	}

	if ((picanm[globalpicnum]&192) != 0) globalpicnum += animateoffs(globalpicnum,sectnum);
	setgotpic(globalpicnum);
	if ((tilesizx[globalpicnum] <= 0) || (tilesizy[globalpicnum] <= 0)) return;
	if (waloff[globalpicnum] == 0) loadtile(globalpicnum);

	wal = &wall[sec->wallptr];
	wx = wall[wal->point2].x - wal->x;
	wy = wall[wal->point2].y - wal->y;
	dasqr = krecipasm(nsqrtasm(wx*wx+wy*wy));
	i = mulscale21(daslope,dasqr);
	wx *= i; wy *= i;

	globalx = -mulscale19(singlobalang,xdimenrecip);
	globaly = mulscale19(cosglobalang,xdimenrecip);
	globalx1 = (globalposx<<8);
	globaly1 = -(globalposy<<8);
	i = (dax1-halfxdimen)*xdimenrecip;
	globalx2 = mulscale16(cosglobalang<<4,viewingrangerecip) - mulscale27(singlobalang,i);
	globaly2 = mulscale16(singlobalang<<4,viewingrangerecip) + mulscale27(cosglobalang,i);
	globalzd = (xdimscale<<9);
	globalzx = -dmulscale17(wx,globaly2,-wy,globalx2) + mulscale10(1-globalhoriz,globalzd);
	globalz = -dmulscale25(wx,globaly,-wy,globalx);

	if (globalorientation&64)  //Relative alignment
	{
		dx = mulscale14(wall[wal->point2].x-wal->x,dasqr);
		dy = mulscale14(wall[wal->point2].y-wal->y,dasqr);

		i = nsqrtasm(daslope*daslope+16777216);

		x = globalx; y = globaly;
		globalx = dmulscale16(x,dx,y,dy);
		globaly = mulscale12(dmulscale16(-y,dx,x,dy),i);

		x = ((wal->x-globalposx)<<8); y = ((wal->y-globalposy)<<8);
		globalx1 = dmulscale16(-x,dx,-y,dy);
		globaly1 = mulscale12(dmulscale16(-y,dx,x,dy),i);

		x = globalx2; y = globaly2;
		globalx2 = dmulscale16(x,dx,y,dy);
		globaly2 = mulscale12(dmulscale16(-y,dx,x,dy),i);
	}
	if (globalorientation&0x4)
	{
		i = globalx; globalx = -globaly; globaly = -i;
		i = globalx1; globalx1 = globaly1; globaly1 = i;
		i = globalx2; globalx2 = -globaly2; globaly2 = -i;
	}
	if (globalorientation&0x10) { globalx1 = -globalx1, globalx2 = -globalx2, globalx = -globalx; }
	if (globalorientation&0x20) { globaly1 = -globaly1, globaly2 = -globaly2, globaly = -globaly; }

	daz = dmulscale9(wx,globalposy-wal->y,-wy,globalposx-wal->x) + ((daz-globalposz)<<8);
	globalx2 = mulscale20(globalx2,daz); globalx = mulscale28(globalx,daz);
	globaly2 = mulscale20(globaly2,-daz); globaly = mulscale28(globaly,-daz);

	i = 8-(picsiz[globalpicnum]&15); j = 8-(picsiz[globalpicnum]>>4);
	if (globalorientation&8) { i++; j++; }
	globalx1 <<= (i+12); globalx2 <<= i; globalx <<= i;
	globaly1 <<= (j+12); globaly2 <<= j; globaly <<= j;

	if (dastat == 0)
	{
		globalx1 += (((long)sec->ceilingxpanning)<<24);
		globaly1 += (((long)sec->ceilingypanning)<<24);
	}
	else
	{
		globalx1 += (((long)sec->floorxpanning)<<24);
		globaly1 += (((long)sec->floorypanning)<<24);
	}

	asm1 = -(globalzd>>(16-BITSOFPRECISION));

	globvis = globalvisibility;
	if (sec->visibility != 0) globvis = mulscale4(globvis,(long)((unsigned char)(sec->visibility+16)));
	globvis = mulscale13(globvis,daz);
	globvis = mulscale16(globvis,xdimscale);
	j = FP_OFF(palookup[globalpal]);

	setupslopevlin(((long)(picsiz[globalpicnum]&15))+(((long)(picsiz[globalpicnum]>>4))<<8),waloff[globalpicnum],-ylookup[1]);

	l = (globalzd>>16);

	shinc = mulscale16(globalz,xdimenscale);
	if (shinc > 0) shoffs = (4<<15); else shoffs = ((2044-ydimen)<<15);
	if (dastat == 0) y1 = umost[dax1]; else y1 = max(umost[dax1],dplc[dax1]);
	m1 = mulscale16(y1,globalzd) + (globalzx>>6);
		//Avoid visibility overflow by crossing horizon
	if (globalzd > 0) m1 += (globalzd>>16); else m1 -= (globalzd>>16);
	m2 = m1+l;
	mptr1 = (long *)&slopalookup[y1+(shoffs>>15)]; mptr2 = mptr1+1;

	for(x=dax1;x<=dax2;x++)
	{
		if (dastat == 0) { y1 = umost[x]; y2 = min(dmost[x],uplc[x])-1; }
						else { y1 = max(umost[x],dplc[x]); y2 = dmost[x]-1; }
		if (y1 <= y2)
		{
			nptr1 = (long *)&slopalookup[y1+(shoffs>>15)];
			nptr2 = (long *)&slopalookup[y2+(shoffs>>15)];
			while (nptr1 <= mptr1)
			{
				*mptr1-- = j + (getpalookup((long)mulscale24(krecipasm(m1),globvis),globalshade)<<8);
				m1 -= l;
			}
			while (nptr2 >= mptr2)
			{
				*mptr2++ = j + (getpalookup((long)mulscale24(krecipasm(m2),globvis),globalshade)<<8);
				m2 += l;
			}

			globalx3 = (globalx2>>10);
			globaly3 = (globaly2>>10);
			asm3 = mulscale16(y2,globalzd) + (globalzx>>6);
			slopevlin(ylookup[y2]+x+frameoffset,krecipasm(asm3>>3),(long)nptr2,y2-y1+1,globalx1,globaly1);

			if ((x&15) == 0) faketimerhandler();
		}
		globalx2 += globalx;
		globaly2 += globaly;
		globalzx += globalz;
		shoffs += shinc;
	}
}
*/
long getpalookup(long davis, long dashade);
/*
parascan (long dax1, long dax2, long sectnum, char dastat, long bunch)
{
	sectortype *sec;
	long i, j, k, l, m, n, x, y, z, wallnum, nextsectnum, globalhorizbak;
	short *topptr, *botptr;

	sectnum = thesector[bunchfirst[bunch]]; sec = &sector[sectnum];

	globalhorizbak = globalhoriz;
	if (parallaxyscale != 65536)
		globalhoriz = mulscale16(globalhoriz-(ydimen>>1),parallaxyscale) + (ydimen>>1);
	globvis = globalpisibility;
	//globalorientation = 0L;
	if (sec->visibility != 0) globvis = mulscale4(globvis,(long)((unsigned char)(sec->visibility+16)));

	if (dastat == 0)
	{
		globalpal = sec->ceilingpal;
		globalpicnum = sec->ceilingpicnum;
		globalshade = (long)sec->ceilingshade;
		globalxpanning = (long)sec->ceilingxpanning;
		globalypanning = (long)sec->ceilingypanning;
		topptr = umost;
		botptr = uplc;
	}
	else
	{
		globalpal = sec->floorpal;
		globalpicnum = sec->floorpicnum;
		globalshade = (long)sec->floorshade;
		globalxpanning = (long)sec->floorxpanning;
		globalypanning = (long)sec->floorypanning;
		topptr = dplc;
		botptr = dmost;
	}

	if ((unsigned)globalpicnum >= (unsigned)MAXTILES) globalpicnum = 0;
	if (picanm[globalpicnum]&192) globalpicnum += animateoffs(globalpicnum,(short)sectnum);
	globalshiftval = (picsiz[globalpicnum]>>4);
	if (pow2long[globalshiftval] != tilesizy[globalpicnum]) globalshiftval++;
	globalshiftval = 32-globalshiftval;
	globalzd = (((tilesizy[globalpicnum]>>1)+parallaxyoffs)<<globalshiftval)+(globalypanning<<24);
	globalyscale = (8<<(globalshiftval-19));
	//if (globalorientation&256) globalyscale = -globalyscale, globalzd = -globalzd;

	k = 11 - (picsiz[globalpicnum]&15) - pskybits;
	x = -1;

	for(z=bunchfirst[bunch];z>=0;z=p2[z])
	{
		wallnum = thewall[z]; nextsectnum = wall[wallnum].nextsector;

		if (dastat == 0) j = sector[nextsectnum].ceilingstat;
						else j = sector[nextsectnum].floorstat;

		if ((nextsectnum < 0) || (wall[wallnum].cstat&32) || ((j&1) == 0))
		{
			if (x == -1) x = xb1[z];

			if (parallaxtype == 0)
			{
				n = mulscale16(xdimenrecip,viewingrange);
				for(j=xb1[z];j<=xb2[z];j++)
					lplc[j] = (((mulscale23(j-halfxdimen,n)+globalang)&2047)>>k);
			}
			else
			{
				for(j=xb1[z];j<=xb2[z];j++)
					lplc[j] = ((((long)radarang2[j]+globalang)&2047)>>k);
			}
			if (parallaxtype == 2)
			{
				n = mulscale16(xdimscale,viewingrange);
				for(j=xb1[z];j<=xb2[z];j++)
					swplc[j] = mulscale14(sintable[((long)radarang2[j]+512)&2047],n);
			}
			else
				clearbuf((long)(&swplc[xb1[z]]),xb2[z]-xb1[z]+1,mulscale16(xdimscale,viewingrange));
		}
		else if (x >= 0)
		{
			l = globalpicnum; m = (picsiz[globalpicnum]&15);
			globalpicnum = l+pskyoff[lplc[x]>>m];

			if (((lplc[x]^lplc[xb1[z]-1])>>m) == 0)
				wallscan(x,xb1[z]-1,topptr,botptr,swplc,lplc);
			else
			{
				j = x;
				while (x < xb1[z])
				{
					n = l+pskyoff[lplc[x]>>m];
					if (n != globalpicnum)
					{
						wallscan(j,x-1,topptr,botptr,swplc,lplc);
						j = x;
						globalpicnum = n;
					}
					x++;
				}
				if (j < x)
					wallscan(j,x-1,topptr,botptr,swplc,lplc);
			}

			globalpicnum = l;
			x = -1;
		}
	}

	if (x >= 0)
	{
		l = globalpicnum; m = (picsiz[globalpicnum]&15);
		globalpicnum = l+pskyoff[lplc[x]>>m];

		if (((lplc[x]^lplc[xb2[bunchlast[bunch]]])>>m) == 0)
			wallscan(x,xb2[bunchlast[bunch]],topptr,botptr,swplc,lplc);
		else
		{
			j = x;
			while (x <= xb2[bunchlast[bunch]])
			{
				n = l+pskyoff[lplc[x]>>m];
				if (n != globalpicnum)
				{
					wallscan(j,x-1,topptr,botptr,swplc,lplc);
					j = x;
					globalpicnum = n;
				}
				x++;
			}
			if (j <= x)
				wallscan(j,x,topptr,botptr,swplc,lplc);
		}
		globalpicnum = l;
	}
	globalhoriz = globalhorizbak;
}

interrupt stereohandler1()
{
		//VR flag
	if (kinp(0x3c2)&128)
	{
		laststereoint = 0;
		koutpw(0x3d4,((long)(overtbits)<<8)+0x11);
		koutp(0x3d5,overtbits+16);
	}
	if (laststereoint == 1)
	{
		visualpage ^= 1;
		setvisualpage(visualpage|0x80000000);  //0x80000000 to ignore qlimitrate
	}
	laststereoint++;
	koutp(0x70,0xc); kinp(0x71);
	koutp(0xa0,0x20);
	koutp(0x20,0x20);
}

interrupt stereohandler2()
{
		//VR flag
	if (kinp(0x3c2)&128)
	{
		laststereoint = 0;
		koutp(0x378,0xfb+((visualpage&1^1)<<2));
		koutpw(0x3d4,((long)overtbits<<8)+0x11);
		koutp(0x3d5,overtbits+16);
	}
	if (laststereoint == 1)
	{
		visualpage ^= 1;
		setvisualpage(visualpage|0x80000000);  //0x80000000 to ignore qlimitrate
	}
	laststereoint++;
	koutp(0x70,0xc); kinp(0x71);
	koutp(0xa0,0x20);
	koutp(0x20,0x20);
}

stereonextpage()
{
	koutp(0x70,0xc); kinp(0x71);
	//koutpw(0x70,0x420b);
	if ((activepage&1) == 0)
	{
		if (stereomode == 1)
		{
			clearbuf(ylookup[ydim-1]+frameplace,xdim>>4,whiteband);
			clearbuf(ylookup[ydim-1]+frameplace+(xdim>>2),(xdim>>2)-(xdim>>4),blackband);
		}
		activepage++;
		setactivepage(activepage);
		return;
	}
	if (stereomode == 1)
	{
		clearbuf(ylookup[ydim-1]+frameplace,(xdim>>2)-(xdim>>4),whiteband);
		clearbuf(ylookup[ydim-1]+frameplace+xdim-(xdim>>2),xdim>>4,blackband);
	}
	if (visualpage < (numpages&~1)-2) visualpage += 2; else visualpage &= 1;
	if (activepage < (numpages&~1)-1) activepage++; else activepage = 0;
	setactivepage(activepage);
}

setstereo(long dastereomode)
{
	long i, dist, blackdist, whitedist, t1, t2, numlines;
	char c1, c2;

	if ((vidoption != 1) || (numpages < 2)) return;

	if (stereomode)  //---------------Uninitialize old stereo mode
	{
		if ((xdim == 320) && (ydim == 200))
		{
				//back to 70 hz
			koutp(0x3c2,o3c2);
		}

			//Uninit VR flag
		koutpw(0x3d4,(((long)overtbits+32)<<8)+0x11);

			//Uninit RTC
		_disable();
		koutp(0xa1,(kinp(0xa1)&~1)|(oa1&1));
		koutp(0x70,0xa); koutp(0x71,ortca);
		koutp(0x70,0xb); koutp(0x71,ortcb);
		uninstallbistereohandlers();
		_enable();

		stereomode = 0;
		ostereopixelwidth = -1;
		setview(windowx1,windowy1,windowx2,windowy2);
		if (stereomode == 1)
		{
			for(i=0;i<numpages;i++)
			{
				setactivepage(i);
				clearbuf(ylookup[ydim-1]+frameplace,xdim>>2,blackband);
			}
			setactivepage(activepage);
		}
	}

	//------------------------------------- Initialize new stereo mode
	stereomode = dastereomode; if (!stereomode) return;

	activepage = (visualpage & ~1)+2;
	if (activepage >= numpages-1) activepage = 0;

	if (stereomode == 1)
	{
		blackdist = 0x7fffffff; whitedist = 0x80000000;
		koutp(0x3c7,0);
		for(i=0;i<256;i++)
		{
			dist = (kinp(0x3c9)&255)+(kinp(0x3c9)&255)+(kinp(0x3c9)&255);
			if (dist < blackdist) { blackdist = dist; blackband = i; }
			if (dist > whitedist) { whitedist = dist; whiteband = i; }
		}
		blackband += (blackband<<8); blackband += (blackband<<16);
		whiteband += (whiteband<<8); whiteband += (whiteband<<16);
	}

	if ((xdim == 320) && (ydim == 200))
	{
			//80 hz
		o3c2 = kinp(0x3cc);
		koutp(0x3c2,(o3c2&0xf3)+4);
	}

		//Init RTC
	_disable();
	if (stereomode == 1) installbistereohandlers(stereohandler1);
	if (stereomode == 2) installbistereohandlers(stereohandler2);
	koutp(0x70,0xa); ortca = kinp(0x71);
	if (stereomode == 1) koutp(0x71,0x28); //+8 = 256hz
	if (stereomode == 2) koutp(0x71,0x26); //+6 = 1024hz
	koutp(0x70,0xb); ortcb = kinp(0x71); koutp(0x71,0x42);
	koutp(0x70,0xc); kinp(0x71);
	oa1 = kinp(0xa1); koutp(0xa1,oa1&~1);
	_enable();

		//Init VR flag
	koutp(0x3d4,0x11);
	overtbits = kinp(0x3d5) & ~(16+32);
	koutp(0x3d5,overtbits);
	koutp(0x3d5,overtbits+16);
}

#define RTCBUFSIZ 16
static unsigned short rtcopmsel, rtcormseg, rtcormoff;
static unsigned long rtcopmoff;
//Use bicomc.asm as a template if this asm code needs re-writing
static char rtcrmbuffer[RTCBUFSIZ] =
{
	0x50,          //push ax
	0xb0,0x0c,     //mov al, 0ch
	0xe6,0x70,     //out 70h, al
	0xe4,0x71,     //in al, 71h
	0xb0,0x20,     //mov al, 20h
	0xe6,0xa0,     //out 0a0h, al
	0xe6,0x20,     //out 20h, al
	0x58,          //pop ax
	0xcf           //iret
};

void *engconvalloc32 (unsigned long size)
{
	 union REGS r;

	 r.x.eax = 0x0100;           //DPMI allocate DOS memory
	 r.x.ebx = ((size+15)>>4);   //Number of paragraphs requested
	 int386(0x31,&r,&r);

	 if (r.x.cflag != 0)         //Failed
		 return ((unsigned long)0);
	 return ((void *)((r.x.eax&0xffff)<<4));   //Returns full 32-bit offset
}

installbistereohandlers(void far *stereohan)
{
	char *ptr;
	union REGS r;
	struct SREGS sr;
	void *lowp;
	int c;

		//Get old protected mode handler
	r.x.eax = 0x3500+0x70;   // DOS get vector (INT 0Ch)
	sr.ds = sr.es = 0;
	int386x(0x21,&r,&r,&sr);
	rtcopmsel = (unsigned short)sr.es;
	rtcopmoff = r.x.ebx;

		//Get old real mode handler
	r.x.eax = 0x0200;   // DPMI get real mode vector
	r.h.bl = 0x70;
	int386(0x31,&r,&r);
	rtcormseg = (unsigned short)r.x.ecx;
	rtcormoff = (unsigned short)r.x.edx;

		//Allocate memory in low memory to store real mode handler
	if ((lowp = engconvalloc32(RTCBUFSIZ)) == 0)
	{
		printf("Couldn't allocate conventional memory.\n");
		exit(1);
	}

	memcpy(lowp,(void *)rtcrmbuffer,RTCBUFSIZ);

		//Set new protected mode handler
	r.x.eax = 0x2500+0x70;   // DOS set vector (INT 0Ch)
	r.x.edx = FP_OFF(stereohan);
	sr.ds = FP_SEG(stereohan);      //DS:EDX == &handler
	sr.es = 0;
	int386x(0x21,&r,&r,&sr);

		//Set new real mode handler (must be after setting protected mode)
	r.x.eax = 0x0201;
	r.h.bl = 0x70;              //CX:DX == real mode &handler
	r.x.ecx = ((((long)lowp)>>4)&0xffff);   //D32realseg
	r.x.edx = (((long)lowp)&0xf);           //D32realoff
	int386(0x31,&r,&r);
}

uninstallbistereohandlers()
{
	union REGS r;
	struct SREGS sr;

		//restore old protected mode handler
	r.x.eax = 0x2500+0x70;   // DOS set vector (INT 0Ch)
	r.x.edx = rtcopmoff;
	sr.ds = rtcopmsel;    // DS:EDX == &handler
	sr.es = 0;
	int386x(0x21,&r,&r,&sr);

		//restore old real mode handler
	r.x.eax = 0x0201;   // DPMI set real mode vector
	r.h.bl = 0x70;
	r.x.ecx = (unsigned long)rtcormseg;     //CX:DX == real mode &handler
	r.x.edx = (unsigned long)rtcormoff;
	int386(0x31,&r,&r);
}
*/
long loopnumofsector(short sectnum, short wallnum);

setfirstwall(short sectnum, short newfirstwall)
{
	long i, j, k, numwallsofloop;
	long startwall, endwall, danumwalls, dagoalloop;

	startwall = sector[sectnum].wallptr;
	danumwalls = sector[sectnum].wallnum;
	endwall = startwall+danumwalls;
	if ((newfirstwall < startwall) || (newfirstwall >= startwall+danumwalls)) return;
	for(i=0;i<danumwalls;i++)
		memcpy(&wall[i+numwalls],&wall[i+startwall],sizeof(walltype));

	numwallsofloop = 0;
	i = newfirstwall;
	do
	{
		numwallsofloop++;
		i = wall[i].point2;
	} while (i != newfirstwall);

		//Put correct loop at beginning
	dagoalloop = loopnumofsector(sectnum,newfirstwall);
	if (dagoalloop > 0)
	{
		j = 0;
		while (loopnumofsector(sectnum,j+startwall) != dagoalloop) j++;
		for(i=0;i<danumwalls;i++)
		{
			k = i+j; if (k >= danumwalls) k -= danumwalls;
			memcpy(&wall[startwall+i],&wall[numwalls+k],sizeof(walltype));

			wall[startwall+i].point2 += danumwalls-startwall-j;
			if (wall[startwall+i].point2 >= danumwalls)
				wall[startwall+i].point2 -= danumwalls;
			wall[startwall+i].point2 += startwall;
		}
		newfirstwall += danumwalls-j;
		if (newfirstwall >= startwall+danumwalls) newfirstwall -= danumwalls;
	}

	for(i=0;i<numwallsofloop;i++)
		memcpy(&wall[i+numwalls],&wall[i+startwall],sizeof(walltype));
	for(i=0;i<numwallsofloop;i++)
	{
		k = i+newfirstwall-startwall;
		if (k >= numwallsofloop) k -= numwallsofloop;
		memcpy(&wall[startwall+i],&wall[numwalls+k],sizeof(walltype));

		wall[startwall+i].point2 += numwallsofloop-newfirstwall;
		if (wall[startwall+i].point2 >= numwallsofloop)
			wall[startwall+i].point2 -= numwallsofloop;
		wall[startwall+i].point2 += startwall;
	}

	for(i=startwall;i<endwall;i++)
		if (wall[i].nextwall >= 0) wall[wall[i].nextwall].nextwall = i;
}


#endif