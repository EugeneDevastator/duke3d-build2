//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------
#ifndef GLOBAL_H
#define GLOBAL_H
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <io.h>
#include "duke3d.h"
#include "mmulti.h"

#define MAX_PATH 256
#ifdef __BEOS__
#include <endian.h>
#endif


#define STUBBED(x) printf("STUB: %s in %s:%d\n", x, __FILE__, __LINE__)
char *mymembuf;
char MusicPtr[72000];

short global_random;
short neartagsector, neartagwall, neartagsprite;

long gc,neartaghitdist,lockclock,max_player_health,max_armour_amount,max_ammo_amount[MAX_WEAPONS];

// long temp_data[MAXSPRITES][6];
weaponhit hittype[MAXSPRITES]; // can remain as is.
short spriteq[1024],spriteqloc,spriteqamount;

// ported build engine has this, too.  --ryan.
#if PLATFORM_DOS
short moustat = 0;
#endif

animwalltype animwall[MAXANIMWALLS];
short numanimwalls;
long *animateptr[MAXANIMATES], animategoal[MAXANIMATES], animatevel[MAXANIMATES];
// long oanimateval[MAXANIMATES];
short animatesect[MAXANIMATES];
long msx[2048],msy[2048];
short cyclers[MAXCYCLERS][6],numcyclers;

char fta_quotes[NUMOFFIRSTTIMEACTIVE][64];

unsigned char  packbuf[576]; // tempbuf[2048], in engine,

char buf[80];

short camsprite;
short mirrorwall[64], mirrorsector[64], mirrorcnt;

int current_menu;

char betaname[80];

char level_names[44][33],level_file_names[44][128];
long partime[44],designertime[44];
char volume_names[4][33];
char skill_names[5][33];

volatile long checksume;
long soundsiz[NUM_SOUNDS];

short soundps[NUM_SOUNDS],soundpe[NUM_SOUNDS],soundvo[NUM_SOUNDS];
char soundm[NUM_SOUNDS],soundpr[NUM_SOUNDS];
char sounds[NUM_SOUNDS][14];

short title_zoom;

fx_device device;

SAMPLE Sound[ NUM_SOUNDS ];
SOUNDOWNER SoundOwner[NUM_SOUNDS][4];

char numplayersprites,loadfromgrouponly,earthquaketime;

long fricxv,fricyv;
player_orig po[MAXPLAYERS];
player_struct ps[MAXPLAYERS];
//extern user_defs ud;

char pus, pub;
long syncvalhead[MAXPLAYERS], syncvaltail, syncvaltottail;

input sync[MAXPLAYERS], loc;
input recsync[RECSYNCBUFSIZ];
long avgfvel, avgsvel, avgavel, avghorz, avgbits;


input inputfifo[MOVEFIFOSIZ][MAXPLAYERS];
input recsync[RECSYNCBUFSIZ];

long movefifosendplc;

  //Multiplayer syncing variables
short screenpeek;
long movefifoend[MAXPLAYERS];


    //Game recording variables

//char playerreadyflag[MAXPLAYERS],ready2send;
char playerquitflag[MAXPLAYERS];
long vel, svel, angvel, ototalclock, respawnactortime, respawnitemtime, groupfile;

long script[MAXSCRIPTSIZE],*scriptptr,*insptr,*labelcode,labelcnt;
long *actorscrptr[MAXTILES],*parsing_actor;
char *label,*textptr,error,warning,killit_flag;
char *music_pointer;
char actortype[MAXTILES];


char display_mirror,typebuflen,typebuf[41];

char music_fn[4][11][13],music_select;
char env_music_fn[4][13];
char rtsplaying;


short weaponsandammosprites[15];

long impact_damage;

        //GLOBAL.C - replace the end "my's" with this
long myx, omyx, myxvel, myy, omyy, myyvel, myz, omyz, myzvel;
short myhoriz, omyhoriz, myhorizoff, omyhorizoff;
short myang, omyang, mycursectnum, myjumpingcounter,frags[MAXPLAYERS][MAXPLAYERS];

//GAME.C sync state variables
static char syncstat, othersyncval[MOVEFIFOSIZ];
//static long syncvaltottail, othersyncvalhead, syncvaltail;



char myjumpingtoggle, myonground, myhardlanding, myreturntocenter;
signed char multiwho, multipos, multiwhat, multiflag;

long fakemovefifoplc,movefifoplc;
long myxbak[MOVEFIFOSIZ], myybak[MOVEFIFOSIZ], myzbak[MOVEFIFOSIZ];
long myhorizbak[MOVEFIFOSIZ],dukefriction, show_shareware;

short myangbak[MOVEFIFOSIZ];
char myname[32],camerashitable,freezerhurtowner,lasermode;
// CTW - MODIFICATION
// char networkmode = 255, movesperpacket = 1,gamequit = 0,playonten = 0,everyothertime;
char networkmode, movesperpacket,gamequit,everyothertime;
// CTW END - MODIFICATION
long rpgblastradius,pipebombblastradius,tripbombblastradius,shrinkerblastradius,morterblastradius,bouncemineblastradius,seenineblastradius;
STATUSBARTYPE sbar;


