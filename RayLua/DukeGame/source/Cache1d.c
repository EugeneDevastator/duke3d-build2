#include "cache1d.h"
#include "fcntl.h"
long cachesize = 0;
void  initcache(long dacachestart, long dacachesize)
{
    long i;

    for(i=1;i<200;i++) lockrecip[i] = (1<<28)/(200-i);

    cachestart = dacachestart;
    cachesize = dacachesize;

    cac[0].leng = cachesize;
    cac[0].lock = &zerochar;
    cacnum = 1;
}

void  allocache(long* newhandle, long newbytes, char* newlockptr)
{
    long i, j, z, zz, bestz, daval, bestval, besto, o1, o2, sucklen, suckz;

    newbytes = ((newbytes+15)&0xfffffff0);

    if ((unsigned)newbytes > (unsigned)cachesize)
    {
        printf("Cachesize: %ld\n",cachesize);
        printf("*Newhandle: 0x%x, Newbytes: %ld, *Newlock: %d\n",newhandle,newbytes,*newlockptr);
        reportandexit("BUFFER TOO BIG TO FIT IN CACHE!");
    }

    if (*newlockptr == 0)
    {
        reportandexit("ALLOCACHE CALLED WITH LOCK OF 0!");
    }

    //Find best place
    bestval = 0x7fffffff; o1 = cachesize;
    for(z=cacnum-1;z>=0;z--)
    {
        o1 -= cac[z].leng;
        o2 = o1+newbytes; if (o2 > cachesize) continue;

        daval = 0;
        for(i=o1,zz=z;i<o2;i+=cac[zz++].leng)
        {
            if (*cac[zz].lock == 0) continue;
            if (*cac[zz].lock >= 200) { daval = 0x7fffffff; break; }
            daval += mulscale32(cac[zz].leng+65536,lockrecip[*cac[zz].lock]);
            if (daval >= bestval) break;
        }
        if (daval < bestval)
        {
            bestval = daval; besto = o1; bestz = z;
            if (bestval == 0) break;
        }
    }

    //printf("%ld %ld %ld\n",besto,newbytes,*newlockptr);

    if (bestval == 0x7fffffff)
        reportandexit("CACHE SPACE ALL LOCKED UP!");

    //Suck things out
    for(sucklen=-newbytes,suckz=bestz;sucklen<0;sucklen+=cac[suckz++].leng)
        if (*cac[suckz].lock) *cac[suckz].hand = 0;

    //Remove all blocks except 1
    suckz -= (bestz+1); cacnum -= suckz;
    copybufbyte(&cac[bestz+suckz],&cac[bestz],(cacnum-bestz)*sizeof(cactype));
    cac[bestz].hand = newhandle; *newhandle = cachestart+besto;
    cac[bestz].leng = newbytes;
    cac[bestz].lock = newlockptr;
    cachecount++;

    //Add new empty block if necessary
    if (sucklen <= 0) return;

    bestz++;
    if (bestz == cacnum)
    {
        cacnum++; if (cacnum > MAXCACHEOBJECTS) reportandexit("Too many objects in cache! (cacnum > MAXCACHEOBJECTS)");
        cac[bestz].leng = sucklen;
        cac[bestz].lock = &zerochar;
        return;
    }

    if (*cac[bestz].lock == 0) { cac[bestz].leng += sucklen; return; }

    cacnum++; if (cacnum > MAXCACHEOBJECTS) reportandexit("Too many objects in cache! (cacnum > MAXCACHEOBJECTS)");
    for(z=cacnum-1;z>bestz;z--) cac[z] = cac[z-1];
    cac[bestz].leng = sucklen;
    cac[bestz].lock = &zerochar;
}

void  suckcache(long* suckptr)
{
    long i;

    //Can't exit early, because invalid pointer might be same even though lock = 0
    for(i=0;i<cacnum;i++)
        if ((long)(*cac[i].hand) == (long)suckptr)
        {
            if (*cac[i].lock) *cac[i].hand = 0;
            cac[i].lock = &zerochar;
            cac[i].hand = 0;

            //Combine empty blocks
            if ((i > 0) && (*cac[i-1].lock == 0))
            {
                cac[i-1].leng += cac[i].leng;
                cacnum--; copybuf(&cac[i+1],&cac[i],(cacnum-i)*sizeof(cactype));
            }
            else if ((i < cacnum-1) && (*cac[i+1].lock == 0))
            {
                cac[i+1].leng += cac[i].leng;
                cacnum--; copybuf(&cac[i+1],&cac[i],(cacnum-i)*sizeof(cactype));
            }
        }
}

