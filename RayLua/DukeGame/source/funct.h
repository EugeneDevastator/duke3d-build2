//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment

This file is NOT part of Duke Nukem 3D version 1.5 - Atomic Edition
However, it is either an older version of a file that is, or is
some test code written during the development of Duke Nukem 3D.
This file is provided purely for educational interest.

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

Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------
#ifndef FUNCT_H
#define FUNCT_H
#include "types.h"
#include "task_man.h"

//extern void displayinventory(player_struct *p);
extern void displayfragbar();
extern int gametext(int x,int y,char *t,char s,short dabits);
extern int gametextpal(int x,int y,char *t,char s,char p);
extern int gametextpart(int x,int y,char *t,char s,short p);
extern int minitext(int x,int y,char *t,char p,char sb);
extern int minitextshade(int x,int y,char *t,char s,char p,char sb);
extern void weaponnum(short ind,long x,long y,long num1, long num2,char ha);
extern void weaponnum999(char ind,long x,long y,long num1, long num2,char ha);
extern void weapon_amounts(player_struct* p,long x,long y,long u);
//C:\Eugene\Games\build2-git\RayLua\DukeGame\source\funct.h(39): error C2143: syntax error: missing ')' before '*'
extern void coolgaugetext(short snum);
extern void sendscore(char *s);
extern void computergetinput(long snum, input *syn);
/*
//#line "sounds.c" 25
extern void SoundStartup();
//#line "sounds.c" 95
extern void SoundShutdown();
//#line "sounds.c" 118
extern void MusicStartup();
//#line "sounds.c" 166
extern void MusicShutdown();
//#line "sounds.c" 181
extern int USRHOOKS_GetMem(char **ptr,unsigned long size);
//#line "sounds.c" 192
extern int USRHOOKS_FreeMem(char *ptr);
//#line "sounds.c" 200
extern void intomenusounds();
//#line "sounds.c" 227
extern void playmusic(char *fn);
//#line "sounds.c" 251
extern char loadsound(unsigned short num);
//#line "sounds.c" 277
extern int xyzsound(short num,short i,long x,long y,long z);
//#line "sounds.c" 407
extern void sound(short num);
//#line "sounds.c" 463
extern int spritesound(unsigned short num,short i);
//#line "sounds.c" 469
extern void stopsound(short num);
//#line "sounds.c" 478
extern void stopenvsound(short num,short i);
//#line "sounds.c" 494
extern void pan3dsound();
//#line "sounds.c" 571
*/
//extern void TestCallBack(long num);
//#line "sector.c" 9
extern short callsound(short sn,short whatsprite);
//#line "sector.c" 56
extern short check_activator_motion(short lotag);
//#line "sector.c" 93
extern char isadoorwall(short dapic);
//#line "sector.c" 124
extern char isanunderoperator(short lotag);
//#line "sector.c" 139
extern char isanearoperator(short lotag);
//#line "sector.c" 161
extern short checkcursectnums(short sect);
//#line "sector.c" 169
extern long ldist(spritetype *s1,spritetype *s2);
//#line "sector.c" 177
extern long dist(spritetype *s1,spritetype *s2);
//#line "sector.c" 186
extern short findplayer(spritetype *s,long *d);
//#line "sector.c" 214
extern short findotherplayer(short p,long *d);
//#line "sector.c" 240
extern void doanimations();
//#line "sector.c" 301
extern long getanimationgoal(long *animptr);
//#line "sector.c" 315
extern long setanimation(short animsect,long *animptr, long thegoal, long thevel);
//#line "sector.c" 348
extern void animatecamsprite();
//#line "sector.c" 369
extern void animatewalls();
//#line "sector.c" 457
extern char activatewarpelevators(short s,short d);
//#line "sector.c" 504
extern void operatesectors(short sn,short ii);
//#line "sector.c" 997
extern void operaterespawns(short low);
//#line "sector.c" 1020
extern void operateactivators(short low,short snum);
//#line "sector.c" 1089
extern void operatemasterswitches(short low);
//#line "sector.c" 1104
extern void operateforcefields(short s,short low);
//#line "sector.c" 1140
extern char checkhitswitch(short snum,long w,char switchtype);
//#line "sector.c" 1515
extern void activatebysector(short sect,short j);
//#line "sector.c" 1532
extern void checkhitwall(short spr,short dawallnum,long x,long y,long z,short atwith);
//#line "sector.c" 1746
extern void checkplayerhurt(player_struct *p,short j);
//#line "sector.c" 1816
extern char checkhitceiling(short sn);
//#line "sector.c" 1891
extern void checkhitsprite(short i,short sn);
//#line "sector.c" 2326
extern void allignwarpelevators();
//#line "sector.c" 2357
extern void cheatkeys(short snum);
//#line "sector.c" 2766

