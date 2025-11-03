// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <dos.h>
#include <process.h>
#include <stdarg.h>
#include "pragmas.h"

#define MAXPLAYERS 16
#define BAKSIZ 16384
#define SIMULATEERRORS 0
#define SHOWSENDPACKETS 0
#define SHOWGETPACKETS 0
#define PRINTERRORS 0

#define updatecrc16(crc,dat) crc = (((crc<<8)&65535)^crctable[((((unsigned short)crc)>>8)&65535)^dat])

static long incnt[MAXPLAYERS], outcntplc[MAXPLAYERS], outcntend[MAXPLAYERS];
static char errorgotnum[MAXPLAYERS];
static char errorfixnum[MAXPLAYERS];
static char errorresendnum[MAXPLAYERS];
#if (PRINTERRORS)
	static char lasterrorgotnum[MAXPLAYERS];
#endif

long crctable[256];

static char lastpacket[576], inlastpacket = 0;
static short lastpacketfrom, lastpacketleng;

extern long totalclock;  //MUST EXTERN 1 ANNOYING VARIABLE FROM GAME
static long timeoutcount = 60, resendagaincount = 4, lastsendtime[MAXPLAYERS];

static short bakpacketptr[MAXPLAYERS][256], bakpacketlen[MAXPLAYERS][256];
static char bakpacketbuf[BAKSIZ];
static long bakpacketplc = 0;

short myconnectindex, numplayers;
short connecthead, connectpoint2[MAXPLAYERS];
char syncstate = 0;

extern int _argc;
extern char **_argv;

#define MAXPACKETSIZE 2048
typedef struct
{
	short intnum;                //communication between Game and the driver
	short command;               //1-send, 2-get
	short other;                 //dest for send, set by get (-1 = no packet)
	short numbytes;
	short myconnectindex;
	short numplayers;
	short gametype;              //gametype: 1-serial,2-modem,3-net
	short filler;
	char buffer[MAXPACKETSIZE];
	long longcalladdress;
} gcomtype;
static gcomtype *gcom;

// static union REGS regs;

#pragma aux longcall =\
	"call eax",\
	parm [eax]\

void callcommit();

void initmultiplayers(char damultioption, char dacomrateoption, char dapriority);

void initcrc();

void setpackettimeout(long datimeoutcount, long daresendagaincount);

int getcrc(char *buffer, short bufleng);

void uninitmultiplayers()
{
}

void sendlogon()
{
}

void sendlogoff();
int getoutputcirclesize();

int setsocket(short newsocket);

void sendpacket(long other, char *bufptr, long messleng);

void dosendpackets(long other);

short getpacket (short *other, char *bufptr)
{
	long i, j, messleng;
	unsigned short dacrc;

	if (numplayers < 2) return(0);

	for(i=connecthead;i>=0;i=connectpoint2[i])
		if (i != myconnectindex)
		{
			if (totalclock < lastsendtime[i]) lastsendtime[i] = totalclock;
			if (totalclock > lastsendtime[i]+timeoutcount)
			{
#if (PRINTERRORS)
					printf(" TimeOut!");
#endif
					errorgotnum[i] = errorfixnum[i]+1;

					if ((outcntplc[i] == outcntend[i]) && (outcntplc[i] > 0))
						{ outcntplc[i]--; lastsendtime[i] = totalclock; }
					else
						lastsendtime[i] += resendagaincount;
					dosendpackets(i);
				//}
			}
		}

	if (inlastpacket != 0)
	{
			//2ND half of good double-packet
		inlastpacket = 0;
		*other = lastpacketfrom;
		memcpy(bufptr,lastpacket,lastpacketleng);
		return(lastpacketleng);
	}

	gcom->command = 2;
	callcommit();

#if (SHOWGETPACKETS)
	if (gcom->other != -1)
	{
		printf(" Get(%ld): ",gcom->other);
		for(i=0;i<gcom->numbytes;i++) printf("%2x ",gcom->buffer[i]);
		printf("\n");
	}
#endif

	if (gcom->other < 0) return(0);
	*other = gcom->other-1;

	messleng = gcom->numbytes;

	dacrc = ((unsigned short)gcom->buffer[messleng-2]);
	dacrc += (((unsigned short)gcom->buffer[messleng-1])<<8);
	if (dacrc != getcrc(gcom->buffer,messleng-2))        //CRC check
	{
#if (PRINTERRORS)
		printf("\n%ld CRC",gcom->buffer[0]);
#endif
		errorgotnum[*other] = errorfixnum[*other]+1;
		return(0);
	}

	while ((errorfixnum[*other]&7) != ((gcom->buffer[1]>>3)&7))
		errorfixnum[*other]++;

	if ((gcom->buffer[1]&7) != (errorresendnum[*other]&7))
	{
		errorresendnum[*other]++;
		outcntplc[*other] = (outcntend[*other]&0xffffff00)+gcom->buffer[2];
		if (outcntplc[*other] > outcntend[*other]) outcntplc[*other] -= 256;
	}

	if (gcom->buffer[0] != (incnt[*other]&255))   //CNT check
	{
		if (((incnt[*other]-gcom->buffer[0])&255) > 32)
		{
			errorgotnum[*other] = errorfixnum[*other]+1;
#if (PRINTERRORS)
			printf("\n%ld CNT",gcom->buffer[0]);
#endif
		}
#if (PRINTERRORS)
		else
		{
			if (!(gcom->buffer[1]&128))           //single else double packet
				printf("\n%ld cnt",gcom->buffer[0]);
			else
			{
				if (((gcom->buffer[0]+1)&255) == (incnt[*other]&255))
				{
								 //GOOD! Take second half of double packet
#if (PRINTERRORS)
					printf("\n%ld-%ld .� ",gcom->buffer[0],(gcom->buffer[0]+1)&255);
#endif
					messleng = ((long)gcom->buffer[3]) + (((long)gcom->buffer[4])<<8);
					lastpacketleng = gcom->numbytes-7-messleng;
					memcpy(bufptr,&gcom->buffer[messleng+5],lastpacketleng);
					incnt[*other]++;
					return(lastpacketleng);
				}
				else
					printf("\n%ld-%ld cnt ",gcom->buffer[0],(gcom->buffer[0]+1)&255);
			}
		}
#endif
		return(0);
	}

		//PACKET WAS GOOD!
	if ((gcom->buffer[1]&128) == 0)           //Single packet
	{
#if (PRINTERRORS)
		printf("\n%ld �  ",gcom->buffer[0]);
#endif

		messleng = gcom->numbytes-5;

		memcpy(bufptr,&gcom->buffer[3],messleng);

		incnt[*other]++;
		return(messleng);
	}

														 //Double packet
#if (PRINTERRORS)
	printf("\n%ld-%ld �� ",gcom->buffer[0],(gcom->buffer[0]+1)&255);
#endif

	messleng = ((long)gcom->buffer[3]) + (((long)gcom->buffer[4])<<8);
	lastpacketleng = gcom->numbytes-7-messleng;
	inlastpacket = 1; lastpacketfrom = *other;

	memcpy(bufptr,&gcom->buffer[5],messleng);
	memcpy(lastpacket,&gcom->buffer[messleng+5],lastpacketleng);

	incnt[*other] += 2;
	return(messleng);
}

void flushpackets();

void genericmultifunction(long other, char *bufptr, long messleng, long command);
