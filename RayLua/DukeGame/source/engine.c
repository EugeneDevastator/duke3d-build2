

#include "engine.h"
#include "cache1d.h"
#include "dukewrap.h"
#include "funct.h"
#include "pragmas.h"
//static void faketimerhandler (){};
sectortype sector[MAXSECTORS];
volatile long stereomode = 0, visualpage, activepage, whiteband, blackband;
volatile char oa1, o3c2, ortca, ortcb, overtbits, laststereoint;
long dommxoverlay = 1;
static long frameplace = 1;
long clipmoveboxtracenum = 3;
static long xb1[MAXWALLSB], yb1[MAXWALLSB], xb2[MAXWALLSB], yb2[MAXWALLSB];
static long rx1[MAXWALLSB], ry1[MAXWALLSB], rx2[MAXWALLSB], ry2[MAXWALLSB];
static short p2[MAXWALLSB], thesector[MAXWALLSB], thewall[MAXWALLSB];
char cachedebug = 0;
static short bunchfirst[MAXWALLSB], bunchlast[MAXWALLSB];

static short smost[MAXYSAVES], smostcnt;
static short smoststart[MAXWALLSB];
static char smostwalltype[MAXWALLSB];
static long smostwall[MAXWALLSB], smostwallcnt = -1L;

static short maskwall[MAXWALLSB], maskwallcnt;
static long spritesx[MAXSPRITESONSCREEN];
static long spritesy[MAXSPRITESONSCREEN + 1];
static long spritesz[MAXSPRITESONSCREEN];
static spritetype* tspriteptr[MAXSPRITESONSCREEN];

short umost[MAXXDIM], dmost[MAXXDIM];
static short bakumost[MAXXDIM], bakdmost[MAXXDIM];
short uplc[MAXXDIM], dplc[MAXXDIM];
static short uwall[MAXXDIM], dwall[MAXXDIM];
static long swplc[MAXXDIM], lplc[MAXXDIM];
static long swall[MAXXDIM], lwall[MAXXDIM + 4];
long xdimen = -1, xdimenrecip, halfxdimen, xdimenscale, xdimscale;
long wx1, wy1, wx2, wy2, ydimen;
long viewoffset, frameoffset;

static long rxi[8], ryi[8], rzi[8], rxi2[8], ryi2[8], rzi2[8];
static long xsi[8], ysi[8], *horizlookup, *horizlookup2, horizycent;

long globalposx, globalposy, globalposz, globalhoriz;
short globalang, globalcursectnum;
long globalpal, cosglobalang, singlobalang;
long cosviewingrangeglobalang, sinviewingrangeglobalang;
char* globalpalwritten;
long globaluclip, globaldclip, globvis;
long globalvisibility, globalhisibility, globalpisibility, globalcisibility;
char globparaceilclip, globparaflorclip;

long xyaspect, viewingrangerecip;

long asm1, asm2, asm3, asm4;
long vplce[4], vince[4], palookupoffse[4], bufplce[4];
char globalxshift, globalyshift;
long globalxpanning, globalypanning, globalshade;
short globalpicnum, globalshiftval;
long globalzd, globalbufplc, globalyscale, globalorientation;
long globalx1, globaly1, globalx2, globaly2, globalx3, globaly3, globalzx;
long globalx, globaly, globalz;
static long rdist[129], gdist[129], bdist[129];
static char colhere[((FASTPALGRIDSIZ + 2) * (FASTPALGRIDSIZ + 2) * (FASTPALGRIDSIZ + 2)) >> 3];
static char colhead[(FASTPALGRIDSIZ + 2) * (FASTPALGRIDSIZ + 2) * (FASTPALGRIDSIZ + 2)];
static long colnext[256];
static char coldist[8] = {0, 1, 2, 3, 4, 3, 2, 1};
static long colscan[27];
static linetype clipit[MAXCLIPNUM];
static short clipsectorlist[MAXCLIPNUM], clipsectnum;
static short clipobjectval[MAXCLIPNUM];

static short clipnum, hitwalls[4];
long hitscangoalx = (1 << 29) - 1, hitscangoaly = (1 << 29) - 1;

static short sectorborder[256], sectorbordercnt;
static char tablesloaded = 0;
long pageoffset, ydim16, qsetmode = 0;
long startposx, startposy, startposz;
short startang, startsectnum;
short pointhighlight, linehighlight, highlightcnt;
static long lastx[MAXYDIM];
char *transluc, paletteloaded;
static char pcxheader[128] =
{
    0xa, 0x5, 0x1, 0x8, 0x0, 0x0, 0x0, 0x0, 0x3f, 0x1, 0xc7, 0x0,
    0x40, 0x1, 0xc8, 0x0, 0x0, 0x0, 0x0, 0x8, 0x8, 0x8, 0x10, 0x10,
    0x10, 0x18, 0x18, 0x18, 0x20, 0x20, 0x20, 0x28, 0x28, 0x28, 0x30, 0x30,
    0x30, 0x38, 0x38, 0x38, 0x40, 0x40, 0x40, 0x48, 0x48, 0x48, 0x50, 0x50,
    0x50, 0x58, 0x58, 0x58, 0x60, 0x60, 0x60, 0x68, 0x68, 0x68, 0x70, 0x70,
    0x70, 0x78, 0x78, 0x78, 0x0, 0x1, 0x40, 0x1, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};
static permfifotype permfifo[MAXPERMS];
static long permhead = 0, permtail = 0;

short numscans, numhits, numbunches;
static short posfil, capturecount = 0, hitcnt;

short editstatus = 0;
short searchit;
long searchx = -1, searchy; //search input
short searchsector, searchwall, searchstat; //search output

long totalclocklock;

static char artfilename[20];
static long numtilefiles, artfil = -1, artfilnum, artfilplc;

static char inpreparemirror = 0;
static long mirrorsx1, mirrorsy1, mirrorsx2, mirrorsy2;

static char vgapal16[48] =
{
    00, 00, 00, 00, 00, 42, 00, 42, 00, 00, 42, 42, 42, 00, 00, 42, 00, 42, 42, 21, 00, 42, 42, 42,
    21, 21, 21, 21, 21, 63, 21, 63, 21, 21, 63, 63, 63, 21, 21, 63, 21, 63, 63, 63, 21, 63, 63, 63,
};

static long cachesize;

static char globalpolytype;
static short *dotp1[MAXYDIM], *dotp2[MAXYDIM];
long voxoff[MAXVOXELS][MAXVOXMIPS], voxlock[MAXVOXELS][MAXVOXMIPS];
static long ggxinc[MAXXSIZ + 1], ggyinc[MAXXSIZ + 1];
static long lowrecip[1024], nytooclose, nytoofar;
static unsigned long distrecip[16384];
static unsigned char tempbuf[MAXWALLS];

long transarea = 0, totalarea = 0, beforedrawrooms = 1;

static long oxdimen = -1, oviewingrange = -1, oxyaspect = -1;

static long curbrightness = 0;
long ebpbak, espbak;
long slopalookup[2048];

static char permanentlock = 255;
long artversion, mapversion;
char* pic = NULL;
char picsiz[MAXTILES], tilefilenum[MAXTILES];
long lastageclock;
long tilefileoffs[MAXTILES];
long reciptable[2048], fpuasm;

char britable[16][64];
char textfont[1024], smalltextfont[1024];

long pow2long[32] =
{
    1L, 2L, 4L, 8L,
    16L, 32L, 64L, 128L,
    256L, 512L, 1024L, 2048L,
    4096L, 8192L, 16384L, 32768L,
    65536L, 131072L, 262144L, 524288L,
    1048576L, 2097152L, 4194304L, 8388608L,
    16777216L, 33554432L, 67108864L, 134217728L,
    268435456L, 536870912L, 1073741824L, 2147483647L,
};
static long artsize = 0;

static short radarang[1280], radarang2[MAXXDIM];
unsigned short sqrtable[4096], shlookup[4096 + 256];
char pow2char[8] = {1, 2, 4, 8, 16, 32, 64, 128};

static long stereowidth = 23040, stereopixelwidth = 28, ostereopixelwidth = -1;

uint32_t nsqrtasm(uint32_t value)
{
    uint32_t temp = value;
    uint16_t shift_info;

    if (value & 0xff000000)
    {
        temp >>= 24;
        shift_info = shlookup[temp + 4096]; // +4096 for offset 8192/2
    }
    else
    {
        temp >>= 12;
        shift_info = shlookup[temp];
    }

    value >>= (shift_info & 0xff);
    value = sqrtable[value];
    value >>= (shift_info >> 8);

    return value;
}

uint32_t msqrtasm(uint32_t value)
{
    uint32_t result = 0x40000000;
    uint32_t bit = 0x20000000;

    while (bit != 0) {
        if (value >= result) {
            value -= result;
            result += bit << 2;
        }
        result -= bit;
        result >>= 1;
        bit >>= 2;
    }

    if (value >= result) {
        result++;
    }

    return result >> 1;
}

int32_t krecipasm(int32_t input)
{
    union
    {
        float f;
        int32_t i;
    } fpuasm;

    // Convert integer to float and back to get IEEE 754 representation
    fpuasm.f = (float)input;

    // Extract the original input for further processing
    int32_t eax = fpuasm.i;
    int32_t ebx, ecx;

    // add eax, eax + sbb ebx, ebx (sets ebx to -1 if carry, 0 otherwise)
    uint64_t temp = (uint64_t)eax + (uint64_t)eax;
    ebx = (temp > 0xFFFFFFFF) ? -1 : 0;
    eax = (int32_t)temp;

    // mov ecx, eax
    ecx = eax;

    // and eax, 0x007ff000 + shr eax, 10
    eax = (eax & 0x007ff000) >> 10;

    // sub ecx, 0x3f800000 + shr ecx, 23
    ecx = (ecx - 0x3f800000) >> 23;

    // mov eax, dword ptr reciptable[eax]
    eax = reciptable[eax];

    // sar eax, cl (arithmetic right shift)
    eax = eax >> ecx;

    // xor eax, ebx
    eax = eax ^ ebx;

    return eax;
}

void setgotpic(uint32_t index)
{
    if (walock[index] < 200) {
        walock[index] = 199;
    }

    uint32_t byte_index = index >> 3;
    uint32_t bit_mask = 1 << (index & 7);

    gotpic[byte_index] |= bit_mask;
}

// many will be unused so need to pass by header.
//we can use this to forward camera pos, ezpz
void drawrooms(long daposx, long daposy, long daposz, short daang, long dahoriz, short dacursectnum)
{
    //do the actual render at position.
}

void scansector(short sectnum)
{
    walltype *wal, *wal2;
    spritetype* spr;
    long i, xs, ys, xp, yp, x1, y1, x2, y2, xp1, yp1, xp2, yp2, templong;
    short z, zz, startwall, endwall, numscansbefore, scanfirst, bunchfrst;
    short nextsectnum;

    if (sectnum < 0) return;

    if (automapping) show2dsector[sectnum >> 3] |= pow2char[sectnum & 7];

    sectorborder[0] = sectnum, sectorbordercnt = 1;
    do
    {
        sectnum = sectorborder[--sectorbordercnt];

        for (z = headspritesect[sectnum]; z >= 0; z = nextspritesect[z])
        {
            spr = &sprite[z];
            if ((((spr->cstat & 0x8000) == 0) || (showinvisibility)) &&
                (spr->xrepeat > 0) && (spr->yrepeat > 0) &&
                (spritesortcnt < MAXSPRITESONSCREEN))
            {
                xs = spr->x - globalposx;
                ys = spr->y - globalposy;
                if ((spr->cstat & 48) || (xs * cosglobalang + ys * singlobalang > 0))
                {
                    copybufbyte(spr, &tsprite[spritesortcnt], sizeof(spritetype));
                    tsprite[spritesortcnt++].owner = z;
                }
            }
        }

        gotsector[sectnum >> 3] |= pow2char[sectnum & 7];

        bunchfrst = numbunches;
        numscansbefore = numscans;

        startwall = sector[sectnum].wallptr;
        endwall = startwall + sector[sectnum].wallnum;
        scanfirst = numscans;
        for (z = startwall, wal = &wall[z]; z < endwall; z++, wal++)
        {
            nextsectnum = wal->nextsector;

            wal2 = &wall[wal->point2];
            x1 = wal->x - globalposx;
            y1 = wal->y - globalposy;
            x2 = wal2->x - globalposx;
            y2 = wal2->y - globalposy;

            if ((nextsectnum >= 0) && ((wal->cstat & 32) == 0))
                if ((gotsector[nextsectnum >> 3] & pow2char[nextsectnum & 7]) == 0)
                {
                    templong = x1 * y2 - x2 * y1;
                    if (((unsigned)templong + 262144) < 524288)
                        if (mulscale5(templong, templong) <= (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1))
                            sectorborder[sectorbordercnt++] = nextsectnum;
                }

            if ((z == startwall) || (wall[z - 1].point2 != z))
            {
                xp1 = dmulscale6(y1, cosglobalang, -x1, singlobalang);
                yp1 = dmulscale6(x1, cosviewingrangeglobalang, y1, sinviewingrangeglobalang);
            }
            else
            {
                xp1 = xp2;
                yp1 = yp2;
            }
            xp2 = dmulscale6(y2, cosglobalang, -x2, singlobalang);
            yp2 = dmulscale6(x2, cosviewingrangeglobalang, y2, sinviewingrangeglobalang);
            if ((yp1 < 256) && (yp2 < 256)) goto skipitaddwall;

            //If wall's NOT facing you
            if (dmulscale32(xp1, yp2, -xp2, yp1) >= 0) goto skipitaddwall;

            if (xp1 >= -yp1)
            {
                if ((xp1 > yp1) || (yp1 == 0)) goto skipitaddwall;
                xb1[numscans] = halfxdimen + scale(xp1, halfxdimen, yp1);
                if (xp1 >= 0) xb1[numscans]++; //Fix for SIGNED divide
                if (xb1[numscans] >= xdimen) xb1[numscans] = xdimen - 1;
                yb1[numscans] = yp1;
            }
            else
            {
                if (xp2 < -yp2) goto skipitaddwall;
                xb1[numscans] = 0;
                templong = yp1 - yp2 + xp1 - xp2;
                if (templong == 0) goto skipitaddwall;
                yb1[numscans] = yp1 + scale(yp2 - yp1, xp1 + yp1, templong);
            }
            if (yb1[numscans] < 256) goto skipitaddwall;

            if (xp2 <= yp2)
            {
                if ((xp2 < -yp2) || (yp2 == 0)) goto skipitaddwall;
                xb2[numscans] = halfxdimen + scale(xp2, halfxdimen, yp2) - 1;
                if (xp2 >= 0) xb2[numscans]++; //Fix for SIGNED divide
                if (xb2[numscans] >= xdimen) xb2[numscans] = xdimen - 1;
                yb2[numscans] = yp2;
            }
            else
            {
                if (xp1 > yp1) goto skipitaddwall;
                xb2[numscans] = xdimen - 1;
                templong = xp2 - xp1 + yp1 - yp2;
                if (templong == 0) goto skipitaddwall;
                yb2[numscans] = yp1 + scale(yp2 - yp1, yp1 - xp1, templong);
            }
            if ((yb2[numscans] < 256) || (xb1[numscans] > xb2[numscans])) goto skipitaddwall;

            //Made it all the way!
            thesector[numscans] = sectnum;
            thewall[numscans] = z;
            rx1[numscans] = xp1;
            ry1[numscans] = yp1;
            rx2[numscans] = xp2;
            ry2[numscans] = yp2;
            p2[numscans] = numscans + 1;
            numscans++;
        skipitaddwall:

            if ((wall[z].point2 < z) && (scanfirst < numscans))
                p2[numscans - 1] = scanfirst, scanfirst = numscans;
        }

        for (z = numscansbefore; z < numscans; z++)
            if ((wall[thewall[z]].point2 != thewall[p2[z]]) || (xb2[z] >= xb1[p2[z]]))
                bunchfirst[numbunches++] = p2[z], p2[z] = -1;

        for (z = bunchfrst; z < numbunches; z++)
        {
            for (zz = bunchfirst[z]; p2[zz] >= 0; zz = p2[zz]);
            bunchlast[z] = zz;
        }
    }
    while (sectorbordercnt > 0);
}

int wallfront(long l1, long l2)
{
    walltype* wal;
    long x11, y11, x21, y21, x12, y12, x22, y22, dx, dy, t1, t2;

    wal = &wall[thewall[l1]];
    x11 = wal->x;
    y11 = wal->y;
    wal = &wall[wal->point2];
    x21 = wal->x;
    y21 = wal->y;
    wal = &wall[thewall[l2]];
    x12 = wal->x;
    y12 = wal->y;
    wal = &wall[wal->point2];
    x22 = wal->x;
    y22 = wal->y;

    dx = x21 - x11;
    dy = y21 - y11;
    t1 = dmulscale2(x12 - x11, dy, -dx, y12 - y11); //p1(l2) vs. l1
    t2 = dmulscale2(x22 - x11, dy, -dx, y22 - y11); //p2(l2) vs. l1
    if (t1 == 0)
    {
        t1 = t2;
        if (t1 == 0) return (-1);
    }
    if (t2 == 0) t2 = t1;
    if ((t1 ^ t2) >= 0)
    {
        t2 = dmulscale2(globalposx - x11, dy, -dx, globalposy - y11); //pos vs. l1
        return ((t2 ^ t1) >= 0);
    }

    dx = x22 - x12;
    dy = y22 - y12;
    t1 = dmulscale2(x11 - x12, dy, -dx, y11 - y12); //p1(l1) vs. l2
    t2 = dmulscale2(x21 - x12, dy, -dx, y21 - y12); //p2(l1) vs. l2
    if (t1 == 0)
    {
        t1 = t2;
        if (t1 == 0) return (-1);
    }
    if (t2 == 0) t2 = t1;
    if ((t1 ^ t2) >= 0)
    {
        t2 = dmulscale2(globalposx - x12, dy, -dx, globalposy - y12); //pos vs. l2
        return ((t2 ^ t1) < 0);
    }
    return (-2);
}

int32_t spritewallfront(spritetype* s, long w)
{
    walltype* wal;
    long x1, y1;

    wal = &wall[w];
    x1 = wal->x;
    y1 = wal->y;
    wal = &wall[wal->point2];
    return (dmulscale32(wal->x - x1, s->y - y1, -(s->x - x1), wal->y - y1) >= 0);
}

int bunchfront(long b1, long b2)
{
    long x1b1, x2b1, x1b2, x2b2, b1f, b2f, i;

    b1f = bunchfirst[b1];
    x1b1 = xb1[b1f];
    x2b2 = xb2[bunchlast[b2]] + 1;
    if (x1b1 >= x2b2) return (-1);
    b2f = bunchfirst[b2];
    x1b2 = xb1[b2f];
    x2b1 = xb2[bunchlast[b1]] + 1;
    if (x1b2 >= x2b1) return (-1);

    if (x1b1 >= x1b2)
    {
        for (i = b2f; xb2[i] < x1b1; i = p2[i]);
        return (wallfront(b1f, i));
    }
    for (i = b1f; xb2[i] < x1b2; i = p2[i]);
    return (wallfront(i, b2f));
}

void drawalls(long bunch)
{
    return; //
    sectortype *sec, *nextsec;
    walltype* wal;
    long i, j, k, l, m, n, x, y, x1, x2, cz[5], fz[5];
    long z, wallnum, sectnum, nextsectnum, globalhorizbak;
    long startsmostwallcnt, startsmostcnt, gotswall;
    char andwstat1, andwstat2;

    z = bunchfirst[bunch];
    sectnum = thesector[z];
    sec = &sector[sectnum];

    andwstat1 = 0xff;
    andwstat2 = 0xff;
    // for(;z>=0;z=p2[z])  //uplc/dplc calculation
    // {
    //     andwstat1 &= wallmost(uplc,z,sectnum,(char)0);
    //     andwstat2 &= wallmost(dplc,z,sectnum,(char)1);
    // }
    //
    // if ((andwstat1&3) != 3)     //draw ceilings
    // {
    //     if ((sec->ceilingstat&3) == 2)
    //         grouscan(xb1[bunchfirst[bunch]],xb2[bunchlast[bunch]],sectnum,0);
    //     else if ((sec->ceilingstat&1) == 0)
    //         ceilscan(xb1[bunchfirst[bunch]],xb2[bunchlast[bunch]],sectnum);
    //     else
    //         parascan(xb1[bunchfirst[bunch]],xb2[bunchlast[bunch]],sectnum,0,bunch);
    // }
    // if ((andwstat2&12) != 12)   //draw floors
    // {
    //     if ((sec->floorstat&3) == 2)
    //         grouscan(xb1[bunchfirst[bunch]],xb2[bunchlast[bunch]],sectnum,1);
    //     else if ((sec->floorstat&1) == 0)
    //         florscan(xb1[bunchfirst[bunch]],xb2[bunchlast[bunch]],sectnum);
    //     else
    //         parascan(xb1[bunchfirst[bunch]],xb2[bunchlast[bunch]],sectnum,1,bunch);
    // }

    //DRAW WALLS SECTION!
    for (z = bunchfirst[bunch]; z >= 0; z = p2[z])
    {
        x1 = xb1[z];
        x2 = xb2[z];
        if (umost[x2] >= dmost[x2])
        {
            for (x = x1; x < x2; x++)
                if (umost[x] < dmost[x]) break;
            if (x >= x2)
            {
                smostwall[smostwallcnt] = z;
                smostwalltype[smostwallcnt] = 0;
                smostwallcnt++;
                continue;
            }
        }

        wallnum = thewall[z];
        wal = &wall[wallnum];
        nextsectnum = wal->nextsector;
        nextsec = &sector[nextsectnum];

        gotswall = 0;

        startsmostwallcnt = smostwallcnt;
        startsmostcnt = smostcnt;

        if ((searchit == 2) && (searchx >= x1) && (searchx <= x2))
        {
            if (searchy <= uplc[searchx]) //ceiling
            {
                searchsector = sectnum;
                searchwall = wallnum;
                searchstat = 1;
                searchit = 1;
            }
            else if (searchy >= dplc[searchx]) //floor
            {
                searchsector = sectnum;
                searchwall = wallnum;
                searchstat = 2;
                searchit = 1;
            }
        }

        if (nextsectnum >= 0)
        {
            getzsofslope((short)sectnum, wal->x, wal->y, &cz[0], &fz[0]);
            getzsofslope((short)sectnum, wall[wal->point2].x, wall[wal->point2].y, &cz[1], &fz[1]);
            getzsofslope((short)nextsectnum, wal->x, wal->y, &cz[2], &fz[2]);
            getzsofslope((short)nextsectnum, wall[wal->point2].x, wall[wal->point2].y, &cz[3], &fz[3]);
            getzsofslope((short)nextsectnum, globalposx, globalposy, &cz[4], &fz[4]);

            if ((wal->cstat & 48) == 16) maskwall[maskwallcnt++] = z;

            if (((sec->ceilingstat & 1) == 0) || ((nextsec->ceilingstat & 1) == 0))
            {
                if ((cz[2] <= cz[0]) && (cz[3] <= cz[1]))
                {
                    if (globparaceilclip)
                        for (x = x1; x <= x2; x++)
                            if (uplc[x] > umost[x])
                                if (umost[x] <= dmost[x])
                                {
                                    umost[x] = uplc[x];
                                    if (umost[x] > dmost[x]) numhits--;
                                }
                }
                else
                {
                    //      wallmost(dwall,z,nextsectnum,(char)0);
                    if ((cz[2] > fz[0]) || (cz[3] > fz[1]))
                        for (i = x1; i <= x2; i++) if (dwall[i] > dplc[i]) dwall[i] = dplc[i];

                    if ((searchit == 2) && (searchx >= x1) && (searchx <= x2))
                        if (searchy <= dwall[searchx]) //wall
                        {
                            searchsector = sectnum;
                            searchwall = wallnum;
                            searchstat = 0;
                            searchit = 1;
                        }

                    globalorientation = (long)wal->cstat;
                    globalpicnum = wal->picnum;
                    if ((unsigned)globalpicnum >= (unsigned)MAXTILES) globalpicnum = 0;
                    globalxpanning = (long)wal->xpanning;
                    globalypanning = (long)wal->ypanning;
                    globalshiftval = (picsiz[globalpicnum] >> 4);
                    if (pow2long[globalshiftval] != tilesizy[globalpicnum]) globalshiftval++;
                    globalshiftval = 32 - globalshiftval;
                    if (picanm[globalpicnum] & 192) globalpicnum += animateoffs(globalpicnum, (short)wallnum + 16384);
                    globalshade = (long)wal->shade;
                    globvis = globalvisibility;
                    if (sec->visibility != 0) globvis = mulscale4(globvis, (long)((unsigned char)(sec->visibility+16)));
                    globalpal = (long)wal->pal;
                    globalyscale = (wal->yrepeat << (globalshiftval - 19));
                    if ((globalorientation & 4) == 0)
                        globalzd = (((globalposz - nextsec->ceilingz) * globalyscale) << 8);
                    else
                        globalzd = (((globalposz - sec->ceilingz) * globalyscale) << 8);
                    globalzd += (globalypanning << 24);
                    if (globalorientation & 256) globalyscale = -globalyscale, globalzd = -globalzd;

                    if (gotswall == 0)
                    {
                        gotswall = 1;
                        prepwall(z, wal);
                    }
                    wallscan(x1, x2, uplc, dwall, swall, lwall);

                    if ((cz[2] >= cz[0]) && (cz[3] >= cz[1]))
                    {
                        for (x = x1; x <= x2; x++)
                            if (dwall[x] > umost[x])
                                if (umost[x] <= dmost[x])
                                {
                                    umost[x] = dwall[x];
                                    if (umost[x] > dmost[x]) numhits--;
                                }
                    }
                    else
                    {
                        for (x = x1; x <= x2; x++)
                            if (umost[x] <= dmost[x])
                            {
                                i = max(uplc[x], dwall[x]);
                                if (i > umost[x])
                                {
                                    umost[x] = i;
                                    if (umost[x] > dmost[x]) numhits--;
                                }
                            }
                    }
                }
                if ((cz[2] < cz[0]) || (cz[3] < cz[1]) || (globalposz < cz[4]))
                {
                    i = x2 - x1 + 1;
                    if (smostcnt + i < MAXYSAVES)
                    {
                        smoststart[smostwallcnt] = smostcnt;
                        smostwall[smostwallcnt] = z;
                        smostwalltype[smostwallcnt] = 1; //1 for umost
                        smostwallcnt++;
                        copybufbyte(&umost[x1], &smost[smostcnt], i * sizeof(smost[0]));
                        smostcnt += i;
                    }
                }
            }
            if (((sec->floorstat & 1) == 0) || ((nextsec->floorstat & 1) == 0))
            {
                if ((fz[2] >= fz[0]) && (fz[3] >= fz[1]))
                {
                    if (globparaflorclip)
                        for (x = x1; x <= x2; x++)
                            if (dplc[x] < dmost[x])
                                if (umost[x] <= dmost[x])
                                {
                                    dmost[x] = dplc[x];
                                    if (umost[x] > dmost[x]) numhits--;
                                }
                }
                else
                {
                    // wallmost(uwall,z,nextsectnum,(char)1);
                    if ((fz[2] < cz[0]) || (fz[3] < cz[1]))
                        for (i = x1; i <= x2; i++) if (uwall[i] < uplc[i]) uwall[i] = uplc[i];

                    if ((searchit == 2) && (searchx >= x1) && (searchx <= x2))
                        if (searchy >= uwall[searchx]) //wall
                        {
                            searchsector = sectnum;
                            searchwall = wallnum;
                            if ((wal->cstat & 2) > 0) searchwall = wal->nextwall;
                            searchstat = 0;
                            searchit = 1;
                        }

                    if ((wal->cstat & 2) > 0)
                    {
                        wallnum = wal->nextwall;
                        wal = &wall[wallnum];
                        globalorientation = (long)wal->cstat;
                        globalpicnum = wal->picnum;
                        if ((unsigned)globalpicnum >= (unsigned)MAXTILES) globalpicnum = 0;
                        globalxpanning = (long)wal->xpanning;
                        globalypanning = (long)wal->ypanning;
                        if (picanm[globalpicnum] & 192) globalpicnum += animateoffs(
                            globalpicnum, (short)wallnum + 16384);
                        globalshade = (long)wal->shade;
                        globalpal = (long)wal->pal;
                        wallnum = thewall[z];
                        wal = &wall[wallnum];
                    }
                    else
                    {
                        globalorientation = (long)wal->cstat;
                        globalpicnum = wal->picnum;
                        if ((unsigned)globalpicnum >= (unsigned)MAXTILES) globalpicnum = 0;
                        globalxpanning = (long)wal->xpanning;
                        globalypanning = (long)wal->ypanning;
                        if (picanm[globalpicnum] & 192) globalpicnum += animateoffs(
                            globalpicnum, (short)wallnum + 16384);
                        globalshade = (long)wal->shade;
                        globalpal = (long)wal->pal;
                    }
                    globvis = globalvisibility;
                    if (sec->visibility != 0) globvis = mulscale4(globvis, (long)((unsigned char)(sec->visibility+16)));
                    globalshiftval = (picsiz[globalpicnum] >> 4);
                    if (pow2long[globalshiftval] != tilesizy[globalpicnum]) globalshiftval++;
                    globalshiftval = 32 - globalshiftval;
                    globalyscale = (wal->yrepeat << (globalshiftval - 19));
                    if ((globalorientation & 4) == 0)
                        globalzd = (((globalposz - nextsec->floorz) * globalyscale) << 8);
                    else
                        globalzd = (((globalposz - sec->ceilingz) * globalyscale) << 8);
                    globalzd += (globalypanning << 24);
                    if (globalorientation & 256) globalyscale = -globalyscale, globalzd = -globalzd;

                    if (gotswall == 0)
                    {
                        gotswall = 1;
                        prepwall(z, wal);
                    }
                    wallscan(x1, x2, uwall, dplc, swall, lwall);

                    if ((fz[2] <= fz[0]) && (fz[3] <= fz[1]))
                    {
                        for (x = x1; x <= x2; x++)
                            if (uwall[x] < dmost[x])
                                if (umost[x] <= dmost[x])
                                {
                                    dmost[x] = uwall[x];
                                    if (umost[x] > dmost[x]) numhits--;
                                }
                    }
                    else
                    {
                        for (x = x1; x <= x2; x++)
                            if (umost[x] <= dmost[x])
                            {
                                i = min(dplc[x], uwall[x]);
                                if (i < dmost[x])
                                {
                                    dmost[x] = i;
                                    if (umost[x] > dmost[x]) numhits--;
                                }
                            }
                    }
                }
                if ((fz[2] > fz[0]) || (fz[3] > fz[1]) || (globalposz > fz[4]))
                {
                    i = x2 - x1 + 1;
                    if (smostcnt + i < MAXYSAVES)
                    {
                        smoststart[smostwallcnt] = smostcnt;
                        smostwall[smostwallcnt] = z;
                        smostwalltype[smostwallcnt] = 2; //2 for dmost
                        smostwallcnt++;
                        copybufbyte(&dmost[x1], &smost[smostcnt], i * sizeof(smost[0]));
                        smostcnt += i;
                    }
                }
            }
            if (numhits < 0) return;
            if ((!(wal->cstat & 32)) && ((gotsector[nextsectnum >> 3] & pow2char[nextsectnum & 7]) == 0))
            {
                if (umost[x2] < dmost[x2])
                    scansector(nextsectnum);
                else
                {
                    for (x = x1; x < x2; x++)
                        if (umost[x] < dmost[x])
                        {
                            scansector(nextsectnum);
                            break;
                        }

                    //If can't see sector beyond, then cancel smost array and just
                    //store wall!
                    if (x == x2)
                    {
                        smostwallcnt = startsmostwallcnt;
                        smostcnt = startsmostcnt;
                        smostwall[smostwallcnt] = z;
                        smostwalltype[smostwallcnt] = 0;
                        smostwallcnt++;
                    }
                }
            }
        }
        if ((nextsectnum < 0) || (wal->cstat & 32)) //White/1-way wall
        {
            globalorientation = (long)wal->cstat;
            if (nextsectnum < 0) globalpicnum = wal->picnum;
            else globalpicnum = wal->overpicnum;
            if ((unsigned)globalpicnum >= (unsigned)MAXTILES) globalpicnum = 0;
            globalxpanning = (long)wal->xpanning;
            globalypanning = (long)wal->ypanning;
            if (picanm[globalpicnum] & 192) globalpicnum += animateoffs(globalpicnum, (short)wallnum + 16384);
            globalshade = (long)wal->shade;
            globvis = globalvisibility;
            if (sec->visibility != 0) globvis = mulscale4(globvis, (long)((unsigned char)(sec->visibility+16)));
            globalpal = (long)wal->pal;
            globalshiftval = (picsiz[globalpicnum] >> 4);
            if (pow2long[globalshiftval] != tilesizy[globalpicnum]) globalshiftval++;
            globalshiftval = 32 - globalshiftval;
            globalyscale = (wal->yrepeat << (globalshiftval - 19));
            if (nextsectnum >= 0)
            {
                if ((globalorientation & 4) == 0) globalzd = globalposz - nextsec->ceilingz;
                else globalzd = globalposz - sec->ceilingz;
            }
            else
            {
                if ((globalorientation & 4) == 0) globalzd = globalposz - sec->ceilingz;
                else globalzd = globalposz - sec->floorz;
            }
            globalzd = ((globalzd * globalyscale) << 8) + (globalypanning << 24);
            if (globalorientation & 256) globalyscale = -globalyscale, globalzd = -globalzd;

            if (gotswall == 0)
            {
                gotswall = 1;
                prepwall(z, wal);
            }
            wallscan(x1, x2, uplc, dplc, swall, lwall);

            for (x = x1; x <= x2; x++)
                if (umost[x] <= dmost[x])
                {
                    umost[x] = 1;
                    dmost[x] = 0;
                    numhits--;
                }
            smostwall[smostwallcnt] = z;
            smostwalltype[smostwallcnt] = 0;
            smostwallcnt++;

            if ((searchit == 2) && (searchx >= x1) && (searchx <= x2))
            {
                searchit = 1;
                searchsector = sectnum;
                searchwall = wallnum;
                if (nextsectnum < 0) searchstat = 0;
                else searchstat = 4;
            }
        }
    }
}

