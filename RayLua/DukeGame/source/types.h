//
// Created by omnis on 11/4/2025.
//

#ifndef GAME_TYPES_H
#define GAME_TYPES_H
#include <stdint.h>
#define MAX_WEAPONS  12
/*
 *Sprite statnum
 *Yes 	0 	STAT_DEFAULT 	Sprites that are not defined by the CON code as actors, and are not projectiles, etc, have a statnum of 0. (i.e. the floor texture sprites used to make up a sprite bridge)
Yes 	1 	STAT_ACTOR 	Actors. Sprites with a statnum of 1 will execute the actor code that applies to their tile number in the CON scripts.
Yes 	2 	STAT_ZOMBIEACTOR 	Sleepers. Sprites taking a break from code execution (e.g. a Pig Cop that has been left alone for long enough will revert to statnum 2, only waking up and going back to statnum 1 upon seeing the player again)
Yes 	3 	STAT_EFFECTOR
Yes 	4 	STAT_PROJECTILE 	Projectiles. This includes hardcoded projectiles like RPG, FREEZEBLAST, and SHRINKSPARK, as well as custom projectiles. It does not include hitscan projectiles (bullets), since those are not sprites that exist in the game world. (SHOTSPARK1, the sprite spawned by bullets, is not itself a projectile.)
Yes 	5 	STAT_MISC
Yes 	6 	STAT_STANDABLE 	On spawn, the following sprites have statnum 6: BOLT1+, SIDEBOLT1+, VIEWSCREEN, VIEWSCREEN2, CRANE, TRASH, WATERDRIP, WATERDRIPSPLASH, PLUG, WATERBUBBLEMAKER, MASTERSWITCH, DOORSHOCK, TREE1, TREE2, TIRE, CONE, BOX, FLOORFLAME, CEILINGSTEAM, OOZFILTER, CRACK1-CRACK4, FIREEXT, TOILETWATER.

A few other ones become STANDABLE on seeing the player (changing from ZOMBIEACTOR): RUBBERCAN, EXPLODINGBARREL, WOODENHORSE, HORSEONSIDE, CANWITHSOMETHING, CANWITHSOMETHING2-4, FIREBARREL, FIREVASE, NUKEBARREL, NUKEBARRELDENTED, NUKEBARRELLEAKED, TRIPBOMB.
Yes 	7 	STAT_LOCATOR
Yes 	8 	STAT_ACTIVATOR
Yes 	9 	STAT_TRANSPORT
Yes 	10 	STAT_PLAYER 	Player and Holoduke
Yes 	11 	STAT_FX 	RESPAWN, MUSICANDSFX
Yes 	12 	STAT_FALLER 	Decorative sprites that have a nonzero hitag to make them destructible are assigned to the FALLER statnum.
Yes 	13 	STAT_DUMMYPLAYER
Yes 	14 	STAT_LIGHT
No 	99 	TSPR_TEMP 	A tspr will have this when it is a shadow cast by an actor.
No 	100 	STAT_MIRROREDACTOR 	A tspr will have this when it is part of a mirror reflection.
Yes 	1024 	MAXSTATUS 	A sprite id with this statnum is invalid, meaning that it has been deleted or just never existed in the map. Do NOT try to destroy a sprite by setting this value on it, instead use the killit command or else try setting the sprite's xrepeat to zero.
 */
typedef struct
{
    long x, y;
    short point2, nextwall, nextsector, cstat;
    short picnum, overpicnum;
    signed char shade;
    char pal, xrepeat, yrepeat, xpanning, ypanning;
    short lotag, hitag, extra;
} walltype;

