#ifndef KEN_BUILD2_H
#define KEN_BUILD2_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <malloc.h>

#pragma pack(push,1)

typedef struct { float x, y; } point2d;
#ifdef BUILD2
 //^ FIXFIXFIXFIX: hack to avoid redefinition error!
#define EXTERN
#else
#define EXTERN extern
#endif
typedef struct { float x, y, z, u, v; int n; } kgln_t;
#ifndef KEN_DRAWPOLY_H
typedef struct tiltyp { long f, p, x, y, z; float shsc; tiltyp *lowermip; } tiltyp; //shsc=suggested height scale
typedef struct { float x, y, z; } point3d;
typedef struct { double x, y, z; } dpoint3d;
typedef struct { INT_PTR f; int p, x, y; } tiletype;
typedef struct { tiltyp c, z; point3d p, r, d, f, h; } cam_t;
#endif

typedef struct
{
	char filnam[240]; //Must fit packet header, sector&wall index, null terminator in 256 byte packet
	tiltyp tt; //union! if (!tt.p), it's a 3D model, tt.f points to raw model data, tt.x is type
	long namcrc32, hashnext;
} tile_t;
EXTERN tile_t *gtile;
EXTERN long gnumtiles, gmaltiles, gtilehashead[1024];

//--------------------------------------------------------------------------------------------------
	//Map format:
typedef struct
{
	long tilnum, tilanm/*???*/;
	long flags; //Bit0:Blocking, Bit2:RelativeAlignment, Bit5:1Way, Bit16:IsParallax, Bit17:IsSkybox
	long tag;
	point2d uv[3];
	unsigned short asc, rsc, gsc, bsc; //4096 is no change
} surf_t;

typedef struct
{
	float x, y;
	long n, ns, nw; //n:rel. wall ind.; ns & nw : nextsect & nextwall_of_sect
	long owner; //for dragging while editing, other effects during game
	long surfn;
	surf_t surf, *xsurf; //additional malloced surfs when (surfn > 1)
} wall_t;

typedef struct
{
	point3d p, r, d, f;      //Position, orientation (right, down, forward)
	point3d v, av;           //Position velocity, Angular velocity (direction=axis, magnitude=vel)
	float fat, mas, moi;     //Physics (moi=moment of inertia)
	long tilnum;             //Model file. Ex:"TILES000.ART|64","CARDBOARD.PNG","CACO.KV6","HAND.KCM","IMP.MD3"
	unsigned short asc, rsc, gsc, bsc; //Color scales. 4096 is no change
	long owner, tag;
	long tim, otim;          //Time (in milliseconds) for animation
	long flags;              //Bit0:Blocking, Bit2:1WayOtherSide, Bit5,Bit4:Face/Wall/Floor/.., Bit6:1side, Bit16:IsLight, Bit17-19:SpotAx(1-6), Bit20-29:SpotWid, Bit31:Invisible
	long sect, sectn, sectp; //Current sector / doubly-linked list of indices
} spri_t;

typedef struct
{
	float minx, miny, maxx, maxy; //bounding box
	float z[2];      //ceil&flor height
	point2d grad[2]; //ceil&flor grad. grad.x = norm.x/norm.z, grad.y = norm.y/norm.z
	surf_t surf[2];  //ceil&flor texture info
	wall_t *wall;
	long n, nmax;    //n:numwalls, nmax:walls malloced (nmax >= n)
	long headspri;   //head sprite index (-1 if none)
	long foglev;
	long owner;      //for dragging while editing, other effects during game
} sect_t;
//--------------------------------------------------------------------------------------------------

#ifdef STANDALONE
#define TYPEMESSNUM 8
#define TYPEMESSLENG 256
#endif

typedef struct
{
		//screen/camera state
	int xres, yres, fullscreen;
	float ghx, ghy, ghz, zoom, ozoom;
	point3d ipos, irig, idow, ifor;
	point3d npos, nrig, ndow, nfor; //for 2d/3d swap animation
	point3d grdc, grdu, grdv, grdn; //center,u,v,normal
	float dgridlock, dgridlock3d, compact2d, goalheight2d, height2d;
	int cursect, rendheight, rendinterp, editmode;

#ifdef STANDALONE
		//input state
	float fcmousx, fcmousy;
	int bstatus, obstatus;
	char skeystatus[256>>3]; //1 bit per key

		//editing state
	int playerindex, showdebug, showedges3d, grabmode, grabsect, grabwall, grabcf, circnum;
	int gridlock, boolfunc, gotcopy, docollide;
	short selrgbbak[4];
	float selrgbrad, selrgbintens;
	point2d selcorn;
	surf_t copysurf[1]; //array form allows easier expansion later
	spri_t copyspri[1];

		//personal prototype sector
	sect_t sec;
	int startstate, startsect, startwall;

		//typing state
	int typemode, typecurs, typehighlight, typeowritemode;

	char emoticon_hair;  //0=O->, 1=O-+
	char emoticon_eyes;  //0=o_o, 1=o.O, 2=O.o, 3=O.O, |4=wink_right, |8=wink_left
	char emoticon_nose;  //0=-, 1=(none)
	char emoticon_mouth; //0=|, 1=), 2=(, 3=/, 4=P

	int flashlighton;
#endif
} playerstruct_t;

