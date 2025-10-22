//
// Created by omnis on 10/20/2025.
//
#ifndef BUILD2_LOADERS_H
#define BUILD2_LOADERS_H
// # Prio 1 for Eugene
// finish xsurf implementation
// attempt portals

static char curmappath[MAX_PATH+1] = "";
static unsigned char gammlut[256], gotpal = 0;
static long nullpic [64+1][64]; //Null set icon (image not found)
static __forceinline unsigned int bsf (unsigned int a) { _asm bsf eax, a }
static __forceinline unsigned int bsr (unsigned int a) { _asm bsr eax, a }
#include "mapcore.h"
#include "kplib.h"
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

static int loadmap (char *filnam)
{
	surf_t *sur;
	sect_t *sec;
	wall_t *wal;
	spri_t *spr;
	float f, fx, fy;
	int i, j, k, l;
	long x, y, z, fileid, hitile, warned = 0, altsects, nnumtiles, nnumspris;
	short s, cursect;
	char och, tbuf[256];

	if (!kzopen(filnam))
	{     //Try without full pathname - see if it's in ZIP/GRP/Mounted_Dir
		for(i=j=0;filnam[i];i++) if ((filnam[i] == '/') || (filnam[i] == '\\')) j = i+1;
		if (!j) return(0);
		filnam = &filnam[j];
		if (!kzopen(filnam)) return(0);
	}
	kzread(&fileid,4);
	if ((fileid == 0x04034b50) || (fileid == 0x536e654b)) //'PK\3\4' is ZIP file id, 'KenS' is GRP file id
		{ kzclose(); kzaddstack(filnam); return(1); }
	sec = gst->sect; gst->light_sprinum = 0;
	if (fileid == 0x3142534b) //KSB1
	{
		typedef struct { long tilnum, flags, tag; point2d uv[3]; int dummy[6]; short asc, rsc, gsc, bsc; } surf1_t;
		typedef struct { float x, y; long n, ns, nw; surf1_t surf; } wall1_t;
		typedef struct { float z[2]; point2d grad[2]; surf1_t surf[2]; long foglev; wall1_t *wall; int n, nmax; } sect1_t;
		surf1_t surf1;
		wall1_t wall1;
		sect1_t sect1;

		for(i=gst->numsects-1;i>=0;i--)
			if (gst->sect[i].wall) { free(gst->sect[i].wall); gst->sect[i].wall = 0; }
		kzread(&gst->numsects,4);
		if (gst->numsects > gst->malsects)
		{
			i = gst->malsects; gst->malsects = max(gst->numsects+1,gst->malsects<<1);
			sec = gst->sect = (sect_t *)realloc(sec,gst->malsects*sizeof(sect_t));
			memset(&sec[i],0,(gst->malsects-i)*sizeof(sect_t));
		}
		memset(sec,0,sizeof(sect_t)*gst->numsects);
		for(i=0;i<gst->numsects;i++)
		{
			kzread(&sect1,sizeof(sect1_t));
			for(j=0;j<2;j++)
			{
				sec[i].z[j] = sect1.z[j];
				sec[i].grad[j] = sect1.grad[j];
				//for(k=0;k<3;k++) sec[i].surf[j].uv[k] = sect1.surf[j].uv[k];
				sec[i].surf[j].uv[1].x = sec[i].surf[j].uv[2].y = 1.f;
				sec[i].surf[j].asc = sect1.surf[j].asc;
				sec[i].surf[j].rsc = sect1.surf[j].rsc;
				sec[i].surf[j].gsc = sect1.surf[j].gsc;
				sec[i].surf[j].bsc = sect1.surf[j].bsc;
				sec[i].headspri = -1;
				sec[i].owner = -1;
			}
			sec[i].n = sect1.n;
			sec[i].nmax = sect1.nmax;
		}
		for(i=0;i<gst->numsects;i++)
		{
			sec[i].wall = (wall_t *)malloc(sec[i].nmax*sizeof(wall_t));
			memset(sec[i].wall,0,sec[i].nmax*sizeof(wall_t));
			for(j=0;j<sec[i].n;j++)
			{
				kzread(&wall1,sizeof(wall1_t));
				wal = sec[i].wall;
				wal[j].x = wall1.x;
				wal[j].y = wall1.y;
				wal[j].n = wall1.n;
				wal[j].ns = wall1.ns;
				wal[j].nw = wall1.nw;
				if (!stricmp(&filnam[max(strlen(filnam)-13,0)],"sos_test3.map"))
					  { for(k=0;k<3;k++) wal[j].surf.uv[k] = wall1.surf.uv[k]; }
				else { wal[j].surf.uv[1].x = wal[j].surf.uv[2].y = 1.f; }
				wal[j].surf.asc = wall1.surf.asc;
				wal[j].surf.rsc = wall1.surf.rsc;
				wal[j].surf.gsc = wall1.surf.gsc;
				wal[j].surf.bsc = wall1.surf.bsc;
				wal[j].surfn = 1;
				wal[j].owner = -1;
			}
		}

		gst->numspris = 0;

#ifdef STANDALONE
	//	for(i=numplayers-1;i>=0;i--) gst->p[i].sec.n = 0;
#endif
		checknextwalls();
		checksprisect(-1);
		kzclose();
		return(1);
	}
	else if (fileid == 0x3242534b) //KSB2 (current BUILD2 map format)
	{
		kzread(&gst->startpos,sizeof(gst->startpos));
		kzread(&gst->startrig,sizeof(gst->startrig));
		kzread(&gst->startdow,sizeof(gst->startdow));
		kzread(&gst->startfor,sizeof(gst->startfor));
	//	for(i=numplayers-1;i>=0;i--)
	//	{
	//		gst->p[i].ipos = gst->startpos;
	//		gst->p[i].ifor = gst->startfor;
	//		gst->p[i].irig = gst->startrig;
	//		gst->p[i].idow = gst->startdow;
	//		gst->p[i].cursect = -1;
	//	}

			//Load sectors
		altsects = 0;
		for(i=0;i<gst->numsects;i++)
		{
			if (sec[i].owner < 0)
			{
				while (sec[i].headspri >= 0) delspri(sec[i].headspri);
				if (gst->sect[i].wall) { free(gst->sect[i].wall); gst->sect[i].wall = 0; }
				continue;
			}
			for(j=sec[i].headspri;j>=0;j=gst->spri[j].sectn) gst->spri[j].sect = altsects;
			memcpy(&sec[altsects],&sec[i],sizeof(sect_t)); altsects++;
		}
		kzread(&i,4); gst->numsects = i+altsects;
		if (gst->numsects > gst->malsects)
		{
			i = gst->malsects; gst->malsects = max(gst->numsects+1,gst->malsects<<1);
			sec = gst->sect = (sect_t *)realloc(sec,gst->malsects*sizeof(sect_t));
			memset(&sec[i],0,(gst->malsects-i)*sizeof(sect_t));
		}
		kzread(&sec[altsects],(gst->numsects-altsects)*sizeof(sect_t));

			//Load walls
		for(i=altsects;i<gst->numsects;i++)
		{
			sec[i].wall = (wall_t *)malloc(sec[i].nmax*sizeof(wall_t));
			sec[i].owner = -1;
			for(j=0;j<sec[i].n;j++)
			{
				kzread(&sec[i].wall[j],sizeof(wall_t));

				if (!sec[i].wall[j].n)
				{
					if (!warned)
					{
						warned = 1;
						if (MessageBox(ghwnd,"Your map appears to be corrupt. Load anyway?",prognam,MB_YESNO) == IDNO)
						{
							for(;i>=0;i--) free(sec[i].wall);
							gst->numsects = 0;
							return(-1);
						}
					}
				}

				sec[i].wall[j].owner = -1;
				if (sec[i].wall[j].surfn > 1)
				{
					sec[i].wall[j].xsurf = (surf_t *)malloc((sec[i].wall[j].surfn-1)*sizeof(surf_t));
					kzread(sec[i].wall[j].xsurf,(sec[i].wall[j].surfn-1)*sizeof(surf_t));
				}
			}
		}

			//Load tiles
		kzread(&nnumtiles,4); gnumtiles += nnumtiles;
		if (gnumtiles > gmaltiles)
		{
			gmaltiles = max(gnumtiles+1,gmaltiles<<1);
			gtile = (tile_t *)realloc(gtile,gmaltiles*sizeof(tile_t));
		}
		for(i=gnumtiles-nnumtiles;i<gnumtiles;i++)
		{
			kzread(&s,2); kzread(gtile[i].filnam,s); gtile[i].filnam[s] = 0; //FIX:possible buffer overflow here
			gtile[i].tt.f = 0;
			gtile[i].namcrc32 = getcrc32z(0,(unsigned char *)gtile[i].filnam);
		}

			//Load sprites
		kzread(&nnumspris,4); gst->numspris += nnumspris;
		if (!nnumspris) for(i=0;i<gst->numsects;i++) { sec[i].headspri = -1; sec[i].owner = -1; } //Hack for loading old format
		if (gst->numspris > gst->malspris)
		{
			i = gst->malspris;
			gst->malspris = max(gst->numspris+1,gst->malspris<<1);
			gst->spri = (spri_t *)realloc(gst->spri,gst->malspris*sizeof(spri_t));
#ifndef STANDALONE
			for(;i<gst->malspris;i++) gst->spri[i].sect = -1;
#endif
		}
		kzread(&gst->spri[gst->numspris-nnumspris],nnumspris*sizeof(spri_t));
		for(i=gst->numspris-nnumspris;i<gst->numspris;i++) gst->spri[i].sect += altsects;


			// | 0 ..       altsects ..  gst->numsects   |
			// |   ^old_sects^    |     ^new_sects^      |
			//
			// |0..gst->numspris-nnumspris..gst->numspris|
			// |  ^old_sprites^   |    ^new_sprites^     |
			//
			// | 0 ..  gnumtiles-nnumtiles .. gnumtiles  |
			// |   ^old_tiles^    |     ^new_tiles^      |

			//Adjust tile indices for new sectors(/walls) & sprites
		for(i=altsects;i<gst->numsects;i++)
		{
			for(j=0;j<2       ;j++) sec[i].surf[j].tilnum      += gnumtiles-nnumtiles;
			for(j=0;j<sec[i].n;j++) sec[i].wall[j].surf.tilnum += gnumtiles-nnumtiles;
		}
		for(i=gst->numspris-nnumspris;i<gst->numspris;i++) if (gst->spri[i].tilnum >= 0) gst->spri[i].tilnum += gnumtiles-nnumtiles;

		//-------------------------------------------------------------------

			//Sprite hacks
		for(i=0;i<gst->numspris;i++)
		{
			gst->spri[i].owner = -1;

				//Insert lights
			if (gst->spri[i].flags&(1<<16))
			{
				if (gst->light_sprinum < MAXLIGHTS) gst->light_spri[gst->light_sprinum++] = i;
			}
		}

#ifdef STANDALONE
// mp ommited
		//	for(i=numplayers-1;i>=0;i--) gst->p[i].sec.n = 0;
#endif
		checknextwalls();
		checksprisect(-1);

#if 0
			//Rebuild hash table from scratch
		memset(gtilehashead,-1,sizeof(gtilehashead));
		for(i=0;i<gnumtiles;i++)
		{
			j = (gtile[i].namcrc32&(sizeof(gtilehashead)/sizeof(gtilehashead[0])-1));
			gtile[i].hashnext = gtilehashead[j]; gtilehashead[j] = i;
		}
#else
		compacttilelist_imp(1, (mapstate_t*)gst);
#endif

		kzclose();
		return(1);
	}
	else if ((fileid == 0x00000007) || //Build1 .MAP format 7
				(fileid == 0x00000cbe))   //Cubes5 .CUB format
	{
			//Build1 format variables:
		typedef struct { short picnum, heinum; signed char shade; char pal, xpanning, ypanning; } build7surf_t;
		typedef struct
		{
			short wallptr, wallnum;
			long z[2]; short stat[2]; build7surf_t surf[2];
			char visibility, filler;
			short lotag, hitag, extra;
		} build7sect_t;
		typedef struct
		{
			long x, y;
			short point2, nextwall, nextsect, cstat, picnum, overpicnum;
			signed char shade;
			char pal, xrepeat, yrepeat, xpanning, ypanning;
			short lotag, hitag, extra;
		} build7wall_t;
		typedef struct
		{
			long x, y, z; short cstat, picnum;
			signed char shade; char pal, clipdist, filler;
			unsigned char xrepeat, yrepeat; signed char xoffset, yoffset;
			short sectnum, statnum, ang, owner, xvel, yvel, zvel, lotag, hitag, extra;
		} build7spri_t;
		build7sect_t b7sec;
		build7wall_t b7wal;
		build7spri_t b7spr;

			//Cubes5 format variables:
		#define BSIZ 16
		double c1, c2, c3, s1, s2, s3, c1c3, c1s3, s1c3, s1s3;
		signed short board[6][BSIZ][BSIZ][BSIZ]; //Board layout
		long posx, posy, posz, a1, a2, a3, oy, yy;

		//------------------------------------------------------------------------
		long filnum, arttiles, loctile0, loctile1, iskenbuild = 0;
		short *tilesizx = 0, *tilesizy = 0, *tilefile = 0;
		char tbuf[MAX_PATH*2];

		kzclose();

		strcpy(curmappath,filnam);
		for(i=j=0;curmappath[i];i++) if ((curmappath[i] == '/') || (curmappath[i] == '\\')) j = i+1;
		curmappath[j] = 0;

		arttiles = 0; //Scan .ART files, incrementing number until tile is in range
		for(filnum=0;1;filnum++)
		{
			sprintf(tbuf,"TILES%03d.ART",filnum);
			if (!kzopen(tbuf))
			{
				sprintf(tbuf,"%sTILES%03d.ART",curmappath,filnum);
				if (!kzopen(tbuf)) break;
			}
			kzread(tbuf,16); if (*(long *)&tbuf[0] != 1) break;
			loctile0 = *(long *)&tbuf[8];
			loctile1 = (*(long *)&tbuf[12])+1;
			if ((loctile0 < 0) || (loctile1 <= arttiles) || (loctile0 >= loctile1)) continue;
			i = arttiles; arttiles = loctile1;
			tilesizx = (short *)realloc(tilesizx,arttiles*sizeof(tilesizx[0]));
			tilesizy = (short *)realloc(tilesizy,arttiles*sizeof(tilesizy[0]));
			tilefile = (short *)realloc(tilefile,arttiles*sizeof(tilefile[0]));
			for(;i<arttiles;i++) { tilesizx[i] = 0; tilesizy[i] = 0; tilefile[i] = 0; }
			kzread(&tilesizx[loctile0],(loctile1-loctile0)*sizeof(short));
			kzread(&tilesizy[loctile0],(loctile1-loctile0)*sizeof(short));
			for(i=loctile0;i<loctile1;i++) tilefile[i] = filnum;
		}
		if (!arttiles)
		{
			tilesizx = (short *)malloc(sizeof(tilesizx[0]));
			tilesizy = (short *)malloc(sizeof(tilesizy[0]));
			tilefile = (short *)malloc(sizeof(tilefile[0]));
			tilesizx[0] = tilesizy[0] = 2; tilefile[0] = 0; arttiles = 1;
		}
		else if (arttiles >= 20) //Autodetect KenBuild data
		{
			for(i=24-1;i>=0;i--) //If the sizes of the 1st 24 tiles match that of Kenbuild, then that's what it is
			{
				x = 32; if (i == 4)               x = 16; if (i >= 20) x = 64;
				y = 32; if ((i == 3) || (i == 4)) y = 16; if (i >= 18) y = 64;
				if ((tilesizx[i] != x) || (tilesizy[i] != y)) break;
			}
			if (i < 0) iskenbuild = 1;
		}

		kzclose();
		kzopen(filnam);
		kzread(&i,4);
		//------------------------------------------------------------------------

		hitile = 0;

		if (fileid == 0x00000007) //Build1 .MAP format 7
		{
			kzread(&x,4); //posx
			kzread(&y,4); //posy
			kzread(&z,4); //posz
			kzread(&s,2); //ang
			kzread(&cursect,2); //cursectnum
			gst->startpos.x = ((float)x)*(1.f/512.f);
			gst->startpos.y = ((float)y)*(1.f/512.f);
			gst->startpos.z = ((float)z)*(1.f/(512.f*16.f));
			gst->startfor.x = cos(((float)s)*PI/1024.0);
			gst->startfor.y = sin(((float)s)*PI/1024.0);
			gst->startfor.z = 0.f;
			gst->startrig.x =-gst->startfor.y;
			gst->startrig.y = gst->startfor.x;
			gst->startrig.z = 0.f;
			gst->startdow.x = 0.f;
			gst->startdow.y = 0.f;
			gst->startdow.z = 1.f;
		//	for(i=numplayers-1;i>=0;i--)
		//	{
		//		gst->p[i].ipos = gst->startpos;
		//		gst->p[i].ifor = gst->startfor;
		//		gst->p[i].irig = gst->startrig;
		//		gst->p[i].idow = gst->startdow;
		//		gst->p[i].cursect = cursect;
		//	}

			kzread(&s,2);
			gst->numsects = (int)s; //numsectors
			if (gst->numsects > gst->malsects)
			{
				i = gst->malsects; gst->malsects = max(gst->numsects+1,gst->malsects<<1);
				sec = gst->sect = (sect_t *)realloc(sec,gst->malsects*sizeof(sect_t));
				memset(&sec[i],0,(gst->malsects-i)*sizeof(sect_t));
			}
			for(i=0;i<gst->numsects;i++)
			{
				kzread(&b7sec,sizeof(b7sec));
				sec[i].n = sec[i].nmax = b7sec.wallnum;
				sec[i].wall = (wall_t *)realloc(sec[i].wall,sec[i].nmax*sizeof(wall_t));
				memset(sec[i].wall,0,sec[i].nmax*sizeof(wall_t));
				for(j=0;j<2;j++)
				{
					sec[i].z[j] = ((float)b7sec.z[j])*(1.f/(512.f*16.f));
					sec[i].grad[j].x = sec[i].grad[j].y = 0;
					if (b7sec.stat[j]&2) //Enable slopes flag
						sec[i].grad[j].y = ((float)b7sec.surf[j].heinum)*(1.f/4096.f);
					sur = &sec[i].surf[j];
					sur->flags = 0;
					if (b7sec.stat[j]&1) sur->flags |= (1<<16);
					sur->asc = 4096;
					sur->rsc = (32-b7sec.surf[j].shade)*128;
					sur->gsc = (32-b7sec.surf[j].shade)*128;
					sur->bsc = (32-b7sec.surf[j].shade)*128;
					l = b7sec.surf[j].picnum;
					if ((unsigned)l >= (unsigned)arttiles) l = 0;
					sur->tilnum = l; hitile = max(hitile,l);

					// Convert lotag/hitag to single tag field
					// j=0 is ceiling, j=1 is floor - assign to floor surface only
					if (j == 1) // Floor surface
					{
						// Merge lotag (lower 16 bits) and hitag (upper 16 bits) into single long
						sur->lotag = b7sec.lotag;
						sur->hitag = b7sec.hitag;
					}

					sur->pal = b7sec.surf[j].pal;

					sur->uv[0].x = ((float)b7sec.surf[j].xpanning)/256.0;
					sur->uv[0].y = ((float)b7sec.surf[j].ypanning)/256.0;
					sur->uv[1].y = sur->uv[2].x = 0;
					if (!(b7sec.stat[j]&4))
					{
						sur->uv[1].x = 32.0/((float)tilesizx[l]);
						sur->uv[2].y = 32.0/((float)tilesizy[l]);
					}
					else
					{
						sur->uv[1].x = 32.0/((float)tilesizy[l]);
						sur->uv[2].y = 32.0/((float)tilesizx[l]);
					}
					if (b7sec.stat[j]&8) { sur->uv[1].x *= 2; sur->uv[2].y *= 2; } //double smooshiness
					if (b7sec.stat[j]&16) sur->uv[1].x *= -1; //x-flip
					if (!(b7sec.stat[j]&32)) sur->uv[2].y *= -1; //y-flip
					if (b7sec.stat[j]&64) //relative alignment
					{
						f = ((float)b7sec.surf[j].heinum)*(1.f/4096.f);
						sur->uv[2].y *= -sqrt(f*f + 1.f);
						sur->flags |= 4;
					}
					if (b7sec.stat[j]&4) //swap x&y
					{
						if (((b7sec.stat[j]&16) != 0) != ((b7sec.stat[j]&32) != 0))
							{ sur->uv[1].x *= -1; sur->uv[2].y *= -1; }
						sur->uv[1].y = sur->uv[1].x; sur->uv[1].x = 0;
						sur->uv[2].x = sur->uv[2].y; sur->uv[2].y = 0;
					}

					//FIX:This hack corrects an LHS vs. RHS bug in a later stage of texture mapping (drawsectfill?)
					if (sur->uv[1].x*sur->uv[2].y < sur->uv[1].y*sur->uv[2].x)
						{ sur->uv[2].x *= -1; sur->uv[2].y *= -1; }
				}

				sec[i].headspri = -1;
				sec[i].owner = -1;
				//sec[i].foglev = ?;
			}
			kzread(&s,2); //numwalls
			for(i=k=0;i<gst->numsects;i++)
			{
				for(j=0;j<sec[i].n;j++,k++)
				{
					kzread(&b7wal,sizeof(b7wal));
					sec[i].wall[j].x = ((float)b7wal.x)*(1.f/512.f);
					sec[i].wall[j].y = ((float)b7wal.y)*(1.f/512.f);
					sec[i].wall[j].n = b7wal.point2-k;
					sur = &sec[i].wall[j].surf;
					sur->flags = 0;
					if (b7wal.cstat&1) sur->flags |= 1;

					// flag at byte 1 : double split  = 1, one tile = 0
					// bottom tile is taken from overtile of nextwall(meaning opposite side of the wall)
					// mask tile is undertile field

					sur->lotag = b7wal.lotag;
					sur->hitag = b7wal.hitag;
					sur->pal = b7wal.pal;

					sur->uv[0].x = b7wal.xpanning;
					sur->uv[0].y = b7wal.ypanning;
					sur->uv[1].x = b7wal.xrepeat; if (b7wal.cstat&  8) sur->uv[1].x *= -1;
					sur->uv[1].y = sur->uv[2].x = 0;
					sur->uv[2].y = b7wal.yrepeat; if (b7wal.cstat&256) sur->uv[2].y *= -1;
					if ((b7wal.nextsect < 0) ^ (!(b7wal.cstat&4))) sur->flags ^= 4; //align bot/nextsec
					if (b7wal.cstat&(16+32)) sur->flags |= 32; //bit4:masking, bit5:1-way
					sur->asc = 4096;
					sur->rsc = (32-b7wal.shade)*128;
					sur->gsc = (32-b7wal.shade)*128;
					sur->bsc = (32-b7wal.shade)*128;
					l = b7wal.picnum; if ((unsigned)l >= (unsigned)arttiles) l = 0;
					sur->tilnum = l; hitile = max(hitile,l);
					sec[i].wall[j].surfn = 1;
					sec[i].wall[j].owner = -1;
				}
				// tile adjust?
				for(j=0;j<sec[i].n;j++)
				{
					l = j+sec[i].wall[j].n;
					fx = sec[i].wall[l].x - sec[i].wall[j].x;
					fy = sec[i].wall[l].y - sec[i].wall[j].y;
					f = sqrt(fx*fx + fy*fy);
					sur = &sec[i].wall[j].surf;
					l = sur->tilnum;
					sur->uv[1].x = ((float)sur->uv[1].x*8.0)/(f*((float)tilesizx[l]));
					sur->uv[2].y = ((float)sur->uv[2].y*4.0)/((float)tilesizy[l]);
					sur->uv[0].x = ((float)sur->uv[0].x)/((float)tilesizx[l]);
					sur->uv[0].y = ((float)sur->uv[0].y)/256.f * (1-2*(sur->uv[2].y < 0));
				}

				fx = sec[i].wall[1].y-sec[i].wall[0].y;
				fy = sec[i].wall[0].x-sec[i].wall[1].x;
				f = fx*fx + fy*fy; if (f > 0) f = 1.0/sqrt(f); fx *= f; fy *= f;
				for(j=0;j<2;j++)
				{
					sec[i].grad[j].x = fx*sec[i].grad[j].y;
					sec[i].grad[j].y = fy*sec[i].grad[j].y;
				}
			}

			kzread(&s,2); gst->numspris = (int)s;
			if (gst->numspris > gst->malspris)
			{
				i = gst->malspris;
				gst->malspris = max(gst->numspris+1,gst->malspris<<1);
				gst->spri = (spri_t *)realloc(gst->spri,gst->malspris*sizeof(spri_t));
#ifndef STANDALONE
				for(;i<gst->malspris;i++) gst->spri[i].sect = -1;
#endif
			}
			for(i=0;i<gst->numspris;i++)
			{
				kzread(&b7spr,sizeof(b7spr));
				spr = &gst->spri[i];
				memset(spr,0,sizeof(spri_t));

				l = b7spr.picnum; if ((unsigned)l >= (unsigned)arttiles) l = 0;
				spr->p.x = ((float)b7spr.x)*(1.f/512.f);
				spr->p.y = ((float)b7spr.y)*(1.f/512.f);
				spr->p.z = ((float)b7spr.z)*(1.f/(512.f*16.f));
				spr->flags = 0;
				switch(b7spr.cstat&48)  // https://wiki.eduke32.com/wiki/Cstat_(sprite)
										// 48  =32  +16 wall or  floor only check
					{
					case 0: //Face sprite
						spr->flags |= 16;
						//no break intentional
					case 48: //Voxel sprite
						//no break intentional
					case 16: //Wall sprite
						spr->p.z -= (b7spr.yrepeat/4096.0*(float)tilesizy[l]);
						spr->r.x = sin((float)b7spr.ang*PI/1024.0)*(b7spr.xrepeat/4096.0*(float)tilesizx[l]);
						spr->r.y =-cos((float)b7spr.ang*PI/1024.0)*(b7spr.xrepeat/4096.0*(float)tilesizx[l]);
						spr->d.z = (b7spr.yrepeat/4096.0*(float)tilesizy[l]);
						break;
					case 32: //Floor sprite
						spr->r.x = sin((float)b7spr.ang*PI/1024.0)*(b7spr.xrepeat/4096.0*(float)tilesizx[l]);
						spr->r.y =-cos((float)b7spr.ang*PI/1024.0)*(b7spr.xrepeat/4096.0*(float)tilesizx[l]);
						spr->d.x = cos((float)b7spr.ang*PI/1024.0)*(b7spr.yrepeat/4096.0*(float)tilesizy[l]);
						spr->d.y = sin((float)b7spr.ang*PI/1024.0)*(b7spr.yrepeat/4096.0*(float)tilesizy[l]);
						if (b7spr.cstat&8) { spr->d.x *= -1; spr->d.y *= -1; }
						break;
				}
				if (b7spr.cstat&1) spr->flags |= 1; // blocking
				if (b7spr.cstat&64) spr->flags |= 64; // 1 sided
				if (b7spr.cstat&4) { spr->r.x *= -1; spr->r.y *= -1; spr->r.z *= -1; spr->flags ^= 4; } //&4: x-flipped
				if (b7spr.cstat&8) { spr->d.x *= -1; spr->d.y *= -1; spr->d.z *= -1; spr->flags ^= 4; } //&8: y-flipped?
				if (b7spr.cstat&128) { spr->p.z += (b7spr.yrepeat/4096.0*(float)tilesizy[l]); } //&128: real-centered centering (center at center) - originally half submerged sprite

				if ((unsigned)b7spr.sectnum < (unsigned)gst->numsects) //Make shade relative to sector
				{
					j = b7spr.sectnum; j = 32 - gst->sect[j].surf[gst->sect[j].surf[0].flags&1^1].rsc/128;
					if (iskenbuild) b7spr.shade += j+6;
				}

				spr->f.z=3; // sus
				spr->f.x=cos((float)b7spr.ang*PI/1024.0);
				spr->f.y=sin((float)b7spr.ang*PI/1024.0);
				spr->fat = 0.f;
				spr->asc = 4096;
				spr->rsc = (32-b7spr.shade)*128;
				spr->gsc = (32-b7spr.shade)*128;
				spr->bsc = (32-b7spr.shade)*128;

				spr->mas = spr->moi = 1.0;
				spr->owner = -1;

				spr->tilnum = l; hitile = max(hitile,l);
				spr->sect = b7spr.sectnum;
				spr->sectn = spr->sectp = -1;
				spr->lotag = b7spr.lotag;
				spr->hitag = b7spr.hitag;
				spr->pal = b7spr.pal;
			}
		}
		else //CUBES5 map format (.CUB extension)
		{
			kzread(&x,4); //posx
			kzread(&y,4); //posy
			kzread(&z,4); //posz
			kzread(&a1,4); //angle range: 0-2047
			kzread(&a2,4);
			kzread(&a3,4);
			gst->startpos.x =      ((float)x)*(1.f/1024.f);
			gst->startpos.y = 16.f-((float)z)*(1.f/1024.f);
			gst->startpos.z =      ((float)y)*(1.f/1024.f);
			dcossin((double)a3*PI/1024.0,&c1,&s1);
			dcossin((double)a2*PI/1024.0,&c2,&s2);
			dcossin((double)a1*PI/1024.0,&c3,&s3);
			c1c3 = c1*c3; c1s3 = c1*s3; s1c3 = s1*c3; s1s3 = s1*s3;
			gst->startrig.x = c1c3 + s1s3*s2;
			gst->startrig.y = s1c3 - c1s3*s2;
			gst->startrig.z = -s3*c2;
			gst->startdow.x = c1s3 - s1c3*s2;
			gst->startdow.y = s1s3 + c1c3*s2;
			gst->startdow.z = c3*c2;
			gst->startfor.x = s1*c2;
			gst->startfor.y = -c1*c2;
			gst->startfor.z = s2;
		//	for(i=numplayers-1;i>=0;i--)  //netcode
		//	{
		//		gst->p[i].ipos = gst->startpos;
		//		gst->p[i].irig = gst->startrig;
		//		gst->p[i].idow = gst->startdow;
		//		gst->p[i].ifor = gst->startfor;
		//		gst->p[i].cursect = -1;
		//	}

				//6 faces * BSIZ^3 board map * 2 bytes for picnum
				//if face 0 is -1, the cube is air
			kzread(board,6*BSIZ*BSIZ*BSIZ*sizeof(short));
			gst->numsects = 0;
			for(x=0;x<BSIZ;x++)
				for(z=0;z<BSIZ;z++)
				{
					oy = BSIZ;
					for(y=0;y<BSIZ;y++)
					{
						if (board[0][x][y][z] >= 0) continue;

						if (oy > y) oy = y;
						if ((y < BSIZ-1) && (board[0][x][y+1][z] < 0)) continue;

						if (gst->numsects >= gst->malsects)
						{
							i = gst->malsects; gst->malsects = max(gst->numsects+1,gst->malsects<<1);
							sec = gst->sect = (sect_t *)realloc(sec,gst->malsects*sizeof(sect_t));
							memset(&sec[i],0,(gst->malsects-i)*sizeof(sect_t));
						}

						i = gst->numsects;
						sec[i].n = sec[i].nmax = 4;
						sec[i].wall = (wall_t *)realloc(sec[i].wall,sec[i].nmax*sizeof(wall_t));
						memset(sec[i].wall,0,sec[i].nmax*sizeof(wall_t));
						sec[i].z[0] = (float) oy  ;
						sec[i].z[1] = (float)(y+1);
						for(j=0;j<2;j++)
						{
							sec[i].grad[j].x = sec[i].grad[j].y = 0;
							sur = &sec[i].surf[j];
							sur->uv[1].x = sur->uv[2].y = 1.f;
							sur->asc = 4096;
							sur->rsc = 4096-768+1536*j;
							sur->gsc = 4096-768+1536*j;
							sur->bsc = 4096-768+1536*j;

							if (!j) l = board[4][x][oy-1][z];
								else l = board[1][x][ y+1][z];
							if ((unsigned)l >= (unsigned)arttiles) l = 0;
							sur->tilnum = l; hitile = max(hitile,l);
							sur->uv[1].x = max(64.f/((float)tilesizx[l]),1.f);
							sur->uv[2].y = max(64.f/((float)tilesizy[l]),1.f);
						}
						//sec[i].foglev = ?;
						sec[i].headspri = -1;
						sec[i].owner = -1;

						for(j=0;j<4;j++)
						{
							sec[i].wall[j].x = ((float)(       x+(((j+1)>>1)&1)))*(1.f);
							sec[i].wall[j].y = ((float)(BSIZ-1-z+(( j  )>>1)   ))*(1.f);
							if (j < 3) sec[i].wall[j].n = 1; else sec[i].wall[j].n = -3;
							sec[i].wall[j].surf.uv[1].x = sec[i].wall[j].surf.uv[2].y = 1;
							sec[i].wall[j].surf.asc = 4096;
							sec[i].wall[j].surf.rsc = 4096+(labs(j-1)-1)*512;
							sec[i].wall[j].surf.gsc = 4096+(labs(j-1)-1)*512;
							sec[i].wall[j].surf.bsc = 4096+(labs(j-1)-1)*512;
							sec[i].wall[j].surfn = 1;
							sec[i].wall[j].owner = -1;

							l = -1;
							for(yy=oy;yy<=y;yy++)
							{
								switch (j)
								{
									case 0: if (board[0][x  ][yy][z+1] < 0) break; l = board[2][x  ][yy][z+1]; break;
									case 1: if (board[0][x+1][yy][z  ] < 0) break; l = board[0][x+1][yy][z  ]; break;
									case 2: if (board[0][x  ][yy][z-1] < 0) break; l = board[5][x  ][yy][z-1]; break;
									case 3: if (board[0][x-1][yy][z  ] < 0) break; l = board[3][x-1][yy][z  ]; break;
								}
								if ((unsigned)l < (unsigned)arttiles) break;
							}
							if ((unsigned)l >= (unsigned)arttiles) l = 0;
							sec[i].wall[j].surf.tilnum = l; hitile = max(hitile,l);
							sec[i].wall[j].surf.uv[1].x = max(64.f/((float)tilesizx[l]),1.f);
							sec[i].wall[j].surf.uv[2].y = max(64.f/((float)tilesizy[l]),1.f);
						}
						gst->numsects++;

						oy = BSIZ;
					}
				}

			gst->numspris = 0;
		}

			//Set texture names..
		for(i=gnumtiles-1;i>=0;i--)
			if (gtile[i].tt.f) { free((void *)gtile[i].tt.f); gtile[i].tt.f = 0; }
		gnumtiles = 0; memset(gtilehashead,-1,sizeof(gtilehashead));

		hitile++;
		if (hitile > gmaltiles)
		{
			gmaltiles = hitile;
			gtile = (tile_t *)realloc(gtile,gmaltiles*sizeof(tile_t));
		}
		for(i=0;i<hitile;i++)
		{
			sprintf(tbuf,"tiles%03d.art|%d",tilefile[i],i);
			gettileind(tbuf);
		}

		if (tilesizx) free(tilesizx);
		if (tilesizy) free(tilesizy);
		if (tilefile) free(tilefile);

#ifdef STANDALONE
	//	for(i=numplayers-1;i>=0;i--) gst->p[i].sec.n = 0;
#endif
		checknextwalls();
		checksprisect(-1);
		kzclose();
		return(1);
	}
	else { return(0); } //MessageBox(ghwnd,"Invalid MAP format",prognam,MB_OK);
}