void prepwall(long z, walltype* wal)
{
    long i, l, ol, splc, sinc, x, topinc, top, botinc, bot, hplc, walxrepeat;

    walxrepeat = (wal->xrepeat << 3);

    //lwall calculation
    i = xb1[z] - halfxdimen;
    topinc = -(ry1[z] >> 2);
    botinc = ((ry2[z] - ry1[z]) >> 8);
    top = mulscale5(rx1[z], xdimen) + mulscale2(topinc, i);
    bot = mulscale11(rx1[z]-rx2[z], xdimen) + mulscale2(botinc, i);

    splc = mulscale19(ry1[z], xdimscale);
    sinc = mulscale16(ry2[z]-ry1[z], xdimscale);

    x = xb1[z];
    if (bot != 0)
    {
        l = divscale12(top, bot);
        swall[x] = mulscale21(l, sinc) + splc;
        l *= walxrepeat;
        lwall[x] = (l >> 18);
    }
    while (x + 4 <= xb2[z])
    {
        top += topinc;
        bot += botinc;
        if (bot != 0)
        {
            ol = l;
            l = divscale12(top, bot);
            swall[x + 4] = mulscale21(l, sinc) + splc;
            l *= walxrepeat;
            lwall[x + 4] = (l >> 18);
        }
        i = ((ol + l) >> 1);
        lwall[x + 2] = (i >> 18);
        lwall[x + 1] = ((ol + i) >> 19);
        lwall[x + 3] = ((l + i) >> 19);
        swall[x + 2] = ((swall[x] + swall[x + 4]) >> 1);
        swall[x + 1] = ((swall[x] + swall[x + 2]) >> 1);
        swall[x + 3] = ((swall[x + 4] + swall[x + 2]) >> 1);
        x += 4;
    }
    if (x + 2 <= xb2[z])
    {
        top += (topinc >> 1);
        bot += (botinc >> 1);
        if (bot != 0)
        {
            ol = l;
            l = divscale12(top, bot);
            swall[x + 2] = mulscale21(l, sinc) + splc;
            l *= walxrepeat;
            lwall[x + 2] = (l >> 18);
        }
        lwall[x + 1] = ((l + ol) >> 19);
        swall[x + 1] = ((swall[x] + swall[x + 2]) >> 1);
        x += 2;
    }
    if (x + 1 <= xb2[z])
    {
        bot += (botinc >> 2);
        if (bot != 0)
        {
            l = divscale12(top + (topinc >> 2), bot);
            swall[x + 1] = mulscale21(l, sinc) + splc;
            lwall[x + 1] = mulscale18(l, walxrepeat);
        }
    }

    if (lwall[xb1[z]] < 0) lwall[xb1[z]] = 0;
    if ((lwall[xb2[z]] >= walxrepeat) && (walxrepeat)) lwall[xb2[z]] = walxrepeat - 1;
    if (wal->cstat & 8)
    {
        walxrepeat--;
        for (x = xb1[z]; x <= xb2[z]; x++) lwall[x] = walxrepeat - lwall[x];
    }
}

void ceilscan(long x1, long x2, long sectnum){}
/*{
    long i, j, ox, oy, x, y1, y2, twall, bwall;
    sectortype* sec;

    sec = &sector[sectnum];
    if (palookup[sec->ceilingpal] != globalpalwritten)
    {
        globalpalwritten = palookup[sec->ceilingpal];
        setpalookupaddress(globalpalwritten);
    }

    globalzd = sec->ceilingz - globalposz;
    if (globalzd > 0) return;
    globalpicnum = sec->ceilingpicnum;
    if ((unsigned)globalpicnum >= (unsigned)MAXTILES) globalpicnum = 0;
    setgotpic(globalpicnum);
    if ((tilesizx[globalpicnum] <= 0) || (tilesizy[globalpicnum] <= 0)) return;
    if (picanm[globalpicnum] & 192) globalpicnum += animateoffs((short)globalpicnum, (short)sectnum);

    if (waloff[globalpicnum] == 0) loadtile(globalpicnum);
    globalbufplc = waloff[globalpicnum];

    globalshade = (long)sec->ceilingshade;
    globvis = globalcisibility;
    if (sec->visibility != 0) globvis = mulscale4(globvis, (long)((unsigned char)(sec->visibility+16)));
    globalorientation = (long)sec->ceilingstat;


    if ((globalorientation & 64) == 0)
    {
        globalx1 = singlobalang;
        globalx2 = singlobalang;
        globaly1 = cosglobalang;
        globaly2 = cosglobalang;
        globalxpanning = (globalposx << 20);
        globalypanning = -(globalposy << 20);
    }
    else
    {
        j = sec->wallptr;
        ox = wall[wall[j].point2].x - wall[j].x;
        oy = wall[wall[j].point2].y - wall[j].y;
        i = nsqrtasm(ox * ox + oy * oy);
        if (i == 0) i = 1024;
        else i = 1048576 / i;
        globalx1 = mulscale10(dmulscale10(ox,singlobalang,-oy,cosglobalang), i);
        globaly1 = mulscale10(dmulscale10(ox,cosglobalang,oy,singlobalang), i);
        globalx2 = -globalx1;
        globaly2 = -globaly1;

        ox = ((wall[j].x - globalposx) << 6);
        oy = ((wall[j].y - globalposy) << 6);
        i = dmulscale14(oy, cosglobalang, -ox, singlobalang);
        j = dmulscale14(ox, cosglobalang, oy, singlobalang);
        ox = i;
        oy = j;
        globalxpanning = globalx1 * ox - globaly1 * oy;
        globalypanning = globaly2 * ox + globalx2 * oy;
    }
    globalx2 = mulscale16(globalx2, viewingrangerecip);
    globaly1 = mulscale16(globaly1, viewingrangerecip);
    globalxshift = (8 - (picsiz[globalpicnum] & 15));
    globalyshift = (8 - (picsiz[globalpicnum] >> 4));
    if (globalorientation & 8)
    {
        globalxshift++;
        globalyshift++;
    }

    if ((globalorientation & 0x4) > 0)
    {
        i = globalxpanning;
        globalxpanning = globalypanning;
        globalypanning = i;
        i = globalx2;
        globalx2 = -globaly1;
        globaly1 = -i;
        i = globalx1;
        globalx1 = globaly2;
        globaly2 = i;
    }
    if ((globalorientation & 0x10) > 0) globalx1 = -globalx1, globaly1 = -globaly1, globalxpanning = -globalxpanning;
    if ((globalorientation & 0x20) > 0) globalx2 = -globalx2, globaly2 = -globaly2, globalypanning = -globalypanning;
    globalx1 <<= globalxshift;
    globaly1 <<= globalxshift;
    globalx2 <<= globalyshift;
    globaly2 <<= globalyshift;
    globalxpanning <<= globalxshift;
    globalypanning <<= globalyshift;
    globalxpanning += (((long)sec->ceilingxpanning) << 24);
    globalypanning += (((long)sec->ceilingypanning) << 24);
    globaly1 = (-globalx1 - globaly1) * halfxdimen;
    globalx2 = (globalx2 - globaly2) * halfxdimen;

    sethlinesizes(picsiz[globalpicnum] & 15, picsiz[globalpicnum] >> 4, globalbufplc);

    globalx2 += globaly2 * (x1 - 1);
    globaly1 += globalx1 * (x1 - 1);
    globalx1 = mulscale16(globalx1, globalzd);
    globalx2 = mulscale16(globalx2, globalzd);
    globaly1 = mulscale16(globaly1, globalzd);
    globaly2 = mulscale16(globaly2, globalzd);
    globvis = klabs(mulscale10(globvis, globalzd));

    if (!(globalorientation & 0x180))
    {
        y1 = umost[x1];
        y2 = y1;
        for (x = x1; x <= x2; x++)
        {
            twall = umost[x] - 1;
            bwall = min(uplc[x], dmost[x]);
            if (twall < bwall - 1)
            {
                if (twall >= y2)
                {
                    while (y1 < y2 - 1) hline(x - 1, ++y1);
                    y1 = twall;
                }
                else
                {
                    while (y1 < twall) hline(x - 1, ++y1);
                    while (y1 > twall) lastx[y1--] = x;
                }
                while (y2 > bwall) hline(x - 1, --y2);
                while (y2 < bwall) lastx[y2++] = x;
            }
            else
            {
                while (y1 < y2 - 1) hline(x - 1, ++y1);
                if (x == x2)
                {
                    globalx2 += globaly2;
                    globaly1 += globalx1;
                    break;
                }
                y1 = umost[x + 1];
                y2 = y1;
            }
            globalx2 += globaly2;
            globaly1 += globalx1;
        }
        while (y1 < y2 - 1) hline(x2, ++y1);
        faketimerhandler();
        return;
    }

   // switch (globalorientation & 0x180)
   // {
   // case 128:
   //     msethlineshift(picsiz[globalpicnum] & 15, picsiz[globalpicnum] >> 4);
   //     break;
   // case 256:
   //     settransnormal();
   //     tsethlineshift(picsiz[globalpicnum] & 15, picsiz[globalpicnum] >> 4);
   //     break;
   // case 384:
   //     settransreverse();
   //     tsethlineshift(picsiz[globalpicnum] & 15, picsiz[globalpicnum] >> 4);
   //     break;
   // }

    y1 = umost[x1];
    y2 = y1;
    for (x = x1; x <= x2; x++)
    {
        twall = umost[x] - 1;
        bwall = min(uplc[x], dmost[x]);
        if (twall < bwall - 1)
        {
            if (twall >= y2)
            {
                while (y1 < y2 - 1) slowhline(x - 1, ++y1);
                y1 = twall;
            }
            else
            {
                while (y1 < twall) slowhline(x - 1, ++y1);
                while (y1 > twall) lastx[y1--] = x;
            }
            while (y2 > bwall) slowhline(x - 1, --y2);
            while (y2 < bwall) lastx[y2++] = x;
        }
        else
        {
            while (y1 < y2 - 1) slowhline(x - 1, ++y1);
            if (x == x2)
            {
                globalx2 += globaly2;
                globaly1 += globalx1;
                break;
            }
            y1 = umost[x + 1];
            y2 = y1;
        }
        globalx2 += globaly2;
        globaly1 += globalx1;
    }
    while (y1 < y2 - 1) slowhline(x2, ++y1);
    faketimerhandler();
}*/

void florscan(long x1, long x2, long sectnum)
{}//flor render

void wallscan(long x1, long x2, short* uwal, short* dwal, long* swal, long* lwal)
{} // wll render

void maskwallscan(long x1, long x2, short* uwal, short* dwal, long* swal, long* lwal)
{}
void transmaskvline(long x)
{}

void transmaskvline2(long x){}

void transmaskwallscan(long x1, long x2)
{}

int loadboard(char* filename, long* daposx, long* daposy, long* daposz, short* daang, short* dacursectnum)
{
    short fil, i, numsprites;

    i = strlen(filename) - 1;
    if (filename[i] == 255)
    {
        filename[i] = 0;
        i = 1;
    }
    else i = 0;
    if ((fil = kopen4load(filename, i)) == -1)
    {
        mapversion = 7L;
        return (-1);
    }

    kread(fil, &mapversion, 4);
    if (mapversion != 7L) return (-1);

    initspritelists();

    clearbuf((&show2dsector[0]), (long)((MAXSECTORS + 3) >> 5), 0L);
    clearbuf((&show2dsprite[0]), (long)((MAXSPRITES + 3) >> 5), 0L);
    clearbuf((&show2dwall[0]), (long)((MAXWALLS + 3) >> 5), 0L);

    kread(fil, daposx, 4);
    kread(fil, daposy, 4);
    kread(fil, daposz, 4);
    kread(fil, daang, 2);
    kread(fil, dacursectnum, 2);

    kread(fil, &numsectors, 2);  // reading actual map data here, numsectors is num in the map;
    kread(fil, &sector[0], sizeof(sectortype) * numsectors);

    kread(fil, &numwalls, 2);
    kread(fil, &wall[0], sizeof(walltype) * numwalls);

    kread(fil, &numsprites, 2);
    kread(fil, &sprite[0], sizeof(spritetype) * numsprites);

    for (i = 0; i < numsprites; i++)
        insertsprite(sprite[i].sectnum, sprite[i].statnum);

    //Must be after loading sectors, etc!
    bbeng.FindSectorOfPoint(*daposx, *daposy, dacursectnum);

    kclose(fil);
    return (0);
}

int saveboard(char* filename, long* daposx, long* daposy, long* daposz, short* daang, short* dacursectnum)
{
   /* short fil, i, j, numsprites;

    if ((fil = open(filename,O_BINARY | O_TRUNC | O_CREAT | O_WRONLY,S_IWRITE)) == -1)
        return (-1);
    write(fil, &mapversion, 4);

    write(fil, daposx, 4);
    write(fil, daposy, 4);
    write(fil, daposz, 4);
    write(fil, daang, 2);
    write(fil, dacursectnum, 2);

    write(fil, &numsectors, 2);
    write(fil, &sector[0], sizeof(sectortype) * numsectors);

    write(fil, &numwalls, 2);
    write(fil, &wall[0], sizeof(walltype) * numwalls);

    numsprites = 0;
    for (j = 0; j < MAXSTATUS; j++)
    {
        i = headspritestat[j];
        while (i != -1)
        {
            numsprites++;
            i = nextspritestat[i];
        }
    }
    write(fil, &numsprites, 2);

    for (j = 0; j < MAXSTATUS; j++)
    {
        i = headspritestat[j];
        while (i != -1)
        {
            write(fil, &sprite[i], sizeof(spritetype));
            i = nextspritestat[i];
        }
    }

    close(fil);*/
    return (0);
}

void loadtables()
{
    long i, fil;

    if (tablesloaded == 0)
    {
        initksqrt();

        for (i = 0; i < 2048; i++) reciptable[i] = divscale30(2048L, i + 2048);

        if ((fil = kopen4load("tables.dat", 0)) != -1)
        {
            kread(fil, sintable, 2048 * 2);
            kread(fil, radarang, 640 * 2);
            for (i = 0; i < 640; i++) radarang[1279 - i] = -radarang[i];
            kread(fil, textfont, 1024);
            kread(fil, smalltextfont, 1024);
            kread(fil, britable, 1024);
            kclose(fil);
        }
        tablesloaded = 1;
    }
}

void loadpalette()
{
  //  long i, j, k, dist, fil;
  //  char* ptr;
//
  //  if (paletteloaded != 0) return;
  //  if ((fil = kopen4load("palette.dat", 0)) == -1) return;
//
  //  kread(fil, palette, 768);
  //  kread(fil, &numpalookups, 2);
//
  //  if ((palookup[0] = (char*)kkmalloc(numpalookups << 8)) == NULL)
  //      allocache(&palookup[0], numpalookups << 8, &permanentlock);
  //  if ((transluc = (char*)kkmalloc(65536L)) == NULL)
  //      allocache(&transluc, 65536, &permanentlock);
//
  //  globalpalwritten = palookup[0];
  //  globalpal = 0;
  //  setpalookupaddress(globalpalwritten);
//
  //  fixtransluscence(FP_OFF(transluc));
//
  //  kread(fil, palookup[globalpal], numpalookups << 8);
  //  kread(fil, transluc, 65536);
  //  kclose(fil);
//
  //  initfastcolorlookup(30L, 59L, 11L);
//
  //  paletteloaded = 1;
//
  //  if (vidoption == 6)
  //  {
  //      for (k = 0; k < MAXPALOOKUPS; k++)
  //          if (palookup[k] != NULL)
  //              for (i = 0; i < 256; i++)
  //              {
  //                  dist = palette[i * 3] * 3 + palette[i * 3 + 1] * 5 + palette[i * 3 + 2] * 2;
  //                  ptr = (char*)(FP_OFF(palookup[k]) + i);
  //                  for (j = 0; j < 32; j++)
  //                      ptr[j << 8] = (char)min(max(mulscale10(dist,32-j),0), 15);
  //              }
//
  //      if (transluc != NULL)
  //      {
  //          for (i = 0; i < 16; i++)
  //              for (j = 0; j < 16; j++)
  //                  transluc[(i << 8) + j] = ((i + j + 1) >> 1);
  //      }
  //  }
}

int setgamemode(char davidoption, long daxdim, long daydim)
{
    /*
    long i, j, k, ostereomode;

    if ((qsetmode == 200) && (vidoption == davidoption) && (xdim == daxdim) && (ydim == daydim))
        return(0);
    vidoption = davidoption; xdim = daxdim; ydim = daydim;

    strcpy(kensmessage,"!!!! BUILD engine&tools programmed by Ken Silverman of E.G. RI.  (c) Copyright 1995 Ken Silverman.  Summary:  BUILD = Ken. !!!!");
    if (getkensmessagecrc(FP_OFF(kensmessage)) != 0x56c764d4)
    { setvmode(0x3); printf("Nice try.\n"); exit(0); }

    ostereomode = stereomode; if (stereomode) setstereo(0L);

    activepage = visualpage = 0;
    switch(vidoption)
    {
    case 1: i = xdim*ydim; break;
    case 2: xdim = 320; ydim = 200; i = xdim*ydim; break;
    case 6: xdim = 320; ydim = 200; i = 131072; break;
    default: return(-1);
    }
    j = ydim*4*sizeof(long);  //Leave room for horizlookup&horizlookup2

    if (screen != NULL)
    {
        if (screenalloctype == 0) kkfree((void *)screen);
        if (screenalloctype == 1) suckcache((long *)screen);
        screen = NULL;
    }
    screenalloctype = 0;
    if ((screen = (char *)kkmalloc(i+(j<<1))) == NULL)
    {
        allocache((long *)&screen,i+(j<<1),&permanentlock);
        screenalloctype = 1;
    }

    frameplace = FP_OFF(screen);
    horizlookup = (long *)(frameplace+i);
    horizlookup2 = (long *)(frameplace+i+j);
    horizycent = ((ydim*4)>>1);

    switch(vidoption)
    {
    case 1:
        //bytesperline is set in this function
        if (setvesa(xdim,ydim) < 0) return(-1);
        break;
    case 2:
        horizycent = ((ydim*4)>>1);  //HACK for switching to this mode
    case 6:
        bytesperline = xdim;
        setvmode(0x13);
        break;
    default: return(-1);
    }

    //Force drawrooms to call dosetaspect & recalculate stuff
    oxyaspect = oxdimen = oviewingrange = -1;

    setvlinebpl(bytesperline);
    j = 0;
    for(i=0;i<=ydim;i++) ylookup[i] = j, j += bytesperline;

    numpages = 1;
    if (vidoption == 1) numpages = min(maxpages,8);

    setview(0L,0L,xdim-1,ydim-1);
    clearallviews(0L);
    setbrightness((char)curbrightness,(char *)&palette[0]);

    if (searchx < 0) { searchx = halfxdimen; searchy = (ydimen>>1); }

    if (ostereomode) setstereo(ostereomode);

    qsetmode = 200;

    */
    return (0);
}

void hline(long xr, long yp)
{
  // long xl, r, s;

  // xl = lastx[yp];
  // if (xl > xr) return;
  // r = horizlookup2[yp - globalhoriz + horizycent];
  // asm1 = globalx1 * r;
  // asm2 = globaly2 * r;
  // s = ((long)getpalookup((long)mulscale16(r, globvis), globalshade) << 8);

  // hlineasm4(xr - xl, 0L, s, globalx2 * r + globalypanning, globaly1 * r + globalxpanning,
  //           ylookup[yp] + xr + frameoffset);
}

void slowhline(long xr, long yp)
{
  // long xl, x, y, ox, oy, r, p, shade;

  // xl = lastx[yp];
  // if (xl > xr) return;
  // r = horizlookup2[yp - globalhoriz + horizycent];
  // asm1 = globalx1 * r;
  // asm2 = globaly2 * r;

  // asm3 = (long)globalpalwritten + ((long)getpalookup((long)mulscale16(r, globvis), globalshade) << 8);
  // if (!(globalorientation & 256))
  // {
  //     mhline(globalbufplc, globaly1 * r + globalxpanning - asm1 * (xr - xl), (xr - xl) << 16, 0L,
  //            globalx2 * r + globalypanning - asm2 * (xr - xl), ylookup[yp] + xl + frameoffset);
  //     return;
  // }
  // thline(globalbufplc, globaly1 * r + globalxpanning - asm1 * (xr - xl), (xr - xl) << 16, 0L,
  //        globalx2 * r + globalypanning - asm2 * (xr - xl), ylookup[yp] + xl + frameoffset);
  // transarea += (xr - xl);
}

void initengine()
{
    long i, j;

   // if (dommxoverlay) mmxoverlay();

    loadtables();

    xyaspect = -1;

    pskyoff[0] = 0;
    pskybits = 0;

    parallaxtype = 2;
    parallaxyoffs = 0L;
    parallaxyscale = 65536;
    showinvisibility = 0;

#ifdef SUPERBUILD
    for (i = 1; i < 1024; i++) lowrecip[i] = ((1 << 24) - 1) / i;
    for (i = 0; i < MAXVOXELS; i++)
        for (j = 0; j < MAXVOXMIPS; j++)
        {
            voxoff[i][j] = 0L;
            voxlock[i][j] = 200;
        }
#endif

    paletteloaded = 0;

    searchit = 0;
    searchstat = -1;

    for (i = 0; i < MAXPALOOKUPS; i++) palookup[i] = NULL;

    clearbuf((&waloff[0]), (long)MAXTILES, 0L);

    clearbuf((&show2dsector[0]), (long)((MAXSECTORS + 3) >> 5), 0L);
    clearbuf((&show2dsprite[0]), (long)((MAXSPRITES + 3) >> 5), 0L);
    clearbuf((&show2dwall[0]), (long)((MAXWALLS + 3) >> 5), 0L);
    automapping = 0;

    //validmodecnt = 0;

    pointhighlight = -1;
    linehighlight = -1;
    highlightcnt = 0;

    totalclock = 0;
    visibility = 512;
    parallaxvisibility = 512;

    loadpalette();
}

void uninitengine()
{
    long i;

    //   if (vidoption == 1) uninitvesa();
    if (artfil != -1) kclose(artfil);

    //  if (stereomode) setstereo(0L);

    if (transluc != NULL)
    {
        kkfree(transluc);
        transluc = NULL;
    }
    if (pic != NULL)
    {
        kkfree(pic);
        pic = NULL;
    }
    //if (screen != NULL)
    //{
    //    if (screenalloctype == 0) kkfree((void *)screen);
    //    //if (screenalloctype == 1) suckcache(screen);  //Cache already gone
    //    screen = NULL;
    //}
    for (i = 0; i < MAXPALOOKUPS; i++)
        if (palookup[i] != NULL)
        {
            kkfree(palookup[i]);
            palookup[i] = NULL;
        }
}

void nextpage()
{
    // Drawing code.
    /*
        long totbytes, i, j, k;
        permfifotype *per;

        //char snotbuf[32];
        //j = 0; k = 0;
        //for(i=0;i<4096;i++)
        //   if (waloff[i] != 0)
        //   {
        //      sprintf(snotbuf,"%ld-%ld",i,tilesizx[i]*tilesizy[i]);
        //      printext256((j>>5)*40+32,(j&31)*6,walock[i]>>3,-1,snotbuf,1);
        //      k += tilesizx[i]*tilesizy[i];
        //      j++;
        //   }
        //sprintf(snotbuf,"Total: %ld",k);
        //printext256((j>>5)*40+32,(j&31)*6,31,-1,snotbuf,1);

        switch(qsetmode)
        {
        case 200:
            for(i=permtail;i!=permhead;i=((i+1)&(MAXPERMS-1)))
            {
                per = &permfifo[i];
                if ((per->pagesleft > 0) && (per->pagesleft <= numpages))
                    dorotatesprite(per->sx,per->sy,per->z,per->a,per->picnum,
                                   per->dashade,per->dapalnum,per->dastat,
                                   per->cx1,per->cy1,per->cx2,per->cy2);
            }

            switch(vidoption)
            {
            case 1:
                if (stereomode)
                {
                    stereonextpage();
                    if (!origbuffermode) buffermode = transarea = totalarea = 0;
                }
                else
                {
                    visualpage = activepage;
                    setvisualpage(visualpage);
                    if (!origbuffermode)
                    {
                        buffermode = ((transarea<<3) > totalarea);
                        transarea = totalarea = 0;
                    }
                    activepage++; if (activepage >= numpages) activepage = 0;
                    setactivepage(activepage);
                }
                break;
            case 2:
                copybuf(frameplace,0xa0000,64000>>2);
                break;
            case 6:
                if (!activepage) redblueblit(screen,&screen[65536],64000L);
                activepage ^= 1;
                break;
            }


            for(i=permtail;i!=permhead;i=((i+1)&(MAXPERMS-1)))
            {
                per = &permfifo[i];
                if (per->pagesleft >= 130)
                    dorotatesprite(per->sx,per->sy,per->z,per->a,per->picnum,
                                   per->dashade,per->dapalnum,per->dastat,
                                   per->cx1,per->cy1,per->cx2,per->cy2);

                if (per->pagesleft&127) per->pagesleft--;
                if (((per->pagesleft&127) == 0) && (i == permtail))
                    permtail = ((permtail+1)&(MAXPERMS-1));
            }
            break;

        case 350:
            koutpw(0x3d4,0xc+((pageoffset>>11)<<8));
            limitrate();
            pageoffset = 225280-pageoffset; //225280 is 352(multiple of 16)*640
            break;

        case 480:
            koutpw(0x3d4,0xc+((pageoffset>>11)<<8));
            limitrate();
            pageoffset = 399360-pageoffset;
            break;
        }
        faketimerhandler();

        if ((totalclock >= lastageclock+8) || (totalclock < lastageclock))
        { lastageclock = totalclock; agecache(); }

        beforedrawrooms = 1;
        numframes++;
        */
}

void loadtile(short tilenume)
{
  // char* ptr;
  // long i, dasiz;

  // if ((unsigned)tilenume >= (unsigned)MAXTILES) return;
  // dasiz = tilesizx[tilenume] * tilesizy[tilenume];
  // if (dasiz <= 0) return;

  // i = tilefilenum[tilenume];
  // if (i != artfilnum)
  // {
  //     if (artfil != -1) kclose(artfil);
  //     artfilnum = i;
  //     artfilplc = 0L;

  //     artfilename[7] = (i % 10) + 48;
  //     artfilename[6] = ((i / 10) % 10) + 48;
  //     artfilename[5] = ((i / 100) % 10) + 48;
  //     artfil = kopen4load(artfilename, 0);
  //     faketimerhandler();
  // }

  // if (cachedebug) printf("Tile:%ld\n", tilenume);

  // if (waloff[tilenume] == 0)
  // {
  //     walock[tilenume] = 199;
  //     allocache(&waloff[tilenume], dasiz, &walock[tilenume]);
  // }

  // if (artfilplc != tilefileoffs[tilenume])
  // {
  //     klseek(artfil, tilefileoffs[tilenume] - artfilplc,SEEK_CUR);
  //     faketimerhandler();
  // }
  // ptr = (char*)waloff[tilenume];
  // kread(artfil, ptr, dasiz);
  // faketimerhandler();
  // artfilplc = tilefileoffs[tilenume] + dasiz;
}

long allocatepermanenttile(short tilenume, long xsiz, long ysiz)
{
    long i, j, x, y, dasiz;

    if ((xsiz <= 0) || (ysiz <= 0) || ((unsigned)tilenume >= (unsigned)MAXTILES))
        return (0);

    dasiz = xsiz * ysiz;

    walock[tilenume] = 255;
    allocache(&waloff[tilenume], dasiz, &walock[tilenume]);

    tilesizx[tilenume] = xsiz;
    tilesizy[tilenume] = ysiz;
    picanm[tilenume] = 0;

    j = 15;
    while ((j > 1) && (pow2long[j] > xsiz)) j--;
    picsiz[tilenume] = ((char)j);
    j = 15;
    while ((j > 1) && (pow2long[j] > ysiz)) j--;
    picsiz[tilenume] += ((char)(j << 4));

    return (waloff[tilenume]);
}

int loadpics(char* filename){return 0;}
/*
{
    long offscount, siz, localtilestart, localtileend, dasiz;
    short fil, i, j, k;

    strcpy(artfilename, filename);

    for (i = 0; i < MAXTILES; i++)
    {
        tilesizx[i] = 0;
        tilesizy[i] = 0;
        picanm[i] = 0L;
    }

    artsize = 0L;

    numtilefiles = 0;
    do
    {
        k = numtilefiles;

        artfilename[7] = (k % 10) + 48;
        artfilename[6] = ((k / 10) % 10) + 48;
        artfilename[5] = ((k / 100) % 10) + 48;
        if ((fil = kopen4load(artfilename, 0)) != -1)
        {
            kread(fil, &artversion, 4);
            if (artversion != 1) return (-1);
            kread(fil, &numtiles, 4);
            kread(fil, &localtilestart, 4);
            kread(fil, &localtileend, 4);
            kread(fil, &tilesizx[localtilestart], (localtileend - localtilestart + 1) << 1);
            kread(fil, &tilesizy[localtilestart], (localtileend - localtilestart + 1) << 1);
            kread(fil, &picanm[localtilestart], (localtileend - localtilestart + 1) << 2);

            offscount = 4 + 4 + 4 + 4 + ((localtileend - localtilestart + 1) << 3);
            for (i = localtilestart; i <= localtileend; i++)
            {
                tilefilenum[i] = k;
                tilefileoffs[i] = offscount;
                dasiz = (long)(tilesizx[i] * tilesizy[i]);
                offscount += dasiz;
                artsize += ((dasiz + 15) & 0xfffffff0);
            }
            kclose(fil);

            numtilefiles++;
        }
    }
    while (k != numtilefiles);

    clearbuf((&gotpic[0]), (long)((MAXTILES + 31) >> 5), 0L);

    //try dpmi_DETERMINEMAXREALALLOC!

    cachesize = max(artsize, 1048576);
    while ((pic = (char*)kkmalloc(cachesize)) == NULL)
    {
        cachesize -= 65536L;
        if (cachesize < 65536) return (-1);
    }
    initcache((FP_OFF(pic) + 15) & 0xfffffff0, (cachesize - ((-FP_OFF(pic)) & 15)) & 0xfffffff0);

    for (i = 0; i < MAXTILES; i++)
    {
        j = 15;
        while ((j > 1) && (pow2long[j] > tilesizx[i])) j--;
        picsiz[i] = ((char)j);
        j = 15;
        while ((j > 1) && (pow2long[j] > tilesizy[i])) j--;
        picsiz[i] += ((char)(j << 4));
    }

    artfil = -1;
    artfilnum = -1;
    artfilplc = 0L;

    return (0);
}*/

void qloadkvx(long voxindex, char* filename)
{
    long i, fil, dasiz, lengcnt, lengtot;
    char* ptr;

    if ((fil = kopen4load(filename, 0)) == -1) return;

    lengcnt = 0;
    lengtot = kfilelength(fil);

    for (i = 0; i < MAXVOXMIPS; i++)
    {
        kread(fil, &dasiz, 4);
        //Must store filenames to use cacheing system :(
        voxlock[voxindex][i] = 200;
        allocache(&voxoff[voxindex][i], dasiz, &voxlock[voxindex][i]);
        ptr = (char*)voxoff[voxindex][i];
        kread(fil, ptr, dasiz);

        lengcnt += dasiz + 4;
        if (lengcnt >= lengtot - 768) break;
    }
    kclose(fil);
}

int clipinsidebox(long x, long y, short wallnum, long walldist)
{
    walltype* wal;
    long x1, y1, x2, y2, r;

    r = (walldist << 1);
    wal = &wall[wallnum];
    x1 = wal->x + walldist - x;
    y1 = wal->y + walldist - y;
    wal = &wall[wal->point2];
    x2 = wal->x + walldist - x;
    y2 = wal->y + walldist - y;

    if ((x1 < 0) && (x2 < 0)) return (0);
    if ((y1 < 0) && (y2 < 0)) return (0);
    if ((x1 >= r) && (x2 >= r)) return (0);
    if ((y1 >= r) && (y2 >= r)) return (0);

    x2 -= x1;
    y2 -= y1;
    if (x2 * (walldist - y1) >= y2 * (walldist - x1)) //Front
    {
        if (x2 > 0) x2 *= (0 - y1);
        else x2 *= (r - y1);
        if (y2 > 0) y2 *= (r - x1);
        else y2 *= (0 - x1);
        return (x2 < y2);
    }
    if (x2 > 0) x2 *= (r - y1);
    else x2 *= (0 - y1);
    if (y2 > 0) y2 *= (0 - x1);
    else y2 *= (r - x1);
    return ((x2 >= y2) << 1);
}

int clipinsideboxline(long x, long y, long x1, long y1, long x2, long y2, long walldist)
{
    long r;

    r = (walldist << 1);

    x1 += walldist - x;
    x2 += walldist - x;
    if ((x1 < 0) && (x2 < 0)) return (0);
    if ((x1 >= r) && (x2 >= r)) return (0);

    y1 += walldist - y;
    y2 += walldist - y;
    if ((y1 < 0) && (y2 < 0)) return (0);
    if ((y1 >= r) && (y2 >= r)) return (0);

    x2 -= x1;
    y2 -= y1;
    if (x2 * (walldist - y1) >= y2 * (walldist - x1)) //Front
    {
        if (x2 > 0) x2 *= (0 - y1);
        else x2 *= (r - y1);
        if (y2 > 0) y2 *= (r - x1);
        else y2 *= (0 - x1);
        return (x2 < y2);
    }
    if (x2 > 0) x2 *= (r - y1);
    else x2 *= (0 - y1);
    if (y2 > 0) y2 *= (0 - x1);
    else y2 *= (r - x1);
    return ((x2 >= y2) << 1);
}

long readpixel16(long p)
{
    return 0;
    // long mask, dat;

    // mask = pow2long[p&7^7];

    // if ((qsetmode == 480) && (ydim16 <= 336) && (p >= 640*336))
    //     p -= 640*336;
    // else
    //     p += pageoffset;

    // p >>= 3;

    // koutp(0x3ce,0x4);
    // koutp(0x3cf,0); dat = ((readpixel(p+0xa0000)&mask)>0);
    // koutp(0x3cf,1); dat += (((readpixel(p+0xa0000)&mask)>0)<<1);
    // koutp(0x3cf,2); dat += (((readpixel(p+0xa0000)&mask)>0)<<2);
    // koutp(0x3cf,3); dat += (((readpixel(p+0xa0000)&mask)>0)<<3);
    // return(dat);
}

