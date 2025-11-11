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

#ifndef DUKE3DMAIN_H
#define DUKE3DMAIN_H


//#include <bios.h>

#include "build.h"
#include "engine.h"
#include "function.h"
#include "mmulti.h"
#include "pragmas.h"
#include "funct.h"
#define VOLUMEALL
#define PLUTOPAK
// #define VOLUMEONE
// #define ONELEVELDEMO

// #define TEN
// #define BETA

// #define AUSTRALIA

#define MAXSLEEPDIST  16384
#define SLEEPTIME 24*64

#ifdef VOLUMEONE
    #define BYTEVERSION 27
#else
    #define BYTEVERSION 116
#endif

#define NUMPAGES 1

#define AUTO_AIM_ANGLE          48
#define RECSYNCBUFSIZ 2520   //2520 is the (LCM of 1-8)*3
#define MOVEFIFOSIZ 256

#define FOURSLEIGHT (1<<8)

//#include "types.h"
#include "file_lib.h"
//#include "develop.h"
#include "gamedefs.h"
#include "keyboard.h"
//#include "util_lib.h"
#include "config.h"
#include "function.h"
#include "fx_man.h"
//#include "sounds.h"
#include "control.h"
//#include "_rts.h"
//#include "rts.h"
#include "soundefs.h"


#include "task_man.h"
//#include "music.h"
//#include "sndcards.h"

#include "names.h"


#define TICRATE (120)
#define TICSPERFRAME (TICRATE/26)

// #define GC (TICSPERFRAME*44)

#define NUM_SOUNDS 450

int sgn(int value);

#define    ALT_IS_PRESSED ( KB_KeyPressed( sc_RightAlt ) || KB_KeyPressed( sc_LeftAlt ) )
#define    SHIFTS_IS_PRESSED ( KB_KeyPressed( sc_RightShift ) || KB_KeyPressed( sc_LeftShift ) )
#define    RANDOMSCRAP EGS(s->sectnum,s->x+(TRAND&255)-128,s->y+(TRAND&255)-128,s->z-(8<<8)-(TRAND&8191),SCRAP6+(TRAND&15),-8,48,48,TRAND&2047,(TRAND&63)+64,-512-(TRAND&2047),i,5)

#define    BLACK 0
#define    DARKBLUE 1
#define    DARKGREEN 2
#define    DARKCYAN 3
#define    DARKRED 4
#define    DARKPURPLE 5
#define    BROWN 6
#define    LIGHTGRAY 7

#define    DARKGRAY 8
#define    BLUE 9
#define    GREEN 10
#define    CYAN 11
#define    RED 12
#define    PURPLE 13
#define    YELLOW 14
#define    WHITE 15

#define    PHEIGHT (38<<8)

// #define P(X) printf("%ld\n",X);

#define WAIT(X) ototalclock=totalclock+(X);while(totalclock<ototalclock)


#define MODE_MENU       1
#define MODE_DEMO       2
#define MODE_GAME       4
#define MODE_EOL        8
#define MODE_TYPE       16
#define MODE_RESTART    32
#define MODE_SENDTOWHOM 64
#define MODE_END        128


#define MAXANIMWALLS 512
#define MAXINTERPOLATIONS 2048
#define NUMOFFIRSTTIMEACTIVE 192

#define MAXCYCLERS 256
#define MAXSCRIPTSIZE 20460
#define MAXANIMATES 64

#define face_player 1
#define geth 2
#define getv 4
#define random_angle 8
#define face_player_slow 16
#define spin 32
#define face_player_smart 64
#define fleeenemy 128
#define jumptoplayer 257
#define seekplayer 512
#define furthestdir 1024
#define dodgebullet 4096

#define TRAND krand()




#define KNEE_WEAPON          0
#define PISTOL_WEAPON        1
#define SHOTGUN_WEAPON       2
#define CHAINGUN_WEAPON      3
#define RPG_WEAPON           4
#define HANDBOMB_WEAPON      5
#define SHRINKER_WEAPON      6
#define DEVISTATOR_WEAPON    7
#define TRIPBOMB_WEAPON      8
#define FREEZE_WEAPON        9
#define HANDREMOTE_WEAPON    10
#define GROW_WEAPON          11

#define T1  hittype[i].temp_data[0]
#define T2  hittype[i].temp_data[1]
#define T3  hittype[i].temp_data[2]
#define T4  hittype[i].temp_data[3]
#define T5  hittype[i].temp_data[4]
#define T6  hittype[i].temp_data[5]

#define ESCESCAPE if(KB_KeyPressed( sc_Escape ) ) gameexit(" ");

#define IFWITHIN(B,E) if((sprite[i].picnum)>=(B) && (sprite[i].picnum)<=(E))
#define KILLIT(KX) {deletesprite(KX);goto BOLT;}


#define IFMOVING if(ssp(i,CLIPMASK0))
#define IFHIT j=ifhitbyweapon(i);if(j >= 0)
#define IFHITSECT j=ifhitsectors(s->sectnum);if(j >= 0)

