// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

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
void *kmalloc(size_t size) { return(malloc(size)); }
void *kkmalloc(size_t size);
#pragma aux kkmalloc =\
	"call kmalloc",\
	parm [eax]\

	//MUST CALL FREE THIS WAY TO FORCE CALLS TO KFREE!
void kfree(void *buffer) { free(buffer); }
void kkfree(void *buffer);
#pragma aux kkfree =\
	"call kfree",\
	parm [eax]\

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

long artsize = 0, cachesize = 0;

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

#pragma aux nsqrtasm =\
	"test eax, 0xff000000",\
	"mov ebx, eax",\
	"jnz short over24",\
	"shr ebx, 12",\
	"mov cx, word ptr shlookup[ebx*2]",\
	"jmp short under24",\
	"over24: shr ebx, 24",\
	"mov cx, word ptr shlookup[ebx*2+8192]",\
	"under24: shr eax, cl",\
	"mov cl, ch",\
	"mov ax, word ptr sqrtable[eax*2]",\
	"shr eax, cl",\
	parm nomemory [eax]\
	modify exact [eax ebx ecx]\

#pragma aux msqrtasm =\
	"mov eax, 0x40000000",\
	"mov ebx, 0x20000000",\
	"begit: cmp ecx, eax",\
	"jl skip",\
	"sub ecx, eax",\
	"lea eax, [eax+ebx*4]",\
	"skip: sub eax, ebx",\
	"shr eax, 1",\
	"shr ebx, 2",\
	"jnz begit",\
	"cmp ecx, eax",\
	"sbb eax, -1",\
	"shr eax, 1",\
	parm nomemory [ecx]\
	modify exact [eax ebx ecx]\

	//0x007ff000 is (11<<13), 0x3f800000 is (127<<23)
#pragma aux krecipasm =\
	"mov fpuasm, eax",\
	"fild dword ptr fpuasm",\
	"add eax, eax",\
	"fstp dword ptr fpuasm",\
	"sbb ebx, ebx",\
	"mov eax, fpuasm",\
	"mov ecx, eax",\
	"and eax, 0x007ff000",\
	"shr eax, 10",\
	"sub ecx, 0x3f800000",\
	"shr ecx, 23",\
	"mov eax, dword ptr reciptable[eax]",\
	"sar eax, cl",\
	"xor eax, ebx",\
	parm [eax]\
	modify exact [eax ebx ecx]\

#pragma aux setgotpic =\
	"mov ebx, eax",\
	"cmp byte ptr walock[eax], 200",\
	"jae skipit",\
	"mov byte ptr walock[eax], 199",\
	"skipit: shr eax, 3",\
	"and ebx, 7",\
	"mov dl, byte ptr gotpic[eax]",\
	"mov bl, byte ptr pow2char[ebx]",\
	"or dl, bl",\
	"mov byte ptr gotpic[eax], dl",\
	parm [eax]\
	modify exact [eax ebx ecx edx]\

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

scansector (short sectnum);

wallfront (long l1, long l2);

spritewallfront (spritetype *s, long w)
{
	walltype *wal;
	long x1, y1;

	wal = &wall[w]; x1 = wal->x; y1 = wal->y;
	wal = &wall[wal->point2];
	return (dmulscale32(wal->x-x1,s->y-y1,-(s->x-x1),wal->y-y1) >= 0);
}

bunchfront (long b1, long b2)
{
	long x1b1, x2b1, x1b2, x2b2, b1f, b2f, i;

	b1f = bunchfirst[b1]; x1b1 = xb1[b1f]; x2b2 = xb2[bunchlast[b2]]+1;
	if (x1b1 >= x2b2) return(-1);
	b2f = bunchfirst[b2]; x1b2 = xb1[b2f]; x2b1 = xb2[bunchlast[b1]]+1;
	if (x1b2 >= x2b1) return(-1);

	if (x1b1 >= x1b2)
	{
		for(i=b2f;xb2[i]<x1b1;i=p2[i]);
		return(wallfront(b1f,i));
	}
	for(i=b1f;xb2[i]<x1b2;i=p2[i]);
	return(wallfront(i,b2f));
}

drawalls (long bunch);

void prepwall(long z, walltype *wal);

ceilscan (long x1, long x2, long sectnum);

florscan (long x1, long x2, long sectnum);

wallscan(long x1, long x2, short *uwal, short *dwal, long *swal, long *lwal);

maskwallscan(long x1, long x2, short *uwal, short *dwal, long *swal, long *lwal);

transmaskvline (long x);

transmaskvline2 (long x);

transmaskwallscan(long x1, long x2);

loadboard(char *filename, long *daposx, long *daposy, long *daposz,
		  short *daang, short *dacursectnum);

saveboard(char *filename, long *daposx, long *daposy, long *daposz,
		  short *daang, short *dacursectnum);

loadtables();

loadpalette();

static char screenalloctype = 255;
setgamemode(char davidoption, long daxdim, long daydim);


hline (long xr, long yp);

slowhline (long xr, long yp);

long dommxoverlay = 1;
initengine();

uninitengine();

nextpage();

char cachedebug = 0;
loadtile (short tilenume);

allocatepermanenttile(short tilenume, long xsiz, long ysiz);

loadpics(char *filename);

#ifdef SUPERBUILD
qloadkvx(long voxindex, char *filename);
#endif

clipinsidebox(long x, long y, short wallnum, long walldist);

clipinsideboxline(long x, long y, long x1, long y1, long x2, long y2, long walldist);

readpixel16(long p);

screencapture(char *filename, char inverseit);

inside (long x, long y, short sectnum);

getangle(long xvect, long yvect);

ksqrt(long num);

krecip(long num);

initksqrt();

copytilepiece(long tilenume1, long sx1, long sy1, long xsiz, long ysiz,
			  long tilenume2, long sx2, long sy2);

drawmasks();