int screencapture(char* filename, char inverseit) { return 0; };
/*
{
    char *ptr;
    long fil, i, bufplc, p, col, ncol, leng, numbytes, xres;

    filename[4] = ((capturecount/1000)%10)+48;
    filename[5] = ((capturecount/100)%10)+48;
    filename[6] = ((capturecount/10)%10)+48;
    filename[7] = (capturecount%10)+48;
    if ((fil=open(filename,O_BINARY|O_CREAT|O_TRUNC|O_WRONLY,S_IWRITE))==-1)
        return(-1);

    if (qsetmode == 200)
    {
        pcxheader[8] = ((xdim-1)&255); pcxheader[9] = (((xdim-1)>>8)&255);
        pcxheader[10] = ((ydim-1)&255); pcxheader[11] = (((ydim-1)>>8)&255);
        pcxheader[12] = (xdim&255); pcxheader[13] = ((xdim>>8)&255);
        pcxheader[14] = (ydim&255); pcxheader[15] = ((ydim>>8)&255);
        pcxheader[66] = (xdim&255); pcxheader[67] = ((xdim>>8)&255);
    }
    else
    {
        pcxheader[8] = ((640-1)&255); pcxheader[9] = (((640-1)>>8)&255);
        pcxheader[10] = ((qsetmode-1)&255); pcxheader[11] = (((qsetmode-1)>>8)&255);
        pcxheader[12] = (640&255); pcxheader[13] = ((640>>8)&255);
        pcxheader[14] = (qsetmode&255); pcxheader[15] = ((qsetmode>>8)&255);
        pcxheader[66] = (640&255); pcxheader[67] = ((640>>8)&255);
    }

    write(fil,&pcxheader[0],128);

    if (qsetmode == 200)
    {
        ptr = (char *)frameplace;
        numbytes = xdim*ydim;
        xres = xdim;
    }
    else
    {
        numbytes = (mul5(qsetmode)<<7);
        xres = 640;
    }

    bufplc = 0; p = 0;
    while (p < numbytes)
    {
        koutp(97,kinp(97)|3);

        if (qsetmode == 200) { col = *ptr; p++; ptr++; }
        else
        {
            col = readpixel16(p);
            p++;
            if ((inverseit == 1) && (((col&7) == 0) || ((col&7) == 7))) col ^= 15;
        }

        leng = 1;

        if (qsetmode == 200) ncol = *ptr;
        else
        {
            ncol = readpixel16(p);
            if ((inverseit == 1) && (((ncol&7) == 0) || ((ncol&7) == 7))) ncol ^= 15;
        }

        while ((ncol == col) && (p < numbytes) && (leng < 63) && ((p%xres) != 0))
        {
            leng++;

            if (qsetmode == 200) { p++; ptr++; ncol = *ptr; }
            else
            {
                p++;
                ncol = readpixel16(p);
                if ((inverseit == 1) && (((ncol&7) == 0) || ((ncol&7) == 7))) ncol ^= 15;
            }
        }

        koutp(97,kinp(97)&252);

        if ((leng > 1) || (col >= 0xc0))
        {
            tempbuf[bufplc++] = (leng|0xc0);
            if (bufplc == 4096) { bufplc = 0; if (write(fil,&tempbuf[0],4096) < 4096) { close(fil); return(-1); } }
        }
        tempbuf[bufplc++] = col;
        if (bufplc == 4096) { bufplc = 0; if (write(fil,&tempbuf[0],4096) < 4096) { close(fil); return(-1); } }
    }

    tempbuf[bufplc++] = 0xc;
    if (bufplc == 4096) { bufplc = 0; if (write(fil,&tempbuf[0],4096) < 4096) { close(fil); return(-1); } }

    if (qsetmode == 200)
    {
        VBE_getPalette(0,256,&tempbuf[4096]);
        for(i=0;i<256;i++)
        {
            tempbuf[bufplc++] = (tempbuf[(i<<2)+4096+2]<<2);
            if (bufplc == 4096) { bufplc = 0; if (write(fil,&tempbuf[0],4096) < 4096) { close(fil); return(-1); } }
            tempbuf[bufplc++] = (tempbuf[(i<<2)+4096+1]<<2);
            if (bufplc == 4096) { bufplc = 0; if (write(fil,&tempbuf[0],4096) < 4096) { close(fil); return(-1); } }
            tempbuf[bufplc++] = (tempbuf[(i<<2)+4096+0]<<2);
            if (bufplc == 4096) { bufplc = 0; if (write(fil,&tempbuf[0],4096) < 4096) { close(fil); return(-1); } }
        }
    }
    else
    {
        for(i=0;i<768;i++)
        {
            if (i < 48)
                tempbuf[bufplc++] = (vgapal16[i]<<2);
            else
                tempbuf[bufplc++] = 0;
            if (bufplc == 4096) { bufplc = 0; if (write(fil,&tempbuf[0],4096) < 4096) { close(fil); return(-1); } }
        }

    }

    if (bufplc > 0)
        if (write(fil,&tempbuf[0],bufplc) < bufplc) { close(fil); return(-1); }

    close(fil);
    capturecount++;
    return(0);
}
*/
int inside(long x, long y, short sectnum)
{
    walltype* wal;
    long i, x1, y1, x2, y2;
    unsigned long cnt;

    if ((sectnum < 0) || (sectnum >= numsectors)) return (-1);

    cnt = 0;
    wal = &wall[sector[sectnum].wallptr];
    i = sector[sectnum].wallnum;
    do
    {
        y1 = wal->y - y;
        y2 = wall[wal->point2].y - y;
        if ((y1 ^ y2) < 0)
        {
            x1 = wal->x - x;
            x2 = wall[wal->point2].x - x;
            if ((x1 ^ x2) >= 0) cnt ^= x1;
            else cnt ^= (x1 * y2 - x2 * y1) ^ y2;
        }
        wal++;
        i--;
    }
    while (i);
    return (cnt >> 31);
}

long getangle(long xvect, long yvect)
{
    if ((xvect | yvect) == 0) return (0);
    if (xvect == 0) return (512 + ((yvect < 0) << 10));
    if (yvect == 0) return (((xvect < 0) << 10));
    if (xvect == yvect) return (256 + ((xvect < 0) << 10));
    if (xvect == -yvect) return (768 + ((xvect > 0) << 10));
    if (klabs(xvect) > klabs(yvect))
        return (((radarang[640 + scale(160, yvect, xvect)] >> 6) + ((xvect < 0) << 10)) & 2047);
    return (((radarang[640 - scale(160, xvect, yvect)] >> 6) + 512 + ((yvect < 0) << 10)) & 2047);
}

long ksqrt(long num)
{
    return (nsqrtasm(num));
}

long krecip(long num)
{
    return (krecipasm(num));
}

void initksqrt()
{
    long i, j, k;

    j = 1;
    k = 0;
    for (i = 0; i < 4096; i++)
    {
        if (i >= j)
        {
            j <<= 2;
            k++;
        }
        sqrtable[i] = (unsigned short)(msqrtasm((i << 18) + 131072) << 1);
        shlookup[i] = (k << 1) + ((10 - k) << 8);
        if (i < 256) shlookup[i + 4096] = ((k + 6) << 1) + ((10 - (k + 6)) << 8);
    }
}

void copytilepiece(long tilenume1, long sx1, long sy1, long xsiz, long ysiz, long tilenume2, long sx2, long sy2)
{
    char *ptr1, *ptr2, dat;
    long xsiz1, ysiz1, xsiz2, ysiz2, i, j, x1, y1, x2, y2;

    xsiz1 = tilesizx[tilenume1];
    ysiz1 = tilesizy[tilenume1];
    xsiz2 = tilesizx[tilenume2];
    ysiz2 = tilesizy[tilenume2];
    if ((xsiz1 > 0) && (ysiz1 > 0) && (xsiz2 > 0) && (ysiz2 > 0))
    {
        if (waloff[tilenume1] == 0) loadtile(tilenume1);
        if (waloff[tilenume2] == 0) loadtile(tilenume2);

        x1 = sx1;
        for (i = 0; i < xsiz; i++)
        {
            y1 = sy1;
            for (j = 0; j < ysiz; j++)
            {
                x2 = sx2 + i;
                y2 = sy2 + j;
                if ((x2 >= 0) && (y2 >= 0) && (x2 < xsiz2) && (y2 < ysiz2))
                {
                    ptr1 = (char*)(waloff[tilenume1] + x1 * ysiz1 + y1);
                    ptr2 = (char*)(waloff[tilenume2] + x2 * ysiz2 + y2);
                    dat = *ptr1;
                    if (dat != 255)
                        *ptr2 = *ptr1;
                }

                y1++;
                if (y1 >= ysiz1) y1 = 0;
            }
            x1++;
            if (x1 >= xsiz1) x1 = 0;
        }
    }
}

int drawmasks()
{
    // scuba and stuff
    /*
    long i, j, k, l, gap, xs, ys, zs, xp, yp, zp, z1, z2, yoff, yspan;

    for(i=spritesortcnt-1;i>=0;i--) tspriteptr[i] = &tsprite[i];
    for(i=spritesortcnt-1;i>=0;i--)
    {
        xs = tspriteptr[i]->x-globalposx; ys = tspriteptr[i]->y-globalposy;
        yp = dmulscale6(xs,cosviewingrangeglobalang,ys,sinviewingrangeglobalang);
        if (yp > (4<<8))
        {
            xp = dmulscale6(ys,cosglobalang,-xs,singlobalang);
            spritesx[i] = scale(xp+yp,xdimen<<7,yp);
        }
        else if ((tspriteptr[i]->cstat&48) == 0)
        {
            spritesortcnt--;  //Delete face sprite if on wrong side!
            if (i != spritesortcnt)
            {
                tspriteptr[i] = tspriteptr[spritesortcnt];
                spritesx[i] = spritesx[spritesortcnt];
                spritesy[i] = spritesy[spritesortcnt];
            }
            continue;
        }
        spritesy[i] = yp;
    }

    gap = 1; while (gap < spritesortcnt) gap = (gap<<1)+1;
    for(gap>>=1;gap>0;gap>>=1)      //Sort sprite list
        for(i=0;i<spritesortcnt-gap;i++)
            for(l=i;l>=0;l-=gap)
            {
                if (spritesy[l] <= spritesy[l+gap]) break;
                swaplong(&tspriteptr[l],&tspriteptr[l+gap]);
                swaplong(&spritesx[l],&spritesx[l+gap]);
                swaplong(&spritesy[l],&spritesy[l+gap]);
            }

    if (spritesortcnt > 0)
        spritesy[spritesortcnt] = (spritesy[spritesortcnt-1]^1);

    ys = spritesy[0]; i = 0;
    for(j=1;j<=spritesortcnt;j++)
    {
        if (spritesy[j] == ys) continue;
        ys = spritesy[j];
        if (j > i+1)
        {
            for(k=i;k<j;k++)
            {
                spritesz[k] = tspriteptr[k]->z;
                if ((tspriteptr[k]->cstat&48) != 32)
                {
                    yoff = (long)((signed char)((picanm[tspriteptr[k]->picnum]>>16)&255))+((long)tspriteptr[k]->yoffset);
                    spritesz[k] -= ((yoff*tspriteptr[k]->yrepeat)<<2);
                    yspan = (tilesizy[tspriteptr[k]->picnum]*tspriteptr[k]->yrepeat<<2);
                    if (!(tspriteptr[k]->cstat&128)) spritesz[k] -= (yspan>>1);
                    if (klabs(spritesz[k]-globalposz) < (yspan>>1)) spritesz[k] = globalposz;
                }
            }
            for(k=i+1;k<j;k++)
                for(l=i;l<k;l++)
                    if (klabs(spritesz[k]-globalposz) < klabs(spritesz[l]-globalposz))
                    {
                        swaplong(&tspriteptr[k],&tspriteptr[l]);
                        swaplong(&spritesx[k],&spritesx[l]);
                        swaplong(&spritesy[k],&spritesy[l]);
                        swaplong(&spritesz[k],&spritesz[l]);
                    }
            for(k=i+1;k<j;k++)
                for(l=i;l<k;l++)
                    if (tspriteptr[k]->statnum < tspriteptr[l]->statnum)
                    {
                        swaplong(&tspriteptr[k],&tspriteptr[l]);
                        swaplong(&spritesx[k],&spritesx[l]);
                        swaplong(&spritesy[k],&spritesy[l]);
                    }
        }
        i = j;
    }

    /*for(i=spritesortcnt-1;i>=0;i--)
	{
		xs = tspriteptr[i].x-globalposx;
		ys = tspriteptr[i].y-globalposy;
		zs = tspriteptr[i].z-globalposz;

		xp = ys*cosglobalang-xs*singlobalang;
		yp = (zs<<1);
		zp = xs*cosglobalang+ys*singlobalang;

		xs = scale(xp,halfxdimen<<12,zp)+((halfxdimen+windowx1)<<12);
		ys = scale(yp,xdimenscale<<12,zp)+((globalhoriz+windowy1)<<12);

		drawline256(xs-65536,ys-65536,xs+65536,ys+65536,31);
		drawline256(xs+65536,ys-65536,xs-65536,ys+65536,31);
	}

    while ((spritesortcnt > 0) && (maskwallcnt > 0))  //While BOTH > 0
    {
        j = maskwall[maskwallcnt-1];
        if (spritewallfront(tspriteptr[spritesortcnt-1],(long)thewall[j]) == 0)
            drawsprite(--spritesortcnt);
        else
        {
            //Check to see if any sprites behind the masked wall...
            k = -1;
            gap = 0;
            for(i=spritesortcnt-2;i>=0;i--)
                if ((xb1[j] <= (spritesx[i]>>8)) && ((spritesx[i]>>8) <= xb2[j]))
                    if (spritewallfront(tspriteptr[i],(long)thewall[j]) == 0)
                    {
                        drawsprite(i);
                        tspriteptr[i]->owner = -1;
                        k = i;
                        gap++;
                    }
            if (k >= 0)       //remove holes in sprite list
            {
                for(i=k;i<spritesortcnt;i++)
                    if (tspriteptr[i]->owner >= 0)
                    {
                        if (i > k)
                        {
                            tspriteptr[k] = tspriteptr[i];
                            spritesx[k] = spritesx[i];
                            spritesy[k] = spritesy[i];
                        }
                        k++;
                    }
                spritesortcnt -= gap;
            }

            //finally safe to draw the masked wall
            drawmaskwall(--maskwallcnt);
        }
    }
    while (spritesortcnt > 0) drawsprite(--spritesortcnt);
    while (maskwallcnt > 0) drawmaskwall(--maskwallcnt);
    */
    return 0;
}

void drawmaskwall(short damaskwallcnt){};
/*
{
    long i, j, k, x, z, sectnum, z1, z2, lx, rx;
    sectortype *sec, *nsec;
    walltype *wal;

    z = maskwall[damaskwallcnt];
    wal = &wall[thewall[z]];
    sectnum = thesector[z]; sec = &sector[sectnum];
    nsec = &sector[wal->nextsector];
    z1 = max(nsec->ceilingz,sec->ceilingz);
    z2 = min(nsec->floorz,sec->floorz);

    wallmost(uwall,z,sectnum,(char)0);
    wallmost(uplc,z,(long)wal->nextsector,(char)0);
    for(x=xb1[z];x<=xb2[z];x++) if (uplc[x] > uwall[x]) uwall[x] = uplc[x];
    wallmost(dwall,z,sectnum,(char)1);
    wallmost(dplc,z,(long)wal->nextsector,(char)1);
    for(x=xb1[z];x<=xb2[z];x++) if (dplc[x] < dwall[x]) dwall[x] = dplc[x];
    prepwall(z,wal);

    globalorientation = (long)wal->cstat;
    globalpicnum = wal->overpicnum;
    if ((unsigned)globalpicnum >= (unsigned)MAXTILES) globalpicnum = 0;
    globalxpanning = (long)wal->xpanning;
    globalypanning = (long)wal->ypanning;
    if (picanm[globalpicnum]&192) globalpicnum += animateoffs(globalpicnum,(short)thewall[z]+16384);
    globalshade = (long)wal->shade;
    globvis = globalvisibility;
    if (sec->visibility != 0) globvis = mulscale4(globvis,(long)((unsigned char)(sec->visibility+16)));
    globalpal = (long)wal->pal;
    globalshiftval = (picsiz[globalpicnum]>>4);
    if (pow2long[globalshiftval] != tilesizy[globalpicnum]) globalshiftval++;
    globalshiftval = 32-globalshiftval;
    globalyscale = (wal->yrepeat<<(globalshiftval-19));
    if ((globalorientation&4) == 0)
        globalzd = (((globalposz-z1)*globalyscale)<<8);
    else
        globalzd = (((globalposz-z2)*globalyscale)<<8);
    globalzd += (globalypanning<<24);
    if (globalorientation&256) globalyscale = -globalyscale, globalzd = -globalzd;

    for(i=smostwallcnt-1;i>=0;i--)
    {
        j = smostwall[i];
        if ((xb1[j] > xb2[z]) || (xb2[j] < xb1[z])) continue;
        if (wallfront(j,z)) continue;

        lx = max(xb1[j],xb1[z]); rx = min(xb2[j],xb2[z]);

        switch(smostwalltype[i])
        {
        case 0:
            if (lx <= rx)
            {
                if ((lx == xb1[z]) && (rx == xb2[z])) return;
                clearbufbyte(&dwall[lx],(rx-lx+1)*sizeof(dwall[0]),0L);
            }
            break;
        case 1:
            k = smoststart[i] - xb1[j];
            for(x=lx;x<=rx;x++)
                if (smost[k+x] > uwall[x]) uwall[x] = smost[k+x];
            break;
        case 2:
            k = smoststart[i] - xb1[j];
            for(x=lx;x<=rx;x++)
                if (smost[k+x] < dwall[x]) dwall[x] = smost[k+x];
            break;
        }
    }

    //maskwall
    if ((searchit >= 1) && (searchx >= xb1[z]) && (searchx <= xb2[z]))
        if ((searchy >= uwall[searchx]) && (searchy <= dwall[searchx]))
        {
            searchsector = sectnum; searchwall = thewall[z];
            searchstat = 4; searchit = 1;
        }

    if ((globalorientation&128) == 0)
        maskwallscan(xb1[z],xb2[z],uwall,dwall,swall,lwall);
    else
    {
        if (globalorientation&128)
        {
            if (globalorientation&512) settransreverse(); else settransnormal();
        }
        transmaskwallscan(xb1[z],xb2[z]);
    }
}*/

