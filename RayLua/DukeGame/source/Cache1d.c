#include "cache1d.h"

int initcache(long dacachestart, long dacachesize)
{
    long i;

    for(i=1;i<200;i++) lockrecip[i] = (1<<28)/(200-i);

    cachestart = dacachestart;
    cachesize = dacachesize;

    cac[0].leng = cachesize;
    cac[0].lock = &zerochar;
    cacnum = 1;
}

int allocache(long* newhandle, long newbytes, char* newlockptr)
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

int suckcache(long* suckptr)
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

int agecache()
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

int reportandexit(char* errormessage)
{
    long i, j;

    setvmode(0x3);
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
    exit(0);
}

int initgroupfile(char* filename)
{
    char buf[16];
    long i, j, k;

    if (numgroupfiles >= MAXGROUPFILES) return(-1);

    groupfil[numgroupfiles] = open(filename,O_BINARY|O_RDWR,S_IREAD);
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
        { printf("Not enough memory for file grouping system\n"); exit(0); }
        if ((gfileoffs[numgroupfiles] = (long *)kmalloc((gnumfiles[numgroupfiles]+1)<<2)) == 0)
        { printf("Not enough memory for file grouping system\n"); exit(0); }

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

int uninitgroupfile()
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

int kopen4load(char* filename, char searchfirst)
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
            exit(0);
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

int klseek(long handle, long offset, long whence)
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

int kfilelength(long handle)
{
    long i, groupnum;

    groupnum = filegrp[handle];
    if (groupnum == 255) return(filelength(filehan[handle]));
    i = filehan[handle];
    return(gfileoffs[groupnum][i+1]-gfileoffs[groupnum][i]);
}

int kclose(long handle)
{
    if (handle < 0) return;
    if (filegrp[handle] == 255) close(filehan[handle]);
    filehan[handle] = -1;
}
