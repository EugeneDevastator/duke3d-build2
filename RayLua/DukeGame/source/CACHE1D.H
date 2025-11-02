// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

//#include <dos.h>
#include <stdio.h>
#include "pragmas.h"
#include <fcntl.h>
#include <io.h>

//   This module keeps track of a standard linear cacheing system.
//   To use this module, here's all you need to do:
//
//   Step 1: Allocate a nice BIG buffer, like from 1MB-4MB and
//           Call initcache(long cachestart, long cachesize) where
//
//              cachestart = (long)(pointer to start of BIG buffer)
//              cachesize = length of BIG buffer
//
//   Step 2: Call allocache(long *bufptr, long bufsiz, char *lockptr)
//              whenever you need to allocate a buffer, where:
//
//              *bufptr = pointer to 4-byte pointer to buffer
//                 Confused?  Using this method, cache2d can remove
//                 previously allocated things from the cache safely by
//                 setting the 4-byte pointer to 0.
//              bufsiz = number of bytes to allocate
//              *lockptr = pointer to locking char which tells whether
//                 the region can be removed or not.  If *lockptr = 0 then
//                 the region is not locked else its locked.
//
//   Step 3: If you need to remove everything from the cache, or every
//           unlocked item from the cache, you can call uninitcache();
//              Call uninitcache(0) to remove all unlocked items, or
//              Call uninitcache(1) to remove everything.
//           After calling uninitcache, it is still ok to call allocache
//           without first calling initcache.

#define MAXCACHEOBJECTS 9216

static long cachesize = 0;
long cachecount = 0;
char zerochar = 0;
long cachestart = 0, cacnum = 0, agecount = 0;
typedef struct { long *hand, leng; char *lock; } cactype;
extern cactype cac[MAXCACHEOBJECTS];
static long lockrecip[200];

initcache(long dacachestart, long dacachesize);

allocache (long *newhandle, long newbytes, char *newlockptr);

suckcache (long *suckptr);

agecache();

reportandexit(char *errormessage);


//#include <io.h>
//#include <sys\types.h>
//#include <sys\stat.h>

#define MAXGROUPFILES 4     //Warning: Fix groupfil if this is changed
#define MAXOPENFILES 64     //Warning: Fix filehan if this is changed

static unit8_t toupperlookup[256] =
{
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
	0x60,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x7b,0x7c,0x7d,0x7e,0x7f,
	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
	0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
	0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
	0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
	0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
	0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
	0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff,
};

static long numgroupfiles = 0;
static long gnumfiles[MAXGROUPFILES];
static long groupfil[MAXGROUPFILES] = {-1,-1,-1,-1};
static long groupfilpos[MAXGROUPFILES];
static char *gfilelist[MAXGROUPFILES];
static long *gfileoffs[MAXGROUPFILES];

static char filegrp[MAXOPENFILES];
static long filepos[MAXOPENFILES];
static long filehan[MAXOPENFILES] =
{
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
};

initgroupfile(char *filename);

uninitgroupfile();

kopen4load(char *filename, char searchfirst);

kread(long handle, void *buffer, long leng);

klseek(long handle, long offset, long whence);

kfilelength(long handle);

kclose(long handle);


//Internal LZW variables
#define LZWSIZE 16384           //Watch out for shorts!
static char *lzwbuf1, *lzwbuf4, *lzwbuf5, lzwbuflock[5];
static short *lzwbuf2, *lzwbuf3;

kdfread(void *buffer, size_t dasizeof, size_t count, long fil)
{
	long i, j, k, kgoal;
	short leng;
	char *ptr;

	lzwbuflock[0] = lzwbuflock[1] = lzwbuflock[2] = lzwbuflock[3] = lzwbuflock[4] = 200;
	if (lzwbuf1 == NULL) allocache((long *)&lzwbuf1,LZWSIZE+(LZWSIZE>>4),&lzwbuflock[0]);
	if (lzwbuf2 == NULL) allocache((long *)&lzwbuf2,(LZWSIZE+(LZWSIZE>>4))*2,&lzwbuflock[1]);
	if (lzwbuf3 == NULL) allocache((long *)&lzwbuf3,(LZWSIZE+(LZWSIZE>>4))*2,&lzwbuflock[2]);
	if (lzwbuf4 == NULL) allocache((long *)&lzwbuf4,LZWSIZE,&lzwbuflock[3]);
	if (lzwbuf5 == NULL) allocache((long *)&lzwbuf5,LZWSIZE+(LZWSIZE>>4),&lzwbuflock[4]);

	if (dasizeof > LZWSIZE) { count *= dasizeof; dasizeof = 1; }
	ptr = (char *)buffer;

	kread(fil,&leng,2); kread(fil,lzwbuf5,(long)leng);
	k = 0; kgoal = uncompress(lzwbuf5,(long)leng,lzwbuf4);

	copybufbyte(lzwbuf4,ptr,(long)dasizeof);
	k += (long)dasizeof;

	for(i=1;i<count;i++)
	{
		if (k >= kgoal)
		{
			kread(fil,&leng,2); kread(fil,lzwbuf5,(long)leng);
			k = 0; kgoal = uncompress(lzwbuf5,(long)leng,lzwbuf4);
		}
		for(j=0;j<dasizeof;j++) ptr[j+dasizeof] = ((ptr[j]+lzwbuf4[j+k])&255);
		k += dasizeof;
		ptr += dasizeof;
	}
	lzwbuflock[0] = lzwbuflock[1] = lzwbuflock[2] = lzwbuflock[3] = lzwbuflock[4] = 1;
}