extern void checksectors(short snum);
/*
//#line "rts.c" 36
extern void RTS_AddFile(char *filename);
//#line "rts.c" 93
extern void RTS_Init(char *filename);
//#line "rts.c" 126
extern int32_t RTS_NumSounds();
//#line "rts.c" 141
extern int32_t RTS_SoundLength(int32_t lump);
//#line "rts.c" 157
extern char *RTS_GetSoundName(int32_t i);
//#line "rts.c" 174
extern void RTS_ReadLump(int32_t lump,void *dest);
//#line "rts.c" 194
extern void *RTS_GetSound(int32_t lump);
*/
//#line "premap.c" 7
extern void xyzmirror(short i,short wn);
//#line "premap.c" 20
extern void vscrn();
//#line "premap.c" 58
extern void pickrandomspot(short snum);
//#line "premap.c" 80
extern void resetplayerstats(short snum);
//#line "premap.c" 187
extern void resetweapons(short snum);
//#line "premap.c" 212
extern void resetinventory(short snum);
//#line "premap.c" 232
extern void resetprestat(short snum,char g);
//#line "premap.c" 294
extern void setupbackdrop(short backpicnum);
//#line "premap.c" 329
extern void cachespritenum(short i);
//#line "premap.c" 443
extern void cachegoodsprites();
//#line "premap.c" 516
extern void prelevel(char g);
//#line "premap.c" 823
extern void newgame(char vn,char ln,char sk);
//#line "premap.c" 874
extern void resetpspritevars(char g);
//#line "premap.c" 1012
extern void resettimevars();
//#line "premap.c" 1043
extern void genspriteremaps();
//#line "premap.c" 1077
extern void waitforeverybody();
//#line "premap.c" 1131
extern char checksum(long sum);
//#line "premap.c" 1163
extern char getsound(unsigned short num);
//#line "premap.c" 1189
extern void precachenecessarysounds();
//#line "premap.c" 1201
extern void cacheit();
//#line "premap.c" 1244
extern void dofrontscreens();
//#line "premap.c" 1285
extern void enterlevel(char g);
//#line "player.c" 10
extern void setpal(player_struct *p);
//#line "player.c" 28
extern void incur_damage(player_struct *p);
//#line "player.c" 59
extern void quickkill(player_struct *p);
//#line "player.c" 73
extern void forceplayerangle(player_struct *p);
//#line "player.c" 85
extern void tracers(long x1,long y1,long z1,long x2,long y2,long z2,long n);
//#line "player.c" 114
extern long hits(short i);
//#line "player.c" 131
extern long hitasprite(short i,short *hitsp);
//#line "player.c" 152
extern long hitawall(player_struct *p,short *hitw);
//#line "player.c" 163
extern short aim(spritetype *s,short aang);
//#line "player.c" 234
extern void shoot(short i,short atwith);
//#line "player.c" 939
extern void displayloogie(short snum);
//#line "player.c" 958
extern char animatefist(short gs,short snum);
//#line "player.c" 986
extern char animateknee(short gs,short snum);
//#line "player.c" 1002
extern char animateknuckles(short gs,short snum);
//#line "player.c" 1022
extern void displaymasks(short snum);
//#line "player.c" 1043
extern char animatetip(short gs,short snum);
//#line "player.c" 1064
extern char animateaccess(short gs,short snum);
//#line "player.c" 1090
extern void displayweapon(short snum);
//#line "player.c" 1501
extern void getinput(short snum);
//#line "player.c" 1742
extern char doincrements(player_struct *p);
//#line "player.c" 1935
extern void checkweapons(player_struct *p);
//#line "player.c" 1951

