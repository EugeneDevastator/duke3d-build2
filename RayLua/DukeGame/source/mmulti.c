//
// Created by omnis on 11/3/2025.
//
#include "mmulti.h"

void  callcommit()
{
    //if (gcom->intnum&0xff00)
    //	longcall(gcom->longcalladdress);
    //else
    //	int386(gcom->intnum,&regs,&regs);
}

void initmultiplayers(char damultioption, char dacomrateoption, char dapriority)
{
    long i;
    char *parm, delims[4] = {'\\','-','/','\0'};

    initcrc();
    for(i=0;i<MAXPLAYERS;i++)
    {
        incnt[i] = 0L;
        outcntplc[i] = 0L;
        outcntend[i] = 0L;
        bakpacketlen[i][255] = -1;
    }

    for(i=_argc-1;i>0;i--)
        if ((parm = strtok(_argv[i],&delims[0])) != NULL)
            if (!stricmp("net",parm)) break;
    if (i == 0)
    {
        numplayers = 1; myconnectindex = 0;
        connecthead = 0; connectpoint2[0] = -1;
        return;
    }
    gcom = (gcomtype *)atol(_argv[i+1]);

    numplayers = gcom->numplayers;
    myconnectindex = gcom->myconnectindex-1;
#if (SIMULATEERRORS != 0)
    srand(myconnectindex*24572457+345356);
#endif
    connecthead = 0;
    for(i=0;i<numplayers-1;i++) connectpoint2[i] = i+1;
    connectpoint2[numplayers-1] = -1;

    for(i=0;i<numplayers;i++) lastsendtime[i] = totalclock;
}

void initcrc()
{
    long i, j, k, a;

    for(j=0;j<256;j++)      //Calculate CRC table
    {
        k = (j<<8); a = 0;
        for(i=7;i>=0;i--)
        {
            if (((k^a)&0x8000) > 0)
                a = ((a<<1)&65535) ^ 0x1021;   //0x1021 = genpoly
            else
                a = ((a<<1)&65535);
            k = ((k<<1)&65535);
        }
        crctable[j] = (a&65535);
    }
}

void setpackettimeout(long datimeoutcount, long daresendagaincount)
{
    long i;

    timeoutcount = datimeoutcount;
    resendagaincount = daresendagaincount;
    for(i=0;i<numplayers;i++) lastsendtime[i] = totalclock;
}

int getcrc(char* buffer, short bufleng)
{
    long i, j;

    j = 0;
    for(i=bufleng-1;i>=0;i--) updatecrc16(j,buffer[i]);
    return(j&65535);
}

void sendlogoff()
{
    long i;
    char tempbuf[2];

    tempbuf[0] = 255;
    tempbuf[1] = myconnectindex;
    for(i=connecthead;i>=0;i=connectpoint2[i])
        if (i != myconnectindex)
            sendpacket(i,tempbuf,2L);
}

int getoutputcirclesize()
{
    return(0);
}

int setsocket(short newsocket)
{
}

void sendpacket(long other, char* bufptr, long messleng)
{
    return;//
    long i, j, k, l,cnt;
    unsigned short dacrc;

    if (numplayers < 2) return;

    i = 0;
    if (bakpacketlen[other][(outcntend[other]-1)&255] == messleng)
    {
        j = bakpacketptr[other][(outcntend[other]-1)&255];
        for(i=messleng-1;i>=0;i--)
            if (bakpacketbuf[(i+j)&(BAKSIZ-1)] != bufptr[i]) break;
    }
    bakpacketlen[other][outcntend[other]&255] = messleng;

    if (i < 0)   //Point to last packet to save space on bakpacketbuf
        bakpacketptr[other][outcntend[other]&255] = j;
    else
    {
        bakpacketptr[other][outcntend[other]&255] = bakpacketplc;
        for(i=0;i<messleng;i++)
            bakpacketbuf[(bakpacketplc+i)&(BAKSIZ-1)] = bufptr[i];
        bakpacketplc = ((bakpacketplc+messleng)&(BAKSIZ-1));
    }
    outcntend[other]++;

    lastsendtime[other] = totalclock;
    dosendpackets(other);
}