#define AFLAMABLE(X) (X==BOX||X==TREE1||X==TREE2||X==TIRE||X==CONE)


#define IFSKILL1 if(player_skill<1)
#define IFSKILL2 if(player_skill<2)
#define IFSKILL3 if(player_skill<3)
#define IFSKILL4 if(player_skill<4)

#define rnd(X) ((TRAND>>8)>=(255-(X)))

typedef struct
{
    short i;
    int voice;
} SOUNDOWNER;

#define __USRHOOKS_H

enum USRHOOKS_Errors
   {
   USRHOOKS_Warning = -2,
   USRHOOKS_Error   = -1,
   USRHOOKS_Ok      = 0
   };



extern input inputfifo[MOVEFIFOSIZ][MAXPLAYERS], sync[MAXPLAYERS];
extern input recsync[RECSYNCBUFSIZ];

extern long movefifosendplc;

typedef struct
{
    char *ptr;
    volatile char lock;
    int  length, num;
} SAMPLE;

typedef struct
{
        short wallnum;
        long tag;
}animwalltype;
extern animwalltype animwall[MAXANIMWALLS];
extern short numanimwalls,probey,lastprobey;

extern char *mymembuf;
extern char typebuflen,typebuf[41];
extern char MusicPtr[72000];
extern long msx[2048],msy[2048];
extern short cyclers[MAXCYCLERS][6],numcyclers;
extern char myname[32];

typedef struct
{
    char god,warp_on,cashman,eog,showallmap;
    char show_help,scrollmode,clipping;
    char user_name[MAXPLAYERS][32];
    char ridecule[10][40];
    char savegame[10][22];
    char pwlockout[128],rtsname[128];
    char overhead_on,last_overhead,showweapons;

    short pause_on,from_bonus;
    short camerasprite,last_camsprite;
    short last_level,secretlevel;

    long const_visibility,uw_framerate;
    long camera_time,folfvel,folavel,folx,foly,fola;
    long reccnt;

    int32_t entered_name,screen_tilting,shadows,fta_on,executions,auto_run;
    int32_t coords,tickrate,m_coop,coop,screen_size,lockout,crosshair;
    int32_t wchoice[MAXPLAYERS][MAX_WEAPONS],playerai;

    int32_t respawn_monsters,respawn_items,respawn_inventory,recstat,monsters_off,brightness;
    int32_t m_respawn_items,m_respawn_monsters,m_respawn_inventory,m_recstat,m_monsters_off,detail;
    int32_t m_ffire,ffire,m_player_skill,m_level_number,m_volume_number,multimode;
    int32_t player_skill,level_number,volume_number,m_marker,marker,mouseflip;

} user_defs;

typedef struct
{
    long ox,oy,oz;
    short oa,os;
} player_orig;


extern char numplayersprites;
extern char picsiz[MAXTILES];

void add_ammo( short, short, short, short );


extern long fricxv,fricyv;



//same type, is in engine.h
//extern unsigned char tempbuf[2048];
extern unsigned char packbuf[576];

extern long gc,max_player_health,max_armour_amount,max_ammo_amount[MAX_WEAPONS];

extern long impact_damage;

#define MOVFIFOSIZ 256

extern short spriteq[1024],spriteqloc,spriteqamount;
extern player_struct ps[MAXPLAYERS];
extern player_orig po[MAXPLAYERS];
extern user_defs ud;
extern short int moustat;
extern short int global_random;
extern long scaredfallz;
extern char buf[80]; //My own generic input buffer

extern char fta_quotes[NUMOFFIRSTTIMEACTIVE][64];
extern char scantoasc[128],ready2send;
extern char scantoascwithshift[128];

//extern fx_device device;
extern SAMPLE Sound[ NUM_SOUNDS ];
extern int32_t VoiceToggle,AmbienceToggle;
extern SOUNDOWNER SoundOwner[NUM_SOUNDS][4];

extern char playerreadyflag[MAXPLAYERS],playerquitflag[MAXPLAYERS];
extern char sounds[NUM_SOUNDS][14];

extern long script[MAXSCRIPTSIZE],*scriptptr,*insptr,*labelcode,labelcnt;
extern char *label,*textptr,error,warning,killit_flag;
extern long *actorscrptr[MAXTILES],*parsing_actor;
extern char actortype[MAXTILES];
extern char *music_pointer;

extern char ipath[80],opath[80];

extern char music_fn[4][11][13],music_select;
extern char env_music_fn[4][13];
extern short camsprite;

// extern char gotz;
extern char inspace(short sectnum);


typedef struct
{
    char cgg;
    short picnum,ang,extra,owner,movflag;
    short tempang,actorstayput,dispicnum;
    short timetosleep;
    long floorz,ceilingz,lastvx,lastvy,bposx,bposy,bposz;
    long temp_data[6];
} weaponhit;

