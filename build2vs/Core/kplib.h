#ifndef BUILD2_KPLIB_H
#define BUILD2_KPLIB_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define MAX_PATH 260
enum //kpgetdim() return values:
{
	KPLIB_NONE=0,
	KPLIB_PNG, KPLIB_JPG, KPLIB_GIF, KPLIB_CEL,
	KPLIB_BMP, KPLIB_PCX, KPLIB_DDS, KPLIB_TGA,
};

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <io.h>
#else
#include <dirent.h>
typedef long long __int64;
static inline int _lrotl(int i, int sh) {
	return ((i >> (-sh)) | (i << sh));
}
static inline int filelength(int h) {
	struct stat st;
	if (fstat(h, &st) < 0) return -1;
	return st.st_size;
}
#define _fileno fileno
#endif

#define MAX_PATH 260

// Endianness handling - runtime detection is more portable
static inline int is_big_endian(void) {
	union { uint32_t i; char c[4]; } test = {0x01020304};
	return test.c[0] == 1;
}

static inline uint32_t swap32(uint32_t a) {
	return ((a >> 24) | ((a >> 8) & 0xff00) | ((a & 0xff00) << 8) | (a << 24));
}

static inline uint16_t swap16(uint16_t a) {
	return ((a >> 8) | (a << 8));
}

#define LSWAPIB(a) (is_big_endian() ? (a) : swap32(a))
#define SSWAPIB(a) (is_big_endian() ? (a) : swap16(a))
#define LSWAPIL(a) (is_big_endian() ? swap32(a) : (a))
#define SSWAPIL(a) (is_big_endian() ? swap16(a) : (a))

#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
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


	//Hack for peekbits,getbits,suckbits (to prevent lots of duplicate code)
	//   0: PNG: do 12-byte chunk_header removal hack
	// !=0: ZIP: use 64K buffer (olinbuf)

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

//KPLIB.H:
//High-level (easy) picture loading function:
void kpzload (const char *filnam, intptr_t *pic, long *bpl, int *xsiz, int *ysiz);
//Low-level PNG/JPG functions:
int kpgetdim (const char *, int, int *, int *);
int kprender (const char *, int, intptr_t, int, int, int, int, int);
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