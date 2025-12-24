#ifndef KEN_BUILD2_H
#define KEN_BUILD2_H

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <malloc.h>

#include "scenerender.h"

#pragma pack(push,1)


#ifdef BUILD2
 //^ FIXFIXFIXFIX: hack to avoid redefinition error!
#define EXTERN
#else
#define EXTERN extern
#endif
//--------------------------------------------------------------------------------------------------

#ifdef STANDALONE
#define TYPEMESSNUM 8
#define TYPEMESSLENG 256
#endif
#include "Core/mapcore.h"
typedef struct
{
	union {
		struct {
			player_transform tr;
		};

		//screen/camera state
		struct {
			int xres, yres, fullscreen;
			float ghx, ghy, ghz, zoom, ozoom; // halfx y z of screen in px.
			point3d ipos, irig, idow, ifor;
			point3d npos, nrig, ndow, nfor; //for 2d/3d swap animation
			point3d grdc, grdu, grdv, grdn; //center,u,v,normal
			int cursect;
		};
	};

	float dgridlock, dgridlock3d, compact2d, goalheight2d, height2d;
	int rendheight, rendinterp, editmode;

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
	union {
		// New way: access via map
		struct {
			mapstate_t map;
		};

		// Old way: direct access (maintains binary compatibility)
		struct {
			point3d startpos, startrig, startdow, startfor;
			int numsects, malsects; sect_t *sect;
			int numspris, malspris; spri_t *spri;
			int blankheadspri;
			int light_spri[MAXLIGHTS], light_sprinum;
		};
	};
	// end of map storage
	playerstruct_t p[MAXPLAYERS];

	int rseed;
#ifdef STANDALONE
		//typemess&nick are not in the player structure to make gamestate_crc32 easier to optimize
	char typemess[MAXPLAYERS][TYPEMESSLENG], nick[MAXPLAYERS][64];
	char chatmess[TYPEMESSNUM][TYPEMESSLENG], chatmessowner[TYPEMESSNUM];
	int chatmessn;
#endif
} gamestate_t;




	//Build2 shared global variables:


#pragma pack(pop)

	//General functions:
long build2_init ();
void build2_uninit ();

	//Map functions:
int build2_loadmap (const char *filnam, int *cursect,
									dpoint3d *npos, dpoint3d *nrig, dpoint3d *ndow, dpoint3d *nfor);
void build2_copygamestate (gamestate_t *dst, gamestate_t *src);
double getslopez (sect_t *s, int i, double x, double y);
int wallprev (sect_t *s, int w);
void getcentroid (wall_t *wal, int n, float *retcx, float *retcy);
float getarea (wall_t *wal, int n);
void reversewalls (wall_t *wal, int n);
void rotatewallsurfsleft1 (wall_t *wal, int n);
void dragpoint (gamestate_t *lst, int s, int w, float x, float y);
void delwall (sect_t *s, int w);
//extern int dupwall (sect_t *s, int w);
//extern void delsect (int s);
int insidesect (double x, double y, wall_t *wal, int w);
//extern void updatesect (float x, float y, float z, int *cursect);
int polybool (wall_t *wal0, int n0, wall_t *wal1, int n1, wall_t **wal2, int *n2, int op);
void checknextwalls ();
long gettileind (char *filnam);
long settilefilename (long hitsect, long hitwall, char *filnam);
void delspri (int i);
long insspri (int sect, float x, float y, float z);
void changesprisect (int i, int nsect);
void checksprisect (int s); //s: sector of sprites to check; -1 to check all sprites

	 //Render functions:
void build2_render (tiletype *dd, long lzbufoff, long cursect,
									dpoint3d *npos, dpoint3d *nrig, dpoint3d *ndow, dpoint3d *nfor,
									double hx, double hy, double hz);
void drawpol (cam_t *cc, kgln_t *vert, long num, tile_t *tpic, long curcol);
int isvispol (cam_t *cc, kgln_t *vert, long num);
void drawsectfill3d (cam_t *cc, sect_t *sec, int isflor, int col);
void drawsprite (cam_t *cc, spri_t *spr);
void drawline3d (cam_t *cc, float x0, float y0, float z0, float x1, float y1, float z1, long col);
	//Physics functions:
int build2_hitmove (int *cursect, dpoint3d *p, dpoint3d *v, double cr, int isslide, int *hitsect, int *hitwall);
int hitscan (point3d *p0, point3d *pv, float vscale, int cursect, int *hitsect, int *hitwall, point3d *hit);
//extern double findmaxcr (dpoint3d *p0, int cursect, double mindist, dpoint3d *hit);

	//Math functions:
void orthofit3x3 (point3d *v0, point3d *v1, point3d *v2);
void orthorotate (double ox, double oy, double oz,  point3d *iri,  point3d *ido,  point3d *ifo);
void orthorotate (double ox, double oy, double oz, dpoint3d *iri, dpoint3d *ido, dpoint3d *ifo);
void slerp (point3d *irig,  point3d *idow,  point3d *ifor,
						 point3d *irig2, point3d *idow2, point3d *ifor2,
						 point3d *iri,   point3d *ido,   point3d *ifo,   float rat);

//Disk functions:
void savemap (char *filnam);
void savekc (char *filnam);
void screencapture (tiletype *tt);

#endif