dfread(void *buffer, size_t dasizeof, size_t count, FILE *fil)
{
	long i, j, k, kgoal;
	short leng;
	char *ptr;

	lzwbuflock[0] = lzwbuflock[1] = lzwbuflock[2] = lzwbuflock[3] = lzwbuflock[4] = 200;
	if (lzwbuf1 == NULL) allocache((long *)&lzwbuf1,LZWSIZE+(LZWSIZE>>4),&lzwbuflock[0]);
	if (lzwbuf2 == NULL) allocache((long *)&lzwbuf2,(LZWSIZE+(LZWSIZE>>4))*2,&lzwbuflock[1]);
	if (lzwbuf3 == NULL) allocache((long *)&lzwbuf3,(LZWSIZE+(LZWSIZE>>4))*2,&lzwbuflock[2]);
	if (lzwbuf4 == NULL) allocache((long *)&lzwbuf4,LZWSIZE,&lzwbuflock[3]);
	if (lzwbuf5 == NULL) allocache((long *)&lzwbuf5,LZWSIZE+(LZWSIZE>>4),&lzwbuflock[4]);

	if (dasizeof > LZWSIZE) { count *= dasizeof; dasizeof = 1; }
	ptr = (char *)buffer;

	fread(&leng,2,1,fil); fread(lzwbuf5,(long)leng,1,fil);
	k = 0; kgoal = uncompress(lzwbuf5,(long)leng,lzwbuf4);

	copybufbyte(lzwbuf4,ptr,(long)dasizeof);
	k += (long)dasizeof;

	for(i=1;i<count;i++)
	{
		if (k >= kgoal)
		{
			fread(&leng,2,1,fil); fread(lzwbuf5,(long)leng,1,fil);
			k = 0; kgoal = uncompress(lzwbuf5,(long)leng,lzwbuf4);
		}
		for(j=0;j<dasizeof;j++) ptr[j+dasizeof] = ((ptr[j]+lzwbuf4[j+k])&255);
		k += dasizeof;
		ptr += dasizeof;
	}
	lzwbuflock[0] = lzwbuflock[1] = lzwbuflock[2] = lzwbuflock[3] = lzwbuflock[4] = 1;
}

dfwrite(void *buffer, size_t dasizeof, size_t count, FILE *fil)
{
	long i, j, k;
	short leng;
	char *ptr;

	lzwbuflock[0] = lzwbuflock[1] = lzwbuflock[2] = lzwbuflock[3] = lzwbuflock[4] = 200;
	if (lzwbuf1 == NULL) allocache((long *)&lzwbuf1,LZWSIZE+(LZWSIZE>>4),&lzwbuflock);
	if (lzwbuf2 == NULL) allocache((long *)&lzwbuf2,(LZWSIZE+(LZWSIZE>>4))*2,&lzwbuflock);
	if (lzwbuf3 == NULL) allocache((long *)&lzwbuf3,(LZWSIZE+(LZWSIZE>>4))*2,&lzwbuflock);
	if (lzwbuf4 == NULL) allocache((long *)&lzwbuf4,LZWSIZE,&lzwbuflock);
	if (lzwbuf5 == NULL) allocache((long *)&lzwbuf5,LZWSIZE+(LZWSIZE>>4),&lzwbuflock);

	if (dasizeof > LZWSIZE) { count *= dasizeof; dasizeof = 1; }
	ptr = (char *)buffer;

	copybufbyte(ptr,lzwbuf4,(long)dasizeof);
	k = dasizeof;

	if (k > LZWSIZE-dasizeof)
	{
		leng = (short)compress(lzwbuf4,k,lzwbuf5); k = 0;
		fwrite(&leng,2,1,fil); fwrite(lzwbuf5,(long)leng,1,fil);
	}

	for(i=1;i<count;i++)
	{
		for(j=0;j<dasizeof;j++) lzwbuf4[j+k] = ((ptr[j+dasizeof]-ptr[j])&255);
		k += dasizeof;
		if (k > LZWSIZE-dasizeof)
		{
			leng = (short)compress(lzwbuf4,k,lzwbuf5); k = 0;
			fwrite(&leng,2,1,fil); fwrite(lzwbuf5,(long)leng,1,fil);
		}
		ptr += dasizeof;
	}
	if (k > 0)
	{
		leng = (short)compress(lzwbuf4,k,lzwbuf5);
		fwrite(&leng,2,1,fil); fwrite(lzwbuf5,(long)leng,1,fil);
	}
	lzwbuflock[0] = lzwbuflock[1] = lzwbuflock[2] = lzwbuflock[3] = lzwbuflock[4] = 1;
}