void drawsprite(long snum)
{
};
/*
{

    spritetype *tspr;
    sectortype *sec;
    long startum, startdm, sectnum, xb, yp, cstat;
    long siz, xsiz, ysiz, xoff, yoff, xspan, yspan;
    long x1, y1, x2, y2, lx, rx, dalx2, darx2, i, j, k, x, linum, linuminc;
    long yplc, yinc, z, z1, z2, xp1, yp1, xp2, yp2;
    long xs, ys, xpos, ypos, xv, yv, top, topinc, bot, botinc, hplc, hinc;
    long cosang, sinang, dax, day, lpoint, lmax, rpoint, rmax, dax1, dax2, y;
    long npoints, npoints2, zz, t, zsgn, zzsgn, *longptr;
    signed short shade;
    short tilenum, spritenum;
    char swapped, daclip;

    tspr = tspriteptr[snum];

    xb = spritesx[snum];
    yp = spritesy[snum];
    tilenum = tspr->picnum;
    spritenum = tspr->owner;
    cstat = tspr->cstat;

    if ((cstat&48) != 48)
    {
        if (picanm[tilenum]&192) tilenum += animateoffs(tilenum,spritenum+32768);
        if ((tilesizx[tilenum] <= 0) || (tilesizy[tilenum] <= 0) || (spritenum < 0))
            return;
    }
    if ((tspr->xrepeat <= 0) || (tspr->yrepeat <= 0)) return;

    sectnum = tspr->sectnum; sec = &sector[sectnum];
    globalpal = tspr->pal;
    globalshade = tspr->shade;
    if (cstat&2)
    {
        if (cstat&512) settransreverse(); else settransnormal();
    }

    xoff = (long)((signed char)((picanm[tilenum]>>8)&255))+((long)tspr->xoffset);
    yoff = (long)((signed char)((picanm[tilenum]>>16)&255))+((long)tspr->yoffset);

    if ((cstat&48) == 0)
    {
        if (yp <= (4<<8)) return;

        siz = divscale19(xdimenscale,yp);

        xv = mulscale16(((long)tspr->xrepeat)<<16,xyaspect);

        xspan = tilesizx[tilenum];
        yspan = tilesizy[tilenum];
        xsiz = mulscale30(siz,xv*xspan);
        ysiz = mulscale14(siz,tspr->yrepeat*yspan);

        if (((tilesizx[tilenum]>>11) >= xsiz) || (yspan >= (ysiz>>1)))
            return;  //Watch out for divscale overflow

        x1 = xb-(xsiz>>1);
        if (xspan&1) x1 += mulscale31(siz,xv);  //Odd xspans
        i = mulscale30(siz,xv*xoff);
        if ((cstat&4) == 0) x1 -= i; else x1 += i;

        y1 = mulscale16(tspr->z-globalposz,siz);
        y1 -= mulscale14(siz,tspr->yrepeat*yoff);
        y1 += (globalhoriz<<8)-ysiz;
        if (cstat&128)
        {
            y1 += (ysiz>>1);
            if (yspan&1) y1 += mulscale15(siz,tspr->yrepeat);  //Odd yspans
        }

        x2 = x1+xsiz-1;
        y2 = y1+ysiz-1;
        if ((y1|255) >= (y2|255)) return;

        lx = (x1>>8)+1; if (lx < 0) lx = 0;
        rx = (x2>>8); if (rx >= xdimen) rx = xdimen-1;
        if (lx > rx) return;

        yinc = divscale32(yspan,ysiz);

        if ((sec->ceilingstat&3) == 0)
            startum = globalhoriz+mulscale24(siz,sec->ceilingz-globalposz)-1;
        else
            startum = 0;
        if ((sec->floorstat&3) == 0)
            startdm = globalhoriz+mulscale24(siz,sec->floorz-globalposz)+1;
        else
            startdm = 0x7fffffff;
        if ((y1>>8) > startum) startum = (y1>>8);
        if ((y2>>8) < startdm) startdm = (y2>>8);

        if (startum < -32768) startum = -32768;
        if (startdm > 32767) startdm = 32767;
        if (startum >= startdm) return;

        if ((cstat&4) == 0)
        {
            linuminc = divscale24(xspan,xsiz);
            linum = mulscale8((lx<<8)-x1,linuminc);
        }
        else
        {
            linuminc = -divscale24(xspan,xsiz);
            linum = mulscale8((lx<<8)-x2,linuminc);
        }
        if ((cstat&8) > 0)
        {
            yinc = -yinc;
            i = y1; y1 = y2; y2 = i;
        }

        for(x=lx;x<=rx;x++)
        {
            uwall[x] = max(startumost[x+windowx1]-windowy1,(short)startum);
            dwall[x] = min(startdmost[x+windowx1]-windowy1,(short)startdm);
        }
        daclip = 0;
        for(i=smostwallcnt-1;i>=0;i--)
        {
            if (smostwalltype[i]&daclip) continue;
            j = smostwall[i];
            if ((xb1[j] > rx) || (xb2[j] < lx)) continue;
            if ((yp <= yb1[j]) && (yp <= yb2[j])) continue;
            if (spritewallfront(tspr,(long)thewall[j]) && ((yp <= yb1[j]) || (yp <= yb2[j]))) continue;

            dalx2 = max(xb1[j],lx); darx2 = min(xb2[j],rx);

            switch(smostwalltype[i])
            {
            case 0:
                if (dalx2 <= darx2)
                {
                    if ((dalx2 == lx) && (darx2 == rx)) return;
                    clearbufbyte(&dwall[dalx2],(darx2-dalx2+1)*sizeof(dwall[0]),0L);
                }
                break;
            case 1:
                k = smoststart[i] - xb1[j];
                for(x=dalx2;x<=darx2;x++)
                    if (smost[k+x] > uwall[x]) uwall[x] = smost[k+x];
                if ((dalx2 == lx) && (darx2 == rx)) daclip |= 1;
                break;
            case 2:
                k = smoststart[i] - xb1[j];
                for(x=dalx2;x<=darx2;x++)
                    if (smost[k+x] < dwall[x]) dwall[x] = smost[k+x];
                if ((dalx2 == lx) && (darx2 == rx)) daclip |= 2;
                break;
            }
        }

        if (uwall[rx] >= dwall[rx])
        {
            for(x=lx;x<rx;x++)
                if (uwall[x] < dwall[x]) break;
            if (x == rx) return;
        }

        //sprite
        if ((searchit >= 1) && (searchx >= lx) && (searchx <= rx))
            if ((searchy >= uwall[searchx]) && (searchy < dwall[searchx]))
            {
                searchsector = sectnum; searchwall = spritenum;
                searchstat = 3; searchit = 1;
            }

        z2 = tspr->z - ((yoff*tspr->yrepeat)<<2);
        if (cstat&128)
        {
            z2 += ((yspan*tspr->yrepeat)<<1);
            if (yspan&1) z2 += (tspr->yrepeat<<1);        //Odd yspans
        }
        z1 = z2 - ((yspan*tspr->yrepeat)<<2);

        globalorientation = 0;
        globalpicnum = tilenum;
        if ((unsigned)globalpicnum >= (unsigned)MAXTILES) globalpicnum = 0;
        globalxpanning = 0L;
        globalypanning = 0L;
        globvis = globalvisibility;
        if (sec->visibility != 0) globvis = mulscale4(globvis,(long)((unsigned char)(sec->visibility+16)));
        globalshiftval = (picsiz[globalpicnum]>>4);
        if (pow2long[globalshiftval] != tilesizy[globalpicnum]) globalshiftval++;
        globalshiftval = 32-globalshiftval;
        globalyscale = divscale(512,tspr->yrepeat,globalshiftval-19);
        globalzd = (((globalposz-z1)*globalyscale)<<8);
        if ((cstat&8) > 0)
        {
            globalyscale = -globalyscale;
            globalzd = (((globalposz-z2)*globalyscale)<<8);
        }

        qinterpolatedown16(&lwall[lx],rx-lx+1,linum,linuminc);
        clearbuf(&swall[lx],rx-lx+1,mulscale19(yp,xdimscale));

        if ((cstat&2) == 0)
            maskwallscan(lx,rx,uwall,dwall,swall,lwall);
        else
            transmaskwallscan(lx,rx);
    }
    else if ((cstat&48) == 16)
    {
        if ((cstat&4) > 0) xoff = -xoff;
        if ((cstat&8) > 0) yoff = -yoff;

        xspan = tilesizx[tilenum]; yspan = tilesizy[tilenum];
        xv = tspr->xrepeat*sintable[(tspr->ang+2560+1536)&2047];
        yv = tspr->xrepeat*sintable[(tspr->ang+2048+1536)&2047];
        i = (xspan>>1)+xoff;
        x1 = tspr->x-globalposx-mulscale16(xv,i); x2 = x1+mulscale16(xv,xspan);
        y1 = tspr->y-globalposy-mulscale16(yv,i); y2 = y1+mulscale16(yv,xspan);

        yp1 = dmulscale6(x1,cosviewingrangeglobalang,y1,sinviewingrangeglobalang);
        yp2 = dmulscale6(x2,cosviewingrangeglobalang,y2,sinviewingrangeglobalang);
        if ((yp1 <= 0) && (yp2 <= 0)) return;
        xp1 = dmulscale6(y1,cosglobalang,-x1,singlobalang);
        xp2 = dmulscale6(y2,cosglobalang,-x2,singlobalang);

        x1 += globalposx; y1 += globalposy;
        x2 += globalposx; y2 += globalposy;

        swapped = 0;
        if (dmulscale32(xp1,yp2,-xp2,yp1) >= 0)  //If wall's NOT facing you
        {
            if ((cstat&64) != 0) return;
            i = xp1, xp1 = xp2, xp2 = i;
            i = yp1, yp1 = yp2, yp2 = i;
            i = x1, x1 = x2, x2 = i;
            i = y1, y1 = y2, y2 = i;
            swapped = 1;
        }

        if (xp1 >= -yp1)
        {
            if (xp1 > yp1) return;

            if (yp1 == 0) return;
            xb1[MAXWALLSB-1] = halfxdimen + scale(xp1,halfxdimen,yp1);
            if (xp1 >= 0) xb1[MAXWALLSB-1]++;   //Fix for SIGNED divide
            if (xb1[MAXWALLSB-1] >= xdimen) xb1[MAXWALLSB-1] = xdimen-1;
            yb1[MAXWALLSB-1] = yp1;
        }
        else
        {
            if (xp2 < -yp2) return;
            xb1[MAXWALLSB-1] = 0;
            i = yp1-yp2+xp1-xp2;
            if (i == 0) return;
            yb1[MAXWALLSB-1] = yp1 + scale(yp2-yp1,xp1+yp1,i);
        }
        if (xp2 <= yp2)
        {
            if (xp2 < -yp2) return;

            if (yp2 == 0) return;
            xb2[MAXWALLSB-1] = halfxdimen + scale(xp2,halfxdimen,yp2) - 1;
            if (xp2 >= 0) xb2[MAXWALLSB-1]++;   //Fix for SIGNED divide
            if (xb2[MAXWALLSB-1] >= xdimen) xb2[MAXWALLSB-1] = xdimen-1;
            yb2[MAXWALLSB-1] = yp2;
        }
        else
        {
            if (xp1 > yp1) return;

            xb2[MAXWALLSB-1] = xdimen-1;
            i = xp2-xp1+yp1-yp2;
            if (i == 0) return;
            yb2[MAXWALLSB-1] = yp1 + scale(yp2-yp1,yp1-xp1,i);
        }

        if ((yb1[MAXWALLSB-1] < 256) || (yb2[MAXWALLSB-1] < 256) || (xb1[MAXWALLSB-1] > xb2[MAXWALLSB-1]))
            return;

        topinc = -mulscale10(yp1,xspan);
        top = (((mulscale10(xp1,xdimen) - mulscale9(xb1[MAXWALLSB-1]-halfxdimen,yp1))*xspan)>>3);
        botinc = ((yp2-yp1)>>8);
        bot = mulscale11(xp1-xp2,xdimen) + mulscale2(xb1[MAXWALLSB-1]-halfxdimen,botinc);

        j = xb2[MAXWALLSB-1]+3;
        z = mulscale20(top,krecipasm(bot));
        lwall[xb1[MAXWALLSB-1]] = (z>>8);
        for(x=xb1[MAXWALLSB-1]+4;x<=j;x+=4)
        {
            top += topinc; bot += botinc;
            zz = z; z = mulscale20(top,krecipasm(bot));
            lwall[x] = (z>>8);
            i = ((z+zz)>>1);
            lwall[x-2] = (i>>8);
            lwall[x-3] = ((i+zz)>>9);
            lwall[x-1] = ((i+z)>>9);
        }

        if (lwall[xb1[MAXWALLSB-1]] < 0) lwall[xb1[MAXWALLSB-1]] = 0;
        if (lwall[xb2[MAXWALLSB-1]] >= xspan) lwall[xb2[MAXWALLSB-1]] = xspan-1;

        if ((swapped^((cstat&4)>0)) > 0)
        {
            j = xspan-1;
            for(x=xb1[MAXWALLSB-1];x<=xb2[MAXWALLSB-1];x++)
                lwall[x] = j-lwall[x];
        }

        rx1[MAXWALLSB-1] = xp1; ry1[MAXWALLSB-1] = yp1;
        rx2[MAXWALLSB-1] = xp2; ry2[MAXWALLSB-1] = yp2;

        hplc = divscale19(xdimenscale,yb1[MAXWALLSB-1]);
        hinc = divscale19(xdimenscale,yb2[MAXWALLSB-1]);
        hinc = (hinc-hplc)/(xb2[MAXWALLSB-1]-xb1[MAXWALLSB-1]+1);

        z2 = tspr->z - ((yoff*tspr->yrepeat)<<2);
        if (cstat&128)
        {
            z2 += ((yspan*tspr->yrepeat)<<1);
            if (yspan&1) z2 += (tspr->yrepeat<<1);        //Odd yspans
        }
        z1 = z2 - ((yspan*tspr->yrepeat)<<2);

        globalorientation = 0;
        globalpicnum = tilenum;
        if ((unsigned)globalpicnum >= (unsigned)MAXTILES) globalpicnum = 0;
        globalxpanning = 0L;
        globalypanning = 0L;
        globvis = globalvisibility;
        if (sec->visibility != 0) globvis = mulscale4(globvis,(long)((unsigned char)(sec->visibility+16)));
        globalshiftval = (picsiz[globalpicnum]>>4);
        if (pow2long[globalshiftval] != tilesizy[globalpicnum]) globalshiftval++;
        globalshiftval = 32-globalshiftval;
        globalyscale = divscale(512,tspr->yrepeat,globalshiftval-19);
        globalzd = (((globalposz-z1)*globalyscale)<<8);
        if ((cstat&8) > 0)
        {
            globalyscale = -globalyscale;
            globalzd = (((globalposz-z2)*globalyscale)<<8);
        }

        if (((sec->ceilingstat&1) == 0) && (z1 < sec->ceilingz))
            z1 = sec->ceilingz;
        if (((sec->floorstat&1) == 0) && (z2 > sec->floorz))
            z2 = sec->floorz;

        owallmost(uwall,(long)(MAXWALLSB-1),z1-globalposz);
        owallmost(dwall,(long)(MAXWALLSB-1),z2-globalposz);
        for(i=xb1[MAXWALLSB-1];i<=xb2[MAXWALLSB-1];i++)
        { swall[i] = (krecipasm(hplc)<<2); hplc += hinc; }

        for(i=smostwallcnt-1;i>=0;i--)
        {
            j = smostwall[i];

            if ((xb1[j] > xb2[MAXWALLSB-1]) || (xb2[j] < xb1[MAXWALLSB-1])) continue;

            dalx2 = xb1[j]; darx2 = xb2[j];
            if (max(yb1[MAXWALLSB-1],yb2[MAXWALLSB-1]) > min(yb1[j],yb2[j]))
            {
                if (min(yb1[MAXWALLSB-1],yb2[MAXWALLSB-1]) > max(yb1[j],yb2[j]))
                {
                    x = 0x80000000;
                }
                else
                {
                    x = thewall[j]; xp1 = wall[x].x; yp1 = wall[x].y;
                    x = wall[x].point2; xp2 = wall[x].x; yp2 = wall[x].y;

                    z1 = (xp2-xp1)*(y1-yp1) - (yp2-yp1)*(x1-xp1);
                    z2 = (xp2-xp1)*(y2-yp1) - (yp2-yp1)*(x2-xp1);
                    if ((z1^z2) >= 0)
                        x = (z1+z2);
                    else
                    {
                        z1 = (x2-x1)*(yp1-y1) - (y2-y1)*(xp1-x1);
                        z2 = (x2-x1)*(yp2-y1) - (y2-y1)*(xp2-x1);

                        if ((z1^z2) >= 0)
                            x = -(z1+z2);
                        else
                        {
                            if ((xp2-xp1)*(tspr->y-yp1) == (tspr->x-xp1)*(yp2-yp1))
                            {
                                if (wall[thewall[j]].nextsector == tspr->sectnum)
                                    x = 0x80000000;
                                else
                                    x = 0x7fffffff;
                            }
                            else
                            {     //INTERSECTION!
                                x = (xp1-globalposx) + scale(xp2-xp1,z1,z1-z2);
                                y = (yp1-globalposy) + scale(yp2-yp1,z1,z1-z2);

                                yp1 = dmulscale14(x,cosglobalang,y,singlobalang);
                                if (yp1 > 0)
                                {
                                    xp1 = dmulscale14(y,cosglobalang,-x,singlobalang);

                                    x = halfxdimen + scale(xp1,halfxdimen,yp1);
                                    if (xp1 >= 0) x++;   //Fix for SIGNED divide

                                    if (z1 < 0)
                                    { if (dalx2 < x) dalx2 = x; }
                                    else
                                    { if (darx2 > x) darx2 = x; }
                                    x = 0x80000001;
                                }
                                else
                                    x = 0x7fffffff;
                            }
                        }
                    }
                }
                if (x < 0)
                {
                    if (dalx2 < xb1[MAXWALLSB-1]) dalx2 = xb1[MAXWALLSB-1];
                    if (darx2 > xb2[MAXWALLSB-1]) darx2 = xb2[MAXWALLSB-1];
                    switch(smostwalltype[i])
                    {
                    case 0:
                        if (dalx2 <= darx2)
                        {
                            if ((dalx2 == xb1[MAXWALLSB-1]) && (darx2 == xb2[MAXWALLSB-1])) return;
                            clearbufbyte(&dwall[dalx2],(darx2-dalx2+1)*sizeof(dwall[0]),0L);
                        }
                        break;
                    case 1:
                        k = smoststart[i] - xb1[j];
                        for(x=dalx2;x<=darx2;x++)
                            if (smost[k+x] > uwall[x]) uwall[x] = smost[k+x];
                        break;
                    case 2:
                        k = smoststart[i] - xb1[j];
                        for(x=dalx2;x<=darx2;x++)
                            if (smost[k+x] < dwall[x]) dwall[x] = smost[k+x];
                        break;
                    }
                }
            }
        }

        //sprite
        if ((searchit >= 1) && (searchx >= xb1[MAXWALLSB-1]) && (searchx <= xb2[MAXWALLSB-1]))
            if ((searchy >= uwall[searchx]) && (searchy <= dwall[searchx]))
            {
                searchsector = sectnum; searchwall = spritenum;
                searchstat = 3; searchit = 1;
            }

        if ((cstat&2) == 0)
            maskwallscan(xb1[MAXWALLSB-1],xb2[MAXWALLSB-1],uwall,dwall,swall,lwall);
        else
            transmaskwallscan(xb1[MAXWALLSB-1],xb2[MAXWALLSB-1]);
    }
    else if ((cstat&48) == 32)
    {
        if ((cstat&64) != 0)
            if ((globalposz > tspr->z) == ((cstat&8)==0))
                return;

        if ((cstat&4) > 0) xoff = -xoff;
        if ((cstat&8) > 0) yoff = -yoff;
        xspan = tilesizx[tilenum];
        yspan = tilesizy[tilenum];

        //Rotate center point
        dax = tspr->x-globalposx;
        day = tspr->y-globalposy;
        rzi[0] = dmulscale10(cosglobalang,dax,singlobalang,day);
        rxi[0] = dmulscale10(cosglobalang,day,-singlobalang,dax);

        //Get top-left corner
        i = ((tspr->ang+2048-globalang)&2047);
        cosang = sintable[(i+512)&2047]; sinang = sintable[i];
        dax = ((xspan>>1)+xoff)*tspr->xrepeat;
        day = ((yspan>>1)+yoff)*tspr->yrepeat;
        rzi[0] += dmulscale12(sinang,dax,cosang,day);
        rxi[0] += dmulscale12(sinang,day,-cosang,dax);

        //Get other 3 corners
        dax = xspan*tspr->xrepeat;
        day = yspan*tspr->yrepeat;
        rzi[1] = rzi[0]-mulscale12(sinang,dax);
        rxi[1] = rxi[0]+mulscale12(cosang,dax);
        dax = -mulscale12(cosang,day);
        day = -mulscale12(sinang,day);
        rzi[2] = rzi[1]+dax; rxi[2] = rxi[1]+day;
        rzi[3] = rzi[0]+dax; rxi[3] = rxi[0]+day;

        //Put all points on same z
        ryi[0] = scale((tspr->z-globalposz),yxaspect,320<<8);
        if (ryi[0] == 0) return;
        ryi[1] = ryi[2] = ryi[3] = ryi[0];

        if ((cstat&4) == 0)
        { z = 0; z1 = 1; z2 = 3; }
        else
        { z = 1; z1 = 0; z2 = 2; }

        dax = rzi[z1]-rzi[z]; day = rxi[z1]-rxi[z];
        bot = dmulscale8(dax,dax,day,day);
        if (((klabs(dax)>>13) >= bot) || ((klabs(day)>>13) >= bot)) return;
        globalx1 = divscale18(dax,bot);
        globalx2 = divscale18(day,bot);

        dax = rzi[z2]-rzi[z]; day = rxi[z2]-rxi[z];
        bot = dmulscale8(dax,dax,day,day);
        if (((klabs(dax)>>13) >= bot) || ((klabs(day)>>13) >= bot)) return;
        globaly1 = divscale18(dax,bot);
        globaly2 = divscale18(day,bot);

        //Calculate globals for hline texture mapping function
        globalxpanning = (rxi[z]<<12);
        globalypanning = (rzi[z]<<12);
        globalzd = (ryi[z]<<12);

        rzi[0] = mulscale16(rzi[0],viewingrange);
        rzi[1] = mulscale16(rzi[1],viewingrange);
        rzi[2] = mulscale16(rzi[2],viewingrange);
        rzi[3] = mulscale16(rzi[3],viewingrange);

        if (ryi[0] < 0)   //If ceilsprite is above you, reverse order of points
        {
            i = rxi[1]; rxi[1] = rxi[3]; rxi[3] = i;
            i = rzi[1]; rzi[1] = rzi[3]; rzi[3] = i;
        }


        //Clip polygon in 3-space
        npoints = 4;

        //Clip edge 1
        npoints2 = 0;
        zzsgn = rxi[0]+rzi[0];
        for(z=0;z<npoints;z++)
        {
            zz = z+1; if (zz == npoints) zz = 0;
            zsgn = zzsgn; zzsgn = rxi[zz]+rzi[zz];
            if (zsgn >= 0)
            {
                rxi2[npoints2] = rxi[z]; ryi2[npoints2] = ryi[z]; rzi2[npoints2] = rzi[z];
                npoints2++;
            }
            if ((zsgn^zzsgn) < 0)
            {
                t = divscale30(zsgn,zsgn-zzsgn);
                rxi2[npoints2] = rxi[z] + mulscale30(t,rxi[zz]-rxi[z]);
                ryi2[npoints2] = ryi[z] + mulscale30(t,ryi[zz]-ryi[z]);
                rzi2[npoints2] = rzi[z] + mulscale30(t,rzi[zz]-rzi[z]);
                npoints2++;
            }
        }
        if (npoints2 <= 2) return;

        //Clip edge 2
        npoints = 0;
        zzsgn = rxi2[0]-rzi2[0];
        for(z=0;z<npoints2;z++)
        {
            zz = z+1; if (zz == npoints2) zz = 0;
            zsgn = zzsgn; zzsgn = rxi2[zz]-rzi2[zz];
            if (zsgn <= 0)
            {
                rxi[npoints] = rxi2[z]; ryi[npoints] = ryi2[z]; rzi[npoints] = rzi2[z];
                npoints++;
            }
            if ((zsgn^zzsgn) < 0)
            {
                t = divscale30(zsgn,zsgn-zzsgn);
                rxi[npoints] = rxi2[z] + mulscale30(t,rxi2[zz]-rxi2[z]);
                ryi[npoints] = ryi2[z] + mulscale30(t,ryi2[zz]-ryi2[z]);
                rzi[npoints] = rzi2[z] + mulscale30(t,rzi2[zz]-rzi2[z]);
                npoints++;
            }
        }
        if (npoints <= 2) return;

        //Clip edge 3
        npoints2 = 0;
        zzsgn = ryi[0]*halfxdimen + (rzi[0]*(globalhoriz-0));
        for(z=0;z<npoints;z++)
        {
            zz = z+1; if (zz == npoints) zz = 0;
            zsgn = zzsgn; zzsgn = ryi[zz]*halfxdimen + (rzi[zz]*(globalhoriz-0));
            if (zsgn >= 0)
            {
                rxi2[npoints2] = rxi[z];
                ryi2[npoints2] = ryi[z];
                rzi2[npoints2] = rzi[z];
                npoints2++;
            }
            if ((zsgn^zzsgn) < 0)
            {
                t = divscale30(zsgn,zsgn-zzsgn);
                rxi2[npoints2] = rxi[z] + mulscale30(t,rxi[zz]-rxi[z]);
                ryi2[npoints2] = ryi[z] + mulscale30(t,ryi[zz]-ryi[z]);
                rzi2[npoints2] = rzi[z] + mulscale30(t,rzi[zz]-rzi[z]);
                npoints2++;
            }
        }
        if (npoints2 <= 2) return;

        //Clip edge 4
        npoints = 0;
        zzsgn = ryi2[0]*halfxdimen + (rzi2[0]*(globalhoriz-ydimen));
        for(z=0;z<npoints2;z++)
        {
            zz = z+1; if (zz == npoints2) zz = 0;
            zsgn = zzsgn; zzsgn = ryi2[zz]*halfxdimen + (rzi2[zz]*(globalhoriz-ydimen));
            if (zsgn <= 0)
            {
                rxi[npoints] = rxi2[z];
                ryi[npoints] = ryi2[z];
                rzi[npoints] = rzi2[z];
                npoints++;
            }
            if ((zsgn^zzsgn) < 0)
            {
                t = divscale30(zsgn,zsgn-zzsgn);
                rxi[npoints] = rxi2[z] + mulscale30(t,rxi2[zz]-rxi2[z]);
                ryi[npoints] = ryi2[z] + mulscale30(t,ryi2[zz]-ryi2[z]);
                rzi[npoints] = rzi2[z] + mulscale30(t,rzi2[zz]-rzi2[z]);
                npoints++;
            }
        }
        if (npoints <= 2) return;

        //Project onto screen
        lpoint = -1; lmax = 0x7fffffff;
        rpoint = -1; rmax = 0x80000000;
        for(z=0;z<npoints;z++)
        {
            xsi[z] = scale(rxi[z],xdimen<<15,rzi[z]) + (xdimen<<15);
            ysi[z] = scale(ryi[z],xdimen<<15,rzi[z]) + (globalhoriz<<16);
            if (xsi[z] < 0) xsi[z] = 0;
            if (xsi[z] > (xdimen<<16)) xsi[z] = (xdimen<<16);
            if (ysi[z] < ((long)0<<16)) ysi[z] = ((long)0<<16);
            if (ysi[z] > ((long)ydimen<<16)) ysi[z] = ((long)ydimen<<16);
            if (xsi[z] < lmax) lmax = xsi[z], lpoint = z;
            if (xsi[z] > rmax) rmax = xsi[z], rpoint = z;
        }

        //Get uwall arrays
        for(z=lpoint;z!=rpoint;z=zz)
        {
            zz = z+1; if (zz == npoints) zz = 0;

            dax1 = ((xsi[z]+65535)>>16);
            dax2 = ((xsi[zz]+65535)>>16);
            if (dax2 > dax1)
            {
                yinc = divscale16(ysi[zz]-ysi[z],xsi[zz]-xsi[z]);
                y = ysi[z] + mulscale16((dax1<<16)-xsi[z],yinc);
                qinterpolatedown16short((&uwall[dax1]),dax2-dax1,y,yinc);
            }
        }

        //Get dwall arrays
        for(;z!=lpoint;z=zz)
        {
            zz = z+1; if (zz == npoints) zz = 0;

            dax1 = ((xsi[zz]+65535)>>16);
            dax2 = ((xsi[z]+65535)>>16);
            if (dax2 > dax1)
            {
                yinc = divscale16(ysi[zz]-ysi[z],xsi[zz]-xsi[z]);
                y = ysi[zz] + mulscale16((dax1<<16)-xsi[zz],yinc);
                qinterpolatedown16short((&dwall[dax1]),dax2-dax1,y,yinc);
            }
        }


        lx = ((lmax+65535)>>16);
        rx = ((rmax+65535)>>16);
        for(x=lx;x<=rx;x++)
        {
            uwall[x] = max(uwall[x],startumost[x+windowx1]-windowy1);
            dwall[x] = min(dwall[x],startdmost[x+windowx1]-windowy1);
        }

        //Additional uwall/dwall clipping goes here
        for(i=smostwallcnt-1;i>=0;i--)
        {
            j = smostwall[i];
            if ((xb1[j] > rx) || (xb2[j] < lx)) continue;
            if ((yp <= yb1[j]) && (yp <= yb2[j])) continue;

            //if (spritewallfront(tspr,thewall[j]) == 0)
            x = thewall[j]; xp1 = wall[x].x; yp1 = wall[x].y;
            x = wall[x].point2; xp2 = wall[x].x; yp2 = wall[x].y;
            x = (xp2-xp1)*(tspr->y-yp1)-(tspr->x-xp1)*(yp2-yp1);
            if ((yp > yb1[j]) && (yp > yb2[j])) x = -1;
            if ((x >= 0) && ((x != 0) || (wall[thewall[j]].nextsector != tspr->sectnum))) continue;

            dalx2 = max(xb1[j],lx); darx2 = min(xb2[j],rx);

            switch(smostwalltype[i])
            {
            case 0:
                if (dalx2 <= darx2)
                {
                    if ((dalx2 == lx) && (darx2 == rx)) return;
                    clearbufbyte(&dwall[dalx2],(darx2-dalx2+1)*sizeof(dwall[0]),0L);
                }
                break;
            case 1:
                k = smoststart[i] - xb1[j];
                for(x=dalx2;x<=darx2;x++)
                    if (smost[k+x] > uwall[x]) uwall[x] = smost[k+x];
                break;
            case 2:
                k = smoststart[i] - xb1[j];
                for(x=dalx2;x<=darx2;x++)
                    if (smost[k+x] < dwall[x]) dwall[x] = smost[k+x];
                break;
            }
        }

        //sprite
        if ((searchit >= 1) && (searchx >= lx) && (searchx <= rx))
            if ((searchy >= uwall[searchx]) && (searchy <= dwall[searchx]))
            {
                searchsector = sectnum; searchwall = spritenum;
                searchstat = 3; searchit = 1;
            }

        globalorientation = cstat;
        globalpicnum = tilenum;
        if ((unsigned)globalpicnum >= (unsigned)MAXTILES) globalpicnum = 0;
        //if (picanm[globalpicnum]&192) globalpicnum += animateoffs((short)globalpicnum,spritenum+32768);

        if (waloff[globalpicnum] == 0) loadtile(globalpicnum);
        setgotpic(globalpicnum);
        globalbufplc = waloff[globalpicnum];

        globvis = mulscale16(globalhisibility,viewingrange);
        if (sec->visibility != 0) globvis = mulscale4(globvis,(long)((unsigned char)(sec->visibility+16)));

        x = picsiz[globalpicnum]; y = ((x>>4)&15); x &= 15;
        if (pow2long[x] != xspan)
        {
            x++;
            globalx1 = mulscale(globalx1,xspan,x);
            globalx2 = mulscale(globalx2,xspan,x);
        }

        dax = globalxpanning; day = globalypanning;
        globalxpanning = -dmulscale6(globalx1,day,globalx2,dax);
        globalypanning = -dmulscale6(globaly1,day,globaly2,dax);

        globalx2 = mulscale16(globalx2,viewingrange);
        globaly2 = mulscale16(globaly2,viewingrange);
        globalzd = mulscale16(globalzd,viewingrangerecip);

        globalx1 = (globalx1-globalx2)*halfxdimen;
        globaly1 = (globaly1-globaly2)*halfxdimen;

        if ((cstat&2) == 0)
            msethlineshift(x,y);
        else
            tsethlineshift(x,y);

        //Draw it!
        ceilspritescan(lx,rx-1);
    }
#ifdef SUPERBUILD
    else if ((cstat&48) == 48)
    {
        lx = 0; rx = xdim-1;
        for(x=lx;x<=rx;x++)
        {
            lwall[x] = (long)startumost[x+windowx1]-windowy1;
            swall[x] = (long)startdmost[x+windowx1]-windowy1;
        }
        for(i=smostwallcnt-1;i>=0;i--)
        {
            j = smostwall[i];
            if ((xb1[j] > rx) || (xb2[j] < lx)) continue;
            if ((yp <= yb1[j]) && (yp <= yb2[j])) continue;
            if (spritewallfront(tspr,(long)thewall[j]) && ((yp <= yb1[j]) || (yp <= yb2[j]))) continue;

            dalx2 = max(xb1[j],lx); darx2 = min(xb2[j],rx);

            switch(smostwalltype[i])
            {
            case 0:
                if (dalx2 <= darx2)
                {
                    if ((dalx2 == lx) && (darx2 == rx)) return;
                    clearbufbyte(&swall[dalx2],(darx2-dalx2+1)*sizeof(swall[0]),0L);
                }
                break;
            case 1:
                k = smoststart[i] - xb1[j];
                for(x=dalx2;x<=darx2;x++)
                    if (smost[k+x] > lwall[x]) lwall[x] = smost[k+x];
                break;
            case 2:
                k = smoststart[i] - xb1[j];
                for(x=dalx2;x<=darx2;x++)
                    if (smost[k+x] < swall[x]) swall[x] = smost[k+x];
                break;
            }
        }

        if (lwall[rx] >= swall[rx])
        {
            for(x=lx;x<rx;x++)
                if (lwall[x] < swall[x]) break;
            if (x == rx) return;
        }

        for(i=0;i<MAXVOXMIPS;i++)
            if (!voxoff[tilenum][i])
            {
                kloadvoxel(tilenum);
                break;
            }

        longptr = (long *)voxoff[tilenum][0];
        if (!(cstat&128)) tspr->z -= mulscale6(longptr[5],(long)tspr->yrepeat);
        yoff = (long)((signed char)((picanm[sprite[tspr->owner].picnum]>>16)&255))+((long)tspr->yoffset);
        tspr->z -= ((yoff*tspr->yrepeat)<<2);

        globvis = globalvisibility;
        if (sec->visibility != 0) globvis = mulscale4(globvis,(long)((unsigned char)(sec->visibility+16)));

        if ((searchit >= 1) && (yp > (4<<8)) && (searchy >= lwall[searchx]) && (searchy < swall[searchx]))
        {
            siz = divscale19(xdimenscale,yp);

            xv = mulscale16(((long)tspr->xrepeat)<<16,xyaspect);

            xspan = ((longptr[0]+longptr[1])>>1);
            yspan = longptr[2];
            xsiz = mulscale30(siz,xv*xspan);
            ysiz = mulscale14(siz,tspr->yrepeat*yspan);

            //Watch out for divscale overflow
            if (((xspan>>11) < xsiz) && (yspan < (ysiz>>1)))
            {
                x1 = xb-(xsiz>>1);
                if (xspan&1) x1 += mulscale31(siz,xv);  //Odd xspans
                i = mulscale30(siz,xv*xoff);
                if ((cstat&4) == 0) x1 -= i; else x1 += i;

                y1 = mulscale16(tspr->z-globalposz,siz);
                //y1 -= mulscale14(siz,tspr->yrepeat*yoff);
                y1 += (globalhoriz<<8)-ysiz;
                //if (cstat&128)  //Already fixed up above
                y1 += (ysiz>>1);

                x2 = x1+xsiz-1;
                y2 = y1+ysiz-1;
                if (((y1|255) < (y2|255)) && (searchx >= (x1>>8)+1) && (searchx <= (x2>>8)))
                {
                    if ((sec->ceilingstat&3) == 0)
                        startum = globalhoriz+mulscale24(siz,sec->ceilingz-globalposz)-1;
                    else
                        startum = 0;
                    if ((sec->floorstat&3) == 0)
                        startdm = globalhoriz+mulscale24(siz,sec->floorz-globalposz)+1;
                    else
                        startdm = 0x7fffffff;

                    //sprite
                    if ((searchy >= max(startum,(y1>>8))) && (searchy < min(startdm,(y2>>8))))
                    {
                        searchsector = sectnum; searchwall = spritenum;
                        searchstat = 3; searchit = 1;
                    }
                }
            }
        }

        drawvox(tspr->x,tspr->y,tspr->z,(long)tspr->ang+1536,(long)tspr->xrepeat,(long)tspr->yrepeat,tilenum,tspr->shade,tspr->pal,lwall,swall);
    }
#endif
    if (automapping == 1) show2dsprite[spritenum>>3] |= pow2char[spritenum&7];
}
*/
void drawvox(long dasprx, long daspry, long dasprz, long dasprang, long daxscale, long dayscale, char daindex,
             signed char dashade, char dapal, long* daumost, long* dadmost)
{
    // use kv6; or use this.
}
/*
{
    long i, j, k, x, y, z, syoff, ggxstart, ggystart, nxoff;
    long cosang, sinang, sprcosang, sprsinang, backx, backy, gxinc, gyinc;
    long daxsiz, daysiz, dazsiz, daxpivot, daypivot, dazpivot;
    long daxscalerecip, dayscalerecip, cnt, gxstart, gystart, odayscale;
    long l1, l2, p, pend, slabxoffs, xyvoxoffs, *longptr;
    long lx, rx, nx, ny, zx, zy, x1, y1, z1, x2, y2, z2, yplc, yinc, bufplc;
    long yoff, xs, ys, xe, ye, xi, yi, cbackx, cbacky, dagxinc, dagyinc;
    short *shortptr;
    char *voxptr, *voxend, *davoxptr, oand, oand16, oand32;

    cosang = sintable[(globalang+512)&2047];
    sinang = sintable[globalang&2047];
    sprcosang = sintable[(dasprang+512)&2047];
    sprsinang = sintable[dasprang&2047];

    i = klabs(dmulscale6(dasprx-globalposx,cosang,daspry-globalposy,sinang));
    j = (long)(getpalookup((long)mulscale21(globvis,i),(long)dashade)<<8);
    setupdrawslab(ylookup[1],FP_OFF(palookup[dapal])+j);
    j = 1310720;
    j *= min(daxscale,dayscale); j >>= 6;  //New hacks (for sized-down voxels)
    for(k=0;k<MAXVOXMIPS;k++)
    {
        if (i < j) { i = k; break; }
        j <<= 1;
    }
    if (k >= MAXVOXMIPS) i = MAXVOXMIPS-1;

    davoxptr = (char *)voxoff[daindex][i]; if (!davoxptr) return;

    daxscale <<= (i+8); dayscale <<= (i+8);
    odayscale = dayscale;
    daxscale = mulscale16(daxscale,xyaspect);
    daxscale = scale(daxscale,xdimenscale,xdimen<<8);
    dayscale = scale(dayscale,mulscale16(xdimenscale,viewingrangerecip),xdimen<<8);

    daxscalerecip = (1<<30)/daxscale;
    dayscalerecip = (1<<30)/dayscale;

    longptr = (long *)davoxptr;
    daxsiz = longptr[0]; daysiz = longptr[1]; dazsiz = longptr[2];
    daxpivot = longptr[3]; daypivot = longptr[4]; dazpivot = longptr[5];
    davoxptr += (6<<2);

    x = mulscale16(globalposx-dasprx,daxscalerecip);
    y = mulscale16(globalposy-daspry,daxscalerecip);
    backx = ((dmulscale10(x,sprcosang,y,sprsinang)+daxpivot)>>8);
    backy = ((dmulscale10(y,sprcosang,x,-sprsinang)+daypivot)>>8);
    cbackx = min(max(backx,0),daxsiz-1);
    cbacky = min(max(backy,0),daysiz-1);

    sprcosang = mulscale14(daxscale,sprcosang);
    sprsinang = mulscale14(daxscale,sprsinang);

    x = (dasprx-globalposx) - dmulscale18(daxpivot,sprcosang,daypivot,-sprsinang);
    y = (daspry-globalposy) - dmulscale18(daypivot,sprcosang,daxpivot,sprsinang);

    cosang = mulscale16(cosang,dayscalerecip);
    sinang = mulscale16(sinang,dayscalerecip);

    gxstart = y*cosang - x*sinang;
    gystart = x*cosang + y*sinang;
    gxinc = dmulscale10(sprsinang,cosang,sprcosang,-sinang);
    gyinc = dmulscale10(sprcosang,cosang,sprsinang,sinang);

    x = 0; y = 0; j = max(daxsiz,daysiz);
    for(i=0;i<=j;i++)
    {
        ggxinc[i] = x; x += gxinc;
        ggyinc[i] = y; y += gyinc;
    }

    if ((klabs(globalposz-dasprz)>>10) >= klabs(odayscale)) return;
    syoff = divscale21(globalposz-dasprz,odayscale) + (dazpivot<<7);
    yoff = ((klabs(gxinc)+klabs(gyinc))>>1);
    longptr = (long *)davoxptr;
    xyvoxoffs = ((daxsiz+1)<<2);

    for(cnt=0;cnt<8;cnt++)
    {
        switch(cnt)
        {
        case 0: xs = 0;        ys = 0;        xi = 1;  yi = 1;  break;
        case 1: xs = daxsiz-1; ys = 0;        xi = -1; yi = 1;  break;
        case 2: xs = 0;        ys = daysiz-1; xi = 1;  yi = -1; break;
        case 3: xs = daxsiz-1; ys = daysiz-1; xi = -1; yi = -1; break;
        case 4: xs = 0;        ys = cbacky;   xi = 1;  yi = 2;  break;
        case 5: xs = daxsiz-1; ys = cbacky;   xi = -1; yi = 2;  break;
        case 6: xs = cbackx;   ys = 0;        xi = 2;  yi = 1;  break;
        case 7: xs = cbackx;   ys = daysiz-1; xi = 2;  yi = -1; break;
        }
        xe = cbackx; ye = cbacky;
        if (cnt < 4)
        {
            if ((xi < 0) && (xe >= xs)) continue;
            if ((xi > 0) && (xe <= xs)) continue;
            if ((yi < 0) && (ye >= ys)) continue;
            if ((yi > 0) && (ye <= ys)) continue;
        }
        else
        {
            if ((xi < 0) && (xe > xs)) continue;
            if ((xi > 0) && (xe < xs)) continue;
            if ((yi < 0) && (ye > ys)) continue;
            if ((yi > 0) && (ye < ys)) continue;
            xe += xi; ye += yi;
        }

        i = ksgn(ys-backy)+ksgn(xs-backx)*3+4;
        switch(i)
        {
        case 6: case 7: x1 = 0; y1 = 0; break;
        case 8: case 5: x1 = gxinc; y1 = gyinc; break;
        case 0: case 3: x1 = gyinc; y1 = -gxinc; break;
        case 2: case 1: x1 = gxinc+gyinc; y1 = gyinc-gxinc; break;
        }
        switch(i)
        {
        case 2: case 5: x2 = 0; y2 = 0; break;
        case 0: case 1: x2 = gxinc; y2 = gyinc; break;
        case 8: case 7: x2 = gyinc; y2 = -gxinc; break;
        case 6: case 3: x2 = gxinc+gyinc; y2 = gyinc-gxinc; break;
        }
        oand = pow2char[(xs<backx)+0]+pow2char[(ys<backy)+2];
        oand16 = oand+16;
        oand32 = oand+32;

        if (yi > 0) { dagxinc = gxinc; dagyinc = mulscale16(gyinc,viewingrangerecip); }
        else { dagxinc = -gxinc; dagyinc = -mulscale16(gyinc,viewingrangerecip); }

        //Fix for non 90 degree viewing ranges
        nxoff = mulscale16(x2-x1,viewingrangerecip);
        x1 = mulscale16(x1,viewingrangerecip);

        ggxstart = gxstart+ggyinc[ys];
        ggystart = gystart-ggxinc[ys];

        for(x=xs;x!=xe;x+=xi)
        {
            slabxoffs = &davoxptr[longptr[x]];
            shortptr = (short *)&davoxptr[((x*(daysiz+1))<<1)+xyvoxoffs];

            nx = mulscale16(ggxstart+ggxinc[x],viewingrangerecip)+x1;
            ny = ggystart+ggyinc[x];
            for(y=ys;y!=ye;y+=yi,nx+=dagyinc,ny-=dagxinc)
            {
                if ((ny <= nytooclose) || (ny >= nytoofar)) continue;
                voxptr = (char *)(shortptr[y]+slabxoffs);
                voxend = (char *)(shortptr[y+1]+slabxoffs);
                if (voxptr == voxend) continue;

                lx = mulscale32(nx>>3,distrecip[(ny+y1)>>14])+halfxdimen;
                if (lx < 0) lx = 0;
                rx = mulscale32((nx+nxoff)>>3,distrecip[(ny+y2)>>14])+halfxdimen;
                if (rx > xdimen) rx = xdimen;
                if (rx <= lx) continue;
                rx -= lx;

                l1 = distrecip[(ny-yoff)>>14];
                l2 = distrecip[(ny+yoff)>>14];
                for(;voxptr<voxend;voxptr+=voxptr[1]+3)
                {
                    j = (voxptr[0]<<15)-syoff;
                    if (j < 0)
                    {
                        k = j+(voxptr[1]<<15);
                        if (k < 0)
                        {
                            if ((voxptr[2]&oand32) == 0) continue;
                            z2 = mulscale32(l2,k) + globalhoriz;     //Below slab
                        }
                        else
                        {
                            if ((voxptr[2]&oand) == 0) continue;    //Middle of slab
                            z2 = mulscale32(l1,k) + globalhoriz;
                        }
                        z1 = mulscale32(l1,j) + globalhoriz;
                    }
                    else
                    {
                        if ((voxptr[2]&oand16) == 0) continue;
                        z1 = mulscale32(l2,j) + globalhoriz;        //Above slab
                        z2 = mulscale32(l1,j+(voxptr[1]<<15)) + globalhoriz;
                    }

                    if (voxptr[1] == 1)
                    {
                        yplc = 0; yinc = 0;
                        if (z1 < daumost[lx]) z1 = daumost[lx];
                    }
                    else
                    {
                        if (z2-z1 >= 1024) yinc = divscale16(voxptr[1],z2-z1);
                        else if (z2 > z1) yinc = (lowrecip[z2-z1]*voxptr[1]>>8);
                        if (z1 < daumost[lx]) { yplc = yinc*(daumost[lx]-z1); z1 = daumost[lx]; } else yplc = 0;
                    }
                    if (z2 > dadmost[lx]) z2 = dadmost[lx];
                    z2 -= z1; if (z2 <= 0) continue;

                    drawslab(rx,yplc,z2,yinc,&voxptr[3],ylookup[z1]+lx+frameoffset);
                }
            }
        }
    }
}*/

void ceilspritescan(long x1, long x2){}/*
{
    long x, y1, y2, twall, bwall;

    y1 = uwall[x1];
    y2 = y1;
    for (x = x1; x <= x2; x++)
    {
        twall = uwall[x] - 1;
        bwall = dwall[x];
        if (twall < bwall - 1)
        {
            if (twall >= y2)
            {
                while (y1 < y2 - 1) ceilspritehline(x - 1, ++y1);
                y1 = twall;
            }
            else
            {
                while (y1 < twall) ceilspritehline(x - 1, ++y1);
                while (y1 > twall) lastx[y1--] = x;
            }
            while (y2 > bwall) ceilspritehline(x - 1, --y2);
            while (y2 < bwall) lastx[y2++] = x;
        }
        else
        {
            while (y1 < y2 - 1) ceilspritehline(x - 1, ++y1);
            if (x == x2) break;
            y1 = uwall[x + 1];
            y2 = y1;
        }
    }
    while (y1 < y2 - 1) ceilspritehline(x2, ++y1);
    faketimerhandler();
}*/

void ceilspritehline(long x2, long y){}/*
{
    long x1, v, bx, by;

    //x = x1 + (x2-x1)t + (y1-y2)u  �  x = 160v
    //y = y1 + (y2-y1)t + (x2-x1)u  �  y = (scrx-160)v
    //z = z1 = z2                   �  z = posz + (scry-horiz)v

    x1 = lastx[y];
    if (x2 < x1) return;

    v = mulscale20(globalzd, horizlookup[y-globalhoriz+horizycent]);
    bx = mulscale14(globalx2*x1+globalx1, v) + globalxpanning;
    by = mulscale14(globaly2*x1+globaly1, v) + globalypanning;
    asm1 = mulscale14(globalx2, v);
    asm2 = mulscale14(globaly2, v);

    asm3 = FP_OFF(palookup[globalpal]) + (getpalookup((long)mulscale28(klabs(v), globvis), globalshade) << 8);

    if ((globalorientation & 2) == 0)
        mhline(globalbufplc, bx, (x2 - x1) << 16, 0L, by, ylookup[y] + x1 + frameoffset);
    else
    {
        thline(globalbufplc, bx, (x2 - x1) << 16, 0L, by, ylookup[y] + x1 + frameoffset);
        transarea += (x2 - x1);
    }
}*/

int setsprite(short spritenum, long newx, long newy, long newz)
{
    // move entirely into new engine
    short bad, j, tempsectnum;

    sprite[spritenum].x = newx;
    sprite[spritenum].y = newy;
    sprite[spritenum].z = newz;

    tempsectnum = sprite[spritenum].sectnum;
    bbeng.FindSectorOfPoint(newx, newy, &tempsectnum);
    if (tempsectnum < 0)
        return (-1);
    if (tempsectnum != sprite[spritenum].sectnum)
        changespritesect(spritenum, tempsectnum);

    return (0);
}

long animateoffs(short tilenum, short fakevar)
{
    long i, k, offs;

    offs = 0;
    i = (totalclocklock >> ((picanm[tilenum] >> 24) & 15));
    if ((picanm[tilenum] & 63) > 0)
    {
        switch (picanm[tilenum] & 192)
        {
        case 64:
            k = (i % ((picanm[tilenum] & 63) << 1));
            if (k < (picanm[tilenum] & 63))
                offs = k;
            else
                offs = (((picanm[tilenum] & 63) << 1) - k);
            break;
        case 128:
            offs = (i % ((picanm[tilenum] & 63) + 1));
            break;
        case 192:
            offs = -(i % ((picanm[tilenum] & 63) + 1));
        }
    }
    return (offs);
}

void initspritelists()
{
    long i;

    for (i = 0; i < MAXSECTORS; i++) //Init doubly-linked sprite sector lists
        headspritesect[i] = -1;
    headspritesect[MAXSECTORS] = 0;
    for (i = 0; i < MAXSPRITES; i++)
    {
        prevspritesect[i] = i - 1;
        nextspritesect[i] = i + 1;
        sprite[i].sectnum = MAXSECTORS;
    }
    prevspritesect[0] = -1;
    nextspritesect[MAXSPRITES - 1] = -1;


    for (i = 0; i < MAXSTATUS; i++) //Init doubly-linked sprite status lists
        headspritestat[i] = -1;
    headspritestat[MAXSTATUS] = 0;
    for (i = 0; i < MAXSPRITES; i++)
    {
        prevspritestat[i] = i - 1;
        nextspritestat[i] = i + 1;
        sprite[i].statnum = MAXSTATUS;
    }
    prevspritestat[0] = -1;
    nextspritestat[MAXSPRITES - 1] = -1;
}

int insertsprite(short sectnum, short statnum)
{
    insertspritestat(statnum);
    return (insertspritesect(sectnum));
}

int insertspritesect(short sectnum)
{
    short blanktouse;

    if ((sectnum >= MAXSECTORS) || (headspritesect[MAXSECTORS] == -1))
        return (-1); //list full

    blanktouse = headspritesect[MAXSECTORS];

    headspritesect[MAXSECTORS] = nextspritesect[blanktouse];
    if (headspritesect[MAXSECTORS] >= 0)
        prevspritesect[headspritesect[MAXSECTORS]] = -1;

    prevspritesect[blanktouse] = -1;
    nextspritesect[blanktouse] = headspritesect[sectnum];
    if (headspritesect[sectnum] >= 0)
        prevspritesect[headspritesect[sectnum]] = blanktouse;
    headspritesect[sectnum] = blanktouse;

    sprite[blanktouse].sectnum = sectnum;

    return (blanktouse);
}

int insertspritestat(short statnum)
{
    short blanktouse;

    if ((statnum >= MAXSTATUS) || (headspritestat[MAXSTATUS] == -1))
        return (-1); //list full

    blanktouse = headspritestat[MAXSTATUS];

    headspritestat[MAXSTATUS] = nextspritestat[blanktouse];
    if (headspritestat[MAXSTATUS] >= 0)
        prevspritestat[headspritestat[MAXSTATUS]] = -1;

    prevspritestat[blanktouse] = -1;
    nextspritestat[blanktouse] = headspritestat[statnum];
    if (headspritestat[statnum] >= 0)
        prevspritestat[headspritestat[statnum]] = blanktouse;
    headspritestat[statnum] = blanktouse;

    sprite[blanktouse].statnum = statnum;

    return (blanktouse);
}

int deletesprite(short spritenum)
{
    deletespritestat(spritenum);
    return (deletespritesect(spritenum));
}

