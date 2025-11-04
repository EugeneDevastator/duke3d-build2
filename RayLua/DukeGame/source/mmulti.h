// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
#ifndef MMULTI_H
#define MMULTI_H
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

#if (PRINTERRORS)
	static char lasterrorgotnum[MAXPLAYERS];
#endif





extern long totalclock;  //MUST EXTERN 1 ANNOYING VARIABLE FROM GAME

short myconnectindex, numplayers;
short connecthead, connectpoint2[MAXPLAYERS];




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

void callcommit();

void initmultiplayers(char damultioption, char dacomrateoption, char dapriority);

void initcrc();

void setpackettimeout(long datimeoutcount, long daresendagaincount);

int getcrc(char *buffer, short bufleng);

void uninitmultiplayers();;

void sendlogon();;

void sendlogoff();
int getoutputcirclesize();

int setsocket(short newsocket);

void sendpacket(long other, char *bufptr, long messleng);

void dosendpackets(long other);

short getpacket (short *other, char *bufptr);

void flushpackets();

void genericmultifunction(long other, char *bufptr, long messleng, long command);
#endif