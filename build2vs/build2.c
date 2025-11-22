#include "shadowtest2.h"
#include "Core/physics.h"
#if 0 //To compile, type: nmake build2.c

!if "$(_NMAKE_VER)" == "6.00.8168.0"
compopts=/MD /G6
linkopts=/opt:nowin98
!else
compopts=/MT
linkopts=
!endif

build2.exe: build2.c\
		  build2.obj shadowtest2.obj drawpoly.obj drawcone.obj drawkv6.obj morph.obj kplib.obj winmain.obj
	link build2.obj shadowtest2.obj drawpoly.obj drawcone.obj drawkv6.obj morph.obj kplib.obj winmain.obj\
	ddraw.lib dinput8.lib dxguid.lib ole32.lib user32.lib gdi32.lib winmm.lib $(linkopts)
	del build2.obj

#zbufmode=/DUSEINTZ
zbufmode=

build2.obj:      build2.c drawpoly.h sysmain.h; cl /c /TP build2.c               /GFy /Gs $(compopts)         $(zbufmode) /DSTANDALONE
shadowtest2.obj: shadowtest2.c                ; cl /c /TP shadowtest2.c /Ox      /GFy /Gs $(compopts) /QIfist $(zbufmode)
drawpoly.obj:    drawpoly.c drawpoly.h        ; cl /c /TP drawpoly.c    /Ox      /GFy /Gs $(compopts) /QIfist $(zbufmode)
drawcone.obj:    drawcone.c                   ; cl /c /TP drawcone.c    /Ox /Ob2 /GFy /Gs $(compopts) /QIfist $(zbufmode)
drawkv6.obj:     drawkv6.c                    ; cl /c /TP drawkv6.c     /Ox /Ob2 /GFy /Gs $(compopts) /QIfist $(zbufmode) /DUSEKZ
morph.obj:       morph.c                      ; cl /c /TP morph.c       /Ox      /GFy /Gs $(compopts) /QIfist $(zbufmode)
kplib.obj:       kplib.c                      ; cl /c /TP kplib.c       /Ox /Ob2 /GFy /Gs $(compopts)
winmain.obj:     winmain.cpp                  ; cl /c /TP winmain.cpp   /Ox /Ob2 /GFy /Gs $(compopts)
!if 0
#endif

#define USEHEIMAP 1

#define NOSOUND 1
#define STANDALONE 1
#define OOS_CHECK 1
#if (OOS_CHECK != 0)
long dispoos = 0, totcrcbytes = 0;
#endif

#if 0
Use Ctrl+Enter on the following lines in MEPAINT, press use your Visual C macro hotkey to run

	//Single player:
topic sos_test3.map

	//Ken launching multi:
topic sos_test3.map /host

	//Other person launching multi:
topic sos_test3.map /join:68.?.?.?

	//Display current topic
print \T

#endif

	//New:
	// * When joining sectors: Exception 0xc0000005 @ 0x0042b5fa in build2.exe, reading location 0x090561fc
	// * LMB on floor does not hold!
	// * When dragging verts in triangle sectors, update slopes to match.
	// * KP key to mirror texture
	// * Bug: do not put ASCII chars >= 128 in chat
	// * ESC in color select mode does not work on sprites
	// * Do not allow simultaneous color editing of same surface?
	// * Allow finer control for KP2468 scaling
	// * Fix multi-mode screen peek's center when viewing different screen size
	// * Ctrl+Enter on ceil/flor does search on same texture
	//
	//High prio:
	// * drag vertex up/down in Z: neighbors are locked; rest recalculate.
	// * filename based animation: fire__{frame#}_{fd/bk/pp}_{#=msec/frame}.png
	// * generate fractal rocks/stalactites
	// * make file MAXMENUFILES&MAXMENUFILEBYTES dynamic!
	// * masking&translucence for shadowtest2:drawpollig()
	// * full screen mode load/save dialogs appear 50% of time
	// * transparency for drawpoly.c
	//
	//Low prio:
	// * method to link sectors for multi-sector elevator. Hitags? User string?
	// * implement xsurf for multi-surface walls
	// * drop-in networking: send game state (GST) at join
	// * background file xfer for textures/models
	// * pushmove (see balloon.kc)
	// * use cylindrical collision for face sprites
	// * don't save: /tag=0 to chat log
	// * boolean bugs: near-invalid sectors: collinear lines going back on self
	// * BUILD2.CFG:key settings, 2d&3d mouse sensitivities
	// * ctrl+Enter on ceil/flor modifies neighbor coplanar surfaces w/same texture
	// * //ctrl+Del on ceil/flor of SOS should kill void section (copy ceil&flor)
	// * clean heightmap artifacts/optimize using walvox morphing tech
	// * local save/load/quit
	// * add rotation/shear texture alignment tool (MAP_UXY/MAP_VXY to Evaldraw lib)
	// * change "..." over avatar to last 16 chars
	// * avatar eyes aren't good in 2d view
	// * personal sector lines should be different colors
	// * mirrors: would make emoticon testing easier
	// * demo recording (even though OOS likely after 1 compile :/)
	// * RR-style tiled texture mapping?
	// * Eval generated&cached textures
	// * Emoticon for bugs flying around head :P
	// * Certain textures in BUILDING.MAP fail with /lights=0
	// * Pass CRC32 with filename and update script if necessary
	//
	// Ideas inspired by reading swpbuildhelp.txt:
	// * ESC menu should include: (N)ew, (L)oad, (S)ave, Save (A)s, (Q)uit
	// * organize texture tool (save in txt file in build2 root dir)
	// * make floor textures 2468 keys direction based on your viewing orientation
	// * shadowtest2: mode to allow viewing from above ceiling
	// * Ctrl+Shift+Enter: remove sectors with <= 0 area
	// * tags should allow full strings
	// * O: on walls should cycle through textures being sideways
	//
	//09/24/2007: Build2 speed test on C2Q 2.66: 1920x1200x32, anginc=2, usemorph=0, force vsync off, numbers in fps:
	//#cpu: -:   I:   H:  HI:
	//  1: 73.5 42.1 13.4 15.1
	//  2: 78.7 60.7 20.4 22.6
	//  3: 84.1 77.4 26.8 29.3
	//  4: 86.7 85.0 31.0 33.4
	//
	//11/08/2006: the BUILD2 team (as I see it):
	// * Ken Silverman    100% programmer + 50% map designer
	// * Dennis Radon     50% map designer
	// * Tom Dobrowolski  interested observer
	// * Ashraf Eassa     interested observer
	// * Jonathon Fowler  interested observer
	// * David Blake      saw it once
	// * Alan Silverman   saw it once

#define MAXXDIM 4096
#define MAXYDIM 2160
#define SCISDIST .001
#define USEMORPH 0
#define USEGROU 1    //if (0) { must remove /DUSEINTZ from drawcone.c compile line! }

#include <basetsd.h>
#ifdef USEGROU
#include "drawpoly.h"
#endif
#define BUILD2
#include "build2.h"
#include "sysmain.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <malloc.h>
#define PI 3.141592653589793

	//DRAWCONE.H:
extern void drawcone_setup (int, int, tiletype *, intptr_t,  point3d *,  point3d *,  point3d *,  point3d *, double, double, double);
extern void drawcone_setup (int, int, tiletype *, intptr_t, dpoint3d *, dpoint3d *, dpoint3d *, dpoint3d *, double, double, double);
extern void drawsph (double, double, double, double, int, double);
extern void drawcone (double, double, double, double, double, double, double, double, int, double, int);
#define DRAWCONE_NOCAP0 1
#define DRAWCONE_NOCAP1 2
#define DRAWCONE_FLAT0 4
#define DRAWCONE_FLAT1 8
#define DRAWCONE_CENT 16

	//SHADOWTEST2.H:
#define LIGHTMAX 256 //FIX:make dynamic!
gamestate_t sst, pst, *gst;
extern int shadowtest2_numlights, shadowtest2_useshadows, shadowtest2_numcpu;
extern int shadowtest2_rendmode, eyepoln, glignum;
extern unsigned int *shadowtest2_sectgot;
extern float shadowtest2_ambrgb[3];
extern void drawsprites (void);
extern void htrun (void (*dacallfunc)(int), int v0, int v1, int danumcpu);
#define MAXCPU 64

int shadowtest2_updatelighting = 1;

	//DRAWKV6.H:
typedef struct
{
	float hx[8], hy[8], hz[8], rhzup20[8];
	short wmin[8], wmax[8];
	short ighyxyx[4], igyxyx[4]; //32-bit only!
	intptr_t ddp, ddf, ddx, ddy, zbufoff;
	point3d p, r, d, f;
} drawkv6_frame_t;
typedef struct { int col; unsigned short z; char vis, dir; } kv6voxtype;
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
extern void drawkv6_init (void);
extern void drawkv6_setup (drawkv6_frame_t *frame, tiletype *, intptr_t,  point3d *,  point3d *,  point3d *,  point3d *, float, float, float);
extern void drawkv6_setup (drawkv6_frame_t *frame, tiletype *, intptr_t, dpoint3d *, dpoint3d *, dpoint3d *, dpoint3d *, float, float, float);
extern kv6data_t *drawkv6_get (char *);
extern void drawkv6 (drawkv6_frame_t *frame, kv6data_t *, float, float, float, float, float, float, float, float, float, float, float, float, int, float);
extern void drawkv6_freeall (void);
typedef struct { int sect; point3d p; float rgb[3]; int useshadow; } drawkv6_lightpos_t;
extern drawkv6_lightpos_t drawkv6_light[MAXLIGHTS];
extern int drawkv6_numlights;
extern float drawkv6_ambrgb[3];
extern drawkv6_frame_t drawkv6_frame;

#if USEMORPH
	//MORPH.H:
extern void morph_init (long, long, long, void (*)(cam_t *));
extern void morph_sleepuntilretrace (void);
extern void morph_drawframe (tiltyp *, long, point3d *, point3d *, point3d *, point3d *, float, float, float);
extern void morph_uninit (void);
#endif

static int cputype = 0;


void (*drawpolfunc)(cam_t *cc, kgln_t *vert, long num, tile_t *tpic, long curcol, float *ouvmat, point3d *norm, long flags);

static long *zbuffermem = 0, zbuffersiz = 0;

#ifdef STANDALONE
static char message[256] = {0};
static double messagetimeout = 0.0;

static char logfilnam[MAX_PATH] = {0};

static double odtotclk, dtotclk;
static double odtotclk2, dtotclk2;
static long doquitloop = 0;

static char *showhelpbuf = 0;
static float showhelppos = 0.f, showhelpgoal = 0.f;
static long showhelpbufleng = 0, showhelpbuflines = 0;

static long mouseacquired = 0;
static HCURSOR ghblankcurs = 0, ghcrosscurs = 0;

	//FPS counter
#define FPSSIZ 128
static long fpsometer[2][FPSSIZ], fpsind[2][FPSSIZ], numframes[2] = {0,0}, microsec[2], showfps = 0;

static long folder  [64+1][64]; //Icon for selecting files in 'v' mode
static long upfolder[64+1][64]; //Icon for selecting files in 'v' mode
#endif
static long nullpic [64+1][64]; //Null set icon (image not found)

//App state variables: -----------------------------------------------------------------------------

static playerstruct_t *gps;
playerstruct_t *gdps; //global temp variable

static int numplayers, moveindex, viewindex;

#ifdef STANDALONE
#define DVERTSNAP (1.0) //Vertex snap distance in grid units
#define DEDGESNAP (0.5) //Edge   snap distance in grid units
enum
{
	GRABSURF,GRABDRAG,GRABDRAG2,GRABDRAG3,
	GRABJOIN,GRABHOME,GRABEND,GRABCIRC,
	GRABCORNVERT,GRABCORNSECT,
	GRABFILE,GRABRGB,
};
static gamestate_t *sndst; //ss=synced state, ps=predicted state, gs=global state pointer

//--------------------------------------------------------------------------------------------------

#define NUMOUSBUTS 4 //of course I only have 2 on my trackball..
#if (STANDALONE)
typedef struct { unsigned char scancode; const char *nam; } keynames_t;
static const keynames_t keynames[] =
{
	0x01,"ESC",       0x02,"1",           0x03,"2",        0x04,"3",           0x05,"4",         0x06,"5",         0x07,"6",          0x08,"7",
	0x09,"8",         0x0a,"9",           0x0b,"0",        0x0c,"MINUS",       0x0c,"HYPHEN",    0x0d,"EQU",       0x0d,"EQUAL",      0x0d,"EQUALS",
	0x0e,"BACK",      0x0e,"BACKSPC",     0x0e,"BACKSPACE",0x0f,"TAB",         0x10,"Q",         0x11,"W",         0x12,"E",          0x13,"R",
	0x14,"T",         0x15,"Y",           0x16,"U",        0x17,"I",           0x18,"O",         0x19,"P",         0x1a,"[",          0x1a,"LEFTBRACKET",
	0x1b,"]",         0x1b,"RIGHTBRACKET",0x1c,"ENTER",    0x1d,"LCTRL",       0x1d,"LEFTCTRL",  0x1e,"A",         0x1f,"S",          0x20,"D",
	0x21,"F",         0x22,"G",           0x23,"H",        0x24,"J",           0x25,"K",         0x26,"L",         0x27,";",          0x27,":",
	0x27,"SEMI",      0x27,"COLON",       0x27,"SEMICOLON",0x28,"'",           0x28,"\"",        0x28,"APOSTROPHE",0x28,"QUOTE",      0x29,"`",
	0x29,"~",         0x29,"TICK",        0x29,"TILDA",    0x2a,"LSHIFT",      0x2a,"LEFTSHIFT", 0x2b,"\\",        0x2b,"|",          0x2b,"BACKSLASH",
	0x2c,"Z",         0x2d,"X",           0x2e,"C",        0x2f,"V",           0x30,"B",         0x31,"N",         0x32,"M",          0x33,",",
	0x33,"<",         0x33,"COMMA",       0x33,"LESSTHAN", 0x34,".",           0x34,">",         0x34,"PERIOD",    0x34,"GREATERTHAN",0x35,"/",
	0x35,"?",         0x35,"SLASH",       0x35,"QUESTION", 0x35,"QUESTIONMARK",0x36,"RSHIFT",    0x36,"RIGHTSHIFT",0x37,"KP*",        0x37,"PADSTAR",
	0x38,"LALT",      0x38,"LEFTALT",     0x39," ",        0x39,"SPC",         0x39,"SPACE",     0x3a,"CAPS",      0x3a,"CAPSLOCK",   0x3b,"F1",
	0x3c,"F2",        0x3d,"F3",          0x3e,"F4",       0x3f,"F5",          0x40,"F6",        0x41,"F7",        0x42,"F8",         0x43,"F9",
	0x44,"F10",       0x45,"NUM",         0x45,"NUMLOCK",  0x46,"SCROLL",      0x46,"SCROLLLOCK",0x47,"KP7",       0x47,"PAD7",       0x48,"KP8",
	0x48,"PAD8",      0x49,"KP9",         0x49,"PAD9",     0x4a,"KP-",         0x4a,"PAD-",      0x4a,"PADMINUS",  0x4b,"KP4",        0x4b,"PAD4",
	0x4c,"KP5",       0x4c,"PAD5",        0x4d,"KP6",      0x4d,"PAD6",        0x4e,"KP+",       0x4e,"PAD+",      0x4e,"PADPLUS",    0x4f,"KP1",
	0x4f,"PAD1",      0x50,"KP2",         0x50,"PAD2",     0x51,"KP3",         0x51,"PAD3",      0x52,"KP0",       0x52,"PAD0",       0x53,"KP.",
	0x53,"PADPERIOD", 0x53,"PAD.",        0x57,"F11",      0x58,"F12",         0x9c,"KPENTER",   0x9c,"PADENTER",  0x9d,"RCTRL",      0x9d,"RIGHTCTRL",
	0xb5,"KP/",       0xb5,"PAD/",        0xb5,"PADSLASH", 0xb8,"RALT",        0xb8,"RIGHTALT",  0xcb,"LEFT",      0xcb,"LEFTARROW",  0xcd,"RIGHT",
	0xcd,"RIGHTARROW",0xc7,"HOME",        0xc8,"UP",       0xc8,"UPARROW",     0xc9,"PGUP",      0xc9,"PAGEUP",    0xcf,"END",        0xd0,"DOWN",
	0xd0,"DOWNARROW", 0xd1,"PGDN",        0xd1,"PAGEDOWN", 0xd2,"INS",         0xd2,"INSERT",    0xd3,"DEL",       0xd3,"DELETE",
};
static const unsigned char keyscan2bit[] = //99 keys ("PrintScrn/SysRq" & "Pause/Break" not represented)
{
	//NOTE: 0x00 must not be defined because of 0 terminator in net protocol
		  0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
	0x50,0x51,0x52,0x53,               0x57,0x58,
																					0x9c,0x9d,
									 0xb5,          0xb8,
												  0xc7,0xc8,0xc9,     0xcb,     0xcd,     0xcf,
	0xd0,0xd1,0xd2,0xd3,
	//NOTE: 0xf0-0xff reserved for mouse buttons 0-15
};
static unsigned char keyscanremap[256], okeyscanremap[256], keyscanremapr[256];
#endif

	//Prediction packet fifo (storage of packets between synced & predicted state)
static unsigned char ppackfif[65536+1024];
static int ppackfifr = 0, ppackfifw = 0; //byte count
static int ppackfifrn = 0, ppackfifwn = 0; //packet count (for debugging only)

	//Command packet fifo (commands can't be sent from in the middle of executepack)
static unsigned char cmdpackfif[65536+1024];
static int cmdpackfifr = 0, cmdpackfifw = 0; //byte count

#define SNDISTSCALEHACK (4.f)
static void myplaysound (const char *filnam, long volperc, float frqmul, void *pos, long flags)
{
#ifndef NOSOUND
	point3d npos;
	if ((!(flags&KSND_3D)) || (!pos)) { playsound(filnam,volperc,frqmul,pos,flags); return; }
	npos.x = ((point3d *)pos)->x*SNDISTSCALEHACK;
	npos.y = ((point3d *)pos)->y*SNDISTSCALEHACK;
	npos.z = ((point3d *)pos)->z*SNDISTSCALEHACK;
	playsound(filnam,volperc,frqmul,&npos,flags);
#endif
}
#endif

//--------------------------------------------------------------------------------------------------

static _inline long testflag (long c)
{
	_asm
	{
		mov ecx, c
		pushfd
		pop eax
		mov edx, eax
		xor eax, ecx
		push eax
		popfd
		pushfd
		pop eax
		xor eax, edx
		mov eax, 1
		jne menostinx
		xor eax, eax
		menostinx:
	}
}

static _inline void cpuid (long a, long *s)
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

	//Bit numbers of return value:
	//0:FPU, 4:RDTSC, 15:CMOV, 22:MMX+, 23:MMX, 25:SSE, 26:SSE2, 30:3DNow!+, 31:3DNow!
static long getcputype (void)
{
	long i, cpb[4], cpid[4];
	if (!testflag(0x200000)) return(0);
	cpuid(0,cpid); if (!cpid[0]) return(0);
	cpuid(1,cpb); i = (cpb[3]&~((1<<22)|(1<<30)|(1<<31)));
	cpuid(0x80000000,cpb);
	if (((unsigned)cpb[0]) > 0x80000000)
	{
		cpuid(0x80000001,cpb);
		i |= (cpb[3]&(1<<31));
		if (!((cpid[1]^0x68747541)|(cpid[3]^0x69746e65)|(cpid[2]^0x444d4163))) //AuthenticAMD
			i |= (cpb[3]&((1<<22)|(1<<30)));
	}
	if (i&(1<<25)) i |= (1<<22); //SSE implies MMX+ support
	return(i);
}
//--------------------------------------------------------------------------------------------------
#ifdef STANDALONE
static long netproto = 0; //0=udp, 1=tcp
//------------------------------------- SIMPLE TCP code begins -------------------------------------
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

#define SIMLAGSIZ 65536
#define SIMLAGOFLOW 1024
static long simlagms = 0, simlagw[MAXPLAYERS] = {0}, simlagr[MAXPLAYERS] = {0}, *simlagtim = 0;
static char *simlagbuf = 0;

static long net_totbytes_write = 0, net_totbytes_read = 0;

static SOCKADDR_IN net_sa;
static WSAEVENT tcp_listenevent = WSA_INVALID_EVENT;
static long tcp_inited = 0, net_isserv = 0;
static char net_err[1024];
static SOCKET listensock = INVALID_SOCKET;

static void tcp_wsaclean (void)
{
	if (!tcp_inited) return;
	if (tcp_listenevent != (WSAEVENT)WSA_INVALID_EVENT) WSACloseEvent(tcp_listenevent);
	if (listensock != INVALID_SOCKET)
		{ closesocket(listensock); listensock = INVALID_SOCKET; }
	WSACleanup(); tcp_inited = 0;
}

static void string2iport (const char *st, long *retip, long *retport)
{
	LPHOSTENT lph;
	char *nst;
	long i;

	for(i=0;st[i];i++)
		if (st[i] == ':') { (*retport) = atoi(&st[i+1]); break; }
	if (!i) return;
	nst = (char *)_alloca(i+1); if (!nst) return;
	memcpy(nst,st,i); nst[i] = 0;
	lph = gethostbyname(nst);
	if (lph) (*retip) = ntohl(*(long *)lph->h_addr); else (*retip) = inet_addr(nst);
}

static SOCKET tcp_open (char *joinip)
{
	WSADATA ws;
	SOCKET sk;
	WSAEVENT hevent = WSA_INVALID_EVENT;
	long i, j, t0;

	if (!tcp_inited)
	{
		if (WSAStartup(0x101,&ws)) { sprintf(net_err,"WSAStartup error: %d",WSAGetLastError()); return(-1); }
		if (ws.wVersion != 0x101) { sprintf(net_err,"WinSock version not supported. Error: %d",WSAGetLastError()); return(-1); }
		atexit(tcp_wsaclean);
		tcp_inited = 1;
	}

	sk = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (sk == INVALID_SOCKET) { sprintf(net_err,"Socket error: %d",WSAGetLastError()); return(-1); }
	net_sa.sin_family = AF_INET;
	net_sa.sin_addr.s_addr = INADDR_ANY;
	net_sa.sin_port = htons(32123);
	if (joinip)
	{
		i = j = 0;
		string2iport(joinip,&i,&j);
		if (i) net_sa.sin_addr.s_addr = htonl(i);
		if (j) net_sa.sin_port = htons((short)j);
	}

	//i = 1; setsockopt(sk,IPPROTO_TCP,TCP_NODELAY,(char *)&i,sizeof(int)); //Disable NAGLE buffering of sends

	if (net_sa.sin_addr.s_addr == INADDR_ANY)
	{
		net_isserv = 1; listensock = sk;

		if (bind(listensock,(LPSOCKADDR)&net_sa,sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
			{ sprintf(net_err,"bind error: %d",WSAGetLastError()); return(-1); }
		if (listen(listensock,SOMAXCONN) == SOCKET_ERROR)
			{ sprintf(net_err,"listen error: %d",WSAGetLastError()); return(-1); }

		tcp_listenevent = WSACreateEvent();
		if (tcp_listenevent == WSA_INVALID_EVENT)
			{ sprintf(net_err,"WSACreateEvent error: %d",WSAGetLastError()); return(-1); }
		if (WSAEventSelect(listensock,tcp_listenevent,FD_ACCEPT) == SOCKET_ERROR)
			{ sprintf(net_err,"WSAEventSelect error: %d",WSAGetLastError()); return(-1); }
	}
	else
	{
		net_isserv = 0; listensock = INVALID_SOCKET;

		hevent = WSACreateEvent();
		if (hevent == WSA_INVALID_EVENT)
			{ sprintf(net_err,"WSACreateEvent error: %d",WSAGetLastError()); return(-1); }
		t0 = GetTickCount();
		while (1)
		{
			if (GetTickCount()-t0 >= 2000) { sprintf(net_err,"Server not responding"); return(-1); }
			if (connect(sk,(LPSOCKADDR)&net_sa,sizeof(SOCKADDR_IN)) != SOCKET_ERROR) break;
			i = WSAGetLastError(); if (i == WSAECONNREFUSED) continue;
			if (i == WSAEADDRNOTAVAIL) { strcpy(net_err,"invalid remote host"); return(-1); }
			sprintf(net_err,"connect error: %d",WSAGetLastError()); return(-1);
		}
		if (WSAEventSelect(sk,hevent,FD_READ) == SOCKET_ERROR)
			{ sprintf(net_err,"WSAEventSelect error: %d",WSAGetLastError()); return(-1); }
		if (hevent != (WSAEVENT)WSA_INVALID_EVENT) WSACloseEvent(hevent);
	}
	return(sk);
}

static SOCKET gsk[MAXPLAYERS] =
{
	INVALID_SOCKET,INVALID_SOCKET,INVALID_SOCKET,INVALID_SOCKET,
};

static SOCKET tcp_getplayer (void) { long i = sizeof(SOCKADDR_IN); return(accept(listensock,(LPSOCKADDR)&net_sa,(int *)&i)); }
static long tcp_write (SOCKET sk, char *b, long l) { l = send(sk,b,l,0); if (l == SOCKET_ERROR) return(0); return(l); }

static long tcp_read (SOCKET sk, char *b, long l)
{
	if (simlagms <= 0)
	{
		l = recv(sk,b,l,0); if (l == SOCKET_ERROR) return(0);
		net_totbytes_read += l; return(l);
	}
	else
	{
		long i, nl, p, tim;

		for(p=0;p<MAXPLAYERS-1;p++) if (sk == gsk[p]) break;
		nl = recv(sk,&simlagbuf[p*(SIMLAGSIZ+SIMLAGOFLOW)+(simlagw[p]&(SIMLAGSIZ-1))],SIMLAGOFLOW,0);
		tim = GetTickCount();
		if (nl != SOCKET_ERROR)
		{
			net_totbytes_read += nl;
			for(i=0;i<nl;i++)
			{
				simlagbuf[p*(SIMLAGSIZ+SIMLAGOFLOW)+((simlagw[p]+i)&(SIMLAGSIZ-1))] = simlagbuf[p*(SIMLAGSIZ+SIMLAGOFLOW)+(simlagw[p]&(SIMLAGSIZ-1))+i];
				simlagtim[p*SIMLAGSIZ+((simlagw[p]+i)&(SIMLAGSIZ-1))] = tim;
			}
			simlagw[p] += nl;
		}
		i = 0;
		while ((i < l) && (simlagr[p]-simlagw[p] < 0) && (tim-simlagtim[p*SIMLAGSIZ+(simlagr[p]&(SIMLAGSIZ-1))] >= simlagms))
			{ b[i] = simlagbuf[p*(SIMLAGSIZ+SIMLAGOFLOW)+(simlagr[p]&(SIMLAGSIZ-1))]; i++; simlagr[p]++; }
		return(i);
	}
}
static void tcp_close (SOCKET sk) { if (sk != INVALID_SOCKET) closesocket(sk); }
//-------------------------------------- SIMPLE TCP code ends --------------------------------------
//----------------------------------------  UDP code begins ----------------------------------------

	//This silly block of code has probably been duplicated about 5 times :P
static long crctab32[256] = {0};  //SEE CRC32.C
#define updatecrc32(c,crc) crc=(crctab32[((c)^crc)&255]^(((unsigned)crc)>>8))
static long crc32_getbuf (char *buf, long leng)
{
	long i, crc = -1;
	for(i=0;i<leng;i++) updatecrc32(buf[i],crc);
	return(crc);
}
static void crc32_init (void)
{
	long i, j, k;
	for(i=255;i>=0;i--)
	{
		k = i; for(j=8;j;j--) k = ((unsigned long)k>>1)^((-(k&1))&0xedb88320);
		crctab32[i] = k;
	}
}

	//UDP simulating TCP packet format:
	//   long udp_rbufw;  //32-bit index to last acked byte receiving
	//   long udp_wbufr;  //32-bit index to first byte in data
	//   short leng;      //length of data
	//   char data[leng]; //window of stream
	//   long crc32;      //crc32 of everything before

static SOCKET udp_mysock = INVALID_SOCKET;
static long udp_inited = 0;
#define UDP_BUFSIZ 16384 //Warning: make sure SIMLAGSIZ is at least this big
#define UDP_ESCCODE_MUL 9 /*pseudorandom escape code to avoid worst case*/
#define UDP_SENDRATE_ACTIVE 64
#define UDP_SENDRATE_IDLE 4
typedef struct
{
	long ip, port, lastsentim, gonetim, wbufw, wbufr, rbufw, rbufr, inesc;
	char wbuf[UDP_BUFSIZ], rbuf[UDP_BUFSIZ]; //512KB :/
} udp_t;
static udp_t udp[MAXPLAYERS];
static long udp_newplay[MAXPLAYERS], udp_newplayw = 0, udp_newplayr = 0;
static long udp_conlist[MAXPLAYERS], udp_conplayers = 0;

static void udp_putbyte (long i, unsigned char ch)
{
	if (simlagms > 0) { simlagtim[i*UDP_BUFSIZ+(udp[i].wbufw&(UDP_BUFSIZ-1))] = GetTickCount()+simlagms; }
	udp[i].wbuf[udp[i].wbufw&(UDP_BUFSIZ-1)] = ch; udp[i].wbufw++; //ESC code
}

static void udp_breath (void)
{
	long i, j, p, q, wbufw, leng, tim;
	char tbuf[512]; //max for UDP would be 1500-overhead

	tim = GetTickCount();
	for(q=udp_conplayers-1;q>=0;q--)
	{
		p = udp_conlist[q];
		if ((udp[p].inesc == 3) && (tim-udp[p].gonetim > 5000)) //kill player slot after disconnected for 5 secs
		{
			//printf("[Kill player slot %d; conplayers:%d->%d]\n",p,udp_conplayers,udp_conplayers-1);
			udp[p].ip = 0; udp[p].port = 0;
			udp[p].lastsentim = 0; udp[p].gonetim = 0;
			udp[p].wbufw = 0; udp[p].wbufr = 0; udp[p].rbufw = 0; udp[p].rbufr = 0;
			if (simlagms > 0) simlagr[p] = 0;
			udp[p].inesc = 0;
			udp_conplayers--; udp_conlist[q] = udp_conlist[udp_conplayers];
			continue;
		}

		//if (net_sendpaused) continue;

		if (simlagms <= 0) wbufw = udp[p].wbufw;
		else
		{
			while ((simlagr[p] < udp[p].wbufw) && (tim-simlagtim[p*UDP_BUFSIZ+(simlagr[p]&(UDP_BUFSIZ-1))] >= 0)) simlagr[p]++;
			wbufw = simlagr[p];
		}

		i = wbufw-udp[p].wbufr;
		if (i > 0) j = 1000/UDP_SENDRATE_ACTIVE; //send full rate if buffer not empty
				else j = 1000/UDP_SENDRATE_IDLE;   //keep alive: send anyway if xmit buffer empty
		if (labs(tim-udp[p].lastsentim) < j) continue;
		udp[p].lastsentim = tim;

		net_sa.sin_family      = AF_INET;
		net_sa.sin_addr.s_addr = htonl(udp[p].ip);
		net_sa.sin_port        = htons(udp[p].port);

			//Send up to sizeof(tbuf)-14 bytes
		leng = min(wbufw-udp[p].wbufr,sizeof(tbuf)-14);
		*(long *)&tbuf[0] = udp[p].rbufw;
		*(long *)&tbuf[4] = udp[p].wbufr;
		*(long *)&tbuf[8] = (short)leng;
		for(i=0;i<leng;i++) tbuf[i+10] = udp[p].wbuf[(udp[p].wbufr+i)&(UDP_BUFSIZ-1)];
		*(long *)&tbuf[leng+10] = crc32_getbuf(tbuf,leng+10);
		leng += 14;

		//if (!(rand()&3)) //Simulate errors (keep disabled!)
		i = sendto(udp_mysock,tbuf,leng,0,(struct sockaddr *)&net_sa,sizeof(sockaddr_in));
		net_totbytes_write += leng;
		//if (i == SOCKET_ERROR) MessageBox(0,"sendto failed",prognam,MB_OK);
	}

	while (1)
	{
		i = sizeof(net_sa);
		leng = recvfrom(udp_mysock,tbuf,sizeof(tbuf),0,(struct sockaddr *)&net_sa,(int *)&i); if (leng <= 0) break;
		net_totbytes_read += leng;

			//convert IP to player index
		for(q=udp_conplayers-1;q>=0;q--)
		{
			p = udp_conlist[q];
			if ((udp[p].ip == ntohl(net_sa.sin_addr.s_addr)) && (udp[p].port == (long)ntohs(net_sa.sin_port))) break;
		}
		if (q < 0)
		{
			for(p=0;p<MAXPLAYERS;p++) if (!udp[p].ip) break; //find available new slot
			if (p >= MAXPLAYERS) continue; //list full
		}

		if (leng != ((long)(*(short *)&tbuf[8]))+14) continue;
		if (crc32_getbuf(tbuf,leng-4) != *(long *)&tbuf[leng-4]) continue;
		if (((signed)((*(long *)&tbuf[0])-udp[p].wbufr)) > 0) udp[p].wbufr = *(long *)&tbuf[0]; //inc ack index
		j = *(long *)&tbuf[4];

		i = udp[p].rbufw-j;
		if (i >= 0)
			for(leng-=14;i<leng;i++)
				{ udp[p].rbuf[udp[p].rbufw&(UDP_BUFSIZ-1)] = tbuf[i+10]; udp[p].rbufw++; }

			//Only add player if crc32 matches and 1st 2 bytes are connect message
		if ((!udp[p].ip) && (udp[p].rbufw >= 2) && (udp[p].rbuf[0] == 0) && (udp[p].rbuf[1] == 0))
		{
			udp[p].ip = ntohl(net_sa.sin_addr.s_addr); udp[p].port = (long)ntohs(net_sa.sin_port);
			udp[p].inesc = 0;
			udp_newplay[udp_newplayw&(MAXPLAYERS-1)] = p; udp_newplayw++;
			udp_conlist[udp_conplayers] = p; udp_conplayers++;
		}
	}
}

static void udp_wsaclean (void)
{
	long i;

	if (!udp_inited) return;
	if ((net_isserv) && (udp_conplayers))
	{
			//NOTE:this is not very reliable :/ if it fails, slaves think master still connected
		i = GetTickCount()+250; do { udp_breath(); } while (GetTickCount() < i);
	}

	if (udp_mysock != INVALID_SOCKET) { closesocket(udp_mysock); udp_mysock = INVALID_SOCKET; }
	WSACleanup(); udp_inited = 0;
}

static long udp_write (SOCKET sk, char *buf, long leng) //0:buffer full... can't send
{
	long i, j, tim;

	if ((unsigned)sk >= (unsigned)MAXPLAYERS) return(0); //FIXFIX:why bad sockets being passed!
	//if (!udp[sk].ip) return(0);
	if (((unsigned)(udp[sk].wbufw-udp[sk].wbufr+leng*2)) > (unsigned)UDP_BUFSIZ) return(0);
	for(i=0;i<leng;i++)
	{
		j = ((udp[sk].wbufw*UDP_ESCCODE_MUL)&255);
		if (buf[i] == j) { udp_putbyte((long)sk,j); udp_putbyte((long)sk,2); continue; } //literal as ESC code
		udp_putbyte((long)sk,buf[i]);
	}
	udp_breath();
	return(1);
}

static long udp_read (SOCKET sk, char *buf, long maxleng) //0:no packets in buffer
{
	long i, p, leng;
	char c;

	if ((unsigned)sk >= (unsigned)MAXPLAYERS) return(0); //FIXFIX:why bad sockets being passed!
	udp_breath(); leng = 0;
	while (((signed)(udp[sk].rbufw-udp[sk].rbufr)) > 0)
	{
		c = udp[sk].rbuf[udp[sk].rbufr&(UDP_BUFSIZ-1)]; udp[sk].rbufr++;
		if (udp[sk].inesc)
		{
				  if (c == 0) { udp[sk].inesc = 0; } //Connect
			else if (c == 1) { udp[sk].inesc = 2; break; } //Disconnect
			else if (c == 2)
			{
				if (udp[sk].inesc == 1) //Ignore literals if not connected
				{
					udp[sk].inesc = 0;
					buf[leng] = (((udp[sk].rbufr-2)*UDP_ESCCODE_MUL)&255); leng++; if (leng >= maxleng) break; //Literal
				}
			}
		}
		else if (c == (((udp[sk].rbufr-1)*UDP_ESCCODE_MUL)&255)) { udp[sk].inesc = 1; }
		else { buf[leng] = c; leng++; if (leng >= maxleng) break; }
	}
	return(leng);
}

static SOCKET udp_getplayer (void)
{
	udp_breath();
	if (udp_newplayr < udp_newplayw) { udp_newplayr++; return(udp_newplay[(udp_newplayr-1)&(MAXPLAYERS-1)]); }
	return(INVALID_SOCKET);
}

static void udp_close (SOCKET sk)
{
	long i;
	char buf[16];

	if ((unsigned)sk >= (unsigned)MAXPLAYERS) return; //FIXFIX:why bad sockets being passed!
	if (!udp[sk].ip) return;
	udp_putbyte((long)sk,((udp[sk].wbufw*UDP_ESCCODE_MUL)&255)); //ESC code
	udp_putbyte((long)sk,1);                                     //disconnect message

	udp_breath();

	if (!net_isserv) //Slave waits for ack
	{
			//This timeout should not occur
		i = GetTickCount()+1000;
		do { udp_read(sk,buf,sizeof(buf)); } while ((udp[sk].inesc < 2) && (GetTickCount() < i));
	}
	else { udp[sk].inesc = 3; udp[sk].gonetim = GetTickCount(); }
}

static SOCKET udp_open (char *joinip)
{
	WSADATA ws;
	long i;

	if (!udp_inited)
	{
		if (WSAStartup(0x101,&ws)) { sprintf(net_err,"WSAStartup error: %d",WSAGetLastError()); return(INVALID_SOCKET); }
		if (ws.wVersion != 0x101) { sprintf(net_err,"WinSock version not supported. Error: %d",WSAGetLastError()); return(INVALID_SOCKET); }
		atexit(udp_wsaclean);
		udp_inited = 1;
		crc32_init();

		for(i=0;i<MAXPLAYERS;i++)
		{
			udp[i].ip = 0; udp[i].port = 0;
			udp[i].lastsentim = 0; udp[i].gonetim = 0;
			udp[i].wbufw = 0; udp[i].wbufr = 0; udp[i].rbufw = 0; udp[i].rbufr = 0;
			if (simlagms > 0) simlagr[i] = 0;
			udp[i].inesc = 0;
		}
	}

	udp_mysock = socket(AF_INET,SOCK_DGRAM,0); if (udp_mysock == INVALID_SOCKET) return(INVALID_SOCKET);
	i = 1; if (ioctlsocket(udp_mysock,FIONBIO,(unsigned long *)&i) == SOCKET_ERROR) return(INVALID_SOCKET);

	net_sa.sin_family = AF_INET;
	net_sa.sin_addr.s_addr = INADDR_ANY;
	net_sa.sin_port = htons(32123);

	udp[0].ip = 0; udp[0].port = 32123;
	if (joinip) string2iport(joinip,&udp[0].ip,&udp[0].port);
	if (udp[0].ip  ) net_sa.sin_addr.s_addr = htonl(       udp[0].ip  );
	if (udp[0].port) net_sa.sin_port        = htons((short)udp[0].port);

	if (net_sa.sin_addr.s_addr == INADDR_ANY)
	{
		net_isserv = 1;
		i = bind(udp_mysock,(struct sockaddr *)&net_sa,sizeof(net_sa));
		if (i == INVALID_SOCKET) { strcpy(net_err,"bind failed (port already in use?)"); return(INVALID_SOCKET); }
	}
	else
	{
		net_isserv = 0;

		udp_putbyte(0,(udp[0].wbufw*UDP_ESCCODE_MUL)&255); //ESC code
		udp_putbyte(0,0);                                  //Connect message
		udp_breath();

		udp_conlist[0] = 0; udp_conplayers = 1;
	}

	return(0);
}
//----------------------------------------- UDP code ends  -----------------------------------------

static void (*net_close) (SOCKET sk);
static SOCKET (*net_getplayer) (void);
static long (*net_write) (SOCKET sk, char *buf, long leng);
static long (*net_read) (SOCKET sk, char *buf, long maxleng);
static void (*net_wsaclean) (void);
static SOCKET net_open (char *joinip)
{
	SOCKET s;
	if (!netproto)
	{
		s = udp_open(joinip);
		net_close     = udp_close;
		net_getplayer = udp_getplayer;
		net_write     = udp_write;
		net_read      = udp_read;
		net_wsaclean  = udp_wsaclean;
	}
	else
	{
		s = tcp_open(joinip);
		net_close     = tcp_close;
		net_getplayer = tcp_getplayer;
		net_write     = tcp_write;
		net_read      = tcp_read;
		net_wsaclean  = tcp_wsaclean;
	}
	net_totbytes_write = 0; net_totbytes_read = 0;
	return(s);
}

static long setclipboardtext (char *st)
{
	HANDLE hbuf;
	long i, j;
	char *cptr;

	for(i=j=0;st[i];i++) if (st[i] == 13) j++;
	if (!OpenClipboard(ghwnd)) return(0);
	EmptyClipboard();
	hbuf = GlobalAlloc(GMEM_MOVEABLE,i+j+1); if (!hbuf) { CloseClipboard(); return(0); }

	cptr = (char *)GlobalLock(hbuf);
	for(i=0;st[i];i++) { *cptr++ = st[i]; if (st[i] == 13) *cptr++ = 10; }
	*cptr++ = 0;
	GlobalUnlock(hbuf);

	SetClipboardData(CF_TEXT,hbuf);
	CloseClipboard();

	//NOTE: Clipboard owns hbuf - don't do "GlobalFree(hbuf);"!
	return(1);
}

static void getclipboardtext (char *st, long maxlen)
{
	HANDLE hbuf;
	char *cptr;

	if (!IsClipboardFormatAvailable(CF_TEXT)) return;
	if (!OpenClipboard(ghwnd)) return;
	hbuf = GetClipboardData(CF_TEXT); if (!hbuf) { CloseClipboard(); return; }
	cptr = (char *)GlobalLock(hbuf); if (!cptr) { CloseClipboard(); return; }

	if (strlen(cptr) < maxlen) strcpy(st,cptr); else { memcpy(st,cptr,maxlen-1); st[maxlen-1] = 0; }

	GlobalUnlock(hbuf);
	CloseClipboard();
}

//----------------------  WIN file select code begins ------------------------

#ifdef _WIN32
#pragma comment(lib,"comdlg32.lib")
#include <commdlg.h>

static char fileselectnam[MAX_PATH+1];
static char *loadfileselect (char *mess, char *spec, char *defext)
{
	long i;
	for(i=0;fileselectnam[i];i++) if (fileselectnam[i] == '/') fileselectnam[i] = '\\';
	OPENFILENAME ofn =
	{
		sizeof(OPENFILENAME),ghwnd,0,spec,0,0,1,fileselectnam,MAX_PATH,0,0,0,mess,
		/*OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|*/ OFN_HIDEREADONLY|OFN_NOCHANGEDIR,0,0,defext,0,0,0
	};
	if (!GetOpenFileName(&ofn)) return(0); else return(fileselectnam);
}
static char *savefileselect (char *mess, char *spec, char *defext)
{
	long i;
	for(i=0;fileselectnam[i];i++) if (fileselectnam[i] == '/') fileselectnam[i] = '\\';
	OPENFILENAME ofn =
	{
		sizeof(OPENFILENAME),ghwnd,0,spec,0,0,1,fileselectnam,MAX_PATH,0,0,0,mess,
		OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_NOCHANGEDIR,0,0,defext,0,0,0
	};
	if (!GetSaveFileName(&ofn)) return(0); else return(fileselectnam);
}
#endif
//---------------------------------- Ken file select code begins  ----------------------------------
static char curpicpath[MAX_PATH+1] = "", curmodpath[MAX_PATH+1] = "";
#define MAXHIGHLIGHTDEP 16 //Number of directories to remember highlight position
static long curpichighlightrec[MAXHIGHLIGHTDEP] = {0}, curmodhighlightrec[MAXHIGHLIGHTDEP] = {0}; //FIXFIX
static long curpichighlightdep = 0, curmodhighlightdep = 0; //FIXFIX

static char *menuchar = 0; static long menucharmal = 0;              //holds filename chars
static long *menuoffs = 0; static long menuoffsmal = 0, curmenuoffs; //holds offsets into chars
static char basepath[MAX_PATH+1] = "", *menupath, myfileselect_filespec[MAX_PATH+5+1];
static long *menuhighlightrec, *menuhighlightdep;
static long menunamecnt = 0, menuhighlight = 0, myfileselectmode = 0, myfileselect_topplc;
static long menupicsiz = 1, menupicperline = 1, menupicoffx = 0;
static float menupicoffy = 56, menupiczoom = 1.0;

static void getfilenames (char *kind)
{
	long i, getdir;
	char tbuf[MAX_PATH+1];

	if ((kind[0] == '.') && (!kind[1]))
		  { kzfindfilestart("*"); getdir = 1; }
	else { kzfindfilestart(kind); getdir = 0; }
	while (kzfindfile(tbuf))
	{
		i = strlen(tbuf); if (i <= 0) continue;
		if (getdir) { if ((tbuf[i-1] != '\\') && (tbuf[i-1] != '/') && (tbuf[0] != '.')) continue; }
				 else { if ((tbuf[i-1] == '\\') || (tbuf[i-1] == '/') || (tbuf[0] == '.')) continue; }

		if (curmenuoffs+i+1 >= menucharmal)
		{
			menucharmal = max(menucharmal<<1,16384);
			menuchar = (char *)realloc(menuchar,menucharmal*sizeof(menuchar[0]));
			if (!menuchar) MessageBox(ghwnd,"menuchar malloc failed!",prognam,MB_OK);
		}
		if (menunamecnt >= menuoffsmal)
		{
			menuoffsmal = max(menuoffsmal<<1,1024);
			menuoffs = (long *)realloc(menuoffs,menuoffsmal*sizeof(menuoffs[0]));
			if (!menuoffs) MessageBox(ghwnd,"menuoffs malloc failed!",prognam,MB_OK);
		}

		strcpy(&menuchar[curmenuoffs],tbuf);
		menuoffs[menunamecnt] = curmenuoffs;
		curmenuoffs += i+1; menunamecnt++;
	}
}

static void myfileselect_start (void)
{
	static long inited[2] = {0,0};
	long i, j, k, x, y, il, jl;
	char tbuf[MAX_PATH+5+1], *ptr, ch0, ch1;

	SetCurrentDirectory(menupath);
	menunamecnt = 0; curmenuoffs = 0;
	getfilenames(".");

	i = 0;
	do
	{
		for(j=i;(myfileselect_filespec[j]) && (myfileselect_filespec[j] != ';');j++);
		memcpy(tbuf,&myfileselect_filespec[i],j-i); tbuf[j-i] = 0;
		getfilenames(tbuf);
		i = j+1;
	} while (myfileselect_filespec[i]);

	if (!menupath[0]) //Remove ..\\ at base data path
		for(i=0;i<menunamecnt;i++)
			if ((menuchar[menuoffs[i]+0] == '.') && (menuchar[menuoffs[i]+1] == '.') && (menuchar[menuoffs[i]+2] == '\\') && (!menuchar[menuoffs[i]+3]))
				{ menunamecnt--; menuoffs[i] = menuoffs[menunamecnt]; break; }

	for(i=1;i<menunamecnt;i++)
	{
		il = max(strlen(&menuchar[menuoffs[i]])-1,0);
		for(j=0;j<i;j++)
		{
			jl = max(strlen(&menuchar[menuoffs[j]])-1,0);
			if ((menuchar[menuoffs[i]+il] == '\\') && (menuchar[menuoffs[j]+jl] != '\\')) //List directories first
				{ k = menuoffs[i]; menuoffs[i] = menuoffs[j]; menuoffs[j] = k; continue; }
			else if ((menuchar[menuoffs[i]+il] != '\\') && (menuchar[menuoffs[j]+jl] == '\\')) continue;
			for(k=0;1;k++)
			{
				ch0 = menuchar[menuoffs[i]+k]; if ((ch0 >= 'a') && (ch0 <= 'z')) ch0 -= 32;
				ch1 = menuchar[menuoffs[j]+k]; if ((ch1 >= 'a') && (ch1 <= 'z')) ch1 -= 32;
				if ((ch0 != ch1) || (!ch0) || (!ch1)) break;
			}
			if (ch0 < ch1) { k = menuoffs[i]; menuoffs[i] = menuoffs[j]; menuoffs[j] = k; }
		}
	}

	SetCurrentDirectory(basepath); //Restore path for unrelated file operations

	menuhighlight = min(max(menuhighlight,0),menunamecnt-1);

	i = (menupath == curpicpath);
	if (!inited[i])
	{
		inited[i] = 1;
		for(j=0;j<menunamecnt;j++)
		{
			if (!i) { if (menuchar[menuoffs[j]+0] == 'm') { menuhighlight = j; break; } } //Starting dir points to models   (or similar)
				else { if (menuchar[menuoffs[j]+0] == 't') { menuhighlight = j; break; } } //Starting dir points to textures (or similar)
		}
	}
	//if (!myfileselectmode) menuhighlight = menuhighlight;
	myfileselectmode = 1;
}

static void drawsph (cam_t *cc, double x, double y, double z, double r, long col);
static void drawkv6 (cam_t *cc, char *filnam, double px, double py, double pz,
															 double rx, double ry, double rz,
															 double dx, double dy, double dz,
															 double fx, double fy, double fz, long col, double shadefac);
static void myfileselect_draw (cam_t *cc)
{
	cam_t ncc;
	tile_t tt;
	kgln_t vert[4];
	long i, j, k, x0, y0, x1, y1, col, stleng, crc32, hashind;
	char tbuf[MAX_PATH+5+1], och;
	extern void print6x8(tiltyp *, long, long, long, long, const char *, ...);
	extern void drawpol(cam_t *cc, kgln_t *vert, long num, tile_t *tpic, long curcol, float *ouvmat, point3d *norm, long flags);

	print6x8(&cc->c,0,16,0xffffff,0,"%s",menupath);
	myfileselect_topplc = max(min(menuhighlight-(((cc->c.y-7-24)>>3)>>1),menunamecnt-((cc->c.y-7-24)>>3)-1),0);

	menupicsiz = max((int)(sqrt((double)xres)*3.0),1)*menupiczoom;
	menupicperline = max(xres/menupicsiz,1);
	menupicoffx = ((xres-menupicsiz*menupicperline)>>1);
	y0 = (menuhighlight/menupicperline)*(menupicsiz+10)+((long)menupicoffy); y1 = y0+menupicsiz;
	if (y0 <       56) menupicoffy += (       56-y0)*min((dtotclk-odtotclk)*16,1);
	if (y1 >= yres-16) menupicoffy -= (y1-(yres-16))*min((dtotclk-odtotclk)*16,1);

	for(i=0,j=cc->z.f;i<cc->c.y;i++,j+=cc->z.p) memset8((void *)j,0x7f7f7f7f,cc->c.x<<2); //Clear z-buffer for rendering models :/

	for(i=0;i<menunamecnt;i++)
	{
		y0 = (i/menupicperline)*(menupicsiz+10)+((long)menupicoffy); y1 = y0+menupicsiz; if ((y1 < 0) || (y0 >= yres)) continue;
		x0 = (i%menupicperline)*(menupicsiz   )+       menupicoffx ; x1 = x0+menupicsiz;

		stleng = strlen(&menuchar[menuoffs[i]]); if (stleng <= 0) continue;

		col = 128; if (i == menuhighlight) col += sin(dtotclk*12.0)*8.0+8;
		col *= 0x10101;

		if (menuchar[menuoffs[i]+stleng-1] == '\\')
		{
			if ((stleng == 3) && (menuchar[menuoffs[i]+0] == '.') && (menuchar[menuoffs[i]+1] == '.'))
				  tt.tt.f = (long)upfolder;
			else tt.tt.f = (long)folder;
			tt.tt.x = 64; tt.tt.y = 64; tt.tt.p = (tt.tt.x<<2); tt.tt.lowermip = 0;
		}
		else
		{
			sprintf(tbuf,menupath); strcat(tbuf,&menuchar[menuoffs[i]]);

			if ((stleng >= 4) && (!stricmp(&menuchar[menuoffs[i]+stleng-4],".kv6")))
			{
				dpoint3d dp, dr, dd, df;
				kv6data_t *kv6;
				double d, c, s;

				dp.x = 0.0; dp.y = 0.0; dp.z = 0.0;
				dr.x = 1.0; dr.y = 0.0; dr.z = 0.0;
				dd.x = 0.0; dd.y = 1.0; dd.z = 0.0;
				df.x = 0.0; df.y = 0.0; df.z = 1.0;
				kv6 = drawkv6_get(tbuf);
				if (!kv6)
				{
					drawcone_setup(cputype,shadowtest2_numcpu,(tiletype *)&cc->c,cc->z.f-cc->c.f,&dp,&dr,&dd,&df,(x0+x1)*.5,(y0+y1)*.5,cc->h.z);
					drawsph(0.0,0.0,cc->h.z,menupicsiz*.4,col,48.0);
				}
				else
				{
					drawkv6_setup(&drawkv6_frame,(tiletype *)&cc->c,cc->z.f-cc->c.f,&dp,&dr,&dd,&df,(x0+x1)*.5,(y0+y1)*.5,cc->h.z);
					d = menupicsiz*.4*2.0/((double)max(max(kv6->xsiz,kv6->ysiz),kv6->zsiz));
					c = cos(dtotclk); s = sin(dtotclk);
					j = drawkv6_numlights; drawkv6_numlights = -1;
					drawkv6(&drawkv6_frame,kv6, 0,0,cc->h.z, c*d,0,s*d, 0,d,0,  s*d,0,-c*d, col, 48.0);
					drawkv6_numlights = j;
				}
				goto menuskipdrawpol;
			}

			j = gettileind(tbuf);
		}

		vert[0].x = vert[3].x = x0; vert[0].y = vert[1].y = y0;
		vert[1].x = vert[2].x = x1; vert[2].y = vert[3].y = y1;
		for(k=4-1;k>=0;k--)
		{
			vert[k].x -= cc->h.x; vert[k].y -= cc->h.y; vert[k].z = cc->h.z;
			vert[k].u = ((k+1)>>1); vert[k].v = (k>>1)+1e-4; vert[k].n = 1;
		}
		vert[3].n = -3;

		if (menuchar[menuoffs[i]+stleng-1] == '\\')
		{
			drawpol(cc,vert,4,&tt,col,0,0,RENDFLAGS_NODEPTHTEST|RENDFLAGS_ALPHAMASK);

			strcpy(tbuf,&menuchar[menuoffs[i]]); stleng--; tbuf[stleng] = 0;
			k = max((menupicsiz-24)/6,0); //Max chars to print
			j = stleng; if (k < j) { j = k+2; strcpy(&tbuf[k],".."); }
			print6x8(&cc->c,((x0+x1)>>1)-j*3+1,((y0+y1)>>1)-4,0x000000,-1,"%s",tbuf);
		}
		else
		{
			drawpol(cc,vert,4,&gtile[j],col,0,0,RENDFLAGS_NODEPTHTEST);
menuskipdrawpol:;
			if (i == menuhighlight) col = 0xffffff; else col = 0xc0c0c0;

			strcpy(tbuf,&menuchar[menuoffs[i]]);
			k = max((menupicsiz-18)/6,0); //k = max chars to print
			j = stleng; if (k < j) { j = k+2; strcpy(&tbuf[k],".."); }
			print6x8(&cc->c,((x0+x1)>>1)-j*3,y0-8,col,0x000000,"%s",tbuf);
		}
	}

		//Draw square around highlighted tile
	x0 = (menuhighlight%menupicperline)*(menupicsiz   )+       menupicoffx ; x1 = x0+menupicsiz;
	y0 = (menuhighlight/menupicperline)*(menupicsiz+10)+((long)menupicoffy); y1 = y0+menupicsiz;
	drawhlin(&cc->c,x0,x1,y0  ,0xffffff);
	drawhlin(&cc->c,x0,x1,y0+1,0xffffff);
	for(k=y0+2;k<y1-2;k++)
	{
		drawhlin(&cc->c,x0  ,x0+2,k,0xffffff);
		drawhlin(&cc->c,x1-2,x1  ,k,0xffffff);
	}
	drawhlin(&cc->c,x0,x1,y1-2,0xffffff);
	drawhlin(&cc->c,x0,x1,y1-1,0xffffff);
}

static void myfileselect_input (playerstruct_t *lps, long key)
{
	char tbuf[256+4];
	long i, j, k;

	switch((key>>8)&255)
	{
		case 0xc8: if (menuhighlight >= menupicperline) menuhighlight -= menupicperline; break; //Up
		case 0xd0: menuhighlight = min(menuhighlight+menupicperline,menunamecnt-1); break;      //Down
		case 0xcb: if (menuhighlight) menuhighlight--; break;                                   //Left
		case 0xcd: if (menuhighlight < menunamecnt-1) menuhighlight++; break;                   //Right
		case 0xc9: for(i=(yres-64)/menupicsiz;i>=0;i--) if (menuhighlight >= menupicperline) menuhighlight -= menupicperline; break; //PGUP
		case 0xd1: for(i=(yres-64)/menupicsiz;i>=0;i--) menuhighlight = min(menuhighlight+menupicperline,menunamecnt-1);      break; //PGDN
		case 0xc7: if (!(key&0xc0000)) menuhighlight -= (menuhighlight%menupicperline); else menuhighlight = 0; break; //Home / Ctrl+Home
		case 0xcf: if (!(key&0xc0000)) menuhighlight = min((menuhighlight/menupicperline)*menupicperline+menupicperline-1,menunamecnt-1); else menuhighlight = menunamecnt-1; break; //End /Ctrl+End
		case 0x37: case 0xb5: //KP*,KP/
			if (((key>>8)&255) == 0x37) menupiczoom = min(menupiczoom*2.0,4.0/1.0); //KP*
										  else menupiczoom = max(menupiczoom*0.5,1.0/4.0); //KP/
			menupicsiz = max((int)(sqrt((double)xres)*3.0),1)*menupiczoom;
			menupicperline = max(xres/menupicsiz,1);
			menupicoffx = ((xres-menupicsiz*menupicperline)>>1);
			menupicoffy = min(((56+yres-16) - menupicsiz)/2 - (menuhighlight/menupicperline)*(menupicsiz+10),56);
			break;

		case 0x0e: //Backspace:go back dir
			for(i=max(strlen(menupath)-1,0);(i > 0) && (menupath[i-1] != '\\') && (menupath[i-1] != '/');i--);
			menupath[i] = 0;
			myfileselect_start();
			if ((*menuhighlightdep) > 0)
			{
				(*menuhighlightdep)--;
				menuhighlight = menuhighlightrec[*menuhighlightdep];
			}
			break;

		case 0x1: //Esc:exit
			tbuf[0] = j = 5;
			tbuf[1] = (unsigned char)moveindex;
			tbuf[2] = 128; //bit fields
			tbuf[3] = 0; //tics
			tbuf[4] = 4; //Command 4: Cancel send filename
				//Copy to local command buffer (can't send in the middle of executepack)
			memcpy(&cmdpackfif[cmdpackfifw&65535],tbuf,j); cmdpackfifw += j;
			myfileselectmode = 0;
			return;

		case 0x1c: case 0x9c: case 0x0f: //Enter:change v'd texture&exit, Tab:change tab grab&exit
			k = strlen(&menuchar[menuoffs[menuhighlight]]);
			if ((k > 0) && (menuchar[menuoffs[menuhighlight]+k-1] == '\\'))
			{
				if ((k == 3) && (menuchar[menuoffs[menuhighlight]+0] == '.') && (menuchar[menuoffs[menuhighlight]+1] == '.'))
				{
					for(i=max(strlen(menupath)-1,0);(i > 0) && (menupath[i-1] != '\\') && (menupath[i-1] != '/');i--);
					menupath[i] = 0;
					myfileselect_start();
					if ((*menuhighlightdep) > 0)
					{
						(*menuhighlightdep)--;
						menuhighlight = menuhighlightrec[*menuhighlightdep];
					}
					break;
				}
				i = strlen(&menuchar[menuoffs[menuhighlight]]);
				if ((menuchar[menuoffs[menuhighlight]+i-1] == '\\') || (menuchar[menuoffs[menuhighlight]+i-1] == '/'))
				{
					strcat(menupath,&menuchar[menuoffs[menuhighlight]]);
					if ((*menuhighlightdep) < MAXHIGHLIGHTDEP)
					{
						menuhighlightrec[*menuhighlightdep] = menuhighlight;
						(*menuhighlightdep)++;
					}
					myfileselect_start();
					menuhighlight = 0;
				}
				break;
			}

			menuhighlight = menuhighlight;
			if (myfileselectmode)
			{
				char *cptr;
				cptr = &menuchar[menuoffs[menuhighlight]];
				tbuf[1] = (unsigned char)moveindex;
				tbuf[2] = 128; //bit fields
				tbuf[3] = 0; //tics
				tbuf[4] = 3; //Command 3: Send filename
				if (((key>>8)&255) == 0x0f) //Tab changes tab grab instead of v'd surface
					  { *(long *)&tbuf[5] = -2; }
				else { *(long *)&tbuf[5] = lps->grabsect; }
				*(long *)&tbuf[9] = lps->grabwall;
				j = 13;
					//2 examples:
					//    in exe dir: cptr:|cobblestone.png|, basepath:|c:\kwin\build2|, menupath:||
					//in \hei subdir: cptr:|cobblestone.png|, basepath:|c:\kwin\build2|, menupath:|hei\|
				for(k=0;(j < sizeof(gtile[0].filnam)) && (menupath[k]);j++,k++) tbuf[j] = menupath[k];
				for(k=0;(j < sizeof(gtile[0].filnam)) && (    cptr[k]);j++,k++) tbuf[j] = cptr[k];
				tbuf[j++] = 0;
				tbuf[0] = j;

					//Copy to local command buffer (can't send in the middle of executepack)
				memcpy(&cmdpackfif[cmdpackfifw&65535],tbuf,j); cmdpackfifw += j;

				if (((key>>8)&255) == 0x0f)
					myplaysound("sounds\\drop1.wav",100,0.25,&gps->ipos,KSND_3D);
				else if ((lps->grabwall&0xc0000000) == 0x40000000)
					myplaysound("sounds\\hammer.wav",100,0.25,&gst->spri[lps->grabwall&0x3fffffff].p,KSND_3D);
				else
				{
					point3d hit;
					getcentroid(gst->sect[lps->grabsect].wall,gst->sect[lps->grabsect].n,&hit.x,&hit.y);
					hit.z = getslopez(&gst->sect[lps->grabsect],lps->grabcf&1,hit.x,hit.y);
					myplaysound("sounds\\hammer.wav",100,0.25,&hit,KSND_3D);
				}

				myfileselectmode = 0;
			}
			break;
	}
}
#endif
//----------------------------------- .CFG load/save code begins -----------------------------------
typedef struct
{
	long xres, yres, fullscreen;
	char nick[64];
	double anginc;
	long drawthreads, usemorph;
	long bilin, lights, shadows;
} build2_t;
static build2_t b2opts, ob2opts;
static char b2cfgfilnam[MAX_PATH] = "build2.cfg";

static char *strcpy_safe (char *dst, int dstleng, char *src)
{
	int i; dstleng--;
	for(i=0;(src[i]) && (i < dstleng);i++) dst[i] = src[i];
	dst[i] = 0; return(dst);
}

#if (STANDALONE)
static int keyremap_getnums (char *tbuf, int *ro, int *rn)
{
	int j, o, n;

	for(o=sizeof(keynames)/sizeof(keynames[0])-1;o>=0;o--)
	{
		j = strlen(keynames[o].nam);
		if ((!memicmp(keynames[o].nam,tbuf,j)) && (tbuf[j] == ']')) { tbuf += j; o = keynames[o].scancode; break; }
	}
	if (o < 0) o = strtol(tbuf,&tbuf,0);

	if ((tbuf[0] != ']') || (tbuf[1] != '=')) return(0);
	tbuf += 2;

	for(n=sizeof(keynames)/sizeof(keynames[0])-1;n>=0;n--)
	{
		j = strlen(keynames[n].nam);
		if (!memicmp(keynames[n].nam,tbuf,j) && (tbuf[j] == 0)) { n = keynames[n].scancode; break; }
	}
	if (n < 0) n = strtol(tbuf,0,0);
	(*ro) = o; (*rn) = n;
	return(1);
}
#endif

static void loadcfg (void)
{
	RECT rw;
	long i, findcharcnt = 0;
	char och, ch, tbuf[1024], b2id[] = "[BUILD2]\r\n";

	SystemParametersInfo(SPI_GETWORKAREA,0,&rw,0);
	b2opts.xres = rw.right -rw.left;
	b2opts.yres = rw.bottom-rw.top - GetSystemMetrics(SM_CYCAPTION);
	b2opts.fullscreen = 0;
	memset(b2opts.nick,0,sizeof(b2opts.nick));
	b2opts.anginc = 2.0;
	b2opts.drawthreads = 0;
	b2opts.usemorph = 0;
	b2opts.bilin = 0;
	b2opts.lights = 1;
	b2opts.shadows = 1;

#if (STANDALONE)
	for(i=256-1;i>=0;i--) { keyscanremap[i] = i; }
	memcpy(keyscanremapr,keyscanremap,sizeof(keyscanremapr));
	memcpy(okeyscanremap,keyscanremap,sizeof(okeyscanremap));
#endif

	if (!kzopen(b2cfgfilnam)) return;
	while (1)
	{
		if (kzeof()) { kzclose(); return; }
		ch = kzgetc();
		if (ch != b2id[findcharcnt]) { findcharcnt = 0; continue; }
		findcharcnt++; if (!b2id[findcharcnt]) break;
	}
	i = 0;
	while (!kzeof())
	{
		och = ch; ch = kzgetc();
		if ((i < sizeof(tbuf)-1) && (ch >= 32)) tbuf[i++] = ch;
		if ((ch == '\r') || (kzeof()))
		{
			if ((och == '\\') && (i > 0)) { tbuf[i-1] = '\r'; continue; }
			tbuf[i] = 0;
			if (!memcmp(tbuf,"xres="       , 5)) b2opts.xres        = min(max(strtol(&tbuf[ 5],0,0),80),MAXXDIM);
			if (!memcmp(tbuf,"yres="       , 5)) b2opts.yres        = min(max(strtol(&tbuf[ 5],0,0),50),MAXYDIM);
			if (!memcmp(tbuf,"fullscreen=" ,11)) b2opts.fullscreen  = min(max(strtol(&tbuf[11],0,0),0),1);
			if (!memcmp(tbuf,"nick="       , 5)) strcpy_safe(b2opts.nick,sizeof(b2opts.nick),&tbuf[5]);
			if (!memcmp(tbuf,"anginc="     , 7)) b2opts.anginc      = min(max(strtod(&tbuf[ 7],0),1.0),64.0);
			if (!memcmp(tbuf,"drawthreads=",12)) b2opts.drawthreads = min(max(strtol(&tbuf[12],0,0),1),64);
			if (!memcmp(tbuf,"usemorph="   , 9)) b2opts.usemorph    = min(max(strtol(&tbuf[ 9],0,0),0),1);
			if (!memcmp(tbuf,"bilin="      , 6)) b2opts.bilin       = min(max(strtol(&tbuf[ 6],0,0),0),1);
			if (!memcmp(tbuf,"lights="     , 7)) b2opts.lights      = min(max(strtol(&tbuf[ 7],0,0),0),1);
			if (!memcmp(tbuf,"shadows="    , 8)) b2opts.shadows     = min(max(strtol(&tbuf[ 8],0,0),0),1);
#if (STANDALONE)
			if (!memcmp(tbuf,"keyremap["   , 9))
			{
				int o, n;
				if (keyremap_getnums(&tbuf[9],&o,&n))
					if ((unsigned)o < sizeof(keyscanremap)/sizeof(keyscanremap[0]))
						{ keyscanremap[o] = n; keyscanremapr[n] = o; okeyscanremap[o] = n; }
			}
#endif
			i = 0;
		}
	}

	if (!b2opts.fullscreen)
	{
		b2opts.xres = min(b2opts.xres,rw.right -rw.left);
		b2opts.yres = min(b2opts.yres,rw.bottom-rw.top - GetSystemMetrics(SM_CYCAPTION));
	}

	kzclose();
}

static void savecfg (void)
{
	FILE *fil, *tfil;
	long i, j, k, findcharcnt = 0, inb2section = 0;
	char och, och2, ch, b2id[] = "[BUILD2]\r\n";

	//char *tfilnam; tfilnam = tmpnam(0); if (!tfilnam) return; //tmpnam always fails in Vista
	char tfilnam[64+1];
	for(j=0;1;j++)
	{
		for(i=0;i<64;i++) tfilnam[i] = (rand()%26)+'A'; tfilnam[64] = 0;
		fil = fopen(tfilnam,"rb"); if (!fil) break;
		fclose(fil);
		if (j > 4) return; //give up: don't save ini. tough luck.
	}

	tfil = fopen(tfilnam,"wb"); if (!tfil) return;
	fil = fopen(b2cfgfilnam,"rb");
	if (fil)
	{
		och = '\n'; och2 = '\n';
		while (1)
		{
			ch = fgetc(fil);
			if (feof(fil))
			{
				if (och != '\n') //Insert \r\n at eof if there isn't one already
					if (fwrite("\r\n",1,2,tfil) != 2) { fclose(fil); fclose(tfil); remove(tfilnam); return; }
				break;
			}
			if (!inb2section)
			{
				och = ch;
				if (fputc(ch,tfil) == EOF) { fclose(fil); fclose(tfil); remove(tfilnam); return; }
				if (ch != b2id[findcharcnt]) { findcharcnt = 0; continue; }
				findcharcnt++;
				if (!b2id[findcharcnt]) { inb2section = 1; fseek(tfil,-findcharcnt,SEEK_CUR); }
			}
			else
			{
				if ((och2 == '\n') && (ch == '[')) { findcharcnt = 0; inb2section = 0; }
				och2 = ch;
			}
		}
		fclose(fil);
	}

	if (fprintf(tfil,"[BUILD2]\r\n") < 0) { fclose(tfil); remove(tfilnam); return; }

	if (fprintf(tfil,"xres=%d\r\n"       ,b2opts.xres)        < 0) { fclose(tfil); remove(tfilnam); return; }
	if (fprintf(tfil,"yres=%d\r\n"       ,b2opts.yres)        < 0) { fclose(tfil); remove(tfilnam); return; }
	if (fprintf(tfil,"fullscreen=%d\r\n" ,b2opts.fullscreen)  < 0) { fclose(tfil); remove(tfilnam); return; }
	if (fprintf(tfil,"nick=%s\r\n"       ,b2opts.nick)        < 0) { fclose(tfil); remove(tfilnam); return; }
	if (fprintf(tfil,"anginc=%g\r\n"     ,b2opts.anginc)      < 0) { fclose(tfil); remove(tfilnam); return; }
	if (fprintf(tfil,"drawthreads=%d\r\n",b2opts.drawthreads) < 0) { fclose(tfil); remove(tfilnam); return; }
	if (fprintf(tfil,"usemorph=%d\r\n"   ,b2opts.usemorph)    < 0) { fclose(tfil); remove(tfilnam); return; }
	if (fprintf(tfil,"bilin=%d\r\n"      ,b2opts.bilin)       < 0) { fclose(tfil); remove(tfilnam); return; }
	if (fprintf(tfil,"lights=%d\r\n"     ,b2opts.lights)      < 0) { fclose(tfil); remove(tfilnam); return; }
	if (fprintf(tfil,"shadows=%d\r\n"    ,b2opts.shadows)     < 0) { fclose(tfil); remove(tfilnam); return; }

#if (STANDALONE)
	for(i=0;i<sizeof(keyscanremap)/sizeof(keyscanremap[0]);i++)
		if (i != keyscanremap[i])
		{
				//Use names if both found in list
			for(j=0;j<sizeof(keynames)/sizeof(keynames[0]);j++) //Keep loop ascending to pick best name
				if (keynames[j].scancode == i) break;
			for(k=0;k<sizeof(keynames)/sizeof(keynames[0]);k++) //Keep loop ascending to pick best name
				if (keynames[k].scancode == keyscanremap[i]) break;
			if (max(j,k) < sizeof(keynames)/sizeof(keynames[0]))
				  { if (fprintf(tfil,"keyremap[%s]=%s\r\n",keynames[j].nam,keynames[k].nam) < 0) { fclose(tfil); remove(tfilnam); return; } }
			else { if (fprintf(tfil,"keyremap[0x%02x]=0x%02x\r\n",i,keyscanremap[i]      ) < 0) { fclose(tfil); remove(tfilnam); return; } }
		}
#endif

	if (fclose(tfil)) { remove(tfilnam); return; }

	remove(b2cfgfilnam); rename(tfilnam,b2cfgfilnam);
}
//------------------------------------ .CFG load/save code ends ------------------------------------
//------------------------ Simple PNG OUT code begins ------------------------
static FILE *pngofil;
static long pngoxplc, pngoyplc, pngoxsiz, pngoysiz;
static unsigned long pngocrc, pngoadcrc;

static inline unsigned long bswap (unsigned long a)
	{ return(((a>>8)&0xff00)+((a&0xff00)<<8)+(a<<24)+(a>>24)); }

#if (STANDALONE == 0)
static long crctab32[256] = {0};  //SEE CRC32.C
#endif

#define updatecrc32(c,crc) crc=(crctab32[((c)^crc)&255]^(((unsigned)crc)>>8))
#define updateadl32(c,crc) \
{  c += (crc&0xffff); if (c   >= 65521) c   -= 65521; \
	crc = (crc>>16)+c; if (crc >= 65521) crc -= 65521; \
	crc = (crc<<16)+c; \
} \

static void fputbytes (unsigned long v, long n)
	{ for(;n;v>>=8,n--) { fputc(v,pngofil); updatecrc32(v,pngocrc); } }

static void pngoutopenfile (char *fnam, long xsiz, long ysiz)
{
	long i, j, k;
	char a[40];

	pngoxsiz = xsiz; pngoysiz = ysiz; pngoxplc = pngoyplc = 0;
	for(i=255;i>=0;i--)
	{
		k = i; for(j=8;j;j--) k = ((unsigned long)k>>1)^((-(k&1))&0xedb88320);
		crctab32[i] = k;
	}
	pngofil = fopen(fnam,"wb");
	*(long *)&a[0] = 0x474e5089; *(long *)&a[4] = 0x0a1a0a0d;
	*(long *)&a[8] = 0x0d000000; *(long *)&a[12] = 0x52444849;
	*(long *)&a[16] = bswap(xsiz); *(long *)&a[20] = bswap(ysiz);
	*(long *)&a[24] = 0x00000208; *(long *)&a[28] = 0;
	for(i=12,j=-1;i<29;i++) updatecrc32(a[i],j);
	*(long *)&a[29] = bswap(j^-1);
	fwrite(a,37,1,pngofil);
	pngocrc = -1; pngoadcrc = 1;
	fputbytes(0x54414449,4); fputbytes(0x0178,2);
}

static void pngoutputpixel (long rgbcol)
{
	long a[4];

	if (!pngoxplc)
	{
		fputbytes(pngoyplc==pngoysiz-1,1);
		fputbytes(((pngoxsiz*3+1)*0x10001)^0xffff0000,4);
		fputbytes(0,1); a[0] = 0; updateadl32(a[0],pngoadcrc);
	}
	fputbytes(bswap(rgbcol<<8),3);
	a[0] = (rgbcol>>16)&255; updateadl32(a[0],pngoadcrc);
	a[0] = (rgbcol>> 8)&255; updateadl32(a[0],pngoadcrc);
	a[0] = (rgbcol    )&255; updateadl32(a[0],pngoadcrc);
	pngoxplc++; if (pngoxplc < pngoxsiz) return;
	pngoxplc = 0; pngoyplc++; if (pngoyplc < pngoysiz) return;
	fputbytes(bswap(pngoadcrc),4);
	a[0] = bswap(pngocrc^-1); a[1] = 0; a[2] = 0x444e4549; a[3] = 0x826042ae;
	fwrite(a,1,16,pngofil);
	a[0] = bswap(ftell(pngofil)-(33+8)-16);
	fseek(pngofil,33,SEEK_SET); fwrite(a,1,4,pngofil);
	fclose(pngofil);
}
//------------------------- Simple PNG OUT code ends -------------------------

static long pngcount = 0;
void screencapture (tiletype *tt)
{
	FILE *fil = 0;
	long x, y, *lptr;
	char filnam[MAX_PATH];

	do
	{
		if (fil) fclose(fil);
		sprintf(filnam,"BLD2%04d.png",pngcount++);
		fil = fopen(filnam,"rb");
	} while (fil);

	pngoutopenfile(filnam,tt->x,tt->y);
	for(y=0,lptr=(long *)tt->f;y<tt->y;y++,lptr=(long *)(((long)lptr)+tt->p))
		for(x=0;x<tt->x;x++) pngoutputpixel(lptr[x]);
#ifdef STANDALONE
	messagetimeout = dtotclk+3.0;
	sprintf(message,"Screen captured to %s",filnam);
#endif
}

void surroundcapture (cam_t *cam, gamestate_t *dast, playerstruct_t *lps, int n)
{
	FILE *fil = 0;
	cam_t ncam;
	int i, j, *colbuf, *depbuf, *lptr;
	char filnam[MAX_PATH];

	do
	{
		if (fil) fclose(fil);
		sprintf(filnam,"BLD2%04d.png",pngcount++);
		fil = fopen(filnam,"rb");
	} while (fil);

	colbuf = (int *)malloc(n*n*4);
	depbuf = (int *)malloc(n*n*4);

	pngoutopenfile(filnam,n,n*6);
	for(i=0;i<6;i++)
	{
		ncam = (*cam);

		switch(i)
		{
			case 0: ncam.r.x = 1; ncam.r.y = 0; ncam.r.z = 0;
					  ncam.d.x = 0; ncam.d.y = 0; ncam.d.z = 1;
					  ncam.f.x = 0; ncam.f.y =-1; ncam.f.z = 0; break;

			case 1: ncam.r.x = 0; ncam.r.y = 1; ncam.r.z = 0;
					  ncam.d.x = 0; ncam.d.y = 0; ncam.d.z = 1;
					  ncam.f.x = 1; ncam.f.y = 0; ncam.f.z = 0; break;

			case 2: ncam.r.x =-1; ncam.r.y = 0; ncam.r.z = 0;
					  ncam.d.x = 0; ncam.d.y = 0; ncam.d.z = 1;
					  ncam.f.x = 0; ncam.f.y = 1; ncam.f.z = 0; break;

			case 3: ncam.r.x = 0; ncam.r.y =-1; ncam.r.z = 0;
					  ncam.d.x = 0; ncam.d.y = 0; ncam.d.z = 1;
					  ncam.f.x =-1; ncam.f.y = 0; ncam.f.z = 0; break;

			case 4: ncam.r.x = 1; ncam.r.y = 0; ncam.r.z = 0;
					  ncam.d.x = 0; ncam.d.y =-1; ncam.d.z = 0;
					  ncam.f.x = 0; ncam.f.y = 0; ncam.f.z =-1; break;

			case 5: ncam.r.x = 1; ncam.r.y = 0; ncam.r.z = 0;
					  ncam.d.x = 0; ncam.d.y = 1; ncam.d.z = 0;
					  ncam.f.x = 0; ncam.f.y = 0; ncam.f.z = 1; break;
		}

		ncam.h.x = n/2; ncam.h.y = n/2; ncam.h.z = n/2;
		ncam.c.f = (long)colbuf; ncam.c.p = n*4; ncam.c.x = n; ncam.c.y = n;
		ncam.z.f = (long)depbuf; ncam.z.p = n*4; ncam.z.x = n; ncam.z.y = n;
		shadowtest2_rendmode = 2; draw_hsr_polymost(&ncam,dast,lps,lps->cursect); shadowtest2_rendmode = 4;
		htrun(drawpollig,0,eyepoln,shadowtest2_numcpu);

		for(j=0;j<n*n;j++) pngoutputpixel(colbuf[j]);
	}

	free(depbuf); free(colbuf);
#ifdef STANDALONE
	messagetimeout = dtotclk+3.0; sprintf(message,"Skybox Captured to %s",filnam);
#endif
}

#ifdef STANDALONE
static void ksrand (long val) { gst->rseed = val*2; }
#ifndef _MSC_VER
static long krand (void) { gst->rseed = (gst->rseed*214013)+2531011*2; return(((unsigned long)gst->rseed)>>1); }
#else
__declspec(naked) static long krand (void)
{
	_asm
	{
		mov edx, gst
		mov eax, gamestate_t.rseed[edx]
		imul eax, 214013
		add eax, 2531011*2
		mov gamestate_t.rseed[edx], eax
		shr eax, 1
		ret
	}
}
#endif

	//NOTE: this code requires SAPI.H & SAPI.LIB (not included by default with VC6)
#include "sapi.h"
#pragma comment(lib,"ole32")
static ISpVoice *tts = 0;
static void uninittts (void) { if ((tts) && (tts != (ISpVoice *)-1)) { tts->Release(); tts = 0; CoUninitialize(); } }
static void playtext (char *st)
{
	long flags;
	WCHAR *wptr;
	if ((!st) || (tts == (ISpVoice *)-1)) return;
	if (!tts)
	{
		CoInitialize(0);
		CoCreateInstance(CLSID_SpVoice,0,CLSCTX_ALL,IID_ISpVoice,(void **)&tts);
		if (tts == 0) { tts = (ISpVoice *)-1; CoUninitialize(); return; }
		atexit(uninittts);
	}
	if (st[0] == ',') { flags = 0; st++; } else flags = SPF_PURGEBEFORESPEAK;
	wptr = (WCHAR *)_alloca((strlen(st)+1)*sizeof(WCHAR)); if (!wptr) return;
	MultiByteToWideChar(CP_ACP,0,st,-1,wptr,strlen(st)+1);
	tts->Speak(wptr,SPF_ASYNC|SPF_IS_XML|flags,0);
}
#endif

static void xformpos (float *x, float *y, float *z)
{
	float t, ox, oy, oz;

	ox = (*x); oy = (*y); oz = (*z);
	t = ((ox-gdps->grdc.x)*gdps->grdn.x + (oy-gdps->grdc.y)*gdps->grdn.y + (oz-gdps->grdc.z)*gdps->grdn.z)*gdps->compact2d;
	ox -= gdps->grdn.x*t; oy -= gdps->grdn.y*t; oz -= gdps->grdn.z*t;
	ox -= gdps->ipos.x;   oy -= gdps->ipos.y;   oz -= gdps->ipos.z;
	(*x) = ox*gdps->irig.x + oy*gdps->irig.y + oz*gdps->irig.z;
	(*y) = ox*gdps->idow.x + oy*gdps->idow.y + oz*gdps->idow.z;
	(*z) = ox*gdps->ifor.x + oy*gdps->ifor.y + oz*gdps->ifor.z;
}

static void xformpos (double *x, double *y, double *z)
{
	double t, ox, oy, oz;

	ox = (*x); oy = (*y); oz = (*z);
	t = ((ox-gdps->grdc.x)*gdps->grdn.x + (oy-gdps->grdc.y)*gdps->grdn.y + (oz-gdps->grdc.z)*gdps->grdn.z)*gdps->compact2d;
	ox -= gdps->grdn.x*t; oy -= gdps->grdn.y*t; oz -= gdps->grdn.z*t;
	ox -= gdps->ipos.x;   oy -= gdps->ipos.y;   oz -= gdps->ipos.z;
	(*x) = ox*gdps->irig.x + oy*gdps->irig.y + oz*gdps->irig.z;
	(*y) = ox*gdps->idow.x + oy*gdps->idow.y + oz*gdps->idow.z;
	(*z) = ox*gdps->ifor.x + oy*gdps->ifor.y + oz*gdps->ifor.z;
}

static void xformrot (float *x, float *y, float *z)
{
	float t, ox, oy, oz;

	ox = (*x); oy = (*y); oz = (*z);
	t = (ox*gdps->grdn.x + oy*gdps->grdn.y + oz*gdps->grdn.z)*gdps->compact2d;
	ox -= gdps->grdn.x*t; oy -= gdps->grdn.y*t; oz -= gdps->grdn.z*t;
	(*x) = ox*gdps->irig.x + oy*gdps->irig.y + oz*gdps->irig.z;
	(*y) = ox*gdps->idow.x + oy*gdps->idow.y + oz*gdps->idow.z;
	(*z) = ox*gdps->ifor.x + oy*gdps->ifor.y + oz*gdps->ifor.z;
}

static void xformrot (double *x, double *y, double *z)
{
	double t, ox, oy, oz;

	ox = (*x); oy = (*y); oz = (*z);
	t = (ox*gdps->grdn.x + oy*gdps->grdn.y + oz*gdps->grdn.z)*gdps->compact2d;
	ox -= gdps->grdn.x*t; oy -= gdps->grdn.y*t; oz -= gdps->grdn.z*t;
	(*x) = ox*gdps->irig.x + oy*gdps->irig.y + oz*gdps->irig.z;
	(*y) = ox*gdps->idow.x + oy*gdps->idow.y + oz*gdps->idow.z;
	(*z) = ox*gdps->ifor.x + oy*gdps->ifor.y + oz*gdps->ifor.z;
}
#ifdef STANDALONE
static void curs2grid (playerstruct_t *pps, double sx, double sy, point3d *fp)
{
	double t, vx, vy, vz;

	vx = pps->irig.x*(sx-pps->ghx) + pps->idow.x*(sy-pps->ghy) + pps->ifor.x*pps->ghz;
	vy = pps->irig.y*(sx-pps->ghx) + pps->idow.y*(sy-pps->ghy) + pps->ifor.y*pps->ghz;
	vz = pps->irig.z*(sx-pps->ghx) + pps->idow.z*(sy-pps->ghy) + pps->ifor.z*pps->ghz;

		//(vx*t + pps->ipos.x-pps->grdc.x)*pps->grdn.x +
		//(vy*t + pps->ipos.y-pps->grdc.y)*pps->grdn.y +
		//(vz*t + pps->ipos.z-pps->grdc.z)*pps->grdn.z = 0
	t = vx*pps->grdn.x + vy*pps->grdn.y + vz*pps->grdn.z;
	if (t != 0)
	{
		t = ((pps->grdc.x-pps->ipos.x)*pps->grdn.x +
			  (pps->grdc.y-pps->ipos.y)*pps->grdn.y +
			  (pps->grdc.z-pps->ipos.z)*pps->grdn.z) / t;
	}

	fp->x = vx*t + pps->ipos.x;
	fp->y = vy*t + pps->ipos.y;
	fp->z = vz*t + pps->ipos.z;
}
#endif

int getverts (int s, int w, vertlist_t *ver, int maxverts)
{
	sect_t *sec;
	float x, y;
	int i, ir, iw, ns, nw;

	if ((maxverts <= 0) || ((unsigned)s >= (unsigned)gst->numsects)) return(0);
	if ((unsigned)w >= (unsigned)gst->sect[s].n) return(0);

	ver[0].s = s; ver[0].w = w; if (maxverts == 1) return(1);
	sec = gst->sect;
	x = sec[s].wall[w].x;
	y = sec[s].wall[w].y;
	ir = 0; iw = 1;
	do
	{
			//CCW next sect
		ns = sec[s].wall[w].ns;
		if (ns >= 0)
		{
			nw = sec[s].wall[w].nw;
			if ((sec[ns].wall[nw].x != x) || (sec[ns].wall[nw].y != y)) nw += sec[ns].wall[nw].n;
			for(i=iw-1;i>=0;i--)
				if ((ver[i].s == ns) && (ver[i].w == nw)) break;
			if ((i < 0) && (sec[ns].wall[nw].x == x) && (sec[ns].wall[nw].y == y))
				{ ver[iw].s = ns; ver[iw].w = nw; iw++; if (iw >= maxverts) break; }
		}

			//CW next sect
		w = wallprev(&sec[s],w);
		ns = sec[s].wall[w].ns;
		if (ns >= 0)
		{
			nw = sec[s].wall[w].nw;
			if ((sec[ns].wall[nw].x != x) || (sec[ns].wall[nw].y != y)) nw += sec[ns].wall[nw].n;
			for(i=iw-1;i>=0;i--)
				if ((ver[i].s == ns) && (ver[i].w == nw)) break;
			if ((i < 0) && (sec[ns].wall[nw].x == x) && (sec[ns].wall[nw].y == y))
				{ ver[iw].s = ns; ver[iw].w = nw; iw++; if (iw >= maxverts) break; }
		}

		if (ir >= iw) break;
		s = ver[ir].s; w = ver[ir].w; ir++;
	} while (1);
	return(iw);
}

int getwalls (int s, int w, vertlist_t *ver, int maxverts)
{
	vertlist_t tver;
	sect_t *sec;
	wall_t *wal, *wal2;
	float fx, fy;
	int i, j, k, bs, bw, nw, vn;

	sec = gst->sect; wal = sec[s].wall; bs = wal[w].ns;
	if ((unsigned)bs >= (unsigned)gst->numsects) return(0);

	vn = 0; nw = wal[w].n+w; bw = wal[w].nw;
	do
	{
		wal2 = sec[bs].wall; i = wal2[bw].n+bw; //Make sure it's an opposite wall
		if ((wal[w].x == wal2[i].x) && (wal[nw].x == wal2[bw].x) &&
			 (wal[w].y == wal2[i].y) && (wal[nw].y == wal2[bw].y))
			{ if (vn < maxverts) { ver[vn].s = bs; ver[vn].w = bw; vn++; } }
		bs = wal2[bw].ns;
		bw = wal2[bw].nw;
	} while (bs != s);

		//Sort next sects by order of height in middle of wall (FIX:sort=crap algo)
	fx = (wal[w].x+wal[nw].x)*.5;
	fy = (wal[w].y+wal[nw].y)*.5;
	for(k=1;k<vn;k++)
		for(j=0;j<k;j++)
			if (getslopez(&sec[ver[j].s],0,fx,fy) + getslopez(&sec[ver[j].s],1,fx,fy) >
				 getslopez(&sec[ver[k].s],0,fx,fy) + getslopez(&sec[ver[k].s],1,fx,fy))
				{ tver = ver[j]; ver[j] = ver[k]; ver[k] = tver; }
	return(vn);
}

#ifdef STANDALONE
static long gquantstat; //-1=none, 0=vertex, 1=edge (wall)
static long gquantsec, gquantwal, gquantcf; //nasty global hack :/
static void cursquant (playerstruct_t *lps, float *qmousx, float *qmousy, float *qmousz, long qsect)
{
	wall_t *wal;
	spri_t *spr;
	float ox, oy, oz, d, mind, x0, y0, dx, dy, dz;
	int i, s, w, bi, bs, bw, s0, s1, numv, avoidownerval;

	ox = (*qmousx);
	oy = (*qmousy);
	oz = (*qmousz);
	gquantstat = -1; gquantsec = -1; gquantwal = 0; gquantcf = 0;

	if ((lps->grabmode == GRABDRAG) || (lps->grabmode == GRABDRAG2))
		  avoidownerval = -1;
	else avoidownerval = lps->playerindex;

	mind = 1e32; bi = (lps->ifor.z > 0); bs = -1;

		//Only search hit sector in 3D mode..
	if (((unsigned)qsect) < (unsigned)gst->numsects) { s0 = qsect; s1 = qsect+1; }
															  else { s0 = 0; s1 = gst->numsects; }

		//Near a sprite?
	//-----------------------------------------------------------------------------------------------
#ifdef STANDALONE
	for(w=gst->numspris-1;w>=0;w--)
	{
		spr = &gst->spri[w];
#else
	for(w=gst->malspris-1;w>=0;w--)
	{
		spr = &gst->spri[w]; if (spr->sect < 0) continue;
#endif
		if (lps->editmode == 2)
		{
			if ((spr->owner >= 0) && (spr->owner != avoidownerval)) continue;
			dx = spr->p.x-ox;
			dy = spr->p.y-oy;
			d = dx*dx + dy*dy; if (d < mind) { mind = d; bs = spr->sect; bw = w+0x40000000; }
		}
	}
	//-----------------------------------------------------------------------------------------------

		//Near a vertex?
	//-----------------------------------------------------------------------------------------------
	if ((lps->sec.n > 1) && (!lps->startstate)) //If drawing loop and not split mode, snap to 1st point
	{
		wal = lps->sec.wall;
		dx = wal[0].x-ox;
		dy = wal[0].y-oy;
		d = dx*dx + dy*dy;
		if (d < lps->dgridlock*lps->dgridlock*(DVERTSNAP*DVERTSNAP))
		{
			(*qmousx) = wal[0].x;
			(*qmousy) = wal[0].y;
			if ((unsigned)lps->startsect < (unsigned)gst->numsects)
				(*qmousz) = getslopez(&gst->sect[lps->startsect],bi,*qmousx,*qmousy);
			return;
		}
	}

	for(s=s0;s<s1;s++)
	{
		wal = gst->sect[s].wall;
		if (lps->editmode == 2)
		{
			for(w=gst->sect[s].n-1;w>=0;w--)
			{
				if ((wal[w].owner >= 0) && (wal[w].owner != avoidownerval)) continue;
				dx = wal[w].x-ox;
				dy = wal[w].y-oy;
				d = dx*dx + dy*dy; if (d < mind) { mind = d; bs = s; bw = w; }
			}
		}
		else
		{
			for(w=gst->sect[s].n-1;w>=0;w--)
			{
				if ((wal[w].owner >= 0) && (wal[w].owner != avoidownerval)) continue;
				dx = wal[w].x-ox;
				dy = wal[w].y-oy;
				for(i=2-1;i>=0;i--)
				{
					d = dx*dx + dy*dy; //dz = getslopez(&gst->sect[s],i,wal[w].x,wal[w].y)-oz; d += dz*dz;
					if (d < mind) { mind = d; bs = s; bw = w; bi = i; }
				}
			}
		}
	}
	if (mind < lps->dgridlock*lps->dgridlock*(DVERTSNAP*DVERTSNAP))
	{
		if ((bw&0xc0000000) == 0x40000000)
		{
			(*qmousx) = gst->spri[bw&0x3fffffff].p.x;
			(*qmousy) = gst->spri[bw&0x3fffffff].p.y;
			(*qmousz) = gst->spri[bw&0x3fffffff].p.z;
		}
		else
		{
			(*qmousx) = gst->sect[bs].wall[bw].x;
			(*qmousy) = gst->sect[bs].wall[bw].y;
			(*qmousz) = getslopez(&gst->sect[bs],bi,*qmousx,*qmousy);
		}
		gquantstat = 0; gquantsec = bs; gquantwal = bw; gquantcf = bi;
		return;
	}
	//-----------------------------------------------------------------------------------------------

		//Near a wall edge?
	//-----------------------------------------------------------------------------------------------
	mind = 1e32; //bi = (lps->ifor.z > 0);
	for(s=s0;s<s1;s++)
	{
		wal = gst->sect[s].wall;
		for(w=gst->sect[s].n-1;w>=0;w--)
		{
													 if ((wal[w].owner >= 0) && (wal[w].owner != avoidownerval)) continue;
			i = wallprev(&gst->sect[s],w); if ((wal[i].owner >= 0) && (wal[i].owner != avoidownerval)) continue;
			i = wal[w].n+w;                if ((wal[i].owner >= 0) && (wal[i].owner != avoidownerval)) continue;

			dx = distpoint2line2(ox,oy,wal[w].x,wal[w].y,wal[i].x,wal[i].y);

				//Use sector on same side of wall as mouse cursor
			if ((wal[w].ns >= 0) && ((ox-wal[w].x)*(wal[w].y-wal[i].y) < (oy-wal[w].y)*(wal[w].x-wal[i].x)))
				continue;

			if (lps->editmode == 2)
			{
				if (dx < mind) { mind = dx; bs = s; bw = w; }
			}
			else
			{
				for(i=2-1;i>=0;i--)
				{
					d = dx; //dz = getslopez(&gst->sect[s],i,wal[w].x,wal[w].y)-oz; d += dz*dz;
					if (d < mind) { mind = d; bs = s; bw = w; bi = i; }
				}
			}
		}
	}
	d = lps->dgridlock*lps->dgridlock*(DEDGESNAP*DEDGESNAP);
	if ((mind < d) && ((bw&0xc0000000) != 0x40000000))
	{
		wal = gst->sect[bs].wall;
		w = wal[bw].n+bw;
		x0 = wal[bw].x; dx = wal[w].x-x0;
		y0 = wal[bw].y; dy = wal[w].y-y0;
		d = ((ox-x0)*dx + (oy-y0)*dy)/(dx*dx + dy*dy);
		if (d < 0.0) d = 0.0;
		if (d > 1.0) d = 1.0;
		(*qmousx) = dx*d + x0;
		(*qmousy) = dy*d + y0;
		if (lps->gridlock)
		{
			ox = floor((*qmousx)/lps->dgridlock+.5)*lps->dgridlock;
			oy = floor((*qmousy)/lps->dgridlock+.5)*lps->dgridlock;
			if ((ox-x0)*dy == (oy-y0)*dx) { (*qmousx) = ox; (*qmousy) = oy; }
		}
		(*qmousz) = getslopez(&gst->sect[bs],bi,*qmousx,*qmousy);
		gquantstat = 1; gquantsec = bs; gquantwal = bw; gquantcf = bi;
		return;
	}
	//-----------------------------------------------------------------------------------------------

		//Align to grid if grid locking enabled
	if (lps->gridlock)
	{
		(*qmousx) = floor(ox/lps->dgridlock+.5)*lps->dgridlock;
		(*qmousy) = floor(oy/lps->dgridlock+.5)*lps->dgridlock;
	}
	else
	{
		(*qmousx) = ox;
		(*qmousy) = oy;
	}
}
#endif

#ifndef _MSC_VER
static _inline int argb_interp (int c0, int c1, int mul15)
{
	unsigned char *u0, *u1;
	u0 = (unsigned char *)&c0;
	u1 = (unsigned char *)&c1;
	u0[0] += (unsigned char)((((long)u1[0]-(long)u0[0])*mul15)>>15);
	u0[1] += (unsigned char)((((long)u1[1]-(long)u0[1])*mul15)>>15);
	u0[2] += (unsigned char)((((long)u1[2]-(long)u0[2])*mul15)>>15);
	u0[3] += (unsigned char)((((long)u1[3]-(long)u0[3])*mul15)>>15);
	return(c0);
}
static _inline int argb_scale (int c0, int mul12)
{
	unsigned char *u0 = (unsigned char *)&c0;
	u0[0] = (unsigned char)((((long)u0[0])*mul12)>>12);
	u0[1] = (unsigned char)((((long)u0[1])*mul12)>>12);
	u0[2] = (unsigned char)((((long)u0[2])*mul12)>>12);
	u0[3] = (unsigned char)((((long)u0[3])*mul12)>>12);
	return(c0);
}
#else
static _inline int argb_interp (int c0, int c1, int mul15)
{
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
static _inline int argb_scale (int c0, int mul12)
{
	_asm
	{
		punpcklbw mm0, c0
		psrlw mm0, 4
		pshufw mm1, mul12, 0
		pmulhw mm0, mm1
		packuswb mm0, mm0
		movd eax, mm0
		emms
	}
}
#endif
static __forceinline int uptil1 (unsigned int *lptr, int z)
{
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

static void dtol (double f, long *a)
{
	_asm
	{
		mov eax, a
		fld f
		fistp dword ptr [eax]
	}
}

void orthofit3x3 (point3d *v0, point3d *v1, point3d *v2)
{
	point3d ov0, ov1;
	float d;

	ov0 = (*v0); ov1 = (*v1);
	v0->x += v1->y*v2->z - v1->z*v2->y;
	v0->y += v1->z*v2->x - v1->x*v2->z;
	v0->z += v1->x*v2->y - v1->y*v2->x;
	d = 1.0/sqrt(v0->x*v0->x + v0->y*v0->y + v0->z*v0->z); //start calculation early
	v1->x += v2->y*ov0.z - v2->z*ov0.y;
	v1->y += v2->z*ov0.x - v2->x*ov0.z;
	v1->z += v2->x*ov0.y - v2->y*ov0.x;
	v2->x += ov0.y*ov1.z - ov0.z*ov1.y;
	v2->y += ov0.z*ov1.x - ov0.x*ov1.z;
	v2->z += ov0.x*ov1.y - ov0.y*ov1.x;
	v0->x *= d; v0->y *= d; v0->z *= d;
	v1->x *= d; v1->y *= d; v1->z *= d;
	v2->x *= d; v2->y *= d; v2->z *= d;
}

void orthorotate (double ox, double oy, double oz, point3d *iri, point3d *ido, point3d *ifo)
{
	double f, t, dx, dy, dz, rr[9];

	dcossin(ox,&ox,&dx);
	dcossin(oy,&oy,&dy);
	dcossin(oz,&oz,&dz);
	f = ox*oz; t = dx*dz; rr[0] =  t*dy + f; rr[7] = -f*dy - t;
	f = ox*dz; t = dx*oz; rr[1] = -f*dy + t; rr[6] =  t*dy - f;
	rr[2] = dz*oy; rr[3] = -dx*oy; rr[4] = ox*oy; rr[8] = oz*oy; rr[5] = dy;
	ox = iri->x; oy = ido->x; oz = ifo->x;
	iri->x = ox*rr[0] + oy*rr[3] + oz*rr[6];
	ido->x = ox*rr[1] + oy*rr[4] + oz*rr[7];
	ifo->x = ox*rr[2] + oy*rr[5] + oz*rr[8];
	ox = iri->y; oy = ido->y; oz = ifo->y;
	iri->y = ox*rr[0] + oy*rr[3] + oz*rr[6];
	ido->y = ox*rr[1] + oy*rr[4] + oz*rr[7];
	ifo->y = ox*rr[2] + oy*rr[5] + oz*rr[8];
	ox = iri->z; oy = ido->z; oz = ifo->z;
	iri->z = ox*rr[0] + oy*rr[3] + oz*rr[6];
	ido->z = ox*rr[1] + oy*rr[4] + oz*rr[7];
	ifo->z = ox*rr[2] + oy*rr[5] + oz*rr[8];
	//orthofit3x3(iri,ido,ifo); //Warning: assumes ido and ifo follow iri in memory!
}

void orthorotate (double ox, double oy, double oz, dpoint3d *iri, dpoint3d *ido, dpoint3d *ifo)
{
	double f, t, dx, dy, dz, rr[9];

	dcossin(ox,&ox,&dx);
	dcossin(oy,&oy,&dy);
	dcossin(oz,&oz,&dz);
	f = ox*oz; t = dx*dz; rr[0] =  t*dy + f; rr[7] = -f*dy - t;
	f = ox*dz; t = dx*oz; rr[1] = -f*dy + t; rr[6] =  t*dy - f;
	rr[2] = dz*oy; rr[3] = -dx*oy; rr[4] = ox*oy; rr[8] = oz*oy; rr[5] = dy;
	ox = iri->x; oy = ido->x; oz = ifo->x;
	iri->x = ox*rr[0] + oy*rr[3] + oz*rr[6];
	ido->x = ox*rr[1] + oy*rr[4] + oz*rr[7];
	ifo->x = ox*rr[2] + oy*rr[5] + oz*rr[8];
	ox = iri->y; oy = ido->y; oz = ifo->y;
	iri->y = ox*rr[0] + oy*rr[3] + oz*rr[6];
	ido->y = ox*rr[1] + oy*rr[4] + oz*rr[7];
	ifo->y = ox*rr[2] + oy*rr[5] + oz*rr[8];
	ox = iri->z; oy = ido->z; oz = ifo->z;
	iri->z = ox*rr[0] + oy*rr[3] + oz*rr[6];
	ido->z = ox*rr[1] + oy*rr[4] + oz*rr[7];
	ifo->z = ox*rr[2] + oy*rr[5] + oz*rr[8];
	//orthofit3x3(iri,ido,ifo); //Warning: assumes ido and ifo follow iri in memory!
}

void slerp (point3d *irig,  point3d *idow,  point3d *ifor,
				point3d *irig2, point3d *idow2, point3d *ifor2,
				point3d *iri,   point3d *ido,   point3d *ifo,   float rat)
{
	point3d ax;
	double c, s, t, ox, oy, oz, k[9];

	iri->x = irig->x; iri->y = irig->y; iri->z = irig->z;
	ido->x = idow->x; ido->y = idow->y; ido->z = idow->z;
	ifo->x = ifor->x; ifo->y = ifor->y; ifo->z = ifor->z;

	ax.x = irig->y*irig2->z - irig->z*irig2->y + idow->y*idow2->z - idow->z*idow2->y + ifor->y*ifor2->z - ifor->z*ifor2->y;
	ax.y = irig->z*irig2->x - irig->x*irig2->z + idow->z*idow2->x - idow->x*idow2->z + ifor->z*ifor2->x - ifor->x*ifor2->z;
	ax.z = irig->x*irig2->y - irig->y*irig2->x + idow->x*idow2->y - idow->y*idow2->x + ifor->x*ifor2->y - ifor->y*ifor2->x;
	t = ax.x*ax.x + ax.y*ax.y + ax.z*ax.z; if (t == 0) return;

		//Based on the vector suck-out method (see ROTATE2.BAS)
	ox = irig->x*ax.x + irig->y*ax.y + irig->z*ax.z;
	oy = idow->x*ax.x + idow->y*ax.y + idow->z*ax.z;
	if (fabs(ox) < fabs(oy))
		{ c = irig->x*irig2->x + irig->y*irig2->y + irig->z*irig2->z; s = ox*ox; }
	else
		{ c = idow->x*idow2->x + idow->y*idow2->y + idow->z*idow2->z; s = oy*oy; }
	if (t == s) return;
	c = (c*t - s) / (t-s);
	if (c < -1) c = -1;
	if (c > 1) c = 1;
	dcossin(acos(c)*rat,&c,&s);

	t = 1.0 / sqrt(t); ax.x *= t; ax.y *= t; ax.z *= t;

	t = 1.0f-c;
	k[0] = ax.x*t; k[7] = ax.x*s; oz = ax.y*k[0];
	k[4] = ax.y*t; k[2] = ax.y*s; oy = ax.z*k[0];
	k[8] = ax.z*t; k[3] = ax.z*s; ox = ax.z*k[4];
	k[0] = ax.x*k[0] + c; k[5] = ox - k[7]; k[7] += ox;
	k[4] = ax.y*k[4] + c; k[6] = oy - k[2]; k[2] += oy;
	k[8] = ax.z*k[8] + c; k[1] = oz - k[3]; k[3] += oz;

	ox = iri->x; oy = iri->y; oz = iri->z;
	iri->x = ox*k[0] + oy*k[1] + oz*k[2];
	iri->y = ox*k[3] + oy*k[4] + oz*k[5];
	iri->z = ox*k[6] + oy*k[7] + oz*k[8];

	ox = ido->x; oy = ido->y; oz = ido->z;
	ido->x = ox*k[0] + oy*k[1] + oz*k[2];
	ido->y = ox*k[3] + oy*k[4] + oz*k[5];
	ido->z = ox*k[6] + oy*k[7] + oz*k[8];

	ox = ifo->x; oy = ifo->y; oz = ifo->z;
	ifo->x = ox*k[0] + oy*k[1] + oz*k[2];
	ifo->y = ox*k[3] + oy*k[4] + oz*k[5];
	ifo->z = ox*k[6] + oy*k[7] + oz*k[8];
}

	//Compatible with memset except:
	//   1. All 32 bits of v are used and expected to be filled
	//   2. Writes max((n+7)&~7,8) bytes
	//   3. Assumes d is aligned on 8 byte boundary

#ifdef STANDALONE

static const char hinge_png[] =
{
	0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,
	0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x10,0x08,0x03,0x00,0x00,0x00,0x40,0xde,0x8d,
	0x6b,0x00,0x00,0x00,0x27,0x50,0x4c,0x54,0x45,0x07,0x07,0x07,0x5d,0x72,0x7d,0x4c,
	0x63,0x6f,0x5b,0x4a,0x46,0x5f,0x62,0x66,0x54,0x6b,0x77,0x57,0x6b,0x75,0x52,0x69,
	0x75,0x4b,0x62,0x6e,0x62,0x79,0x85,0x4e,0x65,0x71,0x55,0x6c,0x78,0x59,0x70,0x7c,
	0xd3,0x85,0x0b,0xc3,0x00,0x00,0x00,0x01,0x74,0x52,0x4e,0x53,0x00,0x40,0xe6,0xd8,
	0x66,0x00,0x00,0x00,0x74,0x49,0x44,0x41,0x54,0x78,0x5e,0x8d,0x8e,0x09,0x0a,0xc0,
	0x20,0x0c,0x04,0x4d,0xbc,0xdb,0xfe,0xff,0xbd,0x4d,0x5c,0x92,0xd2,0x80,0xe0,0x18,
	0xc4,0xc5,0x71,0x31,0x1d,0xd2,0x68,0x06,0xa8,0x85,0xeb,0xce,0xdd,0x05,0x84,0x4f,
	0x59,0xaf,0x99,0xd9,0x04,0x0f,0x04,0xa1,0xf9,0x23,0xc3,0x43,0xb3,0x82,0x1d,0x64,
	0x05,0x59,0xa7,0xe4,0x62,0x07,0x3d,0xc9,0x86,0x0a,0x52,0xc1,0x27,0x4f,0x5d,0x42,
	0xf6,0x8a,0x95,0x04,0x08,0xa5,0x2c,0x45,0x67,0x29,0x26,0xf8,0xa0,0x03,0x35,0x2e,
	0xec,0xff,0x00,0x61,0xcf,0xa1,0x30,0x40,0xfd,0x0b,0x75,0x00,0x11,0x2e,0x70,0x5f,
	0x20,0x44,0x11,0x2a,0x78,0x2a,0x08,0x31,0xbd,0x4d,0xc4,0x0c,0x75,0xc6,0x7a,0xd7,
	0x1e,0x00,0x00,0x00,0x00,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82
};

typedef struct { char *buf; long leng; tiltyp t; } tilelist_t;
static tilelist_t tilelist[] =
{
	{(char *)hinge_png,sizeof(hinge_png)},
};

	//NOTE: font is stored vertically first! (like .ART files)



void drawpix3d(cam_t *cc, float x, float y, float z, long c) {
    xformpos(&x,&y,&z);
    if (z < SCISDIST) return;
    z = cc->h.z/z; drawpix(&cc->c,x*z+cc->h.x,y*z+cc->h.y,c);
}
#if 0
static void drawcirc (tiltyp *dd, long xc, long yc, long r, long c)
{
	long x, y, d;
	for(x=0,y=r,d=1-r;1;x++)
	{
		drawpix(dd,xc+x,yc+y,c); drawpix(dd,xc+y,yc+x,c);
		drawpix(dd,xc+y,yc-x,c); drawpix(dd,xc+x,yc-y,c);
		drawpix(dd,xc-x,yc+y,c); drawpix(dd,xc-y,yc+x,c);
		drawpix(dd,xc-y,yc-x,c); drawpix(dd,xc-x,yc-y,c);
		if (x >= y) break;
		if (d < 0) { d += ( x   <<1)+3; }
				else { d += ((x-y)<<1)+5; y--; }
	}
}
#else
	//06/28/2008: copied&converted from EVALDRAW.C's mydrawsph2d()
static void drawcirc (tiltyp *dd, double fx, double fy, double rad, long curcol)
{
	double d, rad2;
	long x, y, x0, y0, x1, y1;

	if (rad >= 0)
	{
		rad2 = rad*rad;
		y0 = max((long)ceil(fy-rad),    0);
		y1 = min((long)ceil(fy+rad),dd->y);
		for(y=y0;y<y1;y++)
		{
			d = sqrt(rad2 - (y-fy)*(y-fy));
			drawhlin(dd,(long)ceil(fx-d),(long)ceil(fx+d),y,curcol);
		}
	}
	else
	{
		rad = -rad;
		rad2 = rad*rad;
		y0 = max((long)ceil(fy-rad),    0);
		y1 = min((long)ceil(fy+rad),dd->y);
		for(y=y0;y<y1;y++)
		{
			d = rad2 - (y-fy)*(y-fy); if (d < 0) continue; d = sqrt(d);
			drawpix(dd,(long)(fx-d),y,curcol);
			drawpix(dd,(long)(fx+d),y,curcol);
		}
		x0 = max((long)ceil(fx-rad),    0);
		x1 = min((long)ceil(fx+rad),dd->x);
		for(x=x0;x<x1;x++)
		{
			d = rad2 - (x-fx)*(x-fx); if (d < 0) continue; d = sqrt(d);
			drawpix(dd,x,(long)(fy-d),curcol);
			drawpix(dd,x,(long)(fy+d),curcol);
		}
	}
}
#endif

static void drawlinez (cam_t *cc, double x1, double y1, double x2, double y2,
							  double rx0, double ry0, double rz0, double rx1, double ry1, double rz1, long col)
{
	double dx, dy, fxresm1, fyresm1, Za, Zb, Zc;
	float f; //keep me float
	long i, j, k, incr, ie, p, zbufoff;

	dx = x2-x1; dy = y2-y1; if ((dx == 0) && (dy == 0)) return;
	fxresm1 = (double)cc->c.x-.5; fyresm1 = (double)cc->c.y-.5;
		  if (x1 >= fxresm1) { if (x2 >= fxresm1) return; y1 += (fxresm1-x1)*dy/dx; x1 = fxresm1; }
	else if (x1 <        0) { if (x2 <        0) return; y1 += (      0-x1)*dy/dx; x1 =       0; }
		  if (x2 >= fxresm1) {                            y2 += (fxresm1-x2)*dy/dx; x2 = fxresm1; }
	else if (x2 <        0) {                            y2 += (      0-x2)*dy/dx; x2 =       0; }
		  if (y1 >= fyresm1) { if (y2 >= fyresm1) return; x1 += (fyresm1-y1)*dx/dy; y1 = fyresm1; }
	else if (y1 <        0) { if (y2 <        0) return; x1 += (      0-y1)*dx/dy; y1 =       0; }
		  if (y2 >= fyresm1) {                            x2 += (fyresm1-y2)*dx/dy; y2 = fyresm1; }
	else if (y2 <        0) {                            x2 += (      0-y2)*dx/dy; y2 =       0; }

	zbufoff = cc->z.f-cc->c.f;
	if (fabs(dx) >= fabs(dy))
	{
			//Original equation: (rz1*t+rz0) / (rx1*t+rx0) = ghz/(sx-gdps->ghx)
#if (!USEINTZ)
		Za = (rx0*rz1 - rx1*rz0)*gdps->ghz;
#else
		Za = (rx0*rz1 - rx1*rz0)*((float)(1<<28));
#endif
		Zb = rz1; Zc = -gdps->ghx*rz1 - gdps->ghz*rx1;

		if (x2 > x1) { dtol(x1,&i); dtol(x2,&ie); } else { dtol(x2,&i); dtol(x1,&ie); }
		if ((i == 0x80000000) || (ie == 0x80000000)) return; //NaNs can be very bad
		if (i < 0) i = 0;
		if (ie > cc->c.x-1) ie = cc->c.x-1;
		dtol(1048576.0*dy/dx,&incr); dtol(y1*1048576.0+((double)i+.5f-x1)*incr,&j);
		for(;i<=ie;i++,j+=incr)
			if ((unsigned long)(j>>20) < (unsigned long)cc->c.y)
			{
				p = (j>>20)*cc->c.p + (i<<2) + cc->c.f;
#if (USEINTZ)
				dtol(Za / ((double)i*Zb + Zc),&k); if (k >= *(long *)(p+zbufoff)) continue;
				*(long *)(p+zbufoff) = k;
#else
				f = (float)(Za / ((double)i*Zb + Zc)); if (*(long *)&f >= *(long *)(p+zbufoff)) continue;
				*(long *)(p+zbufoff) = *(long *)&f;
#endif
				*(long *)p = col;
			}
	}
	else
	{
#if (!USEINTZ)
		Za = (ry0*rz1 - ry1*rz0)*gdps->ghz;
#else
		Za = (ry0*rz1 - ry1*rz0)*((float)(1<<28));
#endif
		Zb = rz1; Zc = -gdps->ghy*rz1 - gdps->ghz*ry1;

		if (y2 > y1) { dtol(y1,&i); dtol(y2,&ie); } else { dtol(y2,&i); dtol(y1,&ie); }
		if ((i == 0x80000000) || (ie == 0x80000000)) return; //NaNs can be very bad
		if (i < 0) i = 0;
		if (ie > cc->c.y-1) ie = cc->c.y-1;
		dtol(1048576.0*dx/dy,&incr); dtol(x1*1048576.0+((double)i+.5f-y1)*incr,&j);
		for(;i<=ie;i++,j+=incr)
			if ((unsigned long)(j>>20) < (unsigned long)cc->c.x)
			{
				p = i*cc->c.p + ((j>>18)&~3) + cc->c.f;
#if (USEINTZ)
				dtol(Za / ((double)i*Zb + Zc),&k); if (k >= *(long *)(p+zbufoff)) continue;
				*(long *)(p+zbufoff) = k;
#else
				f = (float)(Za / ((double)i*Zb + Zc)); if (*(long *)&f >= *(long *)(p+zbufoff)) continue;
				*(long *)(p+zbufoff) = *(long *)&f;
#endif
				*(long *)p = col;
			}
	}
}

void drawline3d (cam_t *cc, float x0, float y0, float z0, float x1, float y1, float z1, long col)
{
	double ox, oy, oz, r;

	xformpos(&x0,&y0,&z0);
	xformpos(&x1,&y1,&z1);

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

	ox = gdps->ghz/z0;
	oy = gdps->ghz/z1;
	if (col >= 0) drawlinez(cc,x0*ox+gdps->ghx,y0*ox+gdps->ghy,x1*oy+gdps->ghx,y1*oy+gdps->ghy,x0,y0,z0,x1-x0,y1-y0,z1-z0,col);
				else drawline2d(&cc->c,x0*ox+gdps->ghx,y0*ox+gdps->ghy,x1*oy+gdps->ghx,y1*oy+gdps->ghy,col);
}
#endif

static void drawsph (cam_t *cc, double x, double y, double z, double r, long col)
{
	xformpos(&x,&y,&z);
#if (USEINTZ)
	x *= 256; y *= 256; z *= 256; r *= 256;
#endif
	drawsph(x,y,z,r,col,38.4);
}

static void drawcone (cam_t *cc, double x0, double y0, double z0, double r0,
											double x1, double y1, double z1, double r1, long col)
{
	xformpos(&x0,&y0,&z0);
	xformpos(&x1,&y1,&z1);
#if (USEINTZ)
	x0 *= 256; y0 *= 256; z0 *= 256; r0 *= 256;
	x1 *= 256; y1 *= 256; z1 *= 256; r1 *= 256;
#endif
	drawcone(x0,y0,z0,r0,x1,y1,z1,r1,col,38.4,0);
}

static void drawkv6 (cam_t *cc, char *filnam, double px, double py, double pz,
															 double rx, double ry, double rz,
															 double dx, double dy, double dz,
															 double fx, double fy, double fz, long col, double shadefac)
{
	kv6data_t *kv6;
	double d;

	kv6 = drawkv6_get(filnam);
	if (!kv6)
	{
		d = sqrt((rx*rx + ry*ry + rz*rz + dx*dx + dy*dy + dz*dz + fx*fx + fy*fy + fz*fz)/3.0);
		drawsph(cc,px,py,pz,d,col);
		return;
	}

	xformpos(&px,&py,&pz);
	xformrot(&rx,&ry,&rz);
	xformrot(&dx,&dy,&dz);
	xformrot(&fx,&fy,&fz);
#if (USEINTZ)
	px *= 256; py *= 256; pz *= 256;
	rx *= 256; ry *= 256; rz *= 256;
	dx *= 256; dy *= 256; dz *= 256;
	fx *= 256; fy *= 256; fz *= 256;
#endif
#ifdef STANDALONE
	if (gst->p[viewindex].editmode == 2) pz *= 0.99; //Hack to avoid Z-fighting in 2D
#endif

	d = 2.0/((double)max(max(kv6->xsiz,kv6->ysiz),kv6->zsiz));
	rx *= d; ry *= d; rz *= d;
	dx *= d; dy *= d; dz *= d; d = -d;
	fx *= d; fy *= d; fz *= d;

	drawkv6(&drawkv6_frame,kv6, px,py,pz, rx,ry,rz, dx,dy,dz, fx,fy,fz, col, shadefac);
}

void setgammlut (double gammval)
{
	long i;

	gammval = 1.0/gammval;
	for(i=0;i<256;i++) gammlut[i] = pow(((double)i)*(1.0/256.0),gammval)*256.0;

		//Remove all loaded tiles from memory
	if (gtile)
		for(i=gnumtiles-1;i>=0;i--)
			if (gtile[i].tt.f) { free((void *)gtile[i].tt.f); gtile[i].tt.f = 0; }

	gotpal = 0; //Force palette to reload
}

void loadpic (tile_t *tpic)
{
	static unsigned char lastpal[256][4], uch;
	tiltyp *pic;
	long i, j, x, y, filnum, tilenum, loctile0, loctile1, lnx, lny, nx, ny;
	short *sxsiz, *sysiz;
	unsigned char *uptr;
	char tbuf[MAX_PATH*2], tbuf2[MAX_PATH*2];

	pic = &tpic->tt; if (pic->f) return;

	strcpy(tbuf,tpic->filnam);
#if USEGROU

		//.ART loader
	for(i=j=0;tbuf[i];i++) if (tbuf[i] == '|') j = i;
	if (!j) { tilenum = 0; } else { tilenum = atol(&tbuf[j+1]); tbuf[j] = 0; i = j; }
	if ((i >= 5) && (!stricmp(&tbuf[i-4],".ART")))
	{
		if (!gotpal)
		{
			gotpal = 1;
			for(i=j=0;tbuf[i];i++) if ((tbuf[i] == '/') || (tbuf[i] == '\\')) j = i+1;
			strcpy(tbuf2,tbuf);
			strcpy(&tbuf2[j],"palette.dat");
			i = kzopen(tbuf2);
			if (!i)
			{
				strcpy(tbuf2,curmappath); j += strlen(curmappath);
				strcat(tbuf2,tbuf);
				strcpy(&tbuf2[j],"palette.dat");
				i = kzopen(tbuf2);
			}
			if (i)
			{
				kzread(lastpal,768);
				*(long *)&lastpal[255][0] = 0^0xff000000;
				for(i=255-1;i>=0;i--)
				{
					lastpal[i][3] = 255^0xff;
					lastpal[i][2] = gammlut[lastpal[0][i*3+2]<<2];
					lastpal[i][1] = gammlut[lastpal[0][i*3+1]<<2];
					lastpal[i][0] = gammlut[lastpal[0][i*3  ]<<2];
					uch = lastpal[i][0]; lastpal[i][0] = lastpal[i][2]; lastpal[i][2] = uch;
				}
				kzclose();
			}
		}

		filnum = 0; //Scan .ART files, incrementing number until tile is in range
		do
		{
			if (!kzopen(tbuf))
			{
				sprintf(tbuf2,"%s%s",curmappath,tbuf);
				if (!kzopen(tbuf2)) { filnum = -1; break; }
			}
			kzread(tbuf,16); if (*(long *)&tbuf[0] != 1) { filnum = -1; break; }
			loctile0 = *(long *)&tbuf[8];
			loctile1 = (*(long *)&tbuf[12])-loctile0+1;
			i = tilenum-loctile0; if ((unsigned)i < (unsigned)loctile1) { tilenum = i; break; }
			filnum++; sprintf(&tbuf[strlen(tbuf)-7],"%03d.ART",filnum);
		} while (1);
		if (filnum >= 0)
		{
			sxsiz = (short *)_alloca(loctile1<<2); sysiz = &sxsiz[loctile1];
			kzread(sxsiz,loctile1<<2);
			for(i=0,j=16+(loctile1<<3);i<tilenum;i++) j += ((long)sxsiz[i])*((long)sysiz[i]);

			pic->x = (long)sxsiz[tilenum];
			pic->y = (long)sysiz[tilenum];

				//Grab the picanm of the current tile (not currently implemented)
			//kzseek(16+(loctile1<<2)+(tilenum<<2),SEEK_SET);
			//kzread(&i,4);
			//pic->xoffs = ((i<<16)>>24);
			//pic->yoffs = ((i<< 8)>>24);

				//Allocate texture to next higher pow2
			if (pic->x <= 1) lnx = 0; else lnx = bsr(pic->x-1)+1;
			if (pic->y <= 1) lny = 0; else lny = bsr(pic->y-1)+1;
			nx = (1<<lnx); ny = (1<<lny);

			kzseek(j,SEEK_SET);
			uptr = (unsigned char *)_alloca(pic->y);
			pic->p = (nx<<2);
			pic->f = (long)malloc((ny+1)*pic->p+4);

			for(x=0;x<pic->x;x++)
			{
				kzread(uptr,pic->y); i = (x<<2)+pic->f;
				for(y=0;y<pic->y;y++,i+=pic->p) *(long *)i = *(long *)&lastpal[(long)uptr[y]][0];
			}
			kzclose();

				//Scale texture to next higher pow2. Uses box_sum_mip (no bilinear)
			if ((pic->x != nx) || (pic->y != ny))
			{
				tiltyp pow2t;
				pow2t.f = pic->f; pow2t.p = pic->p; pow2t.x = nx; pow2t.y = ny;
				scaletex_boxsum((tiltyp *)pic,&pow2t);
				pic->x = nx; pic->y = ny;
			}

			fixtex4grou((tiltyp *)pic);
			pic->lowermip = 0;
		}
	}
	else
	{
		tiltyp gtt; //FIXFIX
		kpzload4grou(tbuf,&gtt,1.0,2);
		//applyshade(&gtt,1<<14,1<<14); fixtex4grou(&gtt);
		pic->f = gtt.f; pic->p = gtt.p; pic->x = gtt.x; pic->y = gtt.y; pic->lowermip = gtt.lowermip;
	}
#else
	kpzload(tbuf,&pic->f,&pic->p,&pic->x,&pic->y);
	//kpzload(tpic->filnam,&pic->f,&pic->p,&pic->x,&pic->y); //FIX:why don't relative filenames work?
#endif
	if (!pic->f) { pic->f = (long)nullpic; pic->x = 64; pic->y = 64; pic->p = (pic->x<<2); pic->lowermip = 0; }
}

	//copied from evaldraw.c (renamed from drawpol_sse, then modified more) (09/12/2006)
long kglcullmode = 0x405;
__declspec(align(16)) static float dpqmulval[4] = {0,1,2,3}, dpqfours[4] = {4,4,4,4}, dpqones[4] = {1,1,1,1};
__declspec(align(16)) static float dpqaddt[4], dpqaddu[4], dpqaddv[4];
__declspec(align(8)) static __int64 qxmsk, qymsk, qpmul, qpadd;
void drawpol (cam_t *cc, kgln_t *vert, long num, tile_t *tpic, long curcol, float *ouvmat, point3d *norm, long flags)
{
	tiltyp *pic;
	float f;
	long i;

	pic = &tpic->tt; if (!pic->f) loadpic(tpic);

#if USEGROU
	tiltyp gtt;

	gtt.f = pic->f; gtt.p = pic->p; gtt.x = pic->x; gtt.y = pic->y; gtt.z = 1.0; gtt.shsc = 2.0; gtt.lowermip = pic->lowermip;
#if (USEINTZ)
	for(i=num-1;i>=0;i--) { f = 256.f; vert[i].x *= f; vert[i].y *= f; vert[i].z *= f; vert[i].n += i; }
#else
	for(i=num-1;i>=0;i--) vert[i].n += i;
#endif
	if (kglcullmode == 0x404)
	{
#if (USEHEIMAP != 0)
		if ((gdps->rendheight) && (gdps->editmode == 3))
		{
				//Note: drawpolytopo() doesn't support backwards links
			for(i=(num>>1)-1;i>=0;i--)
			{
				kgln_t temp;
				long j = num-1-i, oni, onj;
				oni = vert[i].n; onj = vert[j].n;
				temp = vert[i]; vert[i] = vert[j]; vert[j] = temp;
				vert[i].n = oni; vert[j].n = onj;
			}
		}
		else
#endif
		{
			long *tlong = (long *)_alloca(num*sizeof(tlong[0])); if (!tlong) return;
			for(i=num-1;i>=0;i--) tlong[i] = vert[i].n;
			for(i=num-1;i>=0;i--) vert[tlong[i]].n = i;
		}
	}

	if (gdps->rendinterp) flags |= RENDFLAGS_INTERP;
	if ((gdps->rendheight) && (gdps->editmode == 3)) flags |= RENDFLAGS_HEIGHT|RENDFLAGS_COVSID;
	drawpoly(&gtt,(vertyp *)vert,num,curcol,(((unsigned)curcol)>>24)/16.0,ouvmat,flags);
#else
	kgln_t *pn;
	float g, t, u, v, tx, ty, tb, ux, uy, ub, vx, vy, vb, fx, fy, fz;
	long j, k, l, sx0, sx1, sy, x, xi, pbase, p, pe, n, x0, y0, x1, y1, y, mini, maxi, *ord;
	long lmost[2][MAXYDIM+4], *lptr, *isy, yy0, yy1, rpic, rbpl, rxsiz, rysiz, xmsk, ymsk, zbufoff;

	//FIXFIX: remove this block - obsolete

	rpic = pic->f; rbpl = pic->p; rxsiz = pic->x; rysiz = pic->y;
	for(xmsk=1;(xmsk<<1)+1<rxsiz;xmsk=(xmsk<<1)+1);
	for(ymsk=1;(ymsk<<1)+1<rysiz;ymsk=(ymsk<<1)+1);
	((long *)&qxmsk)[0] = ((long *)&qxmsk)[1] = xmsk;
	((long *)&qymsk)[0] = ((long *)&qymsk)[1] = ymsk;
	((short *)&qpmul)[0] = ((short *)&qpmul)[2] = 4;
	((short *)&qpmul)[1] = ((short *)&qpmul)[3] = rbpl;
	((long *)&qpadd)[0] = ((long *)&qpadd)[1] = rpic;

	pn = (kgln_t *)_alloca((num*2)*sizeof(pn[0]));

	l = n = 0;
	for(i=0;i<num;i++)
	{
		j = vert[i].n+i;
		if (vert[i].z >= SCISDIST) { pn[n] = vert[i]; pn[n].n = 1; n++; }
		if ((vert[j].z >= SCISDIST) != (vert[i].z >= SCISDIST))
		{
			t = (SCISDIST-vert[j].z)/(vert[i].z-vert[j].z);
			pn[n].u = (vert[i].u-vert[j].u)*t + vert[j].u;
			pn[n].v = (vert[i].v-vert[j].v)*t + vert[j].v;
			pn[n].x = (vert[i].x-vert[j].x)*t + vert[j].x;
			pn[n].y = (vert[i].y-vert[j].y)*t + vert[j].y;
			pn[n].z = SCISDIST; //(vert[i].z-vert[j].z)*t + vert[j].z;
			pn[n].n = 1;
			n++;
		}
		if (j < i)
		{
			if (n-l >= 3) { pn[n-1].n = l-(n-1); l = n; } else { n = l; }
		}
	}
	if (n < 3) return;

	for(i=n-1;i>=0;i--) //projection
	{
		pn[i].z = 1.0/pn[i].z;
		f = pn[i].z*gdps->ghz;
		pn[i].x = pn[i].x*f + gdps->ghx;
		pn[i].y = pn[i].y*f + gdps->ghy;
	}

	isy = (long *)_alloca(n*sizeof(isy[0]));
	x0 = cc->c.x; x1 = 0;
	y0 = cc->c.y; y1 = 0;
	for(i=0;i<n;i++)
	{
		x = (long)ceil(pn[i].x);
		y = (long)ceil(pn[i].y); isy[i] = min(max(y,0),cc->c.y);
		if (x < x0) x0 = x;
		if (x > x1) x1 = x;
		if (y < y0) { y0 = y; mini = i; }
		if (y > y1) { y1 = y; maxi = i; }
	}
	if (x0 < 0) x0 = 0; if (x1 > cc->c.x) x1 = cc->c.x; if (x1 <= x0) return;
	if (y0 < 0) y0 = 0; if (y1 > cc->c.y) y1 = cc->c.y; if (y1 <= y0) return;

	for(i=3-1;i>=0;i--) { pn[i].u *= rxsiz; pn[i].v *= rysiz; }
	pn[0].u *= pn[0].z; pn[1].u *= pn[1].z; pn[2].u *= pn[2].z;
	pn[0].v *= pn[0].z; pn[1].v *= pn[1].z; pn[2].v *= pn[2].z;
	fx = pn[1].y-pn[2].y; fy = pn[2].y-pn[0].y; fz = pn[0].y-pn[1].y;
	t = 1.0 / (fx*pn[0].x + fy*pn[1].x + fz*pn[2].x);
	tx = (fx*pn[0].z + fy*pn[1].z + fz*pn[2].z)*t;
	ux = (fx*pn[0].u + fy*pn[1].u + fz*pn[2].u)*t;
	vx = (fx*pn[0].v + fy*pn[1].v + fz*pn[2].v)*t;
	fx = pn[2].x-pn[1].x; fy = pn[0].x-pn[2].x; fz = pn[1].x-pn[0].x;
	ty = (fx*pn[0].z + fy*pn[1].z + fz*pn[2].z)*t;
	uy = (fx*pn[0].u + fy*pn[1].u + fz*pn[2].u)*t;
	vy = (fx*pn[0].v + fy*pn[1].v + fz*pn[2].v)*t;
	tb = pn[0].z - pn[0].x*tx - pn[0].y*ty;
	ub = pn[0].u - pn[0].x*ux - pn[0].y*uy;
	vb = pn[0].v - pn[0].x*vx - pn[0].y*vy;

	_asm
	{
		mov eax, 0x5f80 ;round +inf
		mov i, eax
		ldmxcsr i
	}

	ord = (long *)_alloca(n*sizeof(ord[0]));
	i = mini; j = 0;                while (i != maxi) { ord[j++] = i; i++; if (i >= n) i = 0; }
	i = mini-1; if (i < 0) i = n-1; while (i != maxi) { ord[j++] = i; i--; if (i < 0) i = n-1; }
	ord[j] = maxi;

		//Calculate facing of polygon
	t = 0.f;
	for(i=n-2,j=n-1,k=0;k<n;i=j,j=k,k++) t += (pn[i].x-pn[k].x)*pn[j].y;

	for(k=0;k<n;k++)
	{
		i = ord[k]; j = i+1; if (j >= n) j = 0;
		if (isy[i] == isy[j]) continue;
		if (isy[i] < isy[j]) { yy0 = isy[i]; yy1 = isy[j]; lptr = lmost[1]; }
							 else { yy0 = isy[j]; yy1 = isy[i]; lptr = lmost[0]; }
		if ((kglcullmode == 0x404) || ((!kglcullmode) && (*(long *)&t < 0))) //Cull front or swap to front
			lptr = (long *)(((long)lmost[0]) + ((long)lmost[1]) - ((long)lptr));

		g = (pn[j].x-pn[i].x)/(pn[j].y-pn[i].y); f = (yy0-pn[i].y)*g + pn[i].x;

			//for(y=yy0;y<yy1;y++) { lptr[y] = (long)ceil(f); f += g; }
		_asm
		{
			mov edx, yy0
			mov ecx, lptr
			movss xmm2, f         ;xmm2:  -  , -  , -  , f
			movss xmm0, g         ;xmm0:  -  , -  , -  , g
			shufps xmm2, xmm2, 0  ;xmm2:  f  , f  , f  , f
			shufps xmm0, xmm0, 0  ;xmm0:  g  , g  , g  , g
			movaps xmm1, xmm0     ;xmm1:  g  , g  , g  , g
			mulps xmm0, dpqmulval ;xmm0: 3g  ,2g  ,1g  ,0g
			addps xmm0, xmm2      ;xmm0: 3g+f,2g+f, g+f,  f
			mulps xmm1, dpqfours  ;xmm1: 4g  ,4g  ,4g  ,4g

 dqrast: cvtps2pi mm0, xmm0
			movhlps xmm2, xmm0
			cvtps2pi mm1, xmm2
			movq [ecx+edx*4], mm0
			movq [ecx+edx*4+8], mm1
			addps xmm0, xmm1
			add edx, 4
			cmp edx, yy1
			jl short dqrast

			emms
		}
	}
	_asm
	{
		mov eax, 0x3f80 ;round -inf
		mov i, eax
		ldmxcsr i

		movss xmm0, tx        ;xmm0:  -  , -  , -  ,tx
		shufps xmm0, xmm0, 0  ;xmm0: tx  ,tx  ,tx  ,tx
		movaps xmm5, xmm0     ;xmm5: tx  ,tx  ,tx  ,tx
		mulps xmm0, dpqmulval ;xmm0: tx*3,tx*2,tx*1,tx*0
		mulps xmm5, dpqfours  ;xmm5: tx*4,tx*4,tx*4,tx*4
		movaps dpqaddt, xmm0

		movss xmm0, ux        ;xmm0:  -  , -  , -  ,ux
		shufps xmm0, xmm0, 0  ;xmm0: ux  ,ux  ,ux  ,ux
		movaps xmm6, xmm0     ;xmm6: ux  ,ux  ,ux  ,ux
		mulps xmm0, dpqmulval ;xmm0: ux*3,ux*2,ux*1,ux*0
		mulps xmm6, dpqfours  ;xmm6: ux*4,ux*4,ux*4,ux*4
		movaps dpqaddu, xmm0

		movss xmm0, vx        ;xmm0:  -  , -  , -  ,vx
		shufps xmm0, xmm0, 0  ;xmm0: vx  ,vx  ,vx  ,vx
		movaps xmm7, xmm0     ;xmm7: vx  ,vx  ,vx  ,vx
		mulps xmm0, dpqmulval ;xmm0: vx*3,vx*2,vx*1,vx*0
		mulps xmm7, dpqfours  ;xmm7: vx*4,vx*4,vx*4,vx*4
		movaps dpqaddv, xmm0

		pxor mm7, mm7
		movd mm6, curcol
		punpcklbw mm6, mm6
	}
	pbase = y0*cc->c.p + cc->c.f; zbufoff = cc->z.f-cc->c.f;
	for(sy=y0;sy<y1;sy++,pbase+=cc->c.p)
	{
		sx0 = max(lmost[0][sy],      0);
		sx1 = min(lmost[1][sy],cc->c.x); if (sx0 >= sx1) continue;
		p  = (sx0<<2)+pbase;
		pe = (sx1<<2)+pbase;

		_asm
		{
			push esi
			mov eax, p
			mov ecx, zbufoff

				;xmm0(0) = tx*(float)sx0 + ty*(float)sy + tb;
				;xmm1(0) = ux*(float)sx0 + uy*(float)sy + ub;
				;xmm2(0) = vx*(float)sx0 + vy*(float)sy + vb;
			cvtsi2ss xmm3, sx0
			cvtsi2ss xmm4, sy
			movss xmm0, tx
			mulss xmm0, xmm3
			movss xmm1, ty
			mulss xmm1, xmm4
			addss xmm0, xmm1
			addss xmm0, tb

			movss xmm1, ux
			mulss xmm1, xmm3
			movss xmm2, uy
			mulss xmm2, xmm4
			addss xmm1, xmm2
			addss xmm1, ub

			mulss xmm3, vx
			mulss xmm4, vy
			movss xmm2, vb
			addss xmm2, xmm3
			addss xmm2, xmm4

			shufps xmm0, xmm0, 0
			shufps xmm1, xmm1, 0
			shufps xmm2, xmm2, 0
			addps xmm0, dpqaddt
			addps xmm1, dpqaddu
			addps xmm2, dpqaddv
			movq mm4, qpmul
			movq mm5, qpadd

dpsse:      rcpps xmm4, xmm0

				movaps xmm3, xmm4
				mulps xmm3, xmm1
				cvtps2pi mm0, xmm3
				movhlps xmm3, xmm3
				cvtps2pi mm1, xmm3

				movaps xmm3, xmm4
				mulps xmm3, xmm2
				cvtps2pi mm2, xmm3
				movhlps xmm3, xmm3
				cvtps2pi mm3, xmm3

				pand mm0, qxmsk ;mm0: [u1 u0]
				pand mm1, qxmsk ;mm1: [u3 u2]
				pand mm2, qymsk ;mm2: [v1 v0]
				pand mm3, qymsk ;mm3: [v3 v2]
				pslld mm2, 16
				pslld mm3, 16
				por mm0, mm2 ;mm0: [v1 u1 v0 u0]
				por mm1, mm3 ;mm1: [v3 u3 v2 u2]
				pmaddwd mm0, mm4
				pmaddwd mm1, mm4
				paddd mm0, mm5
				paddd mm1, mm5

				addps xmm0, xmm5
				addps xmm1, xmm6
				addps xmm2, xmm7

					;   xmm4: [z3 z2 z1 z0]
					;mm1:mm0: [r3 r2][r1 r0]
				mov esi, 4
begpsse4:      ucomiss xmm4, [eax+ecx]
					ja short dpskp
					movd edx, mm0
					mov edx, [edx]
					;test edx, edx ;masking
					;jns short dpskp
						movss [eax+ecx], xmm4
						movd mm2, edx
						punpcklbw mm2, mm7
						paddw mm2, mm2
						pmulhuw mm2, mm6
						packuswb mm2, mm2
						movd [eax], mm2    ;mov [eax], edx
dpskp:         add eax, 4
					cmp eax, pe
					jge short dpend
					sub esi, 1
					jz short dpsse
					shufps xmm4, xmm4, 0x39 ;0x39 ;0 3 2 1
					punpckhdq mm0, mm1
					cmp esi, 2
					jne short begpsse4
					movq mm0, mm1
					jmp short begpsse4
dpend:   pop esi
		}
	}
	_asm
	{
		mov eax, 0x1f80
		mov i, eax
		ldmxcsr i
		emms
	}
#endif
}

int isvispol (cam_t *cc, kgln_t *vert, long num)
{
	kgln_t *pn;
	float f, g, t, tx, ty, tb, fx, fy, fz;
	long i, j, k, l, sx0, sx1, sy, x, xi, pbase, p, pe, n, x0, y0, x1, y1, y, mini, maxi, *ord;
	long lmost[2][MAXYDIM+4], *lptr, *isy, yy0, yy1, retval, zbufoff;

	pn = (kgln_t *)_alloca((num*2)*sizeof(pn[0]));

	l = n = 0;
	for(i=0;i<num;i++)
	{
		j = vert[i].n+i;
		if (vert[i].z >= SCISDIST) { pn[n] = vert[i]; pn[n].n = 1; n++; }
		if ((vert[j].z >= SCISDIST) != (vert[i].z >= SCISDIST))
		{
			t = (SCISDIST-vert[j].z)/(vert[i].z-vert[j].z);
			pn[n].x = (vert[i].x-vert[j].x)*t + vert[j].x;
			pn[n].y = (vert[i].y-vert[j].y)*t + vert[j].y;
			pn[n].z = SCISDIST; //(vert[i].z-vert[j].z)*t + vert[j].z;
			pn[n].n = 1;
			n++;
		}
		if (j < i)
		{
			if (n-l >= 3) { pn[n-1].n = l-(n-1); l = n; } else { n = l; }
		}
	}
	if (n < 3) return(0);

	for(i=n-1;i>=0;i--) //projection
	{
		pn[i].z = 1.0/pn[i].z;
		f = pn[i].z*gdps->ghz;
		pn[i].x = pn[i].x*f + gdps->ghx;
		pn[i].y = pn[i].y*f + gdps->ghy;
	}

	isy = (long *)_alloca(n*sizeof(isy[0]));
	x0 = cc->c.x; x1 = 0;
	y0 = cc->c.y; y1 = 0;
	for(i=0;i<n;i++)
	{
		x = (long)ceil(pn[i].x);
		y = (long)ceil(pn[i].y); isy[i] = min(max(y,0),cc->c.y);
		if (x < x0) x0 = x;
		if (x > x1) x1 = x;
		if (y < y0) { y0 = y; mini = i; }
		if (y > y1) { y1 = y; maxi = i; }
	}
	if (x0 < 0) x0 = 0; if (x1 > cc->c.x) x1 = cc->c.x; if (x1 <= x0) return(0);
	if (y0 < 0) y0 = 0; if (y1 > cc->c.y) y1 = cc->c.y; if (y1 <= y0) return(0);

	fx = pn[1].y-pn[2].y; fy = pn[2].y-pn[0].y; fz = pn[0].y-pn[1].y;
	t = 1.0 / (fx*pn[0].x + fy*pn[1].x + fz*pn[2].x);
	tx = (fx*pn[0].z + fy*pn[1].z + fz*pn[2].z)*t;
	fx = pn[2].x-pn[1].x; fy = pn[0].x-pn[2].x; fz = pn[1].x-pn[0].x;
	ty = (fx*pn[0].z + fy*pn[1].z + fz*pn[2].z)*t;
	tb = pn[0].z - pn[0].x*tx - pn[0].y*ty;
#if USEINTZ
	t = gdps->ghz/((float)(1<<28));
	tx *= t; ty *= t; tb *= t;
#endif

	_asm
	{
		mov eax, 0x5f80 ;round +inf
		mov i, eax
		ldmxcsr i
	}

	ord = (long *)_alloca(n*sizeof(ord[0]));
	i = mini; j = 0;                while (i != maxi) { ord[j++] = i; i++; if (i >= n) i = 0; }
	i = mini-1; if (i < 0) i = n-1; while (i != maxi) { ord[j++] = i; i--; if (i < 0) i = n-1; }
	ord[j] = maxi;

		//Calculate facing of polygon
	t = 0.f;
	for(i=n-2,j=n-1,k=0;k<n;i=j,j=k,k++) t += (pn[i].x-pn[k].x)*pn[j].y;

	for(k=0;k<n;k++)
	{
		i = ord[k]; j = i+1; if (j >= n) j = 0;
		if (isy[i] == isy[j]) continue;
		if (isy[i] < isy[j]) { yy0 = isy[i]; yy1 = isy[j]; lptr = lmost[1]; }
							 else { yy0 = isy[j]; yy1 = isy[i]; lptr = lmost[0]; }
		if ((kglcullmode == 0x404) || ((!kglcullmode) && (*(long *)&t < 0))) //Cull front or swap to front
			lptr = (long *)(((long)lmost[0]) + ((long)lmost[1]) - ((long)lptr));

		g = (pn[j].x-pn[i].x)/(pn[j].y-pn[i].y); f = (yy0-pn[i].y)*g + pn[i].x;

			//for(y=yy0;y<yy1;y++) { lptr[y] = (long)ceil(f); f += g; }
		_asm
		{
			mov edx, yy0
			mov ecx, lptr
			movss xmm2, f         ;xmm2:  -  , -  , -  , f
			movss xmm0, g         ;xmm0:  -  , -  , -  , g
			shufps xmm2, xmm2, 0  ;xmm2:  f  , f  , f  , f
			shufps xmm0, xmm0, 0  ;xmm0:  g  , g  , g  , g
			movaps xmm1, xmm0     ;xmm1:  g  , g  , g  , g
			mulps xmm0, dpqmulval ;xmm0: 3g  ,2g  ,1g  ,0g
			addps xmm0, xmm2      ;xmm0: 3g+f,2g+f, g+f,  f
			mulps xmm1, dpqfours  ;xmm1: 4g  ,4g  ,4g  ,4g

 dqrast: cvtps2pi mm0, xmm0
			movhlps xmm2, xmm0
			cvtps2pi mm1, xmm2
			movq [ecx+edx*4], mm0
			movq [ecx+edx*4+8], mm1
			addps xmm0, xmm1
			add edx, 4
			cmp edx, yy1
			jl short dqrast

			emms
		}
	}
	_asm
	{
		mov eax, 0x3f80 ;round -inf
		mov i, eax
		ldmxcsr i

		movss xmm0, tx        ;xmm0:  -  , -  , -  ,tx
		shufps xmm0, xmm0, 0  ;xmm0: tx  ,tx  ,tx  ,tx
		movaps xmm5, xmm0     ;xmm5: tx  ,tx  ,tx  ,tx
		mulps xmm0, dpqmulval ;xmm0: tx*3,tx*2,tx*1,tx*0
		mulps xmm5, dpqfours  ;xmm5: tx*4,tx*4,tx*4,tx*4
		movaps dpqaddt, xmm0
	}
	pbase = y0*cc->c.p + cc->c.f; zbufoff = cc->z.f-cc->c.f; retval = 0;
	for(sy=y0;sy<y1;sy++,pbase+=cc->c.p)
	{
		sx0 = max(lmost[0][sy],      0);
		sx1 = min(lmost[1][sy],cc->c.x); if (sx0 >= sx1) continue;
		p  = (sx0<<2)+pbase+zbufoff;
		pe = (sx1<<2)+pbase+zbufoff;

		_asm
		{
			mov eax, p
//#define TESTVIS
#ifdef TESTVIS
			push esi
			mov esi, zbufoff
			neg esi
#endif

				;xmm0(0) = tx*(float)sx0 + ty*(float)sy + tb;
			cvtsi2ss xmm3, sx0
			cvtsi2ss xmm4, sy
			movss xmm0, tx
			mulss xmm0, xmm3
			movss xmm1, ty
			mulss xmm1, xmm4
			addss xmm0, xmm1
			addss xmm0, tb

			shufps xmm0, xmm0, 0
			addps xmm0, dpqaddt

dpsse:      rcpps xmm4, xmm0 ;xmm4: [z3 z2 z1 z0]
				addps xmm0, xmm5
				mov ecx, 4
begpsse4:
#if (USEINTZ == 0)
					ucomiss xmm4, [eax]
					jbe short ret1
#else
					cvtss2si edx, xmm4
					cmp edx, [eax]
#ifdef TESTVIS
					mov dword ptr [eax+esi], 0xff00ff
					mov dword ptr [eax], 0x1
#endif
					jle short ret1
#endif
					add eax, 4
					cmp eax, pe
					jge short dpend
					sub ecx, 1
					jz short dpsse
					shufps xmm4, xmm4, 0x39 ;0x39 ;0 3 2 1
					jmp short begpsse4
ret1:    mov retval, 1
dpend:
#ifdef TESTVIS
			pop esi
#endif
		}
		if (retval) break;
	}
	_asm
	{
		mov eax, 0x1f80
		mov i, eax
		ldmxcsr i
		emms
	}
	return(retval);
}

void drawparallaxpol (cam_t *cc, kgln_t *vert, long num, tile_t *tpic, long curcol, surf_t *sur, point3d *norm, long flags)
{
	point3d ouvmat[3];
	float f;
	long i, j;
#if 1
		//Crappy paper sky :/
	ouvmat[0].x = 0;     ouvmat[0].y = 0;     ouvmat[0].z = 65536.0;
	ouvmat[1].x = 65536; ouvmat[1].y = 0;     ouvmat[1].z = 65536.0;
	ouvmat[2].x = 0;     ouvmat[2].y = 65536; ouvmat[2].z = 65536.0;

	ouvmat[0].x = sur->uv[0].x*65536;
	ouvmat[0].y = sur->uv[0].y*65536;
	ouvmat[1].x = (sur->uv[1].x+sur->uv[0].x)*65536.0;
	ouvmat[1].y = (sur->uv[1].y+sur->uv[0].y)*65536.0;
	ouvmat[2].x = (sur->uv[2].x+sur->uv[0].x)*65536.0;
	ouvmat[2].y = (sur->uv[2].y+sur->uv[0].y)*65536.0;

	f = atan2(gdps->ifor.y,gdps->ifor.x)*32768/PI*-4.0;
	ouvmat[0].x += f; ouvmat[1].x += f; ouvmat[2].x += f;
	f = atan2(gdps->ifor.z,sqrt(gdps->ifor.x*gdps->ifor.x + gdps->ifor.y*gdps->ifor.y))*32768/PI*-4.0;
	ouvmat[0].y += f; ouvmat[1].y += f; ouvmat[2].y += f;

	drawpol(cc,vert,num,tpic,curcol,(float *)ouvmat,norm,flags|RENDFLAGS_OUVMAT);
#else
	kgln_t *nvert;
	long nnum;

	nvert = (kgln_t *)_alloca(max(num*2,4)*sizeof(nvert[0])); if (!nvert) return;

	nnum = 4;
	f = 16.f;

		//FIXFIX
	for(i=0;i<6;i++)
	{
		static const signed char cubeverts[6][4][3] =
		{
			-1,-1,+1, +1,-1,+1, +1,+1,+1, -1,+1,+1, //Up
			-1,+1,-1, +1,+1,-1, +1,-1,-1, -1,-1,-1, //Down
			-1,-1,-1, -1,-1,+1, -1,+1,+1, -1,+1,-1, //Left
			+1,+1,-1, +1,+1,+1, +1,-1,+1, +1,-1,-1, //Right
			-1,-1,+1, -1,-1,-1, +1,-1,-1, +1,-1,+1, //Front
			+1,+1,+1, +1,+1,-1, -1,+1,-1, -1,+1,+1, //Back
		};
		for(j=0;j<4;j++)
		{
			nvert[j].x = (gdps->irig.x*cubeverts[i][j][0] + gdps->irig.y*cubeverts[i][j][1] + gdps->irig.z*cubeverts[i][j][2])*f;
			nvert[j].y = (gdps->idow.x*cubeverts[i][j][0] + gdps->idow.y*cubeverts[i][j][1] + gdps->idow.z*cubeverts[i][j][2])*f;
			nvert[j].z = (gdps->ifor.x*cubeverts[i][j][0] + gdps->ifor.y*cubeverts[i][j][1] + gdps->ifor.z*cubeverts[i][j][2])*f;
		}
		nvert[0].u = 0; nvert[0].v = 0; nvert[0].n = 1;
		nvert[1].u = 1; nvert[1].v = 0; nvert[1].n = 1;
		nvert[2].u = 1; nvert[2].v = 1; nvert[2].n = 1;
		nvert[3].u = 0; nvert[3].v = 1; nvert[3].n =-3;
		drawpol(cc,nvert,nnum,tpic,curcol,(float *)ouvmat,norm,flags&~RENDFLAGS_OUVMAT); //|RENDFLAGS_OUVMAT);
	}
#endif
}


#ifdef STANDALONE
static void drawmouse (tiltyp *dd, int x, int y, int col)
{
	int i, darkcol;

	darkcol = ((col&0xfefefe)>>1);
	for(i=3;i<=6;i++)
	{
		drawpix(dd,x+i,y-i+1,darkcol);
		drawpix(dd,x+i-1,y-i,darkcol);
		drawpix(dd,x-i,y-i+1,darkcol);
		drawpix(dd,x-i+1,y-i,darkcol);
		drawpix(dd,x-i,y+i-1,darkcol);
		drawpix(dd,x-i+1,y+i,darkcol);
		drawpix(dd,x+i,y+i-1,darkcol);
		drawpix(dd,x+i-1,y+i,darkcol);
		if (i == 6) continue;
		drawpix(dd,x-i,y-i,col);
		drawpix(dd,x+i,y-i,col);
		drawpix(dd,x-i,y+i,col);
		drawpix(dd,x+i,y+i,col);
	}
}

static HCURSOR genblankcursor (void)
{
	unsigned long buf1[32], buf0[32];
	memset(buf1,-1,sizeof(buf1)); memset(buf0,0,sizeof(buf0));
	return(CreateCursor(ghinst,16,16,32,32,buf1,buf0));
}

static HCURSOR gencrosscursor (void)
{
	unsigned long buf1[32], buf0[32];
	long i, j, x, y;

	memset(buf1,-1,sizeof(buf1)); memset(buf0,0,sizeof(buf0));

		//   -6-5-4-3-2-1 0+1+2+3+4+5+6
		//-6   ��                  ��
		//-5 ���۰�              ���۰�
		//-4   ���۰�          ���۰�
		//-3     ���۰�      ���۰�
		//-2       ��          ��
		//-1
		// 0             []
		//+1
		//+2       ��          ��
		//+3     ���۰�      ���۰�
		//+4   ���۰�          ���۰�
		//+5 ���۰�              ���۰�
		//+6   ��                  ��
		//
		//buf1 buf0 display:
		// 0    0   black
		// 0    1   white
		// 1    0   transparent
		// 1    1   xor

	for(j=4;j;j--)
	{
		x = 16 + (j&1)* 6-3; y = 16+((j&2)>>1)* 4-2; buf1[y] &= ~(1<<(x^7)); //black
		x = 16 + (j&1)*10-5; y = 16+((j&2)>>1)*12-6; buf1[y] &= ~(1<<(x^7)); //black
		for(i=5;i>2;i--)
		{
			if (j&1) x = 16+i; else x = 16-i;
			if (j&2) y = 16+i; else y = 16-i;
			buf1[y] &= ~(1<<(x^7));     buf0[y] |= (1<<(x^7)); //white
			buf1[y] &= ~(1<<((x+1)^7)); //black
			buf1[y] &= ~(1<<((x-1)^7)); //black
		}
	}

	return(CreateCursor(ghinst,16,16,32,32,buf1,buf0));
}

#endif

//--------------------------------------------------------------------------------------------------

static void initcrc32 (void)
{
	long i, j, k;
	for(i=255;i>=0;i--)
	{
		k = i;
		for(j=8;j;j--) k = ((unsigned long)k>>1)^((-(k&1))&0xedb88320);
		crctab32[i] = k;
	}
}
static long getcrc32z (long crc32, unsigned char *buf) { long i; for(i=0;buf[i];i++) updatecrc32(buf[i],crc32); return(crc32); }
long gettileind (char *st)
{
	long i, crc32, hashind;

	crc32 = getcrc32z(0,(unsigned char *)st); hashind = (crc32&(sizeof(gtilehashead)/sizeof(gtilehashead[0])-1));
	for(i=gtilehashead[hashind];i>=0;i=gtile[i].hashnext)
	{
		if (gtile[i].namcrc32 != crc32) continue;
		if (!stricmp(gtile[i].filnam,st)) return(i);
	}
	if (gnumtiles >= gmaltiles) { gmaltiles = max(gnumtiles+1,gmaltiles<<1); gtile = (tile_t *)realloc(gtile,gmaltiles*sizeof(tile_t)); }
	strcpy(gtile[gnumtiles].filnam,st);
	gtile[gnumtiles].namcrc32 = crc32;
	gtile[gnumtiles].hashnext = gtilehashead[hashind]; gtilehashead[hashind] = gnumtiles;
	gtile[gnumtiles].tt.f = 0;
	gnumtiles++;
	return(gnumtiles-1);
}

long settilefilename (long hitsect, long hitwall, char *filnam)
{
	surf_t *sur;
	sect_t *sec;
	spri_t *spr;
	long i, j, crc32, hashind;

	if ((unsigned)hitsect >= (unsigned)gst->numsects)
	{
#ifdef STANDALONE
		if (hitsect == -2) //set tab grab instead of v'd surf
		{
			i = gettileind(filnam);

			//if ((hitwall&0xc0000000) == 0x40000000)
			//{
			//   gps->copyspri[0] = gst->spri[hitwall&0x3fffffff];
			//   gps->gotcopy |= 2;
			//}
			//else
			//{
				memset(&gps->copysurf[0],0,sizeof(surf_t));
				gps->copysurf[0].tilnum = i;
				gps->copysurf[0].uv[1].x = 1.f;
				gps->copysurf[0].uv[2].y = 1.f;
				gps->copysurf[0].asc = 4096;
				gps->copysurf[0].rsc = 4096;
				gps->copysurf[0].gsc = 4096;
				gps->copysurf[0].bsc = 4096;
				gps->gotcopy |= 1;
			//}

			return(i);
		}
#endif
		return(-1);
	}

	i = gettileind(filnam);

	if ((hitwall&0xc0000000) == 0x40000000)
	{
		j = 0;
#ifdef STANDALONE
		if ((unsigned)(hitwall&0x3fffffff) < (unsigned)gst->numspris)
			{ spr = &gst->spri[hitwall&0x3fffffff]; j = 1; }
#else
		if ((unsigned)(hitwall&0x3fffffff) < (unsigned)gst->malspris)
			{ spr = &gst->spri[hitwall&0x3fffffff]; if (spr->sect >= 0) j = 1; }
#endif
		if (j)
		{
			spr->tilnum = i;
			i = strlen(filnam)-4;
			if ((i > 0) && (!stricmp(&filnam[i],".KV6")))
			{
				if (fabs(spr->f.x) + fabs(spr->f.y) + fabs(spr->f.z) == 0.f)
				{
					spr->f.x = spr->r.y*spr->d.z - spr->r.z*spr->d.y;
					spr->f.y = spr->r.z*spr->d.x - spr->r.x*spr->d.z;
					spr->f.z = spr->r.x*spr->d.y - spr->r.y*spr->d.x;
				}
				spr->fat = sqrt((spr->r.x*spr->r.x + spr->r.y*spr->r.y + spr->r.z*spr->r.z +
									  spr->d.x*spr->d.x + spr->d.y*spr->d.y + spr->d.z*spr->d.z +
									  spr->f.x*spr->f.x + spr->f.y*spr->f.y + spr->f.z*spr->f.z) / 3.0);
			} else spr->fat = 0;
		} else i = -1;
	}
	else
	{
		sec = gst->sect;
		if (hitwall < 0) sur = &sec[hitsect].surf[hitwall&1];
		else
		{
			sur = &sec[hitsect].wall[hitwall].surf;
			if ((hitwall >= 0) && ((unsigned)hitwall >= (unsigned)&sec[hitsect].n)) sur = 0;
		}
		if (sur)
		{
			sur->tilnum = i;
		} else i = -1;
	}
	return(i);
}

	//Find centroid of polygon (algo copied from TAGPNT2.BAS 09/14/2006)


void reversewalls (wall_t *wal, int n)
{
	wall_t twal;
	int i, j, k, n0, n1;

	for(i=j=0;j<n;j++)
	{
		if (wal[j].n >= 0) continue;
		for(k=((j-i-1)>>1);k>=0;k--) //reverse loop wal[i<=?<=j].x&y (CW <-> CCW)
		{
			n0 = wal[i+k].n; n1 = wal[j-k].n;
			twal = wal[i+k]; wal[i+k] = wal[j-k]; wal[j-k] = twal;
			wal[i+k].n = n0; wal[j-k].n = n1;
		}
		i = j+1;
	}
}

void rotatewallsurfsleft1 (wall_t *wal, int n)
{
	surf_t tsur;
	int i, j, k, n0;

	for(i=j=0;j<n;j++)
	{
		if (wal[j].n >= 0) continue;

			//rotate surfs of loop wal[i<=?<=j].x&y by 1
		tsur = wal[i].surf;
		for(k=i;k<j;k++) wal[k].surf = wal[k+1].surf;
		wal[j].surf = tsur;

		i = j+1;
	}
}

void dragpoint (gamestate_t *lst, int s, int w, float x, float y)
{
	#define MAXVERTS 256 //FIX:timebomb: assumes there are never > 256 sectors connected at same vertex
	vertlist_t verts[MAXVERTS];
	gamestate_t *ost;
	wall_t *wal;
	long i;

	ost = gst; gst = lst;
	for(i=getverts(s,w,verts,MAXVERTS)-1;i>=0;i--)
	{
		wal = &gst->sect[verts[i].s].wall[verts[i].w];
		if ((x != wal->x) || (y != wal->y)) { wal->x = x; wal->y = y; shadowtest2_updatelighting = 1; }
	}
	gst = ost;
}

void delwall (sect_t *s, int w)
{
	wall_t *wal;
	int i;

#if 0
	{ //debug only
		char snotbuf[1024];
		sprintf(snotbuf,"before delwall(wall %d) %d walls\n",w,s->n);
		for(i=0;i<s->n;i++) sprintf(&snotbuf[strlen(snotbuf)],"Wall %d: %f %f %d\n",i,wal[i].x,wal[i].y,wal[i].n);
		MessageBox(ghwnd,snotbuf,prognam,MB_OK);
	}
#endif

	if (!s->n) return;
	wal = s->wall;
	if (wal[w].n < 0) { wal[w-1].n = wal[w].n+1; }
					 else { for(i=w;wal[i].n>0;i++); wal[i].n++; }

#ifdef STANDALONE
	for(i=numplayers-1;i>=0;i--)
	{
		if (((unsigned)gst->p[i].grabsect < (unsigned)gst->numsects) && (&gst->sect[gst->p[i].grabsect] == s))
		{
			if (gst->p[i].grabwall == w) { gst->p[i].grabmode = -1; gst->p[i].grabsect = -1; continue; }
			if (gst->p[i].grabwall > w) gst->p[i].grabwall--;
		}
		if (((unsigned)gst->p[i].startsect < (unsigned)gst->numsects) && (&gst->sect[gst->p[i].startsect] == s))
		{
			if (gst->p[i].startwall == w) { gst->p[i].startsect = -1; continue; }
			if (gst->p[i].startwall > w) gst->p[i].startwall--;
		}
	}
#endif

	s->n--;
	for(i=w;i<s->n;i++) wal[i] = wal[i+1];

#if 0
	{ //debug only
		char snotbuf[1024];
		sprintf(snotbuf,"after delwall(wall %d) %d walls\n",w,s->n);
		for(i=0;i<s->n;i++) sprintf(&snotbuf[strlen(snotbuf)],"Wall %d: %f %f %d\n",i,wal[i].x,wal[i].y,wal[i].n);
		MessageBox(ghwnd,snotbuf,prognam,MB_OK);
	}
#endif
}

int dupwall (sect_t *s, int w)
{
	wall_t *wal;
	int i, j;

	if (s->n >= s->nmax)
	{
		s->nmax = max(s->n+1,s->nmax<<1); s->nmax = max(s->nmax,8);
		s->wall = (wall_t *)realloc(s->wall,s->nmax*sizeof(wall_t));
	}
	wal = s->wall;

#ifdef STANDALONE
	for(i=numplayers-1;i>=0;i--)
	{
		if (((unsigned)gst->p[i].grabsect < (unsigned)gst->numsects) && (&gst->sect[gst->p[i].grabsect] == s))
			if (gst->p[i].grabwall > w) gst->p[i].grabwall++;
		if (((unsigned)gst->p[i].startsect < (unsigned)gst->numsects) && (&gst->sect[gst->p[i].startsect] == s))
			if (gst->p[i].startwall > w) gst->p[i].startwall++;
	}
#endif

	if (!s->n)
	{
		memset(wal,0,sizeof(wall_t));
		wal[0].surf.uv[1].x = wal[0].surf.uv[2].y = 1.f;
		wal[0].ns = wal[0].nw = -1; s->n = 1;
		return(0);
	}
	for(i=s->n;i>w;i--) wal[i] = wal[i-1];
		  if (!wal[0].n)    { wal[0].n = 1; wal[1].n = -1; }
	else if (wal[w].n < 0) { wal[w+1].n = wal[w].n-1; wal[w].n = 1; }
							else { for(i=w+1;wal[i].n>0;i++); wal[i].n--; }
	s->n++;
	return(w+1);
}

void delsect (int s)
{
	sect_t *sec;
	int i;

	sec = gst->sect;

#ifdef STANDALONE
	for(i=numplayers-1;i>=0;i--)
	{
			  if (gst->p[i].grabsect  ==               s) gst->p[i].grabsect  = -1;
		else if (gst->p[i].grabsect  == gst->numsects-1) gst->p[i].grabsect  =  s;
			  if (gst->p[i].startsect  ==               s) gst->p[i].startsect  = -1;
		else if (gst->p[i].startsect  == gst->numsects-1) gst->p[i].startsect  =  s;
	}
#endif

	if (sec[s].wall) free(sec[s].wall);
	gst->numsects--; sec[s] = sec[gst->numsects];
	memset(&sec[gst->numsects],0,sizeof(sect_t));
}

	//Pass z as >1e30 to make updatesect ignore height return first sector containing (x,y)
void updatesect (float x, float y, float z, int *cursect)
{
	sect_t *sec;
	long *gotsect;
	int i, s, w, ns, nw, allsec, cnt, *secfif, secfifw, secfifr;

	sec = gst->sect;
	s = (*cursect);
	if ((unsigned)s >= (unsigned)gst->numsects) //reference invalid; brute force search
	{
		for(s=gst->numsects-1;s>=0;s--)
			if (insidesect(x,y,sec[s].wall,sec[s].n))
				if ((z > 1e30) || ((z >= getslopez(&sec[s],0,x,y)) && (z <= getslopez(&sec[s],1,x,y)))) break;
		(*cursect) = s; return;
	}

	if (insidesect(x,y,sec[s].wall,sec[s].n))
		if ((z > 1e30) || ((z >= getslopez(&sec[s],0,x,y)) && (z <= getslopez(&sec[s],1,x,y)))) return;

	w = (((gst->numsects+31)>>5)<<2);
	gotsect = (long *)_alloca(w);
	memset(gotsect,0,w); gotsect[s>>5] |= (1<<s);
	secfif = (int *)_alloca(gst->numsects*sizeof(secfif[0]));
	secfifw = secfifr = 0;

	(*cursect) = -1; allsec = gst->numsects-1;
	for(cnt=gst->numsects-1;cnt>0;cnt--)
	{
		for(w=sec[s].n-1;w>=0;w--)
		{
			ns = sec[s].wall[w].ns;
			nw = sec[s].wall[w].nw;
			while (((unsigned)ns < (unsigned)gst->numsects) && (ns != s))
			{
				if (!(gotsect[ns>>5]&(1<<ns)))
				{
					gotsect[ns>>5] |= (1<<ns);
					secfif[secfifw] = ns; secfifw++;
				}
				i = ns;
				ns = sec[i].wall[nw].ns;
				nw = sec[i].wall[nw].nw;
			}
		}

		if (secfifr < secfifw)
			{ s = secfif[secfifr]; secfifr++; } //breadth-first
		else
		{     //fifo was empty.. must be some disjoint sectors
			while ((allsec >= 0) && (gotsect[allsec>>5]&(1<<allsec))) allsec--;
			s = allsec; if (s < 0) break;
			gotsect[s>>5] |= (1<<s);
		}

		if (insidesect(x,y,sec[s].wall,sec[s].n))
			if ((z > 1e30) || ((z >= getslopez(&sec[s],0,x,y)) && (z <= getslopez(&sec[s],1,x,y))))
				{ (*cursect) = s; return; }
	}
}

long sect_isneighs (int s0, int s1)
{
	sect_t *sec;
	int i, w, ns, nw;

	sec = gst->sect;
	//if (s0 == s1) return(0); ?
	for(w=sec[s0].n-1;w>=0;w--)
	{
		ns = sec[s0].wall[w].ns;
		nw = sec[s0].wall[w].nw;
		while (((unsigned)ns < (unsigned)gst->numsects) && (ns != s0))
		{
			if (ns == s1) return(1); //s0 and s1 are neighbors
			i = ns;
			ns = sec[i].wall[nw].ns;
			nw = sec[i].wall[nw].nw;
		}
	}
	return(0); //bunches not on neighboring sectors are designated as incomparable
}

long insspri (int sect, float x, float y, float z)
{
	spri_t *spr;
	long i;

	if ((unsigned)sect >= (unsigned)gst->numsects) return(-1);
	if (gst->numspris >= gst->malspris)
	{
		gst->malspris = max(gst->numspris+1,gst->malspris<<1);
		gst->spri = (spri_t *)realloc(gst->spri,gst->malspris*sizeof(spri_t));
#ifndef STANDALONE
		for(i=gst->numspris;i<gst->malspris;i++)
		{
			gst->spri[i].sectn = gst->blankheadspri;
			gst->spri[i].sectp = -1;
			gst->spri[i].sect = -1;
			if (gst->blankheadspri >= 0) gst->spri[gst->blankheadspri].sectp = i;
			gst->blankheadspri = i;
		}
#endif
	}
#ifdef STANDALONE
	i = gst->numspris;
#else
	i = gst->blankheadspri;
	gst->blankheadspri = gst->spri[i].sectn;
	gst->spri[i].sectp = -1;
#endif
	gst->numspris++;
	spr = &gst->spri[i];
	memset(spr,0,sizeof(spri_t));
	spr->p.x = x; spr->p.y = y; spr->p.z = z;
	spr->r.x = .5; spr->d.z = .5; spr->f.y =-.5;
	spr->fat = .5; spr->mas = spr->moi = 1.0;
	spr->tilnum = -1; spr->asc = spr->rsc = spr->gsc = spr->bsc = 4096;
	spr->owner = -1; spr->flags = 0;
	spr->sect = sect; spr->sectn = gst->sect[sect].headspri; spr->sectp = -1;
	if (gst->sect[sect].headspri >= 0) gst->spri[gst->sect[sect].headspri].sectp = i;
	gst->sect[sect].headspri = i;
	return(i);
}

	//          -1      i
	//headspri     i      j
	//               j     -1
void delspri (int i)
{
	spri_t *spr;
	long j;

#ifdef STANDALONE
	if ((unsigned)i >= (unsigned)gst->numspris) return;
#else
	if (((unsigned)i >= (unsigned)gst->malspris) || (gst->spri[i].sect < 0)) return;
#endif
	spr = gst->spri;

		//Delete sprite i
	if (spr[i].sectp <  0) gst->sect[spr[i].sect].headspri = spr[i].sectn;
							else spr[spr[i].sectp].sectn = spr[i].sectn;
	if (spr[i].sectn >= 0) spr[spr[i].sectn].sectp = spr[i].sectp;

	for(j=gst->light_sprinum-1;j>=0;j--)
		if (gst->light_spri[j] == i) gst->light_spri[j] = gst->light_spri[--gst->light_sprinum];

	gst->numspris--;
#ifdef STANDALONE
		//Move sprite numspris to i
	if (i == gst->numspris) return;

	for(j=gst->light_sprinum-1;j>=0;j--)
		if (gst->light_spri[j] == gst->numspris) gst->light_spri[j] = i;

	spr[i] = spr[gst->numspris];
	if (spr[i].sectp <  0) gst->sect[spr[i].sect].headspri = i;
							else spr[spr[i].sectp].sectn = i;
	if (spr[i].sectn >= 0) spr[spr[i].sectn].sectp = i;
#else
		//Add sprite i to blankheadspri list
	gst->spri[i].sectn = gst->blankheadspri;
	gst->spri[i].sectp = -1;
	gst->spri[i].sect = -1;
	if (gst->blankheadspri >= 0) gst->spri[gst->blankheadspri].sectp = i;
	gst->blankheadspri = i;
#endif
}

void changesprisect (int i, int nsect)
{
	spri_t *spr;
	int osect;

#ifdef STANDALONE
	if ((unsigned)i >= (unsigned)gst->numspris) return;
#else
	if (((unsigned)i >= (unsigned)gst->malspris) || (gst->spri[i].sect < 0)) return;
#endif
	if ((unsigned)nsect >= (unsigned)gst->numsects) return;

	spr = &gst->spri[i];
	osect = spr->sect;

		//Remove from old sector list
	//if ((unsigned)osect < (unsigned)gst->numsects)
	//{
		if (spr->sectp < 0) gst->sect[osect].headspri = spr->sectn;
					 else gst->spri[spr->sectp].sectn = spr->sectn;
		if (spr->sectn >= 0) gst->spri[spr->sectn].sectp = spr->sectp;
	//}

		//Insert on new sector list
	//if ((unsigned)nsect < (unsigned)gst->numsects)
	//{
		spr->sectn = gst->sect[nsect].headspri;
		spr->sectp = -1;
		if (gst->sect[nsect].headspri >= 0) gst->spri[gst->sect[nsect].headspri].sectp = i;
		gst->sect[nsect].headspri = i;
	//}

	spr->sect = nsect;
}

	//s: sector of sprites to check
	//Pass -1 to check and compact all valid sprites
void checksprisect (int s)
{
	sect_t *sec;
	spri_t *spr;
	int w, ns, nw, s0, s1;

	sec = gst->sect;
	spr = gst->spri;
#if 0
	//FIXFIX:Warning: this block has not been tested!
	if ((unsigned)s < (unsigned)gst->numsects)
	{
		for(w=sec[s].headspri;w>=0;w=nw)
		{
			nw = spr[w].sectn;
			ns = spr[w].sect; updatesect(spr[w].p.x,spr[w].p.y,spr[w].p.z,&ns);
			if (ns != spr[w].sect) changesprisect(w,ns);
		}
		return;
	}
#endif
	for(s=gst->numsects-1;s>=0;s--) sec[s].headspri = -1;
	for(w=nw=0;w<gst->numspris;w++)
	{
		ns = spr[w].sect; if (ns < 0) continue;
		updatesect(spr[w].p.x,spr[w].p.y,spr[w].p.z,&ns);
		if ((unsigned)ns >= (unsigned)gst->numsects) ns = spr[w].sect;

		spr[nw] = spr[w];
		spr[nw].sect = ns;
		spr[nw].sectn = sec[ns].headspri;
		spr[nw].sectp = -1;
		if (sec[ns].headspri >= 0) spr[sec[ns].headspri].sectp = nw;
		sec[ns].headspri = nw;
		nw++;
	}
#ifndef STANDALONE
	gst->blankheadspri = -1;
	for(;nw<gst->malspris;nw++)
	{
		gst->spri[nw].sectn = gst->blankheadspri;
		gst->spri[nw].sectp = -1;
		gst->spri[nw].sect = -1;
		if (gst->blankheadspri >= 0) gst->spri[gst->blankheadspri].sectp = nw;
		gst->blankheadspri = nw;
	}
#endif
}

//--------------------------------------------------------------------------------------------------

#ifdef STANDALONE
static void drawsectfill (cam_t *cc, wall_t *wal, int n, int col)
{
	float *cross, *xplc, *xinc;
	int i, j, k, g, y, iymin, iymax, ncross, *yceil, *yint;

	yceil = (int   *)_alloca(n*sizeof(yceil[0]));
	yint  = (int   *)_alloca(n*sizeof(yint [0]));
	xplc  = (float *)_alloca(n*sizeof(xplc [0]));
	xinc  = (float *)_alloca(n*sizeof(xinc [0]));
	cross = (float *)_alloca(n*sizeof(cross[0]));

	for(i=n-1;i>=0;i--) yint[i] = yceil[i] = (int)ceil(wal[i].y);
	for(i=n-1;i>=0;i--)
	{
		j = wal[i].n+i; if (wal[i].y == wal[j].y) continue;
		xinc[i] = (wal[j].x-wal[i].x)/(wal[j].y-wal[i].y);
		xplc[i] = (max(min(yceil[i],yceil[j]),0) - wal[i].y)*xinc[i] + wal[i].x;
	}

	shellsrt(yceil,n);
	iymax = min(yceil[n-1],cc->c.y);
	iymin = max(yceil[0],0);
	for(y=iymin;y<iymax;y++) //FIX:optimize
	{
		ncross = 0;
		for(i=n-1;i>=0;i--)
			if ((yint[i] <= y) == (y < yint[wal[i].n+i]))
				{ cross[ncross++] = xplc[i]; xplc[i] += xinc[i]; }
		shellsrt(cross,ncross); ncross--;
		for(i=0;i<ncross;i+=2)
			drawhlin(&cc->c,cross[i],cross[i+1],y,col);
	}
}
#endif

	//Returned trapezoid list is guaranteed to be in English text order (MSD:top->bottom, LSD:left->right)
	//Caller is responsible for freeing memory if ((*retzoids) != 0)
typedef struct { float x[4], y[2]; int pwal[2]; } zoid_t;
static int sect2trap (wall_t *wal, int n, zoid_t **retzoids, int *retnzoids)
{
	zoid_t *zoids;
	float f, x0, y0, x1, y1, sy0, sy1, cury, *secy, *trapx0, *trapx1;
	int i, j, k, g, s, secn, ntrap, tot, zoidalloc, *pwal;

	(*retzoids) = 0; (*retnzoids) = 0; if (n < 3) return(0);

	secy   = (float *)_alloca(n*sizeof(  secy[0])); if (!secy  ) return(0);
	trapx0 = (float *)_alloca(n*sizeof(trapx0[0])); if (!trapx0) return(0);
	trapx1 = (float *)_alloca(n*sizeof(trapx1[0])); if (!trapx1) return(0);
	pwal   = (int   *)_alloca(n*sizeof(  pwal[0])); if (!pwal  ) return(0);

	for(i=n-1;i>=0;i--) secy[i] = wal[i].y;
	shellsrt(secy,n);
	for(i=secn=0,cury=-1e32;i<n;i++) //remove dups
		if (secy[i] > cury) { secy[secn++] = cury = secy[i]; }

	zoidalloc = secn*2; //just a guess (not guaranteed to fit)
	zoids = (zoid_t *)malloc(zoidalloc*sizeof(zoid_t)); if (!zoids) return(0);

	tot = 0;
	for(s=0;s<secn-1;s++)
	{
		sy0 = secy[s]; sy1 = secy[s+1]; ntrap = 0;
		for(i=0;i<n;i++) //FIX:optimize
		{
			x0 = wal[i].x; y0 = wal[i].y; j = wal[i].n+i;
			x1 = wal[j].x; y1 = wal[j].y;
			if (y0 > y1)
			{
				f = x0; x0 = x1; x1 = f;
				f = y0; y0 = y1; y1 = f;
			}
			if ((y0 >= sy1) || (y1 <= sy0)) continue;
			if (y0 < sy0) x0 = (sy0-wal[i].y)*(wal[j].x-wal[i].x)/(wal[j].y-wal[i].y) + wal[i].x;
			if (y1 > sy1) x1 = (sy1-wal[i].y)*(wal[j].x-wal[i].x)/(wal[j].y-wal[i].y) + wal[i].x;
			trapx0[ntrap] = x0; trapx1[ntrap] = x1; pwal[ntrap] = (int)&wal[i]; ntrap++;
		}
		for(g=(ntrap>>1);g;g>>=1)
			for(i=0;i<ntrap-g;i++)
				for(j=i;j>=0;j-=g)
				{
					if (trapx0[j]+trapx1[j] <= trapx0[j+g]+trapx1[j+g]) break;
					f = trapx0[j]; trapx0[j] = trapx0[j+g]; trapx0[j+g] = f;
					f = trapx1[j]; trapx1[j] = trapx1[j+g]; trapx1[j+g] = f;
					k =   pwal[j];   pwal[j] =   pwal[j+g];   pwal[j+g] = k;
				}

		if (tot+ntrap > zoidalloc)
		{
			zoidalloc = max(zoidalloc<<1,tot+ntrap);
			zoids = (zoid_t *)realloc(zoids,zoidalloc*sizeof(zoid_t)); if (!zoids) return(0);
		}
		for(i=0;i<ntrap;i=j+1)
		{
			j = i+1; if ((trapx0[i+1] <= trapx0[i]) && (trapx1[i+1] <= trapx1[i])) continue;
			while ((j+2 < ntrap) && (trapx0[j+1] <= trapx0[j]) && (trapx1[j+1] <= trapx1[j])) j += 2;
			zoids[tot].x[0] = trapx0[i]; zoids[tot].x[1] = trapx0[j]; zoids[tot].y[0] = sy0;
			zoids[tot].x[3] = trapx1[i]; zoids[tot].x[2] = trapx1[j]; zoids[tot].y[1] = sy1;
			zoids[tot].pwal[0] = pwal[i]; zoids[tot].pwal[1] = pwal[j];
			tot++;
		}
	}
	(*retzoids) = zoids; (*retnzoids) = tot; return(1);
}

#if (USEHEIMAP != 0)
void drawsectfill3d2 (cam_t *cc, sect_t *sec, int isflor, int col)
#else
void drawsectfill3d (cam_t *cc, sect_t *sec, int isflor, int col)
#endif
{
	surf_t *sur;
	kgln_t *pol;
	wall_t *wal;
	point3d ouvmat[3], norm;
	point2d *grad;
	float f, g, fz, ax, ay, wx, wy, fk[6];
	int i, j, flags;
	char *gotpt;

	wal = sec->wall; fz = sec->z[isflor]; grad = &sec->grad[isflor]; if (sec->n < 3) return;
	//if (sec->surf[isflor].flags&(1<<16)) return; //FIXFIX:Parallax

		//plane point: (wal[0].x,wal[0].y,fz)
		//plane norm: <grad->x,grad->y,1>
		//
		//   (wal[i].x-wal[0].x)*grad->x +
		//   (wal[i].y-wal[0].y)*grad->y +
		//   (?       -      fz)*      1 = 0
	pol = (kgln_t *)_alloca(sec->n*sizeof(kgln_t)); if (!pol) return;
	sur = &sec->surf[isflor];
	for(i=0;i<sec->n;i++)
	{
		j = wal[i].n+i;
		pol[i].x = wal[i].x;
		pol[i].y = wal[i].y;
		pol[i].z = (wal[0].x-wal[i].x)*grad->x + (wal[0].y-wal[i].y)*grad->y + fz;
		pol[i].n = wal[i].n;
		xformpos(&pol[i].x,&pol[i].y,&pol[i].z);
	}

		//FIXFIX
	//ouvmat[0].x = (sur->uv[0].y*sur->uv[2].x - sur->uv[0].x*sur->uv[2].y) / (sur->uv[1].x*sur->uv[2].y - sur->uv[1].y*sur->uv[2].x);
	//ouvmat[0].y = (sur->uv[1].y*sur->uv[0].x - sur->uv[1].x*sur->uv[0].y) / (sur->uv[1].x*sur->uv[2].y - sur->uv[1].y*sur->uv[2].x);
	//ouvmat[1].x = sur->uv[2].x/(sur->uv[1].x*sur->uv[2].y - sur->uv[1].y*sur->uv[2].x) + ouvmat[0].x;
	//ouvmat[1].y = sur->uv[1].x/(sur->uv[1].x*sur->uv[2].y - sur->uv[1].y*sur->uv[2].x) + ouvmat[0].y;
	//ouvmat[2].x = sur->uv[2].y/(sur->uv[1].x*sur->uv[2].y - sur->uv[1].y*sur->uv[2].x) + ouvmat[0].x;
	//ouvmat[2].y = sur->uv[1].y/(sur->uv[1].x*sur->uv[2].y - sur->uv[1].y*sur->uv[2].x) + ouvmat[0].y;
	//drawcone(cc,ouvmat[0].x,ouvmat[0].y,getslopez(sec,isflor,ouvmat[0].x,ouvmat[0].y),0.02,ouvmat[1].x,ouvmat[1].y,getslopez(sec,isflor,ouvmat[1].x,ouvmat[1].y),0.02,0xffc08080);
	//drawcone(cc,ouvmat[0].x,ouvmat[0].y,getslopez(sec,isflor,ouvmat[0].x,ouvmat[0].y),0.02,ouvmat[2].x,ouvmat[2].y,getslopez(sec,isflor,ouvmat[2].x,ouvmat[2].y),0.02,0xff80c080);


	if (!(sur->flags&4)) //Not relative alignment
	{
			//(sur->uv[1].x)*x + (sur->uv[2].x)*y = (u-sur->uv[0].x)
			//(sur->uv[1].y)*x + (sur->uv[2].y)*y = (v-sur->uv[0].y)
		fk[0] = sur->uv[1].x; fk[2] = sur->uv[2].x;
		fk[1] = sur->uv[1].y; fk[3] = sur->uv[2].y;
		fz = 1.0; ax = -sur->uv[0].x; ay = -sur->uv[0].y;

			//FIXFIX: hack for nukeland starting ceiling wrong scale
		//if ((sec == &gst->sect[117]) && (!(rand()&1))) { fk[3] *= -1; }

			//FIXFIX:fix texture base to be near camera: doesn't work!
		//g = 1.0/(sur->uv[1].x*sur->uv[2].y - sur->uv[1].y*sur->uv[2].x);
		///ax += floor(((sur->uv[0].y-gdps->ipos.y)*sur->uv[2].x - (sur->uv[0].x-gdps->ipos.x)*sur->uv[2].y)*g + .5);
		///ay += floor(((sur->uv[0].x-gdps->ipos.x)*sur->uv[1].y - (sur->uv[0].y-gdps->ipos.y)*sur->uv[1].x)*g + .5);
		//wx = (sur->uv[0].y*sur->uv[2].x - sur->uv[0].x*sur->uv[2].y)*g - gdps->ipos.x;
		//wy = (sur->uv[1].y*sur->uv[0].x - sur->uv[1].x*sur->uv[0].y)*g - gdps->ipos.y;
		//ax += floor((wy*sur->uv[2].x - wx*sur->uv[2].y)*g + .5);
		//ay += floor((wx*sur->uv[1].y - wy*sur->uv[1].x)*g + .5);
	}
	else //Relative alignment
	{
		wx = wal[wal[0].n].x-wal[0].x;
		wy = wal[wal[0].n].y-wal[0].y;
		fk[0] = sur->uv[1].x*wx - sur->uv[2].x*wy; fk[2] = sur->uv[1].x*wy + sur->uv[2].x*wx;
		fk[1] = sur->uv[1].y*wx - sur->uv[2].y*wy; fk[3] = sur->uv[1].y*wy + sur->uv[2].y*wx;
		fz = sqrt(wx*wx + wy*wy);
		ax = (wx*wal[0].x + wy*wal[0].y)*sur->uv[1].x + (wx*wal[0].y - wy*wal[0].x)*sur->uv[2].x - sur->uv[0].x*fz;
		ay = (wx*wal[0].x + wy*wal[0].y)*sur->uv[1].y + (wx*wal[0].y - wy*wal[0].x)*sur->uv[2].y - sur->uv[0].y*fz;
	}

	for(i=3-1;i>=0;i--)
	{          //u,v:
		fk[4] = (i==1)*fz + ax;
		fk[5] = (i==2)*fz + ay;
		f = fk[0]*fk[3] - fk[1]*fk[2]; if (f > 0.f) f = 1.f/f;
		ouvmat[i].x = (fk[3]*fk[4] - fk[2]*fk[5])*f;
		ouvmat[i].y = (fk[0]*fk[5] - fk[1]*fk[4])*f;
		ouvmat[i].z = getslopez(sec,isflor,ouvmat[i].x,ouvmat[i].y);
		xformpos(&ouvmat[i].x,&ouvmat[i].y,&ouvmat[i].z);
#if (USEINTZ)
		ouvmat[i].x *= 256.0; ouvmat[i].y *= 256.0; ouvmat[i].z *= 256.0;
#endif
	}

	flags = RENDFLAGS_OUVMAT;
	if (gdps->compact2d >= 1.0) flags |= RENDFLAGS_NODEPTHTEST;

	norm.x = sec->grad[isflor].x;
	norm.y = sec->grad[isflor].y;
	norm.z = 1.0; if (isflor) { norm.x *= -1; norm.y *= -1; norm.z *= -1; }
	f = 1.0/sqrt(norm.x*norm.x + norm.y*norm.y + norm.z*norm.z); norm.x *= f; norm.y *= f; norm.z *= f;

	if (!isflor) kglcullmode = 0x404;
	if (sec->surf[isflor].flags&(1<<16)) //parallaxing ceil/flor
		  drawparallaxpol(cc,pol,sec->n,&gtile[sur->tilnum],col,sur            ,&norm,flags);
	else drawpolfunc    (cc,pol,sec->n,&gtile[sur->tilnum],col,(float *)ouvmat,&norm,flags);
	if (!isflor) kglcullmode = 0x405;
}

#if (USEHEIMAP != 0)
//FIXFIX:this code is now obsolete! no need to triangulate any more : )
void drawsectfill3d (cam_t *cc, sect_t *sec, int isflor, int col)
{
	point3d norm;
	point2d *grad;
	kgln_t pol[4];
	wall_t *wal;
	zoid_t *zoids;
	float fz, f, ox, oy, nx, ny, wx, wy;
	int i, j, n, nzoids;

	if (((!gdps->rendheight) || (gdps->editmode == 2)) || (sec->surf[isflor].flags&(1<<16))) //parallaxing ceil/flor
		{ drawsectfill3d2(cc,sec,isflor,col); return; }

	wal = sec->wall; fz = sec->z[isflor]; grad = &sec->grad[isflor]; n = sec->n;
	//if (sec->surf[isflor].flags&(1<<16)) return; //FIX:Parallax wrong hack

	if (!sect2trap(wal,n,&zoids,&nzoids)) return;

	norm.x = sec->grad[isflor].x;
	norm.y = sec->grad[isflor].y;
	norm.z = 1.0; if (isflor) { norm.x *= -1; norm.y *= -1; norm.z *= -1; }
	f = 1.0/sqrt(norm.x*norm.x + norm.y*norm.y + norm.z*norm.z); norm.x *= f; norm.y *= f; norm.z *= f;

	if (!isflor) kglcullmode = 0x404;
	for(i=0;i<nzoids;i++)
	{
		for(j=0,n=0;j<4;j++)
		{
			pol[n].x = zoids[i].x[j];
			pol[n].y = zoids[i].y[j>>1];
			if ((!n) || (pol[n].x != pol[n-1].x) || (pol[n].y != pol[n-1].y))
			{
				if (!(sec->surf[isflor].flags&4))
				{
					pol[n].u = sec->surf[isflor].uv[1].x*pol[n].x + sec->surf[isflor].uv[2].x*pol[n].y + sec->surf[isflor].uv[0].x;
					pol[n].v = sec->surf[isflor].uv[1].y*pol[n].x + sec->surf[isflor].uv[2].y*pol[n].y + sec->surf[isflor].uv[0].y;
				}
				else
				{
						//Relative alignment
					ox = pol[n].x-wal[0].x; wx = wal[wal[0].n].x-wal[0].x;
					oy = pol[n].y-wal[0].y; wy = wal[wal[0].n].y-wal[0].y;
					f = 1.0/sqrt(wx*wx + wy*wy);
					nx = (wx*ox + wy*oy)*f;
					ny = (wx*oy - wy*ox)*f;
					pol[n].u = sec->surf[isflor].uv[1].x*nx + sec->surf[isflor].uv[2].x*ny + sec->surf[isflor].uv[0].x;
					pol[n].v = sec->surf[isflor].uv[1].y*nx + sec->surf[isflor].uv[2].y*ny + sec->surf[isflor].uv[0].y;
				}
				pol[n].z = (wal[0].x-pol[n].x)*grad->x + (wal[0].y-pol[n].y)*grad->y + fz;
				pol[n].n = 1; n++;
			}
		}
		if (n < 3) continue;
		pol[n-1].n = 1-n;
		for(j=0;j<n;j++) xformpos(&pol[j].x,&pol[j].y,&pol[j].z);
		drawpolfunc(cc,pol,n,&gtile[sec->surf[isflor].tilnum],col,0,&norm,0);
	}
	if (!isflor) kglcullmode = 0x405;
	free(zoids);
}
#endif

//---------------------------------------------------------------------------------------------

	// ��Ŀ
	// �A��Ŀ
	// ����B�
	//   ����
	//   Collinear line priority:
	//1st sector (wal0): POLYBOOL_AND, POLYBOOL_SUB, POLYBOOL_OR
	//2nd sector (wal1): POLYBOOL_SUB2
	//if retwal is null, returns # walls without generating wall list

	//Split complex polygon by line. Returns complex polygon.
	// owal[],on: input wall list
	//    retwal: output wall list (malloced inside polyspli if not null)
	//(kx,ky,ka): valid half-space test (x*kx + y*ky + ka > 0)
	//   returns: # output walls

	//Split wall list, slope vs. slope (helper function)
int polyspli2 (wall_t *owal, int on, wall_t **retwal, long sec0, long isflor0,
																		long sec1, long isflor1, long dir)
{
	double nx0, ny0, ox0, oy0, oz0, nx1, ny1, ox1, oy1, oz1;
	sect_t *sec = gst->sect;

		//(x-sec[i].wal[0].x)*sec[i].grad[0].x + (y-sec[i].wal[0].y)*sec[i].grad[0].y + (z-sec[i].z[0])*1 > 0 //ceil i
		//(x-sec[i].wal[0].x)*sec[i].grad[1].x + (y-sec[i].wal[0].y)*sec[i].grad[1].y + (z-sec[i].z[1])*1 < 0 //flor i
		//(x-sec[j].wal[0].x)*sec[j].grad[0].x + (y-sec[j].wal[0].y)*sec[j].grad[0].y + (z-sec[j].z[0])*1 > 0 //ceil j
		//(x-sec[j].wal[0].x)*sec[j].grad[1].x + (y-sec[j].wal[0].y)*sec[j].grad[1].y + (z-sec[j].z[1])*1 < 0 //flor j
		//
		//(x-ox0)*nx0 + (y-oy0)*ny0 + (z-oz0) > 0
		//(x-ox1)*nx1 + (y-oy1)*ny1 + (z-oz1) < 0
		//
		//x*(nx0-nx1) + y*(ny0-ny1) + ox1*nx1 + oy1*ny1 + oz1 - ox0*nx0 - oy0*ny0 - oz0 = 0
	nx0 = sec[sec0].grad[isflor0].x; ny0 = sec[sec0].grad[isflor0].y; ox0 = sec[sec0].wall[0].x; oy0 = sec[sec0].wall[0].y; oz0 = sec[sec0].z[isflor0];
	nx1 = sec[sec1].grad[isflor1].x; ny1 = sec[sec1].grad[isflor1].y; ox1 = sec[sec1].wall[0].x; oy1 = sec[sec1].wall[0].y; oz1 = sec[sec1].z[isflor1];
	if (!dir) return(polyspli(owal,on,retwal,nx0-nx1,ny0-ny1,ox1*nx1 + oy1*ny1 + oz1 - ox0*nx0 - oy0*ny0 - oz0));
		  else return(polyspli(owal,on,retwal,nx1-nx0,ny1-ny0,ox0*nx0 + oy0*ny0 + oz0 - ox1*nx1 - oy1*ny1 - oz1));
}

//---------------------------------------------------------------------------------------------

static void freegamestate (gamestate_t *dst)
{
	int i;

#ifdef STANDALONE
	for(i=numplayers-1;i>=0;i--)
		if (dst->p[i].sec.wall) { free(dst->p[i].sec.wall); dst->p[i].sec.wall = 0; }
#endif
	if (dst->sect)
	{
		for(i=dst->numsects-1;i>=0;i--) if (dst->sect[i].wall) free(dst->sect[i].wall);
		free(dst->sect); dst->sect = 0;
	}
	dst->numsects = dst->malsects = 0;

	if (dst->spri) { free(dst->spri); dst->spri = 0; }
	dst->numspris = dst->malspris = 0;
}

void build2_copygamestate (gamestate_t *dst, gamestate_t *src)
{
	sect_t *osecptr;
	wall_t *owalptr, **playsecwals;
	spri_t *osprptr;
	int i, omalsec, omalspr, omalsiz, *playsecwaln;

#ifdef STANDALONE
		//copying entire gamestate_t overwrites too much: remember player's rubber sector's wall ptr&mal
	playsecwals = (wall_t **)_alloca(numplayers*sizeof(playsecwals[0]));
	playsecwaln = (int     *)_alloca(numplayers*sizeof(playsecwaln[0]));
	for(i=0;i<numplayers;i++)
	{
		playsecwals[i] = dst->p[i].sec.wall;
		playsecwaln[i] = dst->p[i].sec.nmax;
	}
#endif

	osecptr = dst->sect; omalsec = dst->malsects;
	osprptr = dst->spri; omalspr = dst->malspris;
	if (src->malsects > omalsec)
	{
		osecptr = (sect_t *)realloc(osecptr,src->malsects*sizeof(sect_t));
		omalsec = src->malsects;
	}
	if (src->malspris > omalspr)
	{
		osprptr = (spri_t *)realloc(osprptr,src->malspris*sizeof(spri_t));
		omalspr = src->malspris;
	}
	memset(&osecptr[dst->numsects],0,(dst->malsects-dst->numsects)*sizeof(sect_t));
	memset(&osprptr[dst->numspris],0,(dst->malspris-dst->numspris)*sizeof(spri_t));
	memcpy(dst,src,sizeof(gamestate_t));
	dst->sect = osecptr; dst->malsects = omalsec;
	dst->spri = osprptr; dst->malspris = omalspr;

#ifdef STANDALONE
	memcpy(dst->spri,src->spri,src->numspris*sizeof(spri_t));
#else
	memcpy(dst->spri,src->spri,src->malspris*sizeof(spri_t));
#endif

	for(i=src->numsects-1;i>=0;i--)
	{
		owalptr = dst->sect[i].wall;
		omalsiz = dst->sect[i].nmax;
		if (src->sect[i].nmax > omalsiz)
		{
			omalsiz = src->sect[i].nmax;
			owalptr = (wall_t *)realloc(owalptr,omalsiz*sizeof(wall_t));
		}
		memcpy(&dst->sect[i],&src->sect[i],sizeof(sect_t));
		dst->sect[i].wall = owalptr;
		dst->sect[i].nmax = omalsiz;
		memcpy(dst->sect[i].wall,src->sect[i].wall,src->sect[i].n*sizeof(wall_t));
	}

#ifdef STANDALONE
	for(i=numplayers-1;i>=0;i--)
	{
		owalptr = playsecwals[i];
		omalsiz = playsecwaln[i];
		if (src->p[i].sec.nmax > omalsiz)
		{
			omalsiz = src->p[i].sec.nmax;
			owalptr = (wall_t *)realloc(owalptr,omalsiz*sizeof(wall_t));
		}
		memcpy(&dst->p[i].sec,&src->p[i].sec,sizeof(sect_t));
		dst->p[i].sec.wall = owalptr;
		dst->p[i].sec.nmax = omalsiz;
		memcpy(dst->p[i].sec.wall,src->p[i].sec.wall,src->p[i].sec.nmax*sizeof(wall_t));
	}
#endif
}

#ifdef STANDALONE

#if (OOS_CHECK != 0)
static long getcrc32 (long crc32, unsigned char *buf, long leng) { long i; for(i=0;i<leng;i++) updatecrc32(buf[i],crc32); return(crc32); }
static long addcrc32 (long crc32, unsigned char *buf, long leng) { totcrcbytes += leng; return(getcrc32(crc32,buf,leng)); }
static long tilnumcrc (long i) { if ((unsigned)i >= (unsigned)gnumtiles) return(-1); return(gtile[i].namcrc32); }
static char *tilnumst (long i) { if ((unsigned)i >= (unsigned)gnumtiles) return(""); return(gtile[i].filnam);   }

	//07/05/2007: totcrcbytes at startup is: 694*numplayers + 82(num*,start*,rseed,chatmessn,"cloud.png")
long gamestate_crc32 (gamestate_t *gs) //Called by OOS_CHECK
{
	playerstruct_t lps;
	sect_t lsec;
	wall_t lwal;
	spri_t lspr;
	long i, j, crc32;
	char *cptr;

	crc32 = 0; totcrcbytes = 0;

	crc32 = addcrc32(crc32,(unsigned char *)&numplayers,sizeof(numplayers));

	crc32 = addcrc32(crc32,(unsigned char *)&gs->startpos,sizeof(gs->startpos));
	crc32 = addcrc32(crc32,(unsigned char *)&gs->startrig,sizeof(gs->startrig));
	crc32 = addcrc32(crc32,(unsigned char *)&gs->startdow,sizeof(gs->startdow));
	crc32 = addcrc32(crc32,(unsigned char *)&gs->startfor,sizeof(gs->startfor));
	crc32 = addcrc32(crc32,(unsigned char *)&gs->numsects,sizeof(gs->numsects));
	crc32 = addcrc32(crc32,(unsigned char *)&gs->numspris,sizeof(gs->numspris));
	crc32 = addcrc32(crc32,(unsigned char *)&gs->blankheadspri,sizeof(gs->blankheadspri));

	crc32 = addcrc32(crc32,(unsigned char *)&gs->light_sprinum,sizeof(gs->light_sprinum));
	crc32 = addcrc32(crc32,(unsigned char *)&gs->light_spri,sizeof(gs->light_spri[0])*gs->light_sprinum);

	for(i=0;i<numplayers;i++)
	{
		lps = gs->p[i];
		for(j=lps.sec.n-1;j>=0;j--)
		{
			lwal = lps.sec.wall[j]; lwal.xsurf = 0;
			crc32 = addcrc32(crc32,(unsigned char *)&lwal,sizeof(wall_t));
		}
		lps.sec.wall = 0;
		lps.copysurf[0].tilnum = tilnumcrc(lps.copysurf[0].tilnum);
		lps.copyspri[0].tilnum = tilnumcrc(lps.copyspri[0].tilnum);
		crc32 = addcrc32(crc32,(unsigned char *)&lps,sizeof(playerstruct_t));

		crc32 = addcrc32(crc32,(unsigned char *)&gs->typemess[i],strlen(gs->typemess[i])+1);
		crc32 = addcrc32(crc32,(unsigned char *)&gs->nick[i],strlen(gs->nick[i])+1);
	}
	crc32 = addcrc32(crc32,(unsigned char *)&gs->rseed,sizeof(gs->rseed));
	crc32 = addcrc32(crc32,(unsigned char *)&gs->chatmessn,sizeof(gs->chatmessn));
	for(j=gs->chatmessn-1;j>=max(gs->chatmessn-TYPEMESSNUM,0);j--)
	{
		crc32 = addcrc32(crc32,(unsigned char *)&gs->chatmessowner[j&(TYPEMESSNUM-1)],sizeof(gs->chatmessowner[0]));
		cptr = &gs->chatmess[j&(TYPEMESSNUM-1)][0];
		crc32 = addcrc32(crc32,(unsigned char *)cptr,strlen(cptr)+1);
	}

	for(i=gs->numsects-1;i>=0;i--)
	{
		for(j=gs->sect[i].n-1;j>=0;j--)
		{
			lwal = gs->sect[i].wall[j]; lwal.xsurf = 0;
			lwal.surf.tilnum = tilnumcrc(lwal.surf.tilnum);
			crc32 = addcrc32(crc32,(unsigned char *)&lwal,sizeof(wall_t));
		}

		lsec = gs->sect[i];
		lsec.wall = 0;
		lsec.surf[0].tilnum = tilnumcrc(lsec.surf[0].tilnum);
		lsec.surf[1].tilnum = tilnumcrc(lsec.surf[1].tilnum);
		crc32 = addcrc32(crc32,(unsigned char *)&lsec,sizeof(sect_t));
	}

	for(i=gs->numspris-1;i>=0;i--)
	{
		lspr = gs->spri[i];
		lspr.tilnum = tilnumcrc(lspr.tilnum);
		crc32 = addcrc32(crc32,(unsigned char *)&lspr,sizeof(spri_t));
	}

	return(crc32);
}

void gamestate_save (char *filnam, gamestate_t *gs)
{
	FILE *fil;
	playerstruct_t lps;
	sect_t lsec;
	wall_t lwal;
	long i, j;
	char *cptr;

	fil = fopen(filnam,"wb"); if (!fil) return;
	fprintf(fil,"numplayers=%d\n",numplayers);
	fprintf(fil,"startpos=%f,%f,%f\n",gs->startpos.x,gs->startpos.y,gs->startpos.z);
	fprintf(fil,"startrig=%f,%f,%f\n",gs->startrig.x,gs->startrig.y,gs->startrig.z);
	fprintf(fil,"startdow=%f,%f,%f\n",gs->startdow.x,gs->startdow.y,gs->startdow.z);
	fprintf(fil,"startfor=%f,%f,%f\n",gs->startfor.x,gs->startfor.y,gs->startfor.z);
	fprintf(fil,"numsects=%d\n",gs->numsects);
	fprintf(fil,"numspris=%d\n",gs->numspris);
	fprintf(fil,"blankheadspri=%d\n",gs->blankheadspri);

	fprintf(fil,"light_sprinum=%d\n",gs->light_sprinum);
	for(i=0;i<gs->light_sprinum;i++) fprintf(fil,"light_spri[%d]=%d\n",i,gs->light_spri[i]);

	for(i=0;i<numplayers;i++)
	{
		lps = gs->p[i];
		for(j=0;j<lps.sec.n;j++)
		{
			lwal = lps.sec.wall[j]; lwal.xsurf = 0;
			fprintf(fil,"wal[play%d][%d].x,y,n,ns,nw=%f,%f,%d,%d,%d\n",i,j,lwal.x,lwal.y,lwal.n,lwal.ns,lwal.nw);
			fprintf(fil,"wal[play%d][%d].owner,surfn=%d,%d\n",i,j,lwal.owner,lwal.surfn);
			fprintf(fil,"wal[play%d][%d].tilnumst,tilanm,flags,tag=%s,%d,%d,%d\n",i,j,tilnumst(lwal.surf.tilnum),lwal.surf.tilanm,lwal.surf.flags,lwal.surf.tag);
			fprintf(fil,"wal[play%d][%d].uv=%f,%f,%f,%f,%f,%f\n",i,j,lwal.surf.uv[0].x,lwal.surf.uv[0].y,lwal.surf.uv[1].x,lwal.surf.uv[1].y,lwal.surf.uv[2].x,lwal.surf.uv[2].y);
			fprintf(fil,"wal[play%d][%d].argb=%d,%d,%d,%d\n",i,j,lwal.surf.asc,lwal.surf.rsc,lwal.surf.gsc,lwal.surf.bsc);
		}
		lps.sec.wall = 0;

		fprintf(fil,"ps[%d].xres,yres,fullscreen=%d,%d,%d\n",i,lps.xres,lps.yres,lps.fullscreen);
		fprintf(fil,"ps[%d].ghx,ghy,ghz,zoom,ozoom=%f,%f,%f,%f,%f\n",i,lps.ghx,lps.ghy,lps.ghz,lps.zoom,lps.ozoom);
		fprintf(fil,"ps[%d].ipos=%f,%f,%f\n",i,lps.ipos.x,lps.ipos.y,lps.ipos.z);
		fprintf(fil,"ps[%d].irig=%f,%f,%f\n",i,lps.irig.x,lps.irig.y,lps.irig.z);
		fprintf(fil,"ps[%d].idow=%f,%f,%f\n",i,lps.idow.x,lps.idow.y,lps.idow.z);
		fprintf(fil,"ps[%d].ifor=%f,%f,%f\n",i,lps.ifor.x,lps.ifor.y,lps.ifor.z);
		fprintf(fil,"ps[%d].npos=%f,%f,%f\n",i,lps.npos.x,lps.npos.y,lps.npos.z);
		fprintf(fil,"ps[%d].nrig=%f,%f,%f\n",i,lps.nrig.x,lps.nrig.y,lps.nrig.z);
		fprintf(fil,"ps[%d].ndow=%f,%f,%f\n",i,lps.ndow.x,lps.ndow.y,lps.ndow.z);
		fprintf(fil,"ps[%d].nfor=%f,%f,%f\n",i,lps.nfor.x,lps.nfor.y,lps.nfor.z);
		fprintf(fil,"ps[%d].grdc=%f,%f,%f\n",i,lps.grdc.x,lps.grdc.y,lps.grdc.z);
		fprintf(fil,"ps[%d].grdu=%f,%f,%f\n",i,lps.grdu.x,lps.grdu.y,lps.grdu.z);
		fprintf(fil,"ps[%d].grdv=%f,%f,%f\n",i,lps.grdv.x,lps.grdv.y,lps.grdv.z);
		fprintf(fil,"ps[%d].grdn=%f,%f,%f\n",i,lps.grdn.x,lps.grdn.y,lps.grdn.z);
		fprintf(fil,"ps[%d].dgridlock,compact2d,goalheight2d,height2d=%f,%f,%f,%f\n",i,lps.dgridlock,lps.compact2d,lps.goalheight2d,lps.height2d);
		fprintf(fil,"ps[%d].cursect,rendheight,rendinterp,editmode=%d,%d,%d,%d\n",i,lps.cursect,lps.rendheight,lps.rendinterp,lps.editmode);
		fprintf(fil,"ps[%d].fcmousx,fcmousy,bstatus,obstatus=%f,%f,%d,%d\n",i,lps.fcmousx,lps.fcmousy,lps.bstatus,lps.obstatus);
		fprintf(fil,"ps[%d].keys=%08x,%08x,%08x,%08x,%08x,%08x,%08x,%08x\n",i,
			*(long *)&lps.skeystatus[0],*(long *)&lps.skeystatus[4],*(long *)&lps.skeystatus[8],*(long *)&lps.skeystatus[12],
			*(long *)&lps.skeystatus[16],*(long *)&lps.skeystatus[20],*(long *)&lps.skeystatus[24],*(long *)&lps.skeystatus[28]);
		fprintf(fil,"ps[%d].playerindex,showdebug,showedges3d,grabmode,grabsect,grabwall,grabcf,circnum=%d,%d,%d,%d,%d,%d,%d,%d\n",i,
			lps.playerindex,lps.showdebug,lps.showedges3d,lps.grabmode,lps.grabsect,lps.grabwall,lps.grabcf,lps.circnum);
		fprintf(fil,"ps[%d].gridlock,boolfunc,gotcopy,docollide=%d,%d,%d,%d\n",i,lps.gridlock,lps.boolfunc,lps.gotcopy,lps.docollide);
		fprintf(fil,"ps[%d].selrgbbak=%d,%d,%d,%d\n",i,lps.selrgbbak[0],lps.selrgbbak[1],lps.selrgbbak[2],lps.selrgbbak[3]);
		fprintf(fil,"ps[%d].selrgbrad,selrgbintens=%f,%f\n",i,lps.selrgbrad,lps.selrgbintens);
		fprintf(fil,"ps[%d].selcorn=%f,%f\n",i,lps.selcorn.x,lps.selcorn.y);

		fprintf(fil,"ps[%d].copysurf.tilnumst,tilanm,flags,tag=%s,%d,%d,%d\n",i,tilnumst(lps.copysurf[0].tilnum),lps.copysurf[0].tilanm,lps.copysurf[0].flags,lps.copysurf[0].tag);
		fprintf(fil,"ps[%d].copysurf.uv=%f,%f,%f,%f,%f,%f\n",i,lps.copysurf[0].uv[0].x,lps.copysurf[0].uv[0].y,lps.copysurf[0].uv[1].x,lps.copysurf[0].uv[1].y,lps.copysurf[0].uv[2].x,lps.copysurf[0].uv[2].y);
		fprintf(fil,"ps[%d].copysurf.argb=%d,%d,%d,%d\n",i,lps.copysurf[0].asc,lps.copysurf[0].rsc,lps.copysurf[0].gsc,lps.copysurf[0].bsc);

		fprintf(fil,"ps[%d].copyspri.p=%f,%f,%f\n",i,lps.copyspri[0].p.x,lps.copyspri[0].p.y,lps.copyspri[0].p.z);
		fprintf(fil,"ps[%d].copyspri.r=%f,%f,%f\n",i,lps.copyspri[0].r.x,lps.copyspri[0].r.y,lps.copyspri[0].r.z);
		fprintf(fil,"ps[%d].copyspri.d=%f,%f,%f\n",i,lps.copyspri[0].d.x,lps.copyspri[0].d.y,lps.copyspri[0].d.z);
		fprintf(fil,"ps[%d].copyspri.f=%f,%f,%f\n",i,lps.copyspri[0].f.x,lps.copyspri[0].f.y,lps.copyspri[0].f.z);
		fprintf(fil,"ps[%d].copyspri.v=%f,%f,%f\n",i,lps.copyspri[0].v.x,lps.copyspri[0].v.y,lps.copyspri[0].v.z);
		fprintf(fil,"ps[%d].copyspri.av=%f,%f,%f\n",i,lps.copyspri[0].av.x,lps.copyspri[0].av.y,lps.copyspri[0].av.z);
		fprintf(fil,"ps[%d].copyspri.fat,mas,moi=%f,%f,%f\n",i,lps.copyspri[0].fat,lps.copyspri[0].mas,lps.copyspri[0].moi);
		fprintf(fil,"ps[%d].copyspri.tilnumst=%s\n",i,tilnumst(lps.copyspri[0].tilnum));
		fprintf(fil,"ps[%d].copyspri.argb=%d,%d,%d,%d\n",i,lps.copyspri[0].asc,lps.copyspri[0].rsc,lps.copyspri[0].gsc,lps.copyspri[0].bsc);
		fprintf(fil,"ps[%d].copyspri.owner,tag=%d,%d\n",i,lps.copyspri[0].owner,lps.copyspri[0].tag);
		fprintf(fil,"ps[%d].copyspri.tim,otim,flags=%d,%d,%d\n",i,lps.copyspri[0].tim,lps.copyspri[0].otim,lps.copyspri[0].flags);
		fprintf(fil,"ps[%d].copyspri.sect,sectn,sectp=%d,%d,%d\n",i,lps.copyspri[0].sect,lps.copyspri[0].sectn,lps.copyspri[0].sectp);

		fprintf(fil,"ps[%d].startstate,startsect,startwall=%d,%d,%d\n",i,lps.startstate,lps.startsect,lps.startwall);
		fprintf(fil,"ps[%d].typemode,typecurs,typehighlight,typeowritemode=%d,%d,%d,%d\n",i,lps.typemode,lps.typecurs,lps.typehighlight,lps.typeowritemode);
		fprintf(fil,"ps[%d].emoticon_hair,eyes,nose,mouth=%d,%d,%d,%d\n",i,lps.emoticon_hair,lps.emoticon_eyes,lps.emoticon_nose,lps.emoticon_mouth);

		fprintf(fil,"ps[%d].flashlighton=%d\n",i,lps.flashlighton);

		fprintf(fil,"typemess[%d]=|%s|\n",i,gs->typemess[i]);
		fprintf(fil,"nick[%d]=|%s|\n",i,gs->nick[i]);
	}
	fprintf(fil,"rseed=%d\n",gs->rseed);
	fprintf(fil,"chatmessn=%d\n",gs->chatmessn);
	for(j=max(gs->chatmessn-TYPEMESSNUM,0);j<gs->chatmessn;j++)
	{
		fprintf(fil,"chatmessowner[%d]=%d\n",j,gs->chatmessowner[j&(TYPEMESSNUM-1)]);
		fprintf(fil,"chatmess[%d]=|%s|\n",j,&gs->chatmess[j&(TYPEMESSNUM-1)][0]);
	}

	for(i=0;i<gs->numsects;i++)
	{
		for(j=0;j<gs->sect[i].n;j++)
		{
			lwal = gs->sect[i].wall[j]; lwal.xsurf = 0;
			fprintf(fil,"wal[sec%d][%d].x,y,n,ns,nw=%f,%f,%d,%d,%d\n",i,j,lwal.x,lwal.y,lwal.n,lwal.ns,lwal.nw);
			fprintf(fil,"wal[sec%d][%d].owner,surfn=%d,%d\n",i,j,lwal.owner,lwal.surfn);
			fprintf(fil,"wal[sec%d][%d].tilnumst,tilanm,flags,tag=%s,%d,%d,%d\n",i,j,tilnumst(lwal.surf.tilnum),lwal.surf.tilanm,lwal.surf.flags,lwal.surf.tag);
			fprintf(fil,"wal[sec%d][%d].uv=%f,%f,%f,%f,%f,%f\n",i,j,lwal.surf.uv[0].x,lwal.surf.uv[0].y,lwal.surf.uv[1].x,lwal.surf.uv[1].y,lwal.surf.uv[2].x,lwal.surf.uv[2].y);
			fprintf(fil,"wal[sec%d][%d].argb=%d,%d,%d,%d\n",i,j,lwal.surf.asc,lwal.surf.rsc,lwal.surf.gsc,lwal.surf.bsc);
		}

		lsec = gs->sect[i];
		lsec.wall = 0;

		fprintf(fil,"sec[%d].bound=%d,%d,%d,%d\n",i,lsec.minx,lsec.miny,lsec.maxx,lsec.maxy);
		fprintf(fil,"sec[%d].z=%f,%f\n",i,lsec.z[0],lsec.z[1]);
		fprintf(fil,"sec[%d].grad=%f,%f,%f,%f\n",i,lsec.grad[0].x,lsec.grad[0].y,lsec.grad[1].x,lsec.grad[1].y);
		for(j=0;j<2;j++)
		{
			fprintf(fil,"sec[%d].surf[%d].tilnumst,tilanm,flags,tag=%s,%d,%d,%d\n",i,j,tilnumst(lsec.surf[j].tilnum),lsec.surf[j].tilanm,lsec.surf[j].flags,lsec.surf[j].tag);
			fprintf(fil,"sec[%d].surf[%d].uv=%f,%f,%f,%f,%f,%f\n",i,j,lsec.surf[j].uv[0].x,lsec.surf[j].uv[0].y,lsec.surf[j].uv[1].x,lsec.surf[j].uv[1].y,lsec.surf[j].uv[2].x,lsec.surf[j].uv[2].y);
			fprintf(fil,"sec[%d].surf[%d].argb=%d,%d,%d,%d\n",i,j,lsec.surf[j].asc,lsec.surf[j].rsc,lsec.surf[j].gsc,lsec.surf[j].bsc);
		}
		fprintf(fil,"sec[%d].n,nmax,headspri,foglev,owner=%d,%d,%d,%d,%d\n",i,lsec.n,lsec.nmax,lsec.headspri,lsec.foglev,lsec.owner);
	}
	for(i=0;i<gs->numspris;i++)
	{
		fprintf(fil,"spr[%d].p=%f,%f,%f\n",i,gs->spri[i].p.x,gs->spri[i].p.y,gs->spri[i].p.z);
		fprintf(fil,"spr[%d].r=%f,%f,%f\n",i,gs->spri[i].r.x,gs->spri[i].r.y,gs->spri[i].r.z);
		fprintf(fil,"spr[%d].d=%f,%f,%f\n",i,gs->spri[i].d.x,gs->spri[i].d.y,gs->spri[i].d.z);
		fprintf(fil,"spr[%d].f=%f,%f,%f\n",i,gs->spri[i].f.x,gs->spri[i].f.y,gs->spri[i].f.z);
		fprintf(fil,"spr[%d].v=%f,%f,%f\n",i,gs->spri[i].v.x,gs->spri[i].v.y,gs->spri[i].v.z);
		fprintf(fil,"spr[%d].av=%f,%f,%f\n",i,gs->spri[i].av.x,gs->spri[i].av.y,gs->spri[i].av.z);
		fprintf(fil,"spr[%d].fat,mas,moi=%f,%f,%f\n",i,gs->spri[i].fat,gs->spri[i].mas,gs->spri[i].moi);
		fprintf(fil,"spr[%d].tilnumst=%s\n",i,tilnumst(gs->spri[i].tilnum));
		fprintf(fil,"spr[%d].argb=%d,%d,%d,%d\n",i,gs->spri[i].asc,gs->spri[i].rsc,gs->spri[i].gsc,gs->spri[i].bsc);
		fprintf(fil,"spr[%d].owner,tag=%d,%d\n",i,gs->spri[i].owner,gs->spri[i].tag);
		fprintf(fil,"spr[%d].tim,otim,flags=%d,%d,%d\n",i,gs->spri[i].tim,gs->spri[i].otim,gs->spri[i].flags);
		fprintf(fil,"spr[%d].sect,sectn,sectp=%d,%d,%d\n",i,gs->spri[i].sect,gs->spri[i].sectn,gs->spri[i].sectp);
	}

	fclose(fil);
}
#endif
#endif

static void compacttilelist_tilenums (void) //uses gtile[?].namcrc32 as the lut - a complete hack to avoid extra allocs :P
{
	sect_t *sec;
	long s, w;

	sec = gst->sect;
	for(s=gst->numsects-1;s>=0;s--)
	{
		for(w=2-1;w>=0;w--)        { sec[s].surf[w].tilnum      = gtile[sec[s].surf[w].tilnum].namcrc32; }
		for(w=sec[s].n-1;w>=0;w--) { sec[s].wall[w].surf.tilnum = gtile[sec[s].wall[w].surf.tilnum].namcrc32; }
#ifndef STANDALONE
		for(w=sec[s].headspri;w>=0;w=gst->spri[w].sectn)
			if (gst->spri[w].tilnum >= 0) gst->spri[w].tilnum = gtile[gst->spri[w].tilnum].namcrc32;
	}
#else
	}
	for(w=gst->numspris-1;w>=0;w--)
		if ((unsigned)gst->spri[w].tilnum < (unsigned)gnumtiles) gst->spri[w].tilnum = gtile[gst->spri[w].tilnum].namcrc32;
	for(w=0;w<numplayers;w++)
	{
		if ((unsigned)gst->p[w].copysurf[0].tilnum < (unsigned)gnumtiles) gst->p[w].copysurf[0].tilnum = gtile[gst->p[w].copysurf[0].tilnum].namcrc32;
		if ((unsigned)gst->p[w].copyspri[0].tilnum < (unsigned)gnumtiles) gst->p[w].copyspri[0].tilnum = gtile[gst->p[w].copyspri[0].tilnum].namcrc32;
	}
#endif
}
static void compacttilelist (long flags)
{
	sect_t *sec;
	long i, j, s, w, nnumtiles;

	sec = gst->sect;

	//gtile[?].namcrc32 used as temp in this function (must be reconstructed before returning)

	if (flags&1) //Remove duplicate filenames (call right after load with alt+sectors copied)
	{
		for(i=gnumtiles-1;i>=0;i--) gtile[i].namcrc32 = i;
		for(s=0;s<sizeof(gtilehashead)/sizeof(gtilehashead[0]);s++)
			for(i=gtilehashead[s];i>=0;i=gtile[i].hashnext) //n^2 compare on linked list
			{
				if (!gtile[i].filnam[0]) continue;
				for(j=gtile[i].hashnext;j>=0;j=gtile[j].hashnext)
					if (!stricmp(gtile[i].filnam,gtile[j].filnam))
					{
						if (gtile[j].tt.f) { free((void *)gtile[j].tt.f); gtile[j].tt.f = 0; }
						gtile[j].filnam[0] = 0;
						gtile[j].namcrc32 = i;
					}
			}
		compacttilelist_tilenums();

		nnumtiles = 0;
		for(i=0;i<gnumtiles;i++)
		{
			if (!gtile[i].filnam[0]) continue;
			j = gtile[nnumtiles].namcrc32; gtile[nnumtiles] = gtile[i]; gtile[nnumtiles].namcrc32 = j; //copy all except namcrc32
			gtile[i].namcrc32 = nnumtiles; nnumtiles++;
		}
		if (nnumtiles != gnumtiles) { compacttilelist_tilenums(); gnumtiles = nnumtiles; }
	}

	if (flags&2) //Remove unused tiles (call just before save)
	{
		for(i=0;i<gnumtiles;i++) gtile[i].namcrc32 = 0;
		gtile[0].namcrc32 = 1; //Keep default tile (cloud.png)
		for(s=gst->numsects-1;s>=0;s--)
		{
			for(w=2-1;w>=0;w--) gtile[sec[s].surf[w].tilnum].namcrc32 = 1;
			for(w=sec[s].n-1;w>=0;w--) gtile[sec[s].wall[w].surf.tilnum].namcrc32 = 1;
#ifndef STANDALONE
			for(w=sec[s].headspri;w>=0;w=gst->spri[w].sectn)
				if (gst->spri[w].tilnum >= 0) gtile[gst->spri[w].tilnum].namcrc32 = 1;
		}
#else
		}
		for(w=gst->numspris-1;w>=0;w--)
			if ((unsigned)gst->spri[w].tilnum < (unsigned)gnumtiles)
				gtile[gst->spri[w].tilnum].namcrc32 = 1;
#endif
		nnumtiles = 0;
		for(i=0;i<gnumtiles;i++)
		{
			if (!gtile[i].namcrc32) continue;
			j = gtile[nnumtiles].namcrc32; gtile[nnumtiles] = gtile[i]; gtile[nnumtiles].namcrc32 = j; //copy all except namcrc32
			gtile[i].namcrc32 = nnumtiles; nnumtiles++;
		}
		if (nnumtiles != gnumtiles) { compacttilelist_tilenums(); gnumtiles = nnumtiles; }
	}

	if (flags&3) //Reconstruct namcrc32's and hash table from scratch
	{
		memset(gtilehashead,-1,sizeof(gtilehashead));
		for(i=0;i<gnumtiles;i++)
		{
			gtile[i].namcrc32 = getcrc32z(0,(unsigned char *)gtile[i].filnam);
			j = (gtile[i].namcrc32&(sizeof(gtilehashead)/sizeof(gtilehashead[0])-1));
			gtile[i].hashnext = gtilehashead[j]; gtilehashead[j] = i;
		}
	}
}

static int arewallstouching (int s0, int w0, int s1, int w1)
{
	sect_t *sec;
	float x[4], y[4];
	int i;

	sec = gst->sect;

	x[0] = sec[s0].wall[w0].x; y[0] = sec[s0].wall[w0].y; i = sec[s0].wall[w0].n+w0;
	x[1] = sec[s0].wall[i ].x; y[1] = sec[s0].wall[i ].y;
	x[2] = sec[s1].wall[w1].x; y[2] = sec[s1].wall[w1].y; i = sec[s1].wall[w1].n+w1;
	x[3] = sec[s1].wall[i ].x; y[3] = sec[s1].wall[i ].y;

		//Make sure x's & y's match (front or back)
	if ((x[0] == x[2]) && (y[0] == y[2])) { if ((x[1] != x[3]) || (y[1] != y[3])) return(0); }
	else { if ((x[0] != x[3]) || (y[0] != y[3]) || (x[1] != x[2]) || (y[1] != y[2])) return(0); }

		//Connect walls only if their z's cross
	for(i=1;i>=0;i--)
		if (max(getslopez(&sec[s0],0,x[i],y[i]),getslopez(&sec[s1],0,x[i],y[i])) <=
			 min(getslopez(&sec[s0],1,x[i],y[i]),getslopez(&sec[s1],1,x[i],y[i]))) return(1);

	return(0);
}

void checknextwalls (void)
{
#if 0
	sect_t *sec;
	float f, x0, y0, x1, y1;
	int s0, w0, w0n, s1, w1, w1n;

	sec = gst->sect;

		//Clear all nextsect/nextwalls
	for(s0=0;s0<gst->numsects;s0++)
		for(w0=0;w0<sec[s0].n;w0++) sec[s0].wall[w0].ns = sec[s0].wall[w0].nw = -1;

	for(s1=1;s1<gst->numsects;s1++)
		for(w1=0;w1<sec[s1].n;w1++)
		{
			x0 = sec[s1].wall[w1].x;  y0 = sec[s1].wall[w1].y; w1n = sec[s1].wall[w1].n+w1;
			x1 = sec[s1].wall[w1n].x; y1 = sec[s1].wall[w1n].y;
			for(s0=0;s0<s1;s0++)
				for(w0=0;w0<sec[s0].n;w0++)
					if ((sec[s0].wall[w0].x == x1) && (sec[s0].wall[w0].y == y1))
					{
						w0n = sec[s0].wall[w0].n+w0;
						if ((sec[s0].wall[w0n].x == x0) && (sec[s0].wall[w0n].y == y0))
						{
							sec[s1].wall[w1].ns = s0; //FIX: obsolete: doesn't support SOS
							sec[s1].wall[w1].nw = w0;
							sec[s0].wall[w0].ns = s1;
							sec[s0].wall[w0].nw = w1;
							goto cnw_break2;
						}
					}
cnw_break2:;
		}
#else
	typedef struct { int w, s; float minpt; } cvertlist_t;
	cvertlist_t *hashead, *hashlist, *subhashlist, vertemp;
	int *hashsiz;
	sect_t *sec;
	float f; //WARNING: keep f float for hash trick!
	float x0, y0, x1, y1, fz[8];
	int i, j, k, m, n, r, w, s0, w0, w0n, s1, w1, w1n, s2, w2, lhsiz, hsiz, numwalls, maxchainleng;
	int gap, z, zz, subn;

	sec = gst->sect;

	for(s0=0,numwalls=0;s0<gst->numsects;s0++) numwalls += sec[s0].n;

	for(lhsiz=4,hsiz=(1<<lhsiz);(hsiz<<1)<numwalls;lhsiz++,hsiz<<=1); //hsiz = 0.5x to 1.0x of numwalls
	hashead = (cvertlist_t *)_alloca(hsiz*sizeof(hashead[0])); memset(hashead,-1,hsiz*sizeof(hashead[0]));
	hashsiz = (int         *)_alloca(hsiz*sizeof(hashsiz[0])); memset(hashsiz, 0,hsiz*sizeof(hashsiz[0]));

	maxchainleng = 0;
	for(s0=0;s0<gst->numsects;s0++)
		for(w0=0;w0<sec[s0].n;w0++)
		{
			i = 0; w0n = sec[s0].wall[w0].n+w0;
				//Hash must give same values if w0 and w0n are swapped (commutativity)
			f = sec[s0].wall[w0].x*sec[s0].wall[w0n].x + sec[s0].wall[w0].y*sec[s0].wall[w0n].y;
			k = *(long *)&f;
			//k ^= (*(long *)&sec[s0].wall[w0].x) ^ (*(long *)&sec[s0].wall[w0n].x);
			//k ^= (*(long *)&sec[s0].wall[w0].y) ^ (*(long *)&sec[s0].wall[w0n].y);
			for(j=lhsiz;j<32;j+=lhsiz) i -= (k>>j);
			i &= (hsiz-1);

			sec[s0].wall[w0].ns = hashead[i].s; hashead[i].s = s0;
			sec[s0].wall[w0].nw = hashead[i].w; hashead[i].w = w0;
			hashsiz[i]++; if (hashsiz[i] > maxchainleng) maxchainleng = hashsiz[i];
		}

	//hashhead -> s0w0 -> s1w1 -> s2w2 -> s3w3 -> s4w4 -> -1
	//              A       B       A               B

	hashlist = (cvertlist_t *)_alloca(maxchainleng*sizeof(hashlist[0]));

	//printf("maxchainleng=%d\n",maxchainleng); //FIX

	for(i=0;i<hsiz;i++)
	{
		n = 0;
		s0 = hashead[i].s;
		w0 = hashead[i].w;
		while (s0 >= 0)
		{
			hashlist[n].s = s0;
			hashlist[n].w = w0;

				//for 2nd-level hash!
			w0n = sec[s0].wall[w0].n+w0; x0 = sec[s0].wall[w0].x; x1 = sec[s0].wall[w0n].x;
			if (x0 != x1) hashlist[n].minpt = min(x0,x1);
						else hashlist[n].minpt = min(sec[s0].wall[w0].y,sec[s0].wall[w0n].y);
			n++;

				//Easier to join chains if inited as pointing to self rather than -1
			s1 = sec[s0].wall[w0].ns; sec[s0].wall[w0].ns = s0;
			w1 = sec[s0].wall[w0].nw; sec[s0].wall[w0].nw = w0;
			s0 = s1; w0 = w1;
		}

		if (n >= 2)
		{
				//Sort points by y's
			for(gap=(n>>1);gap;gap>>=1)
				for(z=0;z<n-gap;z++)
					for(zz=z;zz>=0;zz-=gap)
					{
						if (hashlist[zz].minpt <= hashlist[zz+gap].minpt) break;
						vertemp = hashlist[zz]; hashlist[zz] = hashlist[zz+gap]; hashlist[zz+gap] = vertemp;
					}

			//printf("//n=%d\n",n); //FIX

			for(zz=n,z=n-1;z>=0;z--)
			{
				if ((z) && (hashlist[z-1].minpt == hashlist[z].minpt)) continue;
				subhashlist = &hashlist[z]; subn = zz-z; zz = z;

					//Example: (sector walls overlapping, drawn sideways)
					//   AAA EEE DDD
					//     BBB CCC
					//                                  0    1    2    3    4
					//j=?,w=4,r=5,n=5, s0:?, s1:?, {A->A,B->B,C->C,D->D,E->E}
					//j=?,w=4,r=4,n=5, s0:E, s1:?, {A->A,B->B,C->C,D->D,E->E}
					//j=3,w=4,r=4,n=5, s0:E, s1:D, {A->A,B->B,C->C,D->D,E->E}
					//j=2,w=3,r=4,n=5, s0:E, s1:C, {A->A,B->B,D->D|C->E,E->C}
					//j=1,w=2,r=4,n=5, s0:E, s1:B, {A->A,D->D|B->C,C->E,E->B}
					//j=0,w=2,r=4,n=5, s0:E, s1:A, {A->A,D->D|B->C,C->E,E->B}
					//j=1,w=1,r=3,n=5, s0:C, s1:D, {A->A|D->E,B->C,C->D,E->B}
					//j=0,w=1,r=3,n=5, s0:C, s1:A, {A->A|D->E,B->C,C->D,E->B}
					//j=0,w=0,r=2,n=5, s0:B, s1:A, {A->C,D->E,B->A,C->D,E->B}

					//     s0    s1  s3  s2
					//  +-------+---+---+---+
					//  |   1   | 7 | D | 8 |
					//  |0     2|6 4|C E|B 9|
					//  |   3   | 5 | F | A |
					//  +-------+---+---+---+
					//                           s0,w0  s1,w1
					//cmp: i=5 ,j=1,w=2,r=2,n=3   0,1,   2,3  no
					//cmp: i=5 ,j=0,w=2,r=2,n=3   0,1,   3,2  no
					//cmp: i=5 ,j=0,w=1,r=1,n=3   2,3,   3,2  yes

					//cmp: i=6 ,j=0,w=1,r=1,n=2   0,0,   3,1  no

					//cmp: i=8 ,j=1,w=2,r=2,n=3   0,2,   1,2  yes
					//cmp: i=8 ,j=0,w=1,r=2,n=3   0,2,   2,2  no
					//cmp: i=8 ,j=0,w=1,r=1,n=3   1,2,   2,2  no

					//cmp: i=10,j=0,w=1,r=1,n=2   1,0,   3,0  yes

					//Graph search and connect: fifo is hashlist itself
					//      (write) (read) (total)
					//  0      w      r      n
					//   (left) (fifo) (done)
				//printf("//i=%d:n=%d\n",i,subn); //FIX
					//FIX
				//for(j=0;j<subn;j++)
				//{
				//   s0 = subhashlist[j].s;
				//   w0 = subhashlist[j].w;
				//   x0 = sec[s0].wall[w0].x; x1 = sec[s0].wall[w0n].x; w0n = sec[s0].wall[w0].n+w0;
				//   y0 = sec[s0].wall[w0].y; y1 = sec[s0].wall[w0n].y;
				//   printf("   %2d: %6.1f %6.1f %6.1f %6.1f | %6.1f\n",j,x0,y0,x1,y1,subhashlist[j].minpt);
				//}

				w = subn-1; r = subn;
				while (w > 0)
				{
					r--;
					s0 = subhashlist[r].s;
					w0 = subhashlist[r].w;
					for(j=w-1;j>=0;j--)
					{
						s1 = subhashlist[j].s; if (s0 == s1) continue; //Don't allow 2-vertex loops to become red lines
						w1 = subhashlist[j].w;
						//printf("//   cmp: j=%2d,w=%2d,r=%2d, %3d,%3d, %3d,%3d, ",j,w,r,s0,w0,s1,w1); //FIX
						if (!arewallstouching(s0,w0,s1,w1)) { /*printf("no\n");FIX*/ continue; }
						//printf("yes\n"); //FIX

						s2 = sec[s0].wall[w0].ns;
						w2 = sec[s0].wall[w0].nw;
							  sec[s0].wall[w0].ns = sec[s1].wall[w1].ns;
							  sec[s0].wall[w0].nw = sec[s1].wall[w1].nw;
															sec[s1].wall[w1].ns = s2;
															sec[s1].wall[w1].nw = w2;
						w--; if (w == j) continue;
						vertemp = subhashlist[w];
									 subhashlist[w] = subhashlist[j];
															subhashlist[j] = vertemp;
					}
					if (r == w) w--;
				}
			}
		}

			//convert disjoint walls (self-linked) back to -1's
		for(j=n-1;j>=0;j--)
		{
			s0 = hashlist[j].s; w0 = hashlist[j].w;
			if ((sec[s0].wall[w0].ns == s0) && (sec[s0].wall[w0].nw == w0))
				{ sec[s0].wall[w0].ns = sec[s0].wall[w0].nw = -1; }
		}
	}
#endif
	shadowtest2_updatelighting = 1;
}

	//hitsect must be >= 0 if it hit something
	//hitwall == -2: ceiling
	//hitwall == -1: floor
	//hitwall >= 0: wall_index
	//hitwall >= 0x40000000: sprite_index+0x40000000
int hitscan (point3d *p0, point3d *pv, float vscale, int cursect, int *hitsect, int *hitwall, point3d *hit)
{
	sect_t *sec;
	wall_t *wal, *wal2;
	spri_t *spr;
	float d, t, u, v, x, y, z, z0, z1, bestt;
	long *gotsect;
	int i, s, w, nw, bs, bw, passthru, *secfif, secfifw, secfifr;

	i = (((gst->numsects+31)>>5)<<2);
	gotsect = (long *)_alloca(i);
	secfif = (int *)_alloca(gst->numsects*sizeof(secfif[0]));
	if ((unsigned)cursect >= (unsigned)gst->numsects)
	{
		memset(gotsect,-1,i);
		for(i=0;i<gst->numsects;i++) secfif[i] = i; secfifr = 0; secfifw = gst->numsects;
	}
	else
	{
		memset(gotsect,0,i); gotsect[cursect>>5] |= (1<<cursect);
		secfif[0] = cursect; secfifr = 0; secfifw = 1;
	}

	sec = gst->sect;

		//draw sector filled
	bestt = vscale; (*hitsect) = -1; (*hitwall) = -1;
	if ((pv->x == 0.f) && (pv->y == 0.f) && (pv->z == 0.f)) { (*hit) = (*p0); return(0); } //Avoid bad case (/0)
	hit->x = pv->x + p0->x;
	hit->y = pv->y + p0->y;
	hit->z = pv->z + p0->z;
	while (secfifr < secfifw)
	{
		s = secfif[secfifr]; secfifr++;

		wal = sec[s].wall;
		if (pv->z != 0.0)
		{
			for(i=2-1;i>=0;i--)
			{
				if ((pv->x*sec[s].grad[i].x + pv->y*sec[s].grad[i].y + pv->z < 0.f) == (i)) continue;

					//((p0->x + pv->x*t) - wal[0].x   )*sec[s].grad[i].x +
					//((p0->y + pv->y*t) - wal[0].y   )*sec[s].grad[i].y +
					//((p0->z + pv->z*t) - sec[s].z[i])*      1          = 0
				t = pv->x*sec[s].grad[i].x + pv->y*sec[s].grad[i].y + pv->z;
				if (t == 0.f) continue;
				t = ((wal[0].x-p0->x)*sec[s].grad[i].x + (wal[0].y-p0->y)*sec[s].grad[i].y + (sec[s].z[i]-p0->z)) / t;
				x = pv->x*t + p0->x;
				y = pv->y*t + p0->y;
				if ((t > 0) && (t < bestt) && (insidesect(x,y,sec[s].wall,sec[s].n)))
					{ bestt = t; hit->x = x; hit->y = y; hit->z = pv->z*t + p0->z; (*hitsect) = s; (*hitwall) = i-2; }
			}
		}

		for(w=sec[s].n-1;w>=0;w--)
		{
			nw = wal[w].n+w;

				//pv->x*t + p0->x = (wal[nw].x-wal[w].x)*u + wal[w].x;
				//pv->y*t + p0->y = (wal[nw].y-wal[w].y)*u + wal[w].y;
				//
				//pv->x*t + (wal[w].x-wal[nw].x)*u = (wal[w].x-p0->x)
				//pv->y*t + (wal[w].y-wal[nw].y)*u = (wal[w].y-p0->y)
			d = (wal[w].y-wal[nw].y)*pv->x - (wal[w].x-wal[nw].x)*pv->y; if (d >= 0) continue;
			d = 1.0/d;
			t = ((wal[w].x-p0->x)*(wal[w].y-wal[nw].y) - (wal[w].y-p0->y)*(wal[w].x-wal[nw].x))*d;
			u = ((wal[w].y-p0->y)*pv->x                - (wal[w].x-p0->x)*pv->y               )*d;
			if ((t <= 0) || (t >= bestt) || (u < 0) || (u > 1)) continue;
			x = pv->x*t + p0->x;
			y = pv->y*t + p0->y;
			z = pv->z*t + p0->z;
			z0 = getslopez(&sec[s],0,x,y); if (z < z0) continue;
			z1 = getslopez(&sec[s],1,x,y); if (z > z1) continue;
			bs = wal[w].ns; passthru = 0;
			if (bs >= 0)
			{
				bw = wal[w].nw;
				do
				{
					wal2 = sec[bs].wall; i = wal2[bw].n+bw; //Make sure it's an opposite wall
					if ((wal[w].x == wal2[i].x) && (wal[nw].x == wal2[bw].x) &&
						 (wal[w].y == wal2[i].y) && (wal[nw].y == wal2[bw].y))
						if ((z > getslopez(&sec[bs],0,x,y)) && (z < getslopez(&sec[bs],1,x,y)))
						{
							if (!(wal[w].surf.flags&32)) //If not a 1-way wall..
							{
								if (!(gotsect[bs>>5]&(1<<bs)))
									{ secfif[secfifw] = bs; secfifw++; gotsect[bs>>5] |= (1<<bs); }
								passthru = 1;
							}
						}
					bs = wal2[bw].ns;
					bw = wal2[bw].nw;
				} while (bs != s);
			}
			if (!passthru) { bestt = t; hit->x = x; hit->y = y; hit->z = z; (*hitsect) = s; (*hitwall) = w; }
		}

		for(w=sec[s].headspri;w>=0;w=gst->spri[w].sectn)
		{
			spri_t *spr;
			point3d fp, fp2, fpr, fpd;
			float f, Za, Zb, Zc;

			spr = &gst->spri[w];
			if (spr->owner >= 0) continue;

			fp.x = spr->p.x-p0->x;
			fp.y = spr->p.y-p0->y;
			fp.z = spr->p.z-p0->z;
			if (spr->fat > 0.f) //Sphere of KV6: uses spherical collision
			{
					//x = pv->x*t + p0->x;
					//y = pv->y*t + p0->y;
					//z = pv->z*t + p0->z;
					//(pv->x*t + p0->x-spr->p.x)^2 + "y + "z = spr->fat^2
				Za = pv->x*pv->x + pv->y*pv->y + pv->z*pv->z;
				Zb = fp.x*pv->x + fp.y*pv->y + fp.z*pv->z;
				Zc = fp.x*fp.x + fp.y*fp.y + fp.z*fp.z - spr->fat*spr->fat;
				t = Zb*Zb - Za*Zc; if (t < 0) continue;
				t = (Zb-sqrt(t))/Za;
			}
			else //Flat polygon (wall/floor sprite)
			{
					//Draw flat sprite
				switch(spr->flags&48)
				{
					case 0: case 48: fpr = spr->r; fpd = spr->d; break; //Wall/floor sprites
					case 16: //Face sprites
						f = sqrt(spr->r.x*spr->r.x + spr->r.y*spr->r.y + spr->r.z*spr->r.z);
						fpr.x = gps->irig.x*f;
						fpr.y = gps->irig.y*f;
						fpr.z = gps->irig.z*f;
						fpd = spr->d;
						break;
					case 32:
						f = sqrt(spr->r.x*spr->r.x + spr->r.y*spr->r.y + spr->r.z*spr->r.z);
						fpr.x = gps->irig.x*f;
						fpr.y = gps->irig.y*f;
						fpr.z = gps->irig.z*f;
						f = sqrt(spr->d.x*spr->d.x + spr->d.y*spr->d.y + spr->d.z*spr->d.z);
						fpd.x = gps->idow.x*f;
						fpd.y = gps->idow.y*f;
						fpd.z = gps->idow.z*f;
						break;
				}

					//x = pv->x*t + p0->x = spr->p.x + fpr.x*u + fpd.x*v
					//y = pv->y*t + p0->y = spr->p.y + fpr.y*u + fpd.y*v
					//z = pv->z*t + p0->z = spr->p.z + fpr.z*u + fpd.z*v
					//
					//pv->x*t + fpr.x*(-u) + fpd.x*(-v) = spr->p.x-p0->x
					//pv->y*t + fpr.y*(-u) + fpd.y*(-v) = spr->p.y-p0->y
					//pv->z*t + fpr.z*(-u) + fpd.z*(-v) = spr->p.z-p0->z
				fp2.x = fpr.y*fpd.z - fpd.y*fpr.z;
				fp2.y = fpr.z*fpd.x - fpd.z*fpr.x;
				fp2.z = fpr.x*fpd.y - fpd.x*fpr.y;
				d = fp2.x*pv->x + fp2.y*pv->y + fp2.z*pv->z;
				if (fabs((fpd.z*fp.y - fpd.y*fp.z)*pv->x +
							(fpd.x*fp.z - fpd.z*fp.x)*pv->y +
							(fpd.y*fp.x - fpd.x*fp.y)*pv->z) >= fabs(d)) continue;
				if (fabs((fpr.y*fp.z - fpr.z*fp.y)*pv->x +
							(fpr.z*fp.x - fpr.x*fp.z)*pv->y +
							(fpr.x*fp.y - fpr.y*fp.x)*pv->z) >= fabs(d)) continue;
				t = (fp2.x*fp.x + fp2.y*fp.y + fp2.z*fp.z) / d;
			}
			if ((t <= 0) || (t >= bestt)) continue;
			bestt = t;
			hit->x = pv->x*t + p0->x;
			hit->y = pv->y*t + p0->y;
			hit->z = pv->z*t + p0->z;
			(*hitsect) = s; (*hitwall) = w+0x40000000;
		}
	}
	return(bestt < vscale);
}

#ifdef STANDALONE
static void getcurspoint (playerstruct_t *lps, float *qmousx, float *qmousy, float *qmousz, int *hitsect, int *hitwall, point3d *hit)
{
	point3d fp;

	gquantstat = gquantsec = gquantwal = -1; gquantcf = 0;
	(*hitsect) = (*hitwall) = -1; hit->x = hit->y = hit->z = 0.f;
	if (lps->editmode == 2)
	{
		curs2grid(lps,lps->fcmousx,lps->fcmousy,hit);
		(*qmousx) = hit->x; (*qmousy) = hit->y; (*qmousz) = hit->z;
		cursquant(lps,qmousx,qmousy,qmousz,-1);
		if ((gquantwal&0xc0000000) == 0x40000000)
		{
			(*hitsect) = gquantsec;
			(*hitwall) = gquantwal;
		}
		else
		{
			(*hitsect) = lps->cursect; updatesect(*qmousx,*qmousy,1e32,hitsect);
			if ((*hitsect) >= 0)
			{
				if (lps->ifor.z > 0) (*hitwall) = -1; else (*hitwall) = -2;
				(*qmousz) = getslopez(&gst->sect[*hitsect],(*hitwall)&1,*qmousx,*qmousy);
			}
		}
	}
	else
	{
		(*qmousx) = (*qmousy) = (*qmousz) = 0;
		if (((lps->grabmode == GRABDRAG) || (lps->grabmode == GRABDRAG2) || (lps->grabmode == GRABCIRC)) &&
				((unsigned)lps->grabsect < (unsigned)gst->numsects) && ((lps->grabwall&0xc0000000) != 0x40000000))
		{
			sect_t *sec;
			wall_t *wal;
			float t;
			int i, s, cnt;

			s = lps->grabsect;
			sec = gst->sect; wal = sec[s].wall;

			for(i=lps->grabcf,cnt=1;cnt>=0;cnt--,i^=1)
			{
				t = lps->ifor.x*sec[s].grad[i].x + lps->ifor.y*sec[s].grad[i].y + lps->ifor.z;
				if (((t < 0.f) == i) || (t == 0.f)) continue;
					//((lps->ipos.x + lps->ifor.x*t) - wal[0].x   )*sec[s].grad[i].x +
					//((lps->ipos.y + lps->ifor.y*t) - wal[0].y   )*sec[s].grad[i].y +
					//((lps->ipos.z + lps->ifor.z*t) - sec[s].z[i])*      1          = 0
				t = ((wal[0].x-lps->ipos.x)*sec[s].grad[i].x + (wal[0].y-lps->ipos.y)*sec[s].grad[i].y + (sec[s].z[i]-lps->ipos.z)) / t;
				if ((t <= 0) || (t > 64.0)) continue;
				break;
			}
			if (cnt < 0) { (*hitsect) = -1; (*hitwall) = -1; return; }
			if (!cnt) { lps->grabcf ^= 1; }
			hit->x = lps->ifor.x*t + lps->ipos.x;
			hit->y = lps->ifor.y*t + lps->ipos.y;
			hit->z = lps->ifor.z*t + lps->ipos.z;
			(*hitsect) = s; (*hitwall) = i-2;

			(*qmousx) = hit->x; (*qmousy) = hit->y; (*qmousz) = hit->z;
			cursquant(lps,qmousx,qmousy,qmousz,s);
			(*qmousz) = getslopez(&gst->sect[*hitsect],(*hitwall)&1,*qmousx,*qmousy);
		}
		else
		{
			if (hitscan(&lps->ipos,&lps->ifor,1e32,lps->cursect,hitsect,hitwall,hit))
			{
				(*qmousx) = hit->x; (*qmousy) = hit->y; (*qmousz) = hit->z;
				if ((*hitwall) < 0)
				{
					cursquant(lps,qmousx,qmousy,qmousz,*hitsect);
					if (((unsigned)(*hitsect)) < (unsigned)gst->numsects)
						(*qmousz) = getslopez(&gst->sect[*hitsect],(*hitwall)&1,*qmousx,*qmousy);
				}
				else if (((*hitwall)&0xc0000000) == 0x40000000)
					{ gquantstat = 0; gquantsec = (*hitsect); gquantwal = (*hitwall); }
			}
		}
	}
}

	//Returns best wall choice under cursor in 2D or 3D mode (ceiling&floor in 3D hits wall behind it)
long getcurswall (playerstruct_t *lps, int *hitsect, int *hitwall, point3d *hit)
{
	sect_t *sec;
	wall_t *wal;
	float dx, dy, x0, y0, x1, y1, d, f, t, u, qmousx, qmousy, qmousz;
	int i, w;

	if (lps->grabmode >= 0)
		{ (*hitsect) = lps->grabsect; (*hitwall) = lps->grabwall; hit->x = hit->y = hit->z = 0.f; }
	else { getcurspoint(lps,&qmousx,&qmousy,&qmousz,hitsect,hitwall,hit); }

	if (((lps->editmode == 2) || (gquantstat == 1)) && (gquantsec >= 0))
		{ (*hitsect) = gquantsec; (*hitwall) = gquantwal; }
	if ((unsigned)(*hitsect) >= (unsigned)gst->numsects) return(-1);
	if (((*hitwall)&0xc0000000) == 0x40000000) return(-1);

	sec = gst->sect;
	if ((*hitwall) < 0) //Ceiling or floor was hit: find closest wall behind it (pure 2D problem)
	{
		if (lps->editmode == 3)
		{
			wal = sec[*hitsect].wall; d = 1e32;
			dx = hit->x-lps->ipos.x;
			dy = hit->y-lps->ipos.y;
			for(w=sec[*hitsect].n-1;w>=0;w--)
			{
				i = wal[w].n+w;
				x0 = wal[w].x; x1 = wal[i].x-x0;
				y0 = wal[w].y; y1 = wal[i].y-y0;

					//(lps->ipos.x,lps->ipos.y) - (hit->x,hit->y)
					//(x0,y0) - (x1,y1)
					//
					//lps->ipos.x + dx*t = x1*u + x0
					//lps->ipos.y + dy*t = y1*u + y0
					//
					//dx*t + (-x1)*u = (x0-lps->ipos.x)
					//dy*t + (-y1)*u = (y0-lps->ipos.y)
				f = dy*x1 - dx*y1; if (f >= 0.f) continue;
				f = 1.f/f;
				t = ((y0-lps->ipos.y)*x1 - (x0-lps->ipos.x)*y1)*f;
				u = ((y0-lps->ipos.y)*dx - (x0-lps->ipos.x)*dy)*f;
				if ((t > 1.f) && (t < d) && (u >= 0.f) && (u <= 1.f)) { d = t; (*hitwall) = w; }
			}
		}
	}
	if ((unsigned)(*hitwall) >= (unsigned)sec[*hitsect].n) return(-1);
	return(0);
}

#endif

//--------------------------------------------------------------------------------------------------

	//Clip wall slopes. Returns loop ordered poly (0, 3, or 4 points)
	//pol[0]   pol[1]
	//pol[3]   pol[2]

	//This version also handles u&v. Note: Input should still be simple wall quads
long wallclip (kgln_t *pol, kgln_t *npol)
{
	double f, dz0, dz1;

	dz0 = pol[3].z-pol[0].z; dz1 = pol[2].z-pol[1].z;
	if (dz0 > 0.0) //do not include null case for rendering
	{
		npol[0] = pol[0];
		if (dz1 > 0.0) //do not include null case for rendering
		{
			npol[1] = pol[1];
			npol[2] = pol[2];
			npol[3] = pol[3];
			npol[0].n = npol[1].n = npol[2].n = 1; npol[3].n = -3;
			return(4);
		}
		else
		{
			f = dz0/(dz0-dz1);
			npol[1].x = (pol[1].x-pol[0].x)*f + pol[0].x;
			npol[1].y = (pol[1].y-pol[0].y)*f + pol[0].y;
			npol[1].z = (pol[1].z-pol[0].z)*f + pol[0].z;
			npol[1].u = (pol[1].u-pol[0].u)*f + pol[0].u;
			npol[1].v = (pol[1].v-pol[0].v)*f + pol[0].v;
			npol[2] = pol[3];
			npol[0].n = npol[1].n = 1; npol[2].n = -2;
			return(3);
		}
	}
	if (dz1 <= 0.0) return(0); //do not include null case for rendering
	f = dz0/(dz0-dz1);
	npol[0].x = (pol[1].x-pol[0].x)*f + pol[0].x;
	npol[0].y = (pol[1].y-pol[0].y)*f + pol[0].y;
	npol[0].z = (pol[1].z-pol[0].z)*f + pol[0].z;
	npol[0].u = (pol[1].u-pol[0].u)*f + pol[0].u;
	npol[0].v = (pol[1].v-pol[0].v)*f + pol[0].v;
	npol[1] = pol[1];
	npol[2] = pol[2];
	npol[0].n = npol[1].n = 1; npol[2].n = -2;
	return(3);
}

	//Find maximum clip radius (distance to closest point of any visible polygon)
double findmaxcr (dpoint3d *p0, int cursect, double mindist, dpoint3d *hit)
{
	dpoint3d np, nhit, pol[4], npol[4];
	#define MAXVERTS 256 //FIX:timebomb: assumes there are never > 256 sectors connected at same vertex
	vertlist_t verts[MAXVERTS], tvert;
	sect_t *sec;
	wall_t *wal, *wal2;
	spri_t *spr;
	double d, f, g, dist2, mindist2, mindist2andmaxfat;
	long *gotsect;
	int i, j, k, s, w, nw, bs, bw, vn, s0, s1, cf0, cf1, *secfif, secfifw, secfifr, hitit;

	if ((unsigned)cursect >= (unsigned)gst->numsects) return(mindist);

	i = (((gst->numsects+31)>>5)<<2);
	gotsect = (long *)_alloca(i);
	memset(gotsect,0,i);
	gotsect[cursect>>5] |= (1<<cursect);

	secfif = (int *)_alloca(gst->numsects*sizeof(secfif[0]));
	secfif[0] = cursect; secfifr = 0; secfifw = 1;

	hitit = 0;
	mindist2 = mindist*mindist;
	mindist2andmaxfat = (mindist+build2.fattestsprite)*(mindist+build2.fattestsprite);
	hit->x = hit->y = hit->z = -17.0;
	sec = gst->sect;
	//for(s=gst->numsects-1;s>=0;s--) //Brute force for now.. portals later :/
	while (secfifr < secfifw)
	{
		s = secfif[secfifr]; secfifr++;

		//dist2 = 0.0; //FIX: bounded box not implemented yet!
		//d = max(max((sec[s].minx) - p0->x,p0->x - (sec[s].maxx)),0.0); dist2 += d*d;
		//d = max(max((sec[s].miny) - p0->y,p0->y - (sec[s].maxy)),0.0); dist2 += d*d;
		//if (dist2 >= mindist2) continue;

		wal = sec[s].wall; j = 0;
		for(i=2-1;i>=0;i--)
		{
			if ((p0->z > getslopez(&sec[s],i,p0->x,p0->y)) == i) { j |= (i+1); continue; }

				// point: <wal[0].x,wal[0].y,sec[s].z[i]>
				//normal: <sec[s].grad[i].x,sec[s].grad[i].y,1>
			f = (p0->x-wal[0].x)*sec[s].grad[i].x + (p0->y-wal[0].y)*sec[s].grad[i].y + (p0->z-sec[s].z[i])*1.0;
			g = ((double)sec[s].grad[i].x)*sec[s].grad[i].x + ((double)sec[s].grad[i].y)*sec[s].grad[i].y + 1.0*1.0;
			f /= g;
			np.x = p0->x - sec[s].grad[i].x*f;
			np.y = p0->y - sec[s].grad[i].y*f;
			np.z = p0->z -              1.0*f;
			if (!insidesect(np.x,np.y,sec[s].wall,sec[s].n)) continue;
			f *= f*g; if (f < mindist2) { mindist2 = f; (*hit) = np; hitit = 1; }
		}
		if (j == 3) continue; //Behind ceiling or floor
		for(w=0;w<sec[s].n;w++)
		{
			nw = wal[w].n+w;
			vn = getwalls(s,w,verts,MAXVERTS);
			if (wal[w].surf.flags&1) vn = 0; //Blocking wall
			pol[0].x = wal[ w].x; pol[0].y = wal[ w].y;
			pol[1].x = wal[nw].x; pol[1].y = wal[nw].y;
			pol[2].x = wal[nw].x; pol[2].y = wal[nw].y;
			pol[3].x = wal[ w].x; pol[3].y = wal[ w].y;
			for(k=0;k<=vn;k++) //Warning: do not reverse for loop!
			{
				if (k >  0) { s0 = verts[k-1].s; cf0 = 1; } else { s0 = s; cf0 = 0; }
				if (k < vn) { s1 = verts[k  ].s; cf1 = 0; } else { s1 = s; cf1 = 1; }

				pol[0].z = getslopez(&sec[s0],cf0,wal[ w].x,wal[ w].y);
				pol[1].z = getslopez(&sec[s0],cf0,wal[nw].x,wal[nw].y);

				if ((k) && (!(gotsect[s0>>5]&(1<<s0))))
				{
						//FIX:Should only test sectors > mindist2 for sprites...
					if (ptpolydist2(p0,pol,4,&nhit) < mindist2andmaxfat)
						{ secfif[secfifw] = s0; secfifw++; gotsect[s0>>5] |= (1<<s0); }
				}

				pol[2].z = getslopez(&sec[s1],cf1,wal[nw].x,wal[nw].y);
				pol[3].z = getslopez(&sec[s1],cf1,wal[ w].x,wal[ w].y);

				i = wallclip(pol,npol);
				if (i) { d = ptpolydist2(p0,npol,i,&nhit); if (d < mindist2) { mindist2 = d; (*hit) = nhit; hitit = 1; } }
			}
		}

		spr = gst->spri;
		for(w=sec[s].headspri;w>=0;w=spr[w].sectn)
		{
			//if (spr[w].owner >= 0) continue;
			if (!(spr[w].flags&1)) continue;
			if (spr[w].fat > 0)
			{
				np.x = p0->x-spr[w].p.x;
				np.y = p0->y-spr[w].p.y;
				np.z = p0->z-spr[w].p.z;
				d = sqrt(np.x*np.x + np.y*np.y + np.z*np.z);
				f = d-spr[w].fat;
				if ((f <= 0.0) || (f*f >= mindist2)) continue;
				mindist2 = f*f;
				d = spr[w].fat/d;
				hit->x = spr[w].p.x + np.x*d;
				hit->y = spr[w].p.y + np.y*d;
				hit->z = spr[w].p.z + np.z*d;
				hitit = 1;
			}
			else //Flat polygon (wall/floor sprite)
			{
				for(i=4-1;i>=0;i--)
				{
					if ((i+1)&2) f = 1; else f = -1;
					if ((i  )&2) g = 1; else g = -1;
					pol[i].x = spr[w].p.x + spr[w].r.x*f + spr[w].d.x*g;
					pol[i].y = spr[w].p.y + spr[w].r.y*f + spr[w].d.y*g;
					pol[i].z = spr[w].p.z + spr[w].r.z*f + spr[w].d.z*g;
				}
				d = ptpolydist2(p0,pol,4,&nhit); if (d < mindist2) { mindist2 = d; (*hit) = nhit; hitit = 1; }
			}
		}
	}
	if (!hitit) return(mindist); //Minor optimization; this guarantees same value returned if nothing hit
	return(sqrt(mindist2));
}

	//Note: pol doesn't support loops as dpoint3d's!
	//(flags&1): collide both sides of plane
static double sphpolydist (dpoint3d *p0, dpoint3d *v0, double cr, dpoint3d *pol, int n, int flags, dpoint3d *hit)
{
	dpoint3d np, fd, e, ff, fg;
	double f, g, t, u, v, nx, ny, nz, mint, Za, Zb, Zc, x0, y0, x1, y1;
	int i, j, k, maxnormaxis;

	mint = 1.0;

#if 0
		//Saving this block in case I choose to optimize wall planes by processing all segments simultaneously
	dpoint3d pol[4], npol[4];
	double wx, wy, wz;
	sect_t *sec;
	wall_t *wal;
	int nw;
	sec = gst->sect; wal = sec[s].wall; nw = wal[w].n+w;

	pol[0].x = wal[ w].x; pol[0].y = wal[ w].y; pol[0].z = getslopez(&sec[s0],cf0,pol[0].x,pol[0].y);
	pol[1].x = wal[nw].x; pol[1].y = wal[nw].y; pol[1].z = getslopez(&sec[s0],cf0,pol[1].x,pol[1].y);
	pol[2].x = wal[nw].x; pol[2].y = wal[nw].y; pol[2].z = getslopez(&sec[s1],cf1,pol[2].x,pol[2].y);
	pol[3].x = wal[ w].x; pol[3].y = wal[ w].y; pol[3].z = getslopez(&sec[s1],cf1,pol[3].x,pol[3].y);
	n = wallclip(pol,npol); if (!n) return(1.0);

		//Raytrace to planes
	nx = wal[w].y-wal[nw].y; ny = wal[nw].x-wal[w].x; //plane's normal (nz = 0)
	t = v0->x*nx + v0->y*ny;
	if (t < 0.0)
	{
		f = 1.0/sqrt(nx*nx + ny*ny); nx *= f; ny *= f; t *= f; //FIX:optimize
		wx = wal[w].x; wy = wal[w].y; wz = 0.0; //point on plane
		t = (cr - ((p0->x-wx)*nx + (p0->y-wy)*ny)) / t;
		np.x = v0->x*t + p0->x - nx*cr;
		np.y = v0->y*t + p0->y - ny*cr;
		np.z = v0->z*t + p0->z;
		if (fabs(wal[nw].x-wal[w].x) >= fabs(wal[nw].y-wal[w].y))
			  { f = np.x-wal[w].x; g = wal[nw].x-wal[w].x; }
		else { f = np.y-wal[w].y; g = wal[nw].y-wal[w].y; }
		if (g < 0) { f = -f; g = -g; }
		if ((t >= 0) && (t < mint) &&
			 (f >= 0.0) && (f <= g) &&
			 (np.z >= getslopez(&sec[s0],cf0,np.x,np.y)) &&
			 (np.z <= getslopez(&sec[s1],cf1,np.x,np.y)))
			{ mint = t; (*hit) = np; }
	}
#else
	nx = ny = nz = 0.0;
	for(i=n-2;i>0;i--)
	{
		nx += (pol[i].y-pol[0].y)*(pol[i+1].z-pol[0].z) - (pol[i].z-pol[0].z)*(pol[i+1].y-pol[0].y);
		ny += (pol[i].z-pol[0].z)*(pol[i+1].x-pol[0].x) - (pol[i].x-pol[0].x)*(pol[i+1].z-pol[0].z);
		nz += (pol[i].x-pol[0].x)*(pol[i+1].y-pol[0].y) - (pol[i].y-pol[0].y)*(pol[i+1].x-pol[0].x);
	}
	f = nx*nx + ny*ny + nz*nz;
	if (f > 0.0) //Plane must have area
	{
		f = -cr/sqrt(f); nx *= f; ny *= f; nz *= f;

		t = v0->x*nx + v0->y*ny + v0->z*nz;
		if ((flags&1) && (t > 0.0)) { t = -t; nx = -nx; ny = -ny; nz = -nz; }
		if (t < 0.0) //Vector must be towards plane
		{
				//(v0->x*t + p0->x - (pol[0].x + nx))*nx
				//(v0->y*t + p0->y - (pol[0].y + ny))*ny
				//(v0->z*t + p0->z - (pol[0].z + nz))*nz = 0
			t = ((pol[0].x+nx-p0->x)*nx +
				  (pol[0].y+ny-p0->y)*ny +
				  (pol[0].z+nz-p0->z)*nz) / t;
			np.x = v0->x*t + p0->x - nx;
			np.y = v0->y*t + p0->y - ny;
			np.z = v0->z*t + p0->z - nz;

			if ((fabs(nx) > fabs(ny)) && (fabs(nx) > fabs(nz))) maxnormaxis = 0;
			else if (fabs(ny) > fabs(nz)) maxnormaxis = 1; else maxnormaxis = 2;

			for(i=n-1,j=k=0;j<n;i=j,j++)
			{
				if (maxnormaxis > 0) { x0 = pol[i].x - np.x; x1 = pol[j].x - np.x; }
									 else { x0 = pol[i].y - np.y; x1 = pol[j].y - np.y; }
				if (maxnormaxis > 1) { y0 = pol[i].y - np.y; y1 = pol[j].y - np.y; }
									 else { y0 = pol[i].z - np.z; y1 = pol[j].z - np.z; }
				if (y0*y1 < 0.0)
				{
					if (x0*x1 >= 0.0) { if (x0 < 0.0) k++; }
					else if ((x0*y1 - x1*y0)*y1 < 0.0) k++;
				}
			}
			if ((k&1) && (t > 0) && (t < mint)) { mint = t; (*hit) = np; }
		}
	}
#endif

	for(i=0;i<n;i++)
	{
		j = i+1; if (j >= n) j = 0;
			//Raytrace to edges (cylinders)
			//ix = t*v0->x+p0->x  (ix,iy,iz)
			//iy = t*v0->y+p0->y     /�
			//iz = t*v0->z+p0->z   c` cr
			//                   /`   �
			//           p->v[i]���a���������p->v[j]
			//
			//a = ((ix-p->v[i].x)*d.x + (iy-p->v[i].y)*dy + (iz-p->v[i].z)*dz) / sqrt(d.x*d.x + dy*dy + dz*dz)
			//c = sqrt((ix-p->v[i].x)^2 + (iy-p->v[i].y)^2 + (iz-p->v[i].z)^2)
			//a*a + cr*cr = c*c
			//0 <= t < mint
			//
			//((t*v0->x+ex)*d.x+(t*v0->y+e.y)*d.y+(t*v0->z+e.z)*d.z)^2-((t*v0->x+e.x)^2+(t*v0->y+e.y)^2+(t*v0->z+e.z)^2-cr^2)*v
		fd.x = pol[j].x-pol[i].x; e.x = p0->x-pol[i].x;
		fd.y = pol[j].y-pol[i].y; e.y = p0->y-pol[i].y;
		fd.z = pol[j].z-pol[i].z; e.z = p0->z-pol[i].z;
		ff.x = fd.x*fd.x; ff.y = fd.y*fd.y; ff.z = fd.z*fd.z; v = ff.x+ff.y+ff.z;
		ff.x -= v; fg.z = fd.x*fd.y;
		ff.y -= v; fg.y = fd.x*fd.z;
		ff.z -= v; fg.x = fd.y*fd.z;
		Za = ff.x*v0->x*v0->x + ff.y*v0->y*v0->y + ff.z*v0->z*v0->z + (fg.z* v0->x*v0->y          + fg.y* v0->x*v0->z          + fg.x* v0->y*v0->z         )*2;
		Zb = ff.x*e.x*v0->x   + ff.y*e.y*v0->y   + ff.z*e.z*v0->z   + (fg.z*(e.x*v0->y+e.y*v0->x) + fg.y*(e.x*v0->z+e.z*v0->x) + fg.x*(e.y*v0->z+e.z*v0->y))  ;
		Zc = ff.x*e.x*e.x     + ff.y*e.y*e.y     + ff.z*e.z*e.z     + (fg.z* e.x*e.y              + fg.y* e.x*e.z              + fg.x* e.y*e.z             )*2;
		Zc += cr*cr*v;
		u = Zb*Zb - Za*Zc; if ((u < 0.0) || (Za == 0.0)) continue;
	 //t = (sqrt(u)-Zb) / Za;  //Classic quadratic equation (Zb premultiplied by .5)
		t = -Zc / (sqrt(u)+Zb); //Alternate quadratic equation (Zb premultiplied by .5)
		if ((t < 0.0) || (t >= mint)) continue;
		u = (t*v0->x+e.x)*fd.x + (t*v0->y+e.y)*fd.y + (t*v0->z+e.z)*fd.z; if ((u < 0) || (u >= v)) continue;
		mint = t; u /= v;
		hit->x = fd.x*u + pol[i].x;
		hit->y = fd.y*u + pol[i].y;
		hit->z = fd.z*u + pol[i].z;
	}

		//Raytrace to vertices (sphere)
		//ix = t*v0->x + p0->x
		//iy = t*v0->y + p0->y
		//iz = t*v0->z + p0->z
		//(px-ix)^2 + (py-iy)^2 + (pz-iz)^2 = cr*cr
		//0 <= t < mint
		//
		//ex = p0->x-px;
		//ey = p0->y-py;
		//ez = p0->z-pz;
		//(t*v0->x + ex)^2 +
		//(t*v0->y + ey)^2 +
		//(t*v0->z + ez)^2 = cr*cr
		//
		//t*t*v0->x^2 + t*v0->x*ex*2 + ex*ex
		//t*t*v0->y^2 + t*v0->y*ey*2 + ey*ey
		//t*t*v0->z^2 + t*v0->z*ez*2 + ez*ez - cr*cr = 0
	for(i=0;i<n;i++)
	{
		e.x = p0->x-pol[i].x; e.y = p0->y-pol[i].y; e.z = p0->z-pol[i].z;
		Za = v0->x*v0->x + v0->y*v0->y + v0->z*v0->z; //FIX:optimize
		Zb = v0->x*e.x + v0->y*e.y + v0->z*e.z;
		Zc = e.x*e.x + e.y*e.y + e.z*e.z - cr*cr;
		u = Zb*Zb - Za*Zc; if ((u < 0.0) || (Za == 0.0)) continue;
	 //t = -(sqrt(u)+Zb) / Za; //Classic quadratic equation (Zb premultiplied by .5)
		t = Zc / (sqrt(u)-Zb); //Alternate quadratic equation (Zb premultiplied by .5)
		if ((t < 0.0) || (t >= mint)) continue;
		mint = t; (*hit) = pol[i];
	}

	return(mint);
}

static double sphtracewall (dpoint3d *p0, dpoint3d *v0, double cr, int s, int w, int s0, int cf0, int s1, int cf1, dpoint3d *hit)
{
	sect_t *sec;
	wall_t *wal;
	dpoint3d pol[4], npol[8];
	int n, nw;

	sec = gst->sect; wal = sec[s].wall; nw = wal[w].n+w;
	pol[0].x = wal[ w].x; pol[0].y = wal[ w].y; pol[0].z = getslopez(&sec[s0],cf0,wal[ w].x,wal[ w].y);
	pol[1].x = wal[nw].x; pol[1].y = wal[nw].y; pol[1].z = getslopez(&sec[s0],cf0,wal[nw].x,wal[nw].y);
	pol[2].x = wal[nw].x; pol[2].y = wal[nw].y; pol[2].z = getslopez(&sec[s1],cf1,wal[nw].x,wal[nw].y);
	pol[3].x = wal[ w].x; pol[3].y = wal[ w].y; pol[3].z = getslopez(&sec[s1],cf1,wal[ w].x,wal[ w].y);
	n = wallclip(pol,npol); if (!n) return(1.0);
	return(sphpolydist(p0,v0,cr,npol,n,0,hit));
}

static double sphtracerec (dpoint3d *p0, dpoint3d *v0, dpoint3d *hit, int *cursect, double cr)
{
	dpoint3d np, nhit, pol[4];
	#define MAXVERTS 256 //FIX:timebomb: assumes there are never > 256 sectors connected at same vertex
	vertlist_t verts[MAXVERTS], tvert;
	sect_t *sec;
	wall_t *wal, *wal2;
	spri_t *spr;
	double d, f, g, t, u, v, wx, wy, wz, nx, ny, nz, mint, Za, Zb, Zc;
	long *gotsect;
	int i, j, k, s, w, nw, bs, bw, vn, s0, s1, cf0, cf1, *secfif, secfifw, secfifr;

	if ((unsigned)(*cursect) >= (unsigned)gst->numsects) return(1.0);

	i = (((gst->numsects+31)>>5)<<2);
	gotsect = (long *)_alloca(i);
	memset(gotsect,0,i);
	gotsect[(*cursect)>>5] |= (1<<(*cursect));

	secfif = (int *)_alloca(gst->numsects*sizeof(secfif[0]));
	secfif[0] = (*cursect); secfifr = 0; secfifw = 1;

	mint = 1.0; hit->x = hit->y = hit->z = -17.0;

	sec = gst->sect;
	while (secfifr < secfifw)
	{
		s = secfif[secfifr]; secfifr++;

			//Raytrace to planes
			//ix = t*v0->x + p0->x
			//iy = t*v0->y + p0->y
			//iz = t*v0->z + p0->z
			//(ix-wx)*nx + (iy-wy)*ny + (iz-wz)*nz = cr
			//
			//t = (cr - ((p0->x-wx)*nx + (p0->y-wy)*ny + (p0->z-wz)*nz)) / (v0->x*nx + v0->y*ny + v0->z*nz)
		wal = sec[s].wall;
		for(i=2-1;i>=0;i--) //Collide ceilings/floors
		{
			nx = sec[s].grad[i].x; ny = sec[s].grad[i].y; nz = 1.0; //plane's normal
			if (i) { nx = -nx; ny = -ny; nz = -nz; }
			t = v0->x*nx + v0->y*ny + v0->z*nz; if (t >= 0.0) continue;
			f = 1.0/sqrt(nx*nx + ny*ny + 1.0); nx *= f; ny *= f; nz *= f; t *= f; //FIX:optimize
			wx = wal[0].x; wy = wal[0].y; wz = sec[s].z[i]; //point on plane
			f = (p0->x-wx)*nx + (p0->y-wy)*ny + (p0->z-wz)*nz; if (f < 0) continue;
			t = (cr-f)/t;
			np.x = v0->x*t + p0->x - nx*cr;
			np.y = v0->y*t + p0->y - ny*cr;
			np.z = v0->z*t + p0->z - nz*cr;
			if (!insidesect(np.x,np.y,sec[s].wall,sec[s].n)) continue;
			if ((t >= 0) && (t < mint))
			{
				mint = t; (*hit) = np;
				build2.clipsect[build2.cliphitnum] = s;
				build2.clipwall[build2.cliphitnum] = i-2;
			}
		}

		for(w=0;w<sec[s].n;w++)
		{
			nw = wal[w].n+w;
			vn = getwalls(s,w,verts,MAXVERTS);
			if (wal[w].surf.flags&1) vn = 0; //Blocking wall

			s0 = s; cf0 = 0;
			for(k=0;k<=vn;k++) //Warning: do not reverse for loop direction!
			{
				if (k)
				{
					s0 = s1; cf0 = 1;
					if (!(gotsect[s0>>5]&(1<<s0)))
					{
						d = distpoint2line2(p0->x,p0->y,wal[w].x,wal[w].y,wal[nw].x,wal[nw].y);
						if ((d < cr*cr) || (sphtracewall(p0,v0,cr,s,w,s1,cf1,s0,cf0,&nhit) < mint))
							{ secfif[secfifw] = s0; secfifw++; gotsect[s0>>5] |= (1<<s0); }
						else
						{
								//FIX:Should only test these sectors for sprites...
							d = roundcylminpath2(p0->x,p0->y,p0->x+v0->x,p0->y+v0->y,wal[w].x,wal[w].y,wal[nw].x,wal[nw].y);
							if (d < (build2.fattestsprite+cr)*(build2.fattestsprite+cr))
								{ secfif[secfifw] = s0; secfifw++; gotsect[s0>>5] |= (1<<s0); }
						}
					}
				}
				if (k < vn) { s1 = verts[k].s; cf1 = 0; } else { s1 = s; cf1 = 1; }
				d = sphtracewall(p0,v0,cr,s,w,s0,cf0,s1,cf1,&nhit);
				if (d < mint)
				{
					mint = d; (*hit) = nhit;
					build2.clipsect[build2.cliphitnum] = s;
					build2.clipwall[build2.cliphitnum] = w;
				}
			}
		}

		spr = gst->spri;
		for(w=sec[s].headspri;w>=0;w=spr[w].sectn)
		{
			//if (spr[w].owner >= 0) continue;
			if (!(spr[w].flags&1)) continue;

			if (spr[w].fat > 0.f)
			{
					//Raytrace to sphere
				np.x = p0->x-spr[w].p.x;
				np.y = p0->y-spr[w].p.y;
				np.z = p0->z-spr[w].p.z;
				Za = v0->x*v0->x + v0->y*v0->y + v0->z*v0->z; //FIX:optimize
				Zb = v0->x*np.x + v0->y*np.y + v0->z*np.z;
				d = np.x*np.x + np.y*np.y + np.z*np.z;
				Zc = d - (spr[w].fat+cr)*(spr[w].fat+cr);
				u = Zb*Zb - Za*Zc; if ((u < 0.0) || (Za == 0.0)) continue;
			 //t = -(sqrt(u)+Zb) / Za; //Classic quadratic equation (Zb premultiplied by .5)
				t = Zc / (sqrt(u)-Zb); //Alternate quadratic equation (Zb premultiplied by .5)
				if ((t < 0.0) || (t >= mint)) continue;
				mint = t;
				if (d != 0) d = spr[w].fat/sqrt(d);
				hit->x = spr[w].p.x + np.x*d;
				hit->y = spr[w].p.y + np.y*d;
				hit->z = spr[w].p.z + np.z*d;
				build2.clipsect[build2.cliphitnum] = s;
				build2.clipwall[build2.cliphitnum] = w+0x40000000;
			}
			else //Flat polygon (wall/floor sprite)
			{
				for(i=4-1;i>=0;i--)
				{
					if ((i+1)&2) f = 1; else f = -1;
					if ((i  )&2) g = 1; else g = -1;
					pol[i].x = spr[w].p.x + spr[w].r.x*f + spr[w].d.x*g;
					pol[i].y = spr[w].p.y + spr[w].r.y*f + spr[w].d.y*g;
					pol[i].z = spr[w].p.z + spr[w].r.z*f + spr[w].d.z*g;
				}
				j = 0;
				if (!(spr[w].flags&64)) j |= 1; //select 1/2-sided collision
				else if (spr[w].flags&4) { np = pol[0]; pol[0] = pol[2]; pol[2] = np; } //Mirrored: use other side
				t = sphpolydist(p0,v0,cr,pol,4,j,&nhit);
				if ((t < 0.0) || (t >= mint)) continue;
				mint = t;
				(*hit) = nhit;
				build2.clipsect[build2.cliphitnum] = s;
				build2.clipwall[build2.cliphitnum] = w+0x40000000;
			}
		}
	}
	return(mint);
}

	//Returns: 1 if no obstacles, 0 if hit something
static long sphtrace (dpoint3d *p0,  //start pt
							 dpoint3d *v0,  //move vector
							 dpoint3d *hit, //pt causing collision
							 int *cursect,
							 double cr)
{
	dpoint3d dp;
	double mint;

	if ((v0->x == 0.0) && (v0->y == 0.0) && (v0->z == 0.0)) return(1);

	mint = sphtracerec(p0,v0,hit,cursect,cr);
	dp.x = v0->x*mint; p0->x += dp.x; v0->x -= dp.x;
	dp.y = v0->y*mint; p0->y += dp.y; v0->y -= dp.y;
	dp.z = v0->z*mint; p0->z += dp.z; v0->z -= dp.z;
	return(mint == 1.0);
}

static void collmove (dpoint3d *p, int *cursect, dpoint3d *v, double cr, long doslide)
{
	dpoint3d nv, n0, n1, n2;
	double f;
	long i;

	f = sqrt(v->x*v->x + v->y*v->y + v->z*v->z) + cr;
	build2.clipmaxcr = findmaxcr(p,*cursect,f,&nv);
	if (build2.clipmaxcr >= f) { p->x += v->x; p->y += v->y; p->z += v->z; build2.cliphitnum = -1; return; }
	cr = min(cr,build2.clipmaxcr);

	nv = (*v);

	build2.cliphitnum = 0; cr -= 1e-7;
	if (!sphtrace(p,&nv,&build2.cliphit[0],cursect,cr))
	{
		build2.cliphitnum = 1;
		if (doslide)
		{
				//Make slide vector (nv) parallel to surface hit (normal: build2.cliphit[0]-p)
			n0.x = build2.cliphit[0].x-p->x; n0.y = build2.cliphit[0].y-p->y; n0.z = build2.cliphit[0].z-p->z;
			f = (nv.x*n0.x + nv.y*n0.y + nv.z*n0.z) / (cr*cr);
			nv.x -= n0.x*f; nv.y -= n0.y*f; nv.z -= n0.z*f;

			cr -= 1e-7;
			if (!sphtrace(p,&nv,&build2.cliphit[1],cursect,cr))
			{
				build2.cliphitnum = 2;

					//Make slide vector (nv) parallel to both surfaces hit (normals: build2.cliphit[0]-p, build2.cliphit[1]-p)
				n1.x = build2.cliphit[1].x-p->x; n1.y = build2.cliphit[1].y-p->y; n1.z = build2.cliphit[1].z-p->z;
				n2.x = n0.z*n1.y - n0.y*n1.z;
				n2.y = n0.x*n1.z - n0.z*n1.x;
				n2.z = n0.y*n1.x - n0.x*n1.y;
				f = (nv.x*n2.x + nv.y*n2.y + nv.z*n2.z) / (n2.x*n2.x + n2.y*n2.y + n2.z*n2.z);
				nv.x = n2.x*f; nv.y = n2.y*f; nv.z = n2.z*f;

				cr -= 1e-7;
				if (!sphtrace(p,&nv,&build2.cliphit[2],cursect,cr)) build2.cliphitnum = 3;
			}
		}
	}
}

static void collmove (point3d *p, int *cursect, point3d *v, double cr, long doslide)
{
	dpoint3d np, nv;

	np.x = (double)p->x; nv.x = v->x;
	np.y = (double)p->y; nv.y = v->y;
	np.z = (double)p->z; nv.z = v->z;
	collmove(&np,cursect,&nv,cr,doslide);
	p->x = (float)np.x;
	p->y = (float)np.y;
	p->z = (float)np.z;
	if (build2.cliphitnum < 2) return;

		//FIX:This evil hack makes sure double->float quantization does not mess
		//    w/3rd stage of sliding collision
	if ((nv.x < 0.0) && ((double)p->x > np.x)) (*(long *)&p->x)--;
	if ((nv.x > 0.0) && ((double)p->x < np.x)) (*(long *)&p->x)++;
	if ((nv.y < 0.0) && ((double)p->y > np.y)) (*(long *)&p->y)--;
	if ((nv.y > 0.0) && ((double)p->y < np.y)) (*(long *)&p->y)++;
	if ((nv.z < 0.0) && ((double)p->z > np.z)) (*(long *)&p->z)--;
	if ((nv.z > 0.0) && ((double)p->z < np.z)) (*(long *)&p->z)++;
}

//--------------------------------------------------------------------------------------------------

#ifdef STANDALONE

static double getgridsiz (double height2d, long lxres)
{
	double d;

	d = ((double)lxres)/768;
	if (height2d <=   0.25*d) return(1.0/128.0);
	if (height2d <=   0.5 *d) return(1.0/ 64.0);
	if (height2d <=   1.0 *d) return(1.0/ 32.0);
	if (height2d <=   2.0 *d) return(1.0/ 16.0);
	if (height2d <=   4.0 *d) return(1.0/  8.0);
	if (height2d <=   8.0 *d) return(1.0/  4.0);
	if (height2d <=  16.0 *d) return(1.0/  2.0);
	if (height2d <=  32.0 *d) return(1.0/  1.0);
	if (height2d <=  64.0 *d) return(2.0/  1.0);
	if (height2d <= 128.0 *d) return(4.0/  1.0);
									  return(8.0/  1.0);
}

static void drawgrid (cam_t *cc, int mode)
{
	point3d fp;
	float d, f, g, gridsiz, fx[4], fy[4];
	int i, j, k, x, y, x0, y0, x1, y1, mgrid = 8, mgridm1;

	if (gdps->editmode == 3) return;

	i = (long)(gdps->compact2d*256.0);
	gridsiz = getgridsiz(gdps->height2d,gdps->xres);

	for(j=4-1;j>=0;j--)
	{
		curs2grid(gdps,(((j+1)>>1)&1)*xres,(j>>1)*yres,&fp);
		fp.x -= gdps->grdc.x; fp.y -= gdps->grdc.y; fp.z -= gdps->grdc.z;
		fx[j] = fp.x*gdps->grdu.x + fp.y*gdps->grdu.y + fp.z*gdps->grdu.z;
		fy[j] = fp.x*gdps->grdv.x + fp.y*gdps->grdv.y + fp.z*gdps->grdv.z;
	}

	f = 1.0/gridsiz; mgridm1 = mgrid-1;
	x0 = ((int)floor(min(min(fx[0],fx[1]),min(fx[2],fx[3]))*f)); x0 = (x0      )&~mgridm1;
	y0 = ((int)floor(min(min(fy[0],fy[1]),min(fy[2],fy[3]))*f)); y0 = (y0      )&~mgridm1;
	x1 = ((int)floor(max(max(fx[0],fx[1]),max(fx[2],fx[3]))*f)); x1 = (x1+mgrid)&~mgridm1;
	y1 = ((int)floor(max(max(fy[0],fy[1]),max(fy[2],fy[3]))*f)); y1 = (y1+mgrid)&~mgridm1;

	if ((x1-x0 > 256) || (y1-y0 > 256)) return;

	if (!mode)
	{
		for(j=mgrid;j>=0;j-=mgrid)
		{
			for(y=y0|j;y<y1;y+=mgrid)
			{
				if (y) k = 0xff784830; else k = 0xffa06040;
				drawline3d(cc,(x0*gdps->grdu.x + y*gdps->grdv.x)*gridsiz + gdps->grdc.x,
								  (x0*gdps->grdu.y + y*gdps->grdv.y)*gridsiz + gdps->grdc.y,
								  (x0*gdps->grdu.z + y*gdps->grdv.z)*gridsiz + gdps->grdc.z,
								  (x1*gdps->grdu.x + y*gdps->grdv.x)*gridsiz + gdps->grdc.x,
								  (x1*gdps->grdu.y + y*gdps->grdv.y)*gridsiz + gdps->grdc.y,
								  (x1*gdps->grdu.z + y*gdps->grdv.z)*gridsiz + gdps->grdc.z,k); //0xff784830);
			}
			for(x=x0|j;x<x1;x+=mgrid)
			{
				if (x) k = 0xff784830; else k = 0xffa06040;
				drawline3d(cc,(x*gdps->grdu.x + y0*gdps->grdv.x)*gridsiz + gdps->grdc.x,
								  (x*gdps->grdu.y + y0*gdps->grdv.y)*gridsiz + gdps->grdc.y,
								  (x*gdps->grdu.z + y0*gdps->grdv.z)*gridsiz + gdps->grdc.z,
								  (x*gdps->grdu.x + y1*gdps->grdv.x)*gridsiz + gdps->grdc.x,
								  (x*gdps->grdu.y + y1*gdps->grdv.y)*gridsiz + gdps->grdc.y,
								  (x*gdps->grdu.z + y1*gdps->grdv.z)*gridsiz + gdps->grdc.z,k); //0xff784830);
			}
		}
	}

	if (mode)
	{
		if (gdps->gridlock)
		{
			for(y=y0;y<=y1;y++)
				for(x=x0;x<=x1;x++)
				{
						  if ((! x         ) || (! y         )) j = 0xffe09060;
					else if ((!(x&mgridm1)) || (!(y&mgridm1))) j = 0xffb46c48;
					else                                       j = 0xff784830;
					drawpix3d(cc,(x*gdps->grdu.x + y*gdps->grdv.x)*gridsiz + gdps->grdc.x,
									 (x*gdps->grdu.y + y*gdps->grdv.y)*gridsiz + gdps->grdc.y,
									 (x*gdps->grdu.z + y*gdps->grdv.z)*gridsiz + gdps->grdc.z,j);
				}
		}

			//Draw compass
		d = ((float)cc->c.y)/((float)cc->c.x)*-.95;
		fp.x = (gdps->irig.x*-.4 + gdps->idow.x*d)*gdps->height2d + gdps->ifor.x + gdps->ipos.x;
		fp.y = (gdps->irig.y*-.4 + gdps->idow.y*d)*gdps->height2d + gdps->ifor.y + gdps->ipos.y;
		fp.z = (gdps->irig.z*-.4 + gdps->idow.z*d)*gdps->height2d + gdps->ifor.z + gdps->ipos.z;
		drawcone(cc,fp.x,gdps->height2d*+.03 + fp.y,fp.z,gdps->height2d*-.005,
						fp.x,gdps->height2d*-.01 + fp.y,fp.z,gdps->height2d*-.005,0xffb46c48);
		drawcone(cc,fp.x,gdps->height2d*+.00 + fp.y,fp.z,gdps->height2d*-.015,
						fp.x,gdps->height2d*-.03 + fp.y,fp.z,gdps->height2d*-.001,0xffb46c48);
		print6x8(&cc->c,xres*.24,yres*.02,0xe49c78,-1,"North");

		if (gdps->compact2d >= 1.f)
		{
				//Draw ruler
			d = fabs(gridsiz)*8;
			if (gdps->idow.x < -.7) f = -gdps->ipos.y;
			if (gdps->idow.x > +.7) f =  gdps->ipos.y;
			if (gdps->idow.y < -.7) f =  gdps->ipos.x;
			if (gdps->idow.y > +.7) f = -gdps->ipos.x;
			if (gdps->ifor.z < 0.0) f = -f;
			f = fmod(f,d); if (f < 0) f += d;
			g = cc->h.z/fabs(gdps->height2d);
			if ((f-d)*g+cc->h.x >= cc->c.x*.32) f -= d;
			x0 = (f  )*g+cc->h.x;
			x1 = (f+d)*g+cc->h.x;
			int lineoff = lineHeight-3;
			drawline2d(&cc->c,x0,15+lineoff,x1,15+lineoff,0xffc47c58); drawline2d(&cc->c,x0-1,12+lineoff,x0-1,21+lineoff,0xffc47c58); drawline2d(&cc->c,x1-1,12+lineoff,x1-1,21+lineoff,0xffc47c58);
			drawline2d(&cc->c,x0,16+lineoff,x1,16+lineoff,0xffe49c78); drawline2d(&cc->c,x0  ,12+lineoff,x0  ,21+lineoff,0xffe49c78); drawline2d(&cc->c,x1  ,12+lineoff,x1  ,21+lineoff,0xffe49c78);
			drawline2d(&cc->c,x0,17+lineoff,x1,17+lineoff,0xffc47c58); drawline2d(&cc->c,x0+1,12+lineoff,x0+1,21+lineoff,0xffc47c58); drawline2d(&cc->c,x1+1,12+lineoff,x1+1,21+lineoff,0xffc47c58);
			print6x8(&cc->c,(x0+x1)*.5-7*3,yres*.005,0xffe49c78,-1,"%g unit%c",gridsiz*8,((-(gridsiz!=.125))&('s'-32))+32);
		}
	}
}

//--------------------------------------------------------------------------------------------------
static long colwheel_getcol (double x, double y, long *r, long *g, long *b)
{
	double rx, ry, bx, by, sc, f, u0, v0, u1, v1, u2, v2, i0, i1, i2;

	rx = cos(PI*11/6); ry = sin(PI*11/6);
 //gx = cos(PI* 3/6); gy = sin(PI* 3/6);
	bx = cos(PI* 7/6); by = sin(PI* 7/6);

		//rx*u + gx*v = x
		//ry*u + gy*v = y
	sc = 256; f = sc/rx;
	v1 = (x          )*f; u0 = -v1;
	v2 = (x*by - y*bx)*f; u1 = -v2;
	v0 = (x*ry - y*rx)*f; u2 = -v0;
	u0 += sc; v0 += sc; u1 += sc; v1 += sc; u2 += sc; v2 += sc;

	i0 = sc/sqrt(u0*u0 + v0*v0 + sc*sc);
	i1 = sc/sqrt(u1*u1 + v1*v1 + sc*sc);
	i2 = sc/sqrt(u2*u2 + v2*v2 + sc*sc);
	if ((u0 < sc) && (v0 < sc)) { (*r) = u0*i0; (*g) = v0*i0; (*b) = sc*i0; }
				else if (u1 < sc)  { (*r) = sc*i1; (*g) = u1*i1; (*b) = v1*i1; }
								  else { (*r) = v2*i2; (*g) = sc*i2; (*b) = u2*i2; }
	return(((*r) >= 0) && ((*g) >= 0) && ((*b) >= 0));
}

#define COLWHEEL_SIZ 16
static long colwheel_col[COLWHEEL_SIZ+2][COLWHEEL_SIZ]; //Requires an extra line + 1 pixel (therefore 2 lines)
static tile_t colwheel;
static void colwheel_init (void)
{
	double rat, f, ff;
	long x, y, r, g, b;

	rat = 1.0 - 1.0/COLWHEEL_SIZ; f = (COLWHEEL_SIZ-1.0)/2.0; ff = 1.0/(rat*f);
	for(y=0;y<COLWHEEL_SIZ;y++)
		for(x=0;x<COLWHEEL_SIZ;x++)
		{
			colwheel_getcol((x-f)*ff,(y-f)*ff,&r,&g,&b);
			colwheel_col[y][x] = (max(r,0)<<16) + (max(g,0)<<8) + max(b,0) + 0xff000000;
		}
	colwheel.filnam[0] = 0;
	colwheel.tt.f = (long)colwheel_col; colwheel.tt.p = (COLWHEEL_SIZ<<2);
	colwheel.tt.x = COLWHEEL_SIZ; colwheel.tt.y = COLWHEEL_SIZ;
	fixtex4grou((tiltyp *)&colwheel.tt);
}

	//Center, radius, mouse pos
	//cx = xres*.83; cy = yres*.70; rad = 128; intens = 1;
static void colwheel_draw (cam_t *cc, double cx, double cy, double rad, double intens, double mx, double my)
{
	double rat, c, s;
	long i, y, r, g, b, col;
	kgln_t vert[6];

	rat = 1.0 - 1.0/COLWHEEL_SIZ;
	for(i=0;i<6;i++)
	{
		c = cos(((double)i+.5)*PI/3);
		s = sin(((double)i+.5)*PI/3);
		vert[i].z = 0.06; //WARNING! This value must exceed SCISDIST, as defined at the top of drawpoly.c!
		vert[i].x = (cx + c*rad - cc->h.x)*vert[i].z/cc->h.z;
		vert[i].y = (cy + s*rad - cc->h.y)*vert[i].z/cc->h.z;
		vert[i].u = (c*rat + 1.0)/2.0;
		vert[i].v = (s*rat + 1.0)/2.0;
		vert[i].n = 1;
	}
	vert[5].n = -5;
	drawpol(cc,vert,6,&colwheel,((long)min(max(intens*128,0),255))*0x10101,0,0,RENDFLAGS_INTERP);

	if (colwheel_getcol((mx-cx)/rad,(my-cy)/rad,&r,&g,&b))
	{
		r = (long)max(r*intens,0);
		g = (long)max(g*intens,0);
		b = (long)max(b*intens,0);
		col = (min(r,255)<<16)+(min(g,255)<<8)+min(b,255);

		i = (long)(cy-rad);
		for(y=i-26;y<i-22;y++) drawhlin(&cc->c,cx-52,cx+52,y,col);
		for(;y<=i-8;y++)
		{
			drawhlin(&cc->c,cx-52,cx-48,y,col);
			drawhlin(&cc->c,cx-48,cx+48,y,0);
			drawhlin(&cc->c,cx+48,cx+52,y,col);
		}
		for(;y<=i-4;y++) drawhlin(&cc->c,cx-52,cx+52,y,col);
		print6x8(&cc->c,cx-43,i-20,0xff8080,0,"%4.2f",((double)r)/256.0);
		print6x8(&cc->c,cx-11,i-20,0x80ff80,0,"%4.2f",((double)g)/256.0);
		print6x8(&cc->c,cx+21,i-20,0x8080ff,0,"%4.2f",((double)b)/256.0);

		drawpix(&cc->c,mx,my,((int)(sin(dtotclk*16.0)*64+128))*0x10101);
	}
}
//--------------------------------------------------------------------------------------------------

#endif

	//Quantizes rotation matrix to one of the 48 possible identity permutations
static void oriquant (point3d *lrig, point3d *ldow, point3d *lfor)
{
	float d, maxd, b[9], c[9], in[9];
	long i, p, s;

	in[0] = lrig->x; in[1] = lrig->y; in[2] = lrig->z;
	in[3] = ldow->x; in[4] = ldow->y; in[5] = ldow->z;
	in[6] = lfor->x; in[7] = lfor->y; in[8] = lfor->z;

	maxd = -1e32;
	for(i=0;i<9;i++) b[i] = (float)(!(i&3));
	for(p=0;p<6;p++)
	{
		for(s=0;s<4;s++)
		{
			d = 0.0; for(i=0;i<9;i++) d += in[i]*b[i];
			if (+d > maxd) { maxd = +d; for(i=0;i<9;i++) c[i] = +b[i]; }
			if (-d > maxd) { maxd = -d; for(i=0;i<9;i++) c[i] = -b[i]; }
			for(i=(s&1)*3;i<(s&1)*3+3;i++) b[i] = -b[i];
		}
		for(i=0;i<3;i++) { d = b[i]; b[i] = b[i+3]; b[i+3] = d; }
		if (p != 2) { for(i=3;i<6;i++) { d = b[i]; b[i] = b[i+3]; b[i+3] = d; } }
	}
	lrig->x = c[0]; lrig->y = c[1]; lrig->z = c[2];
	ldow->x = c[3]; ldow->y = c[4]; ldow->z = c[5];
	lfor->x = c[6]; lfor->y = c[7]; lfor->z = c[8];
}

void saveaskc (char *filnam)
{
	#define MAXVERTS 256 //FIX:timebomb: assumes there are never > 256 sectors connected at same vertex
	vertlist_t verts[MAXVERTS], tvert;
	kgln_t pol[4], npol[4], tpol;
	point2d *grad;
	sect_t *sec;
	wall_t *wal, *wal2;
	zoid_t *zoids;
	FILE *fil;
	float dx, fx, fy, fz;
	int i, j, k, n, w, s, nw, wn, bs, bw, vn, s0, cf0, s1, cf1, isflor, otexi, ocol = -1, col, nzoids;

	fil = fopen(filnam,"wb"); if (!fil) return;

	sec = gst->sect;

	fprintf(fil,"()\n{\n\tcls(0); clz(1e32);\n");
	fprintf(fil,"\tsetcam(%+f,%+f,%+f,\n\t\t\t %+f,%+f,%+f,\n\t\t\t %+f,%+f,%+f,\n\t\t\t %+f,%+f,%+f);\n",
		gst->startpos.x,gst->startpos.y,gst->startpos.z,
		gst->startrig.x,gst->startrig.y,gst->startrig.z,
		gst->startdow.x,gst->startdow.y,gst->startdow.z,
		gst->startfor.x,gst->startfor.y,gst->startfor.z);
	fprintf(fil,"\tdrawmap();\n}\n\ndrawmap ()\n{\n");

	otexi = -1;
	for(s=0;s<gst->numsects;s++)
	{
		sec = &gst->sect[s];

			//draw sector filled
		for(isflor=0;isflor<2;isflor++)
		{
			col = (min(sec->surf[isflor].asc>>8,255)<<24) +
					(min(sec->surf[isflor].rsc>>5,255)<<16) +
					(min(sec->surf[isflor].gsc>>5,255)<< 8) +
					(min(sec->surf[isflor].bsc>>5,255)    );

			wal = sec->wall; fz = sec->z[isflor]; grad = &sec->grad[isflor]; n = sec->n;

			if (!sect2trap(wal,n,&zoids,&nzoids)) continue;

			if (col != ocol) { ocol = col; fprintf(fil,"\tsetcol(%d,%d,%d);\n",((col>>16)&255)<<1,((col>>8)&255)<<1,(col&255)<<1); }
			if (sec->surf[isflor].tilnum != otexi) { otexi = sec->surf[isflor].tilnum; fprintf(fil,"\tglsettex(\"%s\");\n",gtile[otexi].filnam); }

			for(i=0;i<nzoids;i++)
			{
				for(j=0,n=0;j<4;j++)
				{
					pol[n].x = zoids[i].x[j];
					pol[n].y = zoids[i].y[j>>1];
					if ((!n) || (pol[n].x != pol[n-1].x) || (pol[n].y != pol[n-1].y))
					{
						pol[n].u = pol[n].x*sec->surf[isflor].uv[1].x + pol[n].y*sec->surf[isflor].uv[2].x + sec->surf[isflor].uv[0].x;
						pol[n].v = pol[n].x*sec->surf[isflor].uv[1].y + pol[n].y*sec->surf[isflor].uv[2].y + sec->surf[isflor].uv[0].y;
						pol[n].z = (wal[0].x-pol[n].x)*grad->x + (wal[0].y-pol[n].y)*grad->y + fz;
						pol[n].n = 1; n++;
					}
				}
				if (n < 3) continue;
				pol[n-1].n = 1-n;

				fprintf(fil,"\tglBegin(GL_TRIANGLE_FAN);\n");
				for(j=0;j<n;j++)
				{
					if (isflor) k = j; else k = n-1-j;
					fprintf(fil,"\tglTexCoord(%g,%g); ",pol[k].u,pol[k].v);
					fprintf(fil,"glVertex(%g,%g,%g);\n",pol[k].x,pol[k].y,pol[k].z);
				}
			}
			free(zoids);
		}

		sec = gst->sect;

		wal = sec[s].wall; wn = sec[s].n;
		for(w=0;w<wn;w++)
		{
			nw = wal[w].n+w;
			col = (min(wal[w].surf.asc>>8,255)<<24) +
					(min(wal[w].surf.rsc>>5,255)<<16) +
					(min(wal[w].surf.gsc>>5,255)<< 8) +
					(min(wal[w].surf.bsc>>5,255)    );
			dx = sqrt((wal[nw].x-wal[w].x)*(wal[nw].x-wal[w].x) + (wal[nw].y-wal[w].y)*(wal[nw].y-wal[w].y));

			vn = getwalls(s,w,verts,MAXVERTS);
			pol[0].x = wal[ w].x; pol[0].y = wal[ w].y; pol[0].n = 1;
			pol[1].x = wal[nw].x; pol[1].y = wal[nw].y; pol[1].n = 1;
			pol[2].x = wal[nw].x; pol[2].y = wal[nw].y; pol[2].n = 1;
			pol[3].x = wal[ w].x; pol[3].y = wal[ w].y; pol[3].n =-3;
			for(k=0;k<=vn;k++) //Warning: do not reverse for loop!
			{
				if (k >  0) { s0 = verts[k-1].s; cf0 = 1; } else { s0 = s; cf0 = 0; }
				if (k < vn) { s1 = verts[k  ].s; cf1 = 0; } else { s1 = s; cf1 = 1; }

				pol[0].z = getslopez(&sec[s0],cf0,pol[0].x,pol[0].y);
				pol[1].z = getslopez(&sec[s0],cf0,pol[1].x,pol[1].y);
				pol[2].z = getslopez(&sec[s1],cf1,pol[2].x,pol[2].y);
				pol[3].z = getslopez(&sec[s1],cf1,pol[3].x,pol[3].y);

				pol[0].u = wal[w].surf.uv[0].x; //FIXFIX
				pol[0].v = wal[w].surf.uv[0].y + wal[w].surf.uv[2].y*(pol[0].z-sec[s].z[0]);
				pol[1].u = wal[w].surf.uv[2].x*(pol[1].z-pol[0].z) + pol[0].u + wal[w].surf.uv[1].x*dx;
				pol[1].v = wal[w].surf.uv[2].y*(pol[1].z-pol[0].z) + pol[0].v + wal[w].surf.uv[1].y*dx;
				pol[2].u = wal[w].surf.uv[2].x*(pol[2].z-pol[0].z) + pol[0].u + wal[w].surf.uv[1].x*dx;
				pol[2].v = wal[w].surf.uv[2].y*(pol[2].z-pol[0].z) + pol[0].v + wal[w].surf.uv[1].y*dx;
				pol[3].u = wal[w].surf.uv[2].x*(pol[3].z-pol[0].z) + pol[0].u;
				pol[3].v = wal[w].surf.uv[2].y*(pol[3].z-pol[0].z) + pol[0].v;

				i = wallclip(pol,npol); if (!i) continue;

				if (col != ocol) { ocol = col; fprintf(fil,"\tsetcol(%d,%d,%d);\n",((col>>16)&255)<<1,((col>>8)&255)<<1,(col&255)<<1); }
				if (wal[w].surf.tilnum != otexi) { otexi = wal[w].surf.tilnum; fprintf(fil,"\tglsettex(\"%s\");\n",gtile[otexi].filnam); }
				fprintf(fil,"\tglBegin(GL_TRIANGLE_FAN);\n");
				for(j=0;j<i;j++)
				{
					fprintf(fil,"\tglTexCoord(%g,%g); ",npol[j].u,npol[j].v);
					fprintf(fil,"glVertex(%g,%g,%g);\n",npol[j].x,npol[j].y,npol[j].z);
				}
				fprintf(fil,"\tglEnd();\n");
			}
		}
	}
	fprintf(fil,"}");
	fclose(fil);
}

	//   //STL binary format:
	//char filler[80];
	//unsigned long numtris;
	//for(i=0;i<numtris;i++)
	//{
	//   point3d norm, v[3]; //vertices are CCW and must be + coords
	//   short filler;
	//}
void saveasstl (char *filnam)
{
	#define MAXVERTS 256 //FIX:timebomb: assumes there are never > 256 sectors connected at same vertex
	vertlist_t verts[MAXVERTS], tvert;
	kgln_t pol[4], npol[4], tpol;
	point3d fp[3], fp2;
	point2d *grad;
	sect_t *sec;
	wall_t *wal, *wal2;
	zoid_t *zoids;
	FILE *fil;
	float f, fx, fy, fz;
	int i, j, k, n, w, s, nw, wn, bs, bw, vn, s0, cf0, s1, cf1, isflor, nzoids;
	long numtris;
	char tbuf[80];

	fil = fopen(filnam,"wb"); if (!fil) return;
	memset(tbuf,0,80);
	fwrite(tbuf,80,1,fil); //header ignored in STL
	numtris = 0;
	fwrite(&numtris,4,1,fil); //dummy write

	sec = gst->sect;

	//gst->startpos.x,gst->startpos.y,gst->startpos.z,

	for(s=0;s<gst->numsects;s++)
	{
		sec = &gst->sect[s];

			//draw sector filled
		for(isflor=0;isflor<2;isflor++)
		{
			wal = sec->wall; fz = sec->z[isflor]; grad = &sec->grad[isflor]; n = sec->n;

			if (!sect2trap(wal,n,&zoids,&nzoids)) continue;

			for(i=0;i<nzoids;i++)
			{
				for(j=0,n=0;j<4;j++)
				{
					pol[n].x = zoids[i].x[j];
					pol[n].y = zoids[i].y[j>>1];
					if ((!n) || (pol[n].x != pol[n-1].x) || (pol[n].y != pol[n-1].y))
					{
						pol[n].z = (wal[0].x-pol[n].x)*grad->x + (wal[0].y-pol[n].y)*grad->y + fz;
						pol[n].n = 1; n++;
					}
				}
				if (n < 3) continue;
				pol[n-1].n = 1-n;

				fp[0].x = pol[0].x; fp[0].y = pol[0].y; fp[0].z = pol[0].z;
				for(j=2;j<n;j++)
				{
					k = j-isflor;   fp[1].x = pol[k].x; fp[1].y = pol[k].y; fp[1].z = pol[k].z;
					k = j-1+isflor; fp[2].x = pol[k].x; fp[2].y = pol[k].y; fp[2].z = pol[k].z;
						//fp2 = unit norm
					fp2.x = (fp[1].y-fp[0].y)*(fp[2].z-fp[0].z) - (fp[1].z-fp[0].z)*(fp[2].y-fp[0].y);
					fp2.y = (fp[1].z-fp[0].z)*(fp[2].x-fp[0].x) - (fp[1].x-fp[0].x)*(fp[2].z-fp[0].z);
					fp2.z = (fp[1].x-fp[0].x)*(fp[2].y-fp[0].y) - (fp[1].y-fp[0].y)*(fp[2].x-fp[0].x);
					f = fp2.x*fp2.x + fp2.y*fp2.y + fp2.z*fp2.z; if (f > 0.0) f = -1.0/sqrt(f);
					fp2.x *= f; fp2.y *= f; fp2.z *= f; fwrite(&fp2,4*3,1,fil);
					fwrite(fp,4*3*3,1,fil); fwrite(tbuf,2,1,fil); //2 bytes of filler
					numtris++;
				}
			}
			free(zoids);
		}

		sec = gst->sect;

		wal = sec[s].wall; wn = sec[s].n;
		for(w=0;w<wn;w++)
		{
			nw = wal[w].n+w; vn = getwalls(s,w,verts,MAXVERTS);
			pol[0].x = wal[ w].x; pol[0].y = wal[ w].y; pol[0].n = 1;
			pol[1].x = wal[nw].x; pol[1].y = wal[nw].y; pol[1].n = 1;
			pol[2].x = wal[nw].x; pol[2].y = wal[nw].y; pol[2].n = 1;
			pol[3].x = wal[ w].x; pol[3].y = wal[ w].y; pol[3].n =-3;
			for(k=0;k<=vn;k++) //Warning: do not reverse for loop!
			{
				if (k >  0) { s0 = verts[k-1].s; cf0 = 1; } else { s0 = s; cf0 = 0; }
				if (k < vn) { s1 = verts[k  ].s; cf1 = 0; } else { s1 = s; cf1 = 1; }

				pol[0].z = getslopez(&sec[s0],cf0,pol[0].x,pol[0].y);
				pol[1].z = getslopez(&sec[s0],cf0,pol[1].x,pol[1].y);
				pol[2].z = getslopez(&sec[s1],cf1,pol[2].x,pol[2].y);
				pol[3].z = getslopez(&sec[s1],cf1,pol[3].x,pol[3].y);
				i = wallclip(pol,npol); if (!i) continue;

				fp[0].x = npol[0].x; fp[0].y = npol[0].y; fp[0].z = npol[0].z;
				for(j=2;j<i;j++)
				{
					fp[1].x = npol[j-1].x; fp[1].y = npol[j-1].y; fp[1].z = npol[j-1].z;
					fp[2].x = npol[j  ].x; fp[2].y = npol[j  ].y; fp[2].z = npol[j  ].z;
						//fp2 = unit norm
					fp2.x = (fp[1].y-fp[0].y)*(fp[2].z-fp[0].z) - (fp[1].z-fp[0].z)*(fp[2].y-fp[0].y);
					fp2.y = (fp[1].z-fp[0].z)*(fp[2].x-fp[0].x) - (fp[1].x-fp[0].x)*(fp[2].z-fp[0].z);
					fp2.z = (fp[1].x-fp[0].x)*(fp[2].y-fp[0].y) - (fp[1].y-fp[0].y)*(fp[2].x-fp[0].x);
					f = fp2.x*fp2.x + fp2.y*fp2.y + fp2.z*fp2.z; if (f > 0.0) f = -1.0/sqrt(f);
					fp2.x *= f; fp2.y *= f; fp2.z *= f; fwrite(&fp2,4*3,1,fil);
					fwrite(fp,4*3*3,1,fil); fwrite(tbuf,2,1,fil); //2 bytes of filler
					numtris++;
				}
			}
		}
	}

	i = ftell(fil);
	fseek(fil,80,SEEK_SET); fwrite(&numtris,4,1,fil);
	fseek(fil,i,SEEK_SET);
	fclose(fil);
}

void savemap (char *filnam)
{
	FILE *fil;
	sect_t *sec;
	int i, j;
	short s;

	i = strlen(filnam);
	if ((i >= 3) && (!stricmp(&filnam[i-3],".kc"))) { saveaskc(filnam); return; }
	if ((i >= 4) && (!stricmp(&filnam[i-4],".stl"))) { saveasstl(filnam); return; }

	checknextwalls();
	checksprisect(-1);
	compacttilelist(3);

	fil = fopen(filnam,"wb"); if (!fil) return;

	sec = gst->sect;
	i = 0x3242534b; fwrite(&i,4,1,fil); //KSB2
	fwrite(&gst->startpos,sizeof(gst->startpos),1,fil);
	fwrite(&gst->startrig,sizeof(gst->startrig),1,fil);
	fwrite(&gst->startdow,sizeof(gst->startdow),1,fil);
	fwrite(&gst->startfor,sizeof(gst->startfor),1,fil);
	fwrite(&gst->numsects,4,1,fil);
	fwrite(sec,sizeof(sect_t)*gst->numsects,1,fil);
	for(i=0;i<gst->numsects;i++)
	{
		if (!sec[i].wall) continue;
		for(j=0;j<sec[i].n;j++)
		{
			fwrite(&sec[i].wall[j],sizeof(wall_t),1,fil);
			if (sec[i].wall[j].surfn > 1) fwrite(sec[i].wall[j].xsurf,(sec[i].wall[j].surfn-1)*sizeof(surf_t),1,fil);
		}
	}
	fwrite(&gnumtiles,4,1,fil);
	for(i=0;i<gnumtiles;i++)
	{
		s = strlen(gtile[i].filnam); fwrite(&s,2,1,fil);
		fwrite(gtile[i].filnam,s,1,fil);
	}
	fwrite(&gst->numspris,4,1,fil);
	fwrite(gst->spri,gst->numspris*sizeof(spri_t),1,fil);

	fclose(fil);
}

typedef struct { long tilnum, flags, tag; point2d uv[3]; int dummy[6]; short asc, rsc, gsc, bsc; } osurf1_t;
typedef struct { float x, y; long n, ns, nw; surf_t surf; } owall1_t;
typedef struct { float z[2]; point2d grad[2]; surf_t surf[2]; long foglev; wall_t *wall; int n, nmax; } osect1_t;

static int loadmap (char *filnam)
{
	surf_t *sur;
	sect_t *sec;
	wall_t *wal;
	spri_t *spr;
	float f, fx, fy;
	int i, j, k, l;
	long x, y, z, fileid, hitile, warned = 0, altsects, nnumtiles, nnumspris;
	short s, cursect;
	char och, tbuf[256];

	if (!kzopen(filnam))
	{     //Try without full pathname - see if it's in ZIP/GRP/Mounted_Dir
		for(i=j=0;filnam[i];i++) if ((filnam[i] == '/') || (filnam[i] == '\\')) j = i+1;
		if (!j) return(0);
		filnam = &filnam[j];
		if (!kzopen(filnam)) return(0);
	}
	kzread(&fileid,4);
	if ((fileid == 0x04034b50) || (fileid == 0x536e654b)) //'PK\3\4' is ZIP file id, 'KenS' is GRP file id
		{ kzclose(); kzaddstack(filnam); return(1); }
	sec = gst->sect; gst->light_sprinum = 0;
	if (fileid == 0x3142534b) //KSB1
	{
		typedef struct { long tilnum, flags, tag; point2d uv[3]; int dummy[6]; short asc, rsc, gsc, bsc; } surf1_t;
		typedef struct { float x, y; long n, ns, nw; surf1_t surf; } wall1_t;
		typedef struct { float z[2]; point2d grad[2]; surf1_t surf[2]; long foglev; wall1_t *wall; int n, nmax; } sect1_t;
		surf1_t surf1;
		wall1_t wall1;
		sect1_t sect1;

		for(i=gst->numsects-1;i>=0;i--)
			if (gst->sect[i].wall) { free(gst->sect[i].wall); gst->sect[i].wall = 0; }
		kzread(&gst->numsects,4);
		if (gst->numsects > gst->malsects)
		{
			i = gst->malsects; gst->malsects = max(gst->numsects+1,gst->malsects<<1);
			sec = gst->sect = (sect_t *)realloc(sec,gst->malsects*sizeof(sect_t));
			memset(&sec[i],0,(gst->malsects-i)*sizeof(sect_t));
		}
		memset(sec,0,sizeof(sect_t)*gst->numsects);
		for(i=0;i<gst->numsects;i++)
		{
			kzread(&sect1,sizeof(sect1_t));
			for(j=0;j<2;j++)
			{
				sec[i].z[j] = sect1.z[j];
				sec[i].grad[j] = sect1.grad[j];
				//for(k=0;k<3;k++) sec[i].surf[j].uv[k] = sect1.surf[j].uv[k];
				sec[i].surf[j].uv[1].x = sec[i].surf[j].uv[2].y = 1.f;
				sec[i].surf[j].asc = sect1.surf[j].asc;
				sec[i].surf[j].rsc = sect1.surf[j].rsc;
				sec[i].surf[j].gsc = sect1.surf[j].gsc;
				sec[i].surf[j].bsc = sect1.surf[j].bsc;
				sec[i].headspri = -1;
				sec[i].owner = -1;
			}
			sec[i].n = sect1.n;
			sec[i].nmax = sect1.nmax;
		}
		for(i=0;i<gst->numsects;i++)
		{
			sec[i].wall = (wall_t *)malloc(sec[i].nmax*sizeof(wall_t));
			memset(sec[i].wall,0,sec[i].nmax*sizeof(wall_t));
			for(j=0;j<sec[i].n;j++)
			{
				kzread(&wall1,sizeof(wall1_t));
				wal = sec[i].wall;
				wal[j].x = wall1.x;
				wal[j].y = wall1.y;
				wal[j].n = wall1.n;
				wal[j].ns = wall1.ns;
				wal[j].nw = wall1.nw;
				if (!stricmp(&filnam[max(strlen(filnam)-13,0)],"sos_test3.map"))
					  { for(k=0;k<3;k++) wal[j].surf.uv[k] = wall1.surf.uv[k]; }
				else { wal[j].surf.uv[1].x = wal[j].surf.uv[2].y = 1.f; }
				wal[j].surf.asc = wall1.surf.asc;
				wal[j].surf.rsc = wall1.surf.rsc;
				wal[j].surf.gsc = wall1.surf.gsc;
				wal[j].surf.bsc = wall1.surf.bsc;
				wal[j].surfn = 1;
				wal[j].owner = -1;
			}
		}

		gst->numspris = 0;

#ifdef STANDALONE
		for(i=numplayers-1;i>=0;i--) gst->p[i].sec.n = 0;
#endif
		checknextwalls();
		checksprisect(-1);
		kzclose();
		return(1);
	}
	else if (fileid == 0x3242534b) //KSB2 (current BUILD2 map format)
	{
		kzread(&gst->startpos,sizeof(gst->startpos));
		kzread(&gst->startrig,sizeof(gst->startrig));
		kzread(&gst->startdow,sizeof(gst->startdow));
		kzread(&gst->startfor,sizeof(gst->startfor));
		for(i=numplayers-1;i>=0;i--)
		{
			gst->p[i].ipos = gst->startpos;
			gst->p[i].ifor = gst->startfor;
			gst->p[i].irig = gst->startrig;
			gst->p[i].idow = gst->startdow;
			gst->p[i].cursect = -1;
		}

			//Load sectors
		altsects = 0;
		for(i=0;i<gst->numsects;i++)
		{
			if (sec[i].owner < 0)
			{
				while (sec[i].headspri >= 0) delspri(sec[i].headspri);
				if (gst->sect[i].wall) { free(gst->sect[i].wall); gst->sect[i].wall = 0; }
				continue;
			}
			for(j=sec[i].headspri;j>=0;j=gst->spri[j].sectn) gst->spri[j].sect = altsects;
			memcpy(&sec[altsects],&sec[i],sizeof(sect_t)); altsects++;
		}
		kzread(&i,4); gst->numsects = i+altsects;
		if (gst->numsects > gst->malsects)
		{
			i = gst->malsects; gst->malsects = max(gst->numsects+1,gst->malsects<<1);
			sec = gst->sect = (sect_t *)realloc(sec,gst->malsects*sizeof(sect_t));
			memset(&sec[i],0,(gst->malsects-i)*sizeof(sect_t));
		}
		kzread(&sec[altsects],(gst->numsects-altsects)*sizeof(sect_t));

			//Load walls
		for(i=altsects;i<gst->numsects;i++)
		{
			sec[i].wall = (wall_t *)malloc(sec[i].nmax*sizeof(wall_t));
			sec[i].owner = -1;
			for(j=0;j<sec[i].n;j++)
			{
				kzread(&sec[i].wall[j],sizeof(wall_t));

				if (!sec[i].wall[j].n)
				{
					if (!warned)
					{
						warned = 1;
						if (MessageBox(ghwnd,"Your map appears to be corrupt. Load anyway?",prognam,MB_YESNO) == IDNO)
						{
							for(;i>=0;i--) free(sec[i].wall);
							gst->numsects = 0;
							return(-1);
						}
					}
				}

				sec[i].wall[j].owner = -1;
				if (sec[i].wall[j].surfn > 1)
				{
					sec[i].wall[j].xsurf = (surf_t *)malloc((sec[i].wall[j].surfn-1)*sizeof(surf_t));
					kzread(sec[i].wall[j].xsurf,(sec[i].wall[j].surfn-1)*sizeof(surf_t));
				}
			}
		}

			//Load tiles
		kzread(&nnumtiles,4); gnumtiles += nnumtiles;
		if (gnumtiles > gmaltiles)
		{
			gmaltiles = max(gnumtiles+1,gmaltiles<<1);
			gtile = (tile_t *)realloc(gtile,gmaltiles*sizeof(tile_t));
		}
		for(i=gnumtiles-nnumtiles;i<gnumtiles;i++)
		{
			kzread(&s,2); kzread(gtile[i].filnam,s); gtile[i].filnam[s] = 0; //FIX:possible buffer overflow here
			gtile[i].tt.f = 0;
			gtile[i].namcrc32 = getcrc32z(0,(unsigned char *)gtile[i].filnam);
		}

			//Load sprites
		kzread(&nnumspris,4); gst->numspris += nnumspris;
		if (!nnumspris) for(i=0;i<gst->numsects;i++) { sec[i].headspri = -1; sec[i].owner = -1; } //Hack for loading old format
		if (gst->numspris > gst->malspris)
		{
			i = gst->malspris;
			gst->malspris = max(gst->numspris+1,gst->malspris<<1);
			gst->spri = (spri_t *)realloc(gst->spri,gst->malspris*sizeof(spri_t));
#ifndef STANDALONE
			for(;i<gst->malspris;i++) gst->spri[i].sect = -1;
#endif
		}
		kzread(&gst->spri[gst->numspris-nnumspris],nnumspris*sizeof(spri_t));
		for(i=gst->numspris-nnumspris;i<gst->numspris;i++) gst->spri[i].sect += altsects;


			// | 0 ..       altsects ..  gst->numsects   |
			// |   ^old_sects^    |     ^new_sects^      |
			//
			// |0..gst->numspris-nnumspris..gst->numspris|
			// |  ^old_sprites^   |    ^new_sprites^     |
			//
			// | 0 ..  gnumtiles-nnumtiles .. gnumtiles  |
			// |   ^old_tiles^    |     ^new_tiles^      |

			//Adjust tile indices for new sectors(/walls) & sprites
		for(i=altsects;i<gst->numsects;i++)
		{
			for(j=0;j<2       ;j++) sec[i].surf[j].tilnum      += gnumtiles-nnumtiles;
			for(j=0;j<sec[i].n;j++) sec[i].wall[j].surf.tilnum += gnumtiles-nnumtiles;
		}
		for(i=gst->numspris-nnumspris;i<gst->numspris;i++) if (gst->spri[i].tilnum >= 0) gst->spri[i].tilnum += gnumtiles-nnumtiles;

		//-------------------------------------------------------------------

			//Sprite hacks
		for(i=0;i<gst->numspris;i++)
		{
			gst->spri[i].owner = -1;

				//Insert lights
			if (gst->spri[i].flags&(1<<16))
			{
				if (gst->light_sprinum < MAXLIGHTS) gst->light_spri[gst->light_sprinum++] = i;
			}
		}

#ifdef STANDALONE
		for(i=numplayers-1;i>=0;i--) gst->p[i].sec.n = 0;
#endif
		checknextwalls();
		checksprisect(-1);

#if 0
			//Rebuild hash table from scratch
		memset(gtilehashead,-1,sizeof(gtilehashead));
		for(i=0;i<gnumtiles;i++)
		{
			j = (gtile[i].namcrc32&(sizeof(gtilehashead)/sizeof(gtilehashead[0])-1));
			gtile[i].hashnext = gtilehashead[j]; gtilehashead[j] = i;
		}
#else
		compacttilelist(1);
#endif

		kzclose();
		return(1);
	}
	else if ((fileid == 0x00000007) || //Build1 .MAP format 7
				(fileid == 0x00000cbe))   //Cubes5 .CUB format
	{
			//Build1 format variables:
		typedef struct { short picnum, heinum; signed char shade; char pal, xpanning, ypanning; } build7surf_t;
		typedef struct
		{
			short wallptr, wallnum;
			long z[2]; short stat[2]; build7surf_t surf[2];
			char visibility, filler;
			short lotag, hitag, extra;
		} build7sect_t;
		typedef struct
		{
			long x, y;
			short point2, nextwall, nextsect, cstat, picnum, overpicnum;
			signed char shade;
			char pal, xrepeat, yrepeat, xpanning, ypanning;
			short lotag, hitag, extra;
		} build7wall_t;
		typedef struct
		{
			long x, y, z; short cstat, picnum;
			signed char shade; char pal, clipdist, filler;
			unsigned char xrepeat, yrepeat; signed char xoffset, yoffset;
			short sectnum, statnum, ang, owner, xvel, yvel, zvel, lotag, hitag, extra;
		} build7spri_t;
		build7sect_t b7sec;
		build7wall_t b7wal;
		build7spri_t b7spr;

			//Cubes5 format variables:
		#define BSIZ 16
		double c1, c2, c3, s1, s2, s3, c1c3, c1s3, s1c3, s1s3;
		signed short board[6][BSIZ][BSIZ][BSIZ]; //Board layout
		long posx, posy, posz, a1, a2, a3, oy, yy;

		//------------------------------------------------------------------------
		long filnum, arttiles, loctile0, loctile1, iskenbuild = 0;
		short *tilesizx = 0, *tilesizy = 0, *tilefile = 0;
		char tbuf[MAX_PATH*2];

		kzclose();

		strcpy(curmappath,filnam);
		for(i=j=0;curmappath[i];i++) if ((curmappath[i] == '/') || (curmappath[i] == '\\')) j = i+1;
		curmappath[j] = 0;

		arttiles = 0; //Scan .ART files, incrementing number until tile is in range
		for(filnum=0;1;filnum++)
		{
			sprintf(tbuf,"TILES%03d.ART",filnum);
			if (!kzopen(tbuf))
			{
				sprintf(tbuf,"%sTILES%03d.ART",curmappath,filnum);
				if (!kzopen(tbuf)) break;
			}
			kzread(tbuf,16); if (*(long *)&tbuf[0] != 1) break;
			loctile0 = *(long *)&tbuf[8];
			loctile1 = (*(long *)&tbuf[12])+1;
			if ((loctile0 < 0) || (loctile1 <= arttiles) || (loctile0 >= loctile1)) continue;
			i = arttiles; arttiles = loctile1;
			tilesizx = (short *)realloc(tilesizx,arttiles*sizeof(tilesizx[0]));
			tilesizy = (short *)realloc(tilesizy,arttiles*sizeof(tilesizy[0]));
			tilefile = (short *)realloc(tilefile,arttiles*sizeof(tilefile[0]));
			for(;i<arttiles;i++) { tilesizx[i] = 0; tilesizy[i] = 0; tilefile[i] = 0; }
			kzread(&tilesizx[loctile0],(loctile1-loctile0)*sizeof(short));
			kzread(&tilesizy[loctile0],(loctile1-loctile0)*sizeof(short));
			for(i=loctile0;i<loctile1;i++) tilefile[i] = filnum;
		}
		if (!arttiles)
		{
			tilesizx = (short *)malloc(sizeof(tilesizx[0]));
			tilesizy = (short *)malloc(sizeof(tilesizy[0]));
			tilefile = (short *)malloc(sizeof(tilefile[0]));
			tilesizx[0] = tilesizy[0] = 2; tilefile[0] = 0; arttiles = 1;
		}
		else if (arttiles >= 20) //Autodetect KenBuild data
		{
			for(i=24-1;i>=0;i--) //If the sizes of the 1st 24 tiles match that of Kenbuild, then that's what it is
			{
				x = 32; if (i == 4)               x = 16; if (i >= 20) x = 64;
				y = 32; if ((i == 3) || (i == 4)) y = 16; if (i >= 18) y = 64;
				if ((tilesizx[i] != x) || (tilesizy[i] != y)) break;
			}
			if (i < 0) iskenbuild = 1;
		}

		kzclose();
		kzopen(filnam);
		kzread(&i,4);
		//------------------------------------------------------------------------

		hitile = 0;

		if (fileid == 0x00000007) //Build1 .MAP format 7
		{
			kzread(&x,4); //posx
			kzread(&y,4); //posy
			kzread(&z,4); //posz
			kzread(&s,2); //ang
			kzread(&cursect,2); //cursectnum
			gst->startpos.x = ((float)x)*(1.f/512.f);
			gst->startpos.y = ((float)y)*(1.f/512.f);
			gst->startpos.z = ((float)z)*(1.f/(512.f*16.f));
			gst->startfor.x = cos(((float)s)*PI/1024.0);
			gst->startfor.y = sin(((float)s)*PI/1024.0);
			gst->startfor.z = 0.f;
			gst->startrig.x =-gst->startfor.y;
			gst->startrig.y = gst->startfor.x;
			gst->startrig.z = 0.f;
			gst->startdow.x = 0.f;
			gst->startdow.y = 0.f;
			gst->startdow.z = 1.f;
			for(i=numplayers-1;i>=0;i--)
			{
				gst->p[i].ipos = gst->startpos;
				gst->p[i].ifor = gst->startfor;
				gst->p[i].irig = gst->startrig;
				gst->p[i].idow = gst->startdow;
				gst->p[i].cursect = cursect;
			}

			kzread(&s,2);
			gst->numsects = (int)s; //numsectors
			if (gst->numsects > gst->malsects)
			{
				i = gst->malsects; gst->malsects = max(gst->numsects+1,gst->malsects<<1);
				sec = gst->sect = (sect_t *)realloc(sec,gst->malsects*sizeof(sect_t));
				memset(&sec[i],0,(gst->malsects-i)*sizeof(sect_t));
			}
			for(i=0;i<gst->numsects;i++)
			{
				kzread(&b7sec,sizeof(b7sec));
				sec[i].n = sec[i].nmax = b7sec.wallnum;
				sec[i].wall = (wall_t *)realloc(sec[i].wall,sec[i].nmax*sizeof(wall_t));
				memset(sec[i].wall,0,sec[i].nmax*sizeof(wall_t));
				for(j=0;j<2;j++)
				{
					sec[i].z[j] = ((float)b7sec.z[j])*(1.f/(512.f*16.f));
					sec[i].grad[j].x = sec[i].grad[j].y = 0;
					if (b7sec.stat[j]&2) //Enable slopes flag
						sec[i].grad[j].y = ((float)b7sec.surf[j].heinum)*(1.f/4096.f);
					sur = &sec[i].surf[j];
					sur->flags = 0;
					if (b7sec.stat[j]&1) sur->flags |= (1<<16);
					sur->asc = 4096;
					sur->rsc = (32-b7sec.surf[j].shade)*128;
					sur->gsc = (32-b7sec.surf[j].shade)*128;
					sur->bsc = (32-b7sec.surf[j].shade)*128;
					l = b7sec.surf[j].picnum;
					if ((unsigned)l >= (unsigned)arttiles) l = 0;
					sur->tilnum = l; hitile = max(hitile,l);

					// Convert lotag/hitag to single tag field
					// j=0 is ceiling, j=1 is floor - assign to floor surface only
					if (j == 1) // Floor surface
					{
						// Merge lotag (lower 16 bits) and hitag (upper 16 bits) into single long
						sur->lotag = b7sec.lotag;
						sur->hitag = b7sec.hitag;
					}

					sur->pal = b7sec.surf[j].pal;

					sur->uv[0].x = ((float)b7sec.surf[j].xpanning)/256.0;
					sur->uv[0].y = ((float)b7sec.surf[j].ypanning)/256.0;
					sur->uv[1].y = sur->uv[2].x = 0;
					if (!(b7sec.stat[j]&4))
					{
						sur->uv[1].x = 32.0/((float)tilesizx[l]);
						sur->uv[2].y = 32.0/((float)tilesizy[l]);
					}
					else
					{
						sur->uv[1].x = 32.0/((float)tilesizy[l]);
						sur->uv[2].y = 32.0/((float)tilesizx[l]);
					}
					if (b7sec.stat[j]&8) { sur->uv[1].x *= 2; sur->uv[2].y *= 2; } //double smooshiness
					if (b7sec.stat[j]&16) sur->uv[1].x *= -1; //x-flip
					if (!(b7sec.stat[j]&32)) sur->uv[2].y *= -1; //y-flip
					if (b7sec.stat[j]&64) //relative alignment
					{
						f = ((float)b7sec.surf[j].heinum)*(1.f/4096.f);
						sur->uv[2].y *= -sqrt(f*f + 1.f);
						sur->flags |= 4;
					}
					if (b7sec.stat[j]&4) //swap x&y
					{
						if (((b7sec.stat[j]&16) != 0) != ((b7sec.stat[j]&32) != 0))
							{ sur->uv[1].x *= -1; sur->uv[2].y *= -1; }
						sur->uv[1].y = sur->uv[1].x; sur->uv[1].x = 0;
						sur->uv[2].x = sur->uv[2].y; sur->uv[2].y = 0;
					}

					//FIX:This hack corrects an LHS vs. RHS bug in a later stage of texture mapping (drawsectfill?)
					if (sur->uv[1].x*sur->uv[2].y < sur->uv[1].y*sur->uv[2].x)
						{ sur->uv[2].x *= -1; sur->uv[2].y *= -1; }
				}

				sec[i].headspri = -1;
				sec[i].owner = -1;
				//sec[i].foglev = ?;
			}
			kzread(&s,2); //numwalls
			for(i=k=0;i<gst->numsects;i++)
			{
				for(j=0;j<sec[i].n;j++,k++)
				{
					kzread(&b7wal,sizeof(b7wal));
					sec[i].wall[j].x = ((float)b7wal.x)*(1.f/512.f);
					sec[i].wall[j].y = ((float)b7wal.y)*(1.f/512.f);
					sec[i].wall[j].n = b7wal.point2-k;
					sur = &sec[i].wall[j].surf;
					sur->flags = 0;
					if (b7wal.cstat&1) sur->flags |= 1;

					// flag at byte 1 : double split  = 1, one tile = 0
					// bottom tile is taken from overtile of nextwall(meaning opposite side of the wall)
					// mask tile is undertile field

					sur->lotag = b7wal.lotag;
					sur->hitag = b7wal.hitag;
					sur->pal = b7wal.pal;

					sur->uv[0].x = b7wal.xpanning;
					sur->uv[0].y = b7wal.ypanning;
					sur->uv[1].x = b7wal.xrepeat; if (b7wal.cstat&  8) sur->uv[1].x *= -1;
					sur->uv[1].y = sur->uv[2].x = 0;
					sur->uv[2].y = b7wal.yrepeat; if (b7wal.cstat&256) sur->uv[2].y *= -1;
					if ((b7wal.nextsect < 0) ^ (!(b7wal.cstat&4))) sur->flags ^= 4; //align bot/nextsec
					if (b7wal.cstat&(16+32)) sur->flags |= 32; //bit4:masking, bit5:1-way
					sur->asc = 4096;
					sur->rsc = (32-b7wal.shade)*128;
					sur->gsc = (32-b7wal.shade)*128;
					sur->bsc = (32-b7wal.shade)*128;
					l = b7wal.picnum; if ((unsigned)l >= (unsigned)arttiles) l = 0;
					sur->tilnum = l; hitile = max(hitile,l);
					sec[i].wall[j].surfn = 1;
					sec[i].wall[j].owner = -1;
				}
				// tile adjust?
				for(j=0;j<sec[i].n;j++)
				{
					l = j+sec[i].wall[j].n;
					fx = sec[i].wall[l].x - sec[i].wall[j].x;
					fy = sec[i].wall[l].y - sec[i].wall[j].y;
					f = sqrt(fx*fx + fy*fy);
					sur = &sec[i].wall[j].surf;
					l = sur->tilnum;
					sur->uv[1].x = ((float)sur->uv[1].x*8.0)/(f*((float)tilesizx[l]));
					sur->uv[2].y = ((float)sur->uv[2].y*4.0)/((float)tilesizy[l]);
					sur->uv[0].x = ((float)sur->uv[0].x)/((float)tilesizx[l]);
					sur->uv[0].y = ((float)sur->uv[0].y)/256.f * (1-2*(sur->uv[2].y < 0));
				}

				fx = sec[i].wall[1].y-sec[i].wall[0].y;
				fy = sec[i].wall[0].x-sec[i].wall[1].x;
				f = fx*fx + fy*fy; if (f > 0) f = 1.0/sqrt(f); fx *= f; fy *= f;
				for(j=0;j<2;j++)
				{
					sec[i].grad[j].x = fx*sec[i].grad[j].y;
					sec[i].grad[j].y = fy*sec[i].grad[j].y;
				}
			}

			kzread(&s,2); gst->numspris = (int)s;
			if (gst->numspris > gst->malspris)
			{
				i = gst->malspris;
				gst->malspris = max(gst->numspris+1,gst->malspris<<1);
				gst->spri = (spri_t *)realloc(gst->spri,gst->malspris*sizeof(spri_t));
#ifndef STANDALONE
				for(;i<gst->malspris;i++) gst->spri[i].sect = -1;
#endif
			}
			for(i=0;i<gst->numspris;i++)
			{
				kzread(&b7spr,sizeof(b7spr));
				spr = &gst->spri[i];
				memset(spr,0,sizeof(spri_t));

				l = b7spr.picnum; if ((unsigned)l >= (unsigned)arttiles) l = 0;
				spr->p.x = ((float)b7spr.x)*(1.f/512.f);
				spr->p.y = ((float)b7spr.y)*(1.f/512.f);
				spr->p.z = ((float)b7spr.z)*(1.f/(512.f*16.f));
				spr->flags = 0;
				switch(b7spr.cstat&48)  // https://wiki.eduke32.com/wiki/Cstat_(sprite)
										// 48  =32  +16 wall or  floor only check
					{
					case 0: //Face sprite
						spr->flags |= 16;
						//no break intentional
					case 48: //Voxel sprite
						//no break intentional
					case 16: //Wall sprite
						spr->p.z -= (b7spr.yrepeat/4096.0*(float)tilesizy[l]);
						spr->r.x = sin((float)b7spr.ang*PI/1024.0)*(b7spr.xrepeat/4096.0*(float)tilesizx[l]);
						spr->r.y =-cos((float)b7spr.ang*PI/1024.0)*(b7spr.xrepeat/4096.0*(float)tilesizx[l]);
						spr->d.z = (b7spr.yrepeat/4096.0*(float)tilesizy[l]);
						break;
					case 32: //Floor sprite
						spr->r.x = sin((float)b7spr.ang*PI/1024.0)*(b7spr.xrepeat/4096.0*(float)tilesizx[l]);
						spr->r.y =-cos((float)b7spr.ang*PI/1024.0)*(b7spr.xrepeat/4096.0*(float)tilesizx[l]);
						spr->d.x = cos((float)b7spr.ang*PI/1024.0)*(b7spr.yrepeat/4096.0*(float)tilesizy[l]);
						spr->d.y = sin((float)b7spr.ang*PI/1024.0)*(b7spr.yrepeat/4096.0*(float)tilesizy[l]);
						if (b7spr.cstat&8) { spr->d.x *= -1; spr->d.y *= -1; }
						break;
				}
				if (b7spr.cstat&1) spr->flags |= 1; // blocking
				if (b7spr.cstat&64) spr->flags |= 64; // 1 sided
				if (b7spr.cstat&4) { spr->r.x *= -1; spr->r.y *= -1; spr->r.z *= -1; spr->flags ^= 4; } //&4: x-flipped
				if (b7spr.cstat&8) { spr->d.x *= -1; spr->d.y *= -1; spr->d.z *= -1; spr->flags ^= 4; } //&8: y-flipped?
				if (b7spr.cstat&128) { spr->p.z += (b7spr.yrepeat/4096.0*(float)tilesizy[l]); } //&128: real-centered centering (center at center) - originally half submerged sprite

				if ((unsigned)b7spr.sectnum < (unsigned)gst->numsects) //Make shade relative to sector
				{
					j = b7spr.sectnum; j = 32 - gst->sect[j].surf[gst->sect[j].surf[0].flags&1^1].rsc/128;
					if (iskenbuild) b7spr.shade += j+6;
				}

				spr->f.z=3; // sus
				spr->f.x=cos((float)b7spr.ang*PI/1024.0);
				spr->f.y=sin((float)b7spr.ang*PI/1024.0);
				spr->fat = 0.f;
				spr->asc = 4096;
				spr->rsc = (32-b7spr.shade)*128;
				spr->gsc = (32-b7spr.shade)*128;
				spr->bsc = (32-b7spr.shade)*128;

				spr->mas = spr->moi = 1.0;
				spr->owner = -1;

				spr->tilnum = l; hitile = max(hitile,l);
				spr->sect = b7spr.sectnum;
				spr->sectn = spr->sectp = -1;
				spr->lotag = b7spr.lotag;
				spr->hitag = b7spr.hitag;
				spr->pal = b7spr.pal;
			}
		}
		else //CUBES5 map format (.CUB extension)
		{
			kzread(&x,4); //posx
			kzread(&y,4); //posy
			kzread(&z,4); //posz
			kzread(&a1,4); //angle range: 0-2047
			kzread(&a2,4);
			kzread(&a3,4);
			gst->startpos.x =      ((float)x)*(1.f/1024.f);
			gst->startpos.y = 16.f-((float)z)*(1.f/1024.f);
			gst->startpos.z =      ((float)y)*(1.f/1024.f);
			dcossin((double)a3*PI/1024.0,&c1,&s1);
			dcossin((double)a2*PI/1024.0,&c2,&s2);
			dcossin((double)a1*PI/1024.0,&c3,&s3);
			c1c3 = c1*c3; c1s3 = c1*s3; s1c3 = s1*c3; s1s3 = s1*s3;
			gst->startrig.x = c1c3 + s1s3*s2;
			gst->startrig.y = s1c3 - c1s3*s2;
			gst->startrig.z = -s3*c2;
			gst->startdow.x = c1s3 - s1c3*s2;
			gst->startdow.y = s1s3 + c1c3*s2;
			gst->startdow.z = c3*c2;
			gst->startfor.x = s1*c2;
			gst->startfor.y = -c1*c2;
			gst->startfor.z = s2;
			for(i=numplayers-1;i>=0;i--)
			{
				gst->p[i].ipos = gst->startpos;
				gst->p[i].irig = gst->startrig;
				gst->p[i].idow = gst->startdow;
				gst->p[i].ifor = gst->startfor;
				gst->p[i].cursect = -1;
			}

				//6 faces * BSIZ^3 board map * 2 bytes for picnum
				//if face 0 is -1, the cube is air
			kzread(board,6*BSIZ*BSIZ*BSIZ*sizeof(short));
			gst->numsects = 0;
			for(x=0;x<BSIZ;x++)
				for(z=0;z<BSIZ;z++)
				{
					oy = BSIZ;
					for(y=0;y<BSIZ;y++)
					{
						if (board[0][x][y][z] >= 0) continue;

						if (oy > y) oy = y;
						if ((y < BSIZ-1) && (board[0][x][y+1][z] < 0)) continue;

						if (gst->numsects >= gst->malsects)
						{
							i = gst->malsects; gst->malsects = max(gst->numsects+1,gst->malsects<<1);
							sec = gst->sect = (sect_t *)realloc(sec,gst->malsects*sizeof(sect_t));
							memset(&sec[i],0,(gst->malsects-i)*sizeof(sect_t));
						}

						i = gst->numsects;
						sec[i].n = sec[i].nmax = 4;
						sec[i].wall = (wall_t *)realloc(sec[i].wall,sec[i].nmax*sizeof(wall_t));
						memset(sec[i].wall,0,sec[i].nmax*sizeof(wall_t));
						sec[i].z[0] = (float) oy  ;
						sec[i].z[1] = (float)(y+1);
						for(j=0;j<2;j++)
						{
							sec[i].grad[j].x = sec[i].grad[j].y = 0;
							sur = &sec[i].surf[j];
							sur->uv[1].x = sur->uv[2].y = 1.f;
							sur->asc = 4096;
							sur->rsc = 4096-768+1536*j;
							sur->gsc = 4096-768+1536*j;
							sur->bsc = 4096-768+1536*j;

							if (!j) l = board[4][x][oy-1][z];
								else l = board[1][x][ y+1][z];
							if ((unsigned)l >= (unsigned)arttiles) l = 0;
							sur->tilnum = l; hitile = max(hitile,l);
							sur->uv[1].x = max(64.f/((float)tilesizx[l]),1.f);
							sur->uv[2].y = max(64.f/((float)tilesizy[l]),1.f);
						}
						//sec[i].foglev = ?;
						sec[i].headspri = -1;
						sec[i].owner = -1;

						for(j=0;j<4;j++)
						{
							sec[i].wall[j].x = ((float)(       x+(((j+1)>>1)&1)))*(1.f);
							sec[i].wall[j].y = ((float)(BSIZ-1-z+(( j  )>>1)   ))*(1.f);
							if (j < 3) sec[i].wall[j].n = 1; else sec[i].wall[j].n = -3;
							sec[i].wall[j].surf.uv[1].x = sec[i].wall[j].surf.uv[2].y = 1;
							sec[i].wall[j].surf.asc = 4096;
							sec[i].wall[j].surf.rsc = 4096+(labs(j-1)-1)*512;
							sec[i].wall[j].surf.gsc = 4096+(labs(j-1)-1)*512;
							sec[i].wall[j].surf.bsc = 4096+(labs(j-1)-1)*512;
							sec[i].wall[j].surfn = 1;
							sec[i].wall[j].owner = -1;

							l = -1;
							for(yy=oy;yy<=y;yy++)
							{
								switch (j)
								{
									case 0: if (board[0][x  ][yy][z+1] < 0) break; l = board[2][x  ][yy][z+1]; break;
									case 1: if (board[0][x+1][yy][z  ] < 0) break; l = board[0][x+1][yy][z  ]; break;
									case 2: if (board[0][x  ][yy][z-1] < 0) break; l = board[5][x  ][yy][z-1]; break;
									case 3: if (board[0][x-1][yy][z  ] < 0) break; l = board[3][x-1][yy][z  ]; break;
								}
								if ((unsigned)l < (unsigned)arttiles) break;
							}
							if ((unsigned)l >= (unsigned)arttiles) l = 0;
							sec[i].wall[j].surf.tilnum = l; hitile = max(hitile,l);
							sec[i].wall[j].surf.uv[1].x = max(64.f/((float)tilesizx[l]),1.f);
							sec[i].wall[j].surf.uv[2].y = max(64.f/((float)tilesizy[l]),1.f);
						}
						gst->numsects++;

						oy = BSIZ;
					}
				}

			gst->numspris = 0;
		}

			//Set texture names..
		for(i=gnumtiles-1;i>=0;i--)
			if (gtile[i].tt.f) { free((void *)gtile[i].tt.f); gtile[i].tt.f = 0; }
		gnumtiles = 0; memset(gtilehashead,-1,sizeof(gtilehashead));

		hitile++;
		if (hitile > gmaltiles)
		{
			gmaltiles = hitile;
			gtile = (tile_t *)realloc(gtile,gmaltiles*sizeof(tile_t));
		}
		for(i=0;i<hitile;i++)
		{
			sprintf(tbuf,"tiles%03d.art|%d",tilefile[i],i);
			gettileind(tbuf);
		}

		if (tilesizx) free(tilesizx);
		if (tilesizy) free(tilesizy);
		if (tilefile) free(tilefile);

#ifdef STANDALONE
		for(i=numplayers-1;i>=0;i--) gst->p[i].sec.n = 0;
#endif
		checknextwalls();
		checksprisect(-1);
		kzclose();
		return(1);
	}
	else { return(0); } //MessageBox(ghwnd,"Invalid MAP format",prognam,MB_OK);
}

#ifdef STANDALONE

static void releasegrabdrag (long curindex, long doplaysound)
{
	#define MAXVERTS 256 //FIX:timebomb: assumes there are never > 256 sectors connected at same vertex
	vertlist_t verts[MAXVERTS], tvert;
	sect_t *sec;
	wall_t *wal;
	float qmousx, qmousy, qmousz;
	long i, j, k, s, w;
	point3d fp, hit;
	int hitsect, hitwall;

	sec = gst->sect;
	if (gps->grabmode == GRABDRAG3) goto releaselmbskiptjunk;
	if ((gps->grabsect >= 0) && ((gps->grabmode == GRABDRAG) || (gps->grabmode == GRABDRAG2)))
	{
		i = 0;
		for(s=gst->numsects-1;s>=0;s--)
		{
			wal = gst->sect[s].wall;
			for(w=gst->sect[s].n-1;w>=0;w--)
			{
				if (sec[s].wall[w].owner != curindex) continue;
				sec[s].wall[w].owner = -1;
				if (i < MAXVERTS) { verts[i].s = s; verts[i].w = w; i++; }
			}
			for(w=gst->sect[s].headspri;w>=0;w=gst->spri[w].sectn)
			{
				if (gst->spri[w].owner != curindex) continue;
				gst->spri[w].owner = -1;
			}
		}

		getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
		hitwall = 0;

			//Sort verts in ascending order to avoid deletion problems (s=MSD, w=LSD)
		for(k=1;k<i;k++)
			for(j=0;j<k;j++)
				if ((verts[j].s > verts[k].s) || ((verts[j].s == verts[k].s) && (verts[j].w > verts[k].w)))
					{ tvert = verts[j]; verts[j] = verts[k]; verts[k] = tvert; }

		for(i=i-1;i>=0;i--)
		{
			s = verts[i].s; w = verts[i].w; wal = sec[s].wall;
			k = w;
			do
			{
				j = wallprev(&sec[s],w);
				if ((wal[j].x != wal[w].x) || (wal[j].y != wal[w].y)) break;
				w = j;
			} while (w != k);

			while (sec[s].n > 0)
			{
				j = wal[w].n+w; if (j == w) { delwall(&sec[s],w); hitwall = 1; break; }
				if ((wal[j].x != wal[w].x) || (wal[j].y != wal[w].y)) break;

				if (verts[i].w == w) { wal[w].x = wal[j].x; wal[w].y = wal[j].y; } //kill xy of dragged point
				wal[w].surf = wal[j].surf; //preserve texture of non-minimized wall
				delwall(&sec[s],j); hitwall = 1;
				if (w > j) w--;
				if (verts[i].w > j) verts[i].w--;
			}
			if (!sec[s].n) { delsect(s); while ((i >= 0) && (verts[i].s == s)) i--; }
		}
		if (hitwall)
		{
			if (doplaysound)
			{
				fp.x = qmousx; fp.y = qmousy; fp.z = qmousz;
				myplaysound("sounds\\hammer.wav",100,0.75,&fp,KSND_3D);
			}
		}
releaselmbskiptjunk:;
		checknextwalls();
		checksprisect(-1);
		gps->grabmode = -1;
	}
}

	//It is assumed that (x0,y0)-(x1,y1) are pre-sorted left->right, top->bot
static long isintersect_sect_rect (long s, float x0, float y0, float x1, float y1)
{
	sect_t *sec;
	zoid_t *zoids;
	int nzoids;
	float fx, fy, fx0, fy0, fx1, fy1;
	long i, w;

	sec = gst->sect; if (sec[s].n <= 0) return(0);
	fx0 = sec[s].wall[0].x; fx1 = fx0;
	fy0 = sec[s].wall[0].y; fy1 = fy0;
	for(w=sec[s].n-1;w>0;w--)
	{
		fx = sec[s].wall[w].x;
		fy = sec[s].wall[w].y;
		if (fx < fx0) fx0 = fx;
		if (fy < fy0) fy0 = fy;
		if (fx > fx1) fx1 = fx;
		if (fy > fy1) fy1 = fy;
	}
	if ((x0 >= fx1) || (y0 >= fy1) || (fx0 >= x1) || (fy0 >= y1)) return(0);

	if (insidesect(x0,y0,sec[s].wall,sec[s].n)) return(1);

		//FIXFIX
	if (!sect2trap(sec[s].wall,sec[s].n,&zoids,&nzoids)) return(0);
	for(i=0;i<nzoids;i++)
	{
		if (zoids[i].y[1] <= y0) continue;
		if (zoids[i].y[0] >= y1) break;

			//trapezoid-rectangle intersection
		fy0 = max(y0,zoids[i].y[0]);
		fy1 = min(y1,zoids[i].y[1]); if (fy0 >= fy1) continue;
		fy = zoids[i].y[1]-zoids[i].y[0];
		if ((min(fy0,fy1)-zoids[i].y[0])*(zoids[i].x[3]-zoids[i].x[0]) >= (x1-zoids[i].x[0])*fy) continue;
		if ((max(fy0,fy1)-zoids[i].y[0])*(zoids[i].x[2]-zoids[i].x[1]) <= (x0-zoids[i].x[1])*fy) continue;
		free(zoids); return(1);
	}
	free(zoids); return(0);
}

static void executepack (unsigned char *recvbuf, int doplaysound)
{
	FILE *fil;
	#define MAXVERTS 256 //FIX:timebomb: assumes there are never > 256 sectors connected at same vertex
	vertlist_t verts[MAXVERTS], verts2[MAXVERTS], tvert;
	surf_t *sur;
	sect_t *sec;
	wall_t *wal, twal;
	spri_t *spr;
	dpoint3d dp;
	point3d fp, hit, irig2, idow2, ifor2;
	float f, d, dx, dy, x0, y0, x1, y1, mind, dtim, fmousx, fmousy, fmousz, qmousx, qmousy, qmousz;
	int i, j, k, r, g, b, s, w, x, y, bs, bw, hitsect, hitwall, recvcnt, recvleng, curindex, key;
	char ch, *cptr;
	#define TESTKEY(k) (gps->skeystatus[(k)>>3]&(1<<((k)&7)))

	sec = gst->sect;
	recvleng = (int)recvbuf[0];
	curindex = (int)recvbuf[1];

	doplaysound = ((curindex == moveindex) && (curindex == 0) && (gst == &sst)) ||
					  ((curindex == moveindex) && (curindex != 0) && (gst == &pst) && (doplaysound)) ||
					  ((curindex != moveindex) && (gst == &sst));

	gps = &gst->p[curindex];
	if (recvleng == 3) { w = 0;          j = 2; } //Hack packet to be smaller if fields is 0
					  else { w = recvbuf[2]; j = 3; }
	if (!(w&1)) { dtim = ((float)(*(unsigned char  *)&recvbuf[j]))/1024.0; j++;    }
			 else { dtim = ((float)(*(unsigned short *)&recvbuf[j]))/1024.0; j += 2; }
	fmousx = fmousy = fmousz = 0.f;
		  if (w& 4) { fmousx = (float)(*(signed short *)&recvbuf[j]); j += 2; }
	else if (w& 2) { fmousx = (float)(*(signed char  *)&recvbuf[j]); j++;    }
		  if (w&16) { fmousy = (float)(*(signed short *)&recvbuf[j]); j += 2; }
	else if (w& 8) { fmousy = (float)(*(signed char  *)&recvbuf[j]); j++;    }
	if (w&32) { fmousz = (float)(*(short *)&recvbuf[j]); j += 2; }
	gps->obstatus = gps->bstatus;
	if (w&64)
	{
		while (recvbuf[j])
		{
			if (recvbuf[j] >= 0xf0) gps->bstatus ^= (1<<(recvbuf[j]-0xf0));
									 else gps->skeystatus[recvbuf[j]>>3] ^= (1<<(recvbuf[j]&7));
			j++;
		}
		j++;
	}
	if (w&128)
	{
		switch(recvbuf[j])
		{
			case 0: case 1: //Command 0 & Command 1: Receive resolution
				gps->fullscreen = recvbuf[j];
				gps->xres = *(short *)&recvbuf[j+1];
				gps->yres = *(short *)&recvbuf[j+3]; j += 5;
				gps->ghx = (float)(gps->xres/2); gps->ghy = (float)(gps->yres/2);
				break;
			case 2: //Command 2: Receive nick
				for(i=0,j++;recvbuf[j];j++) if (i < 63) gst->nick[curindex][i++] = recvbuf[j];
				gst->nick[curindex][i] = 0; j++;
				if (curindex == moveindex) strcpy_safe(b2opts.nick,sizeof(b2opts.nick),gst->nick[curindex]);
				break;
			case 3: //Command 3: Receive filename
				j++;
				hitsect = *(long *)&recvbuf[j]; j += 4;
				hitwall = *(long *)&recvbuf[j]; j += 4;
				if ((settilefilename(hitsect,hitwall,(char *)&recvbuf[j]) >= 0) && (doplaysound) && (curindex != moveindex))
				{
					if ((unsigned)hitsect >= (unsigned)gst->numsects)
						myplaysound("sounds\\drop1.wav",100,0.25,&gps->ipos,KSND_3D);
					else if ((hitwall&0xc0000000) == 0x40000000)
						myplaysound("sounds\\hammer.wav",100,0.25,&gst->spri[hitwall&0x3fffffff].p,KSND_3D);
					else
					{
						getcentroid(sec[hitsect].wall,sec[hitsect].n,&hit.x,&hit.y);
						hit.z = getslopez(&sec[hitsect],hitwall&1,hit.x,hit.y);
						myplaysound("sounds\\hammer.wav",100,0.25,&hit,KSND_3D);
					}
				}
				j += strlen((char *)&recvbuf[j])+1;
				releasegrabdrag(curindex,doplaysound); gps->grabmode = -1;
				break;
			case 4: //Command 4: Cancel select file
				releasegrabdrag(curindex,doplaysound); gps->grabmode = -1;
				break;
			case 5: //Command 5: Receive OOS message :/
				if (!dispoos)
				{
					dispoos = 1;
					playtext("<rate speed='+0'/>Out of<pitch middle='-5'/>sync");
				}
				break;
			case 6: //Command 6: Receive initial bilin
				gps->rendinterp = recvbuf[j+1]; j += 2;
				if (curindex == moveindex) b2opts.bilin = (gps->rendinterp&1);
				break;
		}
	}
	recvcnt = j;

	gps->ghz = gps->ghx*gps->zoom;

	if (!gps->typemode)
	{
		if (gps->grabmode == GRABRGB)
		{
			if (!(gps->bstatus&2))
			{
				if (!gps->fullscreen)
				{
					gps->fcmousx += fmousx; x = (int)gps->fcmousx;
					gps->fcmousy += fmousy; y = (int)gps->fcmousy;
				}
				else
				{
					gps->fcmousx = min(max(gps->fcmousx+fmousx,0),gps->xres-1);
					gps->fcmousy = min(max(gps->fcmousy+fmousy,0),gps->yres-1);
				}
			}
			f = dtim*4.0;
			if (TESTKEY(0x37)) gps->selrgbrad = min(gps->selrgbrad*pow(1.5,(double)+f),min(gps->ghx,gps->ghy)-32); //KP*
			if (TESTKEY(0xb5)) gps->selrgbrad = max(gps->selrgbrad*pow(1.5,(double)-f),32.0); //KP/
		}
		else if (gps->editmode == 2)
		{
			if (!gps->fullscreen)
			{
				gps->fcmousx += fmousx; x = (int)gps->fcmousx;
				gps->fcmousy += fmousy; y = (int)gps->fcmousy;
			}
			else
			{
				gps->fcmousx = min(max(gps->fcmousx+fmousx,0),gps->xres-1);
				gps->fcmousy = min(max(gps->fcmousy+fmousy,0),gps->yres-1);
			}

			if (fabs(gps->grdn.z) == 1.f)
			{
				d = gps->compact2d;
				if (d < 1.0) //time graph: 0 .. d .. gps->compact2d .. 1
				{
					gps->compact2d = min(d+dtim*3.0,1.0);
					d = (gps->compact2d-d)/(1.0-d);
				} else d = dtim*6.0; //0.05;

					//slerp has precision issues when mats are close; this if ensures that it converges to goal
				if ((fabs(gps->irig.x*gps->grdu.x + gps->irig.y*gps->grdu.y + gps->irig.z*gps->grdu.z) < 0.999) ||
					 (fabs(gps->idow.x*gps->grdv.x + gps->idow.y*gps->grdv.y + gps->idow.z*gps->grdv.z) < 0.999) ||
					 (fabs(gps->ifor.x*gps->grdn.x + gps->ifor.y*gps->grdn.y + gps->ifor.z*gps->grdn.z) < 0.999))
				{
					slerp(&gps->irig,&gps->idow,&gps->ifor, &gps->grdu,&gps->grdv,&gps->grdn, &irig2,&idow2,&ifor2,d);
					gps->irig = irig2; gps->idow = idow2; gps->ifor = ifor2;
				} else { gps->irig = gps->grdu; gps->idow = gps->grdv; gps->ifor = gps->grdn; }

				f = (gps->ipos.x-gps->grdc.x)*gps->grdn.x +
					 (gps->ipos.y-gps->grdc.y)*gps->grdn.y +
					 (gps->ipos.z-gps->grdc.z)*gps->grdn.z;
				f = -gps->goalheight2d-f;
				if (fabs(f) > .01) //Attempt at eliminating unstableness
				{
					gps->npos.x = gps->grdn.x*f + gps->ipos.x;
					gps->npos.y = gps->grdn.y*f + gps->ipos.y;
					gps->npos.z = gps->grdn.z*f + gps->ipos.z;

					gps->ipos.x += (gps->npos.x-gps->ipos.x)*d;
					gps->ipos.y += (gps->npos.y-gps->ipos.y)*d;
					gps->ipos.z += (gps->npos.z-gps->ipos.z)*d;
				}
			}

			if (gps->grabmode != GRABFILE)
			{
				f = dtim*4.0;
				if (TESTKEY(0x2a)) f *= .25; //L.Shift
				if (TESTKEY(0x36)) f *= 4.0; //R.Shift
				if (TESTKEY(0x1e)|TESTKEY(0x52)) { gps->goalheight2d = max(gps->goalheight2d*pow(1.4,(double)-f), 0.25); } //A/KP0
				if (TESTKEY(0x2c)|TESTKEY(0x9d)) { gps->goalheight2d = min(gps->goalheight2d*pow(1.4,(double)+f),256.0); } //Z/RCtrl
				gps->goalheight2d = min(max(gps->goalheight2d*pow(1.4,(double)fmousz*-.01),0.25),256.0); //mouse wheel
				f *= gps->goalheight2d/4.0;
				if (TESTKEY(0xcb)) { gps->ipos.x -= gps->irig.x*f; gps->ipos.y -= gps->irig.y*f; gps->ipos.z -= gps->irig.z*f; } //Left
				if (TESTKEY(0xcd)) { gps->ipos.x += gps->irig.x*f; gps->ipos.y += gps->irig.y*f; gps->ipos.z += gps->irig.z*f; } //Right
				if (TESTKEY(0xc8)) { gps->ipos.x -= gps->idow.x*f; gps->ipos.y -= gps->idow.y*f; gps->ipos.z -= gps->idow.z*f; } //Up
				if (TESTKEY(0xd0)) { gps->ipos.x += gps->idow.x*f; gps->ipos.y += gps->idow.y*f; gps->ipos.z += gps->idow.z*f; } //Down
			}
			gps->height2d = -((gps->ipos.x-gps->grdc.x)*gps->grdn.x +
									(gps->ipos.y-gps->grdc.y)*gps->grdn.y +
									(gps->ipos.z-gps->grdc.z)*gps->grdn.z);
			gps->dgridlock = getgridsiz(gps->height2d,gps->xres);
		}
		else
		{
				//FIXFIXFIX
			//hitscan(&gps->ipos,&gps->ifor,1e32,gps->cursect,&hitsect,&hitwall,&hit);
			////d = sqrt((hit.x-gps->ipos.x)*(hit.x-gps->ipos.x) + (hit.y-gps->ipos.y)*(hit.y-gps->ipos.y) + (hit.z-gps->ipos.z)*(hit.z-gps->ipos.z));
			//d = fabs(hit.z-gps->ipos.z);
			//if (!gps->gridlock) gps->dgridlock = d/(4.0*gps->xres/768);
			//               //else gps->dgridlock = getgridsiz(d*2.0,gps->xres);

			gps->fcmousx = (float)(gps->xres>>1);
			gps->fcmousy = (float)(gps->yres>>1);

			d = gps->compact2d;
			if (d > 0.0) //time graph: 0 .. gps->compact2d .. d .. 1
			{
				gps->compact2d = max(d-dtim*3.0,0.0);
				d = 1.0 - gps->compact2d/d;

					//slerp has precision issues when mats are close; this if ensures that it converges to goal
				if ((fabs(gps->irig.x*gps->nrig.x + gps->irig.y*gps->nrig.y + gps->irig.z*gps->nrig.z) < 0.999) ||
					 (fabs(gps->idow.x*gps->ndow.x + gps->idow.y*gps->ndow.y + gps->idow.z*gps->ndow.z) < 0.999) ||
					 (fabs(gps->ifor.x*gps->nfor.x + gps->ifor.y*gps->nfor.y + gps->ifor.z*gps->nfor.z) < 0.999))
				{
					slerp(&gps->irig,&gps->idow,&gps->ifor, &gps->nrig,&gps->ndow,&gps->nfor, &irig2,&idow2,&ifor2,d);
					gps->irig = irig2; gps->idow = idow2; gps->ifor = ifor2;
				} else { gps->irig = gps->nrig; gps->idow = gps->ndow; gps->ifor = gps->nfor; }

				gps->ipos.x += (gps->npos.x-gps->ipos.x)*d;
				gps->ipos.y += (gps->npos.y-gps->ipos.y)*d;
				gps->ipos.z += (gps->npos.z-gps->ipos.z)*d;
			}

			d = fabs(atan2(gps->irig.z,gps->idow.z)); //WARNING:gps->idow.z can be 0; don't change to atan()
			orthorotate(min(max(gps->irig.z*dtim*16.0,-d),d),fmousy*.008,fmousx*.008,&gps->irig,&gps->idow,&gps->ifor);
			if (fabs(gps->irig.x) <= 1e-32) gps->irig.x = 0.0; //EVILNESS: doubles can become denormal when converted to single precision; killing fps!
			if (fabs(gps->irig.y) <= 1e-32) gps->irig.y = 0.0;
			if (fabs(gps->irig.z) <= 1e-32) gps->irig.z = 0.0;
			if (fabs(gps->idow.x) <= 1e-32) gps->idow.x = 0.0;
			if (fabs(gps->idow.y) <= 1e-32) gps->idow.y = 0.0;
			if (fabs(gps->idow.z) <= 1e-32) gps->idow.z = 0.0;
			if (fabs(gps->ifor.x) <= 1e-32) gps->ifor.x = 0.0;
			if (fabs(gps->ifor.y) <= 1e-32) gps->ifor.y = 0.0;
			if (fabs(gps->ifor.z) <= 1e-32) gps->ifor.z = 0.0;


			f = dtim*4.0; fp.x = fp.y = fp.z = 0.0;
			if (gps->grabmode != GRABFILE)
			{
				if (TESTKEY(0x2a)) f *= .25; //L.Shift
				if (TESTKEY(0x36)) f *= 4.0; //R.Shift
				if (TESTKEY(0xcb)) { fp.x -= gps->irig.x*f; fp.y -= gps->irig.y*f; fp.z -= gps->irig.z*f; } //Left
				if (TESTKEY(0xcd)) { fp.x += gps->irig.x*f; fp.y += gps->irig.y*f; fp.z += gps->irig.z*f; } //Right
				if (TESTKEY(0xc8)) { fp.x += gps->ifor.x*f; fp.y += gps->ifor.y*f; fp.z += gps->ifor.z*f; } //Up
				if (TESTKEY(0xd0)) { fp.x -= gps->ifor.x*f; fp.y -= gps->ifor.y*f; fp.z -= gps->ifor.z*f; } //Down
				if (TESTKEY(0x9d)) { fp.x -= gps->idow.x*f; fp.y -= gps->idow.y*f; fp.z -= gps->idow.z*f; } //Rt.Ctrl
				if (TESTKEY(0x52)) { fp.x += gps->idow.x*f; fp.y += gps->idow.y*f; fp.z += gps->idow.z*f; } //KP0
			}

				//Sliding collision (Note: floating point seems to be enough precision)
			if (!gps->docollide) { gps->ipos.x += fp.x; gps->ipos.y += fp.y; gps->ipos.z += fp.z; }
			else collmove(&gps->ipos,&gps->cursect,&fp,0.25,1);
			updatesect(gps->ipos.x,gps->ipos.y,gps->ipos.z,&gps->cursect);

			if (gps->grabmode != GRABFILE)
			{
				if (TESTKEY(0x37)) //KP*
				{
					gps->zoom = min(gps->zoom*pow(1.5,(double)+f),1.0*8.0);
					if ((gps->zoom >= 1.f) && (gps->ozoom < 1.f)) gps->zoom = 1.f;
				}
				if (TESTKEY(0xb5)) //KP/
				{
					gps->zoom = max(gps->zoom*pow(1.5,(double)-f),1.0/8.0);
					if ((gps->zoom <= 1.f) && (gps->ozoom > 1.f)) gps->zoom = 1.f;
				}
				if (!(TESTKEY(0x37)|TESTKEY(0xb5))) gps->ozoom = gps->zoom;
			}
		}
	}

	while (recvcnt < recvleng)
	{
		key = ((*(long *)&recvbuf[recvcnt])&0xffffff); recvcnt += 3;
		if (gps->typemode)
		{
			cptr = gst->typemess[curindex];
			j = strlen(cptr);
			if ((key&255) && (!(key&0x3c0000)))
			{
				switch(key&255)
				{
					case 27: gps->typemode = 0; cptr[0] = 0; break; //ESC
					case 13: gps->typemode = 0; //Enter here
						if ((logfilnam[0]) && (gst == &sst))
						{
							FILE *fil;
							time_t ltime;
							char tbuf[16];

							fil = fopen(logfilnam,"a");
							time(&ltime);
							strftime(tbuf,sizeof(tbuf),"%H:%M:%S",localtime(&ltime));
							if (!gst->nick[curindex][0]) fprintf(fil,"%s Player %d: %s\n",tbuf,curindex,cptr);
															else fprintf(fil,"%s %s: %s\n",tbuf,gst->nick[curindex],cptr);
							fclose(fil);
						}

						if (cptr[0] == '/')
						{
							if (((!memicmp(&cptr[1],"name",4)) ||
								  (!memicmp(&cptr[1],"nick",4))) && ((cptr[5] == '=') || (cptr[5] == ':') || (cptr[5] == ' ')))
							{
								strcpy_safe(gst->nick[curindex],sizeof(gst->nick[0]),&cptr[6]);
								if (curindex == moveindex) strcpy_safe(b2opts.nick,sizeof(b2opts.nick),&cptr[6]);
							}
							if ((!memicmp(&cptr[1],"say",3)) && ((cptr[4] == '=') || (cptr[4] == ':') || (cptr[4] == ' ')))
							{
								if (doplaysound) playtext(&cptr[5]);
								cptr[0] = 0; break;
							}
							if (!memicmp(&cptr[1],"keyremap[",9))
							{
								int o, n;
								if (curindex != moveindex) break;
								if (keyremap_getnums(&cptr[10],&o,&n))
									if ((unsigned)o < sizeof(keyscanremap)/sizeof(keyscanremap[0]))
										{ keyscanremap[o] = n; keyscanremapr[n] = o; }
								break;
							}
							if (!memicmp(&cptr[1],"startpos",8)) { gps->ipos = gst->startpos; gps->irig = gst->startrig; gps->idow = gst->startdow; gps->ifor = gst->startfor; cptr[0] = 0; break; }
							if (!memicmp(&cptr[1],"oriquant",8)) { oriquant(&gps->irig,&gps->idow,&gps->ifor); cptr[0] = 0; break; }
							if (!memicmp(&cptr[1],"posquant",8)) { gps->ipos.x = floor(gps->ipos.x+.5); gps->ipos.y = floor(gps->ipos.y+.5); gps->ipos.z = floor(gps->ipos.z+.5); cptr[0] = 0; break; }
							if ((!memicmp(&cptr[1],"x",1)) && ((cptr[2] == '=') || (cptr[2] == ':') || (cptr[2] == ' ')))
							{
								if ((unsigned)gps->grabsect < (unsigned)gst->numsects)
								{
									x0 = atof(&cptr[3]);
									if ((unsigned)(gps->grabwall-0x40000000) < (unsigned)gst->numsects)
										gst->spri[gps->grabwall&0x3fffffff].p.x = x0;
									else if ((unsigned)gps->grabwall < (unsigned)gst->sect[gps->grabsect].n)
									{
										for(i=getverts(gps->grabsect,gps->grabwall,verts,MAXVERTS)-1;i>=0;i--)
											gst->sect[verts[i].s].wall[verts[i].w].x = x0;
									}
								}
								cptr[0] = 0; break;
							}
							if ((!memicmp(&cptr[1],"y",1)) && ((cptr[2] == '=') || (cptr[2] == ':') || (cptr[2] == ' ')))
							{
								if ((unsigned)gps->grabsect < (unsigned)gst->numsects)
								{
									y0 = atof(&cptr[3]);
									if ((unsigned)(gps->grabwall-0x40000000) < (unsigned)gst->numsects)
										gst->spri[gps->grabwall&0x3fffffff].p.y = y0;
									else if ((unsigned)gps->grabwall < (unsigned)gst->sect[gps->grabsect].n)
									{
										for(i=getverts(gps->grabsect,gps->grabwall,verts,MAXVERTS)-1;i>=0;i--)
											gst->sect[verts[i].s].wall[verts[i].w].y = y0;
									}
								}
								cptr[0] = 0; break;
							}
							if ((!memicmp(&cptr[1],"z",1)) && ((cptr[2] == '=') || (cptr[2] == ':') || (cptr[2] == ' ')))
							{
								if ((unsigned)gps->grabsect < (unsigned)gst->numsects)
								{
									if ((unsigned)(gps->grabwall-0x40000000) < (unsigned)gst->numsects)
										gst->spri[gps->grabwall&0x3fffffff].p.z = atof(&cptr[3]);
									else if (gps->grabwall < 0)
										gst->sect[gps->grabsect].z[gps->grabcf&1] = atof(&cptr[3]);
								}
								cptr[0] = 0; break;
							}
							if ((!memicmp(&cptr[1],"tag",3)) && ((cptr[4] == '=') || (cptr[4] == ':') || (cptr[4] == ' ')))
							{
								if ((unsigned)gps->grabsect < (unsigned)gst->numsects)
								{
									char *ptr = &cptr[5];
									short lotag = 0, hitag = 0, pal = 0;

									// Parse lotag
									lotag = atol(ptr);

									// Find next separator
									while (*ptr && *ptr != ',' && *ptr != ' ' && *ptr != '.') ptr++;
									while (*ptr && (*ptr == ',' || *ptr == ' ' || *ptr == '.')) ptr++; // skip all separators

									// Parse hitag if present
									if (*ptr && (*ptr >= '0' && *ptr <= '9' || *ptr == '-'))
									{
										hitag = atol(ptr);
										while (*ptr && *ptr != ',' && *ptr != ' ' && *ptr != '.') ptr++;
										while (*ptr && (*ptr == ',' || *ptr == ' ' || *ptr == '.')) ptr++; // skip all separators
									}

									// Parse pal if present
									if (*ptr && (*ptr >= '0' && *ptr <= '9'))
									{
										pal = atol(ptr);
									}

									if ((unsigned)(gps->grabwall-0x40000000) < (unsigned)gst->numspris) {
										gst->spri[gps->grabwall&0x3fffffff].lotag = lotag;
										gst->spri[gps->grabwall&0x3fffffff].hitag = hitag;
									}
									else if ((unsigned)gps->grabwall < (unsigned)gst->sect[gps->grabsect].n) {
										gst->sect[gps->grabsect].wall[gps->grabwall].surf.lotag = lotag;
										gst->sect[gps->grabsect].wall[gps->grabwall].surf.hitag = hitag;
									}
									else {
										gst->sect[gps->grabsect].surf[gps->grabcf&1].lotag = lotag;
										gst->sect[gps->grabsect].surf[gps->grabcf&1].hitag = hitag;
									}
									myplaysound("sounds\\drop1.wav",100,1.0,0,0);
								}
								cptr[0] = 0; break;
							}
							if ((!memicmp(&cptr[1],"texang",6)) && ((cptr[7] == '=') || (cptr[7] == ':') || (cptr[7] == ' '))) //FIXFIXFIXFIX
							{
								if ((unsigned)gps->grabsect < (unsigned)gst->numsects)
								{
									if ((unsigned)(gps->grabwall-0x40000000) < (unsigned)gst->numspris)
										sur = 0;
									else if ((unsigned)gps->grabwall < (unsigned)gst->sect[gps->grabsect].n)
										sur = &gst->sect[gps->grabsect].wall[gps->grabwall].surf;
									else
										sur = &gst->sect[gps->grabsect].surf[gps->grabcf&1];
									if (sur)
									{
										f = atof(&cptr[8])*PI/180.0;
										sur->uv[0].x = 0.0;
										sur->uv[0].y = 0.0;
										sur->uv[1].x = cos(f);
										sur->uv[1].y = sin(f);
										sur->uv[2].x =-sin(f);
										sur->uv[2].y = cos(f);
										myplaysound("sounds\\drop1.wav",100,1.0,0,0);
									}
								}
								cptr[0] = 0; break;
							}
							if ((!memicmp(&cptr[1],"bilin",5)) && ((cptr[7] == '=') || (cptr[7] == ':') || (cptr[7] == ' ') || (!cptr[6])))
							{
								if (curindex != moveindex) break;
								if (cptr[6]) b2opts.bilin = min(max(atol(&cptr[6]),0),1);
								if (!b2opts.bilin) playtext("nearest filter"); else playtext("bilinear filter");
								break;
							}
							if ((!memicmp(&cptr[1],"numcpu",6)) && ((cptr[7] == '=') || (cptr[7] == ':') || (cptr[7] == ' ') || (!cptr[7])))
							{
								if (curindex != moveindex) break;
								if (cptr[7])
								{
									i = min(max(atol(&cptr[8]),1),MAXCPU);
									b2opts.drawthreads = i;
									drawpoly_numcpu = i;
									shadowtest2_numcpu = i;
								}
								break;
							}
							if ((!memicmp(&cptr[1],"lights",6)) && ((cptr[7] == '=') || (cptr[7] == ':') || (cptr[7] == ' ') || (!cptr[7])))
							{
								if (curindex != moveindex) break;
								if (cptr[7]) b2opts.lights = min(max(atol(&cptr[8]),0),1);
								if (!b2opts.lights) playtext("lights off"); else playtext("lights on");
								break;
							}
							if ((!memicmp(&cptr[1],"shadows",7)) && ((cptr[8] == '=') || (cptr[8] == ':') || (cptr[8] == ' ') || (!cptr[8])))
							{
								if (curindex != moveindex) break;
								if (cptr[8]) b2opts.shadows = min(max(atol(&cptr[9]),0),1);
								if (!b2opts.shadows) playtext("shadows off"); else playtext("shadows on");
								shadowtest2_updatelighting = 1;
								break;
							}
							if (!memicmp(&cptr[1],"showoverlaps",12))
							{
									//generate bounded box info as temp for optimization
								for(i=gst->numsects-1;i>=0;i--)
								{
									wal = sec[i].wall;
									sec[i].minx = wal[0].x; sec[i].maxx = wal[0].x;
									sec[i].miny = wal[0].y; sec[i].maxy = wal[0].y;
									for(j=sec[i].n-1;j>0;j--)
									{
										if (wal[j].x < sec[i].minx) sec[i].minx = wal[j].x;
										if (wal[j].y < sec[i].miny) sec[i].miny = wal[j].y;
										if (wal[j].x > sec[i].maxx) sec[i].maxx = wal[j].x;
										if (wal[j].y > sec[i].maxy) sec[i].maxy = wal[j].y;
									}
								}

								bs = gst->numsects;
								for(i=bs-1;i>0;i--)
									for(j=i-1;j>=0;j--)
									{
										wall_t *nwal[5];
										long ind[4], waln[5], seci[2], osectn, nsectn, newsec[2];

										if ((sec[i].minx >= sec[j].maxx) || (sec[i].miny >= sec[j].maxy) ||
											 (sec[j].minx >= sec[i].maxx) || (sec[j].miny >= sec[i].maxy)) continue;
										if (!polybool(sec[i].wall,sec[i].n,sec[j].wall,sec[j].n,&nwal[0],(int *)&waln[0],POLYBOOL_AND)) continue;

											//max new sectors per sector vs. sector is (A-B) + (B-A) + (A&B)*(11/*max regions of 4 split lines*/)
										if (gst->numsects+2+11 > gst->malsects)
										{
											k = gst->malsects; gst->malsects = max(gst->numsects+2+11,gst->malsects<<1);
											sec = gst->sect = (sect_t *)realloc(gst->sect,gst->malsects*sizeof(sect_t));
											memset(&sec[k],0,(gst->malsects-k)*sizeof(sect_t));
										}

											//FIXFIXFIX
										osectn = gst->numsects;
										for(ind[0]=2-1;ind[0]>=0;ind[0]--)
										{
											waln[1] = polyspli2(nwal[0],waln[0],&nwal[1],i,0,j,0,ind[0]); if (!waln[1]) continue;
											for(ind[1]=2-1;ind[1]>=0;ind[1]--)
											{
												waln[2] = polyspli2(nwal[1],waln[1],&nwal[2],i,0,j,1,ind[1]); if (!waln[2]) continue;
												for(ind[2]=2-1;ind[2]>=0;ind[2]--)
												{
													waln[3] = polyspli2(nwal[2],waln[2],&nwal[3],i,1,j,0,ind[2]); if (!waln[3]) continue;
													for(ind[3]=2-1;ind[3]>=0;ind[3]--)
													{
														waln[4] = polyspli2(nwal[3],waln[3],&nwal[4],i,1,j,1,ind[3]); if (!waln[4]) continue;

														switch(ind[0]*8 + ind[1]*4 + ind[2]*2 + ind[3])
														{
															case  0: seci[0] = i; seci[1] = j; break; //fukm.map:test:jc+=11,jf+=4
															case  1: seci[0] = -1; break;
															case  2: seci[0] = i; seci[1] = j; break; //fukm.map:SE corner
															case  3: seci[0] = i; seci[1] = i; break; //fukm.map:SW corner
															case  4: seci[0] = -1; break;
															case  5: seci[0] = -1; break;
															case  6: seci[0] = -1; break;
															case  7: seci[0] = -1; break;
															case  8: seci[0] = -1; break;
															case  9: seci[0] = -1; break;
															case 10: seci[0] = j; seci[1] = j; break; //fukm.map:NE corner
															case 11: seci[0] = j; seci[1] = i; break; //fukm.map:NW corner
															case 12: seci[0] = -1; break;
															case 13: seci[0] = -1; break;
															case 14: seci[0] = -1; break;
															case 15: seci[0] = j; seci[1] = i; break; //fukm.map:test:jc-=4,jf-=10
														}
														if (seci[0] < 0) { free((void *)nwal[4]); continue; }

														s = gst->numsects; sec[s].n = waln[4]; sec[s].nmax = waln[4]; sec[s].wall = nwal[4];
														for(k=2-1;k>=0;k--)
														{
															sec[s].z[k] = getslopez(&sec[seci[k]],k,sec[s].wall[0].x,sec[s].wall[0].y);
															sec[s].grad[k].x = sec[seci[k]].grad[k].x;
															sec[s].grad[k].y = sec[seci[k]].grad[k].y;
															sec[s].surf[k] = sec[seci[k]].surf[k];
														}
														sec[s].headspri = -1; sec[s].owner = -1; //sec[s].owner = curindex;
														gst->numsects++;
													}
													free((void *)nwal[3]);
												}
												free((void *)nwal[2]);
											}
											free((void *)nwal[1]);
										}
										free((void *)nwal[0]);
										nsectn = gst->numsects;

										if (polybool(sec[i].wall,sec[i].n,sec[j].wall,sec[j].n,&nwal[1],(int *)&waln[1],POLYBOOL_SUB))
										{
											seci[0] = i; seci[1] = i; newsec[0] = s;
											s = gst->numsects; sec[s].n = waln[1]; sec[s].nmax = waln[1]; sec[s].wall = nwal[1];

											for(bs=osectn;bs<nsectn;bs++)
												for(bw=sec[bs].n-1;bw>=0;bw--) //FIXFIXFIX
												{
													int nw;

													dx = sec[bs].wall[bw].x;
													dy = sec[bs].wall[bw].y;

													for(w=waln[1]-1;w>=0;w--) if ((dx == nwal[1][w].x) && (dy == nwal[1][w].y)) break;
													if (w >= 0) continue;

													for(w=waln[1]-1;w>=0;w--)
													{
														nw = nwal[1][w].n+w;
														if ((nwal[1][w].x < dx) != (dx < nwal[1][nw].x)) continue;
														if ((nwal[1][w].y < dy) != (dy < nwal[1][nw].y)) continue;
														if ((nwal[1][nw].x-dx)*(nwal[1][w].y-dy) == (nwal[1][w].x-dx)*(nwal[1][nw].y-dy))
														{
															w = dupwall(&sec[s],w);
															sec[s].wall[w].x = dx;
															sec[s].wall[w].y = dy;
															break;
														}
													}
												}

											for(k=2-1;k>=0;k--)
											{
												sec[s].z[k] = getslopez(&sec[seci[k]],k,sec[s].wall[0].x,sec[s].wall[0].y);
												sec[s].grad[k].x = sec[seci[k]].grad[k].x;
												sec[s].grad[k].y = sec[seci[k]].grad[k].y;
												sec[s].surf[k] = sec[seci[k]].surf[k];
											}
											sec[s].headspri = -1; sec[s].owner = -1; //sec[s].owner = curindex;
											gst->numsects++;
										} else newsec[0] = -1;

										if (polybool(sec[j].wall,sec[j].n,sec[i].wall,sec[i].n,&nwal[1],(int *)&waln[1],POLYBOOL_SUB))
										{
											seci[0] = j; seci[1] = j; newsec[1] = s;
											s = gst->numsects; sec[s].n = waln[1]; sec[s].nmax = waln[1]; sec[s].wall = nwal[1];

											for(bs=osectn;bs<nsectn;bs++)
												for(bw=sec[bs].n-1;bw>=0;bw--) //FIXFIXFIX
												{
													int nw;

													dx = sec[bs].wall[bw].x;
													dy = sec[bs].wall[bw].y;

													for(w=waln[1]-1;w>=0;w--) if ((dx == nwal[1][w].x) && (dy == nwal[1][w].y)) break;
													if (w >= 0) continue;

													for(w=waln[1]-1;w>=0;w--)
													{
														nw = nwal[1][w].n+w;
														if ((nwal[1][w].x < dx) != (dx < nwal[1][nw].x)) continue;
														if ((nwal[1][w].y < dy) != (dy < nwal[1][nw].y)) continue;
														if ((nwal[1][nw].x-dx)*(nwal[1][w].y-dy) == (nwal[1][w].x-dx)*(nwal[1][nw].y-dy))
														{
															w = dupwall(&sec[s],w);
															sec[s].wall[w].x = dx;
															sec[s].wall[w].y = dy;
															break;
														}
													}
												}

											for(k=2-1;k>=0;k--)
											{
												sec[s].z[k] = getslopez(&sec[seci[k]],k,sec[s].wall[0].x,sec[s].wall[0].y);
												sec[s].grad[k].x = sec[seci[k]].grad[k].x;
												sec[s].grad[k].y = sec[seci[k]].grad[k].y;
												sec[s].surf[k] = sec[seci[k]].surf[k];
											}
											sec[s].headspri = -1; sec[s].owner = -1; //sec[s].owner = curindex;
											gst->numsects++;
										} else newsec[1] = -1;

											//Delete old sectors
										if (newsec[1] >= 0) { gst->numsects--; memcpy(&sec[j],&sec[gst->numsects],sizeof(sect_t)); memset(&sec[gst->numsects],0,sizeof(sect_t)); }
										if (newsec[0] >= 0) { gst->numsects--; memcpy(&sec[i],&sec[gst->numsects],sizeof(sect_t)); memset(&sec[gst->numsects],0,sizeof(sect_t)); }

										checknextwalls(); checksprisect(-1);
									}
								break;
							}
							if (!memicmp(&cptr[1],"autojoinall",11))
							{
								bs = gst->numsects;
								for(i=gst->numsects-1;i>0;i--)
									for(j=i-1;j>=0;j--)
									{
										if (!sect_isneighs(i,j)) continue;

										for(k=2-1;k>=0;k--)
										{
												//Geometry match test..
											if (fabs(sec[i].grad[k].x-sec[j].grad[k].x) > 1e-4) break;
											if (fabs(sec[i].grad[k].y-sec[j].grad[k].y) > 1e-4) break;
											if (fabs((sec[i].wall[0].x-sec[j].wall[0].x)*sec[i].grad[k].x +
														(sec[i].wall[0].y-sec[j].wall[0].y)*sec[i].grad[k].y - (sec[j].z[k]-sec[i].z[k])) >= 1e-2) break;

												//Texture match test..
											if (sec[i].surf[k].tilnum  != sec[j].surf[k].tilnum ) break;
											if (sec[i].surf[k].tilanm  != sec[j].surf[k].tilanm ) break;
											if (sec[i].surf[k].flags   != sec[j].surf[k].flags  ) break;
											if (sec[i].surf[k].tag     != sec[j].surf[k].tag    ) break;
											if (labs(sec[i].surf[k].asc-sec[j].surf[k].asc) > 64) break;
											if (labs(sec[i].surf[k].rsc-sec[j].surf[k].rsc) > 64) break;
											if (labs(sec[i].surf[k].gsc-sec[j].surf[k].gsc) > 64) break;
											if (labs(sec[i].surf[k].bsc-sec[j].surf[k].bsc) > 64) break;
											if (sec[i].surf[k].uv[0].x != sec[j].surf[k].uv[0].x) break;
											if (sec[i].surf[k].uv[0].y != sec[j].surf[k].uv[0].y) break;
											if (sec[i].surf[k].uv[1].x != sec[j].surf[k].uv[1].x) break;
											if (sec[i].surf[k].uv[1].y != sec[j].surf[k].uv[1].y) break;
											if (sec[i].surf[k].uv[2].x != sec[j].surf[k].uv[2].x) break;
											if (sec[i].surf[k].uv[2].y != sec[j].surf[k].uv[2].y) break;

											if (sec[i].surf[k].tag) break; //unintended things can happen when messing with tags :/
											if (sec[j].surf[k].tag) break;
										}
										if (k >= 0) continue;

										if (!polybool(sec[i].wall,sec[i].n,sec[j].wall,sec[j].n,&wal,&w,POLYBOOL_OR)) continue;

										if (doplaysound) { myplaysound("sounds\\deleteroom.wav",100,1.0,&fp,KSND_3D); }

											//j takes both (i&new); i is deleted
										for(k=0;k<2;k++) sec[j].z[k] = getslopez(&sec[j],k,wal[0].x,wal[0].y);
										free(sec[j].wall);
										sec[j].n = w; sec[j].nmax = w;
										sec[j].wall = wal;
										delsect(i);

										checknextwalls(); checksprisect(-1);
										break;
									}
								if (doplaysound)
								{
										  if (gst->numsects == bs  ) sprintf(message,"no sectors eliminated");
									else if (gst->numsects == bs-1) sprintf(message,"1 sector eliminated");
									else                            sprintf(message,"%d sectors eliminated",bs-gst->numsects);
									messagetimeout = dtotclk+3.0;
									playtext(message);
								}
								break;
							}
							if (!stricmp(&cptr[1],"nose"))   { gps->emoticon_nose ^= 1; }
							if (!stricmp(&cptr[1],"smile"))  { gps->emoticon_mouth = 1; }
							if (!stricmp(&cptr[1],"frown"))  { gps->emoticon_mouth = 2; }
							if (!stricmp(&cptr[1],"slant"))  { gps->emoticon_mouth = 3; }
							if (!stricmp(&cptr[1],"male"))   { gps->emoticon_hair = 0; }
							if (!stricmp(&cptr[1],"female")) { gps->emoticon_hair = 1; }
						}
						else if (!stricmp(cptr,"o->")) { gps->emoticon_hair = 0; }
						else if (!stricmp(cptr,"o-+")) { gps->emoticon_hair = 1; }
						else if (!strcmp(cptr,"8:-)")) { gps->emoticon_hair = 1; }
						else if (!strcmp(cptr,"o.o")) { gps->emoticon_eyes = (gps->emoticon_eyes&~3)+0; }
						else if (!strcmp(cptr,"o.O")) { gps->emoticon_eyes = (gps->emoticon_eyes&~3)+1; }
						else if (!strcmp(cptr,"O.o")) { gps->emoticon_eyes = (gps->emoticon_eyes&~3)+2; }
						else if (!strcmp(cptr,"O.O")) { gps->emoticon_eyes = (gps->emoticon_eyes&~3)+3; }
						else if (!strcmp(cptr,"o_o")) { gps->emoticon_eyes = (gps->emoticon_eyes&~3)+0; }
						else if (!strcmp(cptr,"o_O")) { gps->emoticon_eyes = (gps->emoticon_eyes&~3)+1; }
						else if (!strcmp(cptr,"O_o")) { gps->emoticon_eyes = (gps->emoticon_eyes&~3)+2; }
						else if (!strcmp(cptr,"O_O")) { gps->emoticon_eyes = (gps->emoticon_eyes&~3)+3; }
						else if (!strcmp(cptr,":-|")) { gps->emoticon_nose = 0; gps->emoticon_mouth = 0; gps->emoticon_eyes = (gps->emoticon_eyes&~12); }
						else if (!strcmp(cptr,":-)")) { gps->emoticon_nose = 0; gps->emoticon_mouth = 1; gps->emoticon_eyes = (gps->emoticon_eyes&~12); }
						else if (!strcmp(cptr,":-(")) { gps->emoticon_nose = 0; gps->emoticon_mouth = 2; gps->emoticon_eyes = (gps->emoticon_eyes&~12); }
						else if (!strcmp(cptr,":-/")) { gps->emoticon_nose = 0; gps->emoticon_mouth = 3; gps->emoticon_eyes = (gps->emoticon_eyes&~12); }
						else if (!strcmp(cptr,":-P")) { gps->emoticon_nose = 0; gps->emoticon_mouth = 4; gps->emoticon_eyes = (gps->emoticon_eyes&~12); }
						else if (!strcmp(cptr,":|"))  { gps->emoticon_nose = 1; gps->emoticon_mouth = 0; gps->emoticon_eyes = (gps->emoticon_eyes&~12); }
						else if (!strcmp(cptr,":)"))  { gps->emoticon_nose = 1; gps->emoticon_mouth = 1; gps->emoticon_eyes = (gps->emoticon_eyes&~12); }
						else if (!strcmp(cptr,":("))  { gps->emoticon_nose = 1; gps->emoticon_mouth = 2; gps->emoticon_eyes = (gps->emoticon_eyes&~12); }
						else if (!strcmp(cptr,":/"))  { gps->emoticon_nose = 1; gps->emoticon_mouth = 3; gps->emoticon_eyes = (gps->emoticon_eyes&~12); }
						else if (!strcmp(cptr,":P"))  { gps->emoticon_nose = 1; gps->emoticon_mouth = 4; gps->emoticon_eyes = (gps->emoticon_eyes&~12); }
						else if (!strcmp(cptr,";-|")) { gps->emoticon_nose = 0; gps->emoticon_mouth = 0; gps->emoticon_eyes = (gps->emoticon_eyes&~12)|8; }
						else if (!strcmp(cptr,";-)")) { gps->emoticon_nose = 0; gps->emoticon_mouth = 1; gps->emoticon_eyes = (gps->emoticon_eyes&~12)|8; }
						else if (!strcmp(cptr,";-(")) { gps->emoticon_nose = 0; gps->emoticon_mouth = 2; gps->emoticon_eyes = (gps->emoticon_eyes&~12)|8; }
						else if (!strcmp(cptr,";-/")) { gps->emoticon_nose = 0; gps->emoticon_mouth = 3; gps->emoticon_eyes = (gps->emoticon_eyes&~12)|8; }
						else if (!strcmp(cptr,";-P")) { gps->emoticon_nose = 0; gps->emoticon_mouth = 4; gps->emoticon_eyes = (gps->emoticon_eyes&~12)|8; }
						else if (!strcmp(cptr,";|"))  { gps->emoticon_nose = 1; gps->emoticon_mouth = 0; gps->emoticon_eyes = (gps->emoticon_eyes&~12)|8; }
						else if (!strcmp(cptr,";)"))  { gps->emoticon_nose = 1; gps->emoticon_mouth = 1; gps->emoticon_eyes = (gps->emoticon_eyes&~12)|8; }
						else if (!strcmp(cptr,";("))  { gps->emoticon_nose = 1; gps->emoticon_mouth = 2; gps->emoticon_eyes = (gps->emoticon_eyes&~12)|8; }
						else if (!strcmp(cptr,";/"))  { gps->emoticon_nose = 1; gps->emoticon_mouth = 3; gps->emoticon_eyes = (gps->emoticon_eyes&~12)|8; }
						else if (!strcmp(cptr,";P"))  { gps->emoticon_nose = 1; gps->emoticon_mouth = 4; gps->emoticon_eyes = (gps->emoticon_eyes&~12)|8; }
						else if (!strcmp(cptr,"^^"))  { gps->emoticon_eyes |= 12; }
						else
						{
							for(i=0;cptr[i];i++) if (cptr[i] > 32) break;
							if (cptr[i])
							{
								strcpy_safe(gst->chatmess[gst->chatmessn&(TYPEMESSNUM-1)],TYPEMESSLENG,cptr);
								gst->chatmessowner[gst->chatmessn&(TYPEMESSNUM-1)] = curindex;
								gst->chatmessn++;
							}

							if ((curindex != moveindex) || (numplayers < 2))
							{
								if (cptr[0] != '/') playtext(cptr); else
								myplaysound("sounds\\drop1.wav",100,1.0,0,0);
							}
						}

						cptr[0] = 0;
						break;
					case 8: //Backspace
						if (gps->typehighlight >= 0)
						{
							x = min(gps->typehighlight,gps->typecurs);
							y = max(gps->typehighlight,gps->typecurs);
							memmove(&cptr[x],&cptr[y],strlen(&cptr[y])+1);
							gps->typecurs = x;
							gps->typehighlight = -1;
						}
						else if (gps->typecurs > 0)
						{
							if (curindex != moveindex) myplaysound("sounds\\key.wav",100,1.25,0,0);
							for(k=gps->typecurs-1;k<=j;k++) cptr[k] = cptr[k+1];
							gps->typecurs--;
						}
						break;
					default:
						if ((((key&255) < 32) && ((key&255) != 9)) || (j >= TYPEMESSLENG-1)) break;

						if (gps->typehighlight >= 0)
						{
							x = min(gps->typehighlight,gps->typecurs);
							y = max(gps->typehighlight,gps->typecurs);
							memmove(&cptr[x],&cptr[y],strlen(&cptr[y])+1);
							gps->typecurs = x;
							gps->typehighlight = -1;
						}

						if ((!gps->typeowritemode) || (gps->typecurs >= j))
						{
							j++;
							for(;j>gps->typecurs;j--) cptr[j] = cptr[j-1];
						}
						if (curindex != moveindex) myplaysound("sounds\\key.wav",100,1.0-((double)((key&255)==32))*.25,0,0);
						cptr[gps->typecurs] = (key&255);
						gps->typecurs++;
						break;
				}
			}
			else
			{
				switch((key>>8)&0xff)
				{
					case 0xd2: //Insert
						if (key&0xc0000) //Ctrl+Insert
						{
							if (gps->typehighlight >= 0)
							{
								if (curindex == moveindex) //Copy to clipboard, but only if from local machine!
								{
									x = min(gps->typehighlight,gps->typecurs);
									y = max(gps->typehighlight,gps->typecurs);
									ch = cptr[y]; cptr[y] = 0; setclipboardtext(&cptr[x]); cptr[y] = ch;
								}
								gps->typehighlight = -1;
							}
						}
						else if (!(key&0x30000)) //Note: Shift+Insert handled in getnsendinput!
							{ gps->typeowritemode ^= 1; }
						break;
					case 0xd3: //Delete
						if (gps->typehighlight >= 0)
						{
							x = min(gps->typehighlight,gps->typecurs);
							y = max(gps->typehighlight,gps->typecurs);
							memmove(&cptr[x],&cptr[y],strlen(&cptr[y])+1);
							gps->typecurs = x;
							gps->typehighlight = -1;
						}
						else
						{
							if (!cptr[gps->typecurs]) break;
							memmove(&cptr[gps->typecurs],&cptr[gps->typecurs+1],strlen(&cptr[gps->typecurs+1])+1);
						}
						break;
					case 0xcb: //Left
						if (key&0x30000) //Shift+Left
						{
							if (gps->typehighlight < 0) gps->typehighlight = gps->typecurs;
						} else gps->typehighlight = -1;
						if (key&0xc0000) //Ctrl+Left
						{
							i = gps->typecurs;
							while ((i >= 0) && (((cptr[i] >= '0') && (cptr[i] <= '9')) ||
													  ((cptr[i] >= 'A') && (cptr[i] <= 'Z')) ||
													  ((cptr[i] >= 'a') && (cptr[i] <= 'z')))) i--;
							while ((i >= 0) && (((cptr[i] <  '0') || (cptr[i] >  '9')) &&
													  ((cptr[i] <  'A') || (cptr[i] >  'Z')) &&
													  ((cptr[i] <  'a') || (cptr[i] >  'z')))) i--;
							while ((i >= 0) && (((cptr[i] >= '0') && (cptr[i] <= '9')) ||
													  ((cptr[i] >= 'A') && (cptr[i] <= 'Z')) ||
													  ((cptr[i] >= 'a') && (cptr[i] <= 'z')))) i--;
							gps->typecurs = i+1;
						}
						else
						{
							if (gps->typecurs > 0) gps->typecurs--;
						}
						break;
					case 0xcd: //Right
						if (key&0x30000) //Shift+Left
						{
							if (gps->typehighlight < 0) gps->typehighlight = gps->typecurs;
						} else gps->typehighlight = -1;
						if (key&0xc0000) //Ctrl+Right
						{
							i = gps->typecurs;
							while ((((cptr[i] >= '0') && (cptr[i] <= '9')) ||
									  ((cptr[i] >= 'A') && (cptr[i] <= 'Z')) ||
									  ((cptr[i] >= 'a') && (cptr[i] <= 'z'))) && (cptr[i])) i++;
							while ((((cptr[i] <  '0') || (cptr[i] >  '9')) &&
									  ((cptr[i] <  'A') || (cptr[i] >  'Z')) &&
									  ((cptr[i] <  'a') || (cptr[i] >  'z'))) && (cptr[i])) i++;
							gps->typecurs = i;
						}
						else
						{
							if (cptr[gps->typecurs]) gps->typecurs++;
						}
						break;
					case 0xc7: //Home
						if (key&0x30000) //Shift+Home
						{
							if (gps->typehighlight < 0) gps->typehighlight = gps->typecurs;
						} else gps->typehighlight = -1;
						gps->typecurs = 0;
						break;
					case 0xcf: //End
						if (key&0x30000) //Shift+End
						{
							if (gps->typehighlight < 0) gps->typehighlight = gps->typecurs;
						} else gps->typehighlight = -1;
						gps->typecurs = strlen(cptr);
						break;
					default: break;
				}
			}
			continue;
		}

		//switch((key>>8)&255) //Hack for KP246893 working when Numlock is off
		//{
		//   case 0x4b: case 0x4d: case 0x48: case 0x50: case 0x49: case 0x51: key &= ~255;
		//   default: break;
		//}
		if (!(key&0x3c0000))
		{
			if (((key&255) == 'Y') || ((key&255) == 'y')) //'Y','y'
				if ((dtotclk < messagetimeout) && (!stricmp(message,"Press Y to quit!"))) { doquitloop = 1; return; }

			switch((key>>8)&255)
			{
				case 0x01: //27,ESC
					if (gps->grabmode == GRABRGB)
					{
						if ((gps->grabwall&0xc0000000) != 0x40000000)
						{
							if (gps->grabwall < 0) sur = &sec[gps->grabsect].surf[gps->grabcf&1];
													else sur = &sec[gps->grabsect].wall[gps->grabwall].surf;
							sur->rsc = gps->selrgbbak[1]; //Undo color changes
							sur->gsc = gps->selrgbbak[2];
							sur->bsc = gps->selrgbbak[3];
						}
						gps->grabmode = -1;
						break;
					}
					if (gps->grabmode >= 0) { releasegrabdrag(curindex,doplaysound); gps->grabsect = -1; gps->grabwall = -1; gps->grabmode = -1; break; }
					strcpy(message,"Press Y to quit!"); messagetimeout = dtotclk+3.0;
					break; //ESC
				case 0x29: //'`'
					gps->showdebug = !gps->showdebug;
					break;
				case 0x0e: //8,Backspace
					delwall(&gps->sec,gps->sec.n-1);
					break;
				case 0x0f: //9,Tab
					if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; }
					else getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
					if ((unsigned)hitsect >= (unsigned)gst->numsects) break;
					if ((hitwall&0xc0000000) == 0x40000000)
					{
						gps->copyspri[0] = gst->spri[hitwall&0x3fffffff];
						gps->gotcopy |= 2;
					}
					else
					{
						if (hitwall < 0)
							  { gps->copysurf[0] = sec[hitsect].surf[hitwall&1];    gps->gotcopy |= 1; }
						else { gps->copysurf[0] = sec[hitsect].wall[hitwall].surf; gps->gotcopy |= 1; }
					}
					break;
				case 0x1c: case 0x9c: //13,Enter
					if (gps->grabmode == GRABRGB) { gps->grabmode = -1; break; }

					if (((key>>8)&255) == 0x1c) //Left Enter
					{
						if (gps->gotcopy)
						{
							if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; }
							else getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
							if ((unsigned)hitsect >= (unsigned)gst->numsects) break;
							if ((hitwall&0xc0000000) == 0x40000000)
							{
								if (gps->gotcopy&2)
								{
									spr = &gst->spri[hitwall&0x3fffffff];
									f = sqrt(gps->copyspri[0].r.x*gps->copyspri[0].r.x +
												gps->copyspri[0].r.y*gps->copyspri[0].r.y +
												gps->copyspri[0].r.z*gps->copyspri[0].r.z)/
										 sqrt(spr->r.x*spr->r.x + spr->r.y*spr->r.y + spr->r.z*spr->r.z);
									spr->r.x *= f; spr->r.y *= f; spr->r.z *= f;
									f = sqrt(gps->copyspri[0].d.x*gps->copyspri[0].d.x +
												gps->copyspri[0].d.y*gps->copyspri[0].d.y +
												gps->copyspri[0].d.z*gps->copyspri[0].d.z)/
										 sqrt(spr->d.x*spr->d.x + spr->d.y*spr->d.y + spr->d.z*spr->d.z);
									spr->d.x *= f; spr->d.y *= f; spr->d.z *= f;
									f = sqrt(gps->copyspri[0].f.x*gps->copyspri[0].f.x +
												gps->copyspri[0].f.y*gps->copyspri[0].f.y +
												gps->copyspri[0].f.z*gps->copyspri[0].f.z)/
										 sqrt(spr->f.x*spr->f.x + spr->f.y*spr->f.y + spr->f.z*spr->f.z);
									spr->f.x *= f; spr->f.y *= f; spr->f.z *= f;
									spr->tilnum = gps->copyspri[0].tilnum;
									spr->fat = gps->copyspri[0].fat;
									spr->mas = gps->copyspri[0].mas;
									spr->moi = gps->copyspri[0].moi;
									spr->asc = gps->copyspri[0].asc;
									spr->rsc = gps->copyspri[0].rsc;
									spr->gsc = gps->copyspri[0].gsc;
									spr->bsc = gps->copyspri[0].bsc;
									spr->tag = gps->copyspri[0].tag;
									spr->flags = gps->copyspri[0].flags&~(1<<16); //Don't copy light attribute
								}
							}
							else
							{
								if (gps->gotcopy&1)
								{
									if (hitwall < 0) sur = &sec[hitsect].surf[hitwall&1];
													else sur = &sec[hitsect].wall[hitwall].surf;
									if (!(TESTKEY(0x2a)|TESTKEY(0x36)))
									{
										i = sur->tag; j = sur->flags;
										(*sur) = gps->copysurf[0];
										sur->tag = i; sur->flags = (sur->flags&((1<<16)|(1<<17)))+(j&~((1<<16)|(1<<17))); //Don't copy tag or most flags
									}
									else
									{
										sur->asc = gps->copysurf[0].asc;
										sur->rsc = gps->copysurf[0].rsc;
										sur->gsc = gps->copysurf[0].gsc;
										sur->bsc = gps->copysurf[0].bsc;
									}
								}
							}
						}
						break;
					}
						//KP Enter (0x9c)
					gps->editmode ^= 1;
					if (gps->editmode == 2) //3D->2D
					{
						if (!hitscan(&gps->ipos,&gps->ifor,1e32,gps->cursect,&hitsect,&hitwall,&hit)) break;

						//if (hitwall < 0)
						//{
							gps->grdc.x = gps->grdc.y = 0; gps->grdc.z = hit.z;

							gps->grdn.x = gps->grdn.y = 0;
							if (hitwall == -2) gps->grdn.z =-1;
											  else gps->grdn.z =+1;

								//Quantize idow vector to nearest axis
							gps->grdu.x = gps->grdu.y = gps->grdu.z = 0;
							if (fabs(gps->irig.x) > fabs(gps->irig.y))
								  { if (gps->irig.x > 0) gps->grdu.x = 1; else gps->grdu.x = -1; }
							else { if (gps->irig.y > 0) gps->grdu.y = 1; else gps->grdu.y = -1; }
#if 0
						}
						else
						{
							gps->grdc.x = sec[hitsect].wall[hitwall].x;
							gps->grdc.y = sec[hitsect].wall[hitwall].y;
							gps->grdc.z = sec[hitsect].z[0];

							i = sec[hitsect].wall[hitwall].n+hitwall;
							gps->grdu.x = sec[hitsect].wall[i].x - sec[hitsect].wall[hitwall].x;
							gps->grdu.y = sec[hitsect].wall[i].y - sec[hitsect].wall[hitwall].y;
							f = 1.0/sqrt(gps->grdu.x*gps->grdu.x + gps->grdu.y*gps->grdu.y);
							gps->grdu.x *= f; gps->grdu.y *= f;
							gps->grdu.z = 0;

							gps->grdn.x = gps->grdu.y;
							gps->grdn.y =-gps->grdu.x;
							gps->grdn.z = 0;
						}
#endif
						gps->grdv.x = gps->grdn.y*gps->grdu.z - gps->grdn.z*gps->grdu.y;
						gps->grdv.y = gps->grdn.z*gps->grdu.x - gps->grdn.x*gps->grdu.z;
						gps->grdv.z = gps->grdn.x*gps->grdu.y - gps->grdn.y*gps->grdu.x;
					}
					else //2D->3D
					{
						fp.x = gps->irig.x*(gps->fcmousx-gps->ghx) + gps->idow.x*(gps->fcmousy-gps->ghy) + gps->ifor.x*gps->ghz;
						fp.y = gps->irig.y*(gps->fcmousx-gps->ghx) + gps->idow.y*(gps->fcmousy-gps->ghy) + gps->ifor.y*gps->ghz;
						fp.z = gps->irig.z*(gps->fcmousx-gps->ghx) + gps->idow.z*(gps->fcmousy-gps->ghy) + gps->ifor.z*gps->ghz;
						if (hitscan(&gps->ipos,&fp,1e32,gps->cursect,&hitsect,&hitwall,&hit))
						{
							gps->npos.x = hit.x;
							gps->npos.y = hit.y;
							y0 = getslopez(&gst->sect[hitsect],0,hit.x,hit.y);
							y1 = getslopez(&gst->sect[hitsect],1,hit.x,hit.y);
							if (gps->grdn.z > 0)
								  { if (fabs(y1-y0) < 1.f) gps->npos.z = (y0+y1)*.5; else gps->npos.z = y1-.5; }
							else { if (fabs(y1-y0) < 1.f) gps->npos.z = (y0+y1)*.5; else gps->npos.z = y0+.5; }
							gps->nrig.x = gps->irig.x;
							gps->nrig.y = gps->irig.y;
							gps->nrig.z = gps->irig.z;
							if (gps->grdn.z > 0) d = (     +15.0)*(PI/180.0);
												 else d = (180.0-15.0)*(PI/180.0);
							gps->ndow.x = gps->ifor.x*cos(d) + gps->idow.x*sin(d);
							gps->ndow.y = gps->ifor.y*cos(d) + gps->idow.y*sin(d);
							gps->ndow.z = gps->ifor.z*cos(d) + gps->idow.z*sin(d);
							gps->nfor.x =-gps->idow.x*cos(d) + gps->ifor.x*sin(d);
							gps->nfor.y =-gps->idow.y*cos(d) + gps->ifor.y*sin(d);
							gps->nfor.z =-gps->idow.z*cos(d) + gps->ifor.z*sin(d);
						}
						else
						{
							gps->npos = gps->ipos;
							gps->nrig = gps->grdu;
							gps->ndow = gps->grdv;
							gps->nfor = gps->grdn;
						}
					}
					break;
				case 0x39: //32,Space,' '
					getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
					if ((gps->grabmode == GRABCIRC) && ((unsigned)gps->grabsect < (unsigned)gst->numsects))
					{
						float g, x0, y0, x1, y1, x2, y2, centx, centy, circrad, circang0, circang1;
						bs = gps->grabsect; gps->grabsect = -1; gps->grabmode = -1;
						bw = gps->grabwall;

						x0 = gst->sect[bs].wall[bw].x;
						y0 = gst->sect[bs].wall[bw].y;
						i = gst->sect[bs].wall[bw].n+bw;
						x1 = gst->sect[bs].wall[i].x;
						y1 = gst->sect[bs].wall[i].y;
						x2 = qmousx;
						y2 = qmousy;
						g = (y0-y1)*(x0-x2) + (y0-y2)*(x1-x0); if (fabs(g) <= 0.001) break;
						f = ((x2-x1)*(x0-x2) + (y0-y2)*(y2-y1))/g;
						centx = ((y0-y1)*f + (x0+x1))*.5;
						centy = ((x1-x0)*f + (y0+y1))*.5;

						circang0 = atan2(y0-centy,x0-centx);
						circang1 = atan2(y1-centy,x1-centx);
						f = fmod((double)(circang1-circang0),PI*2.0); if (f < 0.0) f += PI*2;
						if ((x2-x0)*(y1-y0) < (x1-x0)*(y2-y0)) f -= PI*2;
						circrad = sqrt((centx-x0)*(centx-x0) + (centy-y0)*(centy-y0));
						for(k=gps->circnum;k>0;k--)
						{
							g = ((double)k)*f/((double)(gps->circnum+1)) + circang0;
							dx = cos(g)*circrad + centx;
							dy = sin(g)*circrad + centy;

							i = dupwall(&sec[bs],bw);
							wal = sec[bs].wall;
							wal[i].x = dx;
							wal[i].y = dy;

							s = wal[bw].ns;
							if (s >= 0)
							{
								w = wal[bw].nw;
								do
								{
									j = dupwall(&sec[s],w);
									wal = sec[s].wall;
									wal[j].x = dx;
									wal[j].y = dy;
									s = wal[w].ns; if ((s < 0) || (s == bs)) break;
									w = wal[w].nw;
								} while (1);
								checknextwalls(); //FIXFIX:slow
								checksprisect(-1);
							}
						}

						if (doplaysound)
						{
							fp.x = centx; fp.y = centy; fp.z = (sec[bs].z[0]+sec[bs].z[1])*.5;
							myplaysound("sounds\\deleteroom.wav",100,1.0,&fp,KSND_3D);
						}

						checknextwalls();
						checksprisect(-1);

						break;
					}
					if ((!gps->sec.n) || (qmousx != gps->sec.wall[gps->sec.n-1].x) ||
												(qmousy != gps->sec.wall[gps->sec.n-1].y))
					{
						if (!gps->sec.n)
						{
							if (((unsigned)hitwall < (unsigned)gst->sect[hitsect].n) && (gps->editmode == 3))
								{ /*MessageBeep(0);*/ break; } //loops on walls not yet implemented
							if (!gquantstat) //Start sector split
							{
								gps->startsect = gquantsec;
								gps->startwall = gquantwal;
								gps->startstate = 1;
								if ((hitsect >= 0) && (hitwall < 0))
								{
									gps->sec.grad[0].x = gst->sect[hitsect].grad[hitwall&1].x;
									gps->sec.grad[0].y = gst->sect[hitsect].grad[hitwall&1].y;
								}
							}
							else if (gquantstat < 0)
							{
								gps->startstate = 0;
								if ((hitsect >= 0) && (hitwall < 0)) //Start loop drawing
								{
									gps->startsect = hitsect;
									gps->startwall = hitwall;
									gps->sec.grad[0].x = gst->sect[hitsect].grad[hitwall&1].x;
									gps->sec.grad[0].y = gst->sect[hitsect].grad[hitwall&1].y;
								}
								else
								{
									gps->startsect = -1;
								}
							}
							else { /*MessageBeep(0);*/ break; } //can't start sector drawing in middle of wall
							gps->sec.z[0] = qmousz;
							gps->sec.grad[1].x = gps->sec.grad[1].y = 0.f;
							for(j=0;j<2;j++)
							{
								gps->sec.surf[j].uv[0].x = 0.f; gps->sec.surf[j].uv[0].y = 0.f;
								gps->sec.surf[j].uv[1].x = 1.f; gps->sec.surf[j].uv[1].y = 0.f;
								gps->sec.surf[j].uv[2].x = 0.f; gps->sec.surf[j].uv[2].y = 1.f;
							}
						}
						else if (gps->sec.n == 1)
						{
								//Choose sector of splitting sector at 2nd point plotted
							mind = 1e32;
							if (gps->startstate > 0)
							{
								if (gquantstat < 0)
								{
									for(i=getverts(gps->startsect,gps->startwall,verts,MAXVERTS)-1;i>=0;i--)
										if ((verts[i].s == hitsect) || ((hitsect < 0) && (insidesect(qmousx,qmousy,sec[verts[i].s].wall,sec[verts[i].s].n))))
										{
											y0 = getslopez(&sec[verts[i].s],0,qmousx,qmousy);
											y1 = getslopez(&sec[verts[i].s],1,qmousx,qmousy);
												  if (qmousz < y0) d = y0-qmousz;
											else if (qmousz > y1) d =    qmousz-y1;
											else                  d = 0;
											if (d < mind)
											{
												mind = d;
												gps->startsect = verts[i].s;
												gps->startwall = verts[i].w;
												gps->startstate = 2; //= Split sector state!
											}
										}
								}
								else if (!gquantstat) //split in 2 plots
								{
									k = getverts(gquantsec,gquantwal,verts2,MAXVERTS);
									for(i=getverts(gps->startsect,gps->startwall,verts,MAXVERTS)-1;i>=0;i--)
										for(j=k-1;j>=0;j--)
										{
											if ((verts[i].s == verts2[j].s) &&
												 (verts [i].w+sec[verts [i].s].wall[verts [i].w].n != verts2[j].w) && //start&end must not be neighbor wall
												 (verts2[j].w+sec[verts2[j].s].wall[verts2[j].w].n != verts [i].w))
											{
												if (insidesect((gps->sec.wall[0].x+qmousx)*.5,
																	(gps->sec.wall[0].y+qmousy)*.5,sec[verts[i].s].wall,sec[verts[i].s].n))
												{
													y0 = getslopez(&sec[verts[i].s],0,qmousx,qmousy);
													y1 = getslopez(&sec[verts[i].s],1,qmousx,qmousy);
														  if (qmousz < y0) d = y0-qmousz;
													else if (qmousz > y1) d =    qmousz-y1;
													else                  d = 0;
													if (d < mind)
													{
														mind = d;
														gps->startsect = verts[i].s;
														gps->startwall = verts[i].w;
														gps->startstate = 2; //= Split sector state!
													}
												}
											}
										}
								}
							}
						}

						i = dupwall(&gps->sec,gps->sec.n-1);
						gps->sec.wall[i].x = qmousx;
						gps->sec.wall[i].y = qmousy;
						gps->sec.wall[i].owner = -1;
						gps->sec.wall[i].surf.asc = 4096;
						gps->sec.wall[i].surf.rsc = 4096; //FIXFIX: use curcol
						gps->sec.wall[i].surf.gsc = 4096;
						gps->sec.wall[i].surf.bsc = 4096;

						if ((gps->sec.n > 1) && (gps->startstate == 2)) //Sector split end check
						{
							if (gquantstat) i = -1;
							else
							{
								for(i=getverts(gquantsec,gquantwal,verts,MAXVERTS)-1;i>=0;i--)
									if ((verts[i].s == gps->startsect) && (verts[i].w != gps->startwall))
										{ bw = verts[i].w; break; }
							}
							if (i >= 0)
							{
									//split sector: gps->startsect
									//start wall: gps->startwall
									//end wall: bw

									//�������Ŀ
									//�   �   �
									//���������

								if (gst->numsects+2 > gst->malsects)
								{
									i = gst->malsects; gst->malsects = min(gst->numsects+2,gst->malsects<<1);
									sec = gst->sect = (sect_t *)realloc(gst->sect,gst->malsects*sizeof(sect_t));
									memset(&sec[i],0,(gst->malsects-i)*sizeof(sect_t));
								}

								k = 2; //flag for connecting loops: 2=normal, 1=different loops/do not increase sectors

									//Every wall of split sector must be visited and copied
									//This is necessary for inserting loops that are not contacted by split line
								for(w=sec[gps->startsect].n-1;w>=0;w--) sec[gps->startsect].wall[w].ns = 0;

								for(i=gst->numsects;i<gst->numsects+2;i++)
								{
									memcpy(&sec[i],&sec[gps->startsect],sizeof(sect_t));
									sec[i].nmax = sec[gps->startsect].n+(gps->sec.n-1)*2;
									sec[i].wall = (wall_t *)malloc(sec[gst->numsects].nmax*sizeof(wall_t));
									sec[i].n = 0;

									wal = sec[i].wall;
									if (i == gst->numsects)
									{
											//Copy split forwards
										for(j=0;j<gps->sec.n-1;j++)
										{
											memcpy(&wal[sec[i].n],&gps->sec.wall[j],sizeof(wall_t));
											wal[sec[i].n].n = 1; sec[i].n++;
										}

											//Follow wall loop from end to start
										w = bw;
										do
										{
											memcpy(&wal[sec[i].n],&sec[gps->startsect].wall[w],sizeof(wall_t));
											sec[gps->startsect].wall[w].ns = -1;
											wal[sec[i].n].n = 1; sec[i].n++;
											w += sec[gps->startsect].wall[w].n;
											if (w == bw) //contacting different loops
												{ k = 1; gst->numsects--; goto splitsecthack; }
										} while (w != gps->startwall);
									}
									else
									{
splitsecthack:;                  //Copy split backwards
										for(j=gps->sec.n-1;j>0;j--)
										{
											memcpy(&wal[sec[i].n],&gps->sec.wall[j],sizeof(wall_t));
											wal[sec[i].n].n = 1; sec[i].n++;
										}

											//Follow wall loop from start to end
										w = gps->startwall;
										do
										{
											memcpy(&wal[sec[i].n],&sec[gps->startsect].wall[w],sizeof(wall_t));
											sec[gps->startsect].wall[w].ns = -1;
											wal[sec[i].n].n = 1; sec[i].n++;
											w += sec[gps->startsect].wall[w].n;
											if (w == gps->startwall) break;
										} while (w != bw);
									}
									wal[sec[i].n-1].n = -(sec[i].n-1);

									sec[i].z[0] = getslopez(&sec[gps->startsect],0,wal[0].x,wal[0].y);
									sec[i].z[1] = getslopez(&sec[gps->startsect],1,wal[0].x,wal[0].y);
								}

									//copy loops not contacted
								for(w=0;w<sec[gps->startsect].n;w++)
								{
									if (sec[gps->startsect].wall[w].ns) continue;

									i = gst->numsects+1;
									if (k == 2)
									{
										i = insidesect(sec[gps->startsect].wall[w].x,sec[gps->startsect].wall[w].y,sec[gst->numsects  ].wall,sec[gst->numsects  ].n);
										j = insidesect(sec[gps->startsect].wall[w].x,sec[gps->startsect].wall[w].y,sec[gst->numsects+1].wall,sec[gst->numsects+1].n);
										if (i != j) i = gst->numsects+j;
										else //w is possibly on the outer loop?
										{
											i = (getarea(sec[gst->numsects+1].wall,sec[gst->numsects+1].n) <
												  getarea(sec[gst->numsects  ].wall,sec[gst->numsects  ].n)) + gst->numsects;
										}
									}
									else i = gst->numsects+1;


									bw = w; j = sec[i].n;
									do
									{
										memcpy(&sec[i].wall[sec[i].n],&sec[gps->startsect].wall[w],sizeof(wall_t));
										sec[i].wall[sec[i].n].n = 1; sec[i].n++;
										sec[gps->startsect].wall[w].ns = -1;
										w += sec[gps->startsect].wall[w].n;
									} while (w != bw);
									sec[i].wall[sec[i].n-1].n = j-(sec[i].n-1);
								}

								//for(i=gst->numsects+2-k;i<gst->numsects+2;i++) //FIX:necessary?
								//   if (getarea(sec[i].wall,sec[i].n) < 0)
								//      reversewalls(sec[i].wall,sec[i].n);

								gst->numsects += 2;

									//Delete original sector
								if (doplaysound)
								{
									fp.x = qmousx; fp.y = qmousy; fp.z = (sec[gps->startsect].z[0]+sec[gps->startsect].z[1])*.5;
									myplaysound("sounds\\deleteroom.wav",100,1.0,&fp,KSND_3D);
								}
								delsect(gps->startsect);

								memset(&gps->sec,0,sizeof(sect_t));
								checknextwalls();
								checksprisect(-1);
								break;
							}
						}

						if (gps->sec.n > 1) //sector complete loop check
						{
							dx = qmousx-gps->sec.wall[0].x;
							dy = qmousy-gps->sec.wall[0].y;
							if (dx*dx + dy*dy < gps->dgridlock*gps->dgridlock*(DVERTSNAP*DVERTSNAP))
							{
								delwall(&gps->sec,gps->sec.n-1);

								if (getarea(gps->sec.wall,gps->sec.n) < 0)
									reversewalls(gps->sec.wall,gps->sec.n);

									//Get z's&gradients from nearby sector

								wal = gps->sec.wall;
								if ((unsigned)gps->startsect < (unsigned)gst->numsects)
								{
									gps->sec.z[0] = getslopez(&sec[gps->startsect],0,wal[0].x,wal[0].y);
									gps->sec.z[1] = getslopez(&sec[gps->startsect],1,wal[0].x,wal[0].y);
									gps->sec.grad[0].x = sec[gps->startsect].grad[0].x;
									gps->sec.grad[0].y = sec[gps->startsect].grad[0].y;
									gps->sec.grad[1].x = sec[gps->startsect].grad[1].x;
									gps->sec.grad[1].y = sec[gps->startsect].grad[1].y;
								}
								else
								{
									mind = 1e32; bs = -1;
									getcentroid(wal,gps->sec.n,&x0,&y0);
									for(s=gst->numsects-1;s>=0;s--)
									{
										getcentroid(sec[s].wall,sec[s].n,&x1,&y1);
										dx = x1-x0; dy = y1-y0; d = dx*dx + dy*dy;
										if (d < mind) { mind = d; bs = s; }
									}
									if (bs >= 0)
									{
										gps->sec.z[0] = sec[bs].z[0];
										gps->sec.z[1] = sec[bs].z[1];
									}
									else
									{
										gps->sec.z[0] = -1;
										gps->sec.z[1] = +1;
									}
									gps->sec.grad[0].x = gps->sec.grad[1].x = 0.f;
									gps->sec.grad[0].y = gps->sec.grad[1].y = 0.f;
								}
								gps->sec.surf[0].asc = 4096;
								gps->sec.surf[0].rsc = 4096-512; //FIXFIX:use curcol
								gps->sec.surf[0].gsc = 4096-512;
								gps->sec.surf[0].bsc = 4096-512;
								gps->sec.surf[1].asc = 4096;
								gps->sec.surf[1].rsc = 4096+512; //FIXFIX:use curcol
								gps->sec.surf[1].gsc = 4096+512;
								gps->sec.surf[1].bsc = 4096+512;
								gps->sec.headspri = -1;
								gps->sec.owner = -1;
#if 1
								dx = gps->sec.wall[0].x;
								dy = gps->sec.wall[0].y;

								if (((unsigned)gps->startsect < (unsigned)gst->numsects) && (insidesect(dx,dy,sec[gps->startsect].wall,sec[gps->startsect].n)))
									s = gps->startsect;
								else
									{ for(s=gst->numsects-1;s>=0;s--) if (insidesect(dx,dy,sec[s].wall,sec[s].n)) break; }
								if (s >= 0)
								{
									for(w=gps->sec.n-1;w>=0;w--)
										if (!insidesect(gps->sec.wall[w].x,gps->sec.wall[w].y,sec[s].wall,sec[s].n)) break;
								}
								if ((s >= 0) && (w < 0))
								{
									wal = sec[s].wall; d = gps->dgridlock*gps->dgridlock*(DEDGESNAP*DEDGESNAP);
									for(w=sec[s].n-1;w>=0;w--)
									{
										i = wal[w].n+w;
										if (distpoint2line2(dx,dy,wal[w].x,wal[w].y,wal[i].x,wal[i].y) < d) break;
									}
									if (w < 0) //loop is definitely inside (first point not touching any walls)
									{
										if (gst->numsects+1 >= gst->malsects)
										{
											i = gst->malsects; gst->malsects = max(gst->numsects+2,gst->malsects<<1);
											sec = gst->sect = (sect_t *)realloc(gst->sect,gst->malsects*sizeof(sect_t));
											memset(&sec[i],0,(gst->malsects-i)*sizeof(sect_t));
										}

										memcpy(&sec[gst->numsects  ],&sec[s],sizeof(sect_t));
										memcpy(&sec[gst->numsects+1],&sec[s],sizeof(sect_t));
										sec[gst->numsects  ].n = 0; sec[gst->numsects  ].nmax = max(sec[s].n+gps->sec.n,8);
										sec[gst->numsects+1].n = 0; sec[gst->numsects+1].nmax = max(sec[s].n+gps->sec.n,8);
										sec[gst->numsects  ].wall = (wall_t *)malloc(sec[gst->numsects  ].nmax*sizeof(wall_t));
										sec[gst->numsects+1].wall = (wall_t *)malloc(sec[gst->numsects+1].nmax*sizeof(wall_t));

										for(i=w=0;w<sec[s].n;w++)
										{
											if (wal[w].n >= 0) continue;
											if (!insidesect(wal[i].x,wal[i].y,gps->sec.wall,gps->sec.n))
											{
												memcpy(&sec[gst->numsects+1].wall[sec[gst->numsects+1].n],&wal[i],(w+1-i)*sizeof(wall_t));
												sec[gst->numsects+1].n += w+1-i;
											}
											else
											{
												memcpy(&sec[gst->numsects  ].wall[sec[gst->numsects  ].n],&wal[i],(w+1-i)*sizeof(wall_t));
												sec[gst->numsects  ].n += w+1-i;
											}
											i = w+1;
										}

										memcpy(&sec[gst->numsects  ].wall[sec[gst->numsects  ].n],gps->sec.wall,gps->sec.n*sizeof(wall_t));
										sec[gst->numsects  ].n += gps->sec.n;
										sec[gst->numsects  ].z[0] = getslopez(&sec[s],0,sec[gst->numsects  ].wall[0].x,sec[gst->numsects  ].wall[0].y);
										sec[gst->numsects  ].z[1] = getslopez(&sec[s],1,sec[gst->numsects  ].wall[0].x,sec[gst->numsects  ].wall[0].y);

										memcpy(&sec[gst->numsects+1].wall[sec[gst->numsects+1].n],gps->sec.wall,gps->sec.n*sizeof(wall_t));
										reversewalls(&sec[gst->numsects+1].wall[sec[gst->numsects+1].n],gps->sec.n);
										sec[gst->numsects+1].n += gps->sec.n;
										sec[gst->numsects+1].z[0] = getslopez(&sec[s],0,sec[gst->numsects+1].wall[0].x,sec[gst->numsects+1].wall[0].y);
										sec[gst->numsects+1].z[1] = getslopez(&sec[s],1,sec[gst->numsects+1].wall[0].x,sec[gst->numsects+1].wall[0].y);

										memcpy(&sec[s],&sec[gst->numsects+1],sizeof(sect_t));
										memset(&sec[gst->numsects+1],0,sizeof(sect_t));
										//checksprisect(-1); //sec[gst->numsects].headspri = -1; sec[gst->numsects].owner = -1; (int s)checksprisect(s);
										gst->numsects++;
									}
								}
								else
								{
									if (gst->numsects >= gst->malsects)
									{
										i = gst->malsects; gst->malsects <<= 1;
										sec = gst->sect = (sect_t *)realloc(gst->sect,gst->malsects*sizeof(sect_t));
										memset(&sec[i],0,(gst->malsects-i)*sizeof(sect_t));
									}
									memcpy(&sec[gst->numsects],&gps->sec,sizeof(sect_t));
									sec[gst->numsects].headspri = -1; sec[gst->numsects].owner = -1;
									gst->numsects++;
								}
#else
								if (gst->numsects >= gst->malsects)
								{
									i = gst->malsects; gst->malsects <<= 1;
									sec = gst->sect = (sect_t *)realloc(gst->sect,gst->malsects*sizeof(sect_t));
									memset(&sec[i],0,(gst->malsects-i)*sizeof(sect_t));
								}
								memcpy(&sec[gst->numsects],&gps->sec,sizeof(sect_t));
								sec[gst->numsects].headspri = -1; sec[gst->numsects].owner = -1;
								gst->numsects++;

#if 0
								if ((gst->numsects == 2) && (!sec[2].n) && (i&0x30000)) //Shift+Enter
								{
									polybool(sec[0].wall,sec[0].n,
												sec[1].wall,sec[1].n,&sec[2].wall,&sec[2].n,boolfunc);
									if (sec[2].wall)
									{
										free(sec[0].wall);
										free(sec[1].wall);
										memcpy(&sec[0],&sec[2],sizeof(sect_t));
										memset(&sec[1],0,2*sizeof(sect_t));
										gst->numsects--;
									}
								}
#else
									//POLYBOOL loop insert
								dx = sec[gst->numsects-1].wall[0].x;
								dy = sec[gst->numsects-1].wall[0].y;
								s = gst->numsects-2;
								if ((unsigned)gps->startsect < (unsigned)gst->numsects)
									if (insidesect(dx,dy,&sec[gps->startsect])) s = gps->startsect; //hack to avoid a goto :/
								for(;s>=0;s--)
									if (insidesect(dx,dy,&sec[s]))
									{
										wal = sec[s].wall; d = gps->dgridlock*gps->dgridlock*(DEDGESNAP*DEDGESNAP);
										for(w=sec[s].n-1;w>=0;w--)
										{
											i = sec[s].wall[w].n+w;
											if (distpoint2line2(dx,dy,sec[s].wall[w].x,sec[s].wall[w].y,sec[s].wall[i].x,sec[s].wall[i].y) < d) break;
										}
										if ((w < 0) && (polybool(sec[s].wall,sec[s].n,sec[gst->numsects-1].wall,sec[gst->numsects-1].n,&wal,&i,POLYBOOL_SUB)))
										{
												//Adjust heights for new hinge to preserve ceil&flor heights of original loop
											sec[s].z[0] = getslopez(&sec[s],0,wal[0].x,wal[0].y);
											sec[s].z[1] = getslopez(&sec[s],1,wal[0].x,wal[0].y);

											free(sec[s].wall);
											if (gps->editmode == 2) //When in 3D mode, do not delete inner loop
												{ free(sec[gst->numsects-1].wall); gst->numsects--; }
											sec[s].n = sec[s].nmax = i;
											sec[s].wall = wal;

											memset(&sec[gst->numsects],0,sizeof(sect_t));
										}
										break;
									}
#endif
#endif
								if (doplaysound)
								{
									fp.x = qmousx; fp.y = qmousy;
									if (s >= 0) fp.z = (sec[s].z[0]+sec[s].z[1])*.5; else fp.z = 0;
									myplaysound("sounds\\deleteroom.wav",100,1.0,&fp,KSND_3D);
								}

								memset(&gps->sec,0,sizeof(sect_t));
								checknextwalls();
								checksprisect(-1);
								break;
							}
						}

						if (doplaysound)
						{
							fp.x = qmousx; fp.y = qmousy; fp.z = qmousz;
							myplaysound("sounds\\hammer.wav",100,1.0,&fp,KSND_3D);
						}
					}
					break;

				case 0x30: //'B','b'
					if (getcurswall(gps,&hitsect,&hitwall,&hit) < 0)
					{
						//if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; }
						//else getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
						if ((hitwall&0xc0000000) == 0x40000000)
						{
							spr = &gst->spri[hitwall&0x3fffffff];
							spr->flags ^= 1;
							if (doplaysound) myplaysound("sounds\\hammer.wav",100,1.0,&spr->p,KSND_3D);
							if (curindex == viewindex)
							{
								messagetimeout = dtotclk+3.0;
								if (spr->flags&1) strcpy(message,"Sprite is blocking.");
												 else strcpy(message,"Sprite is non blocking.");
							}
						}
					}
					else
					{
						sur = &sec[hitsect].wall[hitwall].surf;
						sur->flags ^= 1;
						if (doplaysound) myplaysound("sounds\\hammer.wav",100,0.75,&hit,KSND_3D);
						if (curindex == viewindex)
						{
							messagetimeout = dtotclk+3.0;
							if (sur->flags&1) strcpy(message,"Wall is blocking.");
											 else strcpy(message,"Wall is non blocking.");
						}
					}
					break;
				case 0x19: //'P','p'
					if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; }
					else getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
					if ((hitwall&0xc0000000) == 0x40000000) break;

					if ((unsigned)hitsect >= (unsigned)gst->numsects) break;
					if (hitwall < 0) sur = &sec[hitsect].surf[hitwall&1];
									else sur = &sec[hitsect].wall[hitwall].surf;
					switch((sur->flags>>16)&3)
					{
						case 0: sur->flags |= (1<<16); break;
						case 1: sur->flags ^= ((1<<16)|(1<<17)); break;
						default:sur->flags &=~((1<<16)|(1<<17)); break;
					}
					if (doplaysound) myplaysound("sounds\\hammer.wav",100,1.0,&hit,KSND_3D);

					break;
				case 0x22: //'G','g'
					gps->gridlock = !gps->gridlock;
					break;
#if (USEHEIMAP != 0)
				case 0x23: //'H','h'
					gps->rendheight ^= 1;
					break;
#endif
				case 0x17: //'I','i'
					gps->rendinterp ^= 1; if (curindex == moveindex) b2opts.bilin = (gps->rendinterp&1);
					break;
				case 0x2f: //'V','v': //select texture
					if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; }
					else getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
					if ((unsigned)hitsect >= (unsigned)gst->numsects) break;
					releasegrabdrag(curindex,doplaysound); gps->grabmode = GRABFILE; gps->grabsect = hitsect; gps->grabwall = hitwall;
					if (curindex == moveindex)
					{
						if ((hitwall&0xc0000000) == 0x40000000)
							  { strcpy(myfileselect_filespec,"*.PNG;*.JPG;*.JPEG;*.GIF;*.DDS;*.TGA;*.PCX;*.BMP;*.ART;*.KV6"); menupath = curmodpath; menuhighlightrec = curpichighlightrec; menuhighlightdep = &curpichighlightdep; myfileselect_start(); }
						else { strcpy(myfileselect_filespec,"*.PNG;*.JPG;*.JPEG;*.GIF;*.DDS;*.TGA;*.PCX;*.BMP;*.ART"      ); menupath = curpicpath; menuhighlightrec = curmodhighlightrec; menuhighlightdep = &curmodhighlightdep; myfileselect_start(); }
					}
					break;
				case 0x26: //'L','l': Set sprite as light source
					if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; }
					else getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
					if ((unsigned)hitsect >= (unsigned)gst->numsects) break;
					if ((hitwall&0xc0000000) == 0x40000000)
					{
						for(j=gst->light_sprinum-1;j>=0;j--)
							if (gst->light_spri[j] == (hitwall&0x3fffffff)) break;
						if (j >= 0)
						{
							if (key&0x30000) //Shift+L: Cycle spotlight on(6 orientations)/off
							{
								j = gst->spri[hitwall&0x3fffffff].flags;
								i = ((j>>17)&7); i++; if (i > 6) i = 0;
								gst->spri[hitwall&0x3fffffff].flags = (j&~(7<<17))+(i<<17);
								if ((i) && (!(j&0x3ff00000))) gst->spri[hitwall&0x3fffffff].flags |= (256<<20); //default:1/4
							}
							else
							{
								gst->light_spri[j] = gst->light_spri[--gst->light_sprinum]; gst->spri[hitwall&0x3fffffff].flags &= ~(1<<16);
							}
						}
						else
						{
							gst->light_spri[gst->light_sprinum++] = (hitwall&0x3fffffff);
							gst->spri[hitwall&0x3fffffff].flags |= (1<<16);
						}
						if (doplaysound) myplaysound("sounds\\hammer.wav",50,2.0,&hit,KSND_3D);
					}
					else
					{
						gps->flashlighton = !gps->flashlighton;
						if (doplaysound) myplaysound("sounds\\hammer.wav",50,1.5,&hit,KSND_3D);
					}
					shadowtest2_updatelighting = 1;
					break;
				case 0x1f: //'S','s': Spawn sprite
					getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
					if ((unsigned)hitsect >= (unsigned)gst->numsects) break;

					if (!insidesect(qmousx,qmousy,sec[hitsect].wall,sec[hitsect].n))
					{
							//Binary search until sprite is in sector
						for(f=.125f,g=.0625f;g>=(1.f/1024.f);g*=.5f)
						{
							fp.x = qmousx - gps->ifor.x*(f-g);
							fp.y = qmousy - gps->ifor.y*(f-g);
							fp.z = qmousz - gps->ifor.z*(f-g);
							if (insidesect(fp.x,fp.y,sec[hitsect].wall,sec[hitsect].n)) f -= g;
						}
						qmousx -= gps->ifor.x*f;
						qmousy -= gps->ifor.y*f;
						qmousz -= gps->ifor.z*f;
					}

					i = insspri(hitsect,qmousx,qmousy,qmousz);
					spr = &gst->spri[i];
					if (gps->gotcopy&2)
					{
						spr->r = gps->copyspri[0].r;
						spr->d = gps->copyspri[0].d;
						spr->f = gps->copyspri[0].f;
						spr->tilnum = gps->copyspri[0].tilnum;
						spr->fat = gps->copyspri[0].fat;
						spr->mas = gps->copyspri[0].mas;
						spr->moi = gps->copyspri[0].moi;
						spr->asc = gps->copyspri[0].asc;
						spr->rsc = gps->copyspri[0].rsc;
						spr->gsc = gps->copyspri[0].gsc;
						spr->bsc = gps->copyspri[0].bsc;
						spr->tag = gps->copyspri[0].tag;
						spr->flags = gps->copyspri[0].flags&~(1<<16); //Don't copy light attribute
					}
					if ((hitwall >= -2) && (hitwall < 0)) //Put on ceil/floor
						spr->p.z = getslopez(&gst->sect[hitsect],hitwall&1,spr->p.x,spr->p.y)-spr->fat*(hitwall*2+3);

					//if (hitwall < 0) //Use pivots in KV6 instead?
					//{
					//   if (hitwall == -1) spr->p.z -= spr->fat?;
					//   if (hitwall == -2) spr->p.z += spr->fat?;
					//}

					if (doplaysound) myplaysound("sounds\\hammer.wav",100,0.75,&hit,KSND_3D);
					break;
				case 0x14: //'T','t'
					gps->typemode = 1;
					gps->typecurs = strlen(gst->typemess[curindex]);
					gps->typehighlight = -1;
					break;
				case 0x1a: case 0x1b: case 0x35: //'[',']','{','}','/'
					if (((key>>8)&255) == 0xb5) break; //ignore KP/
					if (gps->editmode != 3) break;
						  if ((((key>>8)&255) == 0x1a) && (key&0x30000)) { d = -1.f/256.f; f = 3.f; }
					else if ((((key>>8)&255) == 0x1b) && (key&0x30000)) { d = +1.f/256.f; f = 3.f; }
					else if (((key>>8)&255) == 0x1a) { d = -1.f/16.f;  f = 1.f; }
					else if (((key>>8)&255) == 0x1b) { d = +1.f/16.f;  f = 1.f; }
					else                               d = 0.f;
					if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; } else hitsect = -1;
					if ((unsigned)hitsect >= (unsigned)gst->numsects)
					{
						hitscan(&gps->ipos,&gps->ifor,1e32,gps->cursect,&hitsect,&hitwall,&hit);
						if ((unsigned)hitsect >= (unsigned)gst->numsects) break;
					}
					else if (hitwall < 0)
					{
						getcentroid(sec[hitsect].wall,sec[hitsect].n,&hit.x,&hit.y);
						hit.z = getslopez(&sec[hitsect],hitwall&1,hit.x,hit.y);
					}
					else if ((unsigned)hitwall < (unsigned)sec[hitsect].n)
					{
						float x2, y2, z0, z1, z2;

						if (doplaysound) myplaysound("sounds\\rumble.wav",(int)(50.0/f),f,&hit,KSND_3D);

						wal = sec[hitsect].wall; k = gps->grabcf;
																		 x0 = wal[hitwall].x; y0 = wal[hitwall].y; z0 = getslopez(&sec[hitsect],k,x0,y0)+d;
						i = wal[hitwall].n+hitwall;          x1 = wal[      i].x; y1 = wal[      i].y; z1 = getslopez(&sec[hitsect],k,x1,y1);
						i = wallprev(&sec[hitsect],hitwall); x2 = wal[      i].x; y2 = wal[      i].y; z2 = getslopez(&sec[hitsect],k,x2,y2);
						x0 = wal[0].x-x0; y0 = wal[0].y-y0;
						x1 = wal[0].x-x1; y1 = wal[0].y-y1;
						x2 = wal[0].x-x2; y2 = wal[0].y-y2;

							////3eq/3unk(ngx,ngy,nz):
							//x0*ngx + y0*ngy + nz = z0
							//x1*ngx + y1*ngy + nz = z1
							//x2*ngx + y2*ngy + nz = z2

						f = x0*(y1-y2) + x1*(y2-y0) + x2*(y0-y1);
						if (f != 0.0)
						{
							f = 1.0/f;
							sec[hitsect].grad[k].x = (z0*(y1   -y2   ) + z1*(y2   -y0   ) + z2*(y0   -y1   ))*f;
							sec[hitsect].grad[k].y = (x0*(z1   -z2   ) + x1*(z2   -z0   ) + x2*(z0   -z1   ))*f;
							sec[hitsect].z[k]      = (x0*(y1*z2-y2*z1) + x1*(y2*z0-y0*z2) + x2*(y0*z1-y1*z0))*f;
						}

						checknextwalls();
						checksprisect(-1);
						break;
					}
					if ((hitwall&0xc0000000) == 0x40000000)
					{
						spr = &gst->spri[hitwall&0x3fffffff];
						if (((key>>8)&255) == 0x35) //'/'
						{
							if (spr->fat == 0.f)
							{
								spr->r.x = 0.5; spr->r.y = 0.0; spr->r.z = 0.0;
								spr->d.x = 0.0; spr->d.y = 0.0; spr->d.z = 0.5;
								spr->f.x = 0.0; spr->f.y =-0.5; spr->f.z = 0.0;
							}
							else
							{
								spr->fat = 0.5;
								spr->r.x = 0.5; spr->r.y = 0.0; spr->r.z = 0.0;
								spr->d.x = 0.0; spr->d.y = 0.0; spr->d.z = 0.5;
								spr->f.x = 0.0; spr->f.y =-0.5; spr->f.z = 0.0;
							}
							spr->flags &= ~(1023<<20); //reset spotlight width
							spr->flags &= ~(7<<17); //reset light vs. spotlight mode
							if (doplaysound) myplaysound("sounds\\hammer.wav",100,1.0,&spr->p,KSND_3D);
							break;
						}
						if (spr->flags&(1<<16)) //Set spotlight width
						{
							if ((spr->flags>>17)&7)
							{
								j = ((spr->flags>>20)&1023);
									  if ((((key>>8)&255) == 0x1a) && (key&0x30000)) { j -= 1; f = 3.f; }
								else if ((((key>>8)&255) == 0x1b) && (key&0x30000)) { j += 1; f = 3.f; }
								else if (((key>>8)&255) == 0x1a) { j -= 16; f = 2.f; }
								else if (((key>>8)&255) == 0x1b) { j += 16; f = 2.f; }
								spr->flags = (spr->flags&~(1023<<20)) + (min(max(j,0),1023)<<20);

								shadowtest2_updatelighting = 1;
								if (doplaysound) myplaysound("sounds\\hammer.wav",(int)(100.0/f),f,&hit,KSND_3D);
							}
						}
						break;
					}
					if (hitwall >= 0)
					{
						if (((key>>8)&255) == 0x35) //'/'
						{
							sur = &sec[hitsect].wall[hitwall].surf;
							sur->uv[0].x = 0.f; sur->uv[0].y = 0.f;
							sur->uv[1].x = 1.f; sur->uv[1].y = 0.f;
							sur->uv[2].x = 0.f; sur->uv[2].y = 1.f;
							if (doplaysound) myplaysound("sounds\\rumble.wav",50,4.0,&hit,KSND_3D);
						}
						break;
					}
					if (d == 0.f)
					{
						if (doplaysound) myplaysound("sounds\\rumble.wav",50,0.75,&hit,KSND_3D);
						sec[hitsect].grad[hitwall&1].x = 0.f;
						sec[hitsect].grad[hitwall&1].y = 0.f;
						sec[hitsect].surf[hitwall&1].uv[0].x = 0.f; sec[hitsect].surf[hitwall&1].uv[0].y = 0.f;
						sec[hitsect].surf[hitwall&1].uv[1].x = 1.f; sec[hitsect].surf[hitwall&1].uv[1].y = 0.f;
						sec[hitsect].surf[hitwall&1].uv[2].x = 0.f; sec[hitsect].surf[hitwall&1].uv[2].y = 1.f;
						checknextwalls();
						checksprisect(-1);
						break;
					}
					if (doplaysound) myplaysound("sounds\\rumble.wav",(int)(50.0/f),f,&hit,KSND_3D);
					dx = sec[hitsect].wall[1].y-sec[hitsect].wall[0].y;
					dy = sec[hitsect].wall[0].x-sec[hitsect].wall[1].x;
					f = dx*dx + dy*dy; if (f > 0) f = d/sqrt(f); dx *= f; dy *= f;
					sec[hitsect].grad[hitwall&1].x += dx;
					sec[hitsect].grad[hitwall&1].y += dy;
					checknextwalls();
					checksprisect(-1);
					break;
				case 0x02: //'1': Toggle 1-way wall
					if (getcurswall(gps,&hitsect,&hitwall,&hit) < 0) break;

					if (doplaysound) myplaysound("sounds\\hammer.wav",100,0.75,&hit,KSND_3D);
					sec[hitsect].wall[hitwall].surf.flags ^= 32;
					shadowtest2_updatelighting = 1;
					break;

				case 0x4a: case 0x4e: //'-','+'
					if (gps->grabmode == GRABCIRC)
					{
						if (((key>>8)&255) == 0x4a) gps->circnum = max(gps->circnum-1,1);
													  else gps->circnum = min(gps->circnum+1,255);
						break;
					}
					if (((key>>8)&255) == 0x4a) j = +31; else j = +33;
					if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; }
					else getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
					if ((hitwall&0xc0000000) == 0x40000000)
					{
						spr = &gst->spri[hitwall&0x3fffffff];
						k = spr->rsc; spr->rsc = min(max((spr->rsc*j)>>5,0),65535); if ((spr->rsc == k) && (j > 32) && (spr->rsc < 65535)) spr->rsc++;
						k = spr->gsc; spr->gsc = min(max((spr->gsc*j)>>5,0),65535); if ((spr->gsc == k) && (j > 32) && (spr->gsc < 65535)) spr->gsc++;
						k = spr->bsc; spr->bsc = min(max((spr->bsc*j)>>5,0),65535); if ((spr->bsc == k) && (j > 32) && (spr->bsc < 65535)) spr->bsc++;
						if (gps->grabmode == GRABRGB)
							gps->selrgbintens = sqrt(((double)spr->rsc)*((double)spr->rsc) +
															 ((double)spr->gsc)*((double)spr->gsc) +
															 ((double)spr->bsc)*((double)spr->bsc))/4096.0;
					}
					else
					{
						if ((unsigned)hitsect >= (unsigned)gst->numsects) break;
						if (hitwall < 0) sur = &sec[hitsect].surf[hitwall&1];
										else sur = &sec[hitsect].wall[hitwall].surf;
						i = sur->rsc; sur->rsc = min(max((sur->rsc*j)>>5,0),65535); if ((sur->rsc == i) && (j > 32) && (sur->rsc < 65535)) sur->rsc++;
						i = sur->gsc; sur->gsc = min(max((sur->gsc*j)>>5,0),65535); if ((sur->gsc == i) && (j > 32) && (sur->gsc < 65535)) sur->gsc++;
						i = sur->bsc; sur->bsc = min(max((sur->bsc*j)>>5,0),65535); if ((sur->bsc == i) && (j > 32) && (sur->bsc < 65535)) sur->bsc++;
						if (gps->grabmode == GRABRGB)
							gps->selrgbintens = sqrt(((double)sur->rsc)*((double)sur->rsc) +
															 ((double)sur->gsc)*((double)sur->gsc) +
															 ((double)sur->bsc)*((double)sur->bsc))/4096.0;
					}
					break;
				case 0x18: //'O','o': Set texture orientation alignment (BUILD1:'O' for walls, 'R' for ceil/flors)
					if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; }
					else getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
					if ((unsigned)hitsect < (unsigned)gst->numsects)
					{
						if (hitwall < 0)
						{
							sur = &sec[hitsect].surf[hitwall&1];
							sur->flags ^= 4;
							if (doplaysound) myplaysound("sounds\\hammer.wav",50,2.0,&hit,KSND_3D);
						}
						else if ((unsigned)hitwall < (unsigned)gst->sect[hitsect].n)
						{
							sur = &sec[hitsect].wall[hitwall].surf;
							sur->flags ^= 4;
							if (doplaysound) myplaysound("sounds\\hammer.wav",50,2.0,&hit,KSND_3D);
						}
					}
					break;
				case 0x13: //'R','r'
					if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; }
					else getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
					if ((hitwall&0xc0000000) == 0x40000000)
					{
						spr = &gst->spri[hitwall&0x3fffffff];
						if (spr->fat == 0.0)
						{
							if ((spr->flags&48) == 16) //face->billboard sprite
							{
								spr->flags &= ~16; spr->flags |= 32;
							}
							else if ((spr->flags&48) == 32) //billboard->wall sprite
							{
								spr->flags &= ~32;
							}
							else if ((fabs(spr->d.x) == 0) && (fabs(spr->d.y) == 0)) //wall->floor sprite
							{
								f = sqrt(spr->d.x*spr->d.x + spr->d.y*spr->d.y + spr->d.z*spr->d.z) /
									 sqrt(spr->r.x*spr->r.x + spr->r.y*spr->r.y + spr->r.z*spr->r.z);
								spr->d.x = spr->r.y*f;
								spr->d.y =-spr->r.x*f;
								spr->d.z = 0;
							}
							else //floor->face sprite
							{
								spr->d.z = sqrt(spr->d.x*spr->d.x + spr->d.y*spr->d.y + spr->d.z*spr->d.z);
								spr->d.x = 0;
								spr->d.y = 0;
								spr->flags |= 16;
								spr->flags &= ~4; //remove mirroring which would make it disappear forever
							}

							if (doplaysound) myplaysound("sounds\\rumble.wav",12,8.0,&spr->p,KSND_3D);
						}
					}
					break;
				case 0x33: case 0x34: //'<','>',',','.'
					if (key&0x30000) //'<','>'
					{
						if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; }
						else getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
						if (((unsigned)hitsect < (unsigned)gst->numsects) && (gps->editmode == 2) && (gst->sect[hitsect].owner == curindex) && (gps->grabmode == GRABDRAG3))
						{
							shadowtest2_updatelighting = 1;
							x0 = cos(5*PI/180.0);
							y0 = sin(5*PI/180.0); if (((key>>8)&255) == 0x34) y0 = -y0;
							x1 = -y0; y1 = +x0;
							if (doplaysound) myplaysound("sounds\\rumble.wav",12,8.0,&hit,KSND_3D);
							goto althigh_rotsect;
						}
						else if ((hitwall&0xc0000000) == 0x40000000) //Rotate sprite horiz (slower)
						{
							spr = &gst->spri[hitwall&0x3fffffff];
							x0 = cos(PI/256.0); y0 = sin(PI/256.0); if (((key>>8)&255) == 0x33) y0 = -y0;
							f = spr->r.x; spr->r.x = x0*spr->r.x - y0*spr->r.y; spr->r.y = x0*spr->r.y + y0*f;
							f = spr->d.x; spr->d.x = x0*spr->d.x - y0*spr->d.y; spr->d.y = x0*spr->d.y + y0*f;
							f = spr->f.x; spr->f.x = x0*spr->f.x - y0*spr->f.y; spr->f.y = x0*spr->f.y + y0*f;
							if (doplaysound) myplaysound("sounds\\rumble.wav",12,8.0,&spr->p,KSND_3D);
						}
						else if (((unsigned)hitsect < (unsigned)gst->numsects) &&
									((unsigned)hitwall < (unsigned)gst->sect[hitsect].n)) //Align texture(s) recursive
						{
							#define MAXALIGN 256
							long fifsec[MAXALIGN], fifwal[MAXALIGN], fifw, fifr;

							fifsec[0] = hitsect; fifwal[0] = hitwall; fifw = 1; fifr = 0;
							while (fifr < fifw)
							{
								hitsect = fifsec[fifr]; hitwall = fifwal[fifr]; fifr++;

								wal = sec[hitsect].wall; sur = &wal[hitwall].surf;
								x0 = wal[hitwall].x;    y0 = wal[hitwall].y;    w = hitwall;
								x1 = wal[wal[w].n+w].x; y1 = wal[wal[w].n+w].y;
								if (((key>>8)&255) == 0x34) w += wal[w].n;
								for(j=getverts(hitsect,w,verts,MAXVERTS)-1;j>=0;j--)
								{
									s = verts[j].s; w = verts[j].w;
									wal = sec[s].wall; bw = wal[w].n+w;

									if (((key>>8)&255) == 0x34)
										  { if ((wal[w].x != x1) || (wal[w].y != y1)) continue; }
									else { if ((wal[w].x != x0) || (wal[w].y != y0)) continue; w = wallprev(&sec[s],w); }
									for(k=fifr-1;k>=0;k--) if ((fifsec[k] == s) && (fifwal[k] == w)) break; if (k >= 0) continue; //Don't modify wall twice

										//Make it follow only if same texture
									if (wal[w].surf.tilnum != sur->tilnum) continue;

									dx = wal[w].x-x0; dy = wal[w].y-y0; dx = sqrt(dx*dx + dy*dy);
									if (((key>>8)&255) != 0x34) dx = -dx;
									if (s == hitsect) dy = 0.f;
									else { dy = sec[s].z[(wal[w].surf.flags&4)>>2] - sec[hitsect].z[(sur->flags&4)>>2]; }

										//hitsect,hitwall -> s,w
									wal[w].surf.uv[0].x = sur->uv[1].x*dx + sur->uv[2].x*dy + sur->uv[0].x;
									wal[w].surf.uv[0].y = sur->uv[1].y*dx + sur->uv[2].y*dy + sur->uv[0].y;
									wal[w].surf.uv[2].x = sur->uv[2].x;
									wal[w].surf.uv[2].y = sur->uv[2].y;

									if (fifw < MAXALIGN) //recurse
									{
										for(k=fifw-1;k>=0;k--) if ((fifsec[k] == s) && (fifwal[k] == w)) break;
										if (k < 0) { fifsec[fifw] = s; fifwal[fifw] = w; fifw++; }
									}
								}
							}
						}
						break;
					}
					else //'.',','
					{
						if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; }
						else getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
						if (((unsigned)hitsect < (unsigned)gst->numsects) && (gps->editmode == 2) && (gst->sect[hitsect].owner == curindex) && (gps->grabmode == GRABDRAG3))
						{
							shadowtest2_updatelighting = 1;
							if (((key>>8)&255) == 0x34) { x0 = 0.f; y0 =-1.f; x1 =+1.f; y1 = 0.f; }
														  else { x0 = 0.f; y0 =+1.f; x1 =-1.f; y1 = 0.f; }
							if (doplaysound) myplaysound("sounds\\rumble.wav",25,4.0,&hit,KSND_3D);
	althigh_rotsect:; getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
							for(s=gst->numsects-1;s>=0;s--)
							{
								if (gst->sect[s].owner != curindex) continue;

								wal = gst->sect[s].wall;

									//Fix slopes
								for(w=0;w<2;w++)
								{
										//x0*ngradx + x1*ngrady = gst->sect[s].grad[i].x
										//y0*ngradx + y1*ngrady = gst->sect[s].grad[i].y
									f = 1.0/(x0*y1 - x1*y0);
									dx = (gst->sect[s].grad[w].x*y1 - gst->sect[s].grad[w].y*x1)*f;
									dy = (gst->sect[s].grad[w].y*x0 - gst->sect[s].grad[w].x*y0)*f;
									gst->sect[s].grad[w].x = dx;
									gst->sect[s].grad[w].y = dy;
								}

									//Fix walls
								for(w=gst->sect[s].n-1;w>=0;w--)
								{
									dx = wal[w].x-qmousx; dy = wal[w].y-qmousy;
									wal[w].x = dx*x0 + dy*y0 + qmousx;
									wal[w].y = dx*x1 + dy*y1 + qmousy;
								}

									//Fix sprites
								for(w=gst->sect[s].headspri;w>=0;w=spr->sectn)
								{
									spr = &gst->spri[w];
									dx = spr->p.x-qmousx; dy = spr->p.y-qmousy;
									spr->p.x = dx*x0 + dy*y0 + qmousx;
									spr->p.y = dx*x1 + dy*y1 + qmousy;

									dx = spr->r.x; dy = spr->r.y;
									spr->r.x = dx*x0 + dy*y0;
									spr->r.y = dx*x1 + dy*y1;

									dx = spr->d.x; dy = spr->d.y;
									spr->d.x = dx*x0 + dy*y0;
									spr->d.y = dx*x1 + dy*y1;

									dx = spr->f.x; dy = spr->f.y;
									spr->f.x = dx*x0 + dy*y0;
									spr->f.y = dx*x1 + dy*y1;
								}
							}
						}
						else if ((hitwall&0xc0000000) == 0x40000000) //Rotate sprite horiz
						{
							spr = &gst->spri[hitwall&0x3fffffff];
							x0 = cos(PI/8.0); y0 = sin(PI/8.0); if (((key>>8)&255) == 0x33) y0 = -y0;
							f = spr->r.x; spr->r.x = x0*spr->r.x - y0*spr->r.y; spr->r.y = x0*spr->r.y + y0*f;
							f = spr->d.x; spr->d.x = x0*spr->d.x - y0*spr->d.y; spr->d.y = x0*spr->d.y + y0*f;
							f = spr->f.x; spr->f.x = x0*spr->f.x - y0*spr->f.y; spr->f.y = x0*spr->f.y + y0*f;
							if (doplaysound) myplaysound("sounds\\rumble.wav",25,4.0,&spr->p,KSND_3D);
						}
						else if (((unsigned)hitsect < (unsigned)gst->numsects) &&
									((unsigned)hitwall < (unsigned)gst->sect[hitsect].n)) //Align texture(s)
						{
							wal = sec[hitsect].wall; sur = &wal[hitwall].surf;
							x0 = wal[hitwall].x;    y0 = wal[hitwall].y;    w = hitwall;
							x1 = wal[wal[w].n+w].x; y1 = wal[wal[w].n+w].y;
							if (((key>>8)&255) == 0x34) w += wal[w].n;
							for(j=getverts(hitsect,w,verts,MAXVERTS)-1;j>=0;j--)
							{
								s = verts[j].s; w = verts[j].w;
								wal = sec[s].wall; bw = wal[w].n+w;

								if (((key>>8)&255) == 0x34)
									  { if ((wal[w].x != x1) || (wal[w].y != y1)) continue; }
								else { if ((wal[w].x != x0) || (wal[w].y != y0)) continue; w = wallprev(&sec[s],w); }

									//Make it follow only if same texture
								if (wal[w].surf.tilnum != sur->tilnum) continue;

								dx = wal[w].x-x0; dy = wal[w].y-y0; dx = sqrt(dx*dx + dy*dy);
								if (((key>>8)&255) != 0x34) dx = -dx;
								if (s == hitsect) dy = 0.f;
								else { dy = sec[s].z[(wal[w].surf.flags&4)>>2] - sec[hitsect].z[(sur->flags&4)>>2]; }
								wal[w].surf.uv[0].x = sur->uv[1].x*dx + sur->uv[2].x*dy + sur->uv[0].x;
								wal[w].surf.uv[0].y = sur->uv[1].y*dx + sur->uv[2].y*dy + sur->uv[0].y;
								wal[w].surf.uv[2].x = sur->uv[2].x;
								wal[w].surf.uv[2].y = sur->uv[2].y;
							}
						}
					}
					break;
				case 0x2e: //'C','c'
					if ((gps->grabmode == GRABCIRC) || (gps->grabmode == GRABRGB))
						{ gps->grabmode = -1; break; }
					getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);

					if ((gquantstat == 1) && ((gps->editmode == 2) || (gps->showedges3d)))
					{
						if ((unsigned)gquantsec >= (unsigned)gst->numsects) break;
						releasegrabdrag(curindex,doplaysound);
						gps->grabsect = gquantsec;
						gps->grabwall = gquantwal;
						gps->grabmode = GRABCIRC;
						gps->grabcf = (gquantcf&1);
						if (doplaysound)
						{
							fp.x = qmousx; fp.y = qmousy; fp.z = qmousz; //(sec[gps->grabsect].z[0]+sec[gps->grabsect].z[1])*.5;
							myplaysound("sounds\\hammer.wav",100,1.0,&fp,KSND_3D);
						}
					}
					else
					{
						if ((hitwall&0xc0000000) == 0x40000000)
						{
							spr = &gst->spri[hitwall&0x3fffffff];
							gps->selrgbintens = sqrt(((double)spr->rsc)*((double)spr->rsc) +
															 ((double)spr->gsc)*((double)spr->gsc) +
															 ((double)spr->bsc)*((double)spr->bsc))/4096.0;
							gps->selrgbbak[1] = spr->rsc;
							gps->selrgbbak[2] = spr->gsc;
							gps->selrgbbak[3] = spr->bsc;
						}
						else
						{
							if ((unsigned)hitsect >= (unsigned)gst->numsects) break;
							if (hitwall < 0) sur = &sec[hitsect].surf[hitwall&1];
											else sur = &sec[hitsect].wall[hitwall].surf;
							gps->selrgbintens = sqrt(((double)sur->rsc)*((double)sur->rsc) +
															 ((double)sur->gsc)*((double)sur->gsc) +
															 ((double)sur->bsc)*((double)sur->bsc))/4096.0;
							gps->selrgbbak[1] = sur->rsc;
							gps->selrgbbak[2] = sur->gsc;
							gps->selrgbbak[3] = sur->bsc;
						}
						releasegrabdrag(curindex,doplaysound);
						gps->grabmode = GRABRGB; gps->grabcf = (hitwall&1);
						gps->grabsect = hitsect;
						gps->grabwall = hitwall;

						for(j=numplayers-1;j>=0;j--) //If somebody else grabbed same surface, copy their backup col :)
						{
							if (j == curindex) continue;
							if ((gst->p[j].grabmode == GRABRGB) && (gst->p[j].grabsect == hitsect) && (gst->p[j].grabwall == hitwall))
							{
								gps->selrgbbak[1] = gst->p[j].selrgbbak[1];
								gps->selrgbbak[2] = gst->p[j].selrgbbak[2];
								gps->selrgbbak[3] = gst->p[j].selrgbbak[3];
							}
						}
					}
					break;
				case 0x21: //'F','f': SOS test key :P
					if ((gps->sec.n) || (gps->editmode == 2)) break;
					hitscan(&gps->ipos,&gps->ifor,1e32,gps->cursect,&hitsect,&hitwall,&hit);
					if ((unsigned)hitsect >= (unsigned)gst->numsects) break;

					if (hitwall >= 0) break; //FIX: disables 'F' on wall

					if (doplaysound) myplaysound("sounds\\deleteroom.wav",100,1.0,&hit,KSND_3D);

					if ((unsigned)hitwall < (unsigned)sec[hitsect].n)
					{
						if (gst->numsects+2 >= gst->malsects)
						{
							i = gst->malsects; gst->malsects = max(gst->numsects+2,gst->malsects<<1);
							sec = gst->sect = (sect_t *)realloc(gst->sect,gst->malsects*sizeof(sect_t));
							memset(&sec[i],0,(gst->malsects-i)*sizeof(sect_t));
						}
						x0 = sec[hitsect].wall[hitwall].x;
						y0 = sec[hitsect].wall[hitwall].y; i = sec[hitsect].wall[hitwall].n+hitwall;
						x1 = sec[hitsect].wall[i].x;
						y1 = sec[hitsect].wall[i].y;
						dx = x1-x0;
						dy = y1-y0;
						for(s=gst->numsects;s<gst->numsects+2;s++)
						{
							sec[s].n = sec[s].nmax = 4;
							sec[s].wall = (wall_t *)malloc(sec[s].nmax*sizeof(wall_t)); //if (!sec[s].wall) ?; //FIX!
							memset(sec[s].wall,0,sec[s].nmax*sizeof(wall_t));
							for(w=0;w<4;w++)
							{
								switch(w)
								{
									case 0: sec[s].wall[w].x = x0; sec[s].wall[w].y = y0; break;
									case 1: sec[s].wall[w].x = x0+dy; sec[s].wall[w].y = y0-dx; break;
									case 2: sec[s].wall[w].x = x1+dy; sec[s].wall[w].y = y1-dx; break;
									case 3: sec[s].wall[w].x = x1; sec[s].wall[w].y = y1; break;
								}
								sec[s].wall[w].owner = -1;
								sec[s].wall[w].n = ((w+1)&3)-w;
								sec[s].wall[w].surf.uv[1].x = sec[s].wall[w].surf.uv[2].y = 1.f;
								sec[s].wall[w].surf.asc = 4096;
								sec[s].wall[w].surf.rsc = 4096; //FIXFIX: use curcol
								sec[s].wall[w].surf.gsc = 4096;
								sec[s].wall[w].surf.bsc = 4096;
							}
							for(j=0;j<2;j++)
							{
								sec[s].z[j] = (sec[hitsect].z[1]-sec[hitsect].z[0])*(((float)(s-gst->numsects))*.4+(.2+(float)j*.2)) + sec[hitsect].z[0];
								sec[s].grad[j].x = sec[s].grad[j].y = 0.f;
								sec[s].surf[j].uv[0].x = 0.f; sec[s].surf[j].uv[0].y = 0.f;
								sec[s].surf[j].uv[1].x = 1.f; sec[s].surf[j].uv[1].y = 0.f;
								sec[s].surf[j].uv[2].x = 0.f; sec[s].surf[j].uv[2].y = 1.f;
								sec[s].surf[j].asc = 4096;
								sec[s].surf[j].rsc = 4096+(j*2-1)*512; //FIXFIX: use curcol
								sec[s].surf[j].gsc = 4096+(j*2-1)*512;
								sec[s].surf[j].bsc = 4096+(j*2-1)*512;
								sec[s].headspri = -1; sec[s].owner = -1;
							}
						}
						gst->numsects += 2;
					}
					else
					{
						if (gst->numsects+1 >= gst->malsects)
						{
							i = gst->malsects; gst->malsects = max(gst->numsects+1,gst->malsects<<1);
							sec = gst->sect = (sect_t *)realloc(gst->sect,gst->malsects*sizeof(sect_t));
							memset(&sec[i],0,(gst->malsects-i)*sizeof(sect_t));
						}
						s = gst->numsects;

						sec[s].n = sec[hitsect].n;
						sec[s].nmax = sec[hitsect].nmax;
						sec[s].wall = (wall_t *)malloc(sec[s].nmax*sizeof(wall_t)); //if (!sec[s].wall) ?; //FIX!
						memcpy(sec[s].wall,sec[hitsect].wall,sec[s].n*sizeof(wall_t));
						y0 = sec[hitsect].z[0]; dy = sec[hitsect].z[1]-y0;

						sec[s].z[1] = sec[hitsect].z[1];
						sec[s].grad[1].x = sec[hitsect].grad[1].x;
						sec[s].grad[1].y = sec[hitsect].grad[1].y;
						sec[s].surf[1] = sec[hitsect].surf[1];

						f = .75;
						sec[s].z[0] = dy*f + y0;
						sec[s].grad[0].x = (sec[hitsect].grad[1].x-sec[hitsect].grad[0].x)*f + sec[hitsect].grad[0].x;
						sec[s].grad[0].y = (sec[hitsect].grad[1].y-sec[hitsect].grad[0].y)*f + sec[hitsect].grad[0].y;
						sec[s].surf[0].uv[0].x = 0.f; sec[s].surf[0].uv[0].y = 0.f;
						sec[s].surf[0].uv[1].x = 1.f; sec[s].surf[0].uv[1].y = 0.f;
						sec[s].surf[0].uv[2].x = 0.f; sec[s].surf[0].uv[2].y = 1.f;
						sec[s].surf[0].asc = 4096;
						sec[s].surf[0].rsc = 4096-512; //FIXFIX: use curcol
						sec[s].surf[0].gsc = 4096-512;
						sec[s].surf[0].bsc = 4096-512;
						sec[s].headspri = -1; sec[s].owner = -1;

						f = .25;
						sec[hitsect].z[1] = dy*f + y0;
						sec[hitsect].grad[1].x = (sec[hitsect].grad[1].x-sec[hitsect].grad[0].x)*f + sec[hitsect].grad[0].x;
						sec[hitsect].grad[1].y = (sec[hitsect].grad[1].y-sec[hitsect].grad[0].y)*f + sec[hitsect].grad[0].y;
						sec[hitsect].surf[1].uv[0].x = 0.f; sec[hitsect].surf[1].uv[0].y = 0.f;
						sec[hitsect].surf[1].uv[1].x = 1.f; sec[hitsect].surf[1].uv[1].y = 0.f;
						sec[hitsect].surf[1].uv[2].x = 0.f; sec[hitsect].surf[1].uv[2].y = 1.f;
						sec[hitsect].surf[1].asc = 4096;
						sec[hitsect].surf[1].rsc = 4096+512; //FIXFIX: use curcol
						sec[hitsect].surf[1].gsc = 4096+512;
						sec[hitsect].surf[1].bsc = 4096+512;
						sec[hitsect].headspri = -1; sec[hitsect].owner = -1;

						gst->numsects++;
					}
					checknextwalls();
					checksprisect(-1);
					break;
			}
		}
		switch((key>>8)&0xff)
		{
			case 0x1c: //Left Enter
				if ((key&0x300000) && (key&30000)) //Alt+Shift+Left Enter (change all textures)
				{
					if (gps->gotcopy&1)
					{
						if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; } else hitsect = -1;
						if ((unsigned)hitsect >= (unsigned)gst->numsects)
						{
							hitscan(&gps->ipos,&gps->ifor,1e32,gps->cursect,&hitsect,&hitwall,&hit);
							if ((unsigned)hitsect >= (unsigned)gst->numsects) break;
						}
						if (hitwall < 0) i = sec[hitsect].surf[hitwall&1].tilnum;
										else i = sec[hitsect].wall[hitwall].surf.tilnum;
						for(s=gst->numsects-1;s>=0;s--) //FIXFIXFIX
						{
							for(w=2-1;w>=0;w--)        { if (sec[s].surf[w].tilnum      == i) sec[s].surf[w].tilnum      = gps->copysurf[0].tilnum; }
							for(w=sec[s].n-1;w>=0;w--) { if (sec[s].wall[w].surf.tilnum == i) sec[s].wall[w].surf.tilnum = gps->copysurf[0].tilnum; }
						}
						if (doplaysound) //Let user know they did some serious business!
						{
							myplaysound("sounds\\hammer.wav",100,0.5,&hit,KSND_3D);
							myplaysound("sounds\\hammer.wav",100,0.4,&hit,KSND_3D);
						}
					}
				}
				else if (key&0xc0000) //Ctrl+Left Enter
				{
					if (gps->gotcopy&1)
					{
						if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; } else hitsect = -1;
						if ((unsigned)hitsect >= (unsigned)gst->numsects)
						{
							hitscan(&gps->ipos,&gps->ifor,1e32,gps->cursect,&hitsect,&hitwall,&hit);
							if ((unsigned)hitsect >= (unsigned)gst->numsects) break;
						}
						if (hitwall < 0) { sec[hitsect].surf[hitwall&1] = gps->copysurf[0]; break; }
						if (hitwall >= sec[hitsect].n) break;
						i = hitwall;
						do
						{
							if (!(key&0x30000))
								sec[hitsect].wall[i].surf = gps->copysurf[0];
							else
							{
								sec[hitsect].wall[i].surf.asc = gps->copysurf[0].asc;
								sec[hitsect].wall[i].surf.rsc = gps->copysurf[0].rsc;
								sec[hitsect].wall[i].surf.gsc = gps->copysurf[0].gsc;
								sec[hitsect].wall[i].surf.bsc = gps->copysurf[0].bsc;
							}
							i += sec[hitsect].wall[i].n;
						} while (i != hitwall);
					}
					break;
				}
				break;
			case 0xd2: //Insert
				getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);

					//Duplicate sectors currently selected by Alt+highlight
				if (((unsigned)hitsect < (unsigned)gst->numsects) && (gps->editmode == 2) && (gst->sect[hitsect].owner == curindex) && (gps->grabmode == GRABDRAG3))
				{
						//temp lut for remapping newly allocated sec[].ns (without this, infinite loop may occur elsewhere in code during dragging! ;/)
					long *sectlut = (long *)malloc(gst->numsects*sizeof(long));
					i = gst->numsects;
					for(s=gst->numsects-1;s>=0;s--) { if (gst->sect[s].owner == curindex) { sectlut[s] = i; i++; } }

					for(s=gst->numsects-1;s>=0;s--)
					{
						if (gst->sect[s].owner != curindex) continue;

						if (gst->numsects+1 >= gst->malsects)
						{
							i = gst->malsects; gst->malsects = max(gst->numsects+1,gst->malsects<<1);
							sec = gst->sect = (sect_t *)realloc(gst->sect,gst->malsects*sizeof(sect_t));
							memset(&sec[i],0,(gst->malsects-i)*sizeof(sect_t));
						}
						memcpy(&sec[gst->numsects],&sec[s],sizeof(sect_t));
						sec[gst->numsects].wall = (wall_t *)malloc(sec[gst->numsects].nmax*sizeof(wall_t)); //if (!sec[gst->numsects].wall) ?; //FIX!
						memcpy(sec[gst->numsects].wall,sec[s].wall,sec[gst->numsects].n*sizeof(wall_t));

							//remap sec[].ns (sec[].nw needs no remapping)
						for(i=0;i<sec[gst->numsects].n;i++)
						{
							if ((unsigned)sec[gst->numsects].wall[i].ns >= (unsigned)gst->numsects) continue;
							if (sec[sec[gst->numsects].wall[i].ns].owner != curindex)
								{ sec[gst->numsects].wall[i].ns = -1; sec[gst->numsects].wall[i].nw = -1; continue; }
							sec[gst->numsects].wall[i].ns = sectlut[sec[gst->numsects].wall[i].ns];
						}

						if (doplaysound) myplaysound("sounds\\hammer.wav",100,1.0,&hit,KSND_3D);

						sec[gst->numsects].headspri = -1; sec[gst->numsects].owner = -1;
						gst->numsects++;

						for(w=gst->sect[s].headspri;w>=0;w=spr->sectn)
						{
							spr = &gst->spri[w];

							i = insspri(gst->numsects-1,spr->p.x,spr->p.y,spr->p.z);
							j = gst->spri[i].sectn; k = gst->spri[i].sectp;
							memcpy(&gst->spri[i],spr,sizeof(spri_t));
							gst->spri[i].sectn = j; gst->spri[i].sectp = k;
						}
					}

					free(sectlut);

					shadowtest2_updatelighting = 1;
					break;
				}
				else
				{
					if (gquantstat != 1)
					{
						if (gps->editmode != 3) break; //FIXFIX
						if ((unsigned)hitsect >= (unsigned)gst->numsects) break;
						if ((unsigned)hitwall >= (unsigned)gst->sect[hitsect].n) break;
						gquantsec = hitsect;
						gquantwal = hitwall;
					}

					bs = gquantsec;
					bw = gquantwal;

					if (doplaysound)
					{
						fp.x = qmousx; fp.y = qmousy; fp.z = (sec[bs].z[0]+sec[bs].z[1])*.5;
						myplaysound("sounds\\hammer.wav",100,1.0,&fp,KSND_3D);
					}

					i = dupwall(&sec[bs],bw);
					wal = sec[bs].wall;
					wal[i].x = qmousx;
					wal[i].y = qmousy;

					s = wal[bw].ns; if (s < 0) { checknextwalls(); checksprisect(-1); break; }
					w = wal[bw].nw;
					do
					{
						j = dupwall(&sec[s],w);
						wal = sec[s].wall;
						wal[j].x = qmousx;
						wal[j].y = qmousy;
						s = wal[w].ns; if ((s < 0) || (s == bs)) break;
						w = wal[w].nw;
					} while (1);
				}
				checknextwalls();
				checksprisect(-1);
				break;
			case 0xd3: //Delete
				if (key&0xc0000) //Ctrl+Delete
				{
					if (gps->sec.n) break;

					if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; }
					else getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
					if ((unsigned)hitsect >= (unsigned)gst->numsects) break;

					if (doplaysound) myplaysound("sounds\\deleteroom.wav",100,1.0,&hit,KSND_3D);

						//Delete sectors currently selected by Alt+highlight
					if ((gps->editmode == 2) && (sec[hitsect].owner == curindex)) //Do not also require LMB to delete Alt+highlighted sectors && (gps->grabmode == GRABDRAG3))
					{
						for(s=gst->numsects-1;s>=0;s--)
						{
							if (sec[s].owner != curindex) continue;
							while (sec[s].headspri >= 0) delspri(sec[s].headspri);
							delsect(s);
						}
					}
					else
					{
						while (sec[hitsect].headspri >= 0) delspri(sec[hitsect].headspri);
						delsect(hitsect);
					}

					checknextwalls();
					checksprisect(-1);
					break;
				}

					//Delete sprite
				if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; }
				else getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
				if ((hitwall&0xc0000000) == 0x40000000) //hit sprite
				{
					i = (hitwall&0x3fffffff);
					if (doplaysound) myplaysound("sounds\\deleteroom.wav",100,1.0,&gst->spri[i],KSND_3D);
					delspri(i); shadowtest2_updatelighting = 1;
				}

				break;
			case 0x26: //'L'
				if (key&0xc0000) //Ctrl+L (load)
				{
					if (numplayers > 1)
					{
						if (curindex == viewindex)
						{
							messagetimeout = dtotclk+3.0;
							strcpy(message,"Sorry, network load not yet supported!");
						}
					}
					else
					{
						ddflip2gdi(); if (mouseacquired) { mouseacquired = 0; setacquire(0,1); }
						cptr = (char *)loadfileselect("Load MAP..","MAP\0*.map\0All files (*.*)\0*.*\0\0","MAP");
						if (cptr) { loadmap(cptr); sec = gst->sect; }
						keystatus[0x1d] = keystatus[0x9d] = 0;
					}
				}
				break;
			case 0x1f: //'S'
				if (key&0xc0000) //Ctrl+S (save)
				{
					if (gst == &sst)
					{
						ddflip2gdi(); if (mouseacquired) { mouseacquired = 0; setacquire(0,1); }
						cptr = (char *)savefileselect("Save MAP AS..","*.MAP\0*.map\0*.KC\0*.kc\0*.STL\0*.stl\0All files (*.*)\0*.*\0\0","MAP");
						if (cptr) savemap(cptr);
						keystatus[0x1d] = keystatus[0x9d] = 0;
					}
					break;
				}
				if (key&0x300000) //Alt+S
				{
					getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
					if ((gquantstat < 0) && (gps->editmode == 3) && ((unsigned)hitsect < (unsigned)gst->numsects) && ((unsigned)hitwall < (unsigned)gst->sect[hitsect].n))
						{ gquantstat = 0; gquantsec = hitsect; gquantwal = hitwall; }
					if ((gquantstat < 0) || (gquantstat > 1)) break;

					if (gst->numsects+1 >= gst->malsects)
					{
						i = gst->malsects; gst->malsects = max(gst->numsects+1,gst->malsects<<1);
						sec = gst->sect = (sect_t *)realloc(gst->sect,gst->malsects*sizeof(sect_t));
						memset(&sec[i],0,(gst->malsects-i)*sizeof(sect_t));
					}
					s = gst->numsects;

					sec[s].n = 0; sec[s].nmax = 8;
					sec[s].wall = (wall_t *)malloc(sec[s].nmax*sizeof(wall_t)); //if (!sec[s].wall) ?; //FIX!

					bs = gquantsec; bw = gquantwal;
					do
					{
						if (sec[s].n >= sec[s].nmax)
						{
							sec[s].nmax <<= 1;
							sec[s].wall = (wall_t *)realloc(sec[s].wall,sec[s].nmax*sizeof(wall_t)); //if (!sec[s].wall) ?; //FIX!
						}
						memcpy(&sec[s].wall[sec[s].n],&sec[bs].wall[bw],sizeof(wall_t));
						sec[s].wall[sec[s].n].ns = bs; sec[s].wall[sec[s].n].nw = bw; //Infinite loop check
						sec[s].wall[sec[s].n].n = 1; sec[s].n++;

						wal = sec[bs].wall; bw += wal[bw].n; //FIXFIX!
						j = MAXVERTS;
						while (wal[bw].ns >= 0)
						{
							j--; if (j < 0) goto alts_screwed; //Another infinite loop check - very necessary!
							bs = wal[bw].ns;
							bw = wal[bw].nw;
							wal = sec[bs].wall; bw += wal[bw].n;
						}

							//Infinite loop check (skip index 0: that's the one that SHOULD be duplicated)
						for(w=sec[s].n-1;w>0;w--)
							if ((sec[s].wall[w].ns == bs) && (sec[s].wall[w].nw == bw)) goto alts_screwed;

					} while ((bs != gquantsec) || (bw != gquantwal));
					sec[s].wall[sec[s].n-1].n = -(sec[s].n-1);

					if (getarea(sec[s].wall,sec[s].n) >= 0)
					{
						free(sec[s].wall);
						memset(&sec[s],0,sizeof(sect_t));
						break;
					}
					if (doplaysound) myplaysound("sounds\\deleteroom.wav",100,1.0,&hit,KSND_3D);
					reversewalls(sec[s].wall,sec[s].n);

					for(j=0;j<2;j++)
					{
						sec[s].z[j] = getslopez(&sec[gquantsec],j,sec[s].wall[0].x,sec[s].wall[0].y);
						sec[s].grad[j].x = sec[gquantsec].grad[j].x;
						sec[s].grad[j].y = sec[gquantsec].grad[j].y;
						sec[s].surf[j].uv[1].x = sec[s].surf[j].uv[2].y = 1.f;
						sec[s].surf[j].asc = 4096;
						sec[s].surf[j].rsc = 4096+(j*2-1)*512; //FIXFIX: use curcol
						sec[s].surf[j].gsc = 4096+(j*2-1)*512;
						sec[s].surf[j].bsc = 4096+(j*2-1)*512;
					}
					sec[s].headspri = -1; sec[s].owner = -1;
					gst->numsects++;

					checknextwalls();
					checksprisect(-1);
					break;

alts_screwed:  free(sec[gst->numsects].wall);
					memset(&sec[gst->numsects],0,sizeof(sect_t));
					break;
				}
				break;
			case 0x30: //'B'
				if (key&0xc0000) //Ctrl+B
				{
					gps->boolfunc++; if (gps->boolfunc > POLYBOOL_END) gps->boolfunc = 0;

					if (curindex == viewindex)
					{
						messagetimeout = dtotclk+3.0; strcpy(message,"Boolean Op: ");
						switch(gps->boolfunc)
						{
							case POLYBOOL_AND:  strcat(message,"AND" ); break;
							case POLYBOOL_SUB:  strcat(message,"SUB" ); break;
							case POLYBOOL_SUBR: strcat(message,"SUBR"); break;
							case POLYBOOL_OR:   strcat(message,"OR"  ); break;
						 //case POLYBOOL_XOR:  strcat(message,"XOR" ); break;
							case POLYBOOL_END:  strcat(message,"NONE"); break;
						}
					}
				}
				break;
			case 0x2e: //'C'
				if (key&0xc0000) //Ctrl+C
				{
					gps->docollide = (!gps->docollide);
					if (curindex == viewindex)
					{
						messagetimeout = dtotclk+3.0;
						if (!gps->docollide) strcpy(message,"Collision OFF");
											 else strcpy(message,"Collision ON");
					}
				}
				break;
			case 0x22: //'G'
				if (key&0xc0000) //Ctrl+G
				{
					if (gps->editmode == 3)
					{
						if (key&0x30000) gps->dgridlock = min(gps->dgridlock*2.0,8.0/  1.0); //Shift+Ctrl+G
										else gps->dgridlock = max(gps->dgridlock*0.5,1.0/128.0); //Ctrl+G
					}
				}
				break;
			case 0x14: //'T'
				if (key&0xc0000) { gps->showedges3d = !gps->showedges3d; break; } //Ctrl+T
				if (key&0x300000) //Alt+T
				{
					if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; }
					else getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
					if ((unsigned)hitsect >= (unsigned)gst->numsects) break;

					if ((hitwall&0xc0000000) == 0x40000000) i = gst->spri[hitwall&0x3fffffff].tag;
											else if (hitwall < 0) i = sec[hitsect].surf[hitwall&1].tag;
						  else if (hitwall < sec[hitsect].n) i = sec[hitsect].wall[hitwall].surf.tag;

					unsigned short lotag, hitag;
					// Extract tag components
					lotag = i & 0xFFFF;
					hitag = (i >> 16) & 0xFFFF;

					gps->grabsect = hitsect; gps->grabwall = hitwall;
					gps->typemode = 1;
					gps->typehighlight = 5;
					sprintf(gst->typemess[curindex],"/tag=%d,%d",lotag, hitag);
					gps->typecurs = strlen(gst->typemess[curindex]);
					break;
				}
				break;
			case 0x23: //'H'
				if (key&0xc0000) //Ctrl+H (set hinge)
				{
					if (getcurswall(gps,&hitsect,&hitwall,&hit) < 0) break;

					if (doplaysound) myplaysound("sounds\\hammer.wav",100,0.75,&hit,KSND_3D);

						//Set i = first wall of hitwall's loop
						//Set j = last  wall of hitwall's loop (inclusive)
					wal = sec[hitsect].wall;
					for(j=hitwall;wal[j].n>0;j+=wal[j].n);
					i = j+wal[j].n;
					if (i)
					{
							//Rotate loops so selected one is now on first loop
							//
							//Inclusive sector loops before: [0..i-1] | [i..j] [j+1..sec[hitsect].n-1]
							//Inclusive sector loops  after: | [i..j] [j+1..sec[hitsect].n-1] [0..i-1]
						for(k=(i>>1)-1;k>=0;k--) //reverse [0..i-1]
							{ twal = wal[k]; wal[k] = wal[i-1-k]; wal[i-1-k] = twal; }
						for(k=((sec[hitsect].n-i)>>1)-1;k>=0;k--) //reverse [i..sec[hitsect].n-1]
							{ twal = wal[i+k]; wal[i+k] = wal[sec[hitsect].n-1-k]; wal[sec[hitsect].n-1-k] = twal; }
						for(k=(sec[hitsect].n>>1)-1;k>=0;k--) //reverse [0..sec[hitsect].n-1]
							{ twal = wal[k]; wal[k] = wal[sec[hitsect].n-1-k]; wal[sec[hitsect].n-1-k] = twal; }

						j -= i; hitwall -= i; i = 0;
					}

					sec[hitsect].z[0] = getslopez(&sec[hitsect],0,wal[hitwall].x,wal[hitwall].y);
					sec[hitsect].z[1] = getslopez(&sec[hitsect],1,wal[hitwall].x,wal[hitwall].y);

						//Rotate walls so hitwall is first. (Before rotation: i..hitwall-1, hitwall..j)
					for(k=(((hitwall-1)-i-1)>>1);k>=0;k--) //reverse loop wal[i<=?<=j].x&y
						{ twal = wal[i+k]; wal[i+k] = wal[(hitwall-1)-k]; wal[(hitwall-1)-k] = twal; }
					for(k=((j-(hitwall)-1)>>1);k>=0;k--) //reverse loop wal[i<=?<=j].x&y
						{ twal = wal[(hitwall)+k]; wal[(hitwall)+k] = wal[j-k]; wal[j-k] = twal; }
					for(k=((j-i-1)>>1);k>=0;k--) //reverse loop wal[i<=?<=j].x&y
						{ twal = wal[i+k]; wal[i+k] = wal[j-k]; wal[j-k] = twal; }

					for(k=i;k<j;k++) wal[k].n = 1;
					wal[j].n = i-j;

					checknextwalls();
					checksprisect(-1);
					break;
				}
				break;
			case 0x47: case 0x48: case 0x49: //KP7,KP8,KP9
			case 0x4b: case 0x4c: case 0x4d: //KP4,KP5,KP6
			case 0x4f: case 0x50: case 0x51: //KP1,KP2,KP3
										 case 0x53: //        KP.
				if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; }
				else getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
				if ((unsigned)hitsect >= (unsigned)gst->numsects) break;

				if ((hitwall&0xc0000000) == 0x40000000)
				{
					spr = &gst->spri[hitwall&0x3fffffff];
					if (spr->fat != 0.f)
					{
						if ((((key>>8)&255) == 0x4b) || (((key>>8)&255) == 0x4d)) //KP4,KP6
						{
							if (((key>>8)&255) == 0x4b) d = pow(2.0,-1.0/8.0);
														  else d = pow(2.0, 1.0/8.0);
							spr->r.x *= d; spr->r.y *= d;
							spr->d.x *= d; spr->d.y *= d;
							spr->f.x *= d; spr->f.y *= d;
						}
						else if ((((key>>8)&255) == 0x48) || (((key>>8)&255) == 0x50)) //KP2,KP8
						{
							if (((key>>8)&255) == 0x48) d = pow(2.0,-1.0/8.0);
														  else d = pow(2.0, 1.0/8.0);
							spr->r.x *= d; spr->r.y *= d; spr->r.z *= d;
							spr->d.x *= d; spr->d.y *= d; spr->d.z *= d;
							spr->f.x *= d; spr->f.y *= d; spr->f.z *= d;
						}
						spr->fat = sqrt((spr->r.x*spr->r.x + spr->r.y*spr->r.y + spr->r.z*spr->r.z +
											  spr->d.x*spr->d.x + spr->d.y*spr->d.y + spr->d.z*spr->d.z +
											  spr->f.x*spr->f.x + spr->f.y*spr->f.y + spr->f.z*spr->f.z) / 3.0);
					}
					else
					{
						if ((((key>>8)&255) == 0x4b) || (((key>>8)&255) == 0x4d)) //KP4,KP6
						{
							if (((key>>8)&255) == 0x4b) d = pow(2.0,-1.0/8.0);
														  else d = pow(2.0, 1.0/8.0);
							spr->r.x *= d; spr->r.y *= d; spr->r.z *= d;
						}
						else if ((((key>>8)&255) == 0x48) || (((key>>8)&255) == 0x50)) //KP2,KP8
						{
							if (((key>>8)&255) == 0x48) d = pow(2.0,-1.0/8.0);
														  else d = pow(2.0, 1.0/8.0);
							spr->d.x *= d; spr->d.y *= d; spr->d.z *= d;
							spr->f.x *= d; spr->f.y *= d; spr->f.z *= d;
						}
					}
					break;
				}

				if (hitwall < 0) sur = &sec[hitsect].surf[hitwall&1];
								else sur = &sec[hitsect].wall[hitwall].surf;

					//NOTE: Shift+KP# doesn't work right for keyread() when Numlock is on!
				if ((!(key&0x30000)) && (!(TESTKEY(0x2a)|TESTKEY(0x36)|TESTKEY(0x1d)))) //KP#
				{
					surf_t osurf = *sur;
					j = -1;
					switch((key>>8)&255)
					{
						case 0x4b: j = 0; break;
						case 0x48: j = 1; break;
						case 0x4d: j = 2; break;
						case 0x50: j = 3; break;
						case 0x4c: //KP5: reset texture
							sur->uv[0].x = 0.f; sur->uv[0].y = 0.f;
							sur->uv[1].x = 1.f; sur->uv[1].y = 0.f;
							sur->uv[2].x = 0.f; sur->uv[2].y = 1.f;
							if (doplaysound) myplaysound("sounds\\rumble.wav",50,4.0,&hit,KSND_3D);
							break;
						case 0x47: case 0x4f: //KP7,KP1
							if (((key>>8)&255) == 0x4f) f = -15*PI/180.0;
														  else f = +15*PI/180.0;
							sur->uv[0].x = 0;
							sur->uv[0].y = 0;
							sur->uv[1].x = osurf.uv[1].x*cos(f) - osurf.uv[1].y*sin(f);
							sur->uv[1].y = osurf.uv[1].y*cos(f) + osurf.uv[1].x*sin(f);
							sur->uv[2].x = osurf.uv[2].x*cos(f) - osurf.uv[2].y*sin(f);
							sur->uv[2].y = osurf.uv[2].y*cos(f) + osurf.uv[2].x*sin(f);

							for(i=3-1;i>=0;i--) //re-quantize texture parameters to nice values every 90 degrees :)
							{
								g = 4.0; f = floor(sur->uv[i].x*g+.5)/g; if (fabs(sur->uv[i].x-f) < 1e-5) sur->uv[i].x = f;
								g = 4.0; f = floor(sur->uv[i].y*g+.5)/g; if (fabs(sur->uv[i].y-f) < 1e-5) sur->uv[i].y = f;
							}
							if (doplaysound) myplaysound("sounds\\hammer.wav",50,7.0,&hit,KSND_3D);
							break;
						case 0x49: sur->asc = min(max(sur->asc-1024,0),65535); break;
						case 0x51: sur->asc = min(max(sur->asc+1024,0),65535); break;
						case 0x53: sur->uv[1].x *= -1.0; sur->uv[2].x *= -1.0; if (doplaysound) myplaysound("sounds\\hammer.wav",50,6.0,&hit,KSND_3D); break;
					}
					if (j >= 0)
					{
						f = 1.0; g = 0.0;
						if ((hitwall < 0) && (!(sur->flags&4))) //ceil/flor and not relative alignment
						{
							if (fabs(gps->irig.x) > fabs(gps->irig.y)) { if (gps->irig.x > 0) { f = 1.0; g = 0.0; } else { f = -1.0; g = 0.0; } }
																			  else { if (gps->irig.y > 0) { f = 0.0; g = 1.0; } else { f =  0.0; g =-1.0; } }
							//if (hitwall == -2) g *= -1.0;
						}

						if (g != 0.0) j ^= 1;
						switch(j&3)
						{
							case 0: sur->uv[1].x *= pow(2.0,1.0/8.0); break;
							case 1: sur->uv[2].y *= pow(2.0,1.0/8.0); break;
							case 2: sur->uv[1].x /= pow(2.0,1.0/8.0); break;
							case 3: sur->uv[2].y /= pow(2.0,1.0/8.0); break;
						}
						if (hitwall < 0)
						{
							sur->uv[0].x = (osurf.uv[1].x - sur->uv[1].x)*hit.x +
												(osurf.uv[2].x - sur->uv[2].x)*hit.y + osurf.uv[0].x;
							sur->uv[0].y = (osurf.uv[1].y - sur->uv[1].y)*hit.x +
												(osurf.uv[2].y - sur->uv[2].y)*hit.y + osurf.uv[0].y;
						}
						if (doplaysound) myplaysound("sounds\\hammer.wav",50,6.0,&hit,KSND_3D);
					}
				}
				else //Shift+KP#
				{
					surf_t osurf = *sur;

					f = 1.0; g = 0.0;
					if ((hitwall < 0) && (!(sur->flags&4))) //ceil/flor and not relative alignment
					{
						if (fabs(gps->irig.x) > fabs(gps->irig.y)) { if (gps->irig.x > 0) { f = 1.0; g = 0.0; } else { f = -1.0; g = 0.0; } }
																		  else { if (gps->irig.y > 0) { f = 0.0; g = 1.0; } else { f =  0.0; g =-1.0; } }
						//if (hitwall == -2) g *= -1.0;
					}

					switch((key>>8)&255)
					{
						case 0x4b: sur->uv[0].x -= f/32.0; sur->uv[0].y -= g/32.0; break;
						case 0x4d: sur->uv[0].x += f/32.0; sur->uv[0].y += g/32.0; break;
						case 0x48: sur->uv[0].x += g/32.0; sur->uv[0].y -= f/32.0; break;
						case 0x50: sur->uv[0].x -= g/32.0; sur->uv[0].y += f/32.0; break;
						case 0x49: sur->asc = min(max(sur->asc-256,0),65535); break;
						case 0x51: sur->asc = min(max(sur->asc+256,0),65535); break;
						case 0x47: case 0x4f: //KP7,KP1
							if (((key>>8)&255) == 0x4f) f = -1*PI/180.0;
														  else f = +1*PI/180.0;
							sur->uv[0].x = 0;
							sur->uv[0].y = 0;
							sur->uv[1].x = osurf.uv[1].x*cos(f) - osurf.uv[1].y*sin(f);
							sur->uv[1].y = osurf.uv[1].y*cos(f) + osurf.uv[1].x*sin(f);
							sur->uv[2].x = osurf.uv[2].x*cos(f) - osurf.uv[2].y*sin(f);
							sur->uv[2].y = osurf.uv[2].y*cos(f) + osurf.uv[2].x*sin(f);

							for(i=3-1;i>=0;i--) //re-quantize texture parameters to nice values every 90 degrees :)
							{
								g = 4.0; f = floor(sur->uv[i].x*g+.5)/g; if (fabs(sur->uv[i].x-f) < 1e-5) sur->uv[i].x = f;
								g = 4.0; f = floor(sur->uv[i].y*g+.5)/g; if (fabs(sur->uv[i].y-f) < 1e-5) sur->uv[i].y = f;
							}
							if (doplaysound) myplaysound("sounds\\hammer.wav",50,7.0,&hit,KSND_3D);
							break;
					}

					if (sur->uv[0].x < 0.f) sur->uv[0].x += 1.f;
					if (sur->uv[0].x > 1.f) sur->uv[0].x -= 1.f;
					if (sur->uv[0].y < 0.f) sur->uv[0].y += 1.f;
					if (sur->uv[0].y > 1.f) sur->uv[0].y -= 1.f;
					if (doplaysound) myplaysound("sounds\\hammer.wav",50,8.0,&hit,KSND_3D);
				}
				break;

			case 0xc7: case 0xcf: //Home/End
				if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; }
				else getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
				if ((hitwall&0xc0000000) == 0x40000000) //Rotate sprite vert
				{
					spr = &gst->spri[hitwall&0x3fffffff];
					if (!(key&0x30000)) { x0 = cos(PI/  8.0); y0 = sin(PI/  8.0); }
										else { x0 = cos(PI/256.0); y0 = sin(PI/256.0); } //Shift+Home/End = slower
					if (((key>>8)&255) == 0xcf) y0 = -y0;
					f = spr->d.x; spr->d.x = x0*spr->d.x - y0*spr->f.x; spr->f.x = x0*spr->f.x + y0*f;
					f = spr->d.y; spr->d.y = x0*spr->d.y - y0*spr->f.y; spr->f.y = x0*spr->f.y + y0*f;
					f = spr->d.z; spr->d.z = x0*spr->d.z - y0*spr->f.z; spr->f.z = x0*spr->f.z + y0*f;
					if (doplaysound)
					{
						if (key&0x30000) myplaysound("sounds\\rumble.wav",12,8.0,&spr->p,KSND_3D);
										else myplaysound("sounds\\rumble.wav",25,4.0,&spr->p,KSND_3D);
					}
				}
				break;

			case 0x33: case 0x34: //Ctrl+,.
				if (!(key&0xc0000)) break;

				if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; }
				else getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
				if ((hitwall&0xc0000000) == 0x40000000) //Rotate sprite tilt
				{
					spr = &gst->spri[hitwall&0x3fffffff];
					if (!(key&0x30000)) { x0 = cos(PI/  8.0); y0 = sin(PI/  8.0); }
										else { x0 = cos(PI/256.0); y0 = sin(PI/256.0); } //Shift+Ctrl+,. = slower
					if (((key>>8)&255) == 0x33) y0 = -y0;

					f = spr->d.x; spr->d.x = x0*spr->d.x - y0*spr->r.x; spr->r.x = x0*spr->r.x + y0*f;
					f = spr->d.y; spr->d.y = x0*spr->d.y - y0*spr->r.y; spr->r.y = x0*spr->r.y + y0*f;
					f = spr->d.z; spr->d.z = x0*spr->d.z - y0*spr->r.z; spr->r.z = x0*spr->r.z + y0*f;

					if (doplaysound)
					{
						if (key&0x30000) myplaysound("sounds\\rumble.wav",12,8.0,&spr->p,KSND_3D);
										else myplaysound("sounds\\rumble.wav",25,4.0,&spr->p,KSND_3D);
					}
				}
				break;

			case 0xc9: case 0xd1: //PGUP/PGDN
				if (gps->editmode != 3) break;
				if (((key>>8)&0xff) == 0xc9) d = -.25; else d = +.25;
				f = 1.f;
				if (key&0x30000) { d *= 1.0/16.0; f = 3.f; } //Shift+PGUP/PGDN
				if (gps->grabmode >= 0) { hitsect = gps->grabsect; hitwall = gps->grabwall; if (hitwall < 0) hitwall = gps->grabcf|~1; } else hitsect = -1;
				if ((unsigned)hitsect >= (unsigned)gst->numsects)
				{
					hitscan(&gps->ipos,&gps->ifor,1e32,gps->cursect,&hitsect,&hitwall,&hit);
					if ((unsigned)hitsect >= (unsigned)gst->numsects) break;
				}
				if (hitwall < 0)
				{
					if (doplaysound)
					{
						getcentroid(sec[hitsect].wall,sec[hitsect].n,&hit.x,&hit.y);
						hit.z = getslopez(&sec[hitsect],hitwall&1,hit.x,hit.y);
						myplaysound("sounds\\rumble.wav",(int)(50.0/f),f,&hit,KSND_3D);
					}
					if (sec[hitsect].owner == curindex)
					{
						for(s=gst->numsects-1;s>=0;s--)
						{
							if (sec[s].owner != curindex) continue;
							sec[s].z[hitwall&1] += d;
							for(i=sec[s].headspri;i>=0;i=gst->spri[i].sectn) //FIXFIX
							{
								f = getslopez(&sec[s],hitwall&1,gst->spri[i].p.x,gst->spri[i].p.y);
								if (((!(hitwall&1)) && (gst->spri[i].p.z-gst->spri[i].fat-fabs(d) <= f)) ||
									 (( (hitwall&1)) && (gst->spri[i].p.z+gst->spri[i].fat+fabs(d) >= f)))
									gst->spri[i].p.z += d;
							}
						}
					}
					else
					{
						sec[hitsect].z[hitwall&1] += d;
						for(i=sec[hitsect].headspri;i>=0;i=gst->spri[i].sectn) //FIXFIX
						{
							f = getslopez(&sec[hitsect],hitwall&1,gst->spri[i].p.x,gst->spri[i].p.y);
							if (((!(hitwall&1)) && (gst->spri[i].p.z-gst->spri[i].fat-fabs(d) <= f)) ||
								 (( (hitwall&1)) && (gst->spri[i].p.z+gst->spri[i].fat+fabs(d) >= f)))
								gst->spri[i].p.z += d;
						}
					}
					checknextwalls();
					checksprisect(-1);
					break;
				}
				else if ((unsigned)hitwall < (unsigned)sec[hitsect].n)
				{
					float x2, y2, z0, z1, z2, origz;

					wal = sec[hitsect].wall; k = (gps->grabcf&1);
					if (doplaysound)
					{
						hit.z = getslopez(&sec[hitsect],k,wal[hitwall].x,wal[hitwall].y);
						myplaysound("sounds\\rumble.wav",(int)(50.0/f),f*2,&hit,KSND_3D);
					}

					x0 = wal[hitwall].x; y0 = wal[hitwall].y; origz = getslopez(&sec[hitsect],k,x0,y0);
					for(i=getverts(hitsect,hitwall,verts,MAXVERTS)-1;i>=0;i--)
					{
						hitsect = verts[i].s; hitwall = verts[i].w; wal = sec[hitsect].wall;

																		 x0 = wal[hitwall].x; y0 = wal[hitwall].y; z0 = getslopez(&sec[hitsect],k,x0,y0); if (fabs(z0-origz) >= 1e-4) continue;
						j = wal[hitwall].n+hitwall;          x1 = wal[      j].x; y1 = wal[      j].y; z1 = getslopez(&sec[hitsect],k,x1,y1);
						j = wallprev(&sec[hitsect],hitwall); x2 = wal[      j].x; y2 = wal[      j].y; z2 = getslopez(&sec[hitsect],k,x2,y2);
						x0 = wal[0].x-x0; y0 = wal[0].y-y0; z0 += d;
						x1 = wal[0].x-x1; y1 = wal[0].y-y1;
						x2 = wal[0].x-x2; y2 = wal[0].y-y2;

							////3eq/3unk(ngx,ngy,nz):
							//x0*ngx + y0*ngy + nz = z0
							//x1*ngx + y1*ngy + nz = z1
							//x2*ngx + y2*ngy + nz = z2

						f = x0*(y1-y2) + x1*(y2-y0) + x2*(y0-y1);
						if (f != 0.0)
						{
							f = 1.0/f;
							sec[hitsect].grad[k].x = (z0*(y1   -y2   ) + z1*(y2   -y0   ) + z2*(y0   -y1   ))*f;
							sec[hitsect].grad[k].y = (x0*(z1   -z2   ) + x1*(z2   -z0   ) + x2*(z0   -z1   ))*f;
							sec[hitsect].z[k]      = (x0*(y1*z2-y2*z1) + x1*(y2*z0-y0*z2) + x2*(y0*z1-y1*z0))*f;
						}
					}

					checknextwalls();
					checksprisect(-1);
				}
				else if ((hitwall&0xc0000000) == 0x40000000)
				{
					spr = &gst->spri[hitwall&0x3fffffff];
					if (doplaysound) myplaysound("sounds\\rumble.wav",(int)(50.0/f),f*2.0f,&spr->p,KSND_3D);
					if (key&0xc0000)
					{
						if (((key>>8)&0xff) == 0xc9) //Ctrl+PGUP/PGDN
							  spr->p.z = getslopez(&sec[hitsect],0,spr->p.x,spr->p.y)+spr->fat;
						else spr->p.z = getslopez(&sec[hitsect],1,spr->p.x,spr->p.y)-spr->fat;

						break;
					} else spr->p.z += d;
					shadowtest2_updatelighting = 1;
					break;
				}
#if 0
				if ((unsigned)hitwall < (unsigned)sec[hitsect].n) //WTF does dragging verts have to do with polybool()?
				{
					wall_t twal[4];
					wal = sec[hitsect].wall; i = wal[hitwall].n+hitwall;
					x0 = wal[hitwall].x; x1 = wal[i].x;
					y0 = wal[hitwall].y; y1 = wal[i].y;
					dx = x1-x0; dy = y1-y0; f = .25/sqrt(dx*dx + dy*dy); dx *= f; dy *= f;
					if (d < 0)
					{
						twal[0].x = x1;    twal[0].y = y1;    twal[0].n = 1;
						twal[1].x = x0;    twal[1].y = y0;    twal[1].n = 1;
						twal[2].x = x0+dy; twal[2].y = y0-dx; twal[2].n = 1;
						twal[3].x = x1+dy; twal[3].y = y1-dx; twal[3].n =-3;
						polybool(sec[hitsect].wall,sec[hitsect].n,twal,4,&wal,&i,POLYBOOL_OR);
					}
					else
					{
						twal[0].x = x1-dy; twal[0].y = y1+dx; twal[0].n = 1;
						twal[1].x = x0-dy; twal[1].y = y0+dx; twal[1].n = 1;
						twal[2].x = x0;    twal[2].y = y0;    twal[2].n = 1;
						twal[3].x = x1;    twal[3].y = y1;    twal[3].n =-3;
						polybool(sec[hitsect].wall,sec[hitsect].n,twal,4,&wal,&i,POLYBOOL_SUB);
					}
					if (wal)
					{
						free(sec[hitsect].wall);
						sec[hitsect].wall = wal;
						sec[hitsect].n = sec[hitsect].nmax = i;
						checknextwalls();
						checksprisect(-1);
					}
				}
#endif
				break;
			case 0x46: //Scroll lock (set start position&orientation)
				gst->startpos = gps->ipos;
				gst->startrig = gps->irig;
				gst->startdow = gps->idow;
				gst->startfor = gps->ifor;
				if (doplaysound) myplaysound("sounds\\hammer.wav",100.0,1.f,&gst->startpos,KSND_3D);
				break;
			case 0x29:
				if (key&0xc0000) //Ctrl+'`' Save gamestate (for debug)
				{
					if (gst == &sst)
					{
						ddflip2gdi(); if (mouseacquired) { mouseacquired = 0; setacquire(0,1); }
						cptr = (char *)savefileselect("Save gamestate AS..","*.GST\0*.gst\0All files (*.*)\0*.*\0\0","GST");
						if (cptr) gamestate_save(cptr,gst);
						keystatus[0x1d] = keystatus[0x9d] = 0;
					}
					break;
				}
				break;
		}
	}

	if (!gps->typemode)
	{
		if (gps->bstatus&1) //Drag
		{
			if (gps->grabmode == GRABFILE)
			{
				;
			}
			else if (gps->grabmode == GRABRGB)
			{
				if (colwheel_getcol((gps->fcmousx-gps->ghx)/gps->selrgbrad,(gps->fcmousy-gps->ghy)/gps->selrgbrad,(long *)&r,(long *)&g,(long *)&b))
				{
					if ((gps->grabwall&0xc0000000) == 0x40000000)
					{
						i = (gps->grabwall&0x3fffffff);
						gst->spri[i].asc = 4096;
						gst->spri[i].rsc = max(r,0)*16*gps->selrgbintens;
						gst->spri[i].gsc = max(g,0)*16*gps->selrgbintens;
						gst->spri[i].bsc = max(b,0)*16*gps->selrgbintens;
					}
					else
					{
						if (gps->grabwall < 0) sur = &sec[gps->grabsect].surf[gps->grabcf&1];
												else sur = &sec[gps->grabsect].wall[gps->grabwall].surf;
						sur->asc = 4096;
						sur->rsc = max(r,0)*16*gps->selrgbintens;
						sur->gsc = max(g,0)*16*gps->selrgbintens;
						sur->bsc = max(b,0)*16*gps->selrgbintens;
					}
				}
			}
			else
			{
				getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);

				if (!(gps->obstatus&1))
				{
					gps->grabsect = -1; gps->grabmode = -1;
					if (!gquantstat) //Grab vertex
					{
						if ((gps->editmode == 2) || (gps->showedges3d) || (gps->bstatus&2))
						{
							if ((hitsect >= 0) && ((hitwall&0xc0000000) == 0x40000000))
							{
								spr = &gst->spri[hitwall&0x3fffffff];
								qmousx = spr->p.x;
								qmousy = spr->p.y;

								if (TESTKEY(0x1d)) //Use LCTRL to xor selected point
								{
									if (spr->owner == curindex) spr->owner = -1;
									else if (spr->owner < 0) spr->owner = curindex;
									goto grabskip2end;
								}
								else
								{
									gps->grabsect = 0x7fffffff;
									if (spr->owner == curindex)
									{
										gps->grabmode = GRABDRAG; gps->grabcf = 0; gps->grabwall = gquantwal;
										gps->selcorn.x = qmousx; gps->selcorn.y = qmousy; goto grabskip2end;
									}
									gps->grabmode = GRABDRAG2; gps->grabcf = 0; //Hacks to fool releasegrabdrag() into not exiting immediately
								}

								releasegrabdrag(curindex,doplaysound);
									//check if sprite is already grabbed by another player
								if (spr->owner >= 0) { goto grabskip2end; }
								spr->owner = curindex;
								gps->grabsect = gquantsec;
							}
							else
							{
									//check if vertex is already grabbed by another player
								if (!(TESTKEY(0x2a)|TESTKEY(0x36)))
									  { k = getverts(gquantsec,gquantwal,verts,MAXVERTS); }
								else { verts[0].s = gquantsec; verts[0].w = gquantwal; k = 1; }
								for(j=k-1;j>=0;j--)
									if ((sec[verts[j].s].wall[verts[j].w].owner >= 0) &&
										 (sec[verts[j].s].wall[verts[j].w].owner != curindex))
										{ releasegrabdrag(curindex,doplaysound); goto grabskip2end; }

								if (TESTKEY(0x1d)) //Use LCTRL to xor selected point
								{
									for(j=k-1;j>=0;j--)
									{
										if (sec[verts[j].s].wall[verts[j].w].owner == curindex)
											sec[verts[j].s].wall[verts[j].w].owner = -1;
										else if (sec[verts[j].s].wall[verts[j].w].owner < 0)
											sec[verts[j].s].wall[verts[j].w].owner = curindex;
									}
									goto grabskip2end;
								}
								else
								{
									gps->grabsect = 0x7fffffff;
									for(j=k-1;j>=0;j--) if (sec[verts[j].s].wall[verts[j].w].owner == curindex) break;
									if (j < 0)
									{
										gps->grabmode = GRABDRAG2; gps->grabcf = 0; //Hacks to fool releasegrabdrag() into not exiting immediately
										releasegrabdrag(curindex,doplaysound);
										for(j=k-1;j>=0;j--) sec[verts[j].s].wall[verts[j].w].owner = curindex;
										gps->grabsect = gquantsec;
									}
								}
							}
							if (((unsigned)hitsect < (unsigned)gst->numsects) &&
							  (gst->sect[hitsect].owner == curindex) && (!(TESTKEY(0x1d)|TESTKEY(0x9d)))) { gps->grabmode = GRABDRAG3; } //Grab sectors
							else if (!(TESTKEY(0x2a)|TESTKEY(0x36))) { gps->grabmode = GRABDRAG;  } //Grab all walls on stack
																			else { gps->grabmode = GRABDRAG2; } //Only 1 wall
							gps->grabcf = (gquantcf&1);
							gps->grabwall = gquantwal;
							gps->selcorn.x = qmousx; gps->selcorn.y = qmousy;
						}
						else
						{
							releasegrabdrag(curindex,doplaysound);
							if (!(TESTKEY(0x2a)|TESTKEY(0x36))) gps->grabmode = GRABDRAG;  //Grab all walls on stack
																	 else gps->grabmode = GRABDRAG2; //Only 1 wall
							gps->grabsect = hitsect; gps->grabwall = hitwall; gps->grabcf = (gquantcf&1);
							gps->selcorn.x = qmousx; gps->selcorn.y = qmousy;
						}
					}
					else
					{
						if (gquantstat == 1) //Grab edge
						{
							releasegrabdrag(curindex,doplaysound);
							if (((unsigned)hitsect < (unsigned)gst->numsects) &&
							  (gst->sect[hitsect].owner == curindex) && (!(TESTKEY(0x1d)|TESTKEY(0x9d)))) gps->grabmode = GRABDRAG3; //Grab sectors
							else if (gps->editmode == 3) //Allow LMB holding capture in 3D mode
							{
								if (!(TESTKEY(0x2a)|TESTKEY(0x36))) gps->grabmode = GRABDRAG;  //Grab all walls on stack
								else                                gps->grabmode = GRABDRAG2; //Only 1 wall
							}
							gps->grabsect = gquantsec; gps->grabwall = gquantcf|~1; gps->grabcf = (gquantcf&1);
							gps->selcorn.x = qmousx; gps->selcorn.y = qmousy;
						}
						else if ((gquantstat < 0) && (!gps->sec.n) && ((unsigned)hitsect < (unsigned)gst->numsects)) //Grab sector
						{
							releasegrabdrag(curindex,doplaysound);
							if ((gst->sect[hitsect].owner == curindex) && (!(TESTKEY(0x1d)|TESTKEY(0x9d)))) gps->grabmode = GRABDRAG3; //Grab sectors
							else if (gps->editmode == 3) //Allow LMB holding capture in 3D mode
							{
								if (!(TESTKEY(0x2a)|TESTKEY(0x36))) gps->grabmode = GRABDRAG;  //Grab all walls on stack
								else                                gps->grabmode = GRABDRAG2; //Only 1 wall
							}
							gps->grabsect = hitsect; gps->grabwall = hitwall; gps->grabcf = (gquantcf&1);
							gps->selcorn.x = qmousx; gps->selcorn.y = qmousy;
						}
						else //Grab nothing
						{
							releasegrabdrag(curindex,doplaysound);
							gps->grabmode = -1; gps->grabsect = -1;
						}

						if ((gps->editmode == 2) && (gps->grabmode < 0))
						{
								//Save 1st corner for sector rectangle selection
							if (TESTKEY(0x38)|TESTKEY(0xb8)) //Alt: sector highlight
							{
								releasegrabdrag(curindex,doplaysound);
								curs2grid(gps,gps->fcmousx,gps->fcmousy,&fp); //don't quantize corner
								gps->grabmode = GRABCORNSECT; gps->grabsect = -1;
								gps->selcorn.x = fp.x; gps->selcorn.y = fp.y;
								goto grabskip2end;
							}
								//Save 1st corner for vertex rectangle selection
							if ((gquantstat < 0) || (gquantsec < 0))
							{
								releasegrabdrag(curindex,doplaysound);
								curs2grid(gps,gps->fcmousx,gps->fcmousy,&fp); //don't quantize corner
								gps->grabmode = GRABCORNVERT; gps->grabsect = -1;
								gps->selcorn.x = fp.x; gps->selcorn.y = fp.y;
								goto grabskip2end;
							}
						}
					}
				}

				if ((gps->grabsect >= 0) && (gps->grabmode == GRABDRAG3) && (gps->editmode == 2))
				{
					if ((qmousx != gps->selcorn.x) || (qmousy != gps->selcorn.y))
					{
						for(s=gst->numsects-1;s>=0;s--)
						{
							if (sec[s].owner != curindex) continue;
							shadowtest2_updatelighting = 1;
							wal = gst->sect[s].wall;
							for(w=gst->sect[s].n-1;w>=0;w--)
							{
								sec[s].wall[w].x += qmousx-gps->selcorn.x;
								sec[s].wall[w].y += qmousy-gps->selcorn.y;
							}
							for(w=gst->sect[s].headspri;w>=0;w=spr->sectn)
							{
								spr = &gst->spri[w];
								spr->p.x += qmousx-gps->selcorn.x;
								spr->p.y += qmousy-gps->selcorn.y;
							}
						}

							//Update last mouse pos while dragging
						gps->selcorn.x = qmousx;
						gps->selcorn.y = qmousy;
					}
				}
				if ((gps->grabsect >= 0) && ((gps->grabmode == GRABDRAG) || (gps->grabmode == GRABDRAG2)) &&
					 ((gps->editmode == 2) || (gps->bstatus&2))) //Dragging vertex/sprite in x-y in 3D mode requires RMB pressed also
				{
					if ((qmousx != gps->selcorn.x) || (qmousy != gps->selcorn.y))
					{
						for(s=gst->numsects-1;s>=0;s--)
						{
							wal = gst->sect[s].wall;
							for(w=gst->sect[s].n-1;w>=0;w--)
							{
								if (sec[s].wall[w].owner != curindex) continue;
								shadowtest2_updatelighting = 1;
								sec[s].wall[w].x += qmousx-gps->selcorn.x;
								sec[s].wall[w].y += qmousy-gps->selcorn.y;
							}
							for(w=gst->sect[s].headspri;w>=0;w=spr->sectn)
							{
								spr = &gst->spri[w];
								if (spr->owner != curindex) continue;
								if (spr->flags&(1<<16)) shadowtest2_updatelighting = 1;
								spr->p.x += qmousx-gps->selcorn.x;
								spr->p.y += qmousy-gps->selcorn.y;
							}
						}

							//Must update sprite sectors in separate loop to avoid moving sprite > once!
						for(s=gst->numsects-1;s>=0;s--)
							for(w=gst->sect[s].headspri;w>=0;w=j)
							{
								spr = &gst->spri[w]; j = spr->sectn;
								if (spr->owner != curindex) continue;
								i = spr->sect; updatesect(spr->p.x,spr->p.y,spr->p.z,&i); if (i != spr->sect) changesprisect(w,i);
							}

							//Update last mouse pos while dragging
						gps->selcorn.x = qmousx;
						gps->selcorn.y = qmousy;
					}
				}
grabskip2end:;
			}
		}
		else if ((gps->grabsect >= 0) && ((gps->grabmode == GRABDRAG) || (gps->grabmode == GRABDRAG2) || (gps->grabmode == GRABDRAG3)))
		{
			if (gps->grabsect < gst->numsects) releasegrabdrag(curindex,doplaysound);
			gps->grabmode = -1; gps->grabsect = -1;
		}
		else if (gps->grabmode == GRABCORNVERT)
		{
			curs2grid(gps,gps->fcmousx,gps->fcmousy,&fp);
			for(s=gst->numsects-1;s>=0;s--)
			{
				wal = gst->sect[s].wall;
				for(w=gst->sect[s].n-1;w>=0;w--)
				{
					if ((wal[w].owner >= 0) && (wal[w].owner != curindex)) continue;
					if (((fp.x < wal[w].x) == (wal[w].x < gps->selcorn.x)) &&
						 ((fp.y < wal[w].y) == (wal[w].y < gps->selcorn.y)))
					{
						if ((TESTKEY(0x1d)) && (wal[w].owner == curindex)) //Use LCTRL to xor selected points inside box instead of re-select
							  wal[w].owner = -1;
						else wal[w].owner = curindex;
					}
					else
					{
						if (!(TESTKEY(0x1d))) //Use LCTRL to xor selected points inside box instead of re-select
							wal[w].owner = -1;
					}
				}
				for(w=gst->sect[s].headspri;w>=0;w=gst->spri[w].sectn)
				{
					spr = &gst->spri[w];
					if ((spr->owner >= 0) && (spr->owner != curindex)) continue;
					if (((fp.x < spr->p.x) == (spr->p.x < gps->selcorn.x)) &&
						 ((fp.y < spr->p.y) == (spr->p.y < gps->selcorn.y)))
					{
						if ((TESTKEY(0x1d)) && (spr->owner == curindex)) //Use LCTRL to xor selected points inside box instead of re-select
							  spr->owner = -1;
						else spr->owner = curindex;
					}
					else
					{
						if (!(TESTKEY(0x1d))) //Use LCTRL to xor selected points inside box instead of re-select
							spr->owner = -1;
					}
				}
			}
			gps->selcorn.x = fp.x;
			gps->selcorn.y = fp.y;
			gps->grabmode = -1;
		}
		else if (gps->grabmode == GRABCORNSECT)
		{
			curs2grid(gps,gps->fcmousx,gps->fcmousy,&fp);

			if (fp.x < gps->selcorn.x) { f = fp.x; fp.x = gps->selcorn.x; gps->selcorn.x = f; }
			if (fp.y < gps->selcorn.y) { f = fp.y; fp.y = gps->selcorn.y; gps->selcorn.y = f; }
			for(s=gst->numsects-1;s>=0;s--)
			{
				if ((sec[s].owner >= 0) && (sec[s].owner != curindex)) continue;
				if (isintersect_sect_rect(s,gps->selcorn.x,gps->selcorn.y,fp.x,fp.y))
				{
					if ((TESTKEY(0x1d)) && (sec[s].owner == curindex)) //Use LCTRL to xor selected points inside box instead of re-select
						  sec[s].owner = -1;
					else sec[s].owner = curindex;
				}
				else
				{
					if (!(TESTKEY(0x1d))) //Use LCTRL to xor selected points inside box instead of re-select
						sec[s].owner = -1;
				}
			}
			gps->grabmode = -1;
		}

		if (gps->bstatus&2) //RMB
		{
			if (gps->grabmode == GRABRGB)
			{
				f = 1.0 - fmousy/256.0;
				if ((gps->grabwall&0xc0000000) != 0x40000000)
				{
					if (gps->grabwall < 0) sur = &sec[gps->grabsect].surf[gps->grabcf&1];
											else sur = &sec[gps->grabsect].wall[gps->grabwall].surf;
					i = sur->rsc; sur->rsc = min(max((int)(((float)sur->rsc)*f),0),65535); if ((sur->rsc == i) && (fmousy < 0) && (sur->rsc < 65535)) sur->rsc++;
					i = sur->gsc; sur->gsc = min(max((int)(((float)sur->gsc)*f),0),65535); if ((sur->gsc == i) && (fmousy < 0) && (sur->gsc < 65535)) sur->gsc++;
					i = sur->bsc; sur->bsc = min(max((int)(((float)sur->bsc)*f),0),65535); if ((sur->bsc == i) && (fmousy < 0) && (sur->bsc < 65535)) sur->bsc++;
					gps->selrgbintens = sqrt(((double)sur->rsc)*((double)sur->rsc) +
													 ((double)sur->gsc)*((double)sur->gsc) +
													 ((double)sur->bsc)*((double)sur->bsc))/4096.0;
				}
				else
				{
					spr = &gst->spri[gps->grabwall&0x3fffffff];
					i = spr->rsc; spr->rsc = min(max((int)(((float)spr->rsc)*f),0),65535); if ((spr->rsc == i) && (fmousy < 0) && (spr->rsc < 65535)) spr->rsc++;
					i = spr->gsc; spr->gsc = min(max((int)(((float)spr->gsc)*f),0),65535); if ((spr->gsc == i) && (fmousy < 0) && (spr->gsc < 65535)) spr->gsc++;
					i = spr->bsc; spr->bsc = min(max((int)(((float)spr->bsc)*f),0),65535); if ((spr->bsc == i) && (fmousy < 0) && (spr->bsc < 65535)) spr->bsc++;
					gps->selrgbintens = sqrt(((double)spr->rsc)*((double)spr->rsc) +
													 ((double)spr->gsc)*((double)spr->gsc) +
													 ((double)spr->bsc)*((double)spr->bsc))/4096.0;
				}
			}
		}

		if (TESTKEY(0x24)) //J
		{
			if (gps->grabmode < 0)
			{
				getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
				if ((unsigned)hitsect < (unsigned)gst->numsects)
					{ gps->grabsect = hitsect; gps->grabmode = GRABJOIN; }
			}
		}
		else if ((gps->grabsect >= 0) && (gps->grabmode == GRABJOIN))
		{
			if (!gps->sec.n)
			{
				getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
				if ((unsigned)hitsect < (unsigned)gst->numsects)
					if (polybool(sec[gps->grabsect].wall,sec[gps->grabsect].n,sec[hitsect].wall,sec[hitsect].n,&wal,&i,POLYBOOL_OR))
					{
							//Delete original sector
						if (doplaysound)
						{
							fp.x = qmousx; fp.y = qmousy; fp.z = qmousz;
							myplaysound("sounds\\deleteroom.wav",100,1.0,&fp,KSND_3D);
						}

						for(j=0;j<2;j++)
							sec[gps->grabsect].z[j] = getslopez(&sec[gps->grabsect],j,wal[0].x,wal[0].y);

						free(sec[gps->grabsect].wall);
						sec[gps->grabsect].n = sec[gps->grabsect].nmax = i;
						sec[gps->grabsect].wall = wal;
						if (gps->grabsect != hitsect) delsect(hitsect);
						checknextwalls();
						checksprisect(-1);
					}
			}
			gps->grabmode = -1;
		}

		if (TESTKEY(0xcf)) //End (copy plane orientation: ceil/flor height and slope)
		{
			if (gps->grabmode < 0)
			{
				getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
				if (((unsigned)hitsect < (unsigned)gst->numsects) && (hitwall < 0))
					{ gps->grabsect = hitsect; gps->grabmode = GRABEND; gps->grabcf = (hitwall&1); }
			}
		}
		else if ((gps->grabsect >= 0) && (gps->grabmode == GRABEND))
		{
			if (!gps->sec.n)
			{
				getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
				if (((unsigned)hitsect < (unsigned)gst->numsects) && (hitwall < 0))
				{
					if (doplaysound) myplaysound("sounds\\rumble.wav",50,2.f,&hit,KSND_3D);

					wal = sec[hitsect].wall;
					sec[hitsect].z[hitwall&1] = getslopez(&sec[gps->grabsect],gps->grabcf&1,wal[0].x,wal[0].y);
					sec[hitsect].grad[hitwall&1].x = sec[gps->grabsect].grad[gps->grabcf&1].x;
					sec[hitsect].grad[hitwall&1].y = sec[gps->grabsect].grad[gps->grabcf&1].y;

					checknextwalls();
					checksprisect(-1);
				}
			}
			gps->grabmode = -1;
		}


		if (TESTKEY(0xc7)) //Home (align slope to next sector)
		{
			if (gps->grabmode < 0)
			{
				getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
				if (((unsigned)hitsect < (unsigned)gst->numsects) && (hitwall < 0))
					{ gps->grabsect = hitsect; gps->grabmode = GRABHOME; gps->grabcf = (hitwall&1); }
			}
		}
		else if ((gps->grabsect >= 0) && (gps->grabmode == GRABHOME))
		{
			if (!gps->sec.n)
			{
				getcurspoint(gps,&qmousx,&qmousy,&qmousz,&hitsect,&hitwall,&hit);
				if (((unsigned)hitsect < (unsigned)gst->numsects) && (hitwall < 0))
				{
					if (doplaysound) myplaysound("sounds\\rumble.wav",50,2.f,&hit,KSND_3D);

					wal = sec[hitsect].wall; mind = 1e32; bw = 0;
					for(w=0;w<sec[hitsect].n;w++)
					{
						d = (hit.x-wal[w].x)*(hit.x-wal[w].x) + (hit.y-wal[w].y)*(hit.y-wal[w].y);
						if (d < mind) { mind = d; bw = w; }
					}
					if (bw >= 2)
					{
						dx = wal[bw].x; dy = wal[bw].y;
						d = getslopez(&sec[gps->grabsect],gps->grabcf&1,dx,dy);

							//Must be true: (wal[1].x-wal[0].x)*x + (wal[1].y-wal[0].y)*y = 0

							//d = (wal[0].x-x)*(sec[hitsect].grad[hitwall&1].x + (wal[1].y-wal[0].y)*f) +
							//    (wal[0].y-y)*(sec[hitsect].grad[hitwall&1].y + (wal[0].x-wal[1].x)*f) +
							//    sec[hitsect].z[hitwall&1]

							//d - sec[hitsect].z[hitwall&1]
							// =
							//+ (wal[0].x-x)*(sec[hitsect].grad[hitwall&1].x + (wal[1].y-wal[0].y)*f)
							//+ (wal[0].y-y)*(sec[hitsect].grad[hitwall&1].y + (wal[0].x-wal[1].x)*f)

						f = (d - sec[hitsect].z[hitwall&1]
								 - (wal[0].x-dx)*(sec[hitsect].grad[hitwall&1].x)
								 - (wal[0].y-dy)*(sec[hitsect].grad[hitwall&1].y))
								 / ((wal[0].x-dx)*(wal[1].y-wal[0].y) + (wal[0].y-dy)*(wal[0].x-wal[1].x));
						sec[hitsect].grad[hitwall&1].x += (wal[1].y-wal[0].y)*f;
						sec[hitsect].grad[hitwall&1].y += (wal[0].x-wal[1].x)*f;

						checknextwalls();
						checksprisect(-1);
					}
				}
			}
			gps->grabmode = -1;
		}

		updatesect(gps->ipos.x,gps->ipos.y,gps->ipos.z,&gps->cursect);
	}
}

	//avatar sign cache
#define AVATARSIGNX 128
#define AVATARSIGNY 16
static long avatarsign[MAXPLAYERS][AVATARSIGNX][AVATARSIGNY]; //8KB per player (for 128x16) :/
static char avatarname[MAXPLAYERS][TYPEMESSLENG] = {0};

#define TAGSIGNX 128
#define TAGSIGNY 8
#define TAGSIZE .006f
static long tagsign[TAGSIGNX*(TAGSIGNY+1)+1];
static tile_t tagtil;
static long gtagnum = 0, gtagleng = 0;
static long drawtagtexture (unsigned long tagVal, char pal = 0)
{
	long x, y;
	char tbuf[64];
	unsigned short lotag, hitag;

	// Extract components
	lotag = tagVal & 0xFFFF;
	hitag = (tagVal >> 16) & 0xFFFF;

	if (gtagnum == tagVal) return(gtagleng);

	//Draw tag sign
	tagtil.tt.f = (long)tagsign;
	tagtil.tt.x = TAGSIGNX;
	tagtil.tt.y = TAGSIGNY;
	tagtil.tt.p = (tagtil.tt.x<<2);
	sprintf(tbuf,"%d,%d,%d", lotag, hitag, pal);
	gtagnum = tagVal; gtagleng = strlen(tbuf)*6;
	print6x8((tiltyp *)&tagtil.tt,0,0,0xffffff,0,"%s",tbuf);
	for(x=0;x<gtagleng;x++) if (!tagsign[TAGSIGNX*6+x]) tagsign[TAGSIGNX*6+x] += 0x008080; //Draw underline
	fixtex4grou((tiltyp *)&tagtil.tt);
	return(gtagleng);
}


static void drawtag_sect (cam_t *cc, long s, long isflor)
{
	kgln_t rpol[4];
	sect_t *sec;
	point3d fp;
	float f, g;
	long j;

	sec = gst->sect;
	j = drawtagtexture(sec[s].surf[isflor].tag, sec[s].surf[isflor].pal);
	f = ((float)j)*TAGSIZE;
	g = TAGSIZE*8; if (!isflor) g = -g;
	getcentroid(sec[s].wall,sec[s].n,&fp.x,&fp.y);
	rpol[0].x = fp.x-f; rpol[0].y = fp.y-g;
	rpol[1].x = fp.x+f; rpol[1].y = fp.y-g;
	rpol[2].x = fp.x+f; rpol[2].y = fp.y+g;
	rpol[3].x = fp.x-f; rpol[3].y = fp.y+g;
	rpol[0].u = .5/((float)TAGSIGNX);              rpol[0].v = -.0625; rpol[0].n = 1;
	rpol[1].u = (((float)j)+.5)/((float)TAGSIGNX); rpol[1].v = -.0625; rpol[1].n = 1;
	rpol[2].u = rpol[1].u;                         rpol[2].v = 0.9375; rpol[2].n = 1;
	rpol[3].u = rpol[0].u;                         rpol[3].v = 0.9375; rpol[3].n =-3;
	for(j=0;j<4;j++)
	{
		rpol[j].z = getslopez(&sec[s],isflor,rpol[j].x,rpol[j].y);
		if (!isflor) rpol[j].z += .001f; else rpol[j].z -= .001f;
		xformpos(&rpol[j].x,&rpol[j].y,&rpol[j].z);
	}
	j = 0; if (gdps->compact2d >= 1.0) j |= RENDFLAGS_NODEPTHTEST;
	drawpol(cc,rpol,4,&tagtil,0x808080,0,0,j);
}

static void drawtag_wall (cam_t *cc, long s, long w, float dx, kgln_t *pol)
{
	kgln_t rpol[4];
	wall_t *wal;
	point3d fp, fp2;
	float f;
	long j;

	wal = gst->sect[s].wall;
	fp.x = (pol[0].x+pol[1].x)*.5;
	fp.y = (pol[0].y+pol[1].y)*.5;
	fp.z = (pol[0].z+pol[1].z+pol[2].z+pol[3].z)*.25;
	fp2.x = pol[1].x-pol[0].x;
	fp2.y = pol[1].y-pol[0].y;
	fp2.z = pol[1].z-pol[0].z;
	j = drawtagtexture(wal[w].surf.tag, wal[w].surf.pal);
	f = 1.0/dx;
	fp.x -= fp2.y*f*.001;
	fp.y += fp2.x*f*.001;
	f *= ((float)j)*TAGSIZE;
	rpol[0].x = fp.x-fp2.x*f; rpol[0].y = fp.y-fp2.y*f; rpol[0].z = fp.z-TAGSIZE*8;
	rpol[1].x = fp.x+fp2.x*f; rpol[1].y = fp.y+fp2.y*f; rpol[1].z = fp.z-TAGSIZE*8;
	rpol[2].x = fp.x+fp2.x*f; rpol[2].y = fp.y+fp2.y*f; rpol[2].z = fp.z+TAGSIZE*8;
	rpol[3].x = fp.x-fp2.x*f; rpol[3].y = fp.y-fp2.y*f; rpol[3].z = fp.z+TAGSIZE*8;
	rpol[0].u = .5/((float)TAGSIGNX);              rpol[0].v = -.0625; rpol[0].n = 1;
	rpol[1].u = (((float)j)+.5)/((float)TAGSIGNX); rpol[1].v = -.0625; rpol[1].n = 1;
	rpol[2].u = rpol[1].u;                         rpol[2].v = 0.9375; rpol[2].n = 1;
	rpol[3].u = rpol[0].u;                         rpol[3].v = 0.9375; rpol[3].n =-3;
	for(j=0;j<4;j++) xformpos(&rpol[j].x,&rpol[j].y,&rpol[j].z);
	drawpol(cc,rpol,4,&tagtil,0x808080,0,0,0);
}
static void drawtag_spri (cam_t *cc, long s)
{
	kgln_t pol[4];
	spri_t *spr;
	point3d fp;
	float f;
	long j;

	spr = &gst->spri[s];

	fp.x = gdps->ipos.x-spr->p.x; fp.y = gdps->ipos.y-spr->p.y; fp.z = gdps->ipos.z-spr->p.z;
	f = fp.x*fp.x + fp.y*fp.y + fp.z*fp.z; if (f <= 0.f) return;

	f = (spr->fat+TAGSIZE*8)/sqrt(f);
	j = drawtagtexture(spr->tag, spr->pal);
	pol[3].x = fp.x*f + spr->p.x;
	pol[3].y = fp.y*f + spr->p.y;
	pol[3].z = fp.z*f + spr->p.z; xformpos(&pol[3].x,&pol[3].y,&pol[3].z);
	f = ((float)j)*TAGSIZE;
	pol[0].x = pol[3].x - f; pol[0].y = pol[3].y - TAGSIZE*8; pol[0].z = pol[3].z;
	pol[1].x = pol[3].x + f; pol[1].y = pol[0].y;             pol[1].z = pol[3].z;
	pol[2].x = pol[1].x;     pol[2].y = pol[3].y + TAGSIZE*8; pol[2].z = pol[3].z;
	pol[3].x = pol[0].x;     pol[3].y = pol[2].y;             pol[3].z = pol[3].z;
	pol[0].u = .5/((float)TAGSIGNX);              pol[0].v = -.0625; pol[0].n = 1;
	pol[1].u = (((float)j)+.5)/((float)TAGSIGNX); pol[1].v = -.0625; pol[1].n = 1;
	pol[2].u = pol[1].u;                          pol[2].v = 0.9375; pol[2].n = 1;
	pol[3].u = pol[0].u;                          pol[3].v = 0.9375; pol[3].n =-3;
	drawpol(cc,pol,4,&tagtil,0x808080,0,0,0);
}

static void drawhinge (long s, long w, long isflor)
{
	kgln_t rpol[4];
	point3d fp, fp2;
	sect_t *sec;
	wall_t *wal;
	float f, g, dx;
	long j, nw;

	sec = gst->sect; wal = sec[s].wall;
	nw = wal[w].n+w;
	dx = sqrt((wal[nw].x-wal[w].x)*(wal[nw].x-wal[w].x) + (wal[nw].y-wal[w].y)*(wal[nw].y-wal[w].y));

	fp.x = (wal[w].x+wal[nw].x)*.5; fp2.x = wal[nw].x-wal[w].x;
	fp.y = (wal[w].y+wal[nw].y)*.5; fp2.y = wal[nw].y-wal[w].y;
	fp.z = getslopez(&sec[s],isflor,fp.x,fp.y);
	f = 1.0/dx;
	fp.x -= fp2.y*f*.0005;
	fp.y += fp2.x*f*.0005;
	f *= .2; if (!isflor) g = -1; else g = 1;
	rpol[0].x = fp.x-fp2.x*f*g; rpol[0].y = fp.y-fp2.y*f*g; rpol[0].z = fp.z-g*.2;
	rpol[1].x = fp.x+fp2.x*f*g; rpol[1].y = fp.y+fp2.y*f*g; rpol[1].z = fp.z-g*.2;
	rpol[2].x = fp.x+fp2.x*f*g; rpol[2].y = fp.y+fp2.y*f*g; rpol[2].z = fp.z;
	rpol[3].x = fp.x-fp2.x*f*g; rpol[3].y = fp.y-fp2.y*f*g; rpol[3].z = fp.z;
	rpol[0].u = 0; rpol[0].v = .02; rpol[0].n = 1;
	rpol[1].u = 1; rpol[1].v = .02; rpol[1].n = 2;
	rpol[2].u = 1; rpol[2].v = .98; rpol[2].n = 3;
	rpol[3].u = 0; rpol[3].v = .98; rpol[3].n = 0;
	for(j=0;j<4;j++)
	{
		xformpos(&rpol[j].x,&rpol[j].y,&rpol[j].z);
#if (USEINTZ)
		f = 256.f; rpol[j].x *= f; rpol[j].y *= f; rpol[j].z *= f;
#endif
	}
	if (gdps->rendinterp) j = RENDFLAGS_INTERP; else j = 0;
	drawpoly(&tilelist[0].t,(vertyp *)rpol,4,0xc0c0c0,16.0,0,j|RENDFLAGS_ALPHAMASK);

	fp.x = (wal[w].x+wal[nw].x)*.5; fp2.x = wal[nw].x-wal[w].x;
	fp.y = (wal[w].y+wal[nw].y)*.5; fp2.y = wal[nw].y-wal[w].y;
	fp.z = getslopez(&sec[s],isflor,fp.x,fp.y);
	f = .2/dx; if (!isflor) g = -1; else g = 1;
	rpol[0].x = fp.x - fp2.x*f*g;           rpol[0].y = fp.y - fp2.y*f*g;
	rpol[1].x = fp.x + fp2.x*f*g;           rpol[1].y = fp.y + fp2.y*f*g;
	rpol[2].x = fp.x + fp2.x*f*g - fp2.y*f; rpol[2].y = fp.y + fp2.y*f*g + fp2.x*f;
	rpol[3].x = fp.x - fp2.x*f*g - fp2.y*f; rpol[3].y = fp.y - fp2.y*f*g + fp2.x*f;
	rpol[0].u = 1; rpol[0].v = .98; rpol[0].n = 1;
	rpol[1].u = 0; rpol[1].v = .98; rpol[1].n = 2;
	rpol[2].u = 0; rpol[2].v = .02; rpol[2].n = 3;
	rpol[3].u = 1; rpol[3].v = .02; rpol[3].n = 0;
	for(j=0;j<4;j++)
	{
		rpol[j].z = getslopez(&sec[s],isflor,rpol[j].x,rpol[j].y); if (isflor) rpol[j].z -= .0005; else rpol[j].z += .0005;
		xformpos(&rpol[j].x,&rpol[j].y,&rpol[j].z);
#if (USEINTZ)
		f = 256.f; rpol[j].x *= f; rpol[j].y *= f; rpol[j].z *= f;
#endif
	}
	if (gdps->rendinterp) j = RENDFLAGS_INTERP; else j = 0;
	drawpoly(&tilelist[0].t,(vertyp *)rpol,4,0xc0c0c0,16.0,0,j|RENDFLAGS_ALPHAMASK);
}

#endif

void drawsprite (cam_t *cc, spri_t *spr)
{
	kgln_t pol[4], tpol;
	point3d fp, fp2, norm;
	float f, g;
	long i, j, col;
	char *cptr;

	col = (min(spr->asc>>8,255)<<24) +
			(min(spr->rsc>>5,255)<<16) +
			(min(spr->gsc>>5,255)<< 8) +
			(min(spr->bsc>>5,255)    );
	if ((unsigned)spr->tilnum >= (unsigned)gnumtiles)
	{
		drawsph(cc,spr->p.x,spr->p.y,spr->p.z,spr->fat,col);
	}
	else
	{
		cptr = gtile[spr->tilnum].filnam;
		i = strlen(cptr)-4;
		if ((i > 0) && (!stricmp(&cptr[i],".KV6")))
		{
			drawkv6(cc,cptr,
				spr->p.x,spr->p.y,spr->p.z,
				spr->r.x,spr->r.y,spr->r.z,
				spr->d.x,spr->d.y,spr->d.z,
				spr->f.x,spr->f.y,spr->f.z,col,48.0);
		}
		else
		{
				//Draw flat sprite
			switch(spr->flags&48)
			{
				case 0: case 48: fp = spr->r; fp2 = spr->d; break; //Wall/floor sprites
				case 16: //Face sprites
					f = sqrt(spr->r.x*spr->r.x + spr->r.y*spr->r.y + spr->r.z*spr->r.z);
					fp.x = gdps->irig.x*f;
					fp.y = gdps->irig.y*f;
					fp.z = gdps->irig.z*f;
					fp2 = spr->d;
					break;
				case 32:
					f = sqrt(spr->r.x*spr->r.x + spr->r.y*spr->r.y + spr->r.z*spr->r.z);
					fp.x = gdps->irig.x*f;
					fp.y = gdps->irig.y*f;
					fp.z = gdps->irig.z*f;
					f = sqrt(spr->d.x*spr->d.x + spr->d.y*spr->d.y + spr->d.z*spr->d.z);
					fp2.x = gdps->idow.x*f;
					fp2.y = gdps->idow.y*f;
					fp2.z = gdps->idow.z*f;
					break;
			}
			for(j=4-1;j>=0;j--)
			{
				if ((j+1)&2) f = 1; else f = -1;
				if ((j  )&2) g = 1; else g = -1;
				pol[j].x = fp.x*f + fp2.x*g + spr->p.x;
				pol[j].y = fp.y*f + fp2.y*g + spr->p.y;
				pol[j].z = fp.z*f + fp2.z*g + spr->p.z;
				pol[j].u = (f >= 0); pol[j].v = (g >= 0); pol[j].n = ((j+1)&3)-j;
				xformpos(&pol[j].x,&pol[j].y,&pol[j].z);
					//Fix Z-fighting
				f = .99; pol[j].x *= f; pol[j].y *= f; pol[j].z *= f; pol[j].u *= f; pol[j].v *= f;
			}
			j = RENDFLAGS_RALPHAMASK;
			if (!(spr->flags&64)) j |= RENDFLAGS_CULLNONE;
			else if (spr->flags&4) { tpol = pol[0]; pol[0] = pol[2]; pol[2] = tpol; } //Mirrored: use other side

			norm.x = fp2.y*fp.z - fp2.z*fp.y;
			norm.y = fp2.z*fp.x - fp2.x*fp.z;
			norm.z = fp2.x*fp.y - fp2.y*fp.x;
			f = 1.0/sqrt(norm.x*norm.x + norm.y*norm.y + norm.z*norm.z); norm.x *= f; norm.y *= f; norm.z *= f;
			drawpolfunc(cc,pol,4,&gtile[spr->tilnum],col,0,&norm,j);
		}
	}
}

void drawview (cam_t *cc, playerstruct_t *lps, int skipdrawrooms)
{
	playerstruct_t *pps;
	kgln_t pol[4], rpol[4], npol[4], tpol;
	#define MAXVERTS 256 //FIX:timebomb: assumes there are never > 256 sectors connected at same vertex
	vertlist_t verts[MAXVERTS], tvert;
	tile_t til;
	tiletype tt;
	sect_t *sec;
	wall_t *wal, *wal2;
	spri_t *spr;
	point3d fp, fp2, hit, norm;
	float f, g, dx, dy, fx, fy, lefz[2], rigz[2];
	unsigned int *gotsect;
	int i, j, k, p, s, w, nw, wn, bs, bw, s0, s1, cf0, cf1, vn, col[2], didpolybool = 0;
	int locursect, *secfif, secfifw, secfifr, hitsect, hitwall;
	char *cptr;
#ifdef STANDALONE
	char tbuf[TYPEMESSLENG];
	int showedges3d, flashcol;
#endif

	sec = gst->sect;
	gdps = lps;

	locursect = gdps->cursect;
	updatesect(gdps->ipos.x,gdps->ipos.y,gdps->ipos.z,&locursect);

	if (skipdrawrooms)
	{
		gotsect = shadowtest2_sectgot;

#ifdef STANDALONE
		if (fabs(gdps->grdn.z) == 1.f) drawgrid(cc,0);

		if ((gdps->editmode == 2) && (gst->numsects == 2) && (!gdps->sec.n) && (gdps->boolfunc != POLYBOOL_END))
		{
			if (sec[1].n == 2)
			{
				i = sec[1].wall[0].n;
				fx = sec[1].wall[i].y-sec[1].wall[0].y;
				fy = sec[1].wall[0].x-sec[1].wall[i].x;
				sec[2].n = polyspli(sec[0].wall,sec[0].n,&sec[2].wall,fx,fy,-sec[1].wall[0].x*fx - sec[1].wall[0].y*fy);
			}
			else
			{
				polybool(sec[0].wall,sec[0].n,sec[1].wall,sec[1].n,&sec[2].wall,(int *)&sec[2].n,gdps->boolfunc);
			}
			if (sec[2].wall)
			{
				didpolybool = 1;
				if (gdps->ifor.z >= 0) //View solid color (result sector) when viewing from ceiling
				{
					j = (gdps->ifor.z >= 0);
					sec[2].grad[j].x = sec[2].grad[j].y = 0.f;
					sec[2].surf[j].tilnum = 0;
					sec[2].surf[j].uv[0].x = 0; sec[2].surf[j].uv[0].y = 0;
					sec[2].surf[j].uv[1].x = 1; sec[2].surf[j].uv[1].y = 0;
					sec[2].surf[j].uv[2].x = 0; sec[2].surf[j].uv[2].y = 1;
					drawsectfill3d(cc,&sec[2],j,(0x800080&0xfefefe)>>1);
				}
			}
		}

		getcurspoint(gdps,&fp.x,&fp.y,&fp.z,&hitsect,&hitwall,&hit);

		showedges3d = (((unsigned)locursect < (unsigned)gst->numsects) && (gps->showedges3d));
		if ((!showedges3d) || (!gotsect)) goto skipdrawrooms_lab;
		for(s=uptil1(gotsect,gst->numsects)-1;s>=0;s=uptil1(gotsect,s)-1)
		{
			if (((!gdps->showdebug) || (gdps->editmode != 2)) && (!didpolybool))
			{
				for(i=0;i<2;i++)
				{
					if (gdps->compact2d < 1.0)
					{     //3D back-face cull
						if ((gdps->ipos.z >= getslopez(&sec[s],i,gdps->ipos.x,gdps->ipos.y)) == i) continue;
					}
					else
					{     //2D back-face cull
						if ((gdps->grdn.z >= 0) != i) continue;
					}
					if (sec[s].surf[i].tag) drawtag_sect(cc,s,i);
				}
			}

				//draw walls
			if ((gdps->compact2d != 1.0) || (fabs(gdps->grdn.z) != 1.0))
			{
				wal = sec[s].wall; wn = sec[s].n;
				for(w=0;w<wn;w++)
				{
					nw = wal[w].n+w;

					dx = sqrt((wal[nw].x-wal[w].x)*(wal[nw].x-wal[w].x) + (wal[nw].y-wal[w].y)*(wal[nw].y-wal[w].y));

					for(i=0;i<2;i++) drawsph(cc,wal[w].x,wal[w].y,getslopez(&sec[s],i,wal[w].x,wal[w].y),.025,0x409040);
					if ((!w) && (hitsect == s))
					{
						if (hitwall < 0) drawhinge(s,w,hitwall&1);
						else drawhinge(s,w,fp.z*2 > getslopez(&sec[s],0,fp.x,fp.y)+getslopez(&sec[s],1,fp.x,fp.y));
					}

						//Back-face cull
					if ((gdps->ipos.x-wal[w].x)*(wal[nw].y-wal[w].y) >
						 (gdps->ipos.y-wal[w].y)*(wal[nw].x-wal[w].x)) continue;

					pol[0].x = wal[ w].x; pol[0].y = wal[ w].y; pol[0].n = 1;
					pol[1].x = wal[nw].x; pol[1].y = wal[nw].y; pol[1].n = 1;
					pol[2].x = wal[nw].x; pol[2].y = wal[nw].y; pol[2].n = 1;
					pol[3].x = wal[ w].x; pol[3].y = wal[ w].y; pol[3].n =-3;
					vn = getwalls(s,w,verts,MAXVERTS);

					lefz[0] = getslopez(&sec[s],0,pol[0].x,pol[0].y);
					rigz[0] = getslopez(&sec[s],0,pol[1].x,pol[1].y);
					rigz[1] = getslopez(&sec[s],1,pol[2].x,pol[2].y);
					lefz[1] = getslopez(&sec[s],1,pol[3].x,pol[3].y);

					for(k=0;k<=vn;k++) //Warning: do not reverse for loop!
					{
						if (k > 0)
						{
							s0 = verts[k-1].s;
							pol[0].z = getslopez(&sec[s0],1,pol[0].x,pol[0].y);
							pol[1].z = getslopez(&sec[s0],1,pol[1].x,pol[1].y);
						} else { pol[0].z = lefz[0]; pol[1].z = rigz[0]; }
						if (wal[w].surf.tag) drawtag_wall(cc,s,w,dx,pol);
					}
					if ((wal[w].ns >= 0) && ((gdps->ipos.x-wal[w].x)*(wal[nw].y-wal[w].y) < (gdps->ipos.y-wal[w].y)*(wal[nw].x-wal[w].x)))
						for(i=0;i<2;i++)
						{
							pol[0].z = getslopez(&sec[s],i,pol[0].x,pol[0].y);
							pol[1].z = getslopez(&sec[s],i,pol[1].x,pol[1].y);
							for(k=0;k<vn;k++) //Warning: do not reverse for loop!
							{
								pol[2].z = getslopez(&sec[verts[k].s],i,pol[0].x,pol[0].y);
								pol[3].z = getslopez(&sec[verts[k].s],i,pol[1].x,pol[1].y);
								if (fabs(pol[2].z-pol[0].z) + fabs(pol[1].z-pol[3].z) < .03125)
								{
									drawcone(cc,wal[ w].x,wal[ w].y,getslopez(&sec[s],i,wal[ w].x,wal[ w].y),-.0125,
													wal[nw].x,wal[nw].y,getslopez(&sec[s],i,wal[nw].x,wal[nw].y),-.0125,0x904040);
									break;
								}
							}
						}
				}
			}
		}
#endif
		goto skipdrawrooms_lab;
	}

	w = (((gst->numsects+31)>>5)<<2);
	gotsect = (unsigned int *)_alloca(w);
	secfif = (int *)_alloca(gst->numsects*sizeof(secfif[0]));
	secfifr = 0;
	if (((unsigned)locursect < (unsigned)gst->numsects) && (gdps->editmode == 3))
	{
		secfif[0] = locursect; secfifw = 1;
		memset(gotsect,0,w);
		gotsect[locursect>>5] |= (1<<locursect);
	}
	else
	{
		//clearscreen(0); //FIX: can't allow winmain's clearscreen() inside threads!
		for(i=0,j=cc->c.f;i<cc->c.y;i++,j+=cc->c.p) memset8((void *)j,0,cc->c.x<<2);

		for(s=gst->numsects-1;s>=0;s--) secfif[s] = s;
		secfifw = gst->numsects;
		memset(gotsect,-1,w);
	}

#ifdef STANDALONE
	if (fabs(gdps->grdn.z) == 1.f) drawgrid(cc,0);

	if ((gdps->editmode == 2) && (gst->numsects == 2) && (!gdps->sec.n) && (gdps->boolfunc != POLYBOOL_END))
	{
		if (sec[1].n == 2)
		{
			i = sec[1].wall[0].n;
			fx = sec[1].wall[i].y-sec[1].wall[0].y;
			fy = sec[1].wall[0].x-sec[1].wall[i].x;
			sec[2].n = polyspli(sec[0].wall,sec[0].n,&sec[2].wall,fx,fy,-sec[1].wall[0].x*fx - sec[1].wall[0].y*fy);
			//sec[2].n = polyspli(sec[0].wall,sec[0].n,&sec[2].wall,sec[1].wall[0].x,sec[1].wall[0].y,
			//   sec[1].wall[i].y-sec[1].wall[0].y,sec[1].wall[0].x-sec[1].wall[i].x);
		}
		else
		{
			polybool(sec[0].wall,sec[0].n,sec[1].wall,sec[1].n,&sec[2].wall,(int *)&sec[2].n,gdps->boolfunc);
		}

		if (sec[2].wall)
		{
			didpolybool = 1;
			if (gdps->ifor.z >= 0) //View solid color (result sector) when viewing from ceiling
			{
				j = (gdps->ifor.z >= 0);
				sec[2].grad[j].x = sec[2].grad[j].y = 0.f;
				sec[2].surf[j].tilnum = 0;
				sec[2].surf[j].uv[0].x = 0; sec[2].surf[j].uv[0].y = 0;
				sec[2].surf[j].uv[1].x = 1; sec[2].surf[j].uv[1].y = 0;
				sec[2].surf[j].uv[2].x = 0; sec[2].surf[j].uv[2].y = 1;
				drawsectfill3d(cc,&sec[2],j,(0x800080&0xfefefe)>>1);
			}
		}
	}

	showedges3d = (((unsigned)locursect < (unsigned)gst->numsects) && (gps->showedges3d));

	getcurspoint(gdps,&fp.x,&fp.y,&fp.z,&hitsect,&hitwall,&hit);

	flashcol = argb_interp(0x80ffff,0x000000,((int)(sin(dtotclk*16.f)*8191.f))+8192);
#endif

	while (secfifr < secfifw)
	{
		s = secfif[secfifr]; secfifr++;

#ifdef STANDALONE
		if (((!gdps->showdebug) || (gdps->editmode != 2)) && (!didpolybool))
#else
		if (!didpolybool)
#endif
		{
				//draw sector filled

			//col[0] = 0x808080;
			//if ((insidesect(qmousx,qmousy,&sec[s])) && (fmod(dtotclk,.25) < .125)) col[0] ^= 0x202020;

			for(i=0;i<2;i++)
			{
				if (gdps->compact2d < 1.0)
				{     //3D back-face cull
					if ((gdps->ipos.z >= getslopez(&sec[s],i,gdps->ipos.x,gdps->ipos.y)) == i) continue;
				}
				else
				{     //2D back-face cull
					if ((gdps->grdn.z >= 0) != i) continue;
				}

				col[0] = (min(sec[s].surf[i].asc>>8,255)<<24) +
							(min(sec[s].surf[i].rsc>>5,255)<<16) +
							(min(sec[s].surf[i].gsc>>5,255)<< 8) +
							(min(sec[s].surf[i].bsc>>5,255)    );
				col[0] = argb_interp(col[0],(col[0]&0xff000000)+((col[0]&0xfcfcfc)>>2),(long)(gdps->compact2d*24576.0));
#ifdef STANDALONE
				if (sec[s].owner >= 0) { if (sec[s].owner == gdps->playerindex) col[0] = flashcol; else col[0] = ((flashcol&0xfefefe)>>1); }
#endif
				drawsectfill3d(cc,&sec[s],i,col[0]);

#ifdef STANDALONE
				if ((showedges3d) && (sec[s].surf[i].tag)) drawtag_sect(cc,s,i);
#endif
			}
		}

			//draw walls
		if ((gdps->compact2d != 1.0) || (fabs(gdps->grdn.z) != 1.0))
		{
			wal = sec[s].wall; wn = sec[s].n;
			for(w=0;w<wn;w++)
			{
				nw = wal[w].n+w;

				dx = sqrt((wal[nw].x-wal[w].x)*(wal[nw].x-wal[w].x) + (wal[nw].y-wal[w].y)*(wal[nw].y-wal[w].y));

#ifdef STANDALONE
				if (showedges3d)
				{
					for(i=0;i<2;i++) drawsph(cc,wal[w].x,wal[w].y,getslopez(&sec[s],i,wal[w].x,wal[w].y),.025,0x409040);
					if ((!w) && (hitsect == s))
					{
						if (hitwall < 0) drawhinge(s,w,hitwall&1);
						else drawhinge(s,w,fp.z*2 > getslopez(&sec[s],0,fp.x,fp.y)+getslopez(&sec[s],1,fp.x,fp.y));
					}
				}
#endif

					//Back-face cull
				if ((gdps->ipos.x-wal[w].x)*(wal[nw].y-wal[w].y) >
					 (gdps->ipos.y-wal[w].y)*(wal[nw].x-wal[w].x)) continue;

				col[0] = (min(wal[w].surf.asc>>8,255)<<24) +
							(min(wal[w].surf.rsc>>5,255)<<16) +
							(min(wal[w].surf.gsc>>5,255)<< 8) +
							(min(wal[w].surf.bsc>>5,255)    );

				pol[0].x = wal[ w].x; pol[0].y = wal[ w].y; pol[0].n = 1;
				pol[1].x = wal[nw].x; pol[1].y = wal[nw].y; pol[1].n = 1;
				pol[2].x = wal[nw].x; pol[2].y = wal[nw].y; pol[2].n = 1;
				pol[3].x = wal[ w].x; pol[3].y = wal[ w].y; pol[3].n =-3;
				vn = getwalls(s,w,verts,MAXVERTS);

				lefz[0] = getslopez(&sec[s],0,pol[0].x,pol[0].y);
				rigz[0] = getslopez(&sec[s],0,pol[1].x,pol[1].y);
				rigz[1] = getslopez(&sec[s],1,pol[2].x,pol[2].y);
				lefz[1] = getslopez(&sec[s],1,pol[3].x,pol[3].y);

				norm.x = wal[w].y-wal[nw].y;
				norm.y = wal[nw].x-wal[w].x;
				norm.z = 0;
				f = 1.0/sqrt(norm.x*norm.x + norm.y*norm.y); norm.x *= f; norm.y *= f;

				for(k=0;k<=vn;k++) //Warning: do not reverse for loop!
				{
					if (k > 0)
					{
						s0 = verts[k-1].s;
						pol[0].z = getslopez(&sec[s0],1,pol[0].x,pol[0].y);
						pol[1].z = getslopez(&sec[s0],1,pol[1].x,pol[1].y);
					} else { pol[0].z = lefz[0]; pol[1].z = rigz[0]; }

					if ((k) && ((!(gotsect[s0>>5]&(1<<s0))) || (wal[w].surf.flags&32))) //1-way
					{
						if (distpoint2line2(gdps->ipos.x,gdps->ipos.y,wal[w].x,wal[w].y,wal[nw].x,wal[nw].y) <= SCISDIST*SCISDIST*1.0001)
							{ secfif[secfifw] = s0; secfifw++; gotsect[s0>>5] |= (1<<s0); }
						else
						{
							if (wal[w].surf.flags&32)
							{
								pol[0].u = wal[w].surf.uv[0].x; //FIXFIX
								pol[0].v = wal[w].surf.uv[0].y;
								if (!(wal[w].surf.flags&4))
									  pol[0].v += wal[w].surf.uv[2].y*(pol[0].z-sec[           s].z[0]);
								else pol[0].v += wal[w].surf.uv[2].y*(pol[0].z-sec[verts[k-1].s].z[0]);
								pol[1].u = wal[w].surf.uv[2].x*(pol[1].z-pol[0].z) + pol[0].u + wal[w].surf.uv[1].x*dx;
								pol[1].v = wal[w].surf.uv[2].y*(pol[1].z-pol[0].z) + pol[0].v + wal[w].surf.uv[1].y*dx;
								pol[2].u = wal[w].surf.uv[2].x*(pol[2].z-pol[0].z) + pol[0].u + wal[w].surf.uv[1].x*dx;
								pol[2].v = wal[w].surf.uv[2].y*(pol[2].z-pol[0].z) + pol[0].v + wal[w].surf.uv[1].y*dx;
								pol[3].u = wal[w].surf.uv[2].x*(pol[3].z-pol[0].z) + pol[0].u;
								pol[3].v = wal[w].surf.uv[2].y*(pol[3].z-pol[0].z) + pol[0].v;
							}
							rpol[0] = pol[3]; rpol[1] = pol[2]; rpol[2] = pol[1]; rpol[3] = pol[0];
							if ((rpol[0].z < lefz[0]) && (rpol[1].z < rigz[0])) { rpol[0].z = lefz[0]; rpol[1].z = rigz[0]; }
							if ((rpol[3].z > lefz[1]) && (rpol[2].z > rigz[1])) { rpol[3].z = lefz[1]; rpol[2].z = rigz[1]; }
							i = wallclip(rpol,npol);
							if (i)
							{
								for(j=0;j<i;j++) xformpos(&npol[j].x,&npol[j].y,&npol[j].z);
								if (!(gotsect[s0>>5]&(1<<s0)))
									if (isvispol(cc,npol,i)) { secfif[secfifw] = s0; secfifw++; gotsect[s0>>5] |= (1<<s0); }
								if (wal[w].surf.flags&32)
									drawpolfunc(cc,npol,i,&gtile[wal[w].surf.tilnum],col[0],0,&norm,RENDFLAGS_RALPHAMASK);
							}
						}
					}

					if (k < vn)
					{
						pol[2].z = getslopez(&sec[verts[k].s],0,pol[2].x,pol[2].y);
						pol[3].z = getslopez(&sec[verts[k].s],0,pol[3].x,pol[3].y);
					} else { pol[2].z = rigz[1]; pol[3].z = lefz[1]; }

					if (!(wal[w].surf.flags&(1<<16))) //no parallax
					{
						pol[0].u = wal[w].surf.uv[0].x; //FIXFIX
						pol[0].v = wal[w].surf.uv[0].y;
						if (!(wal[w].surf.flags&4))
										  pol[0].v += wal[w].surf.uv[2].y*(pol[0].z-sec[           s].z[0]);
						else if (!vn) pol[0].v += wal[w].surf.uv[2].y*(pol[0].z-sec[           s].z[1]); //White walls don't have verts[]!
						else if (!k)  pol[0].v += wal[w].surf.uv[2].y*(pol[0].z-sec[verts[  0].s].z[0]);
						else          pol[0].v += wal[w].surf.uv[2].y*(pol[0].z-sec[verts[k-1].s].z[0]);

						pol[1].u = wal[w].surf.uv[2].x*(pol[1].z-pol[0].z) + pol[0].u + wal[w].surf.uv[1].x*dx;
						pol[1].v = wal[w].surf.uv[2].y*(pol[1].z-pol[0].z) + pol[0].v + wal[w].surf.uv[1].y*dx;
						pol[2].u = wal[w].surf.uv[2].x*(pol[2].z-pol[0].z) + pol[0].u + wal[w].surf.uv[1].x*dx;
						pol[2].v = wal[w].surf.uv[2].y*(pol[2].z-pol[0].z) + pol[0].v + wal[w].surf.uv[1].y*dx;
						pol[3].u = wal[w].surf.uv[2].x*(pol[3].z-pol[0].z) + pol[0].u;
						pol[3].v = wal[w].surf.uv[2].y*(pol[3].z-pol[0].z) + pol[0].v;
						i = wallclip(pol,npol); if (!i) continue;
						for(j=0;j<i;j++) xformpos(&npol[j].x,&npol[j].y,&npol[j].z);
						drawpolfunc(cc,npol,i,&gtile[wal[w].surf.tilnum],col[0],0,&norm,0);
					}
					else //parallaxing walls
					{
						i = wallclip(pol,npol); if (!i) continue;
						for(j=0;j<i;j++) xformpos(&npol[j].x,&npol[j].y,&npol[j].z);
						drawparallaxpol(cc,npol,i,&gtile[wal[w].surf.tilnum],col[0],&wal[w].surf,&norm,0);
					}

#ifdef STANDALONE
					if ((showedges3d) && (wal[w].surf.tag)) drawtag_wall(cc,s,w,dx,pol);
#endif
				}
#ifdef STANDALONE
				if (showedges3d)
				{
					if ((wal[w].ns >= 0) && ((gdps->ipos.x-wal[w].x)*(wal[nw].y-wal[w].y) < (gdps->ipos.y-wal[w].y)*(wal[nw].x-wal[w].x)))
						for(i=0;i<2;i++)
						{
							pol[0].z = getslopez(&sec[s],i,pol[0].x,pol[0].y);
							pol[1].z = getslopez(&sec[s],i,pol[1].x,pol[1].y);
							for(k=0;k<vn;k++) //Warning: do not reverse for loop!
							{
								pol[2].z = getslopez(&sec[verts[k].s],i,pol[0].x,pol[0].y);
								pol[3].z = getslopez(&sec[verts[k].s],i,pol[1].x,pol[1].y);
								if (fabs(pol[2].z-pol[0].z) + fabs(pol[1].z-pol[3].z) < .03125)
								{
									drawcone(cc,wal[ w].x,wal[ w].y,getslopez(&sec[s],i,wal[ w].x,wal[ w].y),-.0125,
													wal[nw].x,wal[nw].y,getslopez(&sec[s],i,wal[nw].x,wal[nw].y),-.0125,0x904040);
									break;
								}
							}
						}
				}
#endif
			}
		}
	}

		//FIXFIX:replace with drawkv6:disable_depth_test
	if (gdps->compact2d > 0.0) //Clear Z-buffer for 2D sprites
	{
		for(i=0,j=cc->z.f;i<cc->c.y;i++,j+=cc->z.p) memset8((void *)j,0x7f7f7f7f,cc->c.x<<2); //Clear z-buffer for rendering models :/
	}

skipdrawrooms_lab:;

		//Draw sprites
	//for(secfifr=0;secfifr<secfifw;secfifr++)
	//{
	//   s = secfif[secfifr];
	//   for(w=sec[s].headspri;w>=0;w=gst->spri[w].sectn)
	if (gotsect)
	{
		for(s=uptil1(gotsect,gst->numsects);s>0;s=uptil1(gotsect,s-1))
			for(w=sec[s-1].headspri;w>=0;w=gst->spri[w].sectn)
			{
				spr = &gst->spri[w];
				if (!(spr->flags&0x80000000)) drawsprite(cc,spr); //Draw non-invisible sprites
			}
	}

#ifdef STANDALONE
	if ((!gdps->showdebug) && (!didpolybool))
	{
		if (gdps->editmode == 2)
		{
				//draw new sectors for each player
			for(p=0,pps=&gst->p[0];p<numplayers;p++,pps++)
			{
				if (!pps->sec.wall) continue;
				if (pps->startstate == 2) continue;
				if (p == viewindex) col[0] = 0xc0a080; else col[0] = 0x806040;
				if (gdps->ipos.z > getslopez(&pps->sec,0,gdps->ipos.x,gdps->ipos.y)) drawsectfill3d(cc,&pps->sec,0,(col[0]&0xfefefe)>>1);
				if (gdps->ipos.z < getslopez(&pps->sec,1,gdps->ipos.x,gdps->ipos.y)) drawsectfill3d(cc,&pps->sec,1,(col[0]&0xfefefe)>>1);
			}
		}
	}

	drawgrid(cc,1);

		//draw wall lines for 2d mode
	if ((gdps->compact2d > 0.0) && (fabs(gdps->grdn.z) == 1.0))
	{
		col[0] = 0xa0a0a0; col[1] = 0xa00000;
		if (gdps->compact2d >= 1.0) { col[0] |= 0xff000000; col[1] |= 0xff000000; }
		for(i=2-1;i>=0;i--) col[i] = argb_interp(col[i]&0xff000000,col[i],(int)(gdps->compact2d*32767.0));

		k = (gdps->grdn.z >= 0);
		for(s=0;s<gst->numsects;s++)
		{
			wal = sec[s].wall; i = sec[s].n;
			for(w=0;w<i;w++)
			{
				nw = wal[w].n+w;
				drawline3d(cc,wal[ w].x,wal[ w].y,getslopez(&sec[s],k,wal[ w].x,wal[ w].y),
								  wal[nw].x,wal[nw].y,getslopez(&sec[s],k,wal[nw].x,wal[nw].y),col[wal[w].ns>=0]);
			}
		}
	}

		//draw walls of new sectors for each player
	for(p=0,pps=&gst->p[0];p<numplayers;p++,pps++)
	{
		if (p == viewindex) col[0] = 0xffc0a080; else col[0] = 0xffa08060;
		col[0] = argb_interp(col[0],0xff000000,((int)(sin(dtotclk*16.f)*8191.f))+8192);
		//if (gdps->compact2d == 0.0) col[0] &= 0xffffff;

		wal = pps->sec.wall; i = pps->sec.n;
		if ((i) && (!didpolybool))
		{
			i--;
			getcurspoint(pps,&hit.x,&hit.y,&hit.z,&hitsect,&hitwall,&fp);
			if (gdps->editmode == 3)
				drawcone(cc,wal[i].x,wal[i].y,getslopez(&pps->sec,0,wal[i].x,wal[i].y),-.0125,
									hit.x,   hit.y,getslopez(&pps->sec,0,   hit.x,   hit.y),-.0125,col[0]); //rubber banding
			else
				drawline3d(cc,wal[i].x,wal[i].y,getslopez(&pps->sec,0,wal[i].x,wal[i].y),
									  hit.x,   hit.y,getslopez(&pps->sec,0,   hit.x,   hit.y),col[0]);
			if (pps->editmode == 3) f = .025; else f = min(pps->height2d,64.f)/(128.f-8.f);
			j = argb_interp(0xc0a080,0x000000,((int)(sin(dtotclk*16.f)*8191.f))+8192);
			if (gdps->compact2d < 1.0)
				drawsph(cc,hit.x,hit.y,hit.z,f,j);
			else
			{
				fp.x = hit.x; fp.y = hit.y; fp.z = hit.z; xformpos(&fp.x,&fp.y,&fp.z);
				if (fp.z > SCISDIST)
				{
					fp.z = gdps->ghz/fp.z;
					fp.x = fp.x*fp.z + gdps->ghx;
					fp.y = fp.y*fp.z + gdps->ghy;
					drawcirc(&cc->c,fp.x,fp.y,-3,j+0x101010);
				}
			}
		}
		for(w=0;w<i;w++)
		{
			nw = wal[w].n+w;
			if (gdps->editmode == 3)
				drawcone(cc,wal[ w].x,wal[ w].y,getslopez(&pps->sec,0,wal[ w].x,wal[ w].y),-.0125,
								wal[nw].x,wal[nw].y,getslopez(&pps->sec,0,wal[nw].x,wal[nw].y),-.0125,col[0]);
			else
				drawline3d(cc,wal[ w].x,wal[ w].y,getslopez(&pps->sec,0,wal[ w].x,wal[ w].y),
								  wal[nw].x,wal[nw].y,getslopez(&pps->sec,0,wal[nw].x,wal[nw].y),col[0]);
		}

		if ((pps->grabmode == GRABCIRC) && ((unsigned)pps->grabsect < (unsigned)gst->numsects))
		{
			float x0, y0, x1, y1, x2, y2, centx, centy, circrad, circang0, circang1;
			x0 = gst->sect[pps->grabsect].wall[pps->grabwall].x;
			y0 = gst->sect[pps->grabsect].wall[pps->grabwall].y;
			i = gst->sect[pps->grabsect].wall[pps->grabwall].n+pps->grabwall;
			x1 = gst->sect[pps->grabsect].wall[i].x;
			y1 = gst->sect[pps->grabsect].wall[i].y;
			getcurspoint(pps,&fp.x,&fp.y,&fp.z,&hitsect,&hitwall,&hit);
			x2 = fp.x;
			y2 = fp.y;
			g = (y0-y1)*(x0-x2) + (y0-y2)*(x1-x0);
			if ((fabs(g) > 0.001) && ((hitwall < 0) || (pps->editmode == 2)))
			{
				f = ((x2-x1)*(x0-x2) + (y0-y2)*(y2-y1))/g;
				centx = ((y0-y1)*f + (x0+x1))*.5;
				centy = ((x1-x0)*f + (y0+y1))*.5;
				fp.x = centx; fp.y = centy; fp.z = getslopez(&gst->sect[pps->grabsect],hitwall&1,centx,centy);
				if (gdps->compact2d < 1.0)
					drawsph(cc,fp.x,fp.y,fp.z,.1,0xffff00);
				else
				{
					xformpos(&fp.x,&fp.y,&fp.z);
					if (fp.z > SCISDIST)
					{
						fp.z = gdps->ghz/fp.z;
						fp.x = fp.x*fp.z + gdps->ghx;
						fp.y = fp.y*fp.z + gdps->ghy;
						drawcirc(&cc->c,fp.x,fp.y,4,0xffff00);
					}
				}

				circang0 = atan2(y0-centy,x0-centx);
				circang1 = atan2(y1-centy,x1-centx);
				f = fmod((double)(circang1-circang0),PI*2); if (f < 0) f += PI*2;
				if ((x2-x0)*(y1-y0) < (x1-x0)*(y2-y0)) f -= PI*2;
				circrad = sqrt((centx-x0)*(centx-x0) + (centy-y0)*(centy-y0));
				x2 = x1; y2 = y1;
				for(i=pps->circnum;i>=0;i--)
				{
					if (i > 0)
					{
						g = ((double)i)*f/((double)(pps->circnum+1)) + circang0;
						dx = cos(g)*circrad + centx;
						dy = sin(g)*circrad + centy;
					}
					else { dx = x0; dy = y0; }
					if (gdps->editmode == 3)
						drawcone(cc,x2,y2,getslopez(&gst->sect[pps->grabsect],hitwall&1,x2,y2),-.0125,
										dx,dy,getslopez(&gst->sect[pps->grabsect],hitwall&1,dx,dy),-.0125,0xff909030);
					else
						drawline3d(cc,x2,y2,getslopez(&gst->sect[pps->grabsect],hitwall&1,x2,y2),
										  dx,dy,getslopez(&gst->sect[pps->grabsect],hitwall&1,dx,dy),0xff909030);
					if (i < pps->circnum)
					{
						fp.x = x2; fp.y = y2; fp.z = getslopez(&gst->sect[pps->grabsect],hitwall&1,x2,y2);
						if (gdps->compact2d < 1.0)
							drawsph(cc,fp.x,fp.y,fp.z,.05,0xc0c040);
						else
						{
							xformpos(&fp.x,&fp.y,&fp.z);
							if (fp.z > SCISDIST)
							{
								fp.z = gdps->ghz/fp.z;
								fp.x = fp.x*fp.z + gdps->ghx;
								fp.y = fp.y*fp.z + gdps->ghy;
								drawcirc(&cc->c,fp.x,fp.y,-3,0xc0c040);
							}
						}
					}
					x2 = dx; y2 = dy;
				}

				if (gdps->editmode == 2)
				{
					fp.x = centx; fp.y = centy; fp.z = 0;
					xformpos(&fp.x,&fp.y,&fp.z);
					if (fp.z > SCISDIST)
					{
						fp.z = gdps->ghz/fp.z;
						fp.x = fp.x*fp.z + gdps->ghx;
						fp.y = fp.y*fp.z + gdps->ghy;
						print6x8(&cc->c,fp.x-((pps->circnum>=99)+(pps->circnum>=9)+1)*3,fp.y+lineHeight,0xffff00,-1,"%d",pps->circnum+1);
					}
				}
			}
		}
	}

		//draw quantized point in 3D mode
	if ((gdps->editmode == 3) && ((unsigned)hitsect < (unsigned)gst->numsects) && (gps->showedges3d))
	{
		float f, g, fx, fy, fz, ofx, ofy, ofz, f2;
		int i, j, n, x, y, x0, y0, col;

		getcurspoint(gdps,&fp.x,&fp.y,&fp.z,&hitsect,&hitwall,&hit);
		if (hitwall >= 0) hitwall = (fp.z*2 >= getslopez(&sec[hitsect],0,fp.x,fp.y)+getslopez(&sec[hitsect],1,fp.x,fp.y));

		if (gdps->gridlock) //&& ((gdps->grabmode == GRABDRAG) || (gdps->grabmode == GRABDRAG2)))
		{
			f = 1.f/gdps->dgridlock;
			x0 = (int)floor(fp.x*f+.5);
			y0 = (int)floor(fp.y*f+.5);
			for(y=-1;y<=1;y++)
				for(x=-1;x<=1;x++)
				{
					if (labs(x)+labs(y) > 1) continue;
					fx = (x+x0)*gdps->dgridlock; fy = (y+y0)*gdps->dgridlock;
					drawsph(cc,fx,fy,getslopez(&sec[hitsect],hitwall&1,fx,fy),gdps->dgridlock*.05,0xffb46c48);
				}
		}

		for(j=2-1;j>=0;j--)
		{
			if (!j) { f = gdps->dgridlock*DVERTSNAP; col = 0x506050; }
				else { f = gdps->dgridlock*DEDGESNAP; col = 0x604040; }
			n = 64; g = PI*2.0/(float)n;
			for(i=n;i>=0;i--)
			{
				fx = cos(((float)i)*g)*f + fp.x;
				fy = sin(((float)i)*g)*f + fp.y;
				fz = getslopez(&sec[hitsect],hitwall&1,fx,fy);
				fx += (gdps->ipos.x-fx)*.001;
				fy += (gdps->ipos.y-fy)*.001;
				fz += (gdps->ipos.z-fz)*.001;
				if (i < n) drawline3d(cc,ofx,ofy,ofz,fx,fy,fz,col);
				ofx = fx; ofy = fy; ofz = fz;
			}
		}

		j = argb_interp(0xc0a080,0x000000,((int)(sin(dtotclk*16.f)*8191.f))+8192);
		drawsph(cc,fp.x,fp.y,fp.z,gdps->dgridlock*.15,j);

	}

		//draw quantized point if not already done so in previous loop
	if ((gdps->editmode == 2) && (!((gdps->sec.n) && (!didpolybool))))
	{
		getcurspoint(gdps,&fp.x,&fp.y,&fp.z,&hitsect,&hitwall,&hit);
		xformpos(&fp.x,&fp.y,&fp.z);
		if (fp.z > SCISDIST)
		{
			j = argb_interp(0xc0a080,0x000000,((int)(sin(dtotclk*16.f)*8191.f))+8192);
			fp.z = gdps->ghz/fp.z;
			fp.x = fp.x*fp.z + gdps->ghx;
			fp.y = fp.y*fp.z + gdps->ghy;
			drawcirc(&cc->c,fp.x,fp.y,-3,j);
			if (gdps->editmode == 2) //Draw snap distances
			{
				drawcirc(&cc->c,fp.x,fp.y,-gdps->dgridlock*DEDGESNAP*fp.z,0x604040);
				drawcirc(&cc->c,fp.x,fp.y,-gdps->dgridlock*DVERTSNAP*fp.z,0x506050);
			}
		}
	}

		//draw sprites vertices&tags
	if (gotsect)
	{
		for(s=uptil1(gotsect,gst->numsects);s>0;s=uptil1(gotsect,s-1))
		{
			for(w=sec[s-1].headspri;w>=0;w=gst->spri[w].sectn)
			{
				spr = &gst->spri[w];
				if (gdps->compact2d >= 1.f) //Draw sprite cyan marker for 2D mode
				{
					fp.x = spr->p.x; fp.y = spr->p.y; fp.z = spr->p.z; xformpos(&fp.x,&fp.y,&fp.z);
					if (fp.z > SCISDIST)
					{
						fp.z = gdps->ghz/fp.z;
						fp.x = fp.x*fp.z + gdps->ghx;
						fp.y = fp.y*fp.z + gdps->ghy;
						j = spr->flags & 1 ? 0xc010e0 : 0x40c0c0;  // Face/forward axis cyan // blue normally
						if (spr->owner >= 0)
						{
							if (spr->owner == gdps->playerindex) j = flashcol;
							else j = ((flashcol & 0xfefefe) >> 1);
						}

						drawcirc(&cc->c,fp.x,fp.y,-3-(spr->owner>=0),j);

						// blue axis(f) - rotation around it is ok but displayed improperly by sprite stretch, so prob that is the issue.
						// note that editor rotation via ,. always happens around vertical axis. needs revision.

						fp2.x = spr->f.x; fp2.y = spr->f.y; fp2.z = spr->f.z;
						xformrot(&fp2.x,&fp2.y,&fp2.z);
						// scaling in depth is not supported so sprite depth axis remains constant length
						// but that wil be handy for voxels
						f = 32;//fp2.x*fp2.x + fp2.y*fp2.y; if (f > 0) f = 16.0/sqrt(f);
						drawline2d(&cc->c,fp.x,fp.y,fp.x+fp2.x*f,fp.y+fp2.y*f,j);

						j = 0xc04040; // right axis - yellow // red normally
						if (spr->owner >= 0)
						{
							if (spr->owner == gdps->playerindex) j = flashcol;
							else j = ((flashcol & 0xfefefe) >> 1);
						}
						fp2.x = spr->r.x; fp2.y = spr->r.y; fp2.z = spr->r.z;
						xformrot(&fp2.x,&fp2.y,&fp2.z);
						f = 32;//fp2.x*fp2.x + fp2.y*fp2.y; if (f > 0) f = 16.0/sqrt(f)*(1-fp2.z);
						drawline2d(&cc->c,fp.x,fp.y,fp.x+fp2.x*f,fp.y+fp2.y*f,j);

						j = 0x40c040; // down axis - magenta // green normally
						if (spr->owner >= 0)
						{
							if (spr->owner == gdps->playerindex) j = flashcol;
							else j = ((flashcol & 0xfefefe) >> 1);
						}
						fp2.x = spr->d.x; fp2.y = spr->d.y; fp2.z = spr->d.z;
						xformrot(&fp2.x,&fp2.y,&fp2.z);
						f = 32;// fp2.x*fp2.x + fp2.y*fp2.y  + fp2.z*fp2.z ; //if (f > 0) f = 16.0/sqrt(f);
						drawline2d(&cc->c,fp.x,fp.y,fp.x+fp2.x*5,fp.y+fp2.y*5,j);
					}
				}
				if ((showedges3d) && (spr->tag)) drawtag_spri(cc,w);
			}
		}
	}

		//draw sector vertices
	if ((gdps->compact2d != 0.0) && (!gdps->showdebug))
	{
		k = argb_interp(0x60ef60,0x000000,((int)(sin(dtotclk*16.f)*8191.f))+8192);
		for(s=0;s<gst->numsects;s++)
			for(w=0,wal=sec[s].wall;w<sec[s].n;w++)
			{
				j = 0x009000;
				if (wal[w].owner >= 0) { if (wal[w].owner == gdps->playerindex) j = k; else j = ((k&0xfefefe)>>1); }
				if (gdps->compact2d < 1.0)
					  drawsph(cc,wal[w].x,wal[w].y,getslopez(&sec[s],gdps->grdn.z>=0,wal[w].x,wal[w].y),gdps->compact2d*(min(max(gdps->height2d,1.f),8.f)/128.f),j);
				else
				{
					fp.x = wal[w].x; fp.y = wal[w].y; fp.z = 0; xformpos(&fp.x,&fp.y,&fp.z);
					if (fp.z > SCISDIST)
					{
						fp.z = gdps->ghz/fp.z;
						fp.x = fp.x*fp.z + gdps->ghx;
						fp.y = fp.y*fp.z + gdps->ghy;
						if (fp.z > 8) drawcirc(&cc->c,fp.x,fp.y,-3-(wal[w].owner>=0),j+0x101010);
					}
				}
			}
	}

		//draw vertices of new sectors for each player
	if (!gdps->showdebug)
	{
		for(pps=&gst->p[0];pps<&gst->p[numplayers];pps++)
			for(w=0,wal=pps->sec.wall;w<pps->sec.n;w++)
			{
				if (gdps->compact2d < 1.0)
					  drawsph(cc,wal[w].x,wal[w].y,getslopez(&pps->sec,0,wal[w].x,wal[w].y),.025f,0xc0a080);
				else
				{
					fp.x = wal[w].x; fp.y = wal[w].y; fp.z = 0; xformpos(&fp.x,&fp.y,&fp.z);
					if (fp.z > SCISDIST)
					{
						fp.z = gdps->ghz/fp.z;
						fp.x = fp.x*fp.z + gdps->ghx;
						fp.y = fp.y*fp.z + gdps->ghy;
						drawcirc(&cc->c,fp.x,fp.y,-3+(w==0)*8,0xd0b090);
					}
				}
			}
	}

		//Draw texture in tab
	if (gps->gotcopy&1)
	{
		kgln_t vert[4];

		f = max(cc->c.x*0.06,48);

		vert[0].x = vert[3].x = cc->c.x-f; vert[0].y = vert[1].y = 0;
		vert[1].x = vert[2].x = cc->c.x  ; vert[2].y = vert[3].y = f;
		for(i=4-1;i>=0;i--)
		{
			vert[i].x -= cc->h.x; vert[i].y -= cc->h.y; vert[i].z = cc->h.z;
			vert[i].u = ((i+1)>>1); vert[i].v = (i>>1)+1e-4; vert[i].n = 1;
		}
		vert[3].n = -3;

		i = (min(gps->copysurf[0].asc>>8,255)<<24) +
			 (min(gps->copysurf[0].rsc>>5,255)<<16) +
			 (min(gps->copysurf[0].gsc>>5,255)<< 8) +
			 (min(gps->copysurf[0].bsc>>5,255)    );

		drawpol(cc,vert,4,&gtile[gps->copysurf[0].tilnum],i,0,0,RENDFLAGS_NODEPTHTEST);
		print6x8(&cc->c,cc->c.x-max(f*.5+3*8,6*8)+1,f+1,0x000000,-1,"Tab grab");
		print6x8(&cc->c,cc->c.x-max(f*.5+3*8,6*8)+2,f+2,0xffffff,-1,"Tab grab");
	}

	if (gdps->showdebug)
	{
		j = 16;
		//print6x8(&cc->c,0,j+=8,0xffffff,-1,"mouse: %5.1f %5.1f",qmousx,qmousy);
		for(s=0;s<gst->numsects;s++)
		{
			print6x8(&cc->c,0,j+=8,0xa0e0f0,-1,"Sector%2d:%5.1f %5.1f %08x %d(%d) hspri:%d",s,sec[s].z[0],sec[s].z[1],sec[s].wall,sec[s].n,sec[s].nmax,sec[s].headspri);
			wal = sec[s].wall;
			for(w=0;w<sec[s].n;w++)
				print6x8(&cc->c,0,j+=8,0xf0a090,-1,"%5d:%5.1f %5.1f %+2d %+2d %+2d",w,wal[w].x,wal[w].y,wal[w].n,wal[w].ns,wal[w].nw);
			if (j >= cc->c.y) break;
		}

		j += 8;
		for(s=0;s<gst->numspris;s++)
		{
			if (gst->spri[s].sect < 0) continue;
			print6x8(&cc->c,0,j+=8,0xa0e0f0,-1,"Spri%2d:%5.1f %5.1f %+2d p:%+2d n:%+2d",s,gst->spri[s].p.x,gst->spri[s].p.y,gst->spri[s].sect,gst->spri[s].sectp,gst->spri[s].sectn);
			if (j >= cc->c.y) break;
		}

		j += 8;
		for(s=0;s<numplayers;s++)
		{
			sec = &gst->p[s].sec;
			print6x8(&cc->c,0,j+=8,0xa0e0f0,-1,"Player%2d:%5.1f %5.1f %08x %d(%d)",s,sec->z[0],sec->z[1],sec->wall,sec->n,sec->nmax);
			wal = sec->wall;
			for(w=0;w<sec->n;w++)
				print6x8(&cc->c,0,j+=8,0xf0a090,-1,"%5d:%5.1f %5.1f %+2d %+2d %+2d",w,wal[w].x,wal[w].y,wal[w].n,wal[w].ns,wal[w].nw);
			if (j >= cc->c.y) break;
		}

		sec = gst->sect;
	}

		//Show center of screen (where you would flip to 3D)
	//if (gdps->editmode == 2) drawsph(cc,gdps->ipos.x,gdps->ipos.y,gdps->ipos.z,gdps->height2d*.01,0xc0c0c0);

	if (gdps->showedges3d)
	{
		drawcone(cc,gst->startpos.x+gst->startfor.x*-.20,
						gst->startpos.y+gst->startfor.y*-.20,
						gst->startpos.z+gst->startfor.z*-.20,.02,
						gst->startpos.x+gst->startfor.x*-.05,
						gst->startpos.y+gst->startfor.y*-.05,
						gst->startpos.z+gst->startfor.z*-.05,-.02,0xc0c0c0);
		drawcone(cc,gst->startpos.x+gst->startfor.x*-.10,
						gst->startpos.y+gst->startfor.y*-.10,
						gst->startpos.z+gst->startfor.z*-.10,-.05,
						gst->startpos.x+gst->startfor.x*+.00,
						gst->startpos.y+gst->startfor.y*+.00,
						gst->startpos.z+gst->startfor.z*+.00,.005,0xc0c0c0);
	}

		//Draw avatar
	for(i=0;i<numplayers;i++)
	{
		pps = &gst->p[i]; if (i == viewindex) continue;

		if (pps->editmode == 2)
		{
			curs2grid(pps,pps->fcmousx,pps->fcmousy,&fp); fx = fp.x; fy = fp.y;
			xformpos(&fp.x,&fp.y,&fp.z);
			if (fp.z > SCISDIST)
			{
				fp.z = gdps->ghz/fp.z;
				fp.x = fp.x*fp.z + gdps->ghx;
				fp.y = fp.y*fp.z + gdps->ghy;
				drawmouse(&cc->c,fp.x,fp.y,0x908060);
			}
			//if (pps->typemode) continue;
		}
		else
		{
				//draw other player as sphere head :)
			drawsph(cc,pps->ipos.x,pps->ipos.y,pps->ipos.z,.25f,0x908060); //Draw head

			if (pps->emoticon_hair) //Draw hair
			{
				for(f=PI/24;f<PI*2;f+=PI/12) //pink bow (female)
				{
					fp.x = cos(f)*.1; fp.y = sin(f*2.f)*.05-.3;
					drawsph(cc,pps->ipos.x + fp.x*pps->irig.x + fp.y*pps->idow.x,
								  pps->ipos.y + fp.x*pps->irig.y + fp.y*pps->idow.y,
								  pps->ipos.z + fp.x*pps->irig.z + fp.y*pps->idow.z,.020f,0xc060c0);
				}
			}
			for(j=0;j<2;j++) //Draw eyes
			{
				if (pps->emoticon_eyes&(j+1)) f = 1.3f; else f = 1.f;
				k = (j<<1)-1;
				drawsph(cc,pps->ipos.x + pps->ifor.x*.17f - pps->idow.x*.07 + pps->irig.x*((float)k)*.08f,
							  pps->ipos.y + pps->ifor.y*.17f - pps->idow.y*.07 + pps->irig.y*((float)k)*.08f,
							  pps->ipos.z + pps->ifor.z*.17f - pps->idow.z*.07 + pps->irig.z*((float)k)*.08f,f*.07f,0xffffff);
				drawsph(cc,pps->ipos.x + pps->ifor.x*.22f*(f*.5+.5) - pps->idow.x*.075 + pps->irig.x*((float)k)*.088f, //pupil
							  pps->ipos.y + pps->ifor.y*.22f*(f*.5+.5) - pps->idow.y*.075 + pps->irig.y*((float)k)*.088f,
							  pps->ipos.z + pps->ifor.z*.22f*(f*.5+.5) - pps->idow.z*.075 + pps->irig.z*((float)k)*.088f,f*.03f,0x000000);

				if (pps->emoticon_eyes&(4<<j)) //wink animation
				{
					if ((pps->emoticon_eyes&12) == 12) fp.z = .162f; else fp.z = sin(dtotclk*PI)*.032+.130f;
					drawsph(cc,pps->ipos.x + pps->ifor.x*fp.z - pps->idow.x*.075 + pps->irig.x*((float)k)*.078f,
								  pps->ipos.y + pps->ifor.y*fp.z - pps->idow.y*.075 + pps->irig.y*((float)k)*.078f,
								  pps->ipos.z + pps->ifor.z*fp.z - pps->idow.z*.075 + pps->irig.z*((float)k)*.078f,f*.075f,0x908060);
					drawsph(cc,pps->ipos.x + pps->ifor.x*fp.z - pps->idow.x*.050 + pps->irig.x*((float)k)*.078f,
								  pps->ipos.y + pps->ifor.y*fp.z - pps->idow.y*.050 + pps->irig.y*((float)k)*.078f,
								  pps->ipos.z + pps->ifor.z*fp.z - pps->idow.z*.050 + pps->irig.z*((float)k)*.078f,f*.075f,0x908060);
				}
			}
			if (!pps->emoticon_nose)
			{
				drawsph(cc,pps->ipos.x + pps->ifor.x*.25f - pps->idow.x*.02,
							  pps->ipos.y + pps->ifor.y*.25f - pps->idow.y*.02,
							  pps->ipos.z + pps->ifor.z*.25f - pps->idow.z*.02,.035f,0x987860);
			}
			for(j=-3;j<=3;j++)
			{
				switch(pps->emoticon_mouth)
				{
					case 1: case 4: f = ((float)(j*j))*-.003+.075; break;
					case 2: f = ((float)(j*j))*+.003+.065; break;
					case 3: f = ((float)j)*-.007+.07; break;
					default: f = .07; break;
				}
				if (!pps->emoticon_hair) col[0] = 0xa07060; else col[0] = 0xb85040;

				fp2.x = fp.x; fp.x = pps->ipos.x + pps->ifor.x*.22f + pps->idow.x*f + pps->irig.x*((float)j)*.02f;
				fp2.y = fp.y; fp.y = pps->ipos.y + pps->ifor.y*.22f + pps->idow.y*f + pps->irig.y*((float)j)*.02f;
				fp2.z = fp.z; fp.z = pps->ipos.z + pps->ifor.z*.22f + pps->idow.z*f + pps->irig.z*((float)j)*.02f;
				if (j > -3) drawcone(cc,fp2.x,fp2.y,fp2.z,.03f,fp.x,fp.y,fp.z,.03f,col[0]);
				else if (!pps->emoticon_mouth) j = 2; //optimization for :|
			}
			if (pps->emoticon_mouth == 4) //tongue
			{
				drawcone(cc,pps->ipos.x + pps->ifor.x*.22f + pps->idow.x*+.085 + pps->irig.x*-.04f,
								pps->ipos.y + pps->ifor.y*.22f + pps->idow.y*+.085 + pps->irig.y*-.04f,
								pps->ipos.z + pps->ifor.z*.22f + pps->idow.z*+.085 + pps->irig.z*-.04f,.03f,
								pps->ipos.x + pps->ifor.x*.24f + pps->idow.x*.109f + pps->irig.x*-.056f,
								pps->ipos.y + pps->ifor.y*.24f + pps->idow.y*.109f + pps->irig.y*-.056f,
								pps->ipos.z + pps->ifor.z*.24f + pps->idow.z*.109f + pps->irig.z*-.056f,.03f,
								0xc04040);
			}

				//Draw hitscan point
			if ((pps->grabmode != GRABSURF) && (hitscan(&pps->ipos,&pps->ifor,1e32,pps->cursect,&j,&k,&fp)))
			{
				fp2.x = fp.x-pps->ipos.x;
				fp2.y = fp.y-pps->ipos.y;
				fp2.z = fp.z-pps->ipos.z;
				f = fp2.x*fp2.x + fp2.y*fp2.y + fp2.z*fp2.z;
				if (f > 0.0)
				{
					g = 1 - .25/sqrt(f);
					drawcone(cc,pps->ipos.x+fp2.x*g,pps->ipos.y+fp2.y*g,pps->ipos.z+fp2.z*g,.02,
									pps->ipos.x+fp2.x  ,pps->ipos.y+fp2.y  ,pps->ipos.z+fp2.z  ,.02,0x908060);
					drawsph(cc,fp.x,fp.y,fp.z,sin(dtotclk*16.f)*.005f+.035f,0x908060);

					if (pps->grabmode == GRABRGB)
					{
						double rat, nc, ns;
						kgln_t vert[6];
						rat = 1.0 - 1.0/COLWHEEL_SIZ;

						g = 1 - .29/sqrt(f);
						fp.x = pps->ipos.x + fp2.x*g;
						fp.y = pps->ipos.y + fp2.y*g;
						fp.z = pps->ipos.z + fp2.z*g;

						for(k=0;k<6;k++)
						{
							nc = cos(((double)k+.5)*PI/3);
							ns = sin(((double)k+.5)*PI/3);
							vert[k].x = (nc*pps->irig.x + ns*pps->idow.x)*.0625 + fp.x;
							vert[k].y = (nc*pps->irig.y + ns*pps->idow.y)*.0625 + fp.y;
							vert[k].z = (nc*pps->irig.z + ns*pps->idow.z)*.0625 + fp.z;
							xformpos(&vert[k].x,&vert[k].y,&vert[k].z);
							vert[k].u = (nc*rat + 1.0)/2.0;
							vert[k].v = (ns*rat + 1.0)/2.0;
							vert[k].n = 1;
						}
						vert[5].n = -5;

						k = ((gdps->ipos.x-fp.x)*pps->ifor.x +
							  (gdps->ipos.y-fp.y)*pps->ifor.y +
							  (gdps->ipos.z-fp.z)*pps->ifor.z < 0);

						if (!k) kglcullmode = 0x404;
						drawpol(cc,vert,6,&colwheel,((int)min(max(pps->selrgbintens*128,0),255))*0x10101+0xff000000,0,0,RENDFLAGS_INTERP);
						if (!k) kglcullmode = 0x405;
					}
				}
			}

				//Draw avatar sign (name)
			til.tt.f = (intptr_t)(&avatarsign[i][0][0]);
			til.tt.x = AVATARSIGNX; til.tt.y = AVATARSIGNY; til.tt.p = (til.tt.x<<2);
			if (gst->nick[i][0]) cptr = gst->nick[i]; else { sprintf(tbuf,"Player %d",i); cptr = tbuf; }
			if (strcmp(avatarname[i],cptr))
			{
				strcpy_safe(avatarname[i],TYPEMESSLENG,cptr);
				memset(&avatarsign[i][0][0],0,til.tt.p*til.tt.y);
				print6x8((tiltyp *)&til.tt,0,1,0xff000000,-1,"%s",cptr);
				print6x8((tiltyp *)&til.tt,2,1,0xff000000,-1,"%s",cptr);
				print6x8((tiltyp *)&til.tt,1,0,0xff000000,-1,"%s",cptr);
				print6x8((tiltyp *)&til.tt,1,2,0xff000000,-1,"%s",cptr);
				print6x8((tiltyp *)&til.tt,1,1,0xffffffff,-1,"%s",cptr);
			}
			k = min(strlen(avatarname[i]),til.tt.x/6)*6+2; f = ((float)k)*.006f;
			pol[3].x = pps->ipos.x; pol[3].y = pps->ipos.y; pol[3].z = pps->ipos.z;
			xformpos(&pol[3].x,&pol[3].y,&pol[3].z);
			pol[0].x = pol[3].x - f; pol[0].y = pol[3].y - .50; pol[0].z = pol[3].z;
			pol[1].x = pol[3].x + f; pol[1].y = pol[3].y - .50; pol[1].z = pol[3].z;
			pol[2].x = pol[3].x + f; pol[2].y = pol[3].y - .38; pol[2].z = pol[3].z;
			pol[3].x = pol[3].x - f; pol[3].y = pol[3].y - .38; pol[3].z = pol[3].z;
			pol[0].u = 0;                            pol[0].v =                      0; pol[0].n = 1;
			pol[1].u = ((float)k)/((float)til.tt.x); pol[1].v =                      0; pol[1].n = 1;
			pol[2].u = pol[1].u;                     pol[2].v = 10.f/((float)til.tt.y); pol[2].n = 1;
			pol[3].u = 0;                            pol[3].v =               pol[2].v; pol[3].n =-3;
			j = 0; if (gdps->compact2d >= 1.0) j |= RENDFLAGS_NODEPTHTEST;
			drawpol(cc,pol,4,&til,0x808080,0,0,j);

			if (pps->typemode)
			{
#if 0
				drawsph(cc,pps->ipos.x - pps->idow.x*.45 + pps->irig.x*.20,
							  pps->ipos.y - pps->idow.y*.45 + pps->irig.y*.20,
							  pps->ipos.z - pps->idow.z*.45 + pps->irig.z*.20,.02f,0xc0c080);
				drawsph(cc,pps->ipos.x - pps->idow.x*.35 + pps->irig.x*.20,
							  pps->ipos.y - pps->idow.y*.35 + pps->irig.y*.20,
							  pps->ipos.z - pps->idow.z*.35 + pps->irig.z*.20,.02f,0xc0c080);
				drawsph(cc,pps->ipos.x - pps->idow.x*.33 + pps->irig.x*.20,
							  pps->ipos.y - pps->idow.y*.33 + pps->irig.y*.20,
							  pps->ipos.z - pps->idow.z*.33 + pps->irig.z*.20,.02f,0xc0c080);
				drawsph(cc,pps->ipos.x - pps->idow.x*.31 + pps->irig.x*.21,
							  pps->ipos.y - pps->idow.y*.31 + pps->irig.y*.21,
							  pps->ipos.z - pps->idow.z*.31 + pps->irig.z*.21,.02f,0xc0c080);
				for(f=-PI/3;f<=PI/3;f+=PI/16)
				{
					fp.x = cos(f)*.12-.15; fp.z = sin(f)*.12-.4;
					drawsph(cc,pps->ipos.x - pps->irig.x*fp.x + pps->idow.x*fp.z,
								  pps->ipos.y - pps->irig.y*fp.x + pps->idow.y*fp.z,
								  pps->ipos.z - pps->irig.z*fp.x + pps->idow.z*fp.z,.02f,0xc0c080);
				}
#endif
				for(f=0;f<3;f++)
				{
					fp.x = (f-1)*.1; fp.z = -.30;
					drawsph(cc,pps->ipos.x - pps->irig.x*fp.x + pps->idow.x*fp.z,
								  pps->ipos.y - pps->irig.y*fp.x + pps->idow.y*fp.z,
								  pps->ipos.z - pps->irig.z*fp.x + pps->idow.z*fp.z,.02f,0xc0c080);
				}
			}
		}
	}

	if (gst->p[viewindex].grabmode == GRABFILE)
	{
		if (viewindex == moveindex) myfileselect_draw(cc);
		else print6x8(&cc->c,8,8+24,0xffffff,0,"(%s is selecting a file)",gst->nick[viewindex]);
	}

	k = cc->c.y-lineHeight;
		//Always draw your own chat text on bottom
	i = viewindex;
	if (gst->p[i].typemode)
	{
		col[0] = 0xffffff;
		k -= lineHeight;
		if (!gst->nick[i][0]) { print6x8(&cc->c,0,k,col[0],0,"Player %d: %s",i,gst->typemess[i]); j = (i>=100)+(i>=10)+10; }
							  else { print6x8(&cc->c,0,k,col[0],0,"%s: %s",gst->nick[i],gst->typemess[i]); j = strlen(gst->nick[i])+2; }
		if ((gst->p[i].typehighlight >= 0) && (gst->p[i].typehighlight != gst->p[i].typecurs))
		{
			s0 = min(gst->p[i].typecurs,gst->p[i].typehighlight);
			s1 = max(gst->p[i].typecurs,gst->p[i].typehighlight);
			print6x8(&cc->c,(s0+j)*6*fontscale,k,0,col[0],"%.*s",s1-s0,&gst->typemess[i][s0]);
		}
		if (fmod(dtotclk,0.2) < 0.12)
		{
			if (gst->p[i].typeowritemode) print6x8(&cc->c,(gst->p[i].typecurs+j)*6*fontscale,k  ,col[0],-1,"%c",219);
											 else print6x8(&cc->c,(gst->p[i].typecurs+j)*6*fontscale,k+1,col[0],-1,"_");
		}
	}

		//Draw rest of players..
	for(i=numplayers-1;i>=0;i--)
	{
		if ((i == viewindex) || (!gst->p[i].typemode)) continue;
		col[0] = 0xff40ff;
		k -= lineHeight;
		if (!gst->nick[i][0]) { print6x8(&cc->c,0,k,col[0],0,"Player %d: %s",i,gst->typemess[i]); j = (i>=100)+(i>=10)+10; }
							  else { print6x8(&cc->c,0,k,col[0],0,"%s: %s",gst->nick[i],gst->typemess[i]); j = strlen(gst->nick[i])+2; }
		if ((gst->p[i].typehighlight >= 0) && (gst->p[i].typehighlight != gst->p[i].typecurs))
		{
			s0 = min(gst->p[i].typecurs,gst->p[i].typehighlight);
			s1 = max(gst->p[i].typecurs,gst->p[i].typehighlight);
			print6x8(&cc->c,(s0+j)*6*fontscale,k,0,col[0],"%.*s",s1-s0,&gst->typemess[i][s0]);
		}
		if (fmod(dtotclk,0.2) < 0.12)
		{
			if (gst->p[i].typeowritemode) print6x8(&cc->c,(gst->p[i].typecurs+j)*6*fontscale,k,col[0],-1,"%c",219);
											 else print6x8(&cc->c,(gst->p[i].typecurs+j)*6*fontscale,k+1,col[0],-1,"_");
		}
	}

		//Draw rgb palette selection
	if (gdps->grabmode == GRABRGB)
	{
		colwheel_draw(cc,gdps->ghx    ,gdps->ghy    ,gdps->selrgbrad,gdps->selrgbintens,
							  gdps->fcmousx,gdps->fcmousy);
	}

		//Draw public messages
	for(j=gst->chatmessn-1;j>=max(gst->chatmessn-TYPEMESSNUM,0);j--)
	{
		k -= 8;
		i = gst->chatmessowner[j&(TYPEMESSNUM-1)];
		if (i == viewindex) col[0] = 0xffffff;
							else col[0] = 0xff40ff;
		if (!gst->nick[i][0])
			  print6x8(&cc->c,0,k,col[0],0,"Player %d: %s",i,gst->chatmess[j&(TYPEMESSNUM-1)]);
		else print6x8(&cc->c,0,k,col[0],0,"%s: %s",gst->nick[i],gst->chatmess[j&(TYPEMESSNUM-1)]);
	}

		//Draw rectangular highlights
	if (gps->editmode == 2)
	{
		for(i=0;i<numplayers;i++)
		{
			pps = &gst->p[i];
			if (pps->editmode != 2) continue;
			if ((pps->grabmode == GRABCORNVERT) || (pps->grabmode == GRABCORNSECT))
			{
				curs2grid(pps,pps->fcmousx,pps->fcmousy,&fp);

					  if (pps->grabmode == GRABCORNVERT) j = ((long)(sin(dtotclk*16.0)*16.0+128))*0x10101 + 0x80000000;
				else if (pps->grabmode == GRABCORNSECT) j = ((long)(sin(dtotclk*16.0)*16.0+128))*0x10001 + 0x8000c000;
				drawline3d(cc,pps->selcorn.x,pps->selcorn.y,0,fp.x          ,pps->selcorn.y,0,j);
				drawline3d(cc,fp.x          ,pps->selcorn.y,0,fp.x          ,fp.y          ,0,j);
				drawline3d(cc,fp.x          ,fp.y          ,0,pps->selcorn.x,fp.y          ,0,j);
				drawline3d(cc,pps->selcorn.x,fp.y          ,0,pps->selcorn.x,pps->selcorn.y,0,j);
			}
		}
	}

	if (didpolybool)
	{
		if (gdps->showdebug)
		{
			s = 2; wal = sec[s].wall;
			for(w=0;w<sec[s].n;w++)
			{
				pol[0].x = wal[w].x; pol[0].y = wal[w].y; pol[0].z = 0;
				xformpos(&pol[0].x,&pol[0].y,&pol[0].z); if (pol[0].z < SCISDIST) continue;
				f = gdps->ghz/pol[0].z; print6x8(&cc->c,pol[0].x*f+gdps->ghx-3,pol[0].y*f+gdps->ghy-4,0xc0c0c0,0,"%d",w);
			}
		}
		free(sec[2].wall); sec[2].wall = 0; sec[2].n = sec[2].nmax = 0;
	}

	if ((fabs(gdps->grdn.z) == 1.f) && (gdps->editmode == 2) && (gdps->grdn.z < 0))
		print6x8(&cc->c,(cc->c.x>>1)-11*3,24,0xffffff,0,"Bottom View");

#if (OOS_CHECK != 0)
	if (dispoos)
	{
		g = fmod(dtotclk,PI*2);
		for(f=.38;f<256;f++)
			print6x8(&cc->c,cos(f*(PI*2*(sqrt(5.0)-1.0)/2.0)+g)*sqrt(f)*36+(cc->c.x>>1)-5*3,
								 sin(f*(PI*2*(sqrt(5.0)-1.0)/2.0)+g)*sqrt(f)*27+(cc->c.y>>1)-4,0xffffff,0,"OOS:/");

	}
#endif
#endif
}

#ifdef STANDALONE
static int bytessent = 0, bytesrecv = 0;
static int recvleng[MAXPLAYERS] = {0};
static unsigned char recvbuf[MAXPLAYERS][256];
static void netcheck (void)
{
	int i, j, p, q, p0, p1, updatesync;

	if (!moveindex) { p0 = 1; p1 = numplayers; } else { p0 = 0; p1 = 1; }

	updatesync = 0;
	for(p=p0;p<p1;p++)
	{
		while (1)
		{
			if (!recvleng[p]) i = 1;
			else
			{
				i = ((int)recvbuf[p][0])-recvleng[p]; //do not span packet boundaries
#if (OOS_CHECK != 0)
				if (!p) i += 4;
#endif
			}
			i = net_read(gsk[p],(char *)&recvbuf[p][recvleng[p]],i); if (!i) break;
			recvleng[p] += i;
			bytesrecv += i; //for debugging only
#if (OOS_CHECK == 0)
			if (recvleng[p] >= recvbuf[p][0])
#else
			if (recvleng[p] >= recvbuf[p][0]+((!p)<<2))
#endif
			{
				if (!moveindex)
				{
#if (OOS_CHECK != 0)
					*(long *)&recvbuf[p][recvleng[p]] = gamestate_crc32(&sst); recvleng[p] += 4;
#endif
					for(q=1;q<numplayers;q++)
					{
						net_write(gsk[q],(char *)&recvbuf[p],recvleng[p]); //relay to slaves
						bytessent += recvleng[p]; //for debugging only
					}
				}
				else
				{
					if (recvbuf[p][1] == moveindex)
					{
						ppackfifr += recvleng[p];
#if (OOS_CHECK != 0)
						ppackfifr -= 4;
#endif
						ppackfifrn++;
					} else updatesync = 1;
#if (OOS_CHECK != 0)
					if (gamestate_crc32(&sst) != *(long *)&recvbuf[p][recvbuf[p][0]])
					{
						if (!dispoos)
						{
							char tbuf[256];
							tbuf[0] = 5;
							tbuf[1] = (unsigned char)moveindex;
							tbuf[2] = 128; //bit fields
							tbuf[3] = 0; //tics
							tbuf[4] = 5; //Command 5: Send OOS message :/
							j = 5;
								//Copy to local command buffer (can't send in the middle of executepack)
							memcpy(&cmdpackfif[cmdpackfifw&65535],tbuf,j); cmdpackfifw += j;

							playtext("<rate speed='+0'/>Out of<pitch middle='-5'/>sync");
							dispoos = 1;

								//Send Ctrl+'`' to automatically save GST file as soon as possible
							tbuf[0] = 7;
							tbuf[1] = (unsigned char)moveindex;
							tbuf[2] = 0; //bit fields
							tbuf[3] = 0; //tics
							tbuf[4] = 0x00; //Ctrl+'`' Save gamestate (for debug)
							tbuf[5] = 0x29;
							tbuf[6] = 0x0c;
							j = 7;
							memcpy(&cmdpackfif[cmdpackfifw&65535],tbuf,j); cmdpackfifw += j;
						}
					}
#endif
				}

				gst = &sst;
				executepack(recvbuf[p],(recvbuf[p][1] != moveindex));
				recvleng[p] = 0;
			}
		}
	}
	if (updatesync)
	{
		build2_copygamestate(&pst,&sst); gst = &pst;
		if (ppackfifw-ppackfifr < 65536) //FIX: if this is false, it would be bad (a lot of missed packets!)
			for(i=ppackfifr;i<ppackfifw;i+=(int)ppackfif[i&65535])
				executepack(&ppackfif[i&65535],0);
	}
}

static float fcmousx = 0.f, fcmousy = 0.f, fcmousz = 0.f;

static void getnsendinput (void)
{
	extern long ActiveApp;
	playerstruct_t *lps;
	static long oltotclk = 0, obstatus = 0, oxres = 0, oyres = 0, ofullscreen = 0;
	static char okeystatus[256];
	double d;
	float fx, fy, fz;
	long bstatus;
	int i, j, k, fields, dt, mx, my, mz, ltotclk;
	unsigned char tbuf[256+4], *uptr;

	if (cmdpackfifw > cmdpackfifr) //Send off any local commands
	{
		j = (long)cmdpackfif[cmdpackfifr&65535];
		memcpy(tbuf,&cmdpackfif[cmdpackfifr&65535],j); cmdpackfifr += j;
		if (!moveindex) { gst = &sst; }
					  else { gst = &pst; memcpy(&ppackfif[ppackfifw&65535],tbuf,j); ppackfifw += j; ppackfifwn++; }
		if (numplayers >= 2)
		{
#if (OOS_CHECK != 0)
			if (!moveindex) { *(long *)&tbuf[j] = gamestate_crc32(&sst); j += 4; }
#endif
			if (!moveindex) { for(i=1;i<numplayers;i++) net_write(gsk[i],(char *)tbuf,j); }
						  else { net_write(gsk[0],(char *)tbuf,j); }
		}
		executepack((unsigned char *)tbuf,1);
		bytessent += j; //for debugging only
	}

	readklock(&d);
	ltotclk = (int)(d*1024.0);
	//if (labs(ltotclk-oltotclk) <= 5) return; //limit bandwidth (not necessary)

	if (!moveindex) lps = &sst.p[0]; else lps = &pst.p[moveindex];
	if (ActiveApp)
	{
		if (lps->grabmode == GRABFILE)
		{
			static POINT op0;
			POINT p0;
			if (mouseacquired) { mouseacquired = 0; setacquire(0,1); ext_mwheel = 0; ext_mbstatus[0] = ((obstatus&1)<<1); ext_mbstatus[1] = (obstatus&2); }
			GetCursorPos(&p0);
			ScreenToClient(ghwnd,&p0);
			fcmousx = 0; fcmousy = 0; fcmousz = 0;
			bstatus = ((ext_mbstatus[0]&2)>>1) + (ext_mbstatus[1]&2);
			if ((p0.x-op0.x)*(p0.x-op0.x) + (p0.y-op0.y)*(p0.y-op0.y) >= 2*2)
			{
				op0 = p0;
				mx = (p0.x-(      menupicoffx  ))/(menupicsiz+ 0);
				my = (p0.y-((long)menupicoffy-8))/(menupicsiz+10);
				if ((p0.x >= menupicoffx) && (p0.y >= (long)menupicoffy-8) && ((unsigned)mx < (unsigned)menupicperline))
				{
					i = my*menupicperline + mx;
					if ((unsigned)i < (unsigned)menunamecnt) menuhighlight = i;
				}
			}
			if (bstatus&~obstatus&1) myfileselect_input(lps,0x1c00); //LMB simulates enter key
		}
		else if ((lps->editmode == 3) || (lps->grabmode == GRABRGB))
		{
			if (!mouseacquired) { mouseacquired = 1; setacquire(1,1); }
			readmouse(&fx,&fy,&fz,&bstatus);
			fcmousx += fx; fcmousy += fy; fcmousz += fz;
		}
		else
		{
			POINT p0;
			if (mouseacquired) { mouseacquired = 0; setacquire(0,1); ext_mwheel = 0; ext_mbstatus[0] = ((obstatus&1)<<1); ext_mbstatus[1] = (obstatus&2); }
			GetCursorPos(&p0);
			ScreenToClient(ghwnd,&p0);
			fcmousx = p0.x-lps->fcmousx;
			fcmousy = p0.y-lps->fcmousy;
			fcmousz = ext_mwheel; ext_mwheel = 0;
			bstatus = ((ext_mbstatus[0]&2)>>1) + (ext_mbstatus[1]&2);
		}

		readkeyboard();
	}
	else
	{
		bstatus = 0;
		memset(keystatus,0,sizeof(keystatus));
	}

	dt = min(ltotclk-oltotclk,65535); oltotclk = ltotclk;
	mx = (int)min(max(fcmousx,-32768),32767); fcmousx -= (float)mx; //be sure mx,my,mz are rounded towards 0
	my = (int)min(max(fcmousy,-32768),32767); fcmousy -= (float)my;
	mz = (int)min(max(fcmousz,-32768),32767); fcmousz -= (float)mz;

		//Bit fields byte (packet compression)
		//  &1: tics hi
		//  &2: mousx lo     &4: mousx hi
		//  &8: mousy lo    &16: mousy hi
		// &32: mousz lo&hi
		// &64: keystatus xor list (0 to terminate if exists)
		//&128: xres,yres,fullscreen

	j = 3; fields = 0; //tbuf[0 to 2] written later
	tbuf[j++] = (dt&255); if (dt&0xff00) { tbuf[j] = ((dt>>8)&255); j++; fields |=  1; }
	if (mx)                              { tbuf[j] = ( mx    &255); j++; fields |=  2; }
	if ((mx < -128) || (mx >= 128))      { tbuf[j] = ((mx>>8)&255); j++; fields |=  4; }
	if (my)                              { tbuf[j] = ( my    &255); j++; fields |=  8; }
	if ((my < -128) || (my >= 128))      { tbuf[j] = ((my>>8)&255); j++; fields |= 16; }
	if (mz)                    { *(short *)&tbuf[j] = (short)mz; j += 2; fields |= 32; }
	for(i=NUMOUSBUTS-1;i>=0;i--)
		if ((bstatus^obstatus)&(1<<i)) { tbuf[j++] = 0xf0+i; fields |= 64; }
	obstatus = bstatus;

	for(k=sizeof(keyscan2bit)/sizeof(keyscan2bit[0])-1;k>=0;k--)
	{
		i = keyscan2bit[k]; if ((keystatus[i] != 0) == okeystatus[i]) continue;
		okeystatus[i] ^= 1; tbuf[j++] = keyscanremap[i]; fields |= 64;
	}
	if (fields&64) tbuf[j++] = 0;
	if ((xres != oxres) || (yres != oyres) || (fullscreen != ofullscreen))
	{
		oxres = xres; oyres = yres; ofullscreen = fullscreen; fields |= 128;
		*(char *)&tbuf[j] = (fullscreen != 0); //Command 0 & Command 1: Change resolution
		*(short *)&tbuf[j+1] = (short)xres;
		*(short *)&tbuf[j+3] = (short)yres; j += 5;
	}
	while (i = keyread())
	{
		if (lps->grabmode == GRABFILE) { myfileselect_input(lps,i); continue; }

		if (!lps->typemode) //FIXFIXFIX
			{ i = (i&0xffff00ff) + (((long)keyscanremap[(i>>8)&255])<<8); }

		*(long *)&tbuf[j] = i; j += 3;

			//HACK! Paste from clipboard from local machine. Must be here for fast response & no duplicates
		if ((((i>>8)&255) == 0xd2) && (i&0x30000)) //Shift+Insert
		{
				//Paste from clipboard to local command buffer
			uptr = &cmdpackfif[cmdpackfifw&65535];
			getclipboardtext((char *)&uptr[4],(255-4)/3);
			uptr[0] = strlen((char *)&uptr[4])*3+4; cmdpackfifw += uptr[0];
			uptr[1] = (unsigned char)moveindex;
			uptr[2] = 0; //bit fields
			uptr[3] = 0; //tics
			for(i=strlen((char *)&uptr[4])-1;i>=0;i--) //Simulate keyread() messages for typemode
				{ uptr[i*3+6] = 0; uptr[i*3+5] = 0; uptr[i*3+4] = uptr[i+4]; }
		}

		if (j >= 252) break; //packet length must be < 256
	}

	tbuf[0] = (unsigned char)j;
	tbuf[1] = (unsigned char)moveindex;
	tbuf[2] = (unsigned char)fields;
	if (j == 4) { j = 3; tbuf[0]--; tbuf[2] = tbuf[3]; } //Hack packet to be smaller if 'fields' is 0

	//for(i=0;i<tbuf[0];i++) printf("%02x ",tbuf[i]); printf("\n"); //FIX

	if (!moveindex) { gst = &sst; }
				  else { gst = &pst; memcpy(&ppackfif[ppackfifw&65535],tbuf,j); ppackfifw += j; ppackfifwn++; }
	if (numplayers >= 2)
	{
#if (OOS_CHECK != 0)
		if (!moveindex) { *(long *)&tbuf[j] = gamestate_crc32(&sst); j += 4; }
#endif
		if (!moveindex) { for(i=1;i<numplayers;i++) net_write(gsk[i],(char *)tbuf,j); }
					  else { net_write(gsk[0],(char *)tbuf,j); }
	}
	executepack((unsigned char *)tbuf,1);
	bytessent += j; //for debugging only
}
#endif

void build2_uninit (void)
{
	int i;

#ifdef STANDALONE
	if (moveindex) gst = &pst; else //Predicted state has all of synced state's textures
#endif
	gst = &sst;
	if (gtile)
	{
		for(i=gnumtiles-1;i>=0;i--)
			if (gtile[i].tt.f) free((void *)gtile[i].tt.f);
		free(gtile); gtile = 0;
	}
	gnumtiles = gmaltiles = 0; memset(gtilehashead,-1,sizeof(gtilehashead));

#ifdef STANDALONE
	if (moveindex) freegamestate(&pst); else
#endif
	freegamestate(&pst);
	freegamestate(&sst);

	shadowtest2_uninit();

	if (zbuffermem) { free(zbuffermem); zbuffermem = 0; zbuffersiz = 0; }
}

#ifndef STANDALONE

long build2_init ()
{
	long i;

	cputype = getcputype();

	numplayers = 1; moveindex = viewindex = 0;

	initcrc32();

	gnumtiles = 0; memset(gtilehashead,-1,sizeof(gtilehashead));
	gmaltiles = 256;
	gtile = (tile_t *)malloc(gmaltiles*sizeof(tile_t)); if (!gtile) return(-1);
	//memset(gtile,0,gmaltiles*sizeof(tile_t)); //FIX

	gst = &sst;
	gst->numsects = 0;
	gst->malsects = 256;
	gst->sect = (sect_t *)malloc(gst->malsects*sizeof(sect_t)); if (!gst->sect) return(-1);
	memset(gst->sect,0,gst->malsects*sizeof(sect_t));

	gst->numspris = 0;
	gst->malspris = 256;
	gst->spri = (spri_t *)malloc(gst->malspris*sizeof(spri_t)); if (!gst->spri) return(-1);
	memset(gst->spri,0,gst->malspris*sizeof(spri_t));

	gst->blankheadspri = -1;
	for(i=0;i<gst->malspris;i++)
	{
		gst->spri[i].sectn = gst->blankheadspri;
		gst->spri[i].sectp = -1;
		gst->spri[i].sect = -1;
		if (gst->blankheadspri >= 0) gst->spri[gst->blankheadspri].sectp = i;
		gst->blankheadspri = i;
	}

	build2.gammaval = 1.0; //1.0=no change, useful range={0.0..~4.0)
	setgammlut(build2.gammaval); //Safe here because gst exists and gnumtiles = 0

	drawpolfunc = drawpol;
	gettileind("tex\\cloud.png"); //force it to be index 0

#if USEGROU
	{
	SYSTEM_INFO si; //1 cpu system runs faster with 1 thread.. this helps if people copy the cfg file
	GetSystemInfo(&si);
	b2opts.drawthreads = min(b2opts.drawthreads,si.dwNumberOfProcessors);
	}
		//Note: loadcfg (b2opts) is not inited in evaldraw!
	if (b2opts.anginc     ) drawpoly_anginc = b2opts.anginc;
	if (b2opts.drawthreads) drawpoly_numcpu = b2opts.drawthreads;
	drawpoly_init();
#endif

	shadowtest2_init();
	gst->light_sprinum = 0;

	drawkv6_init();

	gst->startpos.x = 0.0; gst->startpos.y = 0.0; gst->startpos.z = 0.0;
	gst->startrig.x = 1.0; gst->startrig.y = 0.0; gst->startrig.z = 0.0;
	gst->startdow.x = 0.0; gst->startdow.y = 0.0; gst->startdow.z = 1.0;
	gst->startfor.x = 0.0; gst->startfor.y =-1.0; gst->startfor.z = 0.0;
	for(i=0;i<numplayers;i++)
	{
		gps = &gst->p[i];
		memset(gps,0,sizeof(playerstruct_t));
#ifdef STANDALONE
		gps->fcmousx = xres/2;
		gps->fcmousy = yres/2;
		gps->grabsect = -1; gps->grabwall = -1; gps->grabmode = -1; gps->grabcf = 0;
		gps->playerindex = i;
		gps->circnum = 7;
		gps->docollide = 1;
		gps->gridlock = 1;
		gps->boolfunc = POLYBOOL_END;
		gps->showdebug = 0;
		gps->showedges3d = 1;
#endif
		gps->dgridlock = 1.0/8.0;
		gps->compact2d = 1.0;
		gps->goalheight2d = 24.0;
		gps->editmode = 3;
		gps->cursect = -1;
		gps->rendheight = 0;
		gps->rendinterp = 0;

		gps->ipos.x = 0; gps->ipos.y = 0; gps->ipos.z = -gps->goalheight2d;
		gps->irig.x = 1; gps->irig.y = 0; gps->irig.z = 0;
		gps->idow.x = 0; gps->idow.y = 1; gps->idow.z = 0;
		gps->ifor.x = 0; gps->ifor.y = 0; gps->ifor.z = 1;

		gps->grdc.x = 0; gps->grdc.y = 0; gps->grdc.z = 0; //center
		gps->grdu.x = 1; gps->grdu.y = 0; gps->grdu.z = 0;
		gps->grdv.x = 0; gps->grdv.y = 1; gps->grdv.z = 0;
		gps->grdn.x = 0; gps->grdn.y = 0; gps->grdn.z = 1; //normal

		gps->ghx = xres/2;
		gps->ghy = yres/2;
		gps->ghz = gps->ghx;
		gps->zoom = gps->ozoom = 1.f;

		//memset(&gps->sec,0,sizeof(sect_t));
	}

	gps->compact2d = 0.0;

	build2.fattestsprite = 2.0;

	//build2_copygamestate(&pst,&sst);
	memcpy(&pst,&sst,sizeof(gamestate_t));
	pst.sect = (sect_t *)malloc(pst.malsects*sizeof(sect_t)); if (!pst.sect) return(-1);
	pst.spri = (spri_t *)malloc(pst.malspris*sizeof(spri_t)); if (!pst.spri) return(-1);
	memset(pst.sect,0,pst.malsects*sizeof(sect_t));
	memset(pst.spri,0,pst.malspris*sizeof(spri_t));

	return(0);
}

int build2_loadmap (const char *filnam, int *cursect,
						  dpoint3d *npos, dpoint3d *nrig, dpoint3d *ndow, dpoint3d *nfor)
{
	gst = &sst;
	if (!loadmap((char *)filnam)) return(0);
	npos->x = gst->startpos.x; npos->y = gst->startpos.y; npos->z = gst->startpos.z;
	nrig->x = gst->startrig.x; nrig->y = gst->startrig.y; nrig->z = gst->startrig.z;
	ndow->x = gst->startdow.x; ndow->y = gst->startdow.y; ndow->z = gst->startdow.z;
	nfor->x = gst->startfor.x; nfor->y = gst->startfor.y; nfor->z = gst->startfor.z;
	updatesect(npos->x,npos->y,npos->z,(int *)cursect);
	return(1);
}

	//WARNING: Don't use this function for STANDALONE editor - would screw multi sync!
void build2_render (tiletype *dd, long lzbufoff, long cursect,
						  dpoint3d *npos, dpoint3d *nrig, dpoint3d *ndow, dpoint3d *nfor,
						  double hx, double hy, double hz)
{
	cam_t cam;
	dpoint3d dpos, drig, ddow, dfor;
	long i, j, k, m;

	gdps = &gst->p[0]; //Must set before xformpos! (used by drawsprite)

	if (!lzbufoff)
	{
		i = dd->y*dd->p+256;
		if ((i > zbuffersiz) || (!zbuffermem))  //Increase Z buffer size if too small
		{
			if (zbuffermem) { free(zbuffermem); zbuffermem = 0; }
			zbuffersiz = i;
			zbuffermem = (long *)malloc(zbuffersiz);
		}
			//zbuffer aligns its memory to the same pixel boundaries as the screen!
			//WARNING: Pentium 4's L2 cache has severe slowdowns when 65536-64 <= (zbufoff&65535) < 64
		lzbufoff = (((((long)zbuffermem)-dd->f-128)+255)&~255)+128;
		if ((gst->light_sprinum <= 0) && (gdps->cursect < 0))
		for(i=0,j=dd->f+lzbufoff;i<dd->y;i++,j+=dd->p) memset8((void *)j,0x7f7f7f7f,dd->x<<2);
	}
	else if (gdps->cursect < 0) { for(i=0,j=dd->f+lzbufoff;i<dd->y;i++,j+=dd->p) memset8((void *)j,0x7f7f7f7f,dd->x<<2); }

	cam.h.x = hx; cam.h.y = hy; cam.h.z = hz;
	cam.c.f = dd->f; cam.z.f = dd->f+lzbufoff;
	cam.c.p = cam.z.p = dd->p;
	cam.c.x = cam.z.x = dd->x;
	cam.c.y = cam.z.y = dd->y;

	dpos.x = 0.0; dpos.y = 0.0; dpos.z = 0.0;
	drig.x = 1.0; drig.y = 0.0; drig.z = 0.0;
	ddow.x = 0.0; ddow.y = 1.0; ddow.z = 0.0;
	dfor.x = 0.0; dfor.y = 0.0; dfor.z = 1.0;
	drawpoly_setup(                           (tiletype *)&cam.c.f,lzbufoff,&dpos,&drig,&ddow,&dfor,cam.h.x,cam.h.y,cam.h.z);
	drawcone_setup(cputype,shadowtest2_numcpu,(tiletype *)&cam.c.f,lzbufoff,&dpos,&drig,&ddow,&dfor,cam.h.x,cam.h.y,cam.h.z);
	 drawkv6_setup(&drawkv6_frame,            (tiletype *)&cam.c.f,lzbufoff,&dpos,&drig,&ddow,&dfor,cam.h.x,cam.h.y,cam.h.z);

	gst->p[0].ipos.x = npos->x; gst->p[0].ipos.y = npos->y; gst->p[0].ipos.z = npos->z;
	gst->p[0].irig.x = nrig->x; gst->p[0].irig.y = nrig->y; gst->p[0].irig.z = nrig->z;
	gst->p[0].idow.x = ndow->x; gst->p[0].idow.y = ndow->y; gst->p[0].idow.z = ndow->z;
	gst->p[0].ifor.x = nfor->x; gst->p[0].ifor.y = nfor->y; gst->p[0].ifor.z = nfor->z;
	gst->p[0].cursect = cursect;
	gst->p[0].ghx = hx; gst->p[0].ghy = hy; gst->p[0].ghz = hz; //FIX: Why is isvispol() using gdps instead of cam?
	gst->p[0].editmode = 3.0; gst->p[0].compact2d = 0.0;

	if ((gst->light_sprinum > 0) && (gdps->cursect >= 0))
	{
		cam.r.x = nrig->x; cam.r.y = nrig->y; cam.r.z = nrig->z;
		cam.d.x = ndow->x; cam.d.y = ndow->y; cam.d.z = ndow->z;
		cam.f.x = nfor->x; cam.f.y = nfor->y; cam.f.z = nfor->z;
		cam.p.x = npos->x; cam.p.y = npos->y; cam.p.z = npos->z;

		//shadowtest2_useshadows = 1;

		j = 0;
		for(i=gst->light_sprinum-1;i>=0;i--)
		{
			if (((unsigned)gst->light_spri[i] >= (unsigned)gst->malspris) || (gst->spri[gst->light_spri[i]].sect < 0)) continue;
			shadowtest2_light[j].sect   = gst->spri[gst->light_spri[i]].sect;
			shadowtest2_light[j].p      = gst->spri[gst->light_spri[i]].p;
			k = ((gst->spri[gst->light_spri[i]].flags>>17)&7);
			if (!k) { shadowtest2_light[j].spotwid = -1.0; }
			else
			{
				m = ((gst->spri[gst->light_spri[i]].flags>>20)&1023); if (!m) continue;
				shadowtest2_light[j].spotwid = cos(m*PI/1024.0); //FIX:use lut
				switch(k)
				{
					case 1: case 2: shadowtest2_light[j].f = gst->spri[gst->light_spri[i]].d; break;
					case 3: case 4: shadowtest2_light[j].f = gst->spri[gst->light_spri[i]].f; break;
					case 5: case 6: shadowtest2_light[j].f = gst->spri[gst->light_spri[i]].r; break;
				}
				if (!(k&1)) { shadowtest2_light[j].f.x *= -1; shadowtest2_light[j].f.y *= -1; shadowtest2_light[j].f.z *= -1; }
			}
			shadowtest2_light[j].rgb[0] = gst->spri[gst->light_spri[i]].bsc/8192.0; //((float)gst->spri[gst->light_spri[i]].gsc)/8192.0; //gst->spri[gst->light_spri[i]].fat;
			shadowtest2_light[j].rgb[1] = gst->spri[gst->light_spri[i]].gsc/8192.0;
			shadowtest2_light[j].rgb[2] = gst->spri[gst->light_spri[i]].rsc/8192.0;
			shadowtest2_light[j].flags  = 1;
			drawkv6_light[j].sect      = shadowtest2_light[j].sect;
			drawkv6_light[j].p         = shadowtest2_light[j].p;
			drawkv6_light[j].rgb[0]    = shadowtest2_light[j].rgb[0];
			drawkv6_light[j].rgb[1]    = shadowtest2_light[j].rgb[1];
			drawkv6_light[j].rgb[2]    = shadowtest2_light[j].rgb[2];
			drawkv6_light[j].useshadow =(shadowtest2_light[j].flags&1);
			xformpos(&drawkv6_light[j].p.x,&drawkv6_light[j].p.y,&drawkv6_light[j].p.z);
			j++;
		}
		shadowtest2_numlights = j; drawkv6_numlights = j;
		drawkv6_ambrgb[0] = shadowtest2_ambrgb[0];
		drawkv6_ambrgb[1] = shadowtest2_ambrgb[1];
		drawkv6_ambrgb[2] = shadowtest2_ambrgb[2];

		shadowtest2_rendmode = 2; draw_hsr_polymost(&cam,gst,&gst->p[0],gst->p[0].cursect); shadowtest2_rendmode = 4;
		if (shadowtest2_updatelighting) //FIXFIX
		{
			cam_t ncam; ncam = cam;
			shadowtest2_updatelighting = 0; //FIXFIX
			shadowtest2_ligpolreset(-1);
			for(glignum=0;glignum<shadowtest2_numlights;glignum++)
			{
				ncam.p = shadowtest2_light[glignum].p;
				draw_hsr_polymost(&ncam,gst,&gst->p[0],shadowtest2_light[glignum].sect);
			}
			shadowtest2_setcam(&cam);
		}
		htrun(drawpollig,0,eyepoln,shadowtest2_numcpu);
		drawsprites();
		//drawview(&cam,&gst->p[0],1);

		drawkv6_numlights = -1;
	}
	else
	{
		if (gdps->cursect >= 0)
		{
			cam.r.x = nrig->x; cam.r.y = nrig->y; cam.r.z = nrig->z;
			cam.d.x = ndow->x; cam.d.y = ndow->y; cam.d.z = ndow->z;
			cam.f.x = nfor->x; cam.f.y = nfor->y; cam.f.z = nfor->z;
			cam.p.x = npos->x; cam.p.y = npos->y; cam.p.z = npos->z;
			shadowtest2_numlights = 0; drawkv6_numlights = -1;
			drawkv6_ambrgb[0] = shadowtest2_ambrgb[0];
			drawkv6_ambrgb[1] = shadowtest2_ambrgb[1];
			drawkv6_ambrgb[2] = shadowtest2_ambrgb[2];
			shadowtest2_rendmode = 2; draw_hsr_polymost(&cam,gst,&gst->p[0],gst->p[0].cursect);
			if (eyepoln) htrun(eyepol_drawfunc,0,eyepoln,shadowtest2_numcpu);
			drawsprites();
			return;
		}
		cam.r.x = 1.f; cam.r.y = 0.f; cam.r.z = 0.f;
		cam.d.x = 0.f; cam.d.y = 1.f; cam.d.z = 0.f;
		cam.f.x = 0.f; cam.f.y = 0.f; cam.f.z = 1.f;
		cam.p.x = 0.f; cam.p.y = 0.f; cam.p.z = 0.f;
		drawkv6_numlights = -1;
		drawview(&cam,&gst->p[0],0);
	}
}

int build2_hitmove (int *cursect, dpoint3d *p, dpoint3d *v, double cr, int isslide, int *hitsect, int *hitwall)
{
	point3d fp, fv, hit;
	long i; int dummy[2];

	if (!hitsect) hitsect = &dummy[0];
	if (!hitwall) hitwall = &dummy[1];
	(*hitsect) = -1; (*hitwall) = -1;
	if (cr == 0.0)
	{
		fp.x = p->x; fp.y = p->y; fp.z = p->z;
		fv.x = v->x; fv.y = v->y; fv.z = v->z;
		i = hitscan(&fp,&fv,1.0,*cursect,hitsect,hitwall,&hit);
		if ((*hitsect) == -1) updatesect(hit.x,hit.y,hit.z,(int *)hitsect);
		p->x = hit.x; p->y = hit.y; p->z = hit.z; (*cursect) = (long)(*hitsect);
		return(i);
	}
	collmove(p,(int *)cursect,v,cr,isslide);
	if (build2.cliphitnum > 0) { (*hitsect) = build2.clipsect[0]; (*hitwall) = build2.clipwall[0]; }
	updatesect(p->x,p->y,p->z,(int *)cursect);
	return(build2.cliphitnum > 0);
}

#endif
#ifdef STANDALONE

void uninitapp ()
{
	int i;

	if ((showhelpbuf != 0) && (showhelpbuf != (char *)-1)) { free(showhelpbuf); showhelpbuf = 0; }

	if (memcmp(&ob2opts,&b2opts,sizeof(ob2opts)) || memcmp(okeyscanremap,keyscanremap,sizeof(okeyscanremap))) savecfg();

	for(i=sizeof(tilelist)/sizeof(tilelist[0])-1;i>=0;i--)
		if (tilelist[i].t.f) { free((void *)tilelist[i].t.f); tilelist[i].t.f = 0; }

	build2_uninit();
	drawkv6_freeall();

	if (ghcrosscurs) { DestroyCursor(ghcrosscurs); ghcrosscurs = 0; }
	if (ghblankcurs) { DestroyCursor(ghblankcurs); ghblankcurs = 0; }

	if (numplayers >= 2)
	{
		if (!moveindex) { for(i=1;i<numplayers;i++) net_close(gsk[i]); } else net_close(gsk[0]);
	}
	if (zbuffermem) { free(zbuffermem); zbuffermem = 0; zbuffersiz = 0; }
}

long initapp (long argc, char **argv)
{
	int i, j, k, l, x, y, argnoslash[8], argnoslashcnt = 0, setres = 0, joinarg = -1, waitplayers = 0, hostportarg = -1;
	char tbuf[max(MAX_PATH,256)];

	GetModuleFileName(0,basepath,sizeof(basepath));
	for(i=j=0;basepath[i];i++)
		if ((basepath[i] == '\\') || (basepath[i] == '/')) j = i;
	basepath[j] = 0;
	SetCurrentDirectory(basepath);

	loadcfg(); memcpy(&ob2opts,&b2opts,sizeof(ob2opts));

	xres = b2opts.xres; yres = b2opts.yres; colbits = 32; fullscreen = b2opts.fullscreen;
	prognam = "BUILD 2.0 by Ken Silverman";

	for(i=MAXPLAYERS-1;i>=0;i--) gsk[i] = (SOCKET)INVALID_SOCKET;

	for(i=1;i<argc;i++)
	{
		if ((argv[i][0] != '/') && (argv[i][0] != '-'))
		{
			if (argnoslashcnt < sizeof(argnoslash)/sizeof(argnoslash[0])) argnoslash[argnoslashcnt++] = i;
			continue;
		}
		if (!stricmp(&argv[i][1],"full")) { setres = 1; fullscreen = 1; xres = 1024; yres = 768; continue; }
		if (!stricmp(&argv[i][1],"win")) { fullscreen = 0; continue; }
		if (!stricmp(&argv[i][1],"udp")) { netproto = 0; continue; }
		if (!stricmp(&argv[i][1],"tcp")) { netproto = 1; continue; }
		if (!memicmp(&argv[i][1],"host:",5)) { joinarg = 0; waitplayers = min(max(atol(&argv[i][6]),2),MAXPLAYERS); continue; }
		if (!stricmp(&argv[i][1],"host")) { joinarg = 0; waitplayers = 2; continue; }
		if (!memicmp(&argv[i][1],"port:",5)) { hostportarg = i; continue; }
		if (!memicmp(&argv[i][1],"join:",5)) { joinarg = i; continue; }
		if (!stricmp(&argv[i][1],"join")) { joinarg = 0x40000000; continue; }
		if (!memicmp(&argv[i][1],"simlag:",7)){simlagms = min(max(strtol(&argv[i][8],0,0),0),5000); continue; }
		if (!stricmp(&argv[i][1],"simlag"))  { simlagms = 250; continue; }
		if (!memicmp(&argv[i][1],"log:",4)) { strcpy_safe(logfilnam,sizeof(logfilnam),&argv[i][5]); continue; }
		if (!stricmp(&argv[i][1],"log")) { strcpy(logfilnam,"build2.log"); continue; }
		if ((!memicmp(&argv[i][1],"nick:",5)) || (!memicmp(&argv[i][1],"name:",5)))
		{
			if (argv[i][6] == '\"') //Strip quotes
				  { strcpy_safe(b2opts.nick,sizeof(b2opts.nick),&argv[i][7]); b2opts.nick[max(strlen(b2opts.nick)-1,0)] = 0; }
			else { strcpy_safe(b2opts.nick,sizeof(b2opts.nick),&argv[i][6]); }
			continue;
		}
		if (argv[i][1] == '?')
		{
			MessageBox(0,"build2 [?.map] [/host(:#)] [/join(:ip)] [/log(:file)] [/full] [/win] [/#x#(x#)] [/?]",prognam,MB_OK);
			return(-1);
		}
		if ((argv[i][1] >= '0') && (argv[i][1] <= '9'))
		{
			setres = 1; k = 0; l = 0;
			for(j=1;;j++)
			{
				if ((argv[i][j] >= '0') && (argv[i][j] <= '9')) { k = (k*10+argv[i][j]-48); continue; }
				switch (l)
				{
					case 0: xres = k; break;
					case 1: yres = k; break;
					case 2: fullscreen = 1; break;
				}
				if (!argv[i][j]) break;
				l++; if (l > 2) break;
				k = 0;
			}
		}
	}

	b2opts.xres = xres; b2opts.yres = yres; b2opts.fullscreen = fullscreen;

	cputype = getcputype();

	//mousmoth = 0;
	extern long alwaysactive; alwaysactive = 1;

	numplayers = 1; moveindex = viewindex = 0;

	if (simlagms)
	{
		simlagbuf = (char *)malloc(MAXPLAYERS*(SIMLAGSIZ+SIMLAGOFLOW)*sizeof(simlagbuf[0]));
		simlagtim = (long *)malloc(MAXPLAYERS* SIMLAGSIZ             *sizeof(simlagtim[0]));
	}

	if (joinarg >= 0)
	{
		AllocConsole(); HANDLE hconsout = GetStdHandle(STD_OUTPUT_HANDLE);

		if (!joinarg)
		{
			sprintf(tbuf,"BUILD2: waiting for slaves to connect..\n"); WriteConsole(hconsout,tbuf,strlen(tbuf),(unsigned long *)&j,0);
			if (hostportarg < 0) gsk[0] = net_open("");
								 else gsk[0] = net_open(&argv[hostportarg][5]);
		}
		else
		{
			sprintf(tbuf,"BUILD2: waiting for master to collect players..\n"); WriteConsole(hconsout,tbuf,strlen(tbuf),(unsigned long *)&j,0);
			if (joinarg == 0x40000000) gsk[0] = net_open("localhost");
										 else gsk[0] = net_open(&argv[joinarg][6]);
		}
		if (gsk[0] == INVALID_SOCKET) { MessageBox(ghwnd,"net_open failed",prognam,MB_OK); return(-1); }

		if (net_isserv)
		{
			numplayers = 1;
			while (numplayers < waitplayers)
			{
				Sleep(5);
				gsk[numplayers] = net_getplayer();
				if (gsk[numplayers] != INVALID_SOCKET)
				{
					numplayers++;
					sprintf(tbuf,"%d players\n",numplayers); WriteConsole(hconsout,tbuf,strlen(tbuf),(unsigned long *)&j,0);
				}
			}
			for(i=1;i<numplayers;i++)
			{
				tbuf[0] = (char)numplayers;
				tbuf[1] = (char)i;
				net_write(gsk[i],(char *)&tbuf,2);
			}
			moveindex = 0;
		}
		else
		{
			while (!net_read(gsk[0],&tbuf[0],1)) Sleep(5);
			while (!net_read(gsk[0],&tbuf[1],1)) Sleep(5);
			numplayers = (int)tbuf[0];
			moveindex = (int)tbuf[1];
		}
		FreeConsole();
		viewindex = moveindex;

		if (ntohl(net_sa.sin_addr.s_addr) == 0x7f000001)
		{
			extern long progwndx, progwndy;
			progwndx = (moveindex&1)*640; progwndy = (moveindex>>1)*500;
			xres = 640; yres = 480;
		}
		else
		{
			if (simlagms) MessageBox(ghwnd,"WARNING: /simlag option enabled!",prognam,MB_OK);
			if (!logfilnam[0]) strcpy(logfilnam,"build2.log"); //auto log true multi games
		}
	}

	for(i=0;i<sizeof(tilelist)/sizeof(tilelist[0]);i++)
	{
		kpgetdim((char *)tilelist[i].buf,tilelist[i].leng,(int *)&tilelist[i].t.x,(int *)&tilelist[i].t.y);
		tilelist[i].t.p = (tilelist[i].t.x<<2);
		tilelist[i].t.f = (long)malloc(tilelist[i].t.p*(tilelist[i].t.y+1)+4); if (!tilelist[i].t.f) return(-1);
		kprender((char *)tilelist[i].buf,tilelist[i].leng,tilelist[i].t.f,tilelist[i].t.p,tilelist[i].t.x,tilelist[i].t.y,0,0);
		tilelist[i].t.lowermip = 0;
		fixtex4grou(&tilelist[i].t);
	}

	initcrc32();

	gnumtiles = 0; memset(gtilehashead,-1,sizeof(gtilehashead));
	gmaltiles = 256;
	gtile = (tile_t *)malloc(gmaltiles*sizeof(tile_t)); if (!gtile) return(-1);
	//memset(gtile,0,gmaltiles*sizeof(tile_t)); //FIX

	gst = &sst;
	gst->numsects = 0;
	gst->malsects = 256;
	gst->sect = (sect_t *)malloc(gst->malsects*sizeof(sect_t)); if (!gst->sect) return(-1);
	memset(gst->sect,0,gst->malsects*sizeof(sect_t));

	gst->numspris = 0;
	gst->malspris = 256;
	gst->spri = (spri_t *)malloc(gst->malspris*sizeof(spri_t)); if (!gst->spri) return(-1);
	memset(gst->spri,0,gst->malspris*sizeof(spri_t));

	for(y=0;y<64;y++)
		for(x=0;x<64;x++)
		{
				//Generated null sign
			float f, g;
			f = fabs(sqrt((double)((x-32)*(x-32) + (y-32)*(y-32)))-18.0)*sqrt(2.0)/2.0;
			if (labs(x-y) < 25) f = min(f,fabs((double)(x+y-64))*.5);
			if (f < 4) nullpic[y][x] = ((long)((4-f)*63))*0x010000 + 0xff000000;
					else nullpic[y][x] = 0;

				//Generate folder icon
			i = 0x505050;
			if ((x < 32-labs(y-13)) && (x > 19-y) && (x > 6) && (labs(y-13) < 5)) i = 0xffd0d040;
			else
			{
				f = x-32; f *= f; f *= f;
				g = y-32; g *= g; g *= g;
				f = f*f/10 + g*g;
				if (f < 20000000000)
				{
					f = (20000000000-f)/200000000;
					i = 0xff808000 + ((long)f)*0x10101; //+ ((x+y)>>1)*0x10102;
				}
			}
			if ((x > 6) && (x < 28) && ((y == 17) || (y == 18))) i = 0xff909030;
			if ((labs(x+y-44) < 2) && (labs(y-x+15) < 5)) i = 0xff909030;
			folder[y][x] = i;

				//Generated folder icon with green arrow
			if ((labs(x  -32) < 3) && (labs(y  -32) < 12)) i = 0xff00a000;
			if ((labs(x+y-52) < 3) && (labs(x-y- 4) < 10)) i = 0xff00a000;
			if ((labs(x-y-11) < 3) && (labs(x+y-60) < 10)) i = 0xff00a000;
			upfolder[y][x] = i;
		}
	memcpy(&nullpic [64][0],&nullpic [0][0],64<<2);
	memcpy(&folder  [64][0],&folder  [0][0],64<<2);
	memcpy(&upfolder[64][0],&upfolder[0][0],64<<2);

	gst->blankheadspri = -1;

	build2.gammaval = 1.0; //1.0=no change, useful range={0.0..~4.0)
	setgammlut(build2.gammaval); //Safe here because gst exists and gnumtiles = 0

	drawpolfunc = drawpol;
	gettileind("tex\\cloud.png"); //force it to be index 0

#if USEGROU
	{
	SYSTEM_INFO si; //1 cpu system runs faster with 1 thread.. this helps if people copy the cfg file
	GetSystemInfo(&si);
	b2opts.drawthreads = min(b2opts.drawthreads,si.dwNumberOfProcessors);
	}
	drawpoly_anginc = b2opts.anginc;
	drawpoly_numcpu = b2opts.drawthreads;
	drawpoly_init();
#endif

	shadowtest2_numcpu = b2opts.drawthreads;
	shadowtest2_init();

	drawkv6_init();

	colwheel_init();

	gst->startpos.x = 0.0; gst->startpos.y = 0.0; gst->startpos.z = 0.0;
	gst->startrig.x = 1.0; gst->startrig.y = 0.0; gst->startrig.z = 0.0;
	gst->startdow.x = 0.0; gst->startdow.y = 0.0; gst->startdow.z = 1.0;
	gst->startfor.x = 0.0; gst->startfor.y =-1.0; gst->startfor.z = 0.0;
	for(i=0;i<numplayers;i++)
	{
		gps = &gst->p[i];
		memset(gps,0,sizeof(playerstruct_t));
		gps->fcmousx = 800/2; //NOTE: Do not replace with variables - static init needed for sync
		gps->fcmousy = 600/2;
		gps->dgridlock = 1.0/8.0;
		gps->grabsect = -1; gps->grabwall = -1; gps->grabmode = -1; gps->grabcf = 0;
		gps->playerindex = i;
		gps->circnum = 7;
		gps->docollide = 1;
		gps->gridlock = 1;
		gps->compact2d = 1.0;
		gps->goalheight2d = 24.0;
		gps->showdebug = 0;
		gps->showedges3d = 1;
		gps->boolfunc = POLYBOOL_END;
		gps->editmode = 2;
		gps->selrgbrad = 64.0;
		gps->cursect = -1;
		gps->rendheight = 0;
		gps->rendinterp = 0;
		gps->typehighlight = -1;
		gps->flashlighton = 1;

		gps->ipos.x = 0; gps->ipos.y = 0; gps->ipos.z = -gps->goalheight2d;
		gps->irig.x = 1; gps->irig.y = 0; gps->irig.z = 0;
		gps->idow.x = 0; gps->idow.y = 1; gps->idow.z = 0;
		gps->ifor.x = 0; gps->ifor.y = 0; gps->ifor.z = 1;

		gps->grdc.x = 0; gps->grdc.y = 0; gps->grdc.z = 0; //center
		gps->grdu.x = 1; gps->grdu.y = 0; gps->grdu.z = 0;
		gps->grdv.x = 0; gps->grdv.y = 1; gps->grdv.z = 0;
		gps->grdn.x = 0; gps->grdn.y = 0; gps->grdn.z = 1; //normal

		gps->ghx = 800/2; //NOTE: Do not replace with variables - static init needed for sync
		gps->ghy = 600/2;
		gps->ghz = gps->ghx;
		gps->zoom = gps->ozoom = 1.f;

		//memset(&gps->sec,0,sizeof(sect_t));
	}
	if (moveindex)
	{
		//build2_copygamestate(&pst,&sst);
		memcpy(&pst,&sst,sizeof(gamestate_t));
		pst.sect = (sect_t *)malloc(pst.malsects*sizeof(sect_t)); if (!pst.sect) return(-1);
		pst.spri = (spri_t *)malloc(pst.malspris*sizeof(spri_t)); if (!pst.spri) return(-1);
		memset(pst.sect,0,pst.malsects*sizeof(sect_t));
		memset(pst.spri,0,pst.malspris*sizeof(spri_t));

		sndst = &pst;
	} else sndst = &sst;

	build2.fattestsprite = 2.0;

	for(j=0;j<argnoslashcnt;j++)
	{
		if (loadmap(argv[argnoslash[j]])) i = 1;
		else
		{
			char tbuf[MAX_PATH];
			sprintf(tbuf,"%s.map",argv[argnoslash[j]]);
			i = loadmap(tbuf);
		}
		if (i)
		{
			for(i=numplayers-1;i>=0;i--)
			{
				gst->p[i].editmode = 3;
				gst->p[i].compact2d = 0.0;
			}
		}
	}

	if (logfilnam)
	{
		FILE *fil;
		char tbuf[128];

		fil = fopen(logfilnam,"a");
		if (!fil) logfilnam[0] = 0;
		else
		{
			time_t ltime;
			time(&ltime);
			strftime(tbuf,sizeof(tbuf),"%A, %Y.%m.%d %H:%M:%S",localtime(&ltime));
			fprintf(fil,"\nBUILD2 session begins (%s)\n",tbuf);
			fclose(fil);
		}
	}

	memset(fpsometer,0x7f,sizeof(fpsometer));
	for(i=0;i<FPSSIZ;i++) { fpsind[0][i] = fpsind[1][i] = i; numframes[0] = numframes[1] = 0; }
	readklock(&dtotclk); dtotclk2 = dtotclk;

	//extern long dinputkeyboardflags, dinputmouseflags; dinputmouseflags = dinputkeyboardflags; //Set NONEXCLUSIVE mouse!
	setacquire(0,1);
	ghblankcurs = genblankcursor();
	ghcrosscurs = gencrosscursor();

		//Send resolution
	tbuf[0] = 9; //leng
	tbuf[1] = (unsigned char)moveindex;
	tbuf[2] = 128; //bit fields
	tbuf[3] = 0; //tics
	tbuf[4] = (fullscreen != 0); //Command 0 & Command 1: Send resolution
	*(short *)&tbuf[5] = xres;
	*(short *)&tbuf[7] = yres; j = 9;
		//Copy to local command buffer
	memcpy(&cmdpackfif[cmdpackfifw&65535],tbuf,j); cmdpackfifw += j;

		//Send nick
	if (b2opts.nick[0])
	{
		tbuf[1] = (unsigned char)moveindex;
		tbuf[2] = 128; //bit fields
		tbuf[3] = 0; //tics
		tbuf[4] = 2; //Command 2: Send nick
		for(j=0;(j < 63) && (b2opts.nick[j]);j++) tbuf[j+5] = b2opts.nick[j];
		tbuf[j+5] = 0; j += 5+1;
		tbuf[0] = j;
			//Copy to local command buffer
		memcpy(&cmdpackfif[cmdpackfifw&65535],tbuf,j); cmdpackfifw += j;
	}

		//Send bilin
	tbuf[0] = 6; //leng
	tbuf[1] = (unsigned char)moveindex;
	tbuf[2] = 128; //bit fields
	tbuf[3] = 0; //tics
	tbuf[4] = 6; //Command 6: Send initial bilin setting
	tbuf[5] = (b2opts.bilin&1); j = 6;
		//Copy to local command buffer
	memcpy(&cmdpackfif[cmdpackfifw&65535],tbuf,j); cmdpackfifw += j;

	return(0);
}

	//User function must draw to surface: cc->c,cc->z
	//User function must write: cc->p,r,d,f,hx,hy,hz
void drawframe (cam_t *cc)
{
	playerstruct_t cps;
	cam_t cam;
	dpoint3d dp, dr, dd, df;
	long i, j, k, l, m, flashlight1st;

	if (!moveindex) gst = &sst; else gst = &pst;
	//gdps = &gst->p[viewindex];
	memcpy(&cps,&gst->p[viewindex],sizeof(playerstruct_t)); gdps = &cps; //FIX

	if (((!b2opts.lights) || (gdps->cursect < 0)) || (gdps->compact2d > 0.0))

	for(i=0,j=cc->z.f;i<cc->c.y;i++,j+=cc->z.p) memset8((void *)j,0x7f7f7f7f,cc->c.x<<2);

	dp.x = 0.0; dp.y = 0.0; dp.z = 0.0;
	dr.x = 1.0; dr.y = 0.0; dr.z = 0.0;
	dd.x = 0.0; dd.y = 1.0; dd.z = 0.0;
	df.x = 0.0; df.y = 0.0; df.z = 1.0;
	drawpoly_setup(                           (tiletype *)&cc->c,cc->z.f-cc->c.f,&dp,&dr,&dd,&df,gdps->ghx,gdps->ghy,gdps->ghz);
	drawcone_setup(cputype,shadowtest2_numcpu,(tiletype *)&cc->c,cc->z.f-cc->c.f,&dp,&dr,&dd,&df,gdps->ghx,gdps->ghy,gdps->ghz);
	 drawkv6_setup(&drawkv6_frame,            (tiletype *)&cc->c,cc->z.f-cc->c.f,&dp,&dr,&dd,&df,gdps->ghx,gdps->ghy,gdps->ghz);

	cam.h.x = gdps->ghx; cam.h.y = gdps->ghy; cam.h.z = gdps->ghz;
	//cam.h.x = gst->p[moveindex].ghx; //Needs many internal hacks! :/
	//cam.h.y = gst->p[moveindex].ghy;
	//cam.h.z = gst->p[moveindex].ghx * gdps->ghz/gdps->ghx;
	cam.c = cc->c; cam.z = cc->z;

	if ((!b2opts.lights) || (gdps->cursect < 0))
	{
		cam.r.x = 1.f; cam.r.y = 0.f; cam.r.z = 0.f;
		cam.d.x = 0.f; cam.d.y = 1.f; cam.d.z = 0.f;
		cam.f.x = 0.f; cam.f.y = 0.f; cam.f.z = 1.f;
		cam.p.x = 0.f; cam.p.y = 0.f; cam.p.z = 0.f;
		drawkv6_numlights = -1;
		drawview(&cam,gdps,0);
	}
	else
	{
		cam.r.x = gdps->irig.x; cam.r.y = gdps->irig.y; cam.r.z = gdps->irig.z;
		cam.d.x = gdps->idow.x; cam.d.y = gdps->idow.y; cam.d.z = gdps->idow.z;
		cam.f.x = gdps->ifor.x; cam.f.y = gdps->ifor.y; cam.f.z = gdps->ifor.z;
		cam.p.x = gdps->ipos.x; cam.p.y = gdps->ipos.y; cam.p.z = gdps->ipos.z;

#if 0
		//FIXFIX: this block is a useless piece of bloated crud

		//t = ((ox-gdps->grdc.x)*gdps->grdn.x +
		//     (oy-gdps->grdc.y)*gdps->grdn.y +
		//     (oz-gdps->grdc.z)*gdps->grdn.z)*gdps->compact2d;
		//ox -= (gdps->grdn.x*t + gdps->ipos.x);
		//oy -= (gdps->grdn.y*t + gdps->ipos.y);
		//oz -= (gdps->grdn.z*t + gdps->ipos.z);
		//(*x) = ox*gdps->irig.x + oy*gdps->irig.y + oz*gdps->irig.z;
		//(*y) = ox*gdps->idow.x + oy*gdps->idow.y + oz*gdps->idow.z;
		//(*z) = ox*gdps->ifor.x + oy*gdps->ifor.y + oz*gdps->ifor.z;
		if (gdps->compact2d > 0.0)
		{
			float k0, k1, k2, k3, k4, k5, k6, k7, rdet;
			float m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, ma, mb;

			k0 = gdps->grdc.x*gdps->grdn.x + gdps->grdc.y*gdps->grdn.y + gdps->grdc.z*gdps->grdn.z;
			k1 = gdps->irig.x*gdps->grdn.x + gdps->irig.y*gdps->grdn.y + gdps->irig.z*gdps->grdn.z;
			k2 = gdps->idow.x*gdps->grdn.x + gdps->idow.y*gdps->grdn.y + gdps->idow.z*gdps->grdn.z;
			k3 = gdps->ifor.x*gdps->grdn.x + gdps->ifor.y*gdps->grdn.y + gdps->ifor.z*gdps->grdn.z;
			k4 = gdps->ipos.x*gdps->grdn.x + gdps->ipos.y*gdps->grdn.y + gdps->ipos.z*gdps->grdn.z;
			k5 = gdps->irig.x*gdps->ipos.x + gdps->irig.y*gdps->ipos.y + gdps->irig.z*gdps->ipos.z;
			k6 = gdps->idow.x*gdps->ipos.x + gdps->idow.y*gdps->ipos.y + gdps->idow.z*gdps->ipos.z;
			k7 = gdps->ifor.x*gdps->ipos.x + gdps->ifor.y*gdps->ipos.y + gdps->ifor.z*gdps->ipos.z;

			m0 = gdps->irig.x - k1*gdps->grdn.x*gdps->compact2d;
			m1 = gdps->irig.y - k1*gdps->grdn.y*gdps->compact2d;
			m2 = gdps->irig.z - k1*gdps->grdn.z*gdps->compact2d;
			m3 = k1*k0*gdps->compact2d - k5;
			m4 = gdps->idow.x - k2*gdps->grdn.x*gdps->compact2d;
			m5 = gdps->idow.y - k2*gdps->grdn.y*gdps->compact2d;
			m6 = gdps->idow.z - k2*gdps->grdn.z*gdps->compact2d;
			m7 = k2*k0*gdps->compact2d - k6;
			m8 = gdps->ifor.x - k3*gdps->grdn.x*gdps->compact2d;
			m9 = gdps->ifor.y - k3*gdps->grdn.y*gdps->compact2d;
			ma = gdps->ifor.z - k3*gdps->grdn.z*gdps->compact2d;
			mb = k3*k0*gdps->compact2d - k7;

			//(*x) = ox*m0 + oy*m1 + oz*m2 + m3;
			//(*y) = ox*m4 + oy*m5 + oz*m6 + m7;
			//(*z) = ox*m8 + oy*m9 + oz*ma + mb;

			//x = ox*m0 + oy*m1 + oz*m2 + m3
			//y = ox*m4 + oy*m5 + oz*m6 + m7
			//z = ox*m8 + oy*m9 + oz*ma + mb

			//x = (ox-ux)*m0 + (oy-uy)*m1 + (oz-uz)*m2
			//y = (ox-ux)*m4 + (oy-uy)*m5 + (oz-uz)*m6
			//z = (ox-ux)*m8 + (oy-uy)*m9 + (oz-uz)*ma

				//solve for ux,uy,uz
			//ux*m0 + uy*m1 + uz*m2 = -m3
			//ux*m4 + uy*m5 + uz*m6 = -m7
			//ux*m8 + uy*m9 + uz*ma = -mb

			rdet = 1.0/(m0*(m5*ma - m9*m6) + m4*(m9*m2 - m1*ma) + m8*(m1*m6 - m5*m2));
			//cam.p.x =  (m3*(m9*m6 - m5*ma) + m7*(m1*ma - m9*m2) + mb*(m5*m2 - m1*m6))*rdet;
			//cam.p.y =  (m0*(mb*m6 - m7*ma) + m4*(m3*ma - mb*m2) + m8*(m7*m2 - m3*m6))*rdet;
			//cam.p.z =  (m0*(m9*m7 - m5*mb) + m4*(m1*mb - m9*m3) + m8*(m5*m3 - m1*m7))*rdet;

			//(*x) = (ox-gdps->ipos.x)*gdps->irig.x + (oy-gdps->ipos.y)*gdps->irig.y + (oz-gdps->ipos.z)*gdps->irig.z;
			//(*y) = (ox-gdps->ipos.x)*gdps->idow.x + (oy-gdps->ipos.y)*gdps->idow.y + (oz-gdps->ipos.z)*gdps->idow.z;
			//(*z) = (ox-gdps->ipos.x)*gdps->ifor.x + (oy-gdps->ipos.y)*gdps->ifor.y + (oz-gdps->ipos.z)*gdps->ifor.z;

			cam.r.z *= (1.0-gdps->compact2d);
			cam.d.z *= (1.0-gdps->compact2d);
			cam.f.z *= (1.0-gdps->compact2d);
		}
#endif


		if (gdps->compact2d == 0.0)
		{
			shadowtest2_useshadows = b2opts.shadows; shadowtest2_numlights = 0;
			for(i=gst->light_sprinum-1;i>=0;i--)
			{
				if (((unsigned)gst->light_spri[i] < (unsigned)gst->malspris) && (gst->spri[gst->light_spri[i]].sect >= 0) && (shadowtest2_numlights < MAXLIGHTS))
				{
					shadowtest2_light[shadowtest2_numlights].sect   = gst->spri[gst->light_spri[i]].sect;
					shadowtest2_light[shadowtest2_numlights].p      = gst->spri[gst->light_spri[i]].p;
					k = ((gst->spri[gst->light_spri[i]].flags>>17)&7);
					if (!k) { shadowtest2_light[shadowtest2_numlights].spotwid = -1.0; }
					else
					{
						m = ((gst->spri[gst->light_spri[i]].flags>>20)&1023); if (!m) continue;
						shadowtest2_light[shadowtest2_numlights].spotwid = cos(m*PI/1024.0); //FIX:use lut
						switch(k)
						{
							case 1: case 2: shadowtest2_light[shadowtest2_numlights].f = gst->spri[gst->light_spri[i]].d; break;
							case 3: case 4: shadowtest2_light[shadowtest2_numlights].f = gst->spri[gst->light_spri[i]].f; break;
							case 5: case 6: shadowtest2_light[shadowtest2_numlights].f = gst->spri[gst->light_spri[i]].r; break;
						}
						if (!(k&1)) { shadowtest2_light[shadowtest2_numlights].f.x *= -1; shadowtest2_light[shadowtest2_numlights].f.y *= -1; shadowtest2_light[shadowtest2_numlights].f.z *= -1; }
					}
					shadowtest2_light[shadowtest2_numlights].rgb[0] = gst->spri[gst->light_spri[i]].bsc/8192.f; //gsc/8192   gst->spri[gst->light_spri[i]].fat;
					shadowtest2_light[shadowtest2_numlights].rgb[1] = gst->spri[gst->light_spri[i]].gsc/8192.f;
					shadowtest2_light[shadowtest2_numlights].rgb[2] = gst->spri[gst->light_spri[i]].rsc/8192.f;
					shadowtest2_light[shadowtest2_numlights].flags  = 1;
					shadowtest2_numlights++;
				}
			}

			flashlight1st = shadowtest2_numlights;
			for(i=0;i<numplayers;i++)
			{
				if ((gst->p[i].flashlighton) && (shadowtest2_numlights < MAXLIGHTS))
				{
					shadowtest2_light[shadowtest2_numlights].sect   = gst->p[i].cursect;
					shadowtest2_light[shadowtest2_numlights].p      = gst->p[i].ipos;
					shadowtest2_light[shadowtest2_numlights].f      = gst->p[i].ifor;
					shadowtest2_light[shadowtest2_numlights].rgb[0] = 0.5f;
					shadowtest2_light[shadowtest2_numlights].rgb[1] = 0.5f;
					shadowtest2_light[shadowtest2_numlights].rgb[2] = 0.5f;
					shadowtest2_light[shadowtest2_numlights].spotwid= -1.0;
					shadowtest2_light[shadowtest2_numlights].flags  = (i != viewindex);
					shadowtest2_numlights++;
				}
			}

			drawkv6_numlights = shadowtest2_numlights;
			for(i=shadowtest2_numlights-1;i>=0;i--)
			{
				drawkv6_light[i].sect      = shadowtest2_light[i].sect;
				drawkv6_light[i].p         = shadowtest2_light[i].p;
				drawkv6_light[i].rgb[0]    = shadowtest2_light[i].rgb[0];
				drawkv6_light[i].rgb[1]    = shadowtest2_light[i].rgb[1];
				drawkv6_light[i].rgb[2]    = shadowtest2_light[i].rgb[2];
				drawkv6_light[i].useshadow =(shadowtest2_light[i].flags&1);
				xformpos(&drawkv6_light[i].p.x,&drawkv6_light[i].p.y,&drawkv6_light[i].p.z);
			}
			drawkv6_ambrgb[0] = shadowtest2_ambrgb[0];
			drawkv6_ambrgb[1] = shadowtest2_ambrgb[1];
			drawkv6_ambrgb[2] = shadowtest2_ambrgb[2];

			shadowtest2_rendmode = 2; draw_hsr_polymost(&cam,gst,gdps,gdps->cursect); shadowtest2_rendmode = 4;
			if (shadowtest2_updatelighting) //FIXFIX
			{
				cam_t ncam; ncam = cam;
				shadowtest2_updatelighting = 0; //FIXFIX
				shadowtest2_ligpolreset(-1);
				for(glignum=0;glignum<shadowtest2_numlights;glignum++)
				{
					ncam.p = shadowtest2_light[glignum].p;
					draw_hsr_polymost(&ncam,gst,gdps,shadowtest2_light[glignum].sect);
				}
			}
			else
			{
				cam_t ncam; ncam = cam;
				for(glignum=flashlight1st;glignum<shadowtest2_numlights;glignum++)
				{
					shadowtest2_ligpolreset(glignum);
					ncam.p = shadowtest2_light[glignum].p;
					draw_hsr_polymost(&ncam,gst,gdps,shadowtest2_light[glignum].sect);
				}
			}

			shadowtest2_setcam(&cam);
			htrun(drawpollig,0,eyepoln,shadowtest2_numcpu);
			//drawsprites();
			drawview(&cam,gdps,1);

			drawkv6_numlights = -1;
		}
		else
		{
			drawview(&cam,gdps,0);
		}
	}

		//FIX: debug only
	if (((unsigned)gdps->cursect < (unsigned)gst->numsects) && (gdps->editmode == 3) && (gdps->showdebug))
	{
		dpoint3d dp, clos;
		dp.x = (double)gdps->ipos.x;
		dp.y = (double)gdps->ipos.y;
		dp.z = (double)gdps->ipos.z;
		print6x8((tiltyp *)&cam.c,cam.c.x-350*fontscale,48,0xffffff,-1,"maxcr:%.9f, chn=%d",findmaxcr(&dp,gdps->cursect,1e16,&clos),build2.cliphitnum);
		if (build2.cliphitnum < 0) drawsph(&cam,clos.x,clos.y,clos.z,.02+rand()/3276800.0,0x808080);
	}

		//FPS counter
	odtotclk2 = dtotclk2; readklock(&dtotclk2);
	fpsometer[1][numframes[1]&(FPSSIZ-1)] = (long)((dtotclk2-odtotclk2)*1000000); numframes[1]++;
		//Fast sort when already sorted... otherwise slow!
	j = min(numframes[1],FPSSIZ)-1;
	for(k=0;k<j;k++)
		if (fpsometer[1][fpsind[1][k]] > fpsometer[1][fpsind[1][k+1]])
		{
			m = fpsind[1][k+1];
			for(l=k;l>=0;l--)
			{
				fpsind[1][l+1] = fpsind[1][l];
				if (fpsometer[1][fpsind[1][l]] <= fpsometer[1][m]) break;
			}
			fpsind[1][l+1] = m;
		}
	microsec[1] = ((fpsometer[1][fpsind[1][j>>1]]+fpsometer[1][fpsind[1][(j+1)>>1]])>>1); //Median


		//User function must write these:
	cc->p = gdps->ipos; cc->r = gdps->irig; cc->d = gdps->idow; cc->f = gdps->ifor;
	cc->h.x = gdps->ghx; cc->h.y = gdps->ghy; cc->h.z = gdps->ghz;
}

void doframe ()
{
	gamestate_t *dast;
	playerstruct_t *lps;
	cam_t cam;
	sect_t *sec;
	wall_t *wal;
	spri_t *spr;
	tiletype dd;
	double d;
	float f;
	long *lptr;
	int i, j, k, l, m, zbufoff;
	char tbuf[256], *cptr;

#if USEMORPH
	if (b2opts.usemorph)
	{
			//put inside doframe, before startdirectdraw
		static int morphinited = 0;
		if (!morphinited) { morphinited = 1; morph_init(xres,yres,fullscreen,drawframe); }
		morph_sleepuntilretrace();
	}
#endif
	if (doquitloop)
	{
#if USEMORPH
		if (b2opts.usemorph) morph_uninit();
#endif
		quitloop(); return;
	}

	if (!moveindex) dast = &sst; else dast = &pst;
	lps = &dast->p[viewindex];

	if (ext_keystatus[keyscanremapr[0x2b]]) //backslash
	{
		ext_keystatus[keyscanremapr[0x2b]] = 0;
		if (!dast->p[moveindex].typemode)
			{ viewindex++; if (viewindex >= numplayers) viewindex = 0; }
	}

#ifndef NOSOUND
	f = SNDISTSCALEHACK;
	setears3d(dast->p[viewindex].ipos.x  ,dast->p[viewindex].ipos.y  ,dast->p[viewindex].ipos.z  ,  //ipos
				 dast->p[viewindex].ifor.x*f,dast->p[viewindex].ifor.y*f,dast->p[viewindex].ifor.z*f,  //ifor
				 dast->p[viewindex].idow.x*f,dast->p[viewindex].idow.y*f,dast->p[viewindex].idow.z*f); //idow
#endif

//--------------------------------------------------------------------------------------------------

	if (!startdirectdraw((long *)&dd.f,(long *)&dd.p,(long *)&dd.x,(long *)&dd.y)) goto skipdd;

	i = dd.y*dd.p+256;
	if ((i > zbuffersiz) || (!zbuffermem))  //Increase Z buffer size if too small
	{
		if (zbuffermem) { free(zbuffermem); zbuffermem = 0; }
		zbuffersiz = i;
		zbuffermem = (long *)malloc(zbuffersiz);
	}
		//zbuffer aligns its memory to the same pixel boundaries as the screen!
		//WARNING: Pentium 4's L2 cache has severe slowdowns when 65536-64 <= (zbufoff&65535) < 64
	zbufoff = (((((long)zbuffermem)-dd.f-128)+255)&~255)+128;

	netcheck();
	odtotclk = dtotclk; readklock(&dtotclk);
	getnsendinput();

	cam.h.x = lps->ghx; cam.h.y = lps->ghy; cam.h.z = lps->ghz;
	cam.c.f = dd.f;         cam.c.p = dd.p; cam.c.x = dd.x; cam.c.y = dd.y;
	cam.z.f = dd.f+zbufoff; cam.z.p = dd.p; cam.z.x = dd.x; cam.z.y = dd.y;
#if (USEMORPH == 0)
	drawframe(&cam);
#else
	if (!b2opts.usemorph)
		drawframe(&cam);
	else
	{
			//Needed only when USEREVMAP=0
		//lptr = (long *)cam.z.f;
		//for(i=cam.z.y;i>0;i--,lptr=(long *)(((long)lptr)+cam.z.p)) memset8(lptr,0x7f7f7f7f,(cam.z.x<<2));
		morph_drawframe((tiltyp *)&dd,dd.f+zbufoff,&lps->ipos,&lps->irig,&lps->idow,&lps->ifor,lps->ghx,lps->ghy,lps->ghz);
	}
#endif

	if (viewindex != moveindex)
	{
		if (!dast->nick[viewindex][0]) { cptr = tbuf; sprintf(tbuf,"Player %d",viewindex); }
										  else { cptr = dast->nick[viewindex]; }
		print6x8((tiltyp *)&dd,(dd.x>>1)-strlen(cptr)*3,16,0xffffff,0,"%s's view",cptr);
	}
	if ((message[0]) && (dtotclk < messagetimeout)) print6x8((tiltyp *)&dd,(dd.x>>1)-strlen(message)*3,0,0xffff00,0,"%s",message);

	if (lps->showdebug)
	{
		print6x8((tiltyp *)&dd,dd.x-220,0,0xffffff,-1,"%6d byt sent, %6d byt recv",bytessent,bytesrecv);
		print6x8((tiltyp *)&dd,dd.x-220,lineHeight,0xffffff,-1,"%6.0f bps sent, %6.0f bps recv",((double)bytessent)/dtotclk,((double)bytesrecv)/dtotclk);
		if (moveindex) print6x8((tiltyp *)&dd,dd.x-110,16,0xffffff,-1,"lag: %d",ppackfifwn-ppackfifrn);
		print6x8((tiltyp *)&dd,dd.x-220,lineHeight*2,0xffffff,-1,"cursect:%d",lps->cursect);
#if (OOS_CHECK)
		if (numplayers >= 2) print6x8((tiltyp *)&dd,dd.x-220,24,0xffffff,-1,"gamestate:%d",totcrcbytes);
#endif
	}
    int helpinfoXOffset = 320*fontscale;
	if ((unsigned)lps->grabsect < (unsigned)dast->numsects)
	{
		j = lineHeight;
		if (lps->grabwall >= 0)
		{
			if ((lps->grabwall&0xc0000000) == 0x40000000) //sprite
			{
				i = lps->grabwall&0x3fffffff; spr = &dast->spri[i];
				print6x8((tiltyp *)&dd,dd.x-helpinfoXOffset,j,0xffffff,0,"spri:%d (sect:%d) (p:%d n:%d)",i,lps->grabsect,spr->sectp,spr->sectn); j += lineHeight;
				print6x8((tiltyp *)&dd,dd.x-helpinfoXOffset,j,0xffffff,0,"x/y/z:(%g,%g,%g)",spr->p.x,spr->p.y,spr->p.z); j += lineHeight;
				print6x8((tiltyp *)&dd,dd.x-helpinfoXOffset,j,0xffffff,0,"fat:%g, owner:%d",spr->fat,spr->owner); j += lineHeight;
				if ((unsigned)spr->tilnum < (unsigned)gnumtiles)
					{ print6x8((tiltyp *)&dd,dd.x-helpinfoXOffset,j,0xffffff,0,"file:\"%s\"",gtile[spr->tilnum].filnam); j += lineHeight; }
				print6x8((tiltyp *)&dd,dd.x-helpinfoXOffset,j,0xffffff,0,"flags:0x%08x",spr->flags); j += lineHeight;
				print6x8((tiltyp *)&dd,dd.x-helpinfoXOffset,j,0xffffff,0,"tags:%d,%d",spr->lotag,spr->hitag); j += lineHeight;
			}
			else //wall
			{
				wal = &dast->sect[lps->grabsect].wall[lps->grabwall];
				print6x8((tiltyp *)&dd,dd.x-helpinfoXOffset,j,0xffffff,0,"wall:%d (sect:%d)",lps->grabwall,lps->grabsect); j += lineHeight;
				print6x8((tiltyp *)&dd,dd.x-helpinfoXOffset,j,0xffffff,0,"x/y:(%g,%g)",wal->x,wal->y); j += lineHeight;
				print6x8((tiltyp *)&dd,dd.x-helpinfoXOffset,j,0xffffff,0,"n:%d, ns:%d, nw:%d, owner:%d",wal->n,wal->ns,wal->nw,wal->owner); j += lineHeight;
				if ((unsigned)wal->surf.tilnum < (unsigned)gnumtiles) { print6x8((tiltyp *)&dd,dd.x-320,j,0xffffff,0,"file:\"%s\"",gtile[wal->surf.tilnum].filnam); j += lineHeight; }
				print6x8((tiltyp *)&dd,dd.x-helpinfoXOffset,j,0xffffff,0,"flags:0x%08x",wal->surf.flags); j += lineHeight;
				print6x8((tiltyp *)&dd,dd.x-helpinfoXOffset,j,0xffffff,0,"tags:%d,%d",wal->surf.lotag,wal->surf.hitag); j += lineHeight;
			}
		}
		else //sector
		{
			i = (lps->grabcf&1); sec = &dast->sect[lps->grabsect];
			print6x8((tiltyp *)&dd,dd.x-helpinfoXOffset,j,0xffffff,0,"sect:%d",lps->grabsect); j += lineHeight;
			print6x8((tiltyp *)&dd,dd.x-helpinfoXOffset,j,0xffffff,0,"z:%g, gradx/y:(%g,%g)",sec->z[i],sec->grad[i].x,sec->grad[i].y); j += lineHeight;
			print6x8((tiltyp *)&dd,dd.x-helpinfoXOffset,j,0xffffff,0,"n:%d, headspri:%d, owner:%d",sec->n,sec->headspri,sec->owner); j += lineHeight;
			if ((unsigned)sec->surf[i].tilnum < (unsigned)gnumtiles) { print6x8((tiltyp *)&dd,dd.x-320,j,0xffffff,0,"file:\"%s\"",gtile[sec->surf[i].tilnum].filnam); j += lineHeight; }
			print6x8((tiltyp *)&dd,dd.x-helpinfoXOffset,j,0xffffff,0,"flags:0x%08x",sec->surf[i].flags); j += lineHeight;
			print6x8((tiltyp *)&dd,dd.x-helpinfoXOffset,j,0xffffff,0,"tags:%d,%d",sec->surf[i].lotag,sec->surf[i].hitag); j += lineHeight;
		}
	}

	if (showhelpgoal != showhelppos)
	{
		if (showhelpgoal < showhelppos) { showhelppos = max(showhelppos-(dtotclk-odtotclk)*4.f,0.f); }
											else { showhelppos = min(showhelppos+(dtotclk-odtotclk)*4.f,1.f); }
	}
	if (showhelppos > 0.f)
	{
		if (!showhelpbuf)
		{
			FILE *fil; fil = fopen("build2.txt","rb");
			if (!fil) showhelpbuf = (char *)-1;
			else
			{
				fseek(fil,0,SEEK_END); showhelpbufleng = ftell(fil); fseek(fil,0,SEEK_SET);
				showhelpbuf = (char *)malloc(showhelpbufleng);
				if (!showhelpbuf) { showhelpbuf = (char *)-1; }
								 else { fread(showhelpbuf,showhelpbufleng,1,fil); }
				fclose(fil);
				showhelpbuflines = 0; k = 0;
				for(i=j=0;i<showhelpbufleng;i++)
				{
					if (showhelpbuf[i] == 10) continue;
					if (showhelpbuf[i] == 13) { showhelpbuf[j] = 0; if (k == 72) { j++; break; } k = 0; showhelpbuflines++; }
					else
					{
						if (showhelpbuf[i] == '-') k++;
						showhelpbuf[j] = showhelpbuf[i];
					}
					j++;
				}
				showhelpbufleng = j;
			}
		}
		if ((showhelpbuf) && (showhelpbuf != (char *)-1))
		{
			k = max(dd.y-showhelpbuflines*8,0);
			drawhlin((tiltyp *)&dd,dd.x-6*72*showhelppos,dd.x,k-1,0);
			for(i=k;i<dd.y;i++) drawpix((tiltyp *)&dd,dd.x-6*72*showhelppos-1,i,0);
			for(;k<dd.y;k++) drawhlin((tiltyp *)&dd,dd.x-6*72*showhelppos,dd.x,k,0xc0c8d0);
			for(i=0,k=0;i<showhelpbufleng;i+=j+1,k++)
			{
				j = strlen(&showhelpbuf[i]);
				print6x8((tiltyp *)&dd,dd.x-6*72*showhelppos,dd.y+(k-showhelpbuflines)*lineHeight,0x000000+(showhelpbuf[i]!='-')*0x506070,-1,"%s",&showhelpbuf[i]);
			}
		}
	}

		//Display last 16 scancodes pressed at bottom-left (local machine only)
	//j = sizeof(lockeyread)/sizeof(lockeyread[0]);
	//for(i=max(lockeyreadw-min(j,16),0);i<lockeyreadw;i++)
	//   print6x8((tiltyp *)&dd,(i-lockeyreadw+min(j,16))<<4,dd.y-8,0xffffff,-1,"%02x",(lockeyread[i&(j-1)]>>8)&255);

		//FPS counter
	fpsometer[0][numframes[0]&(FPSSIZ-1)] = (long)((dtotclk-odtotclk)*1000000); numframes[0]++;
		//Fast sort when already sorted... otherwise slow!
	j = min(numframes[0],FPSSIZ)-1;
	for(k=0;k<j;k++)
		if (fpsometer[0][fpsind[0][k]] > fpsometer[0][fpsind[0][k+1]])
		{
			m = fpsind[0][k+1];
			for(l=k;l>=0;l--)
			{
				fpsind[0][l+1] = fpsind[0][l];
				if (fpsometer[0][fpsind[0][l]] <= fpsometer[0][m]) break;
			}
			fpsind[0][l+1] = m;
		}
	microsec[0] = ((fpsometer[0][fpsind[0][j>>1]]+fpsometer[0][fpsind[0][(j+1)>>1]])>>1); //Median

	i = 1;
#if USEMORPH
	if (b2opts.usemorph) i = 2;
#endif
	for(i=i-1;i>=0;i--)
	{
		j = microsec[i];
		print6x8((tiltyp *)&dd,2,i*8+2,0xffffff,-1,"%d.%03dms %.2ffps",j/1000,j%1000,1000000.0/(float)j);
	}

	if (!lps->typemode)
	{
		if ((lps->grabmode == GRABFILE) || (lps->editmode == 2))
		{
			if (fullscreen) i = 1;
			else
			{
				POINT p0; GetCursorPos(&p0);
				ScreenToClient(ghwnd,&p0);
				i = (!ismouseout(p0.x,p0.y));
			}
			if (i) SetCursor(ghcrosscurs);
		}
		else
		{
			SetCursor(ghblankcurs);
			drawmouse((tiltyp *)&dd,lps->fcmousx,lps->fcmousy,0xffffff);
		}
	}

	if (ext_keystatus[keyscanremapr[0x3b]]) //F1
	{
		ext_keystatus[keyscanremapr[0x3b]] = 0;
		showhelpgoal = 1.f-showhelpgoal;
		if (showhelpgoal) myplaysound("sounds\\drop2.wav"     ,50,0.5,0,0);
						 else myplaysound("sounds\\deleteroom.wav",50,0.5,0,0);
	}
	if (ext_keystatus[keyscanremapr[0x57]]) //F11
	{
		ext_keystatus[keyscanremapr[0x57]] = 0;
		if (keystatus[0x2a]|keystatus[0x36]) build2.gammaval = max(build2.gammaval-.1,0.0);
												  else build2.gammaval = min(build2.gammaval+.1,3.0);
		setgammlut(build2.gammaval);
		messagetimeout = dtotclk+3.0;
		sprintf(message,"Gamma: %.1f",(double)build2.gammaval);
	}
	if (ext_keystatus[keyscanremapr[0x58]]) //F12
	{
		ext_keystatus[keyscanremapr[0x58]] = 0;
		if (!(keystatus[0x2a]|keystatus[0x36]))
			  screencapture(&dd);
		else surroundcapture(&cam,dast,lps,512);
	}

#if (OOS_CHECK != 0)
		//LCtrl+RAlt+RShift+F6: OOS key! Do not enable in release!!!
	if (keystatus[0x40]&keystatus[0x1d]&keystatus[0xb8]&keystatus[0x36]) { *(long *)&sst.p[0].ipos.x ^= 1; }
#endif

	stopdirectdraw();
	nextpage();
skipdd:;
}

#endif

#if 0
!endif
#endif