drawmaskwall(short damaskwallcnt);

drawsprite (long snum);

#ifdef SUPERBUILD
drawvox(long dasprx, long daspry, long dasprz, long dasprang,
		  long daxscale, long dayscale, char daindex,
		  signed char dashade, char dapal, long *daumost, long *dadmost);
#endif

ceilspritescan (long x1, long x2);

ceilspritehline (long x2, long y);

int setsprite(short spritenum, long newx, long newy, long newz);

animateoffs(short tilenum, short fakevar);

initspritelists();

insertsprite(short sectnum, short statnum);

insertspritesect(short sectnum);

insertspritestat(short statnum);

int deletesprite(short spritenum);

deletespritesect(short deleteme);

deletespritestat (short deleteme);

changespritesect(short spritenum, short newsectnum);

changespritestat(short spritenum, short newstatnum);

nextsectorneighborz(short sectnum, long thez, short topbottom, short direction);

cansee(long x1, long y1, long z1, short sect1, long x2, long y2, long z2, short sect2);

hitscan(long xs, long ys, long zs, short sectnum, long vx, long vy, long vz,
		short *hitsect, short *hitwall, short *hitsprite,
		long *hitx, long *hity, long *hitz, unsigned long cliptype);

neartag (long xs, long ys, long zs, short sectnum, short ange, short *neartagsector, short *neartagwall, short *neartagsprite, long *neartaghitdist, long neartagrange, char tagsearch);

lintersect(long x1, long y1, long z1, long x2, long y2, long z2, long x3,
		   long y3, long x4, long y4, long *intx, long *inty, long *intz);

rintersect(long x1, long y1, long z1, long vx, long vy, long vz, long x3,
		   long y3, long x4, long y4, long *intx, long *inty, long *intz);

dragpoint(short pointhighlight, long dax, long day);

lastwall(short point);

#define addclipline(dax1, day1, dax2, day2, daoval)      \
{                                                        \
	clipit[clipnum].x1 = dax1; clipit[clipnum].y1 = day1; \
	clipit[clipnum].x2 = dax2; clipit[clipnum].y2 = day2; \
	clipobjectval[clipnum] = daoval;                      \
	clipnum++;                                            \
}                                                        \

long clipmoveboxtracenum = 3;
clipmove (long *x, long *y, long *z, short *sectnum,
			 long xvect, long yvect,
			 long walldist, long ceildist, long flordist, unsigned long cliptype);

keepaway (long *x, long *y, long w);

raytrace (long x3, long y3, long *x4, long *y4);

pushmove (long *x, long *y, long *z, short *sectnum,
		  long walldist, long ceildist, long flordist, unsigned long cliptype);

updatesector(long x, long y, short *sectnum);

rotatepoint(long xpivot, long ypivot, long x, long y, short daang, long *x2, long *y2);

initmouse();

getmousevalues(short *mousx, short *mousy, short *bstatus);

printscreeninterrupt();

drawline256 (long x1, long y1, long x2, long y2, char col);

drawline16(long x1, long y1, long x2, long y2, char col);

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

printext256(long xpos, long ypos, short col, short backcol, char name[82], char fontsize);

krand();

getzrange(long x, long y, long z, short sectnum,
		  long *ceilz, long *ceilhit, long *florz, long *florhit,
		  long walldist, unsigned long cliptype);

setview(long x1, long y1, long x2, long y2);

setaspect(long daxrange, long daaspect);

dosetaspect();

flushperms();

rotatesprite (long sx, long sy, long z, short a, short picnum, signed char dashade, char dapalnum, char dastat, long cx1, long cy1, long cx2, long cy2);

