//
// Created by omnis on 11/2/2025.
//
#include "global.h"
short spriteqamount = 64;
char networkmode = 255;
char movesperpacket = 1;
char gamequit = 0;
char playonten = 0;
char everyothertime;
int _argc;
char **_argv;
long myminlag[MAXPLAYERS], mymaxlag, otherminlag, bufferjitter = 1;
short numclouds,clouds[128],cloudx[128],cloudy[128];
long cloudtotalclock = 0,totalmemory = 0;
long numinterpolations = 0, startofdynamicinterpolations = 0;
long oldipos[MAXINTERPOLATIONS];
long bakipos[MAXINTERPOLATIONS];
long *curipos[MAXINTERPOLATIONS];
static void (*shutdown_func)() = NULL;
short weaponsandammosprites[15] = {
    RPGSPRITE,
    CHAINGUNSPRITE,
    DEVISTATORAMMO,
    RPGAMMO,
    RPGAMMO,
    JETPACK,
    SHIELD,
    FIRSTAID,
    STEROIDS,
    RPGAMMO,
    RPGAMMO,
    RPGSPRITE,
    RPGAMMO,
    FREEZESPRITE,
    FREEZEAMMO
};
long  respawnactortime=768, respawnitemtime=768;

void _dos_getdate(dosdate_t* date)
{
	time_t curtime = time(NULL);
	struct tm *tm;

	if (date == NULL) {
		return;
	}

	memset(date, 0, sizeof(dosdate_t));

	if ((tm = localtime(&curtime)) != NULL) {
		date->day = tm->tm_mday;
		date->month = tm->tm_mon + 1;
		date->year = tm->tm_year + 1900;
		date->dayofweek = tm->tm_wday + 1;
	}
}

long FindDistance2D(int ix, int iy)
{
    int   t;

    ix= abs(ix);        /* absolute values */
    iy= abs(iy);

    if (ix<iy)
    {
        int tmp = ix;
        ix = iy;
        iy = tmp;
    }

    t = iy + (iy>>1);

    return (ix - (ix>>5) - (ix>>7)  + (t>>2) + (t>>6));
}

int32_t FindDistance3D(int ix, int iy, int iz)
{
	int   t;

	ix= abs(ix);           /* absolute values */
	iy= abs(iy);
	iz= abs(iz);

	if (ix<iy)
	{
		int tmp = ix;
		ix = iy;
		iy = tmp;
	}

	if (ix<iz)
	{
		int tmp = ix;
		ix = iz;
		iz = tmp;
	}

	t = iy + iz;

	return (ix - (ix>>4) + (t>>2) + (t>>3));
}