void loadpic_imp (tile_t *tpic)
{
	static unsigned char lastpal[256][4], uch;
	tiltyp *pic;
	long i, j, x, y, filnum, tilenum, loctile0, loctile1, lnx, lny, nx, ny;
	short *sxsiz, *sysiz;
	unsigned char *uptr;
	char tbuf[MAX_PATH*2], tbuf2[MAX_PATH*2];

	pic = &tpic->tt; if (pic->f) return;

	strcpy(tbuf,tpic->filnam);
#if USEGROU

		//.ART loader
	for(i=j=0;tbuf[i];i++) if (tbuf[i] == '|') j = i;
	if (!j) { tilenum = 0; } else { tilenum = atol(&tbuf[j+1]); tbuf[j] = 0; i = j; }
	if ((i >= 5) && (!stricmp(&tbuf[i-4],".ART")))
	{
		if (!gotpal)
		{
			gotpal = 1;
			for(i=j=0;tbuf[i];i++) if ((tbuf[i] == '/') || (tbuf[i] == '\\')) j = i+1;
			strcpy(tbuf2,tbuf);
			strcpy(&tbuf2[j],"palette.dat");
			i = kzopen(tbuf2);
			if (!i)
			{
				strcpy(tbuf2,curmappath); j += strlen(curmappath);
				strcat(tbuf2,tbuf);
				strcpy(&tbuf2[j],"palette.dat");
				i = kzopen(tbuf2);
			}
			if (i)
			{
				kzread(lastpal,768);
				*(long *)&lastpal[255][0] = 0^0xff000000;
				for(i=255-1;i>=0;i--)
				{
					lastpal[i][3] = 255^0xff;
					lastpal[i][2] = gammlut[lastpal[0][i*3+2]<<2];
					lastpal[i][1] = gammlut[lastpal[0][i*3+1]<<2];
					lastpal[i][0] = gammlut[lastpal[0][i*3  ]<<2];
					uch = lastpal[i][0]; lastpal[i][0] = lastpal[i][2]; lastpal[i][2] = uch;
				}
				kzclose();
			}
		}

		filnum = 0; //Scan .ART files, incrementing number until tile is in range
		do
		{
			if (!kzopen(tbuf))
			{
				sprintf(tbuf2,"%s%s",curmappath,tbuf);
				if (!kzopen(tbuf2)) { filnum = -1; break; }
			}
			kzread(tbuf,16); if (*(long *)&tbuf[0] != 1) { filnum = -1; break; }
			loctile0 = *(long *)&tbuf[8];
			loctile1 = (*(long *)&tbuf[12])-loctile0+1;
			i = tilenum-loctile0; if ((unsigned)i < (unsigned)loctile1) { tilenum = i; break; }
			filnum++; sprintf(&tbuf[strlen(tbuf)-7],"%03d.ART",filnum);
		} while (1);
		if (filnum >= 0)
		{
			sxsiz = (short *)_alloca(loctile1<<2); sysiz = &sxsiz[loctile1];
			kzread(sxsiz,loctile1<<2);
			for(i=0,j=16+(loctile1<<3);i<tilenum;i++) j += ((long)sxsiz[i])*((long)sysiz[i]);

			pic->x = (long)sxsiz[tilenum];
			pic->y = (long)sysiz[tilenum];

				//Grab the picanm of the current tile (not currently implemented)
			//kzseek(16+(loctile1<<2)+(tilenum<<2),SEEK_SET);
			//kzread(&i,4);
			//pic->xoffs = ((i<<16)>>24);
			//pic->yoffs = ((i<< 8)>>24);

				//Allocate texture to next higher pow2
			if (pic->x <= 1) lnx = 0; else lnx = bsr(pic->x-1)+1;
			if (pic->y <= 1) lny = 0; else lny = bsr(pic->y-1)+1;
			nx = (1<<lnx); ny = (1<<lny);

			kzseek(j,SEEK_SET);
			uptr = (unsigned char *)_alloca(pic->y);
			pic->p = (nx<<2);
			pic->f = (long)malloc((ny+1)*pic->p+4);

			for(x=0;x<pic->x;x++)
			{
				kzread(uptr,pic->y); i = (x<<2)+pic->f;
				for(y=0;y<pic->y;y++,i+=pic->p) *(long *)i = *(long *)&lastpal[(long)uptr[y]][0];
			}
			kzclose();

				//Scale texture to next higher pow2. Uses box_sum_mip (no bilinear)
			if ((pic->x != nx) || (pic->y != ny))
			{
				tiltyp pow2t;
				pow2t.f = pic->f; pow2t.p = pic->p; pow2t.x = nx; pow2t.y = ny;
				scaletex_boxsum((tiltyp *)pic,&pow2t);
				pic->x = nx; pic->y = ny;
			}

			fixtex4grou((tiltyp *)pic);
			pic->lowermip = 0;
		}
	}
	else
	{
		tiltyp gtt; //FIXFIX
		kpzload4grou(tbuf,&gtt,1.0,2);
		//applyshade(&gtt,1<<14,1<<14); fixtex4grou(&gtt);
		pic->f = gtt.f; pic->p = gtt.p; pic->x = gtt.x; pic->y = gtt.y; pic->lowermip = gtt.lowermip;
	}
#else
	kpzload(tbuf,&pic->f,&pic->p,&pic->x,&pic->y);
	//kpzload(tpic->filnam,&pic->f,&pic->p,&pic->x,&pic->y); //FIX:why don't relative filenames work?
#endif
	if (!pic->f) { pic->f = (long)nullpic; pic->x = 64; pic->y = 64; pic->p = (pic->x<<2); pic->lowermip = 0; }
}
#endif //BUILD2_LOADERS_H