void agecache()
{
    long cnt;
    char ch;

    if (agecount >= cacnum) agecount = cacnum-1;
    for(cnt=(cacnum>>4);cnt>=0;cnt--)
    {
        ch = (*cac[agecount].lock);
        if (((ch-2)&255) < 198)
            (*cac[agecount].lock) = ch-1;

        agecount--; if (agecount < 0) agecount = cacnum-1;
    }
}

void reportandexit(char* errormessage)
{
    long i, j;

   // setvmode(0x3);
    j = 0;
    for(i=0;i<cacnum;i++)
    {
        printf("%ld- ",i);
        printf("ptr: 0x%x, ",*cac[i].hand);
        printf("leng: %ld, ",cac[i].leng);
        printf("lock: %ld\n",*cac[i].lock);
        j += cac[i].leng;
    }
    printf("Cachesize = %ld\n",cachesize);
    printf("Cacnum = %ld\n",cacnum);
    printf("Cache length sum = %ld\n",j);
    printf("ERROR: %s",errormessage);
   //exit(0);
}

int initgroupfile(char* filename)
{
    char buf[16];
    long i, j, k;

    if (numgroupfiles >= MAXGROUPFILES) return(-1);

    groupfil[numgroupfiles] = open(filename,0x8000|0x0002); // r bind
    if (groupfil[numgroupfiles] != -1)
    {
        groupfilpos[numgroupfiles] = 0;
        read(groupfil[numgroupfiles],buf,16);
        if ((buf[0] != 'K') || (buf[1] != 'e') || (buf[2] != 'n') ||
            (buf[3] != 'S') || (buf[4] != 'i') || (buf[5] != 'l') ||
            (buf[6] != 'v') || (buf[7] != 'e') || (buf[8] != 'r') ||
            (buf[9] != 'm') || (buf[10] != 'a') || (buf[11] != 'n'))
        {
            close(groupfil[numgroupfiles]);
            groupfil[numgroupfiles] = -1;
            return(-1);
        }
        gnumfiles[numgroupfiles] = *((long *)&buf[12]);

        if ((gfilelist[numgroupfiles] = (char *)kmalloc(gnumfiles[numgroupfiles]<<4)) == 0)
        { printf("Not enough memory for file grouping system\n"); }//exit(0); }
        if ((gfileoffs[numgroupfiles] = (long *)kmalloc((gnumfiles[numgroupfiles]+1)<<2)) == 0)
        { printf("Not enough memory for file grouping system\n"); }//exit(0); }

        read(groupfil[numgroupfiles],gfilelist[numgroupfiles],gnumfiles[numgroupfiles]<<4);

        j = 0;
        for(i=0;i<gnumfiles[numgroupfiles];i++)
        {
            k = *((long *)&gfilelist[numgroupfiles][(i<<4)+12]);
            gfilelist[numgroupfiles][(i<<4)+12] = 0;
            gfileoffs[numgroupfiles][i] = j;
            j += k;
        }
        gfileoffs[numgroupfiles][gnumfiles[numgroupfiles]] = j;
    }
    numgroupfiles++;
    return(groupfil[numgroupfiles-1]);
}

void uninitgroupfile()
{
    long i;

    for(i=numgroupfiles-1;i>=0;i--)
        if (groupfil[i] != -1)
        {
            kfree(gfilelist[i]);
            kfree(gfileoffs[i]);
            close(groupfil[i]);
            groupfil[i] = -1;
        }
}