#define MAXPLAYERS 4
typedef struct
{
	point3d startpos, startrig, startdow, startfor;
	int numsects, malsects; sect_t *sect;
	int numspris, malspris; spri_t *spri;
	int blankheadspri;

	#define MAXLIGHTS 256
	int light_spri[MAXLIGHTS], light_sprinum; //List of active light sprite indices

	playerstruct_t p[MAXPLAYERS];

	int rseed;
#ifdef STANDALONE
		//typemess&nick are not in the player structure to make gamestate_crc32 easier to optimize
	char typemess[MAXPLAYERS][TYPEMESSLENG], nick[MAXPLAYERS][64];
	char chatmess[TYPEMESSNUM][TYPEMESSLENG], chatmessowner[TYPEMESSNUM];
	int chatmessn;
#endif
} gamestate_t;
EXTERN gamestate_t sst, pst, *gst;

typedef struct { int w, s; } vertlist_t;

	//Build2 shared global variables:
EXTERN struct
{
	double gammaval; //1.0=no change, useful range={0.0..~4.0)
	//----------------------- DATA coming from BUILD2.C -----------------------

		//Clipmove hit point info (use this after calling clipmove):
	double clipmaxcr; //clipmove always calls findmaxcr even with no movement
	dpoint3d cliphit[3];
	long cliphitnum, clipsect[3], clipwall[3];

	//----------------------- DATA provided to BUILD2.C -----------------------

	double fattestsprite; //For sprite collision: when to cross sectors
} build2;

#pragma pack(pop)

	//General functions:
extern long build2_init ();
extern void build2_uninit ();

	//Map functions:
extern int build2_loadmap (const char *filnam, int *cursect,
									dpoint3d *npos, dpoint3d *nrig, dpoint3d *ndow, dpoint3d *nfor);
extern void build2_copygamestate (gamestate_t *dst, gamestate_t *src);
extern double getslopez (sect_t *s, int i, double x, double y);
extern int wallprev (sect_t *s, int w);
extern int getverts (int s, int w, vertlist_t *ver, int maxverts);
extern void getcentroid (wall_t *wal, int n, float *retcx, float *retcy);
extern float getarea (wall_t *wal, int n);
extern void reversewalls (wall_t *wal, int n);
extern void rotatewallsurfsleft1 (wall_t *wal, int n);
extern void dragpoint (gamestate_t *lst, int s, int w, float x, float y);
extern void delwall (sect_t *s, int w);
extern int dupwall (sect_t *s, int w);
extern void delsect (int s);
extern int insidesect (double x, double y, wall_t *wal, int w);
extern void updatesect (float x, float y, float z, int *cursect);
extern int polybool (wall_t *wal0, int n0, wall_t *wal1, int n1, wall_t **wal2, int *n2, int op);
extern void checknextwalls ();
extern long gettileind (char *filnam);
extern long settilefilename (long hitsect, long hitwall, char *filnam);
extern void delspri (int i);
extern long insspri (int sect, float x, float y, float z);
extern void changesprisect (int i, int nsect);
extern void checksprisect (int s); //s: sector of sprites to check; -1 to check all sprites

	 //Render functions:
extern void build2_render (tiletype *dd, long lzbufoff, long cursect,
									dpoint3d *npos, dpoint3d *nrig, dpoint3d *ndow, dpoint3d *nfor,
									double hx, double hy, double hz);
extern void drawpol (cam_t *cc, kgln_t *vert, long num, tile_t *tpic, long curcol);
extern int isvispol (cam_t *cc, kgln_t *vert, long num);
extern void memset8 (void *d, long v, long n);
extern void drawsectfill3d (cam_t *cc, sect_t *sec, int isflor, int col);
extern void drawsprite (cam_t *cc, spri_t *spr);

	//Physics functions:
extern int build2_hitmove (int *cursect, dpoint3d *p, dpoint3d *v, double cr, int isslide, int *hitsect, int *hitwall);
extern int hitscan (point3d *p0, point3d *pv, float vscale, int cursect, int *hitsect, int *hitwall, point3d *hit);
extern double findmaxcr (dpoint3d *p0, int cursect, double mindist, dpoint3d *hit);

	//Math functions:
extern void orthofit3x3 (point3d *v0, point3d *v1, point3d *v2);
extern void orthorotate (double ox, double oy, double oz,  point3d *iri,  point3d *ido,  point3d *ifo);
extern void orthorotate (double ox, double oy, double oz, dpoint3d *iri, dpoint3d *ido, dpoint3d *ifo);
extern void slerp (point3d *irig,  point3d *idow,  point3d *ifor,
						 point3d *irig2, point3d *idow2, point3d *ifor2,
						 point3d *iri,   point3d *ido,   point3d *ifo,   float rat);

	//Disk functions:
extern void savemap (char *filnam);
extern void savekc (char *filnam);
extern void screencapture (tiletype *tt);

#endif