extern void processinput(short snum);
//#line "menues.c" 18
extern void cmenu(short cm);
//#line "menues.c" 38
extern void savetemp(char *fn,long daptr,long dasiz);
//#line "menues.c" 49
extern void getangplayers(short snum);
//#line "menues.c" 67
extern int loadpheader(char spot,int32_t *vn,int32_t *ln,int32_t *psk,int32_t *numplr);
//#line "menues.c" 105
extern int loadplayer(signed char spot);
//#line "menues.c" 276
extern int saveplayer(signed char spot);
//#line "menues.c" 421
extern void sendgameinfo();
//#line "menues.c" 434
extern int probe(int x,int y,int i,int n);
//#line "menues.c" 521
extern int menutext(int x,int y,short s,short p,char *t);
//#line "menues.c" 630
extern int menutextc(int x,int y,short s,short p,char *t);
//#line "menues.c" 727
extern void bar(int x,int y,short *p,short dainc,char damodify,short s,short pa);
//#line "menues.c" 806
extern void dispnames();
//#line "menues.c" 832
extern int getfilenames(char kind[]);
//#line "menues.c" 865
extern void sortfilenames();
//#line "menues.c" 886
extern void menus();
//#line "menues.c" 2414
extern void palto(char r,char g,char b,long e);
//#line "menues.c" 2436
extern void drawoverheadmap(long cposx,long cposy,long czoom,short cang);
//#line "menues.c" 2685
extern void playanm(char *fn,char);
//#line "gamedef.c" 122
extern short getincangle(short a,short na);
//#line "gamedef.c" 140
extern char ispecial(char c);
//#line "gamedef.c" 154
extern char isaltok(char c);
//#line "gamedef.c" 159
extern void getglobalz(short i);
//#line "gamedef.c" 203
extern void makeitfall(short i);
//#line "gamedef.c" 243
extern void getlabel();
//#line "gamedef.c" 267
extern long keyword();
//#line "gamedef.c" 300
extern long transword();
//#line "gamedef.c" 342
extern void transnum();
//#line "gamedef.c" 402
extern char parsecommand();
//#line "gamedef.c" 1227
extern void passone();
//#line "gamedef.c" 1239
extern void loadefs(char *fn,char *mptr);
//#line "gamedef.c" 1342
extern char dodge(spritetype *s);
//#line "gamedef.c" 1374
extern short furthestangle(short i,short angs);
//#line "gamedef.c" 1404
extern short furthestcanseepoint(short i,spritetype *ts,long *dax,long *day);
//#line "gamedef.c" 1444
extern void alterang(short a);
//#line "gamedef.c" 1512
extern void move();
//#line "gamedef.c" 1711
extern void parseifelse(long condition);
//#line "gamedef.c" 1729
extern char parse();
//#line "gamedef.c" 2699
extern void execute(short i,short p,long x);
//#line "game.c" 63
extern void overwritesprite(long thex,long they,short tilenum,signed char shade,char stat,char dapalnum);
//#line "game.c" 70
extern void timerhandler(task *);
//#line "game.c" 75
extern void inittimer();
//#line "game.c" 81
extern void uninittimer();
//#line "game.c" 89
extern int gametext(int x,int y,char *t,char s,short dabits);
//#line "game.c" 136
extern int gametextpart(int x,int y,char *t,char s,short p);
extern void lotsofmail(spritetype *s, short n);
extern void lotsofpaper(spritetype *s, short n);
extern long FindDistance2D(int32_t dx, int32_t dy);
extern int32_t FindDistance3D(int32_t dx, int32_t dy, int32_t dz);
//extern int32_t FindDistance3D_HP(int32_t dx, int32_t dy, int32_t dz);
//extern int32_t ArcTangentAppx(int32_t dx, int32_t dy);

//#line "game.c" 192
extern int minitext(int x,int y,char *t,char p,char sb);
//#line "game.c" 231
extern void gamenumber(long x,long y,long n,char s);
//#line "game.c" 248
extern void ShutDown();
//#line "game.c" 260
extern void allowtimetocorrecterrorswhenquitting();
//#line "game.c" 280
extern void getpackets();
//#line "game.c" 502
extern void faketimerhandler();
//#line "game.c" 788
extern void checksync();
//#line "game.c" 815
extern void check_fta_sounds(short i);
//#line "game.c" 869
extern short inventory(spritetype *s);
//#line "game.c" 886
extern short isBadGuy(spritetype *s);
//#line "game.c" 924
extern short badguypic(short pn);
//#line "game.c" 963
extern void myos(long x,long y,short tilenum,signed char shade,char orientation);
//#line "game.c" 976
extern void myospal(long x,long y,short tilenum,signed char shade,char orientation,char p);
//#line "game.c" 991
extern void invennum(long x,long y,char num1,char ha,char sbits);
//#line "game.c" 1021
extern void weaponnum(short ind,long x,long y,long num1,long num2,char ha);
//#line "game.c" 1049
extern void weaponnum999(char ind,long x,long y,long num1,long num2,char ha);
//#line "game.c" 1088
extern void weapon_amounts(player_struct *p,long x,long y,long u);
//#line "game.c" 1197
extern void digitalnumber(long x,long y,long n,char s,char cs);
//#line "game.c" 1223
extern void scratchmarks(long x,long y,long n,char s,char p);
//#line "game.c" 1238
extern void displayinventory(player_struct* p);