long kopen4load(char* filename, char searchfirst)
{
    long i, j, k, fil, newhandle;
    char bad, *gfileptr;

    newhandle = MAXOPENFILES-1;
    while (filehan[newhandle] != -1)
    {
        newhandle--;
        if (newhandle < 0)
        {
            printf("TOO MANY FILES OPEN IN FILE GROUPING SYSTEM!");
            //exit(0);
        }
    }

    if (searchfirst == 0)
        if ((fil = open(filename,O_BINARY|O_RDONLY)) != -1)
        {
            filegrp[newhandle] = 255;
            filehan[newhandle] = fil;
            filepos[newhandle] = 0;
            return(newhandle);
        }
    for(k=numgroupfiles-1;k>=0;k--)
    {
        if (searchfirst != 0) k = 0;
        if (groupfil[k] != -1)
        {
            for(i=gnumfiles[k]-1;i>=0;i--)
            {
                gfileptr = (char *)&gfilelist[k][i<<4];

                bad = 0;
                for(j=0;j<13;j++)
                {
                    if (!filename[j]) break;
                    if (toupperlookup[filename[j]] != toupperlookup[gfileptr[j]])
                    { bad = 1; break; }
                }
                if (bad) continue;

                filegrp[newhandle] = k;
                filehan[newhandle] = i;
                filepos[newhandle] = 0;
                return(newhandle);
            }
        }
    }
    return(-1);
}

int kread(long handle, void* buffer, long leng)
{
    long i, j, filenum, groupnum;

    filenum = filehan[handle];
    groupnum = filegrp[handle];
    if (groupnum == 255) return(read(filenum,buffer,leng));

    if (groupfil[groupnum] != -1)
    {
        i = gfileoffs[groupnum][filenum]+filepos[handle];
        if (i != groupfilpos[groupnum])
        {
            lseek(groupfil[groupnum],i+((gnumfiles[groupnum]+1)<<4),SEEK_SET);
            groupfilpos[groupnum] = i;
        }
        leng = min(leng,(gfileoffs[groupnum][filenum+1]-gfileoffs[groupnum][filenum])-filepos[handle]);
        leng = read(groupfil[groupnum],buffer,leng);
        filepos[handle] += leng;
        groupfilpos[groupnum] += leng;
        return(leng);
    }

    return(0);
}

long klseek(long handle, long offset, long whence)
{
    long i, groupnum;

    groupnum = filegrp[handle];

    if (groupnum == 255) return(lseek(filehan[handle],offset,whence));
    if (groupfil[groupnum] != -1)
    {
        switch(whence)
        {
        case SEEK_SET: filepos[handle] = offset; break;
        case SEEK_END: i = filehan[handle];
            filepos[handle] = (gfileoffs[groupnum][i+1]-gfileoffs[groupnum][i])+offset;
            break;
        case SEEK_CUR: filepos[handle] += offset; break;
        }
        return(filepos[handle]);
    }
    return(-1);
}

long kfilelength(long handle)
{
    long i, groupnum;

    groupnum = filegrp[handle];
    if (groupnum == 255) return(filelength(filehan[handle]));
    i = filehan[handle];
    return(gfileoffs[groupnum][i+1]-gfileoffs[groupnum][i]);
}

void kclose(long handle)
{
    if (handle < 0) return;
    if (filegrp[handle] == 255) close(filehan[handle]);
    filehan[handle] = -1;
}

void kdfread(void* buffer, size_t dasizeof, size_t count, long fil)
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

void dfread(void* buffer, size_t dasizeof, size_t count, FILE* fil)
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

void dfwrite(void* buffer, size_t dasizeof, size_t count, FILE* fil)
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

long compress(char* lzwinbuf, long uncompleng, char* lzwoutbuf)
{
    return 0;

    long i, addr, newaddr, addrcnt, zx, *longptr;
    long bytecnt1, bitcnt, numbits, oneupnumbits;
    short *shortptr;

    for(i=255;i>=0;i--) { lzwbuf1[i] = i; lzwbuf3[i] = (i+1)&255; }
    clearbuf((void*)(lzwbuf2),256>>1,0xffffffff);
    clearbuf((void*)(lzwoutbuf),((uncompleng+15)+3)>>2,0L);

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

long uncompress(char* lzwinbuf, long compleng, char* lzwoutbuf)
{
    long strtot, currstr, numbits, oneupnumbits;
    long i, dat, leng, bitcnt, outbytecnt, *longptr;
    short *shortptr;

    shortptr = (short *)lzwinbuf;
    strtot = (long)shortptr[1];
    if (strtot == 0)
    {
        copybuf((void*)(lzwinbuf)+4,(void*)(lzwoutbuf),((compleng-4)+3)>>2);
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