typedef struct
{
    long x, y, z;
    short cstat, picnum;
    signed char shade;
    char pal, clipdist, filler;
    unsigned char xrepeat, yrepeat;
    signed char xoffset, yoffset;
    short sectnum, statnum;
    short ang, owner, xvel, yvel, zvel;
    short lotag, hitag, extra;
} spritetype;
typedef struct
{
    short wallptr, wallnum;
    long ceilingz, floorz;
    short ceilingstat, floorstat;
    short ceilingpicnum, ceilingheinum;
    signed char ceilingshade;
    char ceilingpal, ceilingxpanning, ceilingypanning;
    short floorpicnum, floorheinum;
    signed char floorshade;
    char floorpal, floorxpanning, floorypanning;
    char visibility, filler;
    short lotag, hitag, extra;
} sectortype;
typedef enum
{
    analog_turning=0,
    analog_strafing=1,
    analog_lookingupanddown=2,
    analog_elevation=3,
    analog_rolling=4,
    analog_moving=5,
    analog_maxtype
    } analogcontrol;
typedef struct
{
    signed char avel, horz;
    short fvel, svel;
    unsigned long bits;
} input;
typedef struct
{
    long zoom,exitx,exity,loogiex[64],loogiey[64],numloogs,loogcnt;

    /*Position Variables:

    oposx, oposy, oposz - Old/Previous position coordinates (x, y, z from last frame)
    posx, posy, posz - Current position coordinates
    posxv, posyv, poszv - Position velocity (movement speed in each axis)
    Bob Variables:

    bobposx, bobposy - Head bobbing position offsets (weapon/view sway when walking)
    bobcounter - Counter for bobbing animation timing
    Vertical Offset Variables:

    pyoff, opyoff - Player Y offset and old player Y offset (likely for crouching/jumping)*/

    long posx, posy, posz, horiz, ohoriz, ohorizoff, invdisptime;
    long bobposx,bobposy,oposx,oposy,oposz,pyoff,opyoff;
    long posxv,posyv,poszv,last_pissed_time,truefz,truecz;
    long player_par,visibility;
    long bobcounter,weapon_sway;
    long pals_time,randomflamex,crack_time;

    int32_t aim_mode;

    short ang,oang,angvel,cursectnum,look_ang,last_extra,subweapon;
    short ammo_amount[MAX_WEAPONS],wackedbyactor,frag,fraggedself;

    short curr_weapon, last_weapon, tipincs, horizoff, wantweaponfire;
    short holoduke_amount,newowner,hurt_delay,hbomb_hold_delay;
    short jumping_counter,airleft,knee_incs,access_incs;
    short fta,ftq,access_wallnum,access_spritenum;
    short kickback_pic,got_access,weapon_ang,firstaid_amount;
    short somethingonplayer,on_crane;
// index of playe's sprite in all sprites on map
    short i;
    short one_parallax_sectnum;
    short over_shoulder_on,random_club_frame,fist_incs;
    short one_eighty_count,cheat_phase;
    short dummyplayersprite,extra_extra8,quick_kick;
    short heat_amount,actorsqu,timebeforeexit,customexitsound;

    short weaprecs[16],weapreccnt,interface_toggle_flag;

    short rotscrnang,dead_flag,show_empty_weapon;
    short scuba_amount,jetpack_amount,steroids_amount,shield_amount;
    short holoduke_on,pycount,weapon_pos,frag_ps;
    short transporter_hold,last_full_weapon,footprintshade,boot_amount;

    int scream_voice;

    char gm,on_warping_sector,footprintcount;
    char hbomb_on,jumping_toggle,rapid_fire_hold,on_ground;
    char name[32],inven_icon,buttonpalette;

    char jetpack_on,spritebridge,lastrandomspot;
    char scuba_on,footprintpal,heat_on;

    char  holster_weapon,falling_counter;
    char  gotweapon[MAX_WEAPONS],refresh_inventory,*palette;

    char toggle_key_flag,knuckle_incs; // ,select_dir;
    char walking_snd_toggle, palookup, hard_landing;
    char max_secret_rooms,secret_rooms,/*fire_flag,*/pals[3];
    char max_actors_killed,actors_killed,return_to_center;
}  player_struct;
#endif //GAME_TYPES_H