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

#include "duke3d.h"
//#include "mouse.h"
//#include "animlib.h"

extern char inputloc;
extern int recfilep;
extern char vgacompatible;
short probey=0,lastprobey=0,last_menu,globalskillsound=-1;
short sh,onbar,buttonstat,deletespot;
short last_zero,last_fifty,last_threehundred = 0;
static char fileselect = 1, menunamecnt, menuname[256][17], curpath[80], menupath[80];

// CTW - REMOVED
/* Error codes */
/*
#define eTenBnNotInWindows 3801
#define eTenBnBadGameIni 3802
#define eTenBnBadTenIni 3803
#define eTenBnBrowseCancel 3804
#define eTenBnBadTenInst 3805

int  tenBnStart();
void tenBnSetBrowseRtn(char *(*rtn)(char *str, int len));
void tenBnSetExitRtn(void (*rtn)());
void tenBnSetEndRtn(void (*rtn)());*/
// CTW END - REMOVED

void dummyfunc()
{
}

void dummymess(int i,char *c)
{
}

// CTW - REMOVED
/*
void TENtext()
{
    long dacount,dalastcount;

    puts("\nDuke Nukem 3D has been licensed exclusively to TEN (Total");
    puts("Entertainment Network) for wide-area networked (WAN) multiplayer");
    puts("games.\n");

    puts("The multiplayer code within Duke Nukem 3D has been highly");
    puts("customized to run best on TEN, where you'll experience fast and");
    puts("stable performance, plus other special benefits.\n");

    puts("We do not authorize or recommend the use of Duke Nukem 3D with");
    puts("gaming services other than TEN.\n");

    puts("Duke Nukem 3D is protected by United States copyright law and");
    puts("international treaty.\n");

    puts("For the best online multiplayer gaming experience, please call TEN");
    puts("at 800-8040-TEN, or visit TEN's Web Site at www.ten.net.\n");

    puts("Press any key to continue.\n");

    _bios_timeofday(0,&dacount);

    while( _bios_keybrd(1) == 0 )
    {
        _bios_timeofday(0,&dalastcount);
        if( (dacount+240) < dalastcount ) break;
    }
}
*/
// CTW END - REMOVED

void cmenu(short cm);


void savetemp(char *fn,long daptr,long dasiz);

void getangplayers(short snum);

int loadpheader(char spot,int32_t *vn,int32_t *ln,int32_t *psk,int32_t *nump);


int loadplayer(signed char spot);

int saveplayer(signed char spot);

#define LMB (buttonstat&1)
#define RMB (buttonstat&2)

ControlInfo minfo;

long mi;

int probe(int x,int y,int i,int n);

int menutext(int x,int y,short s,short p,char *t);

int menutextc(int x,int y,short s,short p,char *t);


void bar(int x,int y,short *p,short dainc,char damodify,short s, short pa);

#define SHX(X) 0
// ((x==X)*(-sh))
#define PHX(X) 0
// ((x==X)?1:2)
#define MWIN(X) rotatesprite( 320<<15,200<<15,X,0,MENUSCREEN,-16,0,10+64,0,0,xdim-1,ydim-1)
#define MWINXY(X,OX,OY) rotatesprite( ( 320+(OX) )<<15, ( 200+(OY) )<<15,X,0,MENUSCREEN,-16,0,10+64,0,0,xdim-1,ydim-1)


int32_t volnum,levnum,plrskl,numplr;
short lastsavedpos = -1;

void dispnames();

int getfilenames(char kind[6]);

void sortfilenames();

long quittimer = 0;

void menus();



void endanimsounds(long fr);

void logoanimsounds(long fr);

void intro4animsounds(long fr);

void first4animsounds(long fr);

void intro42animsounds(long fr);


void endanimvol41(long fr);

void endanimvol42(long fr);

void endanimvol43(long fr);


long lastanimhack=0;
void playanm(char *fn,char t);