short SwapShort(short l)
{
	int8_t	b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

short KeepShort(short l)
{
	return l;
}

long SwapLong(long l)
{
	int8_t	b1,b2,b3,b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((long)b1<<24) + ((long)b2<<16) + ((long)b3<<8) + b4;
}

long KeepLong(long l)
{
	return l;
}

void SwapIntelLong(long* l)
{

}

void SwapIntelShort(short* s)
{

}

void SwapIntelLongArray(long* l, int num)
{
	while (num--) {
		SwapIntelLong(l);
		l++;
	}
}

void SwapIntelShortArray(short* s, int num)
{
	while (num--) {
		SwapIntelShort(s);
		s++;
	}
}

int setup_homedir()
{
#ifdef DC
	strcpy (ApogeePath, "/ram/");
#elif PLATFORM_UNIX
	char *cfgpath;
	int err;

#if PLATFORM_MACOSX
	snprintf (ApogeePath, sizeof (ApogeePath), "%s/Library/", getenv ("HOME"));
	mkdir (ApogeePath, S_IRWXU);
	snprintf (ApogeePath, sizeof (ApogeePath), "%s/Library/Application Support/", getenv ("HOME"));
	mkdir (ApogeePath, S_IRWXU);
	snprintf (ApogeePath, sizeof (ApogeePath), "%s/Library/Application Support/Duke Nukem 3D/", getenv ("HOME"));
#else
	snprintf (ApogeePath, sizeof (ApogeePath), "%s/.duke3d/", getenv ("HOME"));
#endif

	err = mkdir (ApogeePath, S_IRWXU);
	if (err == -1 && errno != EEXIST)
	{
		fprintf (stderr, "Couldn't create preferences directory: %s\n",
				 strerror (errno));
		return -1;
	}

	/* copy duke3d.cfg to prefpath if it doesn't exist... */
	cfgpath = alloca(strlen(ApogeePath) + strlen(SETUPFILENAME) + 1);
	strcpy(cfgpath, ApogeePath);
	strcat(cfgpath, SETUPFILENAME);
	if (access(cfgpath, F_OK) == -1)
	{
		FILE *in = fopen(SETUPFILENAME, "rb");
		if (in)
		{
			FILE *out = fopen(cfgpath, "wb");
			if (out)
			{
				int ch;
				while ((ch = fgetc(in)) != EOF)
					fputc(ch, out);
				fclose(out);
			}
			fclose(in);
		}
	}
#else
	sprintf(ApogeePath, ".%s", PATH_SEP_STR);
#endif

	return 0;
}

int dukescreencapture(char* str, char inverseit)
{
	return 0;/*
// dos screencapture wants string to be in NAME0000.pcx format.
#ifndef PLATFORM_DOS
    // respect prefpath...
    const char *SCREENSHOTDIR = "Screenshots";
    size_t slen = strlen(ApogeePath) + strlen(str) +
                  strlen(PATH_SEP_STR) + strlen(SCREENSHOTDIR) + 1;
    char *path = alloca(slen);
    strcpy(path, ApogeePath);
    strcat(path, SCREENSHOTDIR);
	mkdir(path);//, S_IRWXU);
    strcat(path, PATH_SEP_STR);
    strcat(path, str);
    str = path;
#endif

    return(screencapture(str, inverseit));*/
}

char CheckParm(char* check)
{
	int i;
	for (i = 1; i < _argc; i++)
	{
		if ((*(_argv[i]) == '-') && (strcmpi(_argv[i] + 1, check) == 0))
			return(i);
	}

	return(0);
}

void RegisterShutdownFunction(void(* shutdown)())
{
	shutdown_func = shutdown;
}

void Shutdown()
{
	if (shutdown_func != NULL)
	{
		shutdown_func();
		shutdown_func = NULL;
	}
}

void* SafeMalloc(long size)
{
    void *ptr;

#if 0
    if (zonememorystarted==false)
        Error("Called SafeMalloc without starting zone memory\n");
    ptr = Z_Malloc (size,PU_STATIC,NULL);
#else
    ptr = malloc(size);
#endif

    if (!ptr)
        Error ("SafeMalloc failure for %lu bytes",size);

    return ptr;
}

void SafeRealloc(void** x, int32_t size)
{
    void *ptr;

#if 0
    if (zonememorystarted==false)
        Error("Called SafeMalloc without starting zone memory\n");
    ptr = Z_Malloc (size,PU_STATIC,NULL);
#else
    ptr = realloc(*x, size);
#endif

    if (!ptr)
        Error ("SafeRealloc failure for %lu bytes",size);

    *x = ptr;
}

void Error (char *error, ...)
{
   char msgbuf[300];
   va_list argptr;
   static int inerror = 0;

   inerror++;
   if (inerror > 1)
      return;

   #if USE_SDL
   SDL_Quit();
   #endif

   va_start (argptr, error);
   vfprintf(stderr, error, argptr);
   va_end (argptr);

   exit (1);
}

void FixFilePath(char* filename)
{
#if defined(PLATFORM_UNIX) && !defined(DC)
	char *ptr;
	char *lastsep = filename;

	if ((!filename) || (*filename == '\0'))
		return;

	if (access(filename, F_OK) == 0)  /* File exists; we're good to go. */
		return;

	for (ptr = filename; 1; ptr++)
	{
		if (*ptr == '\\')
			*ptr = PATH_SEP_CHAR;

		if ((*ptr == PATH_SEP_CHAR) || (*ptr == '\0'))
		{
			char pch = *ptr;
			dirent *dent = NULL;
			DIR *dir;

			if ((pch == PATH_SEP_CHAR) && (*(ptr + 1) == '\0'))
				return; /* eos is pathsep; we're done. */

			if (lastsep == ptr)
				continue;  /* absolute path; skip to next one. */

			*ptr = '\0';
			if (lastsep == filename) {
				dir = opendir((*lastsep == PATH_SEP_CHAR) ? ROOTDIR : CURDIR);

				if (*lastsep == PATH_SEP_CHAR) {
					lastsep++;
				}
			}
			else
			{
				*lastsep = '\0';
				dir = opendir(filename);
				*lastsep = PATH_SEP_CHAR;
				lastsep++;
			}

			if (dir == NULL)
			{
				*ptr = PATH_SEP_CHAR;
				return;  /* maybe dir doesn't exist? give up. */
			}

			while ((dent = readdir(dir)) != NULL)
			{
				if (strcasecmp(dent->d_name, lastsep) == 0)
				{
					/* found match; replace it. */
					strcpy(lastsep, dent->d_name);
					break;
				}
			}

			closedir(dir);
			*ptr = pch;
			lastsep = ptr;

			if (dent == NULL)
				return;  /* no match. oh well. */

			if (pch == '\0')  /* eos? */
				return;
		}
	}
#endif
}


int32_t SafeOpenAppend (const char *_filename, int32_t filetype)
{
//	int	handle;
//    char filename[MAX_PATH];
//    strncpy(filename, _filename, sizeof (filename));
//    filename[sizeof (filename) - 1] = '\0';
//    FixFilePath(filename);
//
//	handle = open(filename,O_RDWR | O_BINARY | O_CREAT | O_APPEND
//	, S_IREAD | S_IWRITE);
//
//	if (handle == -1)
//		Error ("Error opening for append %s: %s",filename,strerror(errno));
//
//	return handle;
}

bool SafeFileExists ( const char * _filename )
{
    char filename[MAX_PATH];
    strncpy(filename, _filename, sizeof (filename));
    filename[sizeof (filename) - 1] = '\0';
    FixFilePath(filename);

    return(_access(filename, 0) == 0);
}


int32_t SafeOpenWrite (const char *_filename, int32_t filetype)
{
	//int	handle;
    //char filename[MAX_PATH];
    //strncpy(filename, _filename, sizeof (filename));
    //filename[sizeof (filename) - 1] = '\0';
    //FixFilePath(filename);
//
	//handle = open(filename,O_RDWR | O_BINARY | O_CREAT | O_TRUNC
	//, S_IREAD | S_IWRITE);
//
	//if (handle == -1)
	//	Error ("Error opening %s: %s",filename,strerror(errno));
//
	//return handle;
}

int32_t SafeOpenRead (const char *_filename, int32_t filetype)
{
//	int	handle;
//    char filename[MAX_PATH];
//    strncpy(filename, _filename, sizeof (filename));
//    filename[sizeof (filename) - 1] = '\0';
//    FixFilePath(filename);
//
//	handle = open(filename,O_RDONLY | O_BINARY);
//
//	if (handle == -1)
//		Error ("Error opening %s: %s",filename,strerror(errno));
//
//	return handle;
}


void SafeRead (int32_t handle, void *buffer, int32_t count)
{
	unsigned	iocount;

	while (count)
	{
		iocount = count > 0x8000 ? 0x8000 : count;
		if (read (handle,buffer,iocount) != (int)iocount)
			Error ("File read failure reading %ld bytes",count);
		buffer = (void *)( (int8_t *)buffer + iocount );
		count -= iocount;
	}
}


void SafeWrite (int32_t handle, void *buffer, int32_t count)
{
	unsigned	iocount;

	while (count)
	{
		iocount = count > 0x8000 ? 0x8000 : count;
		if (write (handle,buffer,iocount) != (int)iocount)
			Error ("File write failure writing %ld bytes",count);
		buffer = (void *)( (int8_t *)buffer + iocount );
		count -= iocount;
	}
}


void GetPathFromEnvironment( char *fullname, int32_t length, const char *filename )
{
	snprintf(fullname, length-1, "%s%s", ApogeePath, filename);
}

void SafeWriteString (int handle, char * buffer)
{
	unsigned	iocount;

   iocount=strlen(buffer);
	if (write (handle,buffer,iocount) != (int)iocount)
			Error ("File write string failure writing %s\n",buffer);
}

void *SafeMalloc (long size);

void SafeRealloc (void **x, int32_t size);

void *SafeLevelMalloc (long size)
{
	void *ptr;

#if 0
   if (zonememorystarted==false)
      Error("Called SafeLevelMalloc without starting zone memory\n");
   ptr = Z_LevelMalloc (size,PU_STATIC,NULL);
#else
    ptr = malloc(size);
#endif

	if (!ptr)
      Error ("SafeLevelMalloc failure for %lu bytes",size);

	return ptr;
}

void SafeFree (void * ptr)
{
   if ( ptr == NULL )
      Error ("SafeFree : Tried to free a freed pointer\n");

#if 0
	Z_Free (ptr);
#else
    free(ptr);
#endif
}