extern weaponhit hittype[MAXSPRITES];

extern input loc;
extern input recsync[RECSYNCBUFSIZ];
extern long avgfvel, avgsvel, avgavel, avghorz, avgbits;

extern short numplayers, myconnectindex;
extern short connecthead, connectpoint2[MAXPLAYERS];   //Player linked list variables (indeces, not connection numbers)
extern short screenpeek;

extern int current_menu;
extern long tempwallptr,animatecnt;
extern long lockclock,frameplace;
extern char display_mirror,loadfromgrouponly,rtsplaying;

extern long movefifoend[MAXPLAYERS], groupfile;
extern long ototalclock;

extern long *animateptr[MAXANIMATES], animategoal[MAXANIMATES];
extern long animatevel[MAXANIMATES];
// extern long oanimateval[MAXANIMATES];
extern short neartagsector, neartagwall, neartagsprite;
extern long neartaghitdist;
extern short animatesect[MAXANIMATES];
extern long movefifoplc, vel,svel,angvel,horiz;

extern short mirrorwall[64], mirrorsector[64], mirrorcnt;

extern void TestCallBack(unsigned long);

#define NUMKEYS 19

extern long frameplace, chainplace, chainnumpages;
extern volatile long checksume;

extern char screencapt;
extern short soundps[NUM_SOUNDS],soundpe[NUM_SOUNDS],soundvo[NUM_SOUNDS];
extern char soundpr[NUM_SOUNDS],soundm[NUM_SOUNDS];
extern long soundsiz[NUM_SOUNDS];
extern char level_names[44][33];
extern long partime[44],designertime[44];
extern char volume_names[4][33];
extern char skill_names[5][33];
extern char level_file_names[44][128];

extern int32_t SoundToggle,MusicToggle;
extern short last_threehundred,lastsavedpos;
extern char restorepalette;

extern short buttonstat;

extern char boardfilename[128],waterpal[768],slimepal[768],titlepal[768],drealms[768],endingpal[768];
extern char betaname[80];
extern char cachedebug,earthquaketime;
extern char networkmode;
//extern char lumplockbyte[11];

    //DUKE3D.H - replace the end "my's" with this
extern long myx, omyx, myxvel, myy, omyy, myyvel, myz, omyz, myzvel;
extern short myhoriz, omyhoriz, myhorizoff, omyhorizoff, globalskillsound;
extern short myang, omyang, mycursectnum, myjumpingcounter;
extern char myjumpingtoggle, myonground, myhardlanding,myreturntocenter;
extern long fakemovefifoplc;
extern long myxbak[MOVEFIFOSIZ], myybak[MOVEFIFOSIZ], myzbak[MOVEFIFOSIZ];
extern long myhorizbak[MOVEFIFOSIZ];
extern short myangbak[MOVEFIFOSIZ];

extern short weaponsandammosprites[15];


typedef struct
{
        short frag[MAXPLAYERS], got_access, last_extra, shield_amount, curr_weapon;
        short ammo_amount[MAX_WEAPONS], holoduke_on;
        char gotweapon[MAX_WEAPONS], inven_icon, jetpack_on, heat_on;
        short firstaid_amount, steroids_amount, holoduke_amount, jetpack_amount;
        short heat_amount, scuba_amount, boot_amount;
        short last_weapon, weapon_pos, kickback_pic;

} STATUSBARTYPE;

extern STATUSBARTYPE sbar;
extern short frags[MAXPLAYERS][MAXPLAYERS];
extern long cameradist, cameraclock, dukefriction,show_shareware;
extern char networkmode, movesperpacket;
extern char gamequit;

extern char pus,pub,camerashitable,freezerhurtowner,lasermode;
extern char syncstat, syncval[MAXPLAYERS][MOVEFIFOSIZ];
extern signed char multiwho, multipos, multiwhat, multiflag;
extern long syncvalhead[MAXPLAYERS], syncvaltail, syncvaltottail;
extern long numfreezebounces,rpgblastradius,pipebombblastradius,tripbombblastradius,shrinkerblastradius,morterblastradius,bouncemineblastradius,seenineblastradius;
// CTW - MODIFICATION
// extern char stereo,eightytwofifty,playerswhenstarted,playonten,everyothertime;
extern char stereo,eightytwofifty,playerswhenstarted,everyothertime;
// CTW END - MODIFICATION
extern long myminlag[MAXPLAYERS], mymaxlag, otherminlag, bufferjitter;

extern long numinterpolations, startofdynamicinterpolations;
extern long oldipos[MAXINTERPOLATIONS];
extern long bakipos[MAXINTERPOLATIONS];
extern long *curipos[MAXINTERPOLATIONS];

extern short numclouds,clouds[128],cloudx[128],cloudy[128];
extern long cloudtotalclock,totalmemory;

//extern long stereomode, stereowidth, stereopixelwidth;

extern long myaimmode, myaimstat, omyaimstat;

#endif