void dosendpackets(long other)
{
    return;//
    long i, j, k, messleng;
    unsigned short dacrc;

    if (outcntplc[other] == outcntend[other]) return;

#if (PRINTERRORS)
    if (errorgotnum[other] > lasterrorgotnum[other])
    {
        lasterrorgotnum[other]++;
        printf(" MeWant %ld",incnt[other]&255);
    }
#endif

    if (outcntplc[other]+1 == outcntend[other])
    {     //Send 1 sub-packet
        k = 0;
        gcom->buffer[k++] = (outcntplc[other]&255);
        gcom->buffer[k++] = (errorgotnum[other]&7)+((errorresendnum[other]&7)<<3);
        gcom->buffer[k++] = (incnt[other]&255);

        j = bakpacketptr[other][outcntplc[other]&255];
        messleng = bakpacketlen[other][outcntplc[other]&255];
        for(i=0;i<messleng;i++)
            gcom->buffer[k++] = bakpacketbuf[(i+j)&(BAKSIZ-1)];
        outcntplc[other]++;
    }
    else
    {     //Send 2 sub-packets
        k = 0;
        gcom->buffer[k++] = (outcntplc[other]&255);
        gcom->buffer[k++] = (errorgotnum[other]&7)+((errorresendnum[other]&7)<<3)+128;
        gcom->buffer[k++] = (incnt[other]&255);

        //First half-packet
        j = bakpacketptr[other][outcntplc[other]&255];
        messleng = bakpacketlen[other][outcntplc[other]&255];
        gcom->buffer[k++] = (char)(messleng&255);
        gcom->buffer[k++] = (char)(messleng>>8);
        for(i=0;i<messleng;i++)
            gcom->buffer[k++] = bakpacketbuf[(i+j)&(BAKSIZ-1)];
        outcntplc[other]++;

        //Second half-packet
        j = bakpacketptr[other][outcntplc[other]&255];
        messleng = bakpacketlen[other][outcntplc[other]&255];
        for(i=0;i<messleng;i++)
            gcom->buffer[k++] = bakpacketbuf[(i+j)&(BAKSIZ-1)];
        outcntplc[other]++;

    }

    dacrc = getcrc(gcom->buffer,k);
    gcom->buffer[k++] = (dacrc&255);
    gcom->buffer[k++] = (dacrc>>8);

    gcom->other = other+1;
    gcom->numbytes = k;

#if (SHOWSENDPACKETS)
    printf("Send(%ld): ",gcom->other);
    for(i=0;i<gcom->numbytes;i++) printf("%2x ",gcom->buffer[i]);
    printf("\n");
#endif

#if (SIMULATEERRORS != 0)
    if (!(rand()&SIMULATEERRORS)) gcom->buffer[rand()%gcom->numbytes] = (rand()&255);
    if (rand()&SIMULATEERRORS)
#endif
    { gcom->command = 1; callcommit(); }
}

void flushpackets()
{
    /*long i;

	if (numplayers < 2) return;

	do
	{
		gcom->command = 2;
		callcommit();
	} while (gcom->other >= 0);

	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
		incnt[i] = 0L;
		outcntplc[i] = 0L;
		outcntend[i] = 0L;
		errorgotnum[i] = 0;
		errorfixnum[i] = 0;
		errorresendnum[i] = 0;
		lastsendtime[i] = totalclock;
	}*/
}

void genericmultifunction(long other, char* bufptr, long messleng, long command)
{
    if (numplayers < 2) return;

    gcom->command = command;
    gcom->numbytes = min(messleng,MAXPACKETSIZE);
    copybuf(bufptr,gcom->buffer,(gcom->numbytes+3)>>2);
    gcom->other = other+1;
    callcommit();
}