int deletespritesect(short deleteme)
{
    if (sprite[deleteme].sectnum == MAXSECTORS)
        return (-1);

    if (headspritesect[sprite[deleteme].sectnum] == deleteme)
        headspritesect[sprite[deleteme].sectnum] = nextspritesect[deleteme];

    if (prevspritesect[deleteme] >= 0) nextspritesect[prevspritesect[deleteme]] = nextspritesect[deleteme];
    if (nextspritesect[deleteme] >= 0) prevspritesect[nextspritesect[deleteme]] = prevspritesect[deleteme];

    if (headspritesect[MAXSECTORS] >= 0) prevspritesect[headspritesect[MAXSECTORS]] = deleteme;
    prevspritesect[deleteme] = -1;
    nextspritesect[deleteme] = headspritesect[MAXSECTORS];
    headspritesect[MAXSECTORS] = deleteme;

    sprite[deleteme].sectnum = MAXSECTORS;
    return (0);
}

int deletespritestat(short deleteme)
{
    if (sprite[deleteme].statnum == MAXSTATUS)
        return (-1);

    if (headspritestat[sprite[deleteme].statnum] == deleteme)
        headspritestat[sprite[deleteme].statnum] = nextspritestat[deleteme];

    if (prevspritestat[deleteme] >= 0) nextspritestat[prevspritestat[deleteme]] = nextspritestat[deleteme];
    if (nextspritestat[deleteme] >= 0) prevspritestat[nextspritestat[deleteme]] = prevspritestat[deleteme];

    if (headspritestat[MAXSTATUS] >= 0) prevspritestat[headspritestat[MAXSTATUS]] = deleteme;
    prevspritestat[deleteme] = -1;
    nextspritestat[deleteme] = headspritestat[MAXSTATUS];
    headspritestat[MAXSTATUS] = deleteme;

    sprite[deleteme].statnum = MAXSTATUS;
    return (0);
}

int changespritesect(short spritenum, short newsectnum)
{
    if ((newsectnum < 0) || (newsectnum > MAXSECTORS)) return (-1);
    if (sprite[spritenum].sectnum == newsectnum) return (0);
    if (sprite[spritenum].sectnum == MAXSECTORS) return (-1);
    if (deletespritesect(spritenum) < 0) return (-1);
    insertspritesect(newsectnum);
    return (0);
}

int changespritestat(short spritenum, short newstatnum)
{
    if ((newstatnum < 0) || (newstatnum > MAXSTATUS)) return (-1);
    if (sprite[spritenum].statnum == newstatnum) return (0);
    if (sprite[spritenum].statnum == MAXSTATUS) return (-1);
    if (deletespritestat(spritenum) < 0) return (-1);
    insertspritestat(newstatnum);
    return (0);
}

// returns sector id.
short nextsectorneighborz(short sectnum, long thez, short topbottom, short direction)
{
    walltype* wal;
    long i, testz, nextz;
    short sectortouse;

    if (direction == 1) nextz = 0x7fffffff;
    else nextz = 0x80000000;

    sectortouse = -1;

    wal = &wall[sector[sectnum].wallptr];
    i = sector[sectnum].wallnum;
    do
    {
        if (wal->nextsector >= 0)
        {
            if (topbottom == 1)
            {
                testz = sector[wal->nextsector].floorz;
                if (direction == 1)
                {
                    if ((testz > thez) && (testz < nextz))
                    {
                        nextz = testz;
                        sectortouse = wal->nextsector;
                    }
                }
                else
                {
                    if ((testz < thez) && (testz > nextz))
                    {
                        nextz = testz;
                        sectortouse = wal->nextsector;
                    }
                }
            }
            else
            {
                testz = sector[wal->nextsector].ceilingz;
                if (direction == 1)
                {
                    if ((testz > thez) && (testz < nextz))
                    {
                        nextz = testz;
                        sectortouse = wal->nextsector;
                    }
                }
                else
                {
                    if ((testz < thez) && (testz > nextz))
                    {
                        nextz = testz;
                        sectortouse = wal->nextsector;
                    }
                }
            }
        }
        wal++;
        i--;
    }
    while (i != 0);

    return (sectortouse);
}

int cansee(long x1, long y1, long z1, short sect1, long x2, long y2, long z2, short sect2)
{
    sectortype* sec;
    walltype *wal, *wal2;
    long i, cnt, nexts, x, y, z, cz, fz, dasectnum, dacnt, danum;
    long x21, y21, z21, x31, y31, x34, y34, bot, t;

    if ((x1 == x2) && (y1 == y2)) return (sect1 == sect2);

    x21 = x2 - x1;
    y21 = y2 - y1;
    z21 = z2 - z1;

    clipsectorlist[0] = sect1;
    danum = 1;
    for (dacnt = 0; dacnt < danum; dacnt++)
    {
        dasectnum = clipsectorlist[dacnt];
        sec = &sector[dasectnum];
        for (cnt = sec->wallnum, wal = &wall[sec->wallptr]; cnt > 0; cnt--, wal++)
        {
            wal2 = &wall[wal->point2];
            x31 = wal->x - x1;
            x34 = wal->x - wal2->x;
            y31 = wal->y - y1;
            y34 = wal->y - wal2->y;

            bot = y21 * x34 - x21 * y34;
            if (bot <= 0) continue;
            t = y21 * x31 - x21 * y31;
            if ((unsigned)t >= (unsigned)bot) continue;
            t = y31 * x34 - x31 * y34;
            if ((unsigned)t >= (unsigned)bot) continue;

            nexts = wal->nextsector;
            if ((nexts < 0) || (wal->cstat & 32)) return (0);

            t = divscale24(t, bot);
            x = x1 + mulscale24(x21, t);
            y = y1 + mulscale24(y21, t);
            z = z1 + mulscale24(z21, t);

            getzsofslope((short)dasectnum, x, y, &cz, &fz);
            if ((z <= cz) || (z >= fz)) return (0);
            getzsofslope((short)nexts, x, y, &cz, &fz);
            if ((z <= cz) || (z >= fz)) return (0);

            for (i = danum - 1; i >= 0; i--) if (clipsectorlist[i] == nexts) break;
            if (i < 0) clipsectorlist[danum++] = nexts;
        }
    }
    for (i = danum - 1; i >= 0; i--) if (clipsectorlist[i] == sect2) return (1);
    return (0);
}

int hitscan(long xs, long ys, long zs, short sectnum, long vx, long vy, long vz, short* hitsect, short* hitwall,
            short* hitsprite, long* hitx, long* hity, long* hitz, unsigned long cliptype)
{
    sectortype* sec;
    walltype *wal, *wal2;
    spritetype* spr;
    long z, zz, x1, y1, z1, x2, y2, z2, x3, y3, x4, y4, intx, inty, intz;
    long topt, topu, bot, dist, offx, offy, cstat;
    long i, j, k, l, tilenum, xoff, yoff, dax, day, daz, daz2;
    long ang, cosang, sinang, xspan, yspan, xrepeat, yrepeat;
    long dawalclipmask, dasprclipmask;
    short tempshortcnt, tempshortnum, dasector, startwall, endwall;
    short nextsector;
    char clipyou;

    *hitsect = -1;
    *hitwall = -1;
    *hitsprite = -1;
    if (sectnum < 0) return (-1);

    *hitx = hitscangoalx;
    *hity = hitscangoaly;
    //dawalclipmask = (cliptype & 65535); for clip0 = 1; for clip1 = 64
    //dasprclipmask = (cliptype >> 16); for clip0 = 1; for clip1 = 256
// 256 for sprite is hitscan
    // and 64 is for wall.

    dawalclipmask = (cliptype & 65535);
    dasprclipmask = (cliptype >> 16);

    clipsectorlist[0] = sectnum;
    tempshortcnt = 0;
    tempshortnum = 1;
    do
    {
        dasector = clipsectorlist[tempshortcnt];
        sec = &sector[dasector];

        x1 = 0x7fffffff;
        if (sec->ceilingstat & 2)
        {
            wal = &wall[sec->wallptr];
            wal2 = &wall[wal->point2];
            dax = wal2->x - wal->x;
            day = wal2->y - wal->y;
            i = nsqrtasm(dax * dax + day * day);
            if (i == 0) continue;
            i = divscale15(sec->ceilingheinum, i);
            dax *= i;
            day *= i;

            j = (vz << 8) - dmulscale15(dax, vy, -day, vx);
            if (j != 0)
            {
                i = ((sec->ceilingz - zs) << 8) + dmulscale15(dax, ys - wal->y, -day, xs - wal->x);
                if (((i ^ j) >= 0) && ((klabs(i) >> 1) < klabs(j)))
                {
                    i = divscale30(i, j);
                    x1 = xs + mulscale30(vx, i);
                    y1 = ys + mulscale30(vy, i);
                    z1 = zs + mulscale30(vz, i);
                }
            }
        }
        else if ((vz < 0) && (zs >= sec->ceilingz))
        {
            z1 = sec->ceilingz;
            i = z1 - zs;
            if ((klabs(i) >> 1) < -vz)
            {
                i = divscale30(i, vz);
                x1 = xs + mulscale30(vx, i);
                y1 = ys + mulscale30(vy, i);
            }
        }
        if ((x1 != 0x7fffffff) && (klabs(x1 - xs) + klabs(y1 - ys) < klabs((*hitx) - xs) + klabs((*hity) - ys)))
            if (inside(x1, y1, dasector) != 0)
            {
                *hitsect = dasector;
                *hitwall = -1;
                *hitsprite = -1;
                *hitx = x1;
                *hity = y1;
                *hitz = z1;
            }

        x1 = 0x7fffffff;
        if (sec->floorstat & 2)
        {
            wal = &wall[sec->wallptr];
            wal2 = &wall[wal->point2];
            dax = wal2->x - wal->x;
            day = wal2->y - wal->y;
            i = nsqrtasm(dax * dax + day * day);
            if (i == 0) continue;
            i = divscale15(sec->floorheinum, i);
            dax *= i;
            day *= i;

            j = (vz << 8) - dmulscale15(dax, vy, -day, vx);
            if (j != 0)
            {
                i = ((sec->floorz - zs) << 8) + dmulscale15(dax, ys - wal->y, -day, xs - wal->x);
                if (((i ^ j) >= 0) && ((klabs(i) >> 1) < klabs(j)))
                {
                    i = divscale30(i, j);
                    x1 = xs + mulscale30(vx, i);
                    y1 = ys + mulscale30(vy, i);
                    z1 = zs + mulscale30(vz, i);
                }
            }
        }
        else if ((vz > 0) && (zs <= sec->floorz))
        {
            z1 = sec->floorz;
            i = z1 - zs;
            if ((klabs(i) >> 1) < vz)
            {
                i = divscale30(i, vz);
                x1 = xs + mulscale30(vx, i);
                y1 = ys + mulscale30(vy, i);
            }
        }
        if ((x1 != 0x7fffffff) && (klabs(x1 - xs) + klabs(y1 - ys) < klabs((*hitx) - xs) + klabs((*hity) - ys)))
            if (inside(x1, y1, dasector) != 0)
            {
                *hitsect = dasector;
                *hitwall = -1;
                *hitsprite = -1;
                *hitx = x1;
                *hity = y1;
                *hitz = z1;
            }

        startwall = sec->wallptr;
        endwall = startwall + sec->wallnum;
        for (z = startwall, wal = &wall[startwall]; z < endwall; z++, wal++)
        {
            wal2 = &wall[wal->point2];
            x1 = wal->x;
            y1 = wal->y;
            x2 = wal2->x;
            y2 = wal2->y;

            if ((x1 - xs) * (y2 - ys) < (x2 - xs) * (y1 - ys)) continue;
            if (rintersect(xs, ys, zs, vx, vy, vz, x1, y1, x2, y2, &intx, &inty, &intz) == 0) continue;

            if (klabs(intx - xs) + klabs(inty - ys) >= klabs((*hitx) - xs) + klabs((*hity) - ys)) continue;

            nextsector = wal->nextsector;
            if ((nextsector < 0) || (wal->cstat & dawalclipmask))
            {
                *hitsect = dasector;
                *hitwall = z;
                *hitsprite = -1;
                *hitx = intx;
                *hity = inty;
                *hitz = intz;
                continue;
            }
            getzsofslope(nextsector, intx, inty, &daz, &daz2);
            if ((intz <= daz) || (intz >= daz2))
            {
                *hitsect = dasector;
                *hitwall = z;
                *hitsprite = -1;
                *hitx = intx;
                *hity = inty;
                *hitz = intz;
                continue;
            }

            for (zz = tempshortnum - 1; zz >= 0; zz--)
                if (clipsectorlist[zz] == nextsector) break;
            if (zz < 0) clipsectorlist[tempshortnum++] = nextsector;
        }

        for (z = headspritesect[dasector]; z >= 0; z = nextspritesect[z])
        {
            spr = &sprite[z];
            cstat = spr->cstat;
            if ((cstat & dasprclipmask) == 0) continue;

            x1 = spr->x;
            y1 = spr->y;
            z1 = spr->z;
            switch (cstat & 48)
            {
            case 0:
                topt = vx * (x1 - xs) + vy * (y1 - ys);
                if (topt <= 0) continue;
                bot = vx * vx + vy * vy;
                if (bot == 0) continue;

                intz = zs + scale(vz, topt, bot);

                i = (tilesizy[spr->picnum] * spr->yrepeat << 2);
                if (cstat & 128) z1 += (i >> 1);
                if (picanm[spr->picnum] & 0x00ff0000) z1 -= ((long)((signed char)((picanm[spr->picnum] >> 16) & 255)) *
                    spr->yrepeat << 2);
                if ((intz > z1) || (intz < z1 - i)) continue;
                topu = vx * (y1 - ys) - vy * (x1 - xs);

                offx = scale(vx, topu, bot);
                offy = scale(vy, topu, bot);
                dist = offx * offx + offy * offy;
                i = tilesizx[spr->picnum] * spr->xrepeat;
                i *= i;
                if (dist > (i >> 7)) continue;
                intx = xs + scale(vx, topt, bot);
                inty = ys + scale(vy, topt, bot);

                if (klabs(intx - xs) + klabs(inty - ys) > klabs((*hitx) - xs) + klabs((*hity) - ys)) continue;

                *hitsect = dasector;
                *hitwall = -1;
                *hitsprite = z;
                *hitx = intx;
                *hity = inty;
                *hitz = intz;
                break;
            case 16:
                //These lines get the 2 points of the rotated sprite
                //Given: (x1, y1) starts out as the center point
                tilenum = spr->picnum;
                xoff = (long)((signed char)((picanm[tilenum] >> 8) & 255)) + ((long)spr->xoffset);
                if ((cstat & 4) > 0) xoff = -xoff;
                k = spr->ang;
                l = spr->xrepeat;
                dax = sintable[k & 2047] * l;
                day = sintable[(k + 1536) & 2047] * l;
                l = tilesizx[tilenum];
                k = (l >> 1) + xoff;
                x1 -= mulscale16(dax, k);
                x2 = x1 + mulscale16(dax, l);
                y1 -= mulscale16(day, k);
                y2 = y1 + mulscale16(day, l);

                if ((cstat & 64) != 0) //back side of 1-way sprite
                    if ((x1 - xs) * (y2 - ys) < (x2 - xs) * (y1 - ys)) continue;

                if (rintersect(xs, ys, zs, vx, vy, vz, x1, y1, x2, y2, &intx, &inty, &intz) == 0) continue;

                if (klabs(intx - xs) + klabs(inty - ys) > klabs((*hitx) - xs) + klabs((*hity) - ys)) continue;

                k = ((tilesizy[spr->picnum] * spr->yrepeat) << 2);
                if (cstat & 128) daz = spr->z + (k >> 1);
                else daz = spr->z;
                if (picanm[spr->picnum] & 0x00ff0000) daz -= ((long)((signed char)((picanm[spr->picnum] >> 16) & 255)) *
                    spr->yrepeat << 2);
                if ((intz < daz) && (intz > daz - k))
                {
                    *hitsect = dasector;
                    *hitwall = -1;
                    *hitsprite = z;
                    *hitx = intx;
                    *hity = inty;
                    *hitz = intz;
                }
                break;
            case 32:
                if (vz == 0) continue;
                intz = z1;
                if (((intz - zs) ^ vz) < 0) continue;
                if ((cstat & 64) != 0)
                    if ((zs > intz) == ((cstat & 8) == 0)) continue;

                intx = xs + scale(intz - zs, vx, vz);
                inty = ys + scale(intz - zs, vy, vz);

                if (klabs(intx - xs) + klabs(inty - ys) > klabs((*hitx) - xs) + klabs((*hity) - ys)) continue;

                tilenum = spr->picnum;
                xoff = (long)((signed char)((picanm[tilenum] >> 8) & 255)) + ((long)spr->xoffset);
                yoff = (long)((signed char)((picanm[tilenum] >> 16) & 255)) + ((long)spr->yoffset);
                if ((cstat & 4) > 0) xoff = -xoff;
                if ((cstat & 8) > 0) yoff = -yoff;

                ang = spr->ang;
                cosang = sintable[(ang + 512) & 2047];
                sinang = sintable[ang];
                xspan = tilesizx[tilenum];
                xrepeat = spr->xrepeat;
                yspan = tilesizy[tilenum];
                yrepeat = spr->yrepeat;

                dax = ((xspan >> 1) + xoff) * xrepeat;
                day = ((yspan >> 1) + yoff) * yrepeat;
                x1 += dmulscale16(sinang, dax, cosang, day) - intx;
                y1 += dmulscale16(sinang, day, -cosang, dax) - inty;
                l = xspan * xrepeat;
                x2 = x1 - mulscale16(sinang, l);
                y2 = y1 + mulscale16(cosang, l);
                l = yspan * yrepeat;
                k = -mulscale16(cosang, l);
                x3 = x2 + k;
                x4 = x1 + k;
                k = -mulscale16(sinang, l);
                y3 = y2 + k;
                y4 = y1 + k;

                clipyou = 0;
                if ((y1 ^ y2) < 0)
                {
                    if ((x1 ^ x2) < 0) clipyou ^= (x1 * y2 < x2 * y1) ^ (y1 < y2);
                    else if (x1 >= 0) clipyou ^= 1;
                }
                if ((y2 ^ y3) < 0)
                {
                    if ((x2 ^ x3) < 0) clipyou ^= (x2 * y3 < x3 * y2) ^ (y2 < y3);
                    else if (x2 >= 0) clipyou ^= 1;
                }
                if ((y3 ^ y4) < 0)
                {
                    if ((x3 ^ x4) < 0) clipyou ^= (x3 * y4 < x4 * y3) ^ (y3 < y4);
                    else if (x3 >= 0) clipyou ^= 1;
                }
                if ((y4 ^ y1) < 0)
                {
                    if ((x4 ^ x1) < 0) clipyou ^= (x4 * y1 < x1 * y4) ^ (y4 < y1);
                    else if (x4 >= 0) clipyou ^= 1;
                }

                if (clipyou != 0)
                {
                    *hitsect = dasector;
                    *hitwall = -1;
                    *hitsprite = z;
                    *hitx = intx;
                    *hity = inty;
                    *hitz = intz;
                }
                break;
            }
        }
        tempshortcnt++;
    }
    while (tempshortcnt < tempshortnum);
    return (0);
}

int neartag(long xs, long ys, long zs, short sectnum, short ange, short* neartagsector, short* neartagwall,
            short* neartagsprite, long* neartaghitdist, long neartagrange, char tagsearch)
{
    walltype *wal, *wal2;
    spritetype* spr;
    long i, z, zz, xe, ye, ze, x1, y1, z1, x2, y2, z2, intx, inty, intz;
    long topt, topu, bot, dist, offx, offy, vx, vy, vz;
    short tempshortcnt, tempshortnum, dasector, startwall, endwall;
    short nextsector, good;

    *neartagsector = -1;
    *neartagwall = -1;
    *neartagsprite = -1;
    *neartaghitdist = 0;

    if (sectnum < 0) return (0);
    if ((tagsearch < 1) || (tagsearch > 3)) return (0);

    vx = mulscale14(sintable[(ange+2560)&2047], neartagrange);
    xe = xs + vx;
    vy = mulscale14(sintable[(ange+2048)&2047], neartagrange);
    ye = ys + vy;
    vz = 0;
    ze = 0;

    clipsectorlist[0] = sectnum;
    tempshortcnt = 0;
    tempshortnum = 1;

    do
    {
        dasector = clipsectorlist[tempshortcnt];

        startwall = sector[dasector].wallptr;
        endwall = startwall + sector[dasector].wallnum - 1;
        for (z = startwall, wal = &wall[startwall]; z <= endwall; z++, wal++)
        {
            wal2 = &wall[wal->point2];
            x1 = wal->x;
            y1 = wal->y;
            x2 = wal2->x;
            y2 = wal2->y;

            nextsector = wal->nextsector;

            good = 0;
            if (nextsector >= 0)
            {
                if ((tagsearch & 1) && sector[nextsector].lotag) good |= 1;
                if ((tagsearch & 2) && sector[nextsector].hitag) good |= 1;
            }
            if ((tagsearch & 1) && wal->lotag) good |= 2;
            if ((tagsearch & 2) && wal->hitag) good |= 2;

            if ((good == 0) && (nextsector < 0)) continue;
            if ((x1 - xs) * (y2 - ys) < (x2 - xs) * (y1 - ys)) continue;

            if (lintersect(xs, ys, zs, xe, ye, ze, x1, y1, x2, y2, &intx, &inty, &intz) == 1)
            {
                if (good != 0)
                {
                    if (good & 1) *neartagsector = nextsector;
                    if (good & 2) *neartagwall = z;
                    *neartaghitdist = dmulscale14(intx - xs, sintable[(ange + 2560) & 2047], inty - ys,
                                                  sintable[(ange + 2048) & 2047]);
                    xe = intx;
                    ye = inty;
                    ze = intz;
                }
                if (nextsector >= 0)
                {
                    for (zz = tempshortnum - 1; zz >= 0; zz--)
                        if (clipsectorlist[zz] == nextsector) break;
                    if (zz < 0) clipsectorlist[tempshortnum++] = nextsector;
                }
            }
        }

        for (z = headspritesect[dasector]; z >= 0; z = nextspritesect[z])
        {
            spr = &sprite[z];

            good = 0;
            if ((tagsearch & 1) && spr->lotag) good |= 1;
            if ((tagsearch & 2) && spr->hitag) good |= 1;
            if (good != 0)
            {
                x1 = spr->x;
                y1 = spr->y;
                z1 = spr->z;

                topt = vx * (x1 - xs) + vy * (y1 - ys);
                if (topt > 0)
                {
                    bot = vx * vx + vy * vy;
                    if (bot != 0)
                    {
                        intz = zs + scale(vz, topt, bot);
                        i = tilesizy[spr->picnum] * spr->yrepeat;
                        if (spr->cstat & 128) z1 += (i << 1);
                        if (picanm[spr->picnum] & 0x00ff0000) z1 -= ((long)((signed char)((picanm[spr->picnum] >> 16) &
                            255)) * spr->yrepeat << 2);
                        if ((intz <= z1) && (intz >= z1 - (i << 2)))
                        {
                            topu = vx * (y1 - ys) - vy * (x1 - xs);

                            offx = scale(vx, topu, bot);
                            offy = scale(vy, topu, bot);
                            dist = offx * offx + offy * offy;
                            i = (tilesizx[spr->picnum] * spr->xrepeat);
                            i *= i;
                            if (dist <= (i >> 7))
                            {
                                intx = xs + scale(vx, topt, bot);
                                inty = ys + scale(vy, topt, bot);
                                if (klabs(intx - xs) + klabs(inty - ys) < klabs(xe - xs) + klabs(ye - ys))
                                {
                                    *neartagsprite = z;
                                    *neartaghitdist = dmulscale14(intx - xs, sintable[(ange + 2560) & 2047], inty - ys,
                                                                  sintable[(ange + 2048) & 2047]);
                                    xe = intx;
                                    ye = inty;
                                    ze = intz;
                                }
                            }
                        }
                    }
                }
            }
        }

        tempshortcnt++;
    }
    while (tempshortcnt < tempshortnum);
    return (0);
}

int lintersect(long x1, long y1, long z1, long x2, long y2, long z2, long x3, long y3, long x4, long y4, long* intx,
               long* inty, long* intz)
{
    //p1 to p2 is a line segment
    long x21, y21, x34, y34, x31, y31, bot, topt, topu, t;

    x21 = x2 - x1;
    x34 = x3 - x4;
    y21 = y2 - y1;
    y34 = y3 - y4;
    bot = x21 * y34 - y21 * x34;
    if (bot >= 0)
    {
        if (bot == 0) return (0);
        x31 = x3 - x1;
        y31 = y3 - y1;
        topt = x31 * y34 - y31 * x34;
        if ((topt < 0) || (topt >= bot)) return (0);
        topu = x21 * y31 - y21 * x31;
        if ((topu < 0) || (topu >= bot)) return (0);
    }
    else
    {
        x31 = x3 - x1;
        y31 = y3 - y1;
        topt = x31 * y34 - y31 * x34;
        if ((topt > 0) || (topt <= bot)) return (0);
        topu = x21 * y31 - y21 * x31;
        if ((topu > 0) || (topu <= bot)) return (0);
    }
    t = divscale24(topt, bot);
    *intx = x1 + mulscale24(x21, t);
    *inty = y1 + mulscale24(y21, t);
    *intz = z1 + mulscale24(z2-z1, t);
    return (1);
}

int rintersect(long x1, long y1, long z1, long vx, long vy, long vz, long x3, long y3, long x4, long y4, long* intx,
               long* inty, long* intz)
{
    //p1 towards p2 is a ray
    long x34, y34, x31, y31, bot, topt, topu, t;

    x34 = x3 - x4;
    y34 = y3 - y4;
    bot = vx * y34 - vy * x34;
    if (bot >= 0)
    {
        if (bot == 0) return (0);
        x31 = x3 - x1;
        y31 = y3 - y1;
        topt = x31 * y34 - y31 * x34;
        if (topt < 0) return (0);
        topu = vx * y31 - vy * x31;
        if ((topu < 0) || (topu >= bot)) return (0);
    }
    else
    {
        x31 = x3 - x1;
        y31 = y3 - y1;
        topt = x31 * y34 - y31 * x34;
        if (topt > 0) return (0);
        topu = vx * y31 - vy * x31;
        if ((topu > 0) || (topu <= bot)) return (0);
    }
    t = divscale16(topt, bot);
    *intx = x1 + mulscale16(vx, t);
    *inty = y1 + mulscale16(vy, t);
    *intz = z1 + mulscale16(vz, t);
    return (1);
}

void dragpoint(short pointhighlight, long dax, long day)
{
    short cnt, tempshort;

    wall[pointhighlight].x = dax;
    wall[pointhighlight].y = day;

    cnt = MAXWALLS;
    tempshort = pointhighlight; //search points CCW
    do
    {
        if (wall[tempshort].nextwall >= 0)
        {
            tempshort = wall[wall[tempshort].nextwall].point2;
            wall[tempshort].x = dax;
            wall[tempshort].y = day;
        }
        else
        {
            tempshort = pointhighlight; //search points CW if not searched all the way around
            do
            {
                if (wall[lastwall(tempshort)].nextwall >= 0)
                {
                    tempshort = wall[lastwall(tempshort)].nextwall;
                    wall[tempshort].x = dax;
                    wall[tempshort].y = day;
                }
                else
                {
                    break;
                }
                cnt--;
            }
            while ((tempshort != pointhighlight) && (cnt > 0));
            break;
        }
        cnt--;
    }
    while ((tempshort != pointhighlight) && (cnt > 0));
}

short lastwall(short point)
{
    long i, j, cnt;

    if ((point > 0) && (wall[point - 1].point2 == point)) return (point - 1);
    i = point;
    cnt = MAXWALLS;
    do
    {
        j = wall[i].point2;
        if (j == point) return (i);
        i = j;
        cnt--;
    }
    while (cnt > 0);
    return (point);
}