//#line "game.c" 1296
extern void displayfragbar();
//#line "game.c" 1318
extern void coolgaugetext(short snum);
//#line "game.c" 1557
extern void tics();
//#line "game.c" 1572
extern void clocks();
//#line "game.c" 1582
extern void coords(short snum);
//#line "game.c" 1607
extern void operatefta();
//#line "game.c" 1654
extern void FTA(short q,player_struct *p);
//#line "game.c" 1668
extern void showtwoscreens();
//#line "game.c" 1691
extern void binscreen();
//#line "game.c" 1705
extern void gameexit(char *t);
//#line "game.c" 1752
extern short strget(short x,short y,char *t,short dalen,short c);
//#line "game.c" 1819
extern void displayrest(long smoothratio);
//#line "game.c" 2047
extern void updatesectorz(long x,long y,long z,short *sectnum);
//#line "game.c" 2085
extern void view(player_struct *pp,long *vx,long *vy,long *vz,short *vsectnum,short ang,short horiz);
//#line "game.c" 2137
extern void drawbackground();
//#line "game.c" 2200
extern void displayrooms(short snum,long smoothratio);
//#line "game.c" 2445
extern short LocateTheLocator(short n,short sn);
//#line "game.c" 2459
extern short EGS(short whatsect,long s_x,long s_y,long s_z,short s_pn,signed char s_s,signed char s_xr,signed char s_yr,short s_a,short s_ve,long s_zv,short s_ow,signed char s_ss);
//#line "game.c" 2537
extern char wallswitchcheck(short i);
//#line "game.c" 2588
extern short spawn(short j,short pn);
//#line "game.c" 4181
extern void animatesprites(long x,long y,short a,long smoothratio);
//#line "game.c" 4859
extern void cheats();
//#line "game.c" 5303
extern void nonsharedkeys();
//#line "game.c" 5863
extern void comlinehelp(char **argv);
//#line "game.c" 5889
extern void checkcommandline(int argc,char **argv);
//#line "game.c" 6078
extern void printstr(short x,short y,char string[],char attribute);
//#line "game.c" 6104
extern void Logo();
//#line "game.c" 6187
extern void loadtmb();
//#line "game.c" 6210
extern void compilecons();
//#line "game.c" 6230
extern void Startup();
//#line "game.c" 6284
extern void getnames();
//#line "game.c" 6309
//extern int main(int argc,char **argv);
//#line "game.c" 6563
//extern char opendemoread(char which_demo);
//#line "game.c" 6589
extern void opendemowrite();
//#line "game.c" 6608
extern void record();
//#line "game.c" 6626
extern void closedemowrite();
//#line "game.c" 6642
extern long playback();
//#line "game.c" 6777
extern char moveloop();
//#line "game.c" 6796
extern void fakedomovethingscorrect();
//#line "game.c" 6829
extern void fakedomovethings();
//#line "game.c" 7247
extern char domovethings();
//#line "game.c" 7373
extern void displaybonuspics(short x,short y,short p);
//#line "game.c" 7396
extern void doorders();
//#line "game.c" 7432
extern void dobonus(char bonusonly);
//#line "game.c" 7846
extern void cameratext(short i);
//#line "game.c" 7869
extern void vglass(long x,long y,short a,short wn,short n);
//#line "game.c" 7882
extern void lotsofglass(short i,short wallnum,short n);
//#line "game.c" 7924
extern void spriteglass(short i,short n);
//#line "game.c" 7937
extern void ceilingglass(short i,short sectnum,short n);
//#line "game.c" 7966
extern void lotsofcolourglass(short i,short wallnum,short n);
//#line "game.c" 8004
extern void SetupGameButtons();
//#line "game.c" 8068
extern long GetTime();
//#line "game.c" 8082
extern void CenterCenter();
//#line "game.c" 8095
extern void UpperLeft();
//#line "game.c" 8108
extern void LowerRight();
//#line "game.c" 8121
extern void CenterThrottle();
//#line "game.c" 8134
extern void CenterRudder();
//#line "config.c" 57
extern void CONFIG_GetSetupFilename();
//#line "config.c" 170
extern int32_t CONFIG_FunctionNameToNum(char *func);
//#line "config.c" 192
extern char *CONFIG_FunctionNumToName(int32_t func);
//#line "config.c" 211
extern analogcontrol CONFIG_AnalogNameToNum(char *func);
//#line "config.c" 240
extern void CONFIG_SetDefaults();
//#line "config.c" 264
extern void CONFIG_ReadKeys();
//#line "config.c" 314
extern void CONFIG_SetupMouse(int32_t scripthandle);
//#line "config.c" 376
extern void CONFIG_SetupGamePad(int32_t scripthandle);
//#line "config.c" 427
extern void CONFIG_SetupJoystick(int32_t scripthandle);
//#line "config.c" 485
extern void readsavenames();
//#line "config.c" 512
extern void CONFIG_ReadSetup();
//#line "config.c" 613
extern void CONFIG_WriteSetup();
/*
//#line "animlib.c" 34
extern void CheckAnimStarted(char *funcname);
//#line "animlib.c" 46
extern uint16_t findpage(uint16_t framenumber);
//#line "animlib.c" 71
extern void loadpage(uint16_t pagenumber,uint16_t *pagepointer);
//#line "animlib.c" 97
extern void CPlayRunSkipDump(char *srcP,char *dstP);
//#line "animlib.c" 177
extern void renderframe(uint16_t framenumber,uint16_t *pagepointer);
//#line "animlib.c" 214
extern void drawframe(uint16_t framenumber);
//#line "animlib.c" 228
extern void ANIM_LoadAnim(char *buffer);
//#line "animlib.c" 260
extern void ANIM_FreeAnim();
//#line "animlib.c" 275
extern int32_t ANIM_NumFrames();
//#line "animlib.c" 287
extern uint8_t *ANIM_DrawFrame(int32_t framenumber);
//#line "animlib.c" 312
extern uint8_t *ANIM_GetPalette();
*/
//#line "actors.c" 7
extern void updateinterpolations();
//#line "actors.c" 15
extern void setinterpolation(long *posptr);
//#line "actors.c" 27
extern void stopinterpolation(long *posptr);
//#line "actors.c" 41
extern void dointerpolations(long smoothratio);
//#line "actors.c" 55
extern void restoreinterpolations();
//#line "actors.c" 63
extern long ceilingspace(short sectnum);
//#line "actors.c" 77
extern long floorspace(short sectnum);
//#line "actors.c" 91
extern void addammo(short weapon,player_struct *p,short amount);
//#line "actors.c" 99
extern void addweapon(player_struct *p,short weapon);
//#line "actors.c" 132
extern void checkavailinven(player_struct *p);
//#line "actors.c" 152
extern void checkavailweapon(player_struct *p);
//#line "actors.c" 301
extern long ifsquished(short i,short p);
//#line "actors.c" 326
extern void hitradius(short i,long r,long hp1,long hp2,long hp3,long hp4);
//#line "actors.c" 556
extern int movesprite(short spritenum,long xchange,long ychange,long zchange,unsigned long cliptype);
//#line "actors.c" 643
extern short ssp(short i,unsigned long cliptype);
//#line "actors.c" 658
extern void insertspriteq(short i);
//#line "actors.c" 666
extern void lotsofmoney(spritetype *s,short n);
//#line "actors.c" 676
extern void guts(spritetype *s,short gtype,short n,short p);
//#line "actors.c" 718
extern void setsectinterpolate(short i);
//#line "actors.c" 742
extern void clearsectinterpolate(short i);
//#line "actors.c" 760
extern void ms(short i);
//#line "actors.c" 791
extern void movefta();
//#line "actors.c" 882
extern short ifhitsectors(short sectnum);
//#line "actors.c" 896
extern short ifhitbyweapon(short sn);
//#line "actors.c" 970
extern void movecyclers();
//#line "actors.c" 1007
extern void movedummyplayers();
//#line "actors.c" 1053
extern void moveplayers();
//#line "actors.c" 1192
extern void movefx();
//#line "actors.c" 1294
extern void movefallers();
//#line "actors.c" 1389
extern void movestandables();
//#line "actors.c" 2234
extern void bounce(short i);
//#line "actors.c" 2273
extern void moveweapons();
//#line "actors.c" 2613
extern void movetransports();
//#line "actors.c" 2887
extern void moveeffectors();
//#line "actors.c" 4840
extern void moveactors();
//#line "actors.c" 6005
extern void moveexplosions();

#endif