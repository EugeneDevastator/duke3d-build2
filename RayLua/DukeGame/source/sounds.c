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

#define LOUDESTVOLUME 150
#include <stdint.h>

#include "fx_man.h"
#include "global.h"
#include "funct.h"
long backflag,numenvsnds;

/*
===================
=
= SoundStartup
=
===================
*/
void testcallback(unsigned long num)
{
 //   TestCallBack(num);
}
void SoundStartup()
   {

   }

/*
===================
=
= SoundShutdown
=
===================
*/

void SoundShutdown()
   {

   }

/*
===================
=
= MusicStartup
=
===================
*/

void MusicStartup()
   {

}

/*
===================
=
= MusicShutdown
=
===================
*/

void MusicShutdown()
{}
void  FX_SetReverbDelay( int delay ){};
int USRHOOKS_GetMem(char **ptr, unsigned long size )
{
return 0;
}
int FX_VoiceAvailable( int priority ){return 0;};
int USRHOOKS_FreeMem(char *ptr)
{return 0; }
int FX_StopAllSounds()
{
    return 0;
}
int FX_StopSound( int handle ){return 0;}
void  FX_SetReverb( int reverb ){};
char menunum=0;
void PlayMusic(char *_filename){};
void intomenusounds()
{}

void playmusic(char *fn)
{}

char loadsound(unsigned short num)
{/*
    long   fp, l;

    if(num >= NUM_SOUNDS || SoundToggle == 0) return 0;
   // if (FXDevice == NumSoundCards) return 0;

    fp = kopen4load(sounds[num],loadfromgrouponly);
    if(fp == -1)
    {
        sprintf(&fta_quotes[113][0],"Sound %s(#%ld) not found.",sounds[num],num);
        FTA(113,&ps[myconnectindex]);
        return 0;
    }

    l = kfilelength( fp );
    soundsiz[num] = l;

    Sound[num].lock = 200;

    allocache((long *)&Sound[num].ptr,l,&Sound[num].lock);
    kread( fp, Sound[num].ptr , l);
    kclose( fp );*/
    return 1;
}