long clipmove(long* x, long* y, long* z, short* sectnum, long xvect, long yvect, long walldist, long ceildist,
              long flordist, unsigned long cliptype)
{
    walltype *wal, *wal2;
    spritetype* spr;
    sectortype *sec, *sec2;
    long i, j, templong1, templong2;
    long oxvect, oyvect, goalx, goaly, intx, inty, lx, ly, retval;
    long k, l, clipsectcnt, startwall, endwall, cstat, dasect;
    long x1, y1, x2, y2, cx, cy, rad, xmin, ymin, xmax, ymax, daz, daz2;
    long bsz, dax, day, xoff, yoff, xspan, yspan, cosang, sinang, tilenum;
    long xrepeat, yrepeat, gx, gy, dx, dy, dasprclipmask, dawalclipmask;
    long hitwall, cnt, clipyou;

    if (((xvect | yvect) == 0) || (*sectnum < 0)) return (0);
    retval = 0;

    oxvect = xvect;
    oyvect = yvect;

    goalx = (*x) + (xvect >> 14);
    goaly = (*y) + (yvect >> 14);


    clipnum = 0; // reset clipper

    cx = (((*x) + goalx) >> 1);
    cy = (((*y) + goaly) >> 1);
    //Extra walldist for sprites on sector lines
    gx = goalx - (*x);
    gy = goaly - (*y);
    rad = nsqrtasm(gx * gx + gy * gy) + MAXCLIPDIST + walldist + 8;
    xmin = cx - rad;
    ymin = cy - rad;
    xmax = cx + rad;
    ymax = cy + rad;

    dawalclipmask = (cliptype & 65535); //CLIPMASK0 = 0x00010001
    dasprclipmask = (cliptype >> 16); //CLIPMASK1 = 0x01000040

    clipsectorlist[0] = (*sectnum);
    clipsectcnt = 0;
    clipsectnum = 1;
    do
    {
        dasect = clipsectorlist[clipsectcnt++];
        sectortype tempsec = bbeng.ReadSect(dasect);
        startwall = tempsec.wallptr;
        endwall = startwall + tempsec.wallnum;
        //for (j = startwall, wal = &wall[startwall]; j < endwall; j++, wal++)
        for (j = startwall; j < endwall; j++)
        {
            walltype w = bbeng.ReadWall(j);
            wal = &w;
            walltype w2 = bbeng.ReadWall(wal->point2);
            wal2 = &w2;
            if ((wal->x < xmin) && (wal2->x < xmin)) continue;
            if ((wal->x > xmax) && (wal2->x > xmax)) continue;
            if ((wal->y < ymin) && (wal2->y < ymin)) continue;
            if ((wal->y > ymax) && (wal2->y > ymax)) continue;

            x1 = wal->x;
            y1 = wal->y;
            x2 = wal2->x;
            y2 = wal2->y;

            dx = x2 - x1;
            dy = y2 - y1;
            if (dx * ((*y) - y1) < ((*x) - x1) * dy) continue; //If wall's not facing you

            if (dx > 0) dax = dx * (ymin - y1);
            else dax = dx * (ymax - y1);
            if (dy > 0) day = dy * (xmax - x1);
            else day = dy * (xmin - x1);
            if (dax >= day) continue;

            clipyou = 0;
            if ((wal->nextsector < 0) || (wal->cstat & dawalclipmask)) clipyou = 1;
            else if (editstatus == 0)
            {
                if (rintersect(*x, *y, 0, gx, gy, 0, x1, y1, x2, y2, &dax, &day, &daz) == 0)
                    dax = *x, day = *y;
                daz = bbeng.GetFloorZSloped((short)dasect, dax, day);
                daz2 = bbeng.GetFloorZSloped(wal->nextsector, dax, day);

                sectortype s2 = bbeng.ReadSect(wal->nextsector);
                sec2 = &s2;
                
                if (daz2 < daz - (1 << 8))
                    if ((sec2->floorstat & 1) == 0)
                        if ((*z) >= daz2 - (flordist - 1)) clipyou = 1;
                if (clipyou == 0)
                {
                    daz = bbeng.GetFloorZSloped((short)dasect, dax, day);
                    daz2 = bbeng.GetFloorZSloped(wal->nextsector, dax, day);
                    if (daz2 > daz + (1 << 8))
                        if ((sec2->ceilingstat & 1) == 0)
                            if ((*z) <= daz2 + (ceildist - 1)) clipyou = 1;
                }
            }

            if (clipyou)
            {
                //Add 2 boxes at endpoints
                bsz = walldist;
                if (gx < 0) bsz = -bsz;
                addclipline(x1-bsz, y1-bsz, x1-bsz, y1+bsz, (short)j+32768);
                addclipline(x2-bsz, y2-bsz, x2-bsz, y2+bsz, (short)j+32768);
                bsz = walldist;
                if (gy < 0) bsz = -bsz;
                addclipline(x1+bsz, y1-bsz, x1-bsz, y1-bsz, (short)j+32768);
                addclipline(x2+bsz, y2-bsz, x2-bsz, y2-bsz, (short)j+32768);

                dax = walldist;
                if (dy > 0) dax = -dax;
                day = walldist;
                if (dx < 0) day = -day;
                addclipline(x1+dax, y1+day, x2+dax, y2+day, (short)j+32768);
            }
            else
            {
                for (i = clipsectnum - 1; i >= 0; i--)
                    if (wal->nextsector == clipsectorlist[i]) break;
                if (i < 0) clipsectorlist[clipsectnum++] = wal->nextsector;
            }
        }

        for (j = headspritesect[dasect]; j >= 0; j = nextspritesect[j])
        {
            spritetype s = bbeng.ReadSprite(j);
            spr = &s;
            cstat = spr->cstat;
            if ((cstat & dasprclipmask) == 0) continue;
            x1 = spr->x;
            y1 = spr->y;
            switch (cstat & 48)
            {
            case 0:
                if ((x1 >= xmin) && (x1 <= xmax) && (y1 >= ymin) && (y1 <= ymax))
                {
                    k = ((tilesizy[spr->picnum] * spr->yrepeat) << 2);
                    if (cstat & 128) daz = spr->z + (k >> 1);
                    else daz = spr->z;
                    if (picanm[spr->picnum] & 0x00ff0000) daz -= ((long)((signed char)((picanm[spr->picnum] >> 16) &
                        255)) * spr->yrepeat << 2);
                    if (((*z) < daz + ceildist) && ((*z) > daz - k - flordist))
                    {
                        bsz = (spr->clipdist << 2) + walldist;
                        if (gx < 0) bsz = -bsz;
                        addclipline(x1-bsz, y1-bsz, x1-bsz, y1+bsz, (short)j+49152);
                        bsz = (spr->clipdist << 2) + walldist;
                        if (gy < 0) bsz = -bsz;
                        addclipline(x1+bsz, y1-bsz, x1-bsz, y1-bsz, (short)j+49152);
                    }
                }
                break;
            case 16:
                k = ((tilesizy[spr->picnum] * spr->yrepeat) << 2);
                if (cstat & 128) daz = spr->z + (k >> 1);
                else daz = spr->z;
                if (picanm[spr->picnum] & 0x00ff0000) daz -= ((long)((signed char)((picanm[spr->picnum] >> 16) & 255)) *
                    spr->yrepeat << 2);
                daz2 = daz - k;
                daz += ceildist;
                daz2 -= flordist;
                if (((*z) < daz) && ((*z) > daz2))
                {
                    //These lines get the 2 points of the rotated sprite
                    //Given: (x1, y1) starts out as the center point
                    tilenum = spr->picnum;
                    xoff = (long)((signed char)((picanm[tilenum] >> 8) & 255)) + ((long)spr->xoffset);
                    if ((cstat & 4) > 0) xoff = -xoff;
                    k = spr->ang;
                    l = spr->xrepeat;
                    dax = sintable[k & 2047] * l;
                    day = sintable[(k + 1536) & 2047] * l;
                    l = tilesizx[tilenum];
                    k = (l >> 1) + xoff;
                    x1 -= mulscale16(dax, k);
                    x2 = x1 + mulscale16(dax, l);
                    y1 -= mulscale16(day, k);
                    y2 = y1 + mulscale16(day, l);
                    if (clipinsideboxline(cx, cy, x1, y1, x2, y2, rad) != 0)
                    {
                        dax = mulscale14(sintable[(spr->ang+256+512)&2047], walldist);
                        day = mulscale14(sintable[(spr->ang+256)&2047], walldist);

                        if ((x1 - (*x)) * (y2 - (*y)) >= (x2 - (*x)) * (y1 - (*y))) //Front
                        {
                            addclipline(x1+dax, y1+day, x2+day, y2-dax, (short)j+49152);
                        }
                        else
                        {
                            if ((cstat & 64) != 0) continue;
                            addclipline(x2-dax, y2-day, x1-day, y1+dax, (short)j+49152);
                        }

                        //Side blocker
                        if ((x2 - x1) * ((*x) - x1) + (y2 - y1) * ((*y) - y1) < 0)
                        {
                            addclipline(x1-day, y1+dax, x1+dax, y1+day, (short)j+49152);
                        }
                        else if ((x1 - x2) * ((*x) - x2) + (y1 - y2) * ((*y) - y2) < 0)
                        {
                            addclipline(x2+day, y2-dax, x2-dax, y2-day, (short)j+49152);
                        }
                    }
                }
                break;
            case 32:
                daz = spr->z + ceildist;
                daz2 = spr->z - flordist;
                if (((*z) < daz) && ((*z) > daz2))
                {
                    if ((cstat & 64) != 0)
                        if (((*z) > spr->z) == ((cstat & 8) == 0)) continue;

                    tilenum = spr->picnum;
                    xoff = (long)((signed char)((picanm[tilenum] >> 8) & 255)) + ((long)spr->xoffset);
                    yoff = (long)((signed char)((picanm[tilenum] >> 16) & 255)) + ((long)spr->yoffset);
                    if ((cstat & 4) > 0) xoff = -xoff;
                    if ((cstat & 8) > 0) yoff = -yoff;

                    k = spr->ang;
                    cosang = sintable[(k + 512) & 2047];
                    sinang = sintable[k];
                    xspan = tilesizx[tilenum];
                    xrepeat = spr->xrepeat;
                    yspan = tilesizy[tilenum];
                    yrepeat = spr->yrepeat;

                    dax = ((xspan >> 1) + xoff) * xrepeat;
                    day = ((yspan >> 1) + yoff) * yrepeat;
                    rxi[0] = x1 + dmulscale16(sinang, dax, cosang, day);
                    ryi[0] = y1 + dmulscale16(sinang, day, -cosang, dax);
                    l = xspan * xrepeat;
                    rxi[1] = rxi[0] - mulscale16(sinang, l);
                    ryi[1] = ryi[0] + mulscale16(cosang, l);
                    l = yspan * yrepeat;
                    k = -mulscale16(cosang, l);
                    rxi[2] = rxi[1] + k;
                    rxi[3] = rxi[0] + k;
                    k = -mulscale16(sinang, l);
                    ryi[2] = ryi[1] + k;
                    ryi[3] = ryi[0] + k;

                    dax = mulscale14(sintable[(spr->ang-256+512)&2047], walldist);
                    day = mulscale14(sintable[(spr->ang-256)&2047], walldist);

                    if ((rxi[0] - (*x)) * (ryi[1] - (*y)) < (rxi[1] - (*x)) * (ryi[0] - (*y)))
                    {
                        if (clipinsideboxline(cx, cy, rxi[1], ryi[1], rxi[0], ryi[0], rad) != 0)
                            addclipline(rxi[1]-day, ryi[1]+dax, rxi[0]+dax, ryi[0]+day, (short)j+49152);
                    }
                    else if ((rxi[2] - (*x)) * (ryi[3] - (*y)) < (rxi[3] - (*x)) * (ryi[2] - (*y)))
                    {
                        if (clipinsideboxline(cx, cy, rxi[3], ryi[3], rxi[2], ryi[2], rad) != 0)
                            addclipline(rxi[3]+day, ryi[3]-dax, rxi[2]-dax, ryi[2]-day, (short)j+49152);
                    }

                    if ((rxi[1] - (*x)) * (ryi[2] - (*y)) < (rxi[2] - (*x)) * (ryi[1] - (*y)))
                    {
                        if (clipinsideboxline(cx, cy, rxi[2], ryi[2], rxi[1], ryi[1], rad) != 0)
                            addclipline(rxi[2]-dax, ryi[2]-day, rxi[1]-day, ryi[1]+dax, (short)j+49152);
                    }
                    else if ((rxi[3] - (*x)) * (ryi[0] - (*y)) < (rxi[0] - (*x)) * (ryi[3] - (*y)))
                    {
                        if (clipinsideboxline(cx, cy, rxi[0], ryi[0], rxi[3], ryi[3], rad) != 0)
                            addclipline(rxi[0]+dax, ryi[0]+day, rxi[3]+day, ryi[3]-dax, (short)j+49152);
                    }
                }
                break;
            }
        }
    }
    while (clipsectcnt < clipsectnum);


    hitwall = 0;
    cnt = clipmoveboxtracenum;
    do
    {
        intx = goalx;
        inty = goaly;
        if ((hitwall = raytrace(*x, *y, &intx, &inty)) >= 0)
        {
            lx = clipit[hitwall].x2 - clipit[hitwall].x1;
            ly = clipit[hitwall].y2 - clipit[hitwall].y1;
            templong2 = lx * lx + ly * ly;
            if (templong2 > 0)
            {
                templong1 = (goalx - intx) * lx + (goaly - inty) * ly;

                if ((klabs(templong1) >> 11) < templong2)
                    i = divscale20(templong1, templong2);
                else
                    i = 0;
                goalx = mulscale20(lx, i) + intx;
                goaly = mulscale20(ly, i) + inty;
            }

            templong1 = dmulscale6(lx, oxvect, ly, oyvect);
            for (i = cnt + 1; i <= clipmoveboxtracenum; i++)
            {
                j = hitwalls[i];
                templong2 = dmulscale6(clipit[j].x2 - clipit[j].x1, oxvect, clipit[j].y2 - clipit[j].y1, oyvect);
                if ((templong1 ^ templong2) < 0)
                {
                    bbeng.FindSectorOfPoint(*x, *y, sectnum);
                    return (retval);
                }
            }

            keepaway(&goalx, &goaly, hitwall);
            xvect = ((goalx - intx) << 14);
            yvect = ((goaly - inty) << 14);

            if (cnt == clipmoveboxtracenum) retval = clipobjectval[hitwall];
            hitwalls[cnt] = hitwall;
        }
        cnt--;

        *x = intx;
        *y = inty;
    }
    while (((xvect | yvect) != 0) && (hitwall >= 0) && (cnt > 0));

    for (j = 0; j < clipsectnum; j++)
        if (inside(*x, *y, clipsectorlist[j]) == 1)
        {
            *sectnum = clipsectorlist[j];
            return (retval);
        }

    *sectnum = -1;
    templong1 = 0x7fffffff;
    for (j = numsectors - 1; j >= 0; j--)
        if (inside(*x, *y, j) == 1)
        {
            if (sector[j].ceilingstat & 2)
                templong2 = (bbeng.GetFloorZSloped((short)j, *x, *y) - (*z));
            else
                templong2 = (sector[j].ceilingz - (*z));

            if (templong2 > 0)
            {
                if (templong2 < templong1)
                {
                    *sectnum = j;
                    templong1 = templong2;
                }
            }
            else
            {
                if (sector[j].floorstat & 2)
                    templong2 = ((*z) - getflorzofslope((short)j, *x, *y));
                else
                    templong2 = ((*z) - sector[j].floorz);

                if (templong2 <= 0)
                {
                    *sectnum = j;
                    return (retval);
                }
                if (templong2 < templong1)
                {
                    *sectnum = j;
                    templong1 = templong2;
                }
            }
        }

    return (retval);
}

void keepaway(long* x, long* y, long w)
{
    long dx, dy, ox, oy, x1, y1;
    char first;

    x1 = clipit[w].x1;
    dx = clipit[w].x2 - x1;
    y1 = clipit[w].y1;
    dy = clipit[w].y2 - y1;
    ox = ksgn(-dy);
    oy = ksgn(dx);
    first = (klabs(dx) <= klabs(dy));
    while (1)
    {
        if (dx * (*y - y1) > (*x - x1) * dy) return;
        if (first == 0) *x += ox;
        else *y += oy;
        first ^= 1;
    }
}

long raytrace(long x3, long y3, long* x4, long* y4)
{
    long x1, y1, x2, y2, t, bot, topu, nintx, ninty, cnt, z, hitwall;
    long x21, y21, x43, y43;

    hitwall = -1;
    for (z = clipnum - 1; z >= 0; z--)
    {
        x1 = clipit[z].x1;
        x2 = clipit[z].x2;
        x21 = x2 - x1;
        y1 = clipit[z].y1;
        y2 = clipit[z].y2;
        y21 = y2 - y1;

        topu = x21 * (y3 - y1) - (x3 - x1) * y21;
        if (topu <= 0) continue;
        if (x21 * (*y4 - y1) > (*x4 - x1) * y21) continue;
        x43 = *x4 - x3;
        y43 = *y4 - y3;
        if (x43 * (y1 - y3) > (x1 - x3) * y43) continue;
        if (x43 * (y2 - y3) <= (x2 - x3) * y43) continue;
        bot = x43 * y21 - x21 * y43;
        if (bot == 0) continue;

        cnt = 256;
        do
        {
            cnt--;
            if (cnt < 0)
            {
                *x4 = x3;
                *y4 = y3;
                return (z);
            }
            nintx = x3 + scale(x43, topu, bot);
            ninty = y3 + scale(y43, topu, bot);
            topu--;
        }
        while (x21 * (ninty - y1) <= (nintx - x1) * y21);

        if (klabs(x3 - nintx) + klabs(y3 - ninty) < klabs(x3 - *x4) + klabs(y3 - *y4))
        {
            *x4 = nintx;
            *y4 = ninty;
            hitwall = z;
        }
    }
    return (hitwall);
}

long pushmove(long* x, long* y, long* z, short* sectnum, long walldist, long ceildist, long flordist,
              unsigned long cliptype)
{
    sectortype *sec, *sec2;
    walltype *wal, *wal2;
    spritetype* spr;
    long i, j, k, t, dx, dy, dax, day, daz, daz2, bad, dir;
    long dasprclipmask, dawalclipmask;
    short startwall, endwall, clipsectcnt;
    char bad2;

    if ((*sectnum) < 0) return (-1);

    dawalclipmask = (cliptype & 65535);
    dasprclipmask = (cliptype >> 16);

    k = 32;
    dir = 1;
    do
    {
        bad = 0;

        clipsectorlist[0] = *sectnum;
        clipsectcnt = 0;
        clipsectnum = 1;
        do
        {
            /*Push FACE sprites
			for(i=headspritesect[clipsectorlist[clipsectcnt]];i>=0;i=nextspritesect[i])
			{
				spr = &sprite[i];
				if (((spr->cstat&48) != 0) && ((spr->cstat&48) != 48)) continue;
				if ((spr->cstat&dasprclipmask) == 0) continue;

				dax = (*x)-spr->x; day = (*y)-spr->y;
				t = (spr->clipdist<<2)+walldist;
				if ((klabs(dax) < t) && (klabs(day) < t))
				{
					t = ((tilesizy[spr->picnum]*spr->yrepeat)<<2);
					if (spr->cstat&128) daz = spr->z+(t>>1); else daz = spr->z;
					if (picanm[spr->picnum]&0x00ff0000) daz -= ((long)((signed char)((picanm[spr->picnum]>>16)&255))*spr->yrepeat<<2);
					if (((*z) < daz+ceildist) && ((*z) > daz-t-flordist))
					{
						t = (spr->clipdist<<2)+walldist;

						j = getangle(dax,day);
						dx = (sintable[(j+512)&2047]>>11);
						dy = (sintable[(j)&2047]>>11);
						bad2 = 16;
						do
						{
							*x = (*x) + dx; *y = (*y) + dy;
							bad2--; if (bad2 == 0) break;
						} while ((klabs((*x)-spr->x) < t) && (klabs((*y)-spr->y) < t));
						bad = -1;
						k--; if (k <= 0) return(bad);
						bbeng.FindSectorOfPoint(*x,*y,sectnum);
					}
				}
			}*/

            sec = &sector[clipsectorlist[clipsectcnt]];
            if (dir > 0)
                startwall = sec->wallptr, endwall = startwall + sec->wallnum;
            else
                endwall = sec->wallptr, startwall = endwall + sec->wallnum;

            for (i = startwall, wal = &wall[startwall]; i != endwall; i += dir, wal += dir)
                if (clipinsidebox(*x, *y, i, walldist - 4) == 1)
                {
                    j = 0;
                    if (wal->nextsector < 0) j = 1;
                    if (wal->cstat & dawalclipmask) j = 1;
                    if (j == 0)
                    {
                        sec2 = &sector[wal->nextsector];


                        //Find closest point on wall (dax, day) to (*x, *y)
                        dax = wall[wal->point2].x - wal->x;
                        day = wall[wal->point2].y - wal->y;
                        daz = dax * ((*x) - wal->x) + day * ((*y) - wal->y);
                        if (daz <= 0)
                            t = 0;
                        else
                        {
                            daz2 = dax * dax + day * day;
                            if (daz >= daz2) t = (1 << 30);
                            else t = divscale30(daz, daz2);
                        }
                        dax = wal->x + mulscale30(dax, t);
                        day = wal->y + mulscale30(day, t);


                        daz = getflorzofslope(clipsectorlist[clipsectcnt], dax, day);
                        daz2 = getflorzofslope(wal->nextsector, dax, day);
                        if ((daz2 < daz - (1 << 8)) && ((sec2->floorstat & 1) == 0))
                            if (*z >= daz2 - (flordist - 1)) j = 1;

                        daz = bbeng.GetFloorZSloped(clipsectorlist[clipsectcnt], dax, day);
                        daz2 = bbeng.GetFloorZSloped(wal->nextsector, dax, day);
                        if ((daz2 > daz + (1 << 8)) && ((sec2->ceilingstat & 1) == 0))
                            if (*z <= daz2 + (ceildist - 1)) j = 1;
                    }
                    if (j != 0)
                    {
                        j = getangle(wall[wal->point2].x - wal->x, wall[wal->point2].y - wal->y);
                        dx = (sintable[(j + 1024) & 2047] >> 11);
                        dy = (sintable[(j + 512) & 2047] >> 11);
                        bad2 = 16;
                        do
                        {
                            *x = (*x) + dx;
                            *y = (*y) + dy;
                            bad2--;
                            if (bad2 == 0) break;
                        }
                        while (clipinsidebox(*x, *y, i, walldist - 4) != 0);
                        bad = -1;
                        k--;
                        if (k <= 0) return (bad);
                        bbeng.FindSectorOfPoint(*x, *y, sectnum);
                    }
                    else
                    {
                        for (j = clipsectnum - 1; j >= 0; j--)
                            if (wal->nextsector == clipsectorlist[j]) break;
                        if (j < 0) clipsectorlist[clipsectnum++] = wal->nextsector;
                    }
                }

            clipsectcnt++;
        }
        while (clipsectcnt < clipsectnum);
        dir = -dir;
    }
    while (bad != 0);

    return (bad);
}

// gets valid sector at position. assumes that most of the time it is already in sectnum, otherwise - scan nearby, and then scan all
void bbeng.FindSectorOfPoint(long x, long y, short* sectnum)
{
    walltype* wal;
    long i, j;

    if (inside(x, y, *sectnum) == 1) return;

    if ((*sectnum >= 0) && (*sectnum < numsectors))
    {
        wal = &wall[sector[*sectnum].wallptr];
        j = sector[*sectnum].wallnum;
        do
        {
            i = wal->nextsector;
            if (i >= 0)
                if (inside(x, y, (short)i) == 1)
                {
                    *sectnum = i;
                    return;
                }
            wal++;
            j--;
        }
        while (j != 0);
    }

    for (i = numsectors - 1; i >= 0; i--)
        if (inside(x, y, (short)i) == 1)
        {
            *sectnum = i;
            return;
        }

    *sectnum = -1;
}

// internal usage
void rotatepoint(long xpivot, long ypivot, long x, long y, short daang, long* x2, long* y2)
{
    long dacos, dasin;

    dacos = sintable[(daang + 2560) & 2047];
    dasin = sintable[(daang + 2048) & 2047];
    x -= xpivot;
    y -= ypivot;
    *x2 = dmulscale14(x, dacos, -y, dasin) + xpivot;
    *y2 = dmulscale14(y, dacos, x, dasin) + ypivot;
}

void initmouse()
{
    //  return(moustat = setupmouse());
}

void getmousevalues(short* mousx, short* mousy, short* bstatus)
{
    // Get from RL.
    // if (moustat == 0) { *mousx = 0; *mousy = 0; *bstatus = 0; return; }
    // readmousexy(mousx,mousy);
    // readmousebstatus(bstatus);
}

void printscreeninterrupt()
{
    //  int5();
}

void drawline256(long x1, long y1, long x2, long y2, char col)
{
    // reimpl.
}
/*
{
    long dx, dy, i, j, p, inc, plc, daend;

    col = palookup[0][col];

    dx = x2-x1; dy = y2-y1;
    if (dx >= 0)
    {
        if ((x1 >= wx2) || (x2 < wx1)) return;
        if (x1 < wx1) y1 += scale(wx1-x1,dy,dx), x1 = wx1;
        if (x2 > wx2) y2 += scale(wx2-x2,dy,dx), x2 = wx2;
    }
    else
    {
        if ((x2 >= wx2) || (x1 < wx1)) return;
        if (x2 < wx1) y2 += scale(wx1-x2,dy,dx), x2 = wx1;
        if (x1 > wx2) y1 += scale(wx2-x1,dy,dx), x1 = wx2;
    }
    if (dy >= 0)
    {
        if ((y1 >= wy2) || (y2 < wy1)) return;
        if (y1 < wy1) x1 += scale(wy1-y1,dx,dy), y1 = wy1;
        if (y2 > wy2) x2 += scale(wy2-y2,dx,dy), y2 = wy2;
    }
    else
    {
        if ((y2 >= wy2) || (y1 < wy1)) return;
        if (y2 < wy1) x2 += scale(wy1-y2,dx,dy), y2 = wy1;
        if (y1 > wy2) x1 += scale(wy2-y1,dx,dy), y1 = wy2;
    }

    if (klabs(dx) >= klabs(dy))
    {
        if (dx == 0) return;
        if (dx < 0)
        {
            i = x1; x1 = x2; x2 = i;
            i = y1; y1 = y2; y2 = i;
        }

        inc = divscale12(dy,dx);
        plc = y1+mulscale12((2047-x1)&4095,inc);
        i = ((x1+2048)>>12); daend = ((x2+2048)>>12);
        for(;i<daend;i++)
        {
            j = (plc>>12);
            if ((j >= startumost[i]) && (j < startdmost[i]))
                drawpixel(ylookup[j]+i+frameplace,col);
            plc += inc;
        }
    }
    else
    {
        if (dy < 0)
        {
            i = x1; x1 = x2; x2 = i;
            i = y1; y1 = y2; y2 = i;
        }

        inc = divscale12(dx,dy);
        plc = x1+mulscale12((2047-y1)&4095,inc);
        i = ((y1+2048)>>12); daend = ((y2+2048)>>12);
        p = ylookup[i]+frameplace;
        for(;i<daend;i++)
        {
            j = (plc>>12);
            if ((i >= startumost[j]) && (i < startdmost[j]))
                drawpixel(j+p,col);
            plc += inc; p += ylookup[1];
        }
    }
}*/

void drawline16(long x1, long y1, long x2, long y2, char col)
{
    // DO IMP.
}

/*
{
    long i, dx, dy, p, pinc, d;
    char lmask, rmask;

    dx = x2-x1; dy = y2-y1;
    if (dx >= 0)
    {
        if ((x1 > 639) || (x2 < 0)) return;
        if (x1 < 0) { if (dy) y1 += scale(0-x1,dy,dx); x1 = 0; }
        if (x2 > 639) { if (dy) y2 += scale(639-x2,dy,dx); x2 = 639; }
    }
    else
    {
        if ((x2 > 639) || (x1 < 0)) return;
        if (x2 < 0) { if (dy) y2 += scale(0-x2,dy,dx); x2 = 0; }
        if (x1 > 639) { if (dy) y1 += scale(639-x1,dy,dx); x1 = 639; }
    }
    if (dy >= 0)
    {
        if ((y1 >= ydim16) || (y2 < 0)) return;
        if (y1 < 0) { if (dx) x1 += scale(0-y1,dx,dy); y1 = 0; }
        if (y2 >= ydim16) { if (dx) x2 += scale(ydim16-1-y2,dx,dy); y2 = ydim16-1; }
    }
    else
    {
        if ((y2 >= ydim16) || (y1 < 0)) return;
        if (y2 < 0) { if (dx) x2 += scale(0-y2,dx,dy); y2 = 0; }
        if (y1 >= ydim16) { if (dx) x1 += scale(ydim16-1-y1,dx,dy); y1 = ydim16-1; }
    }

    setcolor16((long)col);
    if (x1 == x2)
    {
        if (y2 < y1) i = y1, y1 = y2, y2 = i;
        koutpw(0x3ce,0x8+(256<<(x1&7^7)));  //bit mask
        vlin16((((mul5(y1)<<7)+x1+pageoffset)>>3)+0xa0000,y2-y1+1);
        return;
    }
    if (y1 == y2)
    {
        if (x2 < x1) i = x1, x1 = x2, x2 = i;
        lmask = (0x00ff>>(x1&7));
        rmask = (0xff80>>(x2&7));

        p = (((mul5(y1)<<7)+x1+pageoffset)>>3)+0xa0000;

        dx = (x2>>3)-(x1>>3);
        if (dx == 0)
        {
            koutpw(0x3ce,0x8+((lmask&rmask)<<8)); drawpixel(p,readpixel(p));
            return;
        }

        dx--;

        koutpw(0x3ce,0x8+(lmask<<8)); drawpixel(p,readpixel(p)); p++;
        if (dx > 0) { koutp(0x3cf,0xff); clearbufbyte(p,dx,0L); p += dx; }
        koutp(0x3cf,rmask); drawpixel(p,readpixel(p));
        return;
    }

    dx = klabs(x2-x1)+1; dy = klabs(y2-y1)+1;
    if (dx >= dy)
    {
        if (x2 < x1)
        {
            i = x1; x1 = x2; x2 = i;
            i = y1; y1 = y2; y2 = i;
        }
        p = (mul5(y1)<<7)+x1+pageoffset;
        d = 0;
        if (y2 > y1) pinc = 640; else pinc = -640;
        for(i=dx;i>0;i--)
        {
            drawpixel16(p);
            d += dy;
            if (d >= dx) { d -= dx; p += pinc; }
            p++;
        }
        return;
    }

    if (y2 < y1)
    {
        i = x1; x1 = x2; x2 = i;
        i = y1; y1 = y2; y2 = i;
    }
    p = (mul5(y1)<<7)+x1+pageoffset;
    d = 0;
    if (x2 > x1) pinc = 1; else pinc = -1;
    for(i=dy;i>0;i--)
    {
        drawpixel16(p);
        d += dx;
        if (d >= dy) { d -= dy; p += pinc; }
        p += 640;
    }
}*/

void printext256(long xpos, long ypos, short col, short backcol, char name[82], char fontsize)
{
    long stx, i, x, y, charxsiz;
    char *fontptr, *letptr, *ptr;

    stx = xpos;

    if (fontsize)
    {
        fontptr = smalltextfont;
        charxsiz = 4;
    }
    else
    {
        fontptr = textfont;
        charxsiz = 8;
    }

    for (i = 0; name[i]; i++)
    {
        letptr = &fontptr[name[i] << 3];
        ptr = (char*)(ylookup[ypos + 7] + (stx - fontsize) + frameplace);
        for (y = 7; y >= 0; y--)
        {
            for (x = charxsiz - 1; x >= 0; x--)
            {
                if (letptr[y] & pow2char[7 - fontsize - x])
                    ptr[x] = (char)col;
                else if (backcol >= 0)
                    ptr[x] = (char)backcol;
            }
            ptr -= ylookup[1];
        }
        stx += charxsiz;
    }
}

int krand()
{
    randomseed = (randomseed * 27584621) + 1;
    return (((unsigned long)randomseed) >> 16);
}

void getzrange(long x, long y, long z, short sectnum, long* ceilz, long* ceilhit, long* florz, long* florhit,
               long walldist, unsigned long cliptype)
{
    sectortype* sec;
    walltype *wal, *wal2;
    spritetype* spr;
    long clipsectcnt, startwall, endwall, tilenum, xoff, yoff, dax, day;
    long xmin, ymin, xmax, ymax, i, j, k, l, daz, daz2, dx, dy;
    long x1, y1, x2, y2, x3, y3, x4, y4, ang, cosang, sinang;
    long xspan, yspan, xrepeat, yrepeat, dasprclipmask, dawalclipmask;
    short cstat;
    char bad, clipyou;

    if (sectnum < 0)
    {
        *ceilz = 0x80000000;
        *ceilhit = -1;
        *florz = 0x7fffffff;
        *florhit = -1;
        return;
    }

    //Extra walldist for sprites on sector lines
    i = walldist + MAXCLIPDIST + 1;
    xmin = x - i;
    ymin = y - i;
    xmax = x + i;
    ymax = y + i;

    getzsofslope(sectnum, x, y, ceilz, florz);
    *ceilhit = sectnum + 16384;
    *florhit = sectnum + 16384;

    dawalclipmask = (cliptype & 65535);
    dasprclipmask = (cliptype >> 16);

    clipsectorlist[0] = sectnum;
    clipsectcnt = 0;
    clipsectnum = 1;

    do //Collect sectors inside your square first
    {
        sec = &sector[clipsectorlist[clipsectcnt]];
        startwall = sec->wallptr;
        endwall = startwall + sec->wallnum;
        for (j = startwall, wal = &wall[startwall]; j < endwall; j++, wal++)
        {
            k = wal->nextsector;
            if (k >= 0)
            {
                wal2 = &wall[wal->point2];
                x1 = wal->x;
                x2 = wal2->x;
                if ((x1 < xmin) && (x2 < xmin)) continue;
                if ((x1 > xmax) && (x2 > xmax)) continue;
                y1 = wal->y;
                y2 = wal2->y;
                if ((y1 < ymin) && (y2 < ymin)) continue;
                if ((y1 > ymax) && (y2 > ymax)) continue;

                dx = x2 - x1;
                dy = y2 - y1;
                if (dx * (y - y1) < (x - x1) * dy) continue; //back
                if (dx > 0) dax = dx * (ymin - y1);
                else dax = dx * (ymax - y1);
                if (dy > 0) day = dy * (xmax - x1);
                else day = dy * (xmin - x1);
                if (dax >= day) continue;

                if (wal->cstat & dawalclipmask) continue;
                sec = &sector[k];
                if (editstatus == 0)
                {
                    if (((sec->ceilingstat & 1) == 0) && (z <= sec->ceilingz + (3 << 8))) continue;
                    if (((sec->floorstat & 1) == 0) && (z >= sec->floorz - (3 << 8))) continue;
                }

                for (i = clipsectnum - 1; i >= 0; i--) if (clipsectorlist[i] == k) break;
                if (i < 0) clipsectorlist[clipsectnum++] = k;

                if ((x1 < xmin + MAXCLIPDIST) && (x2 < xmin + MAXCLIPDIST)) continue;
                if ((x1 > xmax - MAXCLIPDIST) && (x2 > xmax - MAXCLIPDIST)) continue;
                if ((y1 < ymin + MAXCLIPDIST) && (y2 < ymin + MAXCLIPDIST)) continue;
                if ((y1 > ymax - MAXCLIPDIST) && (y2 > ymax - MAXCLIPDIST)) continue;
                if (dx > 0) dax += dx * MAXCLIPDIST;
                else dax -= dx * MAXCLIPDIST;
                if (dy > 0) day -= dy * MAXCLIPDIST;
                else day += dy * MAXCLIPDIST;
                if (dax >= day) continue;

                //It actually got here, through all the continue's!!!
                getzsofslope((short)k, x, y, &daz, &daz2);
                if (daz > *ceilz)
                {
                    *ceilz = daz;
                    *ceilhit = k + 16384;
                }
                if (daz2 < *florz)
                {
                    *florz = daz2;
                    *florhit = k + 16384;
                }
            }
        }
        clipsectcnt++;
    }
    while (clipsectcnt < clipsectnum);

    for (i = 0; i < clipsectnum; i++)
    {
        for (j = headspritesect[clipsectorlist[i]]; j >= 0; j = nextspritesect[j])
        {
            spr = &sprite[j];
            cstat = spr->cstat;
            if (cstat & dasprclipmask)
            {
                x1 = spr->x;
                y1 = spr->y;

                clipyou = 0;
                switch (cstat & 48)
                {
                case 0:
                    k = walldist + (spr->clipdist << 2) + 1;
                    if ((klabs(x1 - x) <= k) && (klabs(y1 - y) <= k))
                    {
                        daz = spr->z;
                        k = ((tilesizy[spr->picnum] * spr->yrepeat) << 1);
                        if (cstat & 128) daz += k;
                        if (picanm[spr->picnum] & 0x00ff0000) daz -= ((long)((signed char)((picanm[spr->picnum] >> 16) &
                            255)) * spr->yrepeat << 2);
                        daz2 = daz - (k << 1);
                        clipyou = 1;
                    }
                    break;
                case 16:
                    tilenum = spr->picnum;
                    xoff = (long)((signed char)((picanm[tilenum] >> 8) & 255)) + ((long)spr->xoffset);
                    if ((cstat & 4) > 0) xoff = -xoff;
                    k = spr->ang;
                    l = spr->xrepeat;
                    dax = sintable[k & 2047] * l;
                    day = sintable[(k + 1536) & 2047] * l;
                    l = tilesizx[tilenum];
                    k = (l >> 1) + xoff;
                    x1 -= mulscale16(dax, k);
                    x2 = x1 + mulscale16(dax, l);
                    y1 -= mulscale16(day, k);
                    y2 = y1 + mulscale16(day, l);
                    if (clipinsideboxline(x, y, x1, y1, x2, y2, walldist + 1) != 0)
                    {
                        daz = spr->z;
                        k = ((tilesizy[spr->picnum] * spr->yrepeat) << 1);
                        if (cstat & 128) daz += k;
                        if (picanm[spr->picnum] & 0x00ff0000) daz -= ((long)((signed char)((picanm[spr->picnum] >> 16) &
                            255)) * spr->yrepeat << 2);
                        daz2 = daz - (k << 1);
                        clipyou = 1;
                    }
                    break;
                case 32:
                    daz = spr->z;
                    daz2 = daz;

                    if ((cstat & 64) != 0)
                        if ((z > daz) == ((cstat & 8) == 0)) continue;

                    tilenum = spr->picnum;
                    xoff = (long)((signed char)((picanm[tilenum] >> 8) & 255)) + ((long)spr->xoffset);
                    yoff = (long)((signed char)((picanm[tilenum] >> 16) & 255)) + ((long)spr->yoffset);
                    if ((cstat & 4) > 0) xoff = -xoff;
                    if ((cstat & 8) > 0) yoff = -yoff;

                    ang = spr->ang;
                    cosang = sintable[(ang + 512) & 2047];
                    sinang = sintable[ang];
                    xspan = tilesizx[tilenum];
                    xrepeat = spr->xrepeat;
                    yspan = tilesizy[tilenum];
                    yrepeat = spr->yrepeat;

                    dax = ((xspan >> 1) + xoff) * xrepeat;
                    day = ((yspan >> 1) + yoff) * yrepeat;
                    x1 += dmulscale16(sinang, dax, cosang, day) - x;
                    y1 += dmulscale16(sinang, day, -cosang, dax) - y;
                    l = xspan * xrepeat;
                    x2 = x1 - mulscale16(sinang, l);
                    y2 = y1 + mulscale16(cosang, l);
                    l = yspan * yrepeat;
                    k = -mulscale16(cosang, l);
                    x3 = x2 + k;
                    x4 = x1 + k;
                    k = -mulscale16(sinang, l);
                    y3 = y2 + k;
                    y4 = y1 + k;

                    dax = mulscale14(sintable[(spr->ang-256+512)&2047], walldist+4);
                    day = mulscale14(sintable[(spr->ang-256)&2047], walldist+4);
                    x1 += dax;
                    x2 -= day;
                    x3 -= dax;
                    x4 += day;
                    y1 += day;
                    y2 += dax;
                    y3 -= day;
                    y4 -= dax;

                    if ((y1 ^ y2) < 0)
                    {
                        if ((x1 ^ x2) < 0) clipyou ^= (x1 * y2 < x2 * y1) ^ (y1 < y2);
                        else if (x1 >= 0) clipyou ^= 1;
                    }
                    if ((y2 ^ y3) < 0)
                    {
                        if ((x2 ^ x3) < 0) clipyou ^= (x2 * y3 < x3 * y2) ^ (y2 < y3);
                        else if (x2 >= 0) clipyou ^= 1;
                    }
                    if ((y3 ^ y4) < 0)
                    {
                        if ((x3 ^ x4) < 0) clipyou ^= (x3 * y4 < x4 * y3) ^ (y3 < y4);
                        else if (x3 >= 0) clipyou ^= 1;
                    }
                    if ((y4 ^ y1) < 0)
                    {
                        if ((x4 ^ x1) < 0) clipyou ^= (x4 * y1 < x1 * y4) ^ (y4 < y1);
                        else if (x4 >= 0) clipyou ^= 1;
                    }
                    break;
                }

                if (clipyou != 0)
                {
                    if ((z > daz) && (daz > *ceilz))
                    {
                        *ceilz = daz;
                        *ceilhit = j + 49152;
                    }
                    if ((z < daz2) && (daz2 < *florz))
                    {
                        *florz = daz2;
                        *florhit = j + 49152;
                    }
                }
            }
        }
    }
}

int setview(long x1, long y1, long x2, long y2)
{
    return 0; // old gfx
    long i, j;

    windowx1 = x1;
    wx1 = (x1 << 12);
    windowy1 = y1;
    wy1 = (y1 << 12);
    windowx2 = x2;
    wx2 = ((x2 + 1) << 12);
    windowy2 = y2;
    wy2 = ((y2 + 1) << 12);

    xdimen = (x2 - x1) + 1;
    halfxdimen = (xdimen >> 1);
    xdimenrecip = divscale32(1L, xdimen);
    ydimen = (y2 - y1) + 1;

    setaspect(65536L, (long)divscale16(ydim * 320L, xdim * 200L));

    for (i = 0; i < windowx1; i++) { startumost[i] = 1, startdmost[i] = 0; }
    for (i = windowx1; i <= windowx2; i++)
    {
        startumost[i] = windowy1, startdmost[i] = windowy2 + 1;
    }
    for (i = windowx2 + 1; i < xdim; i++) { startumost[i] = 1, startdmost[i] = 0; }

    //  viewoffset = windowy1*bytesperline + windowx1;

    if ((stereomode) || (vidoption == 6))
    {
        ostereopixelwidth = stereopixelwidth;
        xdimen = (windowx2 - windowx1 + 1) + (stereopixelwidth << 1);
        halfxdimen = (xdimen >> 1);
        xdimenrecip = divscale32(1L, xdimen);
        setaspect((long)divscale16(xdimen, windowx2 - windowx1 + 1), yxaspect);
    }
}