dorotatesprite (long sx, long sy, long z, short a, short picnum, signed char dashade, char dapalnum, char dastat, long cx1, long cy1, long cx2, long cy2);
/*
//Assume npoints=4 with polygon on &rx1,&ry1
clippoly4(long cx1, long cy1, long cx2, long cy2)
{
	long n, nn, z, zz, x, x1, x2, y, y1, y2, t;

	nn = 0; z = 0;
	do
	{
		zz = ((z+1)&3);
		x1 = rx1[z]; x2 = rx1[zz]-x1;

		if ((cx1 <= x1) && (x1 <= cx2))
			rx2[nn] = x1, ry2[nn] = ry1[z], nn++;

		if (x2 <= 0) x = cx2; else x = cx1;
		t = x-x1;
		if (((t-x2)^t) < 0)
			rx2[nn] = x, ry2[nn] = ry1[z]+scale(t,ry1[zz]-ry1[z],x2), nn++;

		if (x2 <= 0) x = cx1; else x = cx2;
		t = x-x1;
		if (((t-x2)^t) < 0)
			rx2[nn] = x, ry2[nn] = ry1[z]+scale(t,ry1[zz]-ry1[z],x2), nn++;

		z = zz;
	} while (z != 0);
	if (nn < 3) return(0);

	n = 0; z = 0;
	do
	{
		zz = z+1; if (zz == nn) zz = 0;
		y1 = ry2[z]; y2 = ry2[zz]-y1;

		if ((cy1 <= y1) && (y1 <= cy2))
			ry1[n] = y1, rx1[n] = rx2[z], n++;

		if (y2 <= 0) y = cy2; else y = cy1;
		t = y-y1;
		if (((t-y2)^t) < 0)
			ry1[n] = y, rx1[n] = rx2[z]+scale(t,rx2[zz]-rx2[z],y2), n++;

		if (y2 <= 0) y = cy1; else y = cy2;
		t = y-y1;
		if (((t-y2)^t) < 0)
			ry1[n] = y, rx1[n] = rx2[z]+scale(t,rx2[zz]-rx2[z],y2), n++;

		z = zz;
	} while (z != 0);
	return(n);
}

makepalookup(long palnum, char *remapbuf, signed char r, signed char g, signed char b, char dastat)
{
	long i, j, dist, palscale;
	char *ptr, *ptr2;

	if (paletteloaded == 0) return;

	if (palookup[palnum] == NULL)
	{
			//Allocate palookup buffer
		if ((palookup[palnum] = (char *)kkmalloc(numpalookups<<8)) == NULL)
			allocache(&palookup[palnum],numpalookups<<8,&permanentlock);
	}

	if (dastat == 0) return;
	if ((r|g|b|63) != 63) return;

	if ((r|g|b) == 0)
	{
		for(i=0;i<256;i++)
		{
			ptr = (char *)(FP_OFF(palookup[0])+remapbuf[i]);
			ptr2 = (char *)(FP_OFF(palookup[palnum])+i);
			for(j=0;j<numpalookups;j++)
				{ *ptr2 = *ptr; ptr += 256; ptr2 += 256; }
		}
	}
	else
	{
		ptr2 = (char *)FP_OFF(palookup[palnum]);
		for(i=0;i<numpalookups;i++)
		{
			palscale = divscale16(i,numpalookups);
			for(j=0;j<256;j++)
			{
				ptr = (char *)&palette[remapbuf[j]*3];
				*ptr2++ = getclosestcol((long)ptr[0]+mulscale16(r-ptr[0],palscale),
												(long)ptr[1]+mulscale16(g-ptr[1],palscale),
												(long)ptr[2]+mulscale16(b-ptr[2],palscale));
			}
		}
	}

	if ((vidoption == 6) && (qsetmode == 200))
	{
		for(i=0;i<256;i++)
		{
			dist = palette[i*3]*3+palette[i*3+1]*5+palette[i*3+2]*2;
			ptr = (char *)(FP_OFF(palookup[palnum])+i);
			for(j=0;j<32;j++)
				ptr[j<<8] = (char)min(max(mulscale10(dist,32-j),0),15);
		}
	}
}

initfastcolorlookup(long rscale, long gscale, long bscale)
{
	long i, j, x, y, z;
	char *pal1;

	j = 0;
	for(i=64;i>=0;i--)
	{
		//j = (i-64)*(i-64);
		rdist[i] = rdist[128-i] = j*rscale;
		gdist[i] = gdist[128-i] = j*gscale;
		bdist[i] = bdist[128-i] = j*bscale;
		j += 129-(i<<1);
	}

	clearbufbyte(FP_OFF(colhere),sizeof(colhere),0L);
	clearbufbyte(FP_OFF(colhead),sizeof(colhead),0L);

	pal1 = (char *)&palette[768-3];
	for(i=255;i>=0;i--,pal1-=3)
	{
		j = (pal1[0]>>3)*FASTPALGRIDSIZ*FASTPALGRIDSIZ+(pal1[1]>>3)*FASTPALGRIDSIZ+(pal1[2]>>3)+FASTPALGRIDSIZ*FASTPALGRIDSIZ+FASTPALGRIDSIZ+1;
		if (colhere[j>>3]&pow2char[j&7]) colnext[i] = colhead[j]; else colnext[i] = -1;
		colhead[j] = i;
		colhere[j>>3] |= pow2char[j&7];
	}

	i = 0;
	for(x=-FASTPALGRIDSIZ*FASTPALGRIDSIZ;x<=FASTPALGRIDSIZ*FASTPALGRIDSIZ;x+=FASTPALGRIDSIZ*FASTPALGRIDSIZ)
		for(y=-FASTPALGRIDSIZ;y<=FASTPALGRIDSIZ;y+=FASTPALGRIDSIZ)
			for(z=-1;z<=1;z++)
				colscan[i++] = x+y+z;
	i = colscan[13]; colscan[13] = colscan[26]; colscan[26] = i;
}

getclosestcol(long r, long g, long b)
{
	long x, y, z, i, j, k, dist, mindist, retcol;
	char *pal1;

	j = (r>>3)*FASTPALGRIDSIZ*FASTPALGRIDSIZ+(g>>3)*FASTPALGRIDSIZ+(b>>3)+FASTPALGRIDSIZ*FASTPALGRIDSIZ+FASTPALGRIDSIZ+1;
	mindist = min(rdist[coldist[r&7]+64+8],gdist[coldist[g&7]+64+8]);
	mindist = min(mindist,bdist[coldist[b&7]+64+8]);
	mindist++;

	r = 64-r; g = 64-g; b = 64-b;

	retcol = -1;
	for(k=26;k>=0;k--)
	{
		i = colscan[k]+j; if ((colhere[i>>3]&pow2char[i&7]) == 0) continue;
		i = colhead[i];
		do
		{
			pal1 = (char *)&palette[i*3];
			dist = gdist[pal1[1]+g];
			if (dist < mindist)
			{
				dist += rdist[pal1[0]+r];
				if (dist < mindist)
				{
					dist += bdist[pal1[2]+b];
					if (dist < mindist) { mindist = dist; retcol = i; }
				}
			}
			i = colnext[i];
		} while (i >= 0);
	}
	if (retcol >= 0) return(retcol);

	mindist = 0x7fffffff;
	pal1 = (char *)&palette[768-3];
	for(i=255;i>=0;i--,pal1-=3)
	{
		dist = gdist[pal1[1]+g]; if (dist >= mindist) continue;
		dist += rdist[pal1[0]+r]; if (dist >= mindist) continue;
		dist += bdist[pal1[2]+b]; if (dist >= mindist) continue;
		mindist = dist; retcol = i;
	}
	return(retcol);
}

setbrightness(char dabrightness, char *dapal)
{
	char *ptr;
	long i, j, k, dist, daval;

	curbrightness = min(max((long)dabrightness,0),15);

	k = 0;
	if (vidoption == 6)
	{
		for(j=0;j<16;j++)
			for(i=0;i<16;i++)
			{
				tempbuf[k++] = britable[curbrightness][j<<2];
				tempbuf[k++] = 0;
				tempbuf[k++] = britable[curbrightness][i<<2];
				tempbuf[k++] = 0;
			}
	}
	else
	{
		for(i=0;i<256;i++)
		{
			tempbuf[k++] = britable[curbrightness][dapal[i*3+2]];
			tempbuf[k++] = britable[curbrightness][dapal[i*3+1]];
			tempbuf[k++] = britable[curbrightness][dapal[i*3+0]];
			tempbuf[k++] = 0;
		}
	}

	VBE_setPalette(0,256,tempbuf);
}

drawmapview (long dax, long day, long zoome, short ang)
{
	walltype *wal;
	sectortype *sec;
	spritetype *spr;
	long tilenum, xoff, yoff, i, j, k, l, cosang, sinang, xspan, yspan;
	long xrepeat, yrepeat, x, y, x1, y1, x2, y2, x3, y3, x4, y4, bakx1, baky1;
	long s, w, ox, oy, startwall, cx1, cy1, cx2, cy2;
	long bakgxvect, bakgyvect, sortnum, gap, npoints;
	long xvect, yvect, xvect2, yvect2, daslope;

	beforedrawrooms = 0;
	totalarea += (windowx2+1-windowx1)*(windowy2+1-windowy1);

	clearbuf((long)(&gotsector[0]),(long)((numsectors+31)>>5),0L);

	cx1 = (windowx1<<12); cy1 = (windowy1<<12);
	cx2 = ((windowx2+1)<<12)-1; cy2 = ((windowy2+1)<<12)-1;
	zoome <<= 8;
	bakgxvect = divscale28(sintable[(1536-ang)&2047],zoome);
	bakgyvect = divscale28(sintable[(2048-ang)&2047],zoome);
	xvect = mulscale8(sintable[(2048-ang)&2047],zoome);
	yvect = mulscale8(sintable[(1536-ang)&2047],zoome);
	xvect2 = mulscale16(xvect,yxaspect);
	yvect2 = mulscale16(yvect,yxaspect);

	sortnum = 0;
	for(s=0,sec=&sector[s];s<numsectors;s++,sec++)
		if (show2dsector[s>>3]&pow2char[s&7])
		{
			npoints = 0; i = 0;
			startwall = sec->wallptr;
			for(w=sec->wallnum,wal=&wall[startwall];w>0;w--,wal++)
			{
				ox = wal->x - dax; oy = wal->y - day;
				x = dmulscale16(ox,xvect,-oy,yvect) + (xdim<<11);
				y = dmulscale16(oy,xvect2,ox,yvect2) + (ydim<<11);
				i |= getclipmask(x-cx1,cx2-x,y-cy1,cy2-y);
				rx1[npoints] = x;
				ry1[npoints] = y;
				xb1[npoints] = wal->point2 - startwall;
				npoints++;
			}
			if ((i&0xf0) != 0xf0) continue;
			bakx1 = rx1[0]; baky1 = mulscale16(ry1[0]-(ydim<<11),xyaspect)+(ydim<<11);
			if (i&0x0f)
			{
				npoints = clippoly(npoints,i);
				if (npoints < 3) continue;
			}

				//Collect floor sprites to draw
			for(i=headspritesect[s];i>=0;i=nextspritesect[i])
				if ((sprite[i].cstat&48) == 32)
				{
					if ((sprite[i].cstat&(64+8)) == (64+8)) continue;
					tsprite[sortnum++].owner = i;
				}

			gotsector[s>>3] |= pow2char[s&7];

			globalorientation = (long)sec->floorstat;
			if ((globalorientation&1) != 0) continue;

			if (palookup[sec->floorpal] != globalpalwritten)
			{
				globalpalwritten = palookup[sec->floorpal];
				setpalookupaddress(globalpalwritten);
			}
			globalpicnum = sec->floorpicnum;
			if ((unsigned)globalpicnum >= (unsigned)MAXTILES) globalpicnum = 0;
			setgotpic(globalpicnum);
			if ((tilesizx[globalpicnum] <= 0) || (tilesizy[globalpicnum] <= 0)) continue;
			if ((picanm[globalpicnum]&192) != 0) globalpicnum += animateoffs((short)globalpicnum,s);
			if (waloff[globalpicnum] == 0) loadtile(globalpicnum);
			globalbufplc = waloff[globalpicnum];
			globalshade = max(min(sec->floorshade,numpalookups-1),0);
			globvis = globalhisibility;
			if (sec->visibility != 0) globvis = mulscale4(globvis,(long)((unsigned char)(sec->visibility+16)));
			globalpolytype = 0;
			if ((globalorientation&64) == 0)
			{
				globalposx = dax; globalx1 = bakgxvect; globaly1 = bakgyvect;
				globalposy = day; globalx2 = bakgxvect; globaly2 = bakgyvect;
			}
			else
			{
				ox = wall[wall[startwall].point2].x - wall[startwall].x;
				oy = wall[wall[startwall].point2].y - wall[startwall].y;
				i = nsqrtasm(ox*ox+oy*oy); if (i == 0) continue;
				i = 1048576/i;
				globalx1 = mulscale10(dmulscale10(ox,bakgxvect,oy,bakgyvect),i);
				globaly1 = mulscale10(dmulscale10(ox,bakgyvect,-oy,bakgxvect),i);
				ox = (bakx1>>4)-(xdim<<7); oy = (baky1>>4)-(ydim<<7);
				globalposx = dmulscale28(-oy,globalx1,-ox,globaly1);
				globalposy = dmulscale28(-ox,globalx1,oy,globaly1);
				globalx2 = -globalx1;
				globaly2 = -globaly1;

				daslope = sector[s].floorheinum;
				i = nsqrtasm(daslope*daslope+16777216);
				globalposy = mulscale12(globalposy,i);
				globalx2 = mulscale12(globalx2,i);
				globaly2 = mulscale12(globaly2,i);
			}
			globalxshift = (8-(picsiz[globalpicnum]&15));
			globalyshift = (8-(picsiz[globalpicnum]>>4));
			if (globalorientation&8) {globalxshift++; globalyshift++; }

			sethlinesizes(picsiz[globalpicnum]&15,picsiz[globalpicnum]>>4,globalbufplc);

			if ((globalorientation&0x4) > 0)
			{
				i = globalposx; globalposx = -globalposy; globalposy = -i;
				i = globalx2; globalx2 = globaly1; globaly1 = i;
				i = globalx1; globalx1 = -globaly2; globaly2 = -i;
			}
			if ((globalorientation&0x10) > 0) globalx1 = -globalx1, globaly1 = -globaly1, globalposx = -globalposx;
			if ((globalorientation&0x20) > 0) globalx2 = -globalx2, globaly2 = -globaly2, globalposy = -globalposy;
			asm1 = (globaly1<<globalxshift);
			asm2 = (globalx2<<globalyshift);
			globalx1 <<= globalxshift;
			globaly2 <<= globalyshift;
			globalposx = (globalposx<<(20+globalxshift))+(((long)sec->floorxpanning)<<24);
			globalposy = (globalposy<<(20+globalyshift))-(((long)sec->floorypanning)<<24);

			fillpolygon(npoints);
		}

		//Sort sprite list
	gap = 1; while (gap < sortnum) gap = (gap<<1)+1;
	for(gap>>=1;gap>0;gap>>=1)
		for(i=0;i<sortnum-gap;i++)
			for(j=i;j>=0;j-=gap)
			{
				if (sprite[tsprite[j].owner].z <= sprite[tsprite[j+gap].owner].z) break;
				swapshort(&tsprite[j].owner,&tsprite[j+gap].owner);
			}

	for(s=sortnum-1;s>=0;s--)
	{
		spr = &sprite[tsprite[s].owner];
		if ((spr->cstat&48) == 32)
		{
			npoints = 0;

			tilenum = spr->picnum;
			xoff = (long)((signed char)((picanm[tilenum]>>8)&255))+((long)spr->xoffset);
			yoff = (long)((signed char)((picanm[tilenum]>>16)&255))+((long)spr->yoffset);
			if ((spr->cstat&4) > 0) xoff = -xoff;
			if ((spr->cstat&8) > 0) yoff = -yoff;

			k = spr->ang;
			cosang = sintable[(k+512)&2047]; sinang = sintable[k];
			xspan = tilesizx[tilenum]; xrepeat = spr->xrepeat;
			yspan = tilesizy[tilenum]; yrepeat = spr->yrepeat;

			ox = ((xspan>>1)+xoff)*xrepeat; oy = ((yspan>>1)+yoff)*yrepeat;
			x1 = spr->x + mulscale(sinang,ox,16) + mulscale(cosang,oy,16);
			y1 = spr->y + mulscale(sinang,oy,16) - mulscale(cosang,ox,16);
			l = xspan*xrepeat;
			x2 = x1 - mulscale(sinang,l,16);
			y2 = y1 + mulscale(cosang,l,16);
			l = yspan*yrepeat;
			k = -mulscale(cosang,l,16); x3 = x2+k; x4 = x1+k;
			k = -mulscale(sinang,l,16); y3 = y2+k; y4 = y1+k;

			xb1[0] = 1; xb1[1] = 2; xb1[2] = 3; xb1[3] = 0;
			npoints = 4;

			i = 0;

			ox = x1 - dax; oy = y1 - day;
			x = dmulscale16(ox,xvect,-oy,yvect) + (xdim<<11);
			y = dmulscale16(oy,xvect2,ox,yvect2) + (ydim<<11);
			i |= getclipmask(x-cx1,cx2-x,y-cy1,cy2-y);
			rx1[0] = x; ry1[0] = y;

			ox = x2 - dax; oy = y2 - day;
			x = dmulscale16(ox,xvect,-oy,yvect) + (xdim<<11);
			y = dmulscale16(oy,xvect2,ox,yvect2) + (ydim<<11);
			i |= getclipmask(x-cx1,cx2-x,y-cy1,cy2-y);
			rx1[1] = x; ry1[1] = y;

			ox = x3 - dax; oy = y3 - day;
			x = dmulscale16(ox,xvect,-oy,yvect) + (xdim<<11);
			y = dmulscale16(oy,xvect2,ox,yvect2) + (ydim<<11);
			i |= getclipmask(x-cx1,cx2-x,y-cy1,cy2-y);
			rx1[2] = x; ry1[2] = y;

			x = rx1[0]+rx1[2]-rx1[1];
			y = ry1[0]+ry1[2]-ry1[1];
			i |= getclipmask(x-cx1,cx2-x,y-cy1,cy2-y);
			rx1[3] = x; ry1[3] = y;

			if ((i&0xf0) != 0xf0) continue;
			bakx1 = rx1[0]; baky1 = mulscale16(ry1[0]-(ydim<<11),xyaspect)+(ydim<<11);
			if (i&0x0f)
			{
				npoints = clippoly(npoints,i);
				if (npoints < 3) continue;
			}

			globalpicnum = spr->picnum;
			if ((unsigned)globalpicnum >= (unsigned)MAXTILES) globalpicnum = 0;
			setgotpic(globalpicnum);
			if ((tilesizx[globalpicnum] <= 0) || (tilesizy[globalpicnum] <= 0)) continue;
			if ((picanm[globalpicnum]&192) != 0) globalpicnum += animateoffs((short)globalpicnum,s);
			if (waloff[globalpicnum] == 0) loadtile(globalpicnum);
			globalbufplc = waloff[globalpicnum];
			if ((sector[spr->sectnum].ceilingstat&1) > 0)
				globalshade = ((long)sector[spr->sectnum].ceilingshade);
			else
				globalshade = ((long)sector[spr->sectnum].floorshade);
			globalshade = max(min(globalshade+spr->shade+6,numpalookups-1),0);
			asm3 = FP_OFF(palookup[spr->pal]+(globalshade<<8));
			globvis = globalhisibility;
			if (sec->visibility != 0) globvis = mulscale4(globvis,(long)((unsigned char)(sec->visibility+16)));
			globalpolytype = ((spr->cstat&2)>>1)+1;

				//relative alignment stuff
			ox = x2-x1; oy = y2-y1;
			i = ox*ox+oy*oy; if (i == 0) continue; i = (65536*16384)/i;
			globalx1 = mulscale10(dmulscale10(ox,bakgxvect,oy,bakgyvect),i);
			globaly1 = mulscale10(dmulscale10(ox,bakgyvect,-oy,bakgxvect),i);
			ox = y1-y4; oy = x4-x1;
			i = ox*ox+oy*oy; if (i == 0) continue; i = (65536*16384)/i;
			globalx2 = mulscale10(dmulscale10(ox,bakgxvect,oy,bakgyvect),i);
			globaly2 = mulscale10(dmulscale10(ox,bakgyvect,-oy,bakgxvect),i);

			ox = picsiz[globalpicnum]; oy = ((ox>>4)&15); ox &= 15;
			if (pow2long[ox] != xspan)
			{
				ox++;
				globalx1 = mulscale(globalx1,xspan,ox);
				globaly1 = mulscale(globaly1,xspan,ox);
			}

			bakx1 = (bakx1>>4)-(xdim<<7); baky1 = (baky1>>4)-(ydim<<7);
			globalposx = dmulscale28(-baky1,globalx1,-bakx1,globaly1);
			globalposy = dmulscale28(bakx1,globalx2,-baky1,globaly2);

			if ((spr->cstat&2) == 0)
				msethlineshift(ox,oy);
			else
				tsethlineshift(ox,oy);

			if ((spr->cstat&0x4) > 0) globalx1 = -globalx1, globaly1 = -globaly1, globalposx = -globalposx;
			asm1 = (globaly1<<2); globalx1 <<= 2; globalposx <<= (20+2);
			asm2 = (globalx2<<2); globaly2 <<= 2; globalposy <<= (20+2);

			fillpolygon(npoints);
		}
	}
}

clippoly (long npoints, long clipstat)
{
	long z, zz, s1, s2, t, npoints2, start2, z1, z2, z3, z4, splitcnt;
	long cx1, cy1, cx2, cy2;

	cx1 = windowx1;
	cy1 = windowy1;
	cx2 = windowx2+1;
	cy2 = windowy2+1;
	cx1 <<= 12; cy1 <<= 12; cx2 <<= 12; cy2 <<= 12;

	if (clipstat&0xa)   //Need to clip top or left
	{
		npoints2 = 0; start2 = 0; z = 0; splitcnt = 0;
		do
		{
			s2 = cx1-rx1[z];
			do
			{
				zz = xb1[z]; xb1[z] = -1;
				s1 = s2; s2 = cx1-rx1[zz];
				if (s1 < 0)
				{
					rx2[npoints2] = rx1[z]; ry2[npoints2] = ry1[z];
					xb2[npoints2] = npoints2+1; npoints2++;
				}
				if ((s1^s2) < 0)
				{
					rx2[npoints2] = rx1[z]+scale(rx1[zz]-rx1[z],s1,s1-s2);
					ry2[npoints2] = ry1[z]+scale(ry1[zz]-ry1[z],s1,s1-s2);
					if (s1 < 0) p2[splitcnt++] = npoints2;
					xb2[npoints2] = npoints2+1;
					npoints2++;
				}
				z = zz;
			} while (xb1[z] >= 0);

			if (npoints2 >= start2+3)
				xb2[npoints2-1] = start2, start2 = npoints2;
			else
				npoints2 = start2;

			z = 1;
			while ((z < npoints) && (xb1[z] < 0)) z++;
		} while (z < npoints);
		if (npoints2 <= 2) return(0);

		for(z=1;z<splitcnt;z++)
			for(zz=0;zz<z;zz++)
			{
				z1 = p2[z]; z2 = xb2[z1]; z3 = p2[zz]; z4 = xb2[z3];
				s1  = klabs(rx2[z1]-rx2[z2])+klabs(ry2[z1]-ry2[z2]);
				s1 += klabs(rx2[z3]-rx2[z4])+klabs(ry2[z3]-ry2[z4]);
				s2  = klabs(rx2[z1]-rx2[z4])+klabs(ry2[z1]-ry2[z4]);
				s2 += klabs(rx2[z3]-rx2[z2])+klabs(ry2[z3]-ry2[z2]);
				if (s2 < s1)
					{ t = xb2[p2[z]]; xb2[p2[z]] = xb2[p2[zz]]; xb2[p2[zz]] = t; }
			}


		npoints = 0; start2 = 0; z = 0; splitcnt = 0;
		do
		{
			s2 = cy1-ry2[z];
			do
			{
				zz = xb2[z]; xb2[z] = -1;
				s1 = s2; s2 = cy1-ry2[zz];
				if (s1 < 0)
				{
					rx1[npoints] = rx2[z]; ry1[npoints] = ry2[z];
					xb1[npoints] = npoints+1; npoints++;
				}
				if ((s1^s2) < 0)
				{
					rx1[npoints] = rx2[z]+scale(rx2[zz]-rx2[z],s1,s1-s2);
					ry1[npoints] = ry2[z]+scale(ry2[zz]-ry2[z],s1,s1-s2);
					if (s1 < 0) p2[splitcnt++] = npoints;
					xb1[npoints] = npoints+1;
					npoints++;
				}
				z = zz;
			} while (xb2[z] >= 0);

			if (npoints >= start2+3)
				xb1[npoints-1] = start2, start2 = npoints;
			else
				npoints = start2;

			z = 1;
			while ((z < npoints2) && (xb2[z] < 0)) z++;
		} while (z < npoints2);
		if (npoints <= 2) return(0);

		for(z=1;z<splitcnt;z++)
			for(zz=0;zz<z;zz++)
			{
				z1 = p2[z]; z2 = xb1[z1]; z3 = p2[zz]; z4 = xb1[z3];
				s1  = klabs(rx1[z1]-rx1[z2])+klabs(ry1[z1]-ry1[z2]);
				s1 += klabs(rx1[z3]-rx1[z4])+klabs(ry1[z3]-ry1[z4]);
				s2  = klabs(rx1[z1]-rx1[z4])+klabs(ry1[z1]-ry1[z4]);
				s2 += klabs(rx1[z3]-rx1[z2])+klabs(ry1[z3]-ry1[z2]);
				if (s2 < s1)
					{ t = xb1[p2[z]]; xb1[p2[z]] = xb1[p2[zz]]; xb1[p2[zz]] = t; }
			}
	}
	if (clipstat&0x5)   //Need to clip bottom or right
	{
		npoints2 = 0; start2 = 0; z = 0; splitcnt = 0;
		do
		{
			s2 = rx1[z]-cx2;
			do
			{
				zz = xb1[z]; xb1[z] = -1;
				s1 = s2; s2 = rx1[zz]-cx2;
				if (s1 < 0)
				{
					rx2[npoints2] = rx1[z]; ry2[npoints2] = ry1[z];
					xb2[npoints2] = npoints2+1; npoints2++;
				}
				if ((s1^s2) < 0)
				{
					rx2[npoints2] = rx1[z]+scale(rx1[zz]-rx1[z],s1,s1-s2);
					ry2[npoints2] = ry1[z]+scale(ry1[zz]-ry1[z],s1,s1-s2);
					if (s1 < 0) p2[splitcnt++] = npoints2;
					xb2[npoints2] = npoints2+1;
					npoints2++;
				}
				z = zz;
			} while (xb1[z] >= 0);

			if (npoints2 >= start2+3)
				xb2[npoints2-1] = start2, start2 = npoints2;
			else
				npoints2 = start2;

			z = 1;
			while ((z < npoints) && (xb1[z] < 0)) z++;
		} while (z < npoints);
		if (npoints2 <= 2) return(0);

		for(z=1;z<splitcnt;z++)
			for(zz=0;zz<z;zz++)
			{
				z1 = p2[z]; z2 = xb2[z1]; z3 = p2[zz]; z4 = xb2[z3];
				s1  = klabs(rx2[z1]-rx2[z2])+klabs(ry2[z1]-ry2[z2]);
				s1 += klabs(rx2[z3]-rx2[z4])+klabs(ry2[z3]-ry2[z4]);
				s2  = klabs(rx2[z1]-rx2[z4])+klabs(ry2[z1]-ry2[z4]);
				s2 += klabs(rx2[z3]-rx2[z2])+klabs(ry2[z3]-ry2[z2]);
				if (s2 < s1)
					{ t = xb2[p2[z]]; xb2[p2[z]] = xb2[p2[zz]]; xb2[p2[zz]] = t; }
			}


		npoints = 0; start2 = 0; z = 0; splitcnt = 0;
		do
		{
			s2 = ry2[z]-cy2;
			do
			{
				zz = xb2[z]; xb2[z] = -1;
				s1 = s2; s2 = ry2[zz]-cy2;
				if (s1 < 0)
				{
					rx1[npoints] = rx2[z]; ry1[npoints] = ry2[z];
					xb1[npoints] = npoints+1; npoints++;
				}
				if ((s1^s2) < 0)
				{
					rx1[npoints] = rx2[z]+scale(rx2[zz]-rx2[z],s1,s1-s2);
					ry1[npoints] = ry2[z]+scale(ry2[zz]-ry2[z],s1,s1-s2);
					if (s1 < 0) p2[splitcnt++] = npoints;
					xb1[npoints] = npoints+1;
					npoints++;
				}
				z = zz;
			} while (xb2[z] >= 0);

			if (npoints >= start2+3)
				xb1[npoints-1] = start2, start2 = npoints;
			else
				npoints = start2;

			z = 1;
			while ((z < npoints2) && (xb2[z] < 0)) z++;
		} while (z < npoints2);
		if (npoints <= 2) return(0);

		for(z=1;z<splitcnt;z++)
			for(zz=0;zz<z;zz++)
			{
				z1 = p2[z]; z2 = xb1[z1]; z3 = p2[zz]; z4 = xb1[z3];
				s1  = klabs(rx1[z1]-rx1[z2])+klabs(ry1[z1]-ry1[z2]);
				s1 += klabs(rx1[z3]-rx1[z4])+klabs(ry1[z3]-ry1[z4]);
				s2  = klabs(rx1[z1]-rx1[z4])+klabs(ry1[z1]-ry1[z4]);
				s2 += klabs(rx1[z3]-rx1[z2])+klabs(ry1[z3]-ry1[z2]);
				if (s2 < s1)
					{ t = xb1[p2[z]]; xb1[p2[z]] = xb1[p2[zz]]; xb1[p2[zz]] = t; }
			}
	}
	return(npoints);
}

fillpolygon(long npoints)
{
	long z, zz, zzz, x1, y1, x2, y2, miny, maxy, x, y, xinc, cnt;
	long ox, oy, bx, by, bxinc, byinc, xend, p, r, day1, day2;
	short *ptr, *ptr2;

	miny = 0x7fffffff; maxy = 0x80000000;
	for(z=npoints-1;z>=0;z--)
		{ y = ry1[z]; miny = min(miny,y); maxy = max(maxy,y); }
	miny = (miny>>12); maxy = (maxy>>12);
	if (miny < 0) miny = 0;
	if (maxy >= ydim) maxy = ydim-1;
	ptr = smost;    //They're pointers! - watch how you optimize this thing
	for(y=miny;y<=maxy;y++)
	{
		dotp1[y] = ptr; dotp2[y] = ptr+(MAXNODESPERLINE>>1);
		ptr += MAXNODESPERLINE;
	}

	for(z=npoints-1;z>=0;z--)
	{
		zz = xb1[z];
		y1 = ry1[z]; day1 = (y1>>12);
		y2 = ry1[zz]; day2 = (y2>>12);
		if (day1 != day2)
		{
			x1 = rx1[z]; x2 = rx1[zz];
			xinc = divscale12(x2-x1,y2-y1);
			if (day2 > day1)
			{
				x1 += mulscale12((day1<<12)+4095-y1,xinc);
				for(y=day1;y<day2;y++) { *dotp2[y]++ = (x1>>12); x1 += xinc; }
			}
			else
			{
				x2 += mulscale12((day2<<12)+4095-y2,xinc);
				for(y=day2;y<day1;y++) { *dotp1[y]++ = (x2>>12); x2 += xinc; }
			}
		}
	}

	globalx1 = mulscale16(globalx1,xyaspect);
	globaly2 = mulscale16(globaly2,xyaspect);

	oy = miny+1-(ydim>>1);
	globalposx += oy*globalx1;
	globalposy += oy*globaly2;

	setuphlineasm4(asm1,asm2);

	ptr = smost;
	for(y=miny;y<=maxy;y++)
	{
		cnt = dotp1[y]-ptr; ptr2 = ptr+(MAXNODESPERLINE>>1);
		for(z=cnt-1;z>=0;z--)
		{
			day1 = 0; day2 = 0;
			for(zz=z;zz>0;zz--)
			{
				if (ptr[zz] < ptr[day1]) day1 = zz;
				if (ptr2[zz] < ptr2[day2]) day2 = zz;
			}
			x1 = ptr[day1]; ptr[day1] = ptr[z];
			x2 = ptr2[day2]-1; ptr2[day2] = ptr2[z];
			if (x1 > x2) continue;

			if (globalpolytype < 1)
			{
					//maphline
				ox = x2+1-(xdim>>1);
				bx = ox*asm1 + globalposx;
				by = ox*asm2 - globalposy;

				p = ylookup[y]+x2+frameplace;
				hlineasm4(x2-x1,-1L,globalshade<<8,by,bx,p);
			}
			else
			{
					//maphline
				ox = x1+1-(xdim>>1);
				bx = ox*asm1 + globalposx;
				by = ox*asm2 - globalposy;

				p = ylookup[y]+x1+frameplace;
				if (globalpolytype == 1)
					mhline(globalbufplc,bx,(x2-x1)<<16,0L,by,p);
				else
				{
					thline(globalbufplc,bx,(x2-x1)<<16,0L,by,p);
					transarea += (x2-x1);
				}
			}
		}
		globalposx += globalx1;
		globalposy += globaly2;
		ptr += MAXNODESPERLINE;
	}
	faketimerhandler();
}
*/

