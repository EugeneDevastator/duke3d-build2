//
// Created by omnis on 10/20/2025.
//
#ifndef BUILD2_LOADERS_H
#define BUILD2_LOADERS_H
// # Prio 1 for Eugene
// finish xsurf implementation
// attempt portals
#include "mapcore.h"
#include "kplib.h"

static char curmappath[MAX_PATH+1] = "";
static unsigned char gammlut[256], gotpal = 0;
static long nullpic [64+1][64]; //Null set icon (image not found)
//static __forceinline unsigned int bsf (unsigned int a) { _asm bsf eax, a }
//static __forceinline unsigned int bsr (unsigned int a) { _asm bsr eax, a }

static long crctab32[256] = {0};  //SEE CRC32.C
#define updatecrc32(c,crc) crc=(crctab32[((c)^crc)&255]^(((unsigned)crc)>>8))
#define updateadl32(c,crc) \
{  c += (crc&0xffff); if (c   >= 65521) c   -= 65521; \
crc = (crc>>16)+c; if (crc >= 65521) crc -= 65521; \
crc = (crc<<16)+c; \
} \

#define updatecrc32(c,crc) crc=(crctab32[((c)^crc)&255]^(((unsigned)crc)>>8))
static long crc32_getbuf (char *buf, long leng)
{
	long i, crc = -1;
	for(i=0;i<leng;i++) updatecrc32(buf[i],crc);
	return(crc);
}
static void initcrc32 (void)
{
	long i, j, k;
	for(i=255;i>=0;i--)
	{
		k = i; for(j=8;j;j--) k = ((unsigned long)k>>1)^((-(k&1))&0xedb88320);
		crctab32[i] = k;
	}
}

static long getcrc32z (long crc32, unsigned char *buf) { long i; for(i=0;buf[i];i++) updatecrc32(buf[i],crc32); return(crc32); }

static void compacttilelist_tilenums_imp (mapstate_t *map) //uses gtile[?].namcrc32 as the lut - a complete hack to avoid extra allocs :P
{
    sect_t *sec;
    long s, w;

    sec = map->sect;
    for(s=map->numsects-1;s>=0;s--)
    {
        for(w=2-1;w>=0;w--)        { sec[s].surf[w].tilnum      = gtile[sec[s].surf[w].tilnum].namcrc32; }
        for(w=sec[s].n-1;w>=0;w--) { sec[s].wall[w].surf.tilnum = gtile[sec[s].wall[w].surf.tilnum].namcrc32; }
#ifndef STANDALONE
        for(w=sec[s].headspri;w>=0;w=map->spri[w].sectn)
            if (map->spri[w].tilnum >= 0) map->spri[w].tilnum = gtile[map->spri[w].tilnum].namcrc32;
    }
#else
    }
    for(w=map->numspris-1;w>=0;w--)
        if ((unsigned)map->spri[w].tilnum < (unsigned)gnumtiles) map->spri[w].tilnum = gtile[map->spri[w].tilnum].namcrc32;
 //  for(w=0;w<numplayers;w++)
 //  {
 //      if ((unsigned)map->p[w].copysurf[0].tilnum < (unsigned)gnumtiles) map->p[w].copysurf[0].tilnum = gtile[map->p[w].copysurf[0].tilnum].namcrc32;
 //      if ((unsigned)map->p[w].copyspri[0].tilnum < (unsigned)gnumtiles) map->p[w].copyspri[0].tilnum = gtile[map->p[w].copyspri[0].tilnum].namcrc32;
 //  }