extern int _argc;
extern char **_argv;
static char ApogeePath[256];

// portability stuff.  --ryan.
// A good portion of this was ripped from GPL'd Rise of the Triad.  --ryan.
void Error (char *error, ...);
void FixFilePath(char *filename);


#if PLATFORM_DOS
 /* no-op. */

#elif PLATFORM_WIN32
int _dos_findfirst(char *filename, int x, struct find_t *f)
{
    long rc = _findfirst(filename, &f->data);
    f->handle = rc;
    if (rc != -1)
    {
        strncpy(f->name, f->data.name, sizeof (f->name) - 1);
        f->name[sizeof (f->name) - 1] = '\0';
        return(0);
    }
    return(1);
}

int _dos_findnext(struct find_t *f)
{
    int rc = 0;
    if (f->handle == -1)
        return(1);   /* invalid handle. */

    rc = _findnext(f->handle, &f->data);
    if (rc == -1)
    {
        _findclose(f->handle);
        f->handle = -1;
        return(1);
    }

    strncpy(f->name, f->data.name, sizeof (f->name) - 1);
    f->name[sizeof (f->name) - 1] = '\0';
    return(0);
}

#elif PLATFORM_UNIX 
int _dos_findfirst(char *filename, int x, struct find_t *f)
{
    char *ptr;

    if (strlen(filename) >= sizeof (f->pattern))
        return(1);

    strcpy(f->pattern, filename);
    FixFilePath(f->pattern);
    ptr = strrchr(f->pattern, PATH_SEP_CHAR);

    if (ptr == NULL)
    {
        ptr = filename;
        f->dir = opendir(CURDIR);
    }
    else
    {
        *ptr = '\0';
        f->dir = opendir(f->pattern);
        memmove(f->pattern, ptr + 1, strlen(ptr + 1) + 1);
    }

    return(_dos_findnext(f));
}

static int check_pattern_nocase(const char *x, const char *y)
{
    if ((x == NULL) || (y == NULL))
        return(0);  /* not a match. */

    while ((*x) && (*y))
    {
        if (*x == '*')
        {
            x++;
            while (*y != '\0')
            {
                if (toupper((int) *x) == toupper((int) *y))
                    break;
                y++;
            }
        }

        else if (*x == '?')
        {
            if (*y == '\0')
                return(0);  /* anything but EOS is okay. */
        }

        else
        {
            if (toupper((int) *x) != toupper((int) *y))
                return(0);  /* not a match. */
        }

        x++;
        y++;
    }

    return(*x == *y);  /* it's a match (both should be EOS). */
}

int _dos_findnext(struct find_t *f)
{
    dirent *dent;

    if (f->dir == NULL)
        return(1);  /* no such dir or we're just done searching. */

    while ((dent = readdir(f->dir)) != NULL)
    {
        if (check_pattern_nocase(f->pattern, dent->d_name))
        {
            if (strlen(dent->d_name) < sizeof (f->name))
            {
                strcpy(f->name, dent->d_name);
                return(0);  /* match. */
            }
        }
    }

    closedir(f->dir);
    f->dir = NULL;
    return(1);  /* no match in whole directory. */
}
#else
//#error please define for your platform.
#define PATH_SEP_STR "/"
#endif
typedef struct
{
	unsigned char day;
	unsigned char month;
	unsigned int year;
	unsigned char dayofweek;
} dosdate_t;

#if !PLATFORM_DOS
void _dos_getdate(dosdate_t *date);
#endif


long FindDistance2D(int ix, int iy);

int32_t FindDistance3D(int ix, int iy, int iz);


#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#endif

#if PLATFORM_DOS
#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif
#endif

#ifdef PLATFORM_WIN32
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#ifdef _LITTLE_ENDIAN
#define BYTE_ORDER LITTLE_ENDIAN
#elif defined(_BIG_ENDIAN)
#define BYTE_ORDER BIG_ENDIAN
#endif

#ifndef BYTE_ORDER

//#error Please define your platform.

#endif


short SwapShort (short l);

short	KeepShort (short l);


long	SwapLong (long l);

long	KeepLong (long l);


#undef KeepShort
#undef KeepLong
#undef SwapShort
#undef SwapLong

void SwapIntelLong(long *l);

void SwapIntelShort(short *s);

void SwapIntelLongArray(long *l, int num);

void SwapIntelShortArray(short *s, int num);


/* 
  Copied over from Wolf3D Linux: http://www.icculus.org/wolf3d/
  Modified for ROTT.
  Stolen for Duke3D, too.
 */
 


int setup_homedir ();


int dukescreencapture(char *str, char inverseit);


char CheckParm (char *check);


void RegisterShutdownFunction( void (* shutdown) () );

void Shutdown();


/*
 * From Ryan's buildengine CHANGELOG:
 *  Removed global var: cachedebug in engine.c, and put #define
 *  BUILD_CACHEDEBUG 0 at the top of the source. Flip it to 1 if you ever
 *  need to tinker in the cache code.
 */
//char cachedebug = 0;

#endif