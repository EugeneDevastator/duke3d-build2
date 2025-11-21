// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include "build.h"
#include "pragmas.h"


#define MAXMENUFILES 256
#define updatecrc16(crc,dat) (crc = (((crc<<8)&65535)^crctable[((((unsigned short)crc)>>8)&65535)^dat]))
static long crctable[256];
static char kensig[24];

extern void ExtLoadMap(const char *mapname);
extern void ExtSaveMap(const char *mapname);
extern const char *ExtGetSectorCaption(short sectnum);
extern const char *ExtGetWallCaption(short wallnum);
extern const char *ExtGetSpriteCaption(short spritenum);
extern void ExtShowSectorData(short sectnum);
extern void ExtShowWallData(short wallnum);
extern void ExtShowSpriteData(short spritenum);
extern void ExtEditSectorData(short sectnum);
extern void ExtEditWallData(short wallnum);
extern void ExtEditSpriteData(short spritenum);

// void (__interrupt __far *oldtimerhandler)();
// void __interrupt __far timerhandler()

#define KEYFIFOSIZ 64
//void (__interrupt __far *oldkeyhandler)();
// void __interrupt __far keyhandler()
volatile char keystatus[256], keyfifo[KEYFIFOSIZ], keyfifoplc, keyfifoend;
volatile char readch, oldreadch, extended, keytemp;

long vel, svel, angvel;

#define NUMKEYS 19
char buildkeys[NUMKEYS] =
{
	0xc8,0xd0,0xcb,0xcd,0x2a,0x9d,0x1d,0x39,
	0x1e,0x2c,0xd1,0xc9,0x33,0x34,
	0x9c,0x1c,0xd,0xc,0xf,
};

long posx, posy, posz, horiz_ = 100;
short ang, cursectnum;
long hvel;

static long synctics = 0, lockclock = 0;

extern long stereomode;
extern char vgacompatible;

extern char picsiz[MAXTILES];
extern long startposx, startposy, startposz;
extern short startang, startsectnum;
extern long frameplace, pageoffset, ydim16;

static long cachesize, artsize;

static short oldmousebstatus = 0, brightness = 0;
long zlock = 0x7fffffff, zmode = 0, whitecol, kensplayerheight = 32;
short defaultspritecstat = 0;

static short localartfreq[MAXTILES];
static short localartlookup[MAXTILES], localartlookupnum;

char tempbuf[4096];

char names[MAXTILES][17];

short asksave = 0;
extern short editstatus, searchit;
extern long searchx, searchy;                          //search input
extern short searchsector, searchwall, searchstat;     //search output

extern short pointhighlight, linehighlight, highlightcnt;
short grid = 3, gridlock = 1, showtags = 1;
long zoom = 768, gettilezoom = 1;

long numsprites;

short highlight[MAXWALLS];
short highlightsector[MAXSECTORS], highlightsectorcnt = -1;
extern char textfont[128][8];
uint32_t picanm[MAXTILES];
static char pskysearch[MAXSECTORS];

short temppicnum, tempcstat, templotag, temphitag, tempextra;
char tempshade, temppal, tempvis, tempxrepeat, tempyrepeat;
char somethingintab = 255;
//static char boardfilename[13], oboardfilename[13];

static long repeatcountx, repeatcounty;

static char menuname[MAXMENUFILES][17], curpath[80], menupath[80];
static long menunamecnt, menuhighlight;

static long fillist[640];

static char scantoasc[128] =
{
	0,0,'1','2','3','4','5','6','7','8','9','0','-','=',0,0,
	'q','w','e','r','t','y','u','i','o','p','[',']',0,0,'a','s',
	'd','f','g','h','j','k','l',';',39,'`',0,92,'z','x','c','v',
	'b','n','m',',','.','/',0,'*',0,32,0,0,0,0,0,0,
	0,0,0,0,0,0,0,'7','8','9','-','4','5','6','+','1',
	'2','3','0','.',0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};
static char scantoascwithshift[128] =
{
	0,0,'!','@','#','$','%','^','&','*','(',')','_','+',0,0,
	'Q','W','E','R','T','Y','U','I','O','P','{','}',0,0,'A','S',
	'D','F','G','H','J','K','L',':',34,'~',0,'|','Z','X','C','V',
	'B','N','M','<','>','?',0,'*',0,32,0,0,0,0,0,0,
	0,0,0,0,0,0,0,'7','8','9','-','4','5','6','+','1',
	'2','3','0','.',0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