void setaspect(long daxrange, long daaspect)
{
    return;//
    viewingrange = daxrange;
    viewingrangerecip = divscale32(1L, daxrange);

    yxaspect = daaspect;
    xyaspect = divscale32(1, yxaspect);
    xdimenscale = scale(xdimen, yxaspect, 320);
    xdimscale = scale(320, xyaspect, xdimen);
}

void dosetaspect()
{
    long i, j, k, x, y, xinc;

    if (xyaspect != oxyaspect)
    {
        oxyaspect = xyaspect;
        j = xyaspect * 320;
        horizlookup2[horizycent - 1] = divscale26(131072, j);
        for (i = ydim * 4 - 1; i >= 0; i--)
            if (i != (horizycent - 1))
            {
                horizlookup[i] = divscale28(1, i - (horizycent - 1));
                horizlookup2[i] = divscale14(klabs(horizlookup[i]), j);
            }
    }
    if ((xdimen != oxdimen) || (viewingrange != oviewingrange))
    {
        oxdimen = xdimen;
        oviewingrange = viewingrange;
        xinc = mulscale32(viewingrange*320, xdimenrecip);
        x = (640 << 16) - mulscale1(xinc, xdimen);
        for (i = 0; i < xdimen; i++)
        {
            j = (x & 65535);
            k = (x >> 16);
            x += xinc;
            if (j != 0) j = mulscale16((long)radarang[k+1]-(long)radarang[k], j);
            radarang2[i] = (short)(((long)radarang[k] + j) >> 6);
        }
#ifdef SUPERBUILD
        for (i = 1; i < 16384; i++) distrecip[i] = divscale20(xdimen, i);
        nytooclose = xdimen * 2100;
        nytoofar = 16384 * 16384 - 1048576;
#endif
    }
}

void flushperms()
{
    permhead = permtail = 0;
}

void rotatesprite(long sx, long sy, long z, short a, short picnum, signed char dashade, char dapalnum, char dastat,
                  long cx1, long cy1, long cx2, long cy2)
{
}

/*
//With a modern renderer, this becomes:
{
create sprite quad with 4 vertices
set vertex positions (with rotation matrix)
set UV coordinates for texture sampling

bind texture
set shader uniforms:
- transformation matrix
- tint color
- transparency mode

submit draw call
}
{
long i;
permfifotype *per, *per2;

if ((cx1 > cx2) || (cy1 > cy2)) return;
if (z <= 16) return;
if (picanm[picnum]&192) picnum += animateoffs(picnum,(short)0xc000);
if ((tilesizx[picnum] <= 0) || (tilesizy[picnum] <= 0)) return;

if (((dastat&128) == 0) || (numpages < 2) || (beforedrawrooms != 0))
    dorotatesprite(sx,sy,z,a,picnum,dashade,dapalnum,dastat,cx1,cy1,cx2,cy2);

if ((dastat&64) && (cx1 <= 0) && (cy1 <= 0) && (cx2 >= xdim-1) && (cy2 >= ydim-1) &&
    (sx == (160<<16)) && (sy == (100<<16)) && (z == 65536L) && (a == 0) && ((dastat&1) == 0))
    permhead = permtail = 0;

if ((dastat&128) == 0) return;
if (numpages >= 2)
{
    per = &permfifo[permhead];
    per->sx = sx; per->sy = sy; per->z = z; per->a = a;
    per->picnum = picnum;
    per->dashade = dashade; per->dapalnum = dapalnum;
    per->dastat = dastat;
    per->pagesleft = numpages+((beforedrawrooms&1)<<7);
    per->cx1 = cx1; per->cy1 = cy1; per->cx2 = cx2; per->cy2 = cy2;

    //Would be better to optimize out true bounding boxes
    if (dastat&64)  //If non-masking write, checking for overlapping cases
    {
        for(i=permtail;i!=permhead;i=((i+1)&(MAXPERMS-1)))
        {
            per2 = &permfifo[i];
            if ((per2->pagesleft&127) == 0) continue;
            if (per2->sx != per->sx) continue;
            if (per2->sy != per->sy) continue;
            if (per2->z != per->z) continue;
            if (per2->a != per->a) continue;
            if (tilesizx[per2->picnum] > tilesizx[per->picnum]) continue;
            if (tilesizy[per2->picnum] > tilesizy[per->picnum]) continue;
            if (per2->cx1 < per->cx1) continue;
            if (per2->cy1 < per->cy1) continue;
            if (per2->cx2 > per->cx2) continue;
            if (per2->cy2 > per->cy2) continue;
            per2->pagesleft = 0;
        }
        if ((per->z == 65536) && (per->a == 0))
            for(i=permtail;i!=permhead;i=((i+1)&(MAXPERMS-1)))
            {
                per2 = &permfifo[i];
                if ((per2->pagesleft&127) == 0) continue;
                if (per2->z != 65536) continue;
                if (per2->a != 0) continue;
                if (per2->cx1 < per->cx1) continue;
                if (per2->cy1 < per->cy1) continue;
                if (per2->cx2 > per->cx2) continue;
                if (per2->cy2 > per->cy2) continue;
                if ((per2->sx>>16) < (per->sx>>16)) continue;
                if ((per2->sy>>16) < (per->sy>>16)) continue;
                if ((per2->sx>>16)+tilesizx[per2->picnum] > (per->sx>>16)+tilesizx[per->picnum]) continue;
                if ((per2->sy>>16)+tilesizy[per2->picnum] > (per->sy>>16)+tilesizy[per->picnum]) continue;
                per2->pagesleft = 0;
            }
    }

    permhead = ((permhead+1)&(MAXPERMS-1));
}
}
*/
void dorotatesprite(long sx, long sy, long z, short a, short picnum, signed char dashade, char dapalnum, char dastat,
                    long cx1, long cy1, long cx2, long cy2){}
/*
{

    long cosang, sinang, v, nextv, dax1, dax2, oy, bx, by, ny1, ny2;
    long i, x, y, x1, y1, x2, y2, gx1, gy1, p, bufplc, palookupoffs;
    long xsiz, ysiz, xoff, yoff, npoints, yplc, yinc, lx, rx, xx, xend;
    long xv, yv, xv2, yv2, obuffermode, qlinemode, y1ve[4], y2ve[4], u4, d4;
    char bad;

    xsiz = tilesizx[picnum]; ysiz = tilesizy[picnum];
    if (dastat&16) { xoff = 0; yoff = 0; }
    else
    {
        xoff = (long)((signed char)((picanm[picnum]>>8)&255))+(xsiz>>1);
        yoff = (long)((signed char)((picanm[picnum]>>16)&255))+(ysiz>>1);
    }

    if (dastat&4) yoff = ysiz-yoff;

    cosang = sintable[(a+512)&2047]; sinang = sintable[a&2047];

    if ((dastat&2) != 0)  //Auto window size scaling
    {
        if ((dastat&8) == 0)
        {
            x = xdimenscale;   //= scale(xdimen,yxaspect,320);
            if (stereomode) x = scale(windowx2-windowx1+1,yxaspect,320);
            sx = ((cx1+cx2+2)<<15)+scale(sx-(320<<15),xdimen,320);
            sy = ((cy1+cy2+2)<<15)+mulscale16(sy-(200<<15),x);
        }
        else
        {
            //If not clipping to startmosts, & auto-scaling on, as a
            //hard-coded bonus, scale to full screen instead
            x = scale(xdim,yxaspect,320);
            sx = (xdim<<15)+32768+scale(sx-(320<<15),xdim,320);
            sy = (ydim<<15)+32768+mulscale16(sy-(200<<15),x);
        }
        z = mulscale16(z,x);
    }

    xv = mulscale14(cosang,z);
    yv = mulscale14(sinang,z);
    if (((dastat&2) != 0) || ((dastat&8) == 0)) //Don't aspect unscaled perms
    {
        xv2 = mulscale16(xv,xyaspect);
        yv2 = mulscale16(yv,xyaspect);
    }
    else
    {
        xv2 = xv;
        yv2 = yv;
    }

    ry1[0] = sy - (yv*xoff + xv*yoff);
    ry1[1] = ry1[0] + yv*xsiz;
    ry1[3] = ry1[0] + xv*ysiz;
    ry1[2] = ry1[1]+ry1[3]-ry1[0];
    i = (cy1<<16); if ((ry1[0]<i) && (ry1[1]<i) && (ry1[2]<i) && (ry1[3]<i)) return;
    i = (cy2<<16); if ((ry1[0]>i) && (ry1[1]>i) && (ry1[2]>i) && (ry1[3]>i)) return;

    rx1[0] = sx - (xv2*xoff - yv2*yoff);
    rx1[1] = rx1[0] + xv2*xsiz;
    rx1[3] = rx1[0] - yv2*ysiz;
    rx1[2] = rx1[1]+rx1[3]-rx1[0];
    i = (cx1<<16); if ((rx1[0]<i) && (rx1[1]<i) && (rx1[2]<i) && (rx1[3]<i)) return;
    i = (cx2<<16); if ((rx1[0]>i) && (rx1[1]>i) && (rx1[2]>i) && (rx1[3]>i)) return;

    gx1 = rx1[0]; gy1 = ry1[0];   //back up these before clipping

    if ((npoints = clippoly4(cx1<<16,cy1<<16,(cx2+1)<<16,(cy2+1)<<16)) < 3) return;

    lx = rx1[0]; rx = rx1[0];

    nextv = 0;
    for(v=npoints-1;v>=0;v--)
    {
        x1 = rx1[v]; x2 = rx1[nextv];
        dax1 = (x1>>16); if (x1 < lx) lx = x1;
        dax2 = (x2>>16); if (x1 > rx) rx = x1;
        if (dax1 != dax2)
        {
            y1 = ry1[v]; y2 = ry1[nextv];
            yinc = divscale16(y2-y1,x2-x1);
            if (dax2 > dax1)
            {
                yplc = y1 + mulscale16((dax1<<16)+65535-x1,yinc);
                qinterpolatedown16short((&uplc[dax1]),dax2-dax1,yplc,yinc);
            }
            else
            {
                yplc = y2 + mulscale16((dax2<<16)+65535-x2,yinc);
                qinterpolatedown16short((&dplc[dax2]),dax1-dax2,yplc,yinc);
            }
        }
        nextv = v;
    }

    if (waloff[picnum] == 0) loadtile(picnum);
    setgotpic(picnum);
    bufplc = waloff[picnum];

    palookupoffs = FP_OFF(palookup[dapalnum]) + (getpalookup(0L,(long)dashade)<<8);

    i = divscale32(1L,z);
    xv = mulscale14(sinang,i);
    yv = mulscale14(cosang,i);
    if (((dastat&2) != 0) || ((dastat&8) == 0)) //Don't aspect unscaled perms
    {
        yv2 = mulscale16(-xv,yxaspect);
        xv2 = mulscale16(yv,yxaspect);
    }
    else
    {
        yv2 = -xv;
        xv2 = yv;
    }

    x1 = (lx>>16); x2 = (rx>>16);

    oy = 0;
    x = (x1<<16)-1-gx1; y = (oy<<16)+65535-gy1;
    bx = dmulscale16(x,xv2,y,xv);
    by = dmulscale16(x,yv2,y,yv);
    if (dastat&4) { yv = -yv; yv2 = -yv2; by = (ysiz<<16)-1-by; }

    if ((vidoption == 1) && (origbuffermode == 0))
    {
        if (dastat&128)
        {
            obuffermode = buffermode;
            buffermode = 0;
            setactivepage(activepage);
        }
    }
    else if (dastat&8)
        permanentupdate = 1;

    if ((dastat&1) == 0)
    {
        if (((a&1023) == 0) && (ysiz <= 256))  //vlineasm4 has 256 high limit!
        {
            if (dastat&64) setupvlineasm(24L); else setupmvlineasm(24L);
            by <<= 8; yv <<= 8; yv2 <<= 8;

            palookupoffse[0] = palookupoffse[1] = palookupoffse[2] = palookupoffse[3] = palookupoffs;
            vince[0] = vince[1] = vince[2] = vince[3] = yv;

            for(x=x1;x<x2;x+=4)
            {
                bad = 15; xend = min(x2-x,4);
                for(xx=0;xx<xend;xx++)
                {
                    bx += xv2;

                    y1 = uplc[x+xx]; y2 = dplc[x+xx];
                    if ((dastat&8) == 0)
                    {
                        if (startumost[x+xx] > y1) y1 = startumost[x+xx];
                        if (startdmost[x+xx] < y2) y2 = startdmost[x+xx];
                    }
                    if (y2 <= y1) continue;

                    by += yv*(y1-oy); oy = y1;

                    bufplce[xx] = (bx>>16)*ysiz+bufplc;
                    vplce[xx] = by;
                    y1ve[xx] = y1;
                    y2ve[xx] = y2-1;
                    bad &= ~pow2char[xx];
                }

                p = x+frameplace;

                u4 = max(max(y1ve[0],y1ve[1]),max(y1ve[2],y1ve[3]));
                d4 = min(min(y2ve[0],y2ve[1]),min(y2ve[2],y2ve[3]));

                if (dastat&64)
                {
                    if ((bad != 0) || (u4 >= d4))
                    {
                        if (!(bad&1)) prevlineasm1(vince[0],palookupoffse[0],y2ve[0]-y1ve[0],vplce[0],bufplce[0],ylookup[y1ve[0]]+p+0);
                        if (!(bad&2)) prevlineasm1(vince[1],palookupoffse[1],y2ve[1]-y1ve[1],vplce[1],bufplce[1],ylookup[y1ve[1]]+p+1);
                        if (!(bad&4)) prevlineasm1(vince[2],palookupoffse[2],y2ve[2]-y1ve[2],vplce[2],bufplce[2],ylookup[y1ve[2]]+p+2);
                        if (!(bad&8)) prevlineasm1(vince[3],palookupoffse[3],y2ve[3]-y1ve[3],vplce[3],bufplce[3],ylookup[y1ve[3]]+p+3);
                        continue;
                    }

                    if (u4 > y1ve[0]) vplce[0] = prevlineasm1(vince[0],palookupoffse[0],u4-y1ve[0]-1,vplce[0],bufplce[0],ylookup[y1ve[0]]+p+0);
                    if (u4 > y1ve[1]) vplce[1] = prevlineasm1(vince[1],palookupoffse[1],u4-y1ve[1]-1,vplce[1],bufplce[1],ylookup[y1ve[1]]+p+1);
                    if (u4 > y1ve[2]) vplce[2] = prevlineasm1(vince[2],palookupoffse[2],u4-y1ve[2]-1,vplce[2],bufplce[2],ylookup[y1ve[2]]+p+2);
                    if (u4 > y1ve[3]) vplce[3] = prevlineasm1(vince[3],palookupoffse[3],u4-y1ve[3]-1,vplce[3],bufplce[3],ylookup[y1ve[3]]+p+3);

                    if (d4 >= u4) vlineasm4(d4-u4+1,ylookup[u4]+p);

                    i = p+ylookup[d4+1];
                    if (y2ve[0] > d4) prevlineasm1(vince[0],palookupoffse[0],y2ve[0]-d4-1,vplce[0],bufplce[0],i+0);
                    if (y2ve[1] > d4) prevlineasm1(vince[1],palookupoffse[1],y2ve[1]-d4-1,vplce[1],bufplce[1],i+1);
                    if (y2ve[2] > d4) prevlineasm1(vince[2],palookupoffse[2],y2ve[2]-d4-1,vplce[2],bufplce[2],i+2);
                    if (y2ve[3] > d4) prevlineasm1(vince[3],palookupoffse[3],y2ve[3]-d4-1,vplce[3],bufplce[3],i+3);
                }
                else
                {
                    if ((bad != 0) || (u4 >= d4))
                    {
                        if (!(bad&1)) mvlineasm1(vince[0],palookupoffse[0],y2ve[0]-y1ve[0],vplce[0],bufplce[0],ylookup[y1ve[0]]+p+0);
                        if (!(bad&2)) mvlineasm1(vince[1],palookupoffse[1],y2ve[1]-y1ve[1],vplce[1],bufplce[1],ylookup[y1ve[1]]+p+1);
                        if (!(bad&4)) mvlineasm1(vince[2],palookupoffse[2],y2ve[2]-y1ve[2],vplce[2],bufplce[2],ylookup[y1ve[2]]+p+2);
                        if (!(bad&8)) mvlineasm1(vince[3],palookupoffse[3],y2ve[3]-y1ve[3],vplce[3],bufplce[3],ylookup[y1ve[3]]+p+3);
                        continue;
                    }

                    if (u4 > y1ve[0]) vplce[0] = mvlineasm1(vince[0],palookupoffse[0],u4-y1ve[0]-1,vplce[0],bufplce[0],ylookup[y1ve[0]]+p+0);
                    if (u4 > y1ve[1]) vplce[1] = mvlineasm1(vince[1],palookupoffse[1],u4-y1ve[1]-1,vplce[1],bufplce[1],ylookup[y1ve[1]]+p+1);
                    if (u4 > y1ve[2]) vplce[2] = mvlineasm1(vince[2],palookupoffse[2],u4-y1ve[2]-1,vplce[2],bufplce[2],ylookup[y1ve[2]]+p+2);
                    if (u4 > y1ve[3]) vplce[3] = mvlineasm1(vince[3],palookupoffse[3],u4-y1ve[3]-1,vplce[3],bufplce[3],ylookup[y1ve[3]]+p+3);

                    if (d4 >= u4) mvlineasm4(d4-u4+1,ylookup[u4]+p);

                    i = p+ylookup[d4+1];
                    if (y2ve[0] > d4) mvlineasm1(vince[0],palookupoffse[0],y2ve[0]-d4-1,vplce[0],bufplce[0],i+0);
                    if (y2ve[1] > d4) mvlineasm1(vince[1],palookupoffse[1],y2ve[1]-d4-1,vplce[1],bufplce[1],i+1);
                    if (y2ve[2] > d4) mvlineasm1(vince[2],palookupoffse[2],y2ve[2]-d4-1,vplce[2],bufplce[2],i+2);
                    if (y2ve[3] > d4) mvlineasm1(vince[3],palookupoffse[3],y2ve[3]-d4-1,vplce[3],bufplce[3],i+3);
                }

                faketimerhandler();
            }
        }
        else
        {
            if (dastat&64)
            {
                if ((xv2&0x0000ffff) == 0)
                {
                    qlinemode = 1;
                    setupqrhlineasm4(0L,yv2<<16,(xv2>>16)*ysiz+(yv2>>16),palookupoffs,0L,0L);
                }
                else
                {
                    qlinemode = 0;
                    setuprhlineasm4(xv2<<16,yv2<<16,(xv2>>16)*ysiz+(yv2>>16),palookupoffs,ysiz,0L);
                }
            }
            else
                setuprmhlineasm4(xv2<<16,yv2<<16,(xv2>>16)*ysiz+(yv2>>16),palookupoffs,ysiz,0L);

            y1 = uplc[x1];
            if (((dastat&8) == 0) && (startumost[x1] > y1)) y1 = startumost[x1];
            y2 = y1;
            for(x=x1;x<x2;x++)
            {
                ny1 = uplc[x]-1; ny2 = dplc[x];
                if ((dastat&8) == 0)
                {
                    if (startumost[x]-1 > ny1) ny1 = startumost[x]-1;
                    if (startdmost[x] < ny2) ny2 = startdmost[x];
                }

                if (ny1 < ny2-1)
                {
                    if (ny1 >= y2)
                    {
                        while (y1 < y2-1)
                        {
                            y1++; if ((y1&31) == 0) faketimerhandler();

                            //x,y1
                            bx += xv*(y1-oy); by += yv*(y1-oy); oy = y1;
                            if (dastat&64) {  if (qlinemode) qrhlineasm4(x-lastx[y1],(bx>>16)*ysiz+(by>>16)+bufplc,0L,0L    ,by<<16,ylookup[y1]+x+frameplace);
                                else rhlineasm4(x-lastx[y1],(bx>>16)*ysiz+(by>>16)+bufplc,0L,bx<<16,by<<16,ylookup[y1]+x+frameplace);
                            } else rmhlineasm4(x-lastx[y1],(bx>>16)*ysiz+(by>>16)+bufplc,0L,bx<<16,by<<16,ylookup[y1]+x+frameplace);
                        }
                        y1 = ny1;
                    }
                    else
                    {
                        while (y1 < ny1)
                        {
                            y1++; if ((y1&31) == 0) faketimerhandler();

                            //x,y1
                            bx += xv*(y1-oy); by += yv*(y1-oy); oy = y1;
                            if (dastat&64) {  if (qlinemode) qrhlineasm4(x-lastx[y1],(bx>>16)*ysiz+(by>>16)+bufplc,0L,0L    ,by<<16,ylookup[y1]+x+frameplace);
                                else rhlineasm4(x-lastx[y1],(bx>>16)*ysiz+(by>>16)+bufplc,0L,bx<<16,by<<16,ylookup[y1]+x+frameplace);
                            } else rmhlineasm4(x-lastx[y1],(bx>>16)*ysiz+(by>>16)+bufplc,0L,bx<<16,by<<16,ylookup[y1]+x+frameplace);
                        }
                        while (y1 > ny1) lastx[y1--] = x;
                    }
                    while (y2 > ny2)
                    {
                        y2--; if ((y2&31) == 0) faketimerhandler();

                        //x,y2
                        bx += xv*(y2-oy); by += yv*(y2-oy); oy = y2;
                        if (dastat&64) {  if (qlinemode) qrhlineasm4(x-lastx[y2],(bx>>16)*ysiz+(by>>16)+bufplc,0L,0L    ,by<<16,ylookup[y2]+x+frameplace);
                            else rhlineasm4(x-lastx[y2],(bx>>16)*ysiz+(by>>16)+bufplc,0L,bx<<16,by<<16,ylookup[y2]+x+frameplace);
                        } else rmhlineasm4(x-lastx[y2],(bx>>16)*ysiz+(by>>16)+bufplc,0L,bx<<16,by<<16,ylookup[y2]+x+frameplace);
                    }
                    while (y2 < ny2) lastx[y2++] = x;
                }
                else
                {
                    while (y1 < y2-1)
                    {
                        y1++; if ((y1&31) == 0) faketimerhandler();

                        //x,y1
                        bx += xv*(y1-oy); by += yv*(y1-oy); oy = y1;
                        if (dastat&64) {  if (qlinemode) qrhlineasm4(x-lastx[y1],(bx>>16)*ysiz+(by>>16)+bufplc,0L,0L    ,by<<16,ylookup[y1]+x+frameplace);
                            else rhlineasm4(x-lastx[y1],(bx>>16)*ysiz+(by>>16)+bufplc,0L,bx<<16,by<<16,ylookup[y1]+x+frameplace);
                        } else rmhlineasm4(x-lastx[y1],(bx>>16)*ysiz+(by>>16)+bufplc,0L,bx<<16,by<<16,ylookup[y1]+x+frameplace);
                    }
                    if (x == x2-1) { bx += xv2; by += yv2; break; }
                    y1 = uplc[x+1];
                    if (((dastat&8) == 0) && (startumost[x+1] > y1)) y1 = startumost[x+1];
                    y2 = y1;
                }
                bx += xv2; by += yv2;
            }
            while (y1 < y2-1)
            {
                y1++; if ((y1&31) == 0) faketimerhandler();

                //x2,y1
                bx += xv*(y1-oy); by += yv*(y1-oy); oy = y1;
                if (dastat&64) {  if (qlinemode) qrhlineasm4(x2-lastx[y1],(bx>>16)*ysiz+(by>>16)+bufplc,0L,0L    ,by<<16,ylookup[y1]+x2+frameplace);
                    else rhlineasm4(x2-lastx[y1],(bx>>16)*ysiz+(by>>16)+bufplc,0L,bx<<16,by<<16,ylookup[y1]+x2+frameplace);
                } else rmhlineasm4(x2-lastx[y1],(bx>>16)*ysiz+(by>>16)+bufplc,0L,bx<<16,by<<16,ylookup[y1]+x2+frameplace);
            }
        }
    }
    else
    {
        if ((dastat&1) == 0)
        {
            if (dastat&64)
                setupspritevline(palookupoffs,(xv>>16)*ysiz,xv<<16,ysiz,yv,0L);
            else
                msetupspritevline(palookupoffs,(xv>>16)*ysiz,xv<<16,ysiz,yv,0L);
        }
        else
        {
            tsetupspritevline(palookupoffs,(xv>>16)*ysiz,xv<<16,ysiz,yv,0L);
            if (dastat&32) settransreverse(); else settransnormal();
        }
        for(x=x1;x<x2;x++)
        {
            bx += xv2; by += yv2;

            y1 = uplc[x]; y2 = dplc[x];
            if ((dastat&8) == 0)
            {
                if (startumost[x] > y1) y1 = startumost[x];
                if (startdmost[x] < y2) y2 = startdmost[x];
            }
            if (y2 <= y1) continue;

            switch(y1-oy)
            {
            case -1: bx -= xv; by -= yv; oy = y1; break;
            case 0: break;
            case 1: bx += xv; by += yv; oy = y1; break;
            default: bx += xv*(y1-oy); by += yv*(y1-oy); oy = y1; break;
            }

            p = ylookup[y1]+x+frameplace;

            if ((dastat&1) == 0)
            {
                if (dastat&64)
                    spritevline(0L,by<<16,y2-y1+1,bx<<16,(bx>>16)*ysiz+(by>>16)+bufplc,p);
                else
                    mspritevline(0L,by<<16,y2-y1+1,bx<<16,(bx>>16)*ysiz+(by>>16)+bufplc,p);
            }
            else
            {
                tspritevline(0L,by<<16,y2-y1+1,bx<<16,(bx>>16)*ysiz+(by>>16)+bufplc,p);
                transarea += (y2-y1);
            }
            faketimerhandler();
        }
    }

    if ((vidoption == 1) && (dastat&128) && (origbuffermode == 0))
    {
        buffermode = obuffermode;
        setactivepage(activepage);
    }
}
*/
long clippoly4(long cx1, long cy1, long cx2, long cy2)
{
    long n, nn, z, zz, x, x1, x2, y, y1, y2, t;

    nn = 0;
    z = 0;
    do
    {
        zz = ((z + 1) & 3);
        x1 = rx1[z];
        x2 = rx1[zz] - x1;

        if ((cx1 <= x1) && (x1 <= cx2))
            rx2[nn] = x1, ry2[nn] = ry1[z], nn++;

        if (x2 <= 0) x = cx2;
        else x = cx1;
        t = x - x1;
        if (((t - x2) ^ t) < 0)
            rx2[nn] = x, ry2[nn] = ry1[z] + scale(t, ry1[zz] - ry1[z], x2), nn++;

        if (x2 <= 0) x = cx1;
        else x = cx2;
        t = x - x1;
        if (((t - x2) ^ t) < 0)
            rx2[nn] = x, ry2[nn] = ry1[z] + scale(t, ry1[zz] - ry1[z], x2), nn++;

        z = zz;
    }
    while (z != 0);
    if (nn < 3) return (0);

    n = 0;
    z = 0;
    do
    {
        zz = z + 1;
        if (zz == nn) zz = 0;
        y1 = ry2[z];
        y2 = ry2[zz] - y1;

        if ((cy1 <= y1) && (y1 <= cy2))
            ry1[n] = y1, rx1[n] = rx2[z], n++;

        if (y2 <= 0) y = cy2;
        else y = cy1;
        t = y - y1;
        if (((t - y2) ^ t) < 0)
            ry1[n] = y, rx1[n] = rx2[z] + scale(t, rx2[zz] - rx2[z], y2), n++;

        if (y2 <= 0) y = cy1;
        else y = cy2;
        t = y - y1;
        if (((t - y2) ^ t) < 0)
            ry1[n] = y, rx1[n] = rx2[z] + scale(t, rx2[zz] - rx2[z], y2), n++;

        z = zz;
    }
    while (z != 0);
    return (n);
}

void makepalookup(long palnum, char* remapbuf, signed char r, signed char g, signed char b, char dastat){
  //  long i, j, dist, palscale;
  //  char *ptr, *ptr2;
//
  //  if (paletteloaded == 0) return;
//
  //  if (palookup[palnum] == NULL)
  //  {
  //      //Allocate palookup buffer
  //      if ((palookup[palnum] = (char*)kkmalloc(numpalookups << 8)) == NULL)
  //          allocache(&palookup[palnum], numpalookups << 8, &permanentlock);
  //  }
//
  //  if (dastat == 0) return;
  //  if ((r | g | b | 63) != 63) return;
//
  //  if ((r | g | b) == 0)
  //  {
  //      for (i = 0; i < 256; i++)
  //      {
  //          ptr = (char*)(FP_OFF(palookup[0]) + remapbuf[i]);
  //          ptr2 = (char*)(FP_OFF(palookup[palnum]) + i);
  //          for (j = 0; j < numpalookups; j++)
  //          {
  //              *ptr2 = *ptr;
  //              ptr += 256;
  //              ptr2 += 256;
  //          }
  //      }
  //  }
  //  else
  //  {
  //      ptr2 = (char*)FP_OFF(palookup[palnum]);
  //      for (i = 0; i < numpalookups; i++)
  //      {
  //          palscale = divscale16(i, numpalookups);
  //          for (j = 0; j < 256; j++)
  //          {
  //              ptr = (char*)&palette[remapbuf[j] * 3];
  //              *ptr2++ = getclosestcol((long)ptr[0] + mulscale16(r-ptr[0], palscale),
  //                                      (long)ptr[1] + mulscale16(g-ptr[1], palscale),
  //                                      (long)ptr[2] + mulscale16(b-ptr[2], palscale));
  //          }
  //      }
  //  }
//
  //  if ((vidoption == 6) && (qsetmode == 200))
  //  {
  //      for (i = 0; i < 256; i++)
  //      {
  //          dist = palette[i * 3] * 3 + palette[i * 3 + 1] * 5 + palette[i * 3 + 2] * 2;
  //          ptr = (char*)(FP_OFF(palookup[palnum]) + i);
  //          for (j = 0; j < 32; j++)
  //              ptr[j << 8] = (char)min(max(mulscale10(dist,32-j),0), 15);
  //      }
  //  }
}

void initfastcolorlookup(long rscale, long gscale, long bscale)
{
    long i, j, x, y, z;
    char* pal1;

    j = 0;
    for (i = 64; i >= 0; i--)
    {
        //j = (i-64)*(i-64);
        rdist[i] = rdist[128 - i] = j * rscale;
        gdist[i] = gdist[128 - i] = j * gscale;
        bdist[i] = bdist[128 - i] = j * bscale;
        j += 129 - (i << 1);
    }

    clearbufbyte((colhere), sizeof(colhere), 0L);
    clearbufbyte((colhead), sizeof(colhead), 0L);

    pal1 = (char*)&palette[768 - 3];
    for (i = 255; i >= 0; i--, pal1 -= 3)
    {
        j = (pal1[0] >> 3) * FASTPALGRIDSIZ * FASTPALGRIDSIZ + (pal1[1] >> 3) * FASTPALGRIDSIZ + (pal1[2] >> 3) +
            FASTPALGRIDSIZ * FASTPALGRIDSIZ + FASTPALGRIDSIZ + 1;
        if (colhere[j >> 3] & pow2char[j & 7]) colnext[i] = colhead[j];
        else colnext[i] = -1;
        colhead[j] = i;
        colhere[j >> 3] |= pow2char[j & 7];
    }

    i = 0;
    for (x = -FASTPALGRIDSIZ * FASTPALGRIDSIZ; x <= FASTPALGRIDSIZ * FASTPALGRIDSIZ; x += FASTPALGRIDSIZ *
         FASTPALGRIDSIZ)
        for (y = -FASTPALGRIDSIZ; y <= FASTPALGRIDSIZ; y += FASTPALGRIDSIZ)
            for (z = -1; z <= 1; z++)
                colscan[i++] = x + y + z;
    i = colscan[13];
    colscan[13] = colscan[26];
    colscan[26] = i;
}

long getclosestcol(long r, long g, long b)
{
    long x, y, z, i, j, k, dist, mindist, retcol;
    char* pal1;

    j = (r >> 3) * FASTPALGRIDSIZ * FASTPALGRIDSIZ + (g >> 3) * FASTPALGRIDSIZ + (b >> 3) + FASTPALGRIDSIZ *
        FASTPALGRIDSIZ + FASTPALGRIDSIZ + 1;
    mindist = min(rdist[coldist[r&7]+64+8], gdist[coldist[g&7]+64+8]);
    mindist = min(mindist, bdist[coldist[b&7]+64+8]);
    mindist++;

    r = 64 - r;
    g = 64 - g;
    b = 64 - b;

    retcol = -1;
    for (k = 26; k >= 0; k--)
    {
        i = colscan[k] + j;
        if ((colhere[i >> 3] & pow2char[i & 7]) == 0) continue;
        i = colhead[i];
        do
        {
            pal1 = (char*)&palette[i * 3];
            dist = gdist[pal1[1] + g];
            if (dist < mindist)
            {
                dist += rdist[pal1[0] + r];
                if (dist < mindist)
                {
                    dist += bdist[pal1[2] + b];
                    if (dist < mindist)
                    {
                        mindist = dist;
                        retcol = i;
                    }
                }
            }
            i = colnext[i];
        }
        while (i >= 0);
    }
    if (retcol >= 0) return (retcol);

    mindist = 0x7fffffff;
    pal1 = (char*)&palette[768 - 3];
    for (i = 255; i >= 0; i--, pal1 -= 3)
    {
        dist = gdist[pal1[1] + g];
        if (dist >= mindist) continue;
        dist += rdist[pal1[0] + r];
        if (dist >= mindist) continue;
        dist += bdist[pal1[2] + b];
        if (dist >= mindist) continue;
        mindist = dist;
        retcol = i;
    }
    return (retcol);
}

void setbrightness(char dabrightness, char* dapal)
{
    char* ptr;
    long i, j, k, dist, daval;

    curbrightness = min(max((long)dabrightness,0), 15);

    k = 0;
    if (vidoption == 6)
    {
        for (j = 0; j < 16; j++)
            for (i = 0; i < 16; i++)
            {
                tempbuf[k++] = britable[curbrightness][j << 2];
                tempbuf[k++] = 0;
                tempbuf[k++] = britable[curbrightness][i << 2];
                tempbuf[k++] = 0;
            }
    }
    else
    {
        for (i = 0; i < 256; i++)
        {
            tempbuf[k++] = britable[curbrightness][dapal[i * 3 + 2]];
            tempbuf[k++] = britable[curbrightness][dapal[i * 3 + 1]];
            tempbuf[k++] = britable[curbrightness][dapal[i * 3 + 0]];
            tempbuf[k++] = 0;
        }
    }

    //  VBE_setPalette(0,256,tempbuf);
}