int xyzsound(short num,short i,long x,long y,long z)
{
 //  long sndist, cx, cy, cz, j,k;
 //  short pitche,pitchs,cs;
 //  int voice, sndang, ca, pitch;

///    if(num != 358) return 0;

 //  if( num >= NUM_SOUNDS ||
 //      ( (soundm[num]&8) && ud.lockout ) ||
 //      SoundToggle == 0 ||
 //      Sound[num].num > 3 ||
 //      FX_VoiceAvailable(soundpr[num]) == 0 ||
 //      (ps[myconnectindex].timebeforeexit > 0 && ps[myconnectindex].timebeforeexit <= 26*3) ||
 //      ps[myconnectindex].gm&MODE_MENU) return -1;

 //  if( soundm[num]&128 )
 //  {
 //      sound(num);
 //      return 0;
 //  }

 //  if( soundm[num]&4 )
 //  {
 //      if(VoiceToggle==0 || (ud.multimode > 1 && PN == APLAYER && sprite[i].yvel != screenpeek && ud.coop != 1) ) return -1;

 //      for(j=0;j<NUM_SOUNDS;j++)
 //        for(k=0;k<Sound[j].num;k++)
 //          if( (Sound[j].num > 0) && (soundm[j]&4) )
 //            return -1;
 //  }

 //  cx = ps[screenpeek].oposx;
 //  cy = ps[screenpeek].oposy;
 //  cz = ps[screenpeek].oposz;
 //  cs = ps[screenpeek].cursectnum;
 //  ca = ps[screenpeek].ang+ps[screenpeek].look_ang;

 //  sndist = FindDistance3D((cx-x),(cy-y),(cz-z)>>4);

 //  if( i >= 0 && (soundm[num]&16) == 0 && PN == MUSICANDSFX && SLT < 999 && (sector[SECT].lotag&0xff) < 9 )
 //      sndist = divscale14(sndist,(SHT+1));

 //  pitchs = soundps[num];
 //  pitche = soundpe[num];
 //  cx = klabs(pitche-pitchs);

 //  if(cx)
 //  {
 //      if( pitchs < pitche )
 //           pitch = pitchs + ( rand()%cx );
 //      else pitch = pitche + ( rand()%cx );
 //  }
 //  else pitch = pitchs;

 //  sndist += soundvo[num];
 //  if(sndist < 0) sndist = 0;
 //  if( sndist && PN != MUSICANDSFX && !cansee(cx,cy,cz-(24<<8),cs,SX,SY,SZ-(24<<8),SECT) )
 //      sndist += sndist>>5;

 //  switch(num)
 //  {
 //      case PIPEBOMB_EXPLODE:
 //      case LASERTRIP_EXPLODE:
 //      case RPG_EXPLODE:
 //          if(sndist > (6144) )
 //              sndist = 6144;
 //          if(sector[ps[screenpeek].cursectnum].lotag == 2)
 //              pitch -= 1024;
 //          break;
 //      default:
 //          if(sector[ps[screenpeek].cursectnum].lotag == 2 && (soundm[num]&4) == 0)
 //              pitch = -768;
 //          if( sndist > 31444 && PN != MUSICANDSFX)
 //              return -1;
 //          break;
 //  }


 //  if( Sound[num].num > 0 && PN != MUSICANDSFX )
 //  {
 //      if( SoundOwner[num][0].i == i ) stopsound(num);
 //      else if( Sound[num].num > 1 ) stopsound(num);
 //      else if( badguy(&sprite[i]) && sprite[i].extra <= 0 ) stopsound(num);
 //  }

 //  if( PN == APLAYER && sprite[i].yvel == screenpeek )
 //  {
 //      sndang = 0;
 //      sndist = 0;
 //  }
 //  else
 //  {
 //      sndang = 2048 + ca - getangle(cx-x,cy-y);
 //      sndang &= 2047;
 //  }

 //  if(Sound[num].ptr == 0) { if( loadsound(num) == 0 ) return 0; }
 //  else
 //  {
 //     if (Sound[num].lock < 200)
 //        Sound[num].lock = 200;
 //     else Sound[num].lock++;
 //  }

 //  if( soundm[num]&16 ) sndist = 0;

 //  if(sndist < ((255-LOUDESTVOLUME)<<6) )
 //      sndist = ((255-LOUDESTVOLUME)<<6);

 //  if( soundm[num]&1 )
 //  {
 //      unsigned short start;

 //      if(Sound[num].num > 0) return -1;

 //      start = *(unsigned short *)(Sound[num].ptr + 0x14);

 //      if(*Sound[num].ptr == 'C')
 //          voice = FX_PlayLoopedVOC( Sound[num].ptr, start, start + soundsiz[num],
 //                  pitch,sndist>>6,sndist>>6,0,soundpr[num],num);
 //      else
 //          voice = FX_PlayLoopedWAV( Sound[num].ptr, start, start + soundsiz[num],
 //                  pitch,sndist>>6,sndist>>6,0,soundpr[num],num);
 //  }
 //  else
 //  {
 //      if( *Sound[num].ptr == 'C')
 //          voice = FX_PlayVOC3D( Sound[ num ].ptr,pitch,sndang>>6,sndist>>6, soundpr[num], num );
 //      else voice = FX_PlayWAV3D( Sound[ num ].ptr,pitch,sndang>>6,sndist>>6, soundpr[num], num );
 //  }

 //  if ( voice > FX_Ok )
 //  {
 //      SoundOwner[num][Sound[num].num].i = i;
 //      SoundOwner[num][Sound[num].num].voice = voice;
 //      Sound[num].num++;
 //  }
 //  else Sound[num].lock--;
 //   return (voice);
   return (0);
}

void sound(short num)
{}

int spritesound(unsigned short num, short i)
{
    return xyzsound(num,i,SX,SY,SZ);
}

void stopsound(short num)
{

}

void stopenvsound(short num,short i)
{}

void pan3dsound()
{}

int sgn(int value)
{
    return (value > 0) - (value < 0);
}

void TestCallBack(unsigned long num)
{}

void clearsoundlocks()
{}
