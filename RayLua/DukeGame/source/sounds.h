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

//****************************************************************************
//
// sounds.h
//
//****************************************************************************


#ifndef _sounds_public_
#define _sounds_public_
#include "funct.h"


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





/* sounds.c */
void clearsoundlocks();

/* dunno where this came from; I added it. --ryan. */
void testcallback(unsigned long num);


#endif