clearview(long dacol);

clearallviews(long dacol);
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

setviewtotile(short tilenume, long xsiz, long ysiz);

setviewback();

squarerotatetile(short tilenume);

preparemirror(long dax, long day, long daz, short daang, long dahoriz, short dawall, short dasector, long *tposx, long *tposy, short *tang);

completemirror();

sectorofwall(short theline);

getceilzofslope(short sectnum, long dax, long day);

getflorzofslope(short sectnum, long dax, long day);

getzsofslope(short sectnum, long dax, long day, long *ceilz, long *florz);
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

alignflorslope(short dasect, long x, long y, long z)
{
	long i, dax, day;
	walltype *wal;

	wal = &wall[sector[dasect].wallptr];
	dax = wall[wal->point2].x-wal->x;
	day = wall[wal->point2].y-wal->y;

	i = (y-wal->y)*dax - (x-wal->x)*day; if (i == 0) return;
	sector[dasect].floorheinum = scale((z-sector[dasect].floorz)<<8,
												  nsqrtasm(dax*dax+day*day),i);

	if (sector[dasect].floorheinum == 0) sector[dasect].floorstat &= ~2;
											  else sector[dasect].floorstat |= 2;
}

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

getpalookup(long davis, long dashade)
{
	return(min(max(dashade+(davis>>8),0),numpalookups-1));
}

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
	r.x.eax = 0x3500+0x70;   /* DOS get vector (INT 0Ch) */
	sr.ds = sr.es = 0;
	int386x(0x21,&r,&r,&sr);
	rtcopmsel = (unsigned short)sr.es;
	rtcopmoff = r.x.ebx;

		//Get old real mode handler
	r.x.eax = 0x0200;   /* DPMI get real mode vector */
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
	r.x.eax = 0x2500+0x70;   /* DOS set vector (INT 0Ch) */
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
	r.x.eax = 0x2500+0x70;   /* DOS set vector (INT 0Ch) */
	r.x.edx = rtcopmoff;
	sr.ds = rtcopmsel;    /* DS:EDX == &handler */
	sr.es = 0;
	int386x(0x21,&r,&r,&sr);

		//restore old real mode handler
	r.x.eax = 0x0201;   /* DPMI set real mode vector */
	r.h.bl = 0x70;
	r.x.ecx = (unsigned long)rtcormseg;     //CX:DX == real mode &handler
	r.x.edx = (unsigned long)rtcormoff;
	int386(0x31,&r,&r);
}

loopnumofsector(short sectnum, short wallnum)
{
	long i, numloops, startwall, endwall;

	numloops = 0;
	startwall = sector[sectnum].wallptr;
	endwall = startwall + sector[sectnum].wallnum;
	for(i=startwall;i<endwall;i++)
	{
		if (i == wallnum) return(numloops);
		if (wall[i].point2 < i) numloops++;
	}
	return(-1);
}

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

*/