void drawmapview(long dax, long day, long zoome, short ang)
{
    // IMPL.
}
/*
{
    walltype *wal;
    sectortype *sec;
    spritetype *spr;
    long tilenum, xoff, yoff, i, j, k, l, cosang, sinang, xspan, yspan;
    long xrepeat, yrepeat, x, y, x1, y1, x2, y2, x3, y3, x4, y4, bakx1, baky1;
    long s, w, ox, oy, startwall, cx1, cy1, cx2, cy2;
    long bakgxvect, bakgyvect, sortnum, gap, npoints;
    long xvect, yvect, xvect2, yvect2, daslope;

    beforedrawrooms = 0;
    totalarea += (windowx2+1-windowx1)*(windowy2+1-windowy1);

    clearbuf((&gotsector[0]),(long)((numsectors+31)>>5),0L);

    cx1 = (windowx1<<12); cy1 = (windowy1<<12);
    cx2 = ((windowx2+1)<<12)-1; cy2 = ((windowy2+1)<<12)-1;
    zoome <<= 8;
    bakgxvect = divscale28(sintable[(1536-ang)&2047],zoome);
    bakgyvect = divscale28(sintable[(2048-ang)&2047],zoome);
    xvect = mulscale8(sintable[(2048-ang)&2047],zoome);
    yvect = mulscale8(sintable[(1536-ang)&2047],zoome);
    xvect2 = mulscale16(xvect,yxaspect);
    yvect2 = mulscale16(yvect,yxaspect);

    sortnum = 0;
    for(s=0,sec=&sector[s];s<numsectors;s++,sec++)
        if (show2dsector[s>>3]&pow2char[s&7])
        {
            npoints = 0; i = 0;
            startwall = sec->wallptr;
            for(w=sec->wallnum,wal=&wall[startwall];w>0;w--,wal++)
            {
                ox = wal->x - dax; oy = wal->y - day;
                x = dmulscale16(ox,xvect,-oy,yvect) + (xdim<<11);
                y = dmulscale16(oy,xvect2,ox,yvect2) + (ydim<<11);
                i |= getclipmask(x-cx1,cx2-x,y-cy1,cy2-y);
                rx1[npoints] = x;
                ry1[npoints] = y;
                xb1[npoints] = wal->point2 - startwall;
                npoints++;
            }
            if ((i&0xf0) != 0xf0) continue;
            bakx1 = rx1[0]; baky1 = mulscale16(ry1[0]-(ydim<<11),xyaspect)+(ydim<<11);
            if (i&0x0f)
            {
                npoints = clippoly(npoints,i);
                if (npoints < 3) continue;
            }

            //Collect floor sprites to draw
            for(i=headspritesect[s];i>=0;i=nextspritesect[i])
                if ((sprite[i].cstat&48) == 32)
                {
                    if ((sprite[i].cstat&(64+8)) == (64+8)) continue;
                    tsprite[sortnum++].owner = i;
                }

            gotsector[s>>3] |= pow2char[s&7];

            globalorientation = (long)sec->floorstat;
            if ((globalorientation&1) != 0) continue;

            if (palookup[sec->floorpal] != globalpalwritten)
            {
                globalpalwritten = palookup[sec->floorpal];
                setpalookupaddress(globalpalwritten);
            }
            globalpicnum = sec->floorpicnum;
            if ((unsigned)globalpicnum >= (unsigned)MAXTILES) globalpicnum = 0;
            setgotpic(globalpicnum);
            if ((tilesizx[globalpicnum] <= 0) || (tilesizy[globalpicnum] <= 0)) continue;
            if ((picanm[globalpicnum]&192) != 0) globalpicnum += animateoffs((short)globalpicnum,s);
            if (waloff[globalpicnum] == 0) loadtile(globalpicnum);
            globalbufplc = waloff[globalpicnum];
            globalshade = max(min(sec->floorshade,numpalookups-1),0);
            globvis = globalhisibility;
            if (sec->visibility != 0) globvis = mulscale4(globvis,(long)((unsigned char)(sec->visibility+16)));
            globalpolytype = 0;
            if ((globalorientation&64) == 0)
            {
                globalposx = dax; globalx1 = bakgxvect; globaly1 = bakgyvect;
                globalposy = day; globalx2 = bakgxvect; globaly2 = bakgyvect;
            }
            else
            {
                ox = wall[wall[startwall].point2].x - wall[startwall].x;
                oy = wall[wall[startwall].point2].y - wall[startwall].y;
                i = nsqrtasm(ox*ox+oy*oy); if (i == 0) continue;
                i = 1048576/i;
                globalx1 = mulscale10(dmulscale10(ox,bakgxvect,oy,bakgyvect),i);
                globaly1 = mulscale10(dmulscale10(ox,bakgyvect,-oy,bakgxvect),i);
                ox = (bakx1>>4)-(xdim<<7); oy = (baky1>>4)-(ydim<<7);
                globalposx = dmulscale28(-oy,globalx1,-ox,globaly1);
                globalposy = dmulscale28(-ox,globalx1,oy,globaly1);
                globalx2 = -globalx1;
                globaly2 = -globaly1;

                daslope = sector[s].floorheinum;
                i = nsqrtasm(daslope*daslope+16777216);
                globalposy = mulscale12(globalposy,i);
                globalx2 = mulscale12(globalx2,i);
                globaly2 = mulscale12(globaly2,i);
            }
            globalxshift = (8-(picsiz[globalpicnum]&15));
            globalyshift = (8-(picsiz[globalpicnum]>>4));
            if (globalorientation&8) {globalxshift++; globalyshift++; }

            sethlinesizes(picsiz[globalpicnum]&15,picsiz[globalpicnum]>>4,globalbufplc);

            if ((globalorientation&0x4) > 0)
            {
                i = globalposx; globalposx = -globalposy; globalposy = -i;
                i = globalx2; globalx2 = globaly1; globaly1 = i;
                i = globalx1; globalx1 = -globaly2; globaly2 = -i;
            }
            if ((globalorientation&0x10) > 0) globalx1 = -globalx1, globaly1 = -globaly1, globalposx = -globalposx;
            if ((globalorientation&0x20) > 0) globalx2 = -globalx2, globaly2 = -globaly2, globalposy = -globalposy;
            asm1 = (globaly1<<globalxshift);
            asm2 = (globalx2<<globalyshift);
            globalx1 <<= globalxshift;
            globaly2 <<= globalyshift;
            globalposx = (globalposx<<(20+globalxshift))+(((long)sec->floorxpanning)<<24);
            globalposy = (globalposy<<(20+globalyshift))-(((long)sec->floorypanning)<<24);

            fillpolygon(npoints);
        }

    //Sort sprite list
    gap = 1; while (gap < sortnum) gap = (gap<<1)+1;
    for(gap>>=1;gap>0;gap>>=1)
        for(i=0;i<sortnum-gap;i++)
            for(j=i;j>=0;j-=gap)
            {
                if (sprite[tsprite[j].owner].z <= sprite[tsprite[j+gap].owner].z) break;
                swapshort(&tsprite[j].owner,&tsprite[j+gap].owner);
            }

    for(s=sortnum-1;s>=0;s--)
    {
        spr = &sprite[tsprite[s].owner];
        if ((spr->cstat&48) == 32)
        {
            npoints = 0;

            tilenum = spr->picnum;
            xoff = (long)((signed char)((picanm[tilenum]>>8)&255))+((long)spr->xoffset);
            yoff = (long)((signed char)((picanm[tilenum]>>16)&255))+((long)spr->yoffset);
            if ((spr->cstat&4) > 0) xoff = -xoff;
            if ((spr->cstat&8) > 0) yoff = -yoff;

            k = spr->ang;
            cosang = sintable[(k+512)&2047]; sinang = sintable[k];
            xspan = tilesizx[tilenum]; xrepeat = spr->xrepeat;
            yspan = tilesizy[tilenum]; yrepeat = spr->yrepeat;

            ox = ((xspan>>1)+xoff)*xrepeat; oy = ((yspan>>1)+yoff)*yrepeat;
            x1 = spr->x + mulscale(sinang,ox,16) + mulscale(cosang,oy,16);
            y1 = spr->y + mulscale(sinang,oy,16) - mulscale(cosang,ox,16);
            l = xspan*xrepeat;
            x2 = x1 - mulscale(sinang,l,16);
            y2 = y1 + mulscale(cosang,l,16);
            l = yspan*yrepeat;
            k = -mulscale(cosang,l,16); x3 = x2+k; x4 = x1+k;
            k = -mulscale(sinang,l,16); y3 = y2+k; y4 = y1+k;

            xb1[0] = 1; xb1[1] = 2; xb1[2] = 3; xb1[3] = 0;
            npoints = 4;

            i = 0;

            ox = x1 - dax; oy = y1 - day;
            x = dmulscale16(ox,xvect,-oy,yvect) + (xdim<<11);
            y = dmulscale16(oy,xvect2,ox,yvect2) + (ydim<<11);
            i |= getclipmask(x-cx1,cx2-x,y-cy1,cy2-y);
            rx1[0] = x; ry1[0] = y;

            ox = x2 - dax; oy = y2 - day;
            x = dmulscale16(ox,xvect,-oy,yvect) + (xdim<<11);
            y = dmulscale16(oy,xvect2,ox,yvect2) + (ydim<<11);
            i |= getclipmask(x-cx1,cx2-x,y-cy1,cy2-y);
            rx1[1] = x; ry1[1] = y;

            ox = x3 - dax; oy = y3 - day;
            x = dmulscale16(ox,xvect,-oy,yvect) + (xdim<<11);
            y = dmulscale16(oy,xvect2,ox,yvect2) + (ydim<<11);
            i |= getclipmask(x-cx1,cx2-x,y-cy1,cy2-y);
            rx1[2] = x; ry1[2] = y;

            x = rx1[0]+rx1[2]-rx1[1];
            y = ry1[0]+ry1[2]-ry1[1];
            i |= getclipmask(x-cx1,cx2-x,y-cy1,cy2-y);
            rx1[3] = x; ry1[3] = y;

            if ((i&0xf0) != 0xf0) continue;
            bakx1 = rx1[0]; baky1 = mulscale16(ry1[0]-(ydim<<11),xyaspect)+(ydim<<11);
            if (i&0x0f)
            {
                npoints = clippoly(npoints,i);
                if (npoints < 3) continue;
            }

            globalpicnum = spr->picnum;
            if ((unsigned)globalpicnum >= (unsigned)MAXTILES) globalpicnum = 0;
            setgotpic(globalpicnum);
            if ((tilesizx[globalpicnum] <= 0) || (tilesizy[globalpicnum] <= 0)) continue;
            if ((picanm[globalpicnum]&192) != 0) globalpicnum += animateoffs((short)globalpicnum,s);
            if (waloff[globalpicnum] == 0) loadtile(globalpicnum);
            globalbufplc = waloff[globalpicnum];
            if ((sector[spr->sectnum].ceilingstat&1) > 0)
                globalshade = ((long)sector[spr->sectnum].ceilingshade);
            else
                globalshade = ((long)sector[spr->sectnum].floorshade);
            globalshade = max(min(globalshade+spr->shade+6,numpalookups-1),0);
            asm3 = FP_OFF(palookup[spr->pal]+(globalshade<<8));
            globvis = globalhisibility;
            if (sec->visibility != 0) globvis = mulscale4(globvis,(long)((unsigned char)(sec->visibility+16)));
            globalpolytype = ((spr->cstat&2)>>1)+1;

            //relative alignment stuff
            ox = x2-x1; oy = y2-y1;
            i = ox*ox+oy*oy; if (i == 0) continue; i = (65536*16384)/i;
            globalx1 = mulscale10(dmulscale10(ox,bakgxvect,oy,bakgyvect),i);
            globaly1 = mulscale10(dmulscale10(ox,bakgyvect,-oy,bakgxvect),i);
            ox = y1-y4; oy = x4-x1;
            i = ox*ox+oy*oy; if (i == 0) continue; i = (65536*16384)/i;
            globalx2 = mulscale10(dmulscale10(ox,bakgxvect,oy,bakgyvect),i);
            globaly2 = mulscale10(dmulscale10(ox,bakgyvect,-oy,bakgxvect),i);

            ox = picsiz[globalpicnum]; oy = ((ox>>4)&15); ox &= 15;
            if (pow2long[ox] != xspan)
            {
                ox++;
                globalx1 = mulscale(globalx1,xspan,ox);
                globaly1 = mulscale(globaly1,xspan,ox);
            }

            bakx1 = (bakx1>>4)-(xdim<<7); baky1 = (baky1>>4)-(ydim<<7);
            globalposx = dmulscale28(-baky1,globalx1,-bakx1,globaly1);
            globalposy = dmulscale28(bakx1,globalx2,-baky1,globaly2);

            if ((spr->cstat&2) == 0)
                msethlineshift(ox,oy);
            else
                tsethlineshift(ox,oy);

            if ((spr->cstat&0x4) > 0) globalx1 = -globalx1, globaly1 = -globaly1, globalposx = -globalposx;
            asm1 = (globaly1<<2); globalx1 <<= 2; globalposx <<= (20+2);
            asm2 = (globalx2<<2); globaly2 <<= 2; globalposy <<= (20+2);

            fillpolygon(npoints);
        }
    }
}*/

long clippoly(long npoints, long clipstat)
{
    long z, zz, s1, s2, t, npoints2, start2, z1, z2, z3, z4, splitcnt;
    long cx1, cy1, cx2, cy2;

    cx1 = windowx1;
    cy1 = windowy1;
    cx2 = windowx2 + 1;
    cy2 = windowy2 + 1;
    cx1 <<= 12;
    cy1 <<= 12;
    cx2 <<= 12;
    cy2 <<= 12;

    if (clipstat & 0xa) //Need to clip top or left
    {
        npoints2 = 0;
        start2 = 0;
        z = 0;
        splitcnt = 0;
        do
        {
            s2 = cx1 - rx1[z];
            do
            {
                zz = xb1[z];
                xb1[z] = -1;
                s1 = s2;
                s2 = cx1 - rx1[zz];
                if (s1 < 0)
                {
                    rx2[npoints2] = rx1[z];
                    ry2[npoints2] = ry1[z];
                    xb2[npoints2] = npoints2 + 1;
                    npoints2++;
                }
                if ((s1 ^ s2) < 0)
                {
                    rx2[npoints2] = rx1[z] + scale(rx1[zz] - rx1[z], s1, s1 - s2);
                    ry2[npoints2] = ry1[z] + scale(ry1[zz] - ry1[z], s1, s1 - s2);
                    if (s1 < 0) p2[splitcnt++] = npoints2;
                    xb2[npoints2] = npoints2 + 1;
                    npoints2++;
                }
                z = zz;
            }
            while (xb1[z] >= 0);

            if (npoints2 >= start2 + 3)
                xb2[npoints2 - 1] = start2, start2 = npoints2;
            else
                npoints2 = start2;

            z = 1;
            while ((z < npoints) && (xb1[z] < 0)) z++;
        }
        while (z < npoints);
        if (npoints2 <= 2) return (0);

        for (z = 1; z < splitcnt; z++)
            for (zz = 0; zz < z; zz++)
            {
                z1 = p2[z];
                z2 = xb2[z1];
                z3 = p2[zz];
                z4 = xb2[z3];
                s1 = klabs(rx2[z1] - rx2[z2]) + klabs(ry2[z1] - ry2[z2]);
                s1 += klabs(rx2[z3] - rx2[z4]) + klabs(ry2[z3] - ry2[z4]);
                s2 = klabs(rx2[z1] - rx2[z4]) + klabs(ry2[z1] - ry2[z4]);
                s2 += klabs(rx2[z3] - rx2[z2]) + klabs(ry2[z3] - ry2[z2]);
                if (s2 < s1)
                {
                    t = xb2[p2[z]];
                    xb2[p2[z]] = xb2[p2[zz]];
                    xb2[p2[zz]] = t;
                }
            }


        npoints = 0;
        start2 = 0;
        z = 0;
        splitcnt = 0;
        do
        {
            s2 = cy1 - ry2[z];
            do
            {
                zz = xb2[z];
                xb2[z] = -1;
                s1 = s2;
                s2 = cy1 - ry2[zz];
                if (s1 < 0)
                {
                    rx1[npoints] = rx2[z];
                    ry1[npoints] = ry2[z];
                    xb1[npoints] = npoints + 1;
                    npoints++;
                }
                if ((s1 ^ s2) < 0)
                {
                    rx1[npoints] = rx2[z] + scale(rx2[zz] - rx2[z], s1, s1 - s2);
                    ry1[npoints] = ry2[z] + scale(ry2[zz] - ry2[z], s1, s1 - s2);
                    if (s1 < 0) p2[splitcnt++] = npoints;
                    xb1[npoints] = npoints + 1;
                    npoints++;
                }
                z = zz;
            }
            while (xb2[z] >= 0);

            if (npoints >= start2 + 3)
                xb1[npoints - 1] = start2, start2 = npoints;
            else
                npoints = start2;

            z = 1;
            while ((z < npoints2) && (xb2[z] < 0)) z++;
        }
        while (z < npoints2);
        if (npoints <= 2) return (0);

        for (z = 1; z < splitcnt; z++)
            for (zz = 0; zz < z; zz++)
            {
                z1 = p2[z];
                z2 = xb1[z1];
                z3 = p2[zz];
                z4 = xb1[z3];
                s1 = klabs(rx1[z1] - rx1[z2]) + klabs(ry1[z1] - ry1[z2]);
                s1 += klabs(rx1[z3] - rx1[z4]) + klabs(ry1[z3] - ry1[z4]);
                s2 = klabs(rx1[z1] - rx1[z4]) + klabs(ry1[z1] - ry1[z4]);
                s2 += klabs(rx1[z3] - rx1[z2]) + klabs(ry1[z3] - ry1[z2]);
                if (s2 < s1)
                {
                    t = xb1[p2[z]];
                    xb1[p2[z]] = xb1[p2[zz]];
                    xb1[p2[zz]] = t;
                }
            }
    }
    if (clipstat & 0x5) //Need to clip bottom or right
    {
        npoints2 = 0;
        start2 = 0;
        z = 0;
        splitcnt = 0;
        do
        {
            s2 = rx1[z] - cx2;
            do
            {
                zz = xb1[z];
                xb1[z] = -1;
                s1 = s2;
                s2 = rx1[zz] - cx2;
                if (s1 < 0)
                {
                    rx2[npoints2] = rx1[z];
                    ry2[npoints2] = ry1[z];
                    xb2[npoints2] = npoints2 + 1;
                    npoints2++;
                }
                if ((s1 ^ s2) < 0)
                {
                    rx2[npoints2] = rx1[z] + scale(rx1[zz] - rx1[z], s1, s1 - s2);
                    ry2[npoints2] = ry1[z] + scale(ry1[zz] - ry1[z], s1, s1 - s2);
                    if (s1 < 0) p2[splitcnt++] = npoints2;
                    xb2[npoints2] = npoints2 + 1;
                    npoints2++;
                }
                z = zz;
            }
            while (xb1[z] >= 0);

            if (npoints2 >= start2 + 3)
                xb2[npoints2 - 1] = start2, start2 = npoints2;
            else
                npoints2 = start2;

            z = 1;
            while ((z < npoints) && (xb1[z] < 0)) z++;
        }
        while (z < npoints);
        if (npoints2 <= 2) return (0);

        for (z = 1; z < splitcnt; z++)
            for (zz = 0; zz < z; zz++)
            {
                z1 = p2[z];
                z2 = xb2[z1];
                z3 = p2[zz];
                z4 = xb2[z3];
                s1 = klabs(rx2[z1] - rx2[z2]) + klabs(ry2[z1] - ry2[z2]);
                s1 += klabs(rx2[z3] - rx2[z4]) + klabs(ry2[z3] - ry2[z4]);
                s2 = klabs(rx2[z1] - rx2[z4]) + klabs(ry2[z1] - ry2[z4]);
                s2 += klabs(rx2[z3] - rx2[z2]) + klabs(ry2[z3] - ry2[z2]);
                if (s2 < s1)
                {
                    t = xb2[p2[z]];
                    xb2[p2[z]] = xb2[p2[zz]];
                    xb2[p2[zz]] = t;
                }
            }


        npoints = 0;
        start2 = 0;
        z = 0;
        splitcnt = 0;
        do
        {
            s2 = ry2[z] - cy2;
            do
            {
                zz = xb2[z];
                xb2[z] = -1;
                s1 = s2;
                s2 = ry2[zz] - cy2;
                if (s1 < 0)
                {
                    rx1[npoints] = rx2[z];
                    ry1[npoints] = ry2[z];
                    xb1[npoints] = npoints + 1;
                    npoints++;
                }
                if ((s1 ^ s2) < 0)
                {
                    rx1[npoints] = rx2[z] + scale(rx2[zz] - rx2[z], s1, s1 - s2);
                    ry1[npoints] = ry2[z] + scale(ry2[zz] - ry2[z], s1, s1 - s2);
                    if (s1 < 0) p2[splitcnt++] = npoints;
                    xb1[npoints] = npoints + 1;
                    npoints++;
                }
                z = zz;
            }
            while (xb2[z] >= 0);

            if (npoints >= start2 + 3)
                xb1[npoints - 1] = start2, start2 = npoints;
            else
                npoints = start2;

            z = 1;
            while ((z < npoints2) && (xb2[z] < 0)) z++;
        }
        while (z < npoints2);
        if (npoints <= 2) return (0);

        for (z = 1; z < splitcnt; z++)
            for (zz = 0; zz < z; zz++)
            {
                z1 = p2[z];
                z2 = xb1[z1];
                z3 = p2[zz];
                z4 = xb1[z3];
                s1 = klabs(rx1[z1] - rx1[z2]) + klabs(ry1[z1] - ry1[z2]);
                s1 += klabs(rx1[z3] - rx1[z4]) + klabs(ry1[z3] - ry1[z4]);
                s2 = klabs(rx1[z1] - rx1[z4]) + klabs(ry1[z1] - ry1[z4]);
                s2 += klabs(rx1[z3] - rx1[z2]) + klabs(ry1[z3] - ry1[z2]);
                if (s2 < s1)
                {
                    t = xb1[p2[z]];
                    xb1[p2[z]] = xb1[p2[zz]];
                    xb1[p2[zz]] = t;
                }
            }
    }
    return (npoints);
}

void fillpolygon(long npoints){};
void clearview(long dacol)
{
    //   long i, p, y, x1, x2, dx;
    //   char *ptr;
    //
    //   if (qsetmode != 200) return;
    //
    //   dx = windowx2-windowx1+1;
    //   dacol += (dacol<<8); dacol += (dacol<<16);
    //   if (vidoption == 6)
    //   {
    //       p = FP_OFF(screen)+ylookup[windowy1]+windowx1;
    //       for(y=windowy1;y<=windowy2;y++)
    //       {
    //           clearbufbyte(p,dx,dacol);
    //           clearbufbyte(p+65536,dx,dacol);
    //           p += ylookup[1];
    //       }
    //       faketimerhandler();
    //       return;
    //   }
    //   p = frameplace+ylookup[windowy1]+windowx1;
    //   for(y=windowy1;y<=windowy2;y++)
    //   { clearbufbyte(p,dx,dacol); p += ylookup[1]; }
    //   faketimerhandler();
}

void clearallviews(long dacol)
{
    long i;

    if (qsetmode != 200) return;
    dacol += (dacol << 8);
    dacol += (dacol << 16);

    // switch(vidoption)
    // {
    // case 1:
    //     for(i=0;i<numpages;i++)
    //     {
    //         setactivepage(i);
    //         clearbufbyte(frameplace,imageSize,0L);
    //     }
    //     setactivepage(activepage);
    // case 2:
    //     clearbuf(frameplace,(xdim*ydim)>>2,0L);
    //     break;
    // case 6:
    //     clearbuf(screen,128000L>>2,dacol);
    //     break;
    // }
 //   faketimerhandler();
}

void setviewtotile(short tilenume, long xsiz, long ysiz)
{
    return; //
    // replace with rendertextures somehow.
    long i, j;

    //DRAWROOMS TO TILE BACKUP&SET CODE
    tilesizx[tilenume] = xsiz;
    tilesizy[tilenume] = ysiz;
    bakxsiz[setviewcnt] = xsiz;
    bakysiz[setviewcnt] = ysiz;
    bakvidoption[setviewcnt] = vidoption;
    vidoption = 2;
    bakframeplace[setviewcnt] = frameplace;
    frameplace = waloff[tilenume];
    bakwindowx1[setviewcnt] = windowx1;
    bakwindowy1[setviewcnt] = windowy1;
    bakwindowx2[setviewcnt] = windowx2;
    bakwindowy2[setviewcnt] = windowy2;
    copybufbyte(&startumost[windowx1], &bakumost[windowx1], (windowx2 - windowx1 + 1) * sizeof(bakumost[0]));
    copybufbyte(&startdmost[windowx1], &bakdmost[windowx1], (windowx2 - windowx1 + 1) * sizeof(bakdmost[0]));
    setview(0, 0, ysiz - 1, xsiz - 1);
    setaspect(65536, 65536);
    j = 0;
    for (i = 0; i <= xsiz; i++) { ylookup[i] = j, j += ysiz; }
    //setvlinebpl(ysiz);
    setviewcnt++;
}

static int bytesperline;

void setviewback()
{
    long i, j, k, xsiz, ysiz;

    if (setviewcnt <= 0) return;
    setviewcnt--;

    setview(bakwindowx1[setviewcnt], bakwindowy1[setviewcnt],
            bakwindowx2[setviewcnt], bakwindowy2[setviewcnt]);
    copybufbyte(&bakumost[windowx1], &startumost[windowx1], (windowx2 - windowx1 + 1) * sizeof(startumost[0]));
    copybufbyte(&bakdmost[windowx1], &startdmost[windowx1], (windowx2 - windowx1 + 1) * sizeof(startdmost[0]));

    return; //
    vidoption = bakvidoption[setviewcnt];
    frameplace = bakframeplace[setviewcnt];
    if (setviewcnt == 0)
        k = bakxsiz[0];
    else
        k = max(bakxsiz[setviewcnt-1], bakxsiz[setviewcnt]);
    j = 0;
    for (i = 0; i <= k; i++) ylookup[i] = j, j += bytesperline;
    //setvlinebpl(bytesperline);
}

void squarerotatetile(short tilenume)
{
    //  long i, j, k, xsiz, ysiz;
    //  char *ptr1, *ptr2;
    //
    //  xsiz = tilesizx[tilenume]; ysiz = tilesizy[tilenume];
    //
    //  //supports square tiles only for rotation part
    //  if (xsiz == ysiz)
    //  {
    //      k = (xsiz<<1);
    //      for(i=xsiz-1;i>=0;i--)
    //      {
    //          ptr1 = (char *)(waloff[tilenume]+i*(xsiz+1)); ptr2 = ptr1;
    //          if ((i&1) != 0) { ptr1--; ptr2 -= xsiz;  swapchar(ptr1,ptr2); }
    //          for(j=(i>>1)-1;j>=0;j--)
    //          { ptr1 -= 2; ptr2 -= k; swapchar2(ptr1,ptr2,xsiz); }
    //      }
    //  }
}

void preparemirror(long dax, long day, long daz, short daang, long dahoriz, short dawall, short dasector, long* tposx,
                   long* tposy, short* tang)
{
    long i, j, x, y, dx, dy;

    x = wall[dawall].x;
    dx = wall[wall[dawall].point2].x - x;
    y = wall[dawall].y;
    dy = wall[wall[dawall].point2].y - y;
    j = dx * dx + dy * dy;
    if (j == 0) return;
    i = (((dax - x) * dx + (day - y) * dy) << 1);
    *tposx = (x << 1) + scale(dx, i, j) - dax;
    *tposy = (y << 1) + scale(dy, i, j) - day;
    *tang = (((getangle(dx, dy) << 1) - daang) & 2047);

    inpreparemirror = 1;
}

void completemirror()
{
    return; // do mirror external.
    long i, dy, p;

    //Can't reverse with uninitialized data
    if (inpreparemirror)
    {
        inpreparemirror = 0;
        return;
    }
    if (mirrorsx1 > 0) mirrorsx1--;
    if (mirrorsx2 < windowx2 - windowx1 - 1) mirrorsx2++;
    if (mirrorsx2 < mirrorsx1) return;

    transarea += (mirrorsx2 - mirrorsx1) * (windowy2 - windowy1);

    p = frameplace + ylookup[windowy1 + mirrorsy1] + windowx1 + mirrorsx1;
    i = windowx2 - windowx1 - mirrorsx2 - mirrorsx1;
    mirrorsx2 -= mirrorsx1;
    for (dy = mirrorsy2 - mirrorsy1; dy >= 0; dy--)
    {
        copybufbyte(&p + 1, tempbuf, mirrorsx2 + 1);
        tempbuf[mirrorsx2] = tempbuf[mirrorsx2 - 1];
        copybufreverse(&tempbuf[mirrorsx2], &p + i, mirrorsx2 + 1);
        p += ylookup[1];
        faketimerhandler();
    }
}

long sectorofwall(short theline)
{
    long i, j, gap;

    if ((theline < 0) || (theline >= numwalls)) return (-1);
    i = wall[theline].nextwall;
    if (i >= 0) return (wall[i].nextsector);

    gap = (numsectors >> 1);
    i = gap;
    while (gap > 1)
    {
        gap >>= 1;
        if (sector[i].wallptr < theline) i += gap;
        else i -= gap;
    }
    while (sector[i].wallptr > theline) i--;
    while (sector[i].wallptr + sector[i].wallnum <= theline) i++;
    return (i);
}

long bbeng.GetFloorZSloped(short sectnum, long dax, long day)
{
    long dx, dy, i, j;
    walltype* wal;

    if (!(sector[sectnum].ceilingstat & 2)) return (sector[sectnum].ceilingz);
    wal = &wall[sector[sectnum].wallptr];
    dx = wall[wal->point2].x - wal->x;
    dy = wall[wal->point2].y - wal->y;
    i = (nsqrtasm(dx * dx + dy * dy) << 5);
    if (i == 0) return (sector[sectnum].ceilingz);
    j = dmulscale3(dx, day - wal->y, -dy, dax - wal->x);
    return (sector[sectnum].ceilingz + scale(sector[sectnum].ceilingheinum, j, i));
}

long getflorzofslope(short sectnum, long dax, long day)
{
    long dx, dy, i, j;
    walltype* wal;

    if (!(sector[sectnum].floorstat & 2)) return (sector[sectnum].floorz);
    wal = &wall[sector[sectnum].wallptr];
    dx = wall[wal->point2].x - wal->x;
    dy = wall[wal->point2].y - wal->y;
    i = (nsqrtasm(dx * dx + dy * dy) << 5);
    if (i == 0) return (sector[sectnum].floorz);
    j = dmulscale3(dx, day - wal->y, -dy, dax - wal->x);
    return (sector[sectnum].floorz + scale(sector[sectnum].floorheinum, j, i));
}

void getzsofslope(short sectnum, long dax, long day, long* ceilz, long* florz)
{
    long dx, dy, i, j;
    walltype *wal, *wal2;
    sectortype* sec;

    sec = &sector[sectnum];
    *ceilz = sec->ceilingz;
    *florz = sec->floorz;
    if ((sec->ceilingstat | sec->floorstat) & 2)
    {
        wal = &wall[sec->wallptr];
        wal2 = &wall[wal->point2];
        dx = wal2->x - wal->x;
        dy = wal2->y - wal->y;
        i = (nsqrtasm(dx * dx + dy * dy) << 5);
        if (i == 0) return;
        j = dmulscale3(dx, day - wal->y, -dy, dax - wal->x);
        if (sec->ceilingstat & 2) *ceilz = (*ceilz) + scale(sec->ceilingheinum, j, i);
        if (sec->floorstat & 2) *florz = (*florz) + scale(sec->floorheinum, j, i);
    }
}

void alignflorslope(short dasect, long x, long y, long z)
{
    long i, dax, day;
    walltype* wal;

    wal = &wall[sector[dasect].wallptr];
    dax = wall[wal->point2].x - wal->x;
    day = wall[wal->point2].y - wal->y;

    i = (y - wal->y) * dax - (x - wal->x) * day;
    if (i == 0) return;
    sector[dasect].floorheinum = scale((z - sector[dasect].floorz) << 8,
                                       nsqrtasm(dax * dax + day * day), i);

    if (sector[dasect].floorheinum == 0) sector[dasect].floorstat &= ~2;
    else sector[dasect].floorstat |= 2;
}

long getpalookup(long davis, long dashade)
{
    return (min(max(dashade+(davis>>8),0), numpalookups-1));
}

long loopnumofsector(short sectnum, short wallnum)
{
    long i, numloops, startwall, endwall;

    numloops = 0;
    startwall = sector[sectnum].wallptr;
    endwall = startwall + sector[sectnum].wallnum;
    for (i = startwall; i < endwall; i++)
    {
        if (i == wallnum) return (numloops);
        if (wall[i].point2 < i) numloops++;
    }
    return (-1);
}

void setfirstwall(short sectnum, short newfirstwall)
{
    long i, j, k, numwallsofloop;
    long startwall, endwall, danumwalls, dagoalloop;

    startwall = sector[sectnum].wallptr;
    danumwalls = sector[sectnum].wallnum;
    endwall = startwall+danumwalls;
    if ((newfirstwall < startwall) || (newfirstwall >= startwall+danumwalls)) return;
    for(i=0;i<danumwalls;i++)
        memcpy(&wall[i+numwalls],&wall[i+startwall],sizeof(walltype));

    numwallsofloop = 0;
    i = newfirstwall;
    do
    {
        numwallsofloop++;
        i = wall[i].point2;
    } while (i != newfirstwall);

    //Put correct loop at beginning
    dagoalloop = loopnumofsector(sectnum,newfirstwall);
    if (dagoalloop > 0)
    {
        j = 0;
        while (loopnumofsector(sectnum,j+startwall) != dagoalloop) j++;
        for(i=0;i<danumwalls;i++)
        {
            k = i+j; if (k >= danumwalls) k -= danumwalls;
            memcpy(&wall[startwall+i],&wall[numwalls+k],sizeof(walltype));

            wall[startwall+i].point2 += danumwalls-startwall-j;
            if (wall[startwall+i].point2 >= danumwalls)
                wall[startwall+i].point2 -= danumwalls;
            wall[startwall+i].point2 += startwall;
        }
        newfirstwall += danumwalls-j;
        if (newfirstwall >= startwall+danumwalls) newfirstwall -= danumwalls;
    }

    for(i=0;i<numwallsofloop;i++)
        memcpy(&wall[i+numwalls],&wall[i+startwall],sizeof(walltype));
    for(i=0;i<numwallsofloop;i++)
    {
        k = i+newfirstwall-startwall;
        if (k >= numwallsofloop) k -= numwallsofloop;
        memcpy(&wall[startwall+i],&wall[numwalls+k],sizeof(walltype));

        wall[startwall+i].point2 += numwallsofloop-newfirstwall;
        if (wall[startwall+i].point2 >= numwallsofloop)
            wall[startwall+i].point2 -= numwallsofloop;
        wall[startwall+i].point2 += startwall;
    }

    for(i=startwall;i<endwall;i++)
        if (wall[i].nextwall >= 0) wall[wall[i].nextwall].nextwall = i;
}

