//
// Created by omnis on 10/21/2025.
//

#ifndef BUILD2_KPLIB_H
#define BUILD2_KPLIB_H

#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#define MAX_PATH 260
enum //kpgetdim() return values:
{
	KPLIB_NONE=0,
	KPLIB_PNG, KPLIB_JPG, KPLIB_GIF, KPLIB_CEL,
	KPLIB_BMP, KPLIB_PCX, KPLIB_DDS, KPLIB_TGA,
};

#if defined(__POWERPC__)
#define BIGENDIAN 1
#endif

#ifdef BIGENDIAN
static unsigned int LSWAPIB (unsigned int a) { return(((a>>8)&0xff00)+((a&0xff00)<<8)+(a<<24)+(a>>24)); }
static unsigned short SSWAPIB (unsigned short a) { return((a>>8)+(a<<8)); }
#define LSWAPIL(a) (a)
#define SSWAPIL(a) (a)
#else
#define LSWAPIB(a) (a)
#define SSWAPIB(a) (a)
static unsigned int LSWAPIL (unsigned int a) { return(((a>>8)&0xff00)+((a&0xff00)<<8)+(a<<24)+(a>>24)); }
static unsigned short SSWAPIL (unsigned short a) { return((a>>8)+(a<<8)); }
#endif

#ifdef __GNUC__
#include <stdint.h>
#define INT_PTR intptr_t
#define UINT_PTR uintptr_t
#endif

#if !defined(_WIN32) && !defined(__DOS__)
#include <unistd.h>
#include <dirent.h>
typedef long long __int64;
static __inline int _lrotl (int i, int sh)
	{ return((i>>(-sh))|(i<<sh)); }
static __inline int filelength (int h)
{
	struct stat st;
	if (fstat(h,&st) < 0) return(-1);
	return(st.st_size);
}
#define _fileno fileno
#else
#include <io.h>
#endif

#if defined(__DOS__)
#include <dos.h>
#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif
#if !defined(max)
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#if !defined(min)
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#if defined(__GNUC__)
#define _inline inline
#endif

	//use GCC-specific extension to force symbol name to be something in particular to override underscoring.
#if defined(__GNUC__) && defined(__i386__) && !defined(NOASM)
#define ASMNAME(x) asm(x)
#else
#define ASMNAME(x)
#endif


static INT_PTR frameplace;

static const int pow2mask[32] =
{
	0x00000000,0x00000001,0x00000003,0x00000007,
	0x0000000f,0x0000001f,0x0000003f,0x0000007f,
	0x000000ff,0x000001ff,0x000003ff,0x000007ff,
	0x00000fff,0x00001fff,0x00003fff,0x00007fff,
	0x0000ffff,0x0001ffff,0x0003ffff,0x0007ffff,
	0x000fffff,0x001fffff,0x003fffff,0x007fffff,
	0x00ffffff,0x01ffffff,0x03ffffff,0x07ffffff,
	0x0fffffff,0x1fffffff,0x3fffffff,0x7fffffff,
};
static const int pow2long[32] =
{
	0x00000001,0x00000002,0x00000004,0x00000008,
	0x00000010,0x00000020,0x00000040,0x00000080,
	0x00000100,0x00000200,0x00000400,0x00000800,
	0x00001000,0x00002000,0x00004000,0x00008000,
	0x00010000,0x00020000,0x00040000,0x00080000,
	0x00100000,0x00200000,0x00400000,0x00800000,
	0x01000000,0x02000000,0x04000000,0x08000000,
	0x10000000,0x20000000,0x40000000,0x80000000,
};

	//Hack for peekbits,getbits,suckbits (to prevent lots of duplicate code)
	//   0: PNG: do 12-byte chunk_header removal hack
	// !=0: ZIP: use 64K buffer (olinbuf)
static int zipfilmode;
typedef struct
{
	FILE *fil;   //0:no file open, !=0:open file (either stand-alone or zip)
	int comptyp; //0:raw data (can be ZIP or stand-alone), 8:PKZIP LZ77 *flate
	int seek0;   //0:stand-alone file, !=0: start of zip compressed stream data
	int compleng;//Global variable for compression FIFO
	int comptell;//Global variable for compression FIFO
	int leng;    //Uncompressed file size (bytes)
	int pos;     //Current uncompressed relative file position (0<=pos<=leng)
	int endpos;  //Temp global variable for kzread
	int jmpplc;  //Store place where decompression paused
	int i;       //For stand-alone/ZIP comptyp#0, this is like "uncomptell"
					  //For ZIP comptyp#8&btype==0 "<64K store", this saves i state
	int bfinal;  //LZ77 decompression state (for later calls)
} kzfilestate;
static kzfilestate kzfs;
//KPLIB.H:
//High-level (easy) picture loading function:
void kpzload (const char *filnam, long *pic, long *bpl, int *xsiz, int *ysiz);
//Low-level PNG/JPG functions:
int kpgetdim (const char *, int, int *, int *);
int kprender (const char *, int, INT_PTR, int, int, int, int, int);
//Ken's ZIP functions:
int kzaddstack (const char *);
void kzuninit (void);
void kzsetfil (FILE *);
int kzopen (const char *);
int kzread (void *, int);
int kzfilelength (void);
int kzseek (int, int);
int kztell (void);
int kzgetc (void);
int kzeof (void);
void kzclose (void);
void kzfindfilestart (const char *); //pass wildcard string
int kzfindfile (char *); //you alloc buf, returns 1:found,0:~found

#endif //BUILD2_KPLIB_H