compress(char *lzwinbuf, long uncompleng, char *lzwoutbuf)
{
	long i, addr, newaddr, addrcnt, zx, *longptr;
	long bytecnt1, bitcnt, numbits, oneupnumbits;
	short *shortptr;

	for(i=255;i>=0;i--) { lzwbuf1[i] = i; lzwbuf3[i] = (i+1)&255; }
	clearbuf(FP_OFF(lzwbuf2),256>>1,0xffffffff);
	clearbuf(FP_OFF(lzwoutbuf),((uncompleng+15)+3)>>2,0L);

	addrcnt = 256; bytecnt1 = 0; bitcnt = (4<<3);
	numbits = 8; oneupnumbits = (1<<8);
	do
	{
		addr = lzwinbuf[bytecnt1];
		do
		{
			bytecnt1++;
			if (bytecnt1 == uncompleng) break;
			if (lzwbuf2[addr] < 0) {lzwbuf2[addr] = addrcnt; break;}
			newaddr = lzwbuf2[addr];
			while (lzwbuf1[newaddr] != lzwinbuf[bytecnt1])
			{
				zx = lzwbuf3[newaddr];
				if (zx < 0) {lzwbuf3[newaddr] = addrcnt; break;}
				newaddr = zx;
			}
			if (lzwbuf3[newaddr] == addrcnt) break;
			addr = newaddr;
		} while (addr >= 0);
		lzwbuf1[addrcnt] = lzwinbuf[bytecnt1];
		lzwbuf2[addrcnt] = -1;
		lzwbuf3[addrcnt] = -1;

		longptr = (long *)&lzwoutbuf[bitcnt>>3];
		longptr[0] |= (addr<<(bitcnt&7));
		bitcnt += numbits;
		if ((addr&((oneupnumbits>>1)-1)) > ((addrcnt-1)&((oneupnumbits>>1)-1)))
			bitcnt--;

		addrcnt++;
		if (addrcnt > oneupnumbits) { numbits++; oneupnumbits <<= 1; }
	} while ((bytecnt1 < uncompleng) && (bitcnt < (uncompleng<<3)));

	longptr = (long *)&lzwoutbuf[bitcnt>>3];
	longptr[0] |= (addr<<(bitcnt&7));
	bitcnt += numbits;
	if ((addr&((oneupnumbits>>1)-1)) > ((addrcnt-1)&((oneupnumbits>>1)-1)))
		bitcnt--;

	shortptr = (short *)lzwoutbuf;
	shortptr[0] = (short)uncompleng;
	if (((bitcnt+7)>>3) < uncompleng)
	{
		shortptr[1] = (short)addrcnt;
		return((bitcnt+7)>>3);
	}
	shortptr[1] = (short)0;
	for(i=0;i<uncompleng;i++) lzwoutbuf[i+4] = lzwinbuf[i];
	return(uncompleng+4);
}

uncompress(char *lzwinbuf, long compleng, char *lzwoutbuf)
{
	long strtot, currstr, numbits, oneupnumbits;
	long i, dat, leng, bitcnt, outbytecnt, *longptr;
	short *shortptr;

	shortptr = (short *)lzwinbuf;
	strtot = (long)shortptr[1];
	if (strtot == 0)
	{
		copybuf(FP_OFF(lzwinbuf)+4,FP_OFF(lzwoutbuf),((compleng-4)+3)>>2);
		return((long)shortptr[0]); //uncompleng
	}
	for(i=255;i>=0;i--) { lzwbuf2[i] = i; lzwbuf3[i] = i; }
	currstr = 256; bitcnt = (4<<3); outbytecnt = 0;
	numbits = 8; oneupnumbits = (1<<8);
	do
	{
		longptr = (long *)&lzwinbuf[bitcnt>>3];
		dat = ((longptr[0]>>(bitcnt&7)) & (oneupnumbits-1));
		bitcnt += numbits;
		if ((dat&((oneupnumbits>>1)-1)) > ((currstr-1)&((oneupnumbits>>1)-1)))
			{ dat &= ((oneupnumbits>>1)-1); bitcnt--; }

		lzwbuf3[currstr] = dat;

		for(leng=0;dat>=256;leng++,dat=lzwbuf3[dat])
			lzwbuf1[leng] = lzwbuf2[dat];

		lzwoutbuf[outbytecnt++] = dat;
		for(i=leng-1;i>=0;i--) lzwoutbuf[outbytecnt++] = lzwbuf1[i];

		lzwbuf2[currstr-1] = dat; lzwbuf2[currstr] = dat;
		currstr++;
		if (currstr > oneupnumbits) { numbits++; oneupnumbits <<= 1; }
	} while (currstr < strtot);
	return((long)shortptr[0]); //uncompleng
}
