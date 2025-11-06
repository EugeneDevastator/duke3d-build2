// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
#ifndef CACHED_H
#define CACHED_H
//#include <dos.h>
#include <io.h>
#include <stdint.h>
#include <stdio.h>
//#include "fcntl.h"
#include <stdlib.h>

#include "pragmas.h"
#define FP_OFF(x) ((long) (x)) //toggle off floatng point
#define FPP_OFF(x) ((long) ((void*)(x)))
void *kmalloc(size_t size);
void kfree(void *buffer);
void kkfree(void *buffer);
extern long cachecount;
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
//              *bufptr = pointer to 4-int8_t pointer to buffer
//                 Confused?  Using this method, cache2d can remove
//                 previously allocated things from the cache safely by
//                 setting the 4-int8_t pointer to 0.
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


typedef struct
{
	long *hand, leng;
	char* lock;
} cactype;

extern cactype cac[MAXCACHEOBJECTS];


void initcache(long dacachestart, long dacachesize);

void allocache (long *newhandle, long newbytes, char *newlockptr);

void suckcache (long *suckptr);

void agecache();

void reportandexit(char *errormessage);


//#include <io.h>
//#include <sys\types.h>
//#include <sys\stat.h>

#define MAXGROUPFILES 4     //Warning: Fix groupfil if this is changed
#define MAXOPENFILES 64     //Warning: Fix filehan if this is changed


int initgroupfile(const char* filename);

void uninitgroupfile();

long kopen4load(const char* filename, char searchfirst);

int kread(long handle, void *buffer, long leng);

long klseek(long handle, long offset, long whence);

long kfilelength(long handle);

void kclose(long handle);


//Internal LZW variables
#define LZWSIZE 16384           //Watch out for shorts!

void kdfread(void *buffer, size_t dasizeof, size_t count, long fil);

void dfread(void *buffer, size_t dasizeof, size_t count, FILE *fil);

void dfwrite(void *buffer, size_t dasizeof, size_t count, FILE *fil);

long compress(char *lzwinbuf, long uncompleng, char *lzwoutbuf);

long uncompress(char *lzwinbuf, long compleng, char *lzwoutbuf);
#endif