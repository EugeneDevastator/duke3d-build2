//
// Created by omnis on 11/4/2025.
//

#ifndef GAME_TYPES_H
#define GAME_TYPES_H
#include <stdint.h>
#define MAX_WEAPONS  12
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