#endif
}
static void compacttilelist_imp (long flags, mapstate_t* map)
{
	sect_t *sec;
	long i, j, s, w, nnumtiles;

	sec = map->sect;

	//gtile[?].namcrc32 used as temp in this function (must be reconstructed before returning)

	if (flags&1) //Remove duplicate filenames (call right after load with alt+sectors copied)
	{
		for(i=gnumtiles-1;i>=0;i--) gtile[i].namcrc32 = i;
		for(s=0;s<sizeof(gtilehashead)/sizeof(gtilehashead[0]);s++)
			for(i=gtilehashead[s];i>=0;i=gtile[i].hashnext) //n^2 compare on linked list
			{
				if (!gtile[i].filnam[0]) continue;
				for(j=gtile[i].hashnext;j>=0;j=gtile[j].hashnext)
					if (!stricmp(gtile[i].filnam,gtile[j].filnam))
					{
						if (gtile[j].tt.f) { free((void *)gtile[j].tt.f); gtile[j].tt.f = 0; }
						gtile[j].filnam[0] = 0;
						gtile[j].namcrc32 = i;
					}
			}
		compacttilelist_tilenums_imp(map);

		nnumtiles = 0;
		for(i=0;i<gnumtiles;i++)
		{
			if (!gtile[i].filnam[0]) continue;
			j = gtile[nnumtiles].namcrc32; gtile[nnumtiles] = gtile[i]; gtile[nnumtiles].namcrc32 = j; //copy all except namcrc32
			gtile[i].namcrc32 = nnumtiles; nnumtiles++;
		}
		if (nnumtiles != gnumtiles) { compacttilelist_tilenums_imp(map); gnumtiles = nnumtiles; }
	}

	if (flags&2) //Remove unused tiles (call just before save)
	{
		for(i=0;i<gnumtiles;i++) gtile[i].namcrc32 = 0;
		gtile[0].namcrc32 = 1; //Keep default tile (cloud.png)
		for(s=map->numsects-1;s>=0;s--)
		{
			for(w=2-1;w>=0;w--) gtile[sec[s].surf[w].tilnum].namcrc32 = 1;
			for(w=sec[s].n-1;w>=0;w--) gtile[sec[s].wall[w].surf.tilnum].namcrc32 = 1;
#ifndef STANDALONE
			for(w=sec[s].headspri;w>=0;w=map->spri[w].sectn)
				if (map->spri[w].tilnum >= 0) gtile[map->spri[w].tilnum].namcrc32 = 1;
		}
#else
		}
		for(w=map->numspris-1;w>=0;w--)
			if ((unsigned)map->spri[w].tilnum < (unsigned)gnumtiles)
				gtile[map->spri[w].tilnum].namcrc32 = 1;
#endif
		nnumtiles = 0;
		for(i=0;i<gnumtiles;i++)
		{
			if (!gtile[i].namcrc32) continue;
			j = gtile[nnumtiles].namcrc32; gtile[nnumtiles] = gtile[i]; gtile[nnumtiles].namcrc32 = j; //copy all except namcrc32
			gtile[i].namcrc32 = nnumtiles; nnumtiles++;
		}
		if (nnumtiles != gnumtiles) { compacttilelist_tilenums_imp(map); gnumtiles = nnumtiles; }
	}

	if (flags&3) //Reconstruct namcrc32's and hash table from scratch
	{
		memset(gtilehashead,-1,sizeof(gtilehashead));
		for(i=0;i<gnumtiles;i++)
		{
			gtile[i].namcrc32 = getcrc32z(0,(unsigned char *)gtile[i].filnam);
			j = (gtile[i].namcrc32&(sizeof(gtilehashead)/sizeof(gtilehashead[0])-1));
			gtile[i].hashnext = gtilehashead[j]; gtilehashead[j] = i;
		}
	}
}
static long gettileind (char *st)
{
	long i, crc32, hashind;

	crc32 = getcrc32z(0,(unsigned char *)st); hashind = (crc32&(sizeof(gtilehashead)/sizeof(gtilehashead[0])-1));
	for(i=gtilehashead[hashind];i>=0;i=gtile[i].hashnext)
	{
		if (gtile[i].namcrc32 != crc32) continue;
		if (!stricmp(gtile[i].filnam,st)) return(i);
	}
	if (gnumtiles >= gmaltiles) { gmaltiles = max(gnumtiles+1,gmaltiles<<1); gtile = (tile_t *)realloc(gtile,gmaltiles*sizeof(tile_t)); }
	strcpy(gtile[gnumtiles].filnam,st);
	gtile[gnumtiles].namcrc32 = crc32;
	gtile[gnumtiles].hashnext = gtilehashead[hashind]; gtilehashead[hashind] = gnumtiles;
	gtile[gnumtiles].tt.f = 0;
	gnumtiles++;
	return(gnumtiles-1);
}
void loadpic_imp (tile_t *tpic, mapstate_t* gst);
static int arewallstouching (int s0, int w0, int s1, int w1, mapstate_t* map)
{
	sect_t *sec;
	float x[4], y[4];
	int i;

	sec = map->sect;

	x[0] = sec[s0].wall[w0].x; y[0] = sec[s0].wall[w0].y; i = sec[s0].wall[w0].n+w0;
	x[1] = sec[s0].wall[i ].x; y[1] = sec[s0].wall[i ].y;
	x[2] = sec[s1].wall[w1].x; y[2] = sec[s1].wall[w1].y; i = sec[s1].wall[w1].n+w1;
	x[3] = sec[s1].wall[i ].x; y[3] = sec[s1].wall[i ].y;

	//Make sure x's & y's match (front or back)
	if ((x[0] == x[2]) && (y[0] == y[2])) { if ((x[1] != x[3]) || (y[1] != y[3])) return(0); }
	else { if ((x[0] != x[3]) || (y[0] != y[3]) || (x[1] != x[2]) || (y[1] != y[2])) return(0); }

	//Connect walls only if their z's cross
	for(i=1;i>=0;i--)
		if (max(getslopez(&sec[s0],0,x[i],y[i]),getslopez(&sec[s1],0,x[i],y[i])) <=
			 min(getslopez(&sec[s0],1,x[i],y[i]),getslopez(&sec[s1],1,x[i],y[i]))) return(1);

	return(0);
}


