//
// Created by omnis on 10/20/2025.
//

#ifndef BUILD2_LOADERS_H
#define BUILD2_LOADERS_H
#include "mapcore.h"
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
static void crc32_init (void)
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






#endif //BUILD2_LOADERS_H