static void checknextwalls_imp (mapstate_t *map)
{
#if 0
	sect_t *sec;
	float f, x0, y0, x1, y1;
	int s0, w0, w0n, s1, w1, w1n;

	sec = map->sect;

		//Clear all nextsect/nextwalls
	for(s0=0;s0<map->numsects;s0++)
		for(w0=0;w0<sec[s0].n;w0++) sec[s0].wall[w0].ns = sec[s0].wall[w0].nw = -1;

	for(s1=1;s1<map->numsects;s1++)
		for(w1=0;w1<sec[s1].n;w1++)
		{
			x0 = sec[s1].wall[w1].x;  y0 = sec[s1].wall[w1].y; w1n = sec[s1].wall[w1].n+w1;
			x1 = sec[s1].wall[w1n].x; y1 = sec[s1].wall[w1n].y;
			for(s0=0;s0<s1;s0++)
				for(w0=0;w0<sec[s0].n;w0++)
					if ((sec[s0].wall[w0].x == x1) && (sec[s0].wall[w0].y == y1))
					{
						w0n = sec[s0].wall[w0].n+w0;
						if ((sec[s0].wall[w0n].x == x0) && (sec[s0].wall[w0n].y == y0))
						{
							sec[s1].wall[w1].ns = s0; //FIX: obsolete: doesn't support SOS
							sec[s1].wall[w1].nw = w0;
							sec[s0].wall[w0].ns = s1;
							sec[s0].wall[w0].nw = w1;
							goto cnw_break2;
						}
					}
cnw_break2:;
		}
#else
	typedef struct { int w, s; float minpt; } cvertlist_t;
	cvertlist_t *hashead, *hashlist, *subhashlist, vertemp;
	int *hashsiz;
	sect_t *sec;
	float f; //WARNING: keep f float for hash trick!
	float x0, y0, x1, y1, fz[8];
	int i, j, k, m, n, r, w, s0, w0, w0n, s1, w1, w1n, s2, w2, lhsiz, hsiz, numwalls, maxchainleng;
	int gap, z, zz, subn;

	sec = map->sect;

	for(s0=0,numwalls=0;s0<map->numsects;s0++) numwalls += sec[s0].n;

	for(lhsiz=4,hsiz=(1<<lhsiz);(hsiz<<1)<numwalls;lhsiz++,hsiz<<=1); //hsiz = 0.5x to 1.0x of numwalls
	hashead = (cvertlist_t *)_alloca(hsiz*sizeof(hashead[0])); memset(hashead,-1,hsiz*sizeof(hashead[0]));
	hashsiz = (int         *)_alloca(hsiz*sizeof(hashsiz[0])); memset(hashsiz, 0,hsiz*sizeof(hashsiz[0]));

	maxchainleng = 0;
	for(s0=0;s0<map->numsects;s0++)
		for(w0=0;w0<sec[s0].n;w0++)
		{
			i = 0; w0n = sec[s0].wall[w0].n+w0;
				//Hash must give same values if w0 and w0n are swapped (commutativity)
			f = sec[s0].wall[w0].x*sec[s0].wall[w0n].x + sec[s0].wall[w0].y*sec[s0].wall[w0n].y;
			k = *(long *)&f;
			//k ^= (*(long *)&sec[s0].wall[w0].x) ^ (*(long *)&sec[s0].wall[w0n].x);
			//k ^= (*(long *)&sec[s0].wall[w0].y) ^ (*(long *)&sec[s0].wall[w0n].y);
			for(j=lhsiz;j<32;j+=lhsiz) i -= (k>>j);
			i &= (hsiz-1);

			sec[s0].wall[w0].ns = hashead[i].s; hashead[i].s = s0;
			sec[s0].wall[w0].nw = hashead[i].w; hashead[i].w = w0;
			hashsiz[i]++; if (hashsiz[i] > maxchainleng) maxchainleng = hashsiz[i];
		}

	//hashhead -> s0w0 -> s1w1 -> s2w2 -> s3w3 -> s4w4 -> -1
	//              A       B       A               B

	hashlist = (cvertlist_t *)_alloca(maxchainleng*sizeof(hashlist[0]));

	//printf("maxchainleng=%d\n",maxchainleng); //FIX

	for(i=0;i<hsiz;i++)
	{
		n = 0;
		s0 = hashead[i].s;
		w0 = hashead[i].w;
		while (s0 >= 0)
		{
			hashlist[n].s = s0;
			hashlist[n].w = w0;

				//for 2nd-level hash!
			w0n = sec[s0].wall[w0].n+w0; x0 = sec[s0].wall[w0].x; x1 = sec[s0].wall[w0n].x;
			if (x0 != x1) hashlist[n].minpt = min(x0,x1);
						else hashlist[n].minpt = min(sec[s0].wall[w0].y,sec[s0].wall[w0n].y);
			n++;

				//Easier to join chains if inited as pointing to self rather than -1
			s1 = sec[s0].wall[w0].ns; sec[s0].wall[w0].ns = s0;
			w1 = sec[s0].wall[w0].nw; sec[s0].wall[w0].nw = w0;
			s0 = s1; w0 = w1;
		}

		if (n >= 2)
		{
				//Sort points by y's
			for(gap=(n>>1);gap;gap>>=1)
				for(z=0;z<n-gap;z++)
					for(zz=z;zz>=0;zz-=gap)
					{
						if (hashlist[zz].minpt <= hashlist[zz+gap].minpt) break;
						vertemp = hashlist[zz]; hashlist[zz] = hashlist[zz+gap]; hashlist[zz+gap] = vertemp;
					}

			//printf("//n=%d\n",n); //FIX

			for(zz=n,z=n-1;z>=0;z--)
			{
				if ((z) && (hashlist[z-1].minpt == hashlist[z].minpt)) continue;
				subhashlist = &hashlist[z]; subn = zz-z; zz = z;

					//Example: (sector walls overlapping, drawn sideways)
					//   AAA EEE DDD
					//     BBB CCC
					//                                  0    1    2    3    4
					//j=?,w=4,r=5,n=5, s0:?, s1:?, {A->A,B->B,C->C,D->D,E->E}
					//j=?,w=4,r=4,n=5, s0:E, s1:?, {A->A,B->B,C->C,D->D,E->E}
					//j=3,w=4,r=4,n=5, s0:E, s1:D, {A->A,B->B,C->C,D->D,E->E}
					//j=2,w=3,r=4,n=5, s0:E, s1:C, {A->A,B->B,D->D|C->E,E->C}
					//j=1,w=2,r=4,n=5, s0:E, s1:B, {A->A,D->D|B->C,C->E,E->B}
					//j=0,w=2,r=4,n=5, s0:E, s1:A, {A->A,D->D|B->C,C->E,E->B}
					//j=1,w=1,r=3,n=5, s0:C, s1:D, {A->A|D->E,B->C,C->D,E->B}
					//j=0,w=1,r=3,n=5, s0:C, s1:A, {A->A|D->E,B->C,C->D,E->B}
					//j=0,w=0,r=2,n=5, s0:B, s1:A, {A->C,D->E,B->A,C->D,E->B}

					//     s0    s1  s3  s2
					//  +-------+---+---+---+
					//  |   1   | 7 | D | 8 |
					//  |0     2|6 4|C E|B 9|
					//  |   3   | 5 | F | A |
					//  +-------+---+---+---+
					//                           s0,w0  s1,w1
					//cmp: i=5 ,j=1,w=2,r=2,n=3   0,1,   2,3  no
					//cmp: i=5 ,j=0,w=2,r=2,n=3   0,1,   3,2  no
					//cmp: i=5 ,j=0,w=1,r=1,n=3   2,3,   3,2  yes

					//cmp: i=6 ,j=0,w=1,r=1,n=2   0,0,   3,1  no

					//cmp: i=8 ,j=1,w=2,r=2,n=3   0,2,   1,2  yes
					//cmp: i=8 ,j=0,w=1,r=2,n=3   0,2,   2,2  no
					//cmp: i=8 ,j=0,w=1,r=1,n=3   1,2,   2,2  no

					//cmp: i=10,j=0,w=1,r=1,n=2   1,0,   3,0  yes

					//Graph search and connect: fifo is hashlist itself
					//      (write) (read) (total)
					//  0      w      r      n
					//   (left) (fifo) (done)
				//printf("//i=%d:n=%d\n",i,subn); //FIX
					//FIX
				//for(j=0;j<subn;j++)
				//{
				//   s0 = subhashlist[j].s;
				//   w0 = subhashlist[j].w;
				//   x0 = sec[s0].wall[w0].x; x1 = sec[s0].wall[w0n].x; w0n = sec[s0].wall[w0].n+w0;
				//   y0 = sec[s0].wall[w0].y; y1 = sec[s0].wall[w0n].y;
				//   printf("   %2d: %6.1f %6.1f %6.1f %6.1f | %6.1f\n",j,x0,y0,x1,y1,subhashlist[j].minpt);
				//}

				w = subn-1; r = subn;
				while (w > 0)
				{
					r--;
					s0 = subhashlist[r].s;
					w0 = subhashlist[r].w;
					for(j=w-1;j>=0;j--)
					{
						s1 = subhashlist[j].s; if (s0 == s1) continue; //Don't allow 2-vertex loops to become red lines
						w1 = subhashlist[j].w;
						//printf("//   cmp: j=%2d,w=%2d,r=%2d, %3d,%3d, %3d,%3d, ",j,w,r,s0,w0,s1,w1); //FIX
						if (!arewallstouching(s0,w0,s1,w1,map)) { /*printf("no\n");FIX*/ continue; }
						//printf("yes\n"); //FIX

						s2 = sec[s0].wall[w0].ns;
						w2 = sec[s0].wall[w0].nw;
							  sec[s0].wall[w0].ns = sec[s1].wall[w1].ns;
							  sec[s0].wall[w0].nw = sec[s1].wall[w1].nw;
															sec[s1].wall[w1].ns = s2;
															sec[s1].wall[w1].nw = w2;
						w--; if (w == j) continue;
						vertemp = subhashlist[w];
									 subhashlist[w] = subhashlist[j];
															subhashlist[j] = vertemp;
					}
					if (r == w) w--;
				}
			}
		}

			//convert disjoint walls (self-linked) back to -1's
		for(j=n-1;j>=0;j--)
		{
			s0 = hashlist[j].s; w0 = hashlist[j].w;
			if ((sec[s0].wall[w0].ns == s0) && (sec[s0].wall[w0].nw == w0))
				{ sec[s0].wall[w0].ns = sec[s0].wall[w0].nw = -1; }
		}
	}
#endif
}


int loadmap_imp (char *filnam, mapstate_t* map);

#endif //BUILD2_LOADERS_H