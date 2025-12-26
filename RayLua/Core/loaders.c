//
// Created by omnis on 10/22/2025.
//
#include "loaders.h"
// TODO : Replace types with stdint like uint8_t
// TODO : new mapstate should have raylib friendly coords by default. period.
point3d buildToRaylib(point3d buildcoord)
{
	return (point3d){buildcoord.x, -buildcoord.z, buildcoord.y};
}
// In-place conversion - modifies original
void toRaylibInPlace(point3d *buildcoord)
{
	float temp_y = buildcoord->y;
	buildcoord->y = -buildcoord->z;
	buildcoord->z = temp_y;
}
// build format 7 flags.



/* WALL FLAGS
*0 	1 	Enable blocking flag. 	[B]
1 	2 	Enable "bottom texture swap". This makes the top and bottom half of a wall separately editable. However, they will still share repeat values. 	[2]
2 	4 	Align texture to floor (bottom orientation). 	[O]
3 	8 	Flip texture around x-axis. 	[F]
4 	16 	Set as masked wall. Two-sided masked walls must be manually set on each side (the keypress does this automatically). Use it in conjunction with Shift for a one-sided masked wall. 	[M]
5 	32 	Set as 'solid' one-sided masked wall. This differs from Shift + M in that the wall will not be compatible with transparency settings or invisible (pink) pixels. This type of masked wall must be used to make mirrors. 	[1]
6 	64 	Enable hitscan flag. 	[H]
7 	128 	Set as semi-transparent. The keypress applies itself to both sides. 	[T]
8 	256 	Flip texture around y-axis. 	[F]
9 	512 	Set as transparent (must be combined with cstat value 128). The keypress applies itself to both sides. 	[T]
10-15 	1024-32768 	*RESERVED* 	N/A
 */
void initTiles()
{
	gnumtiles = 0;
	memset(gtilehashead, -1, sizeof(gtilehashead));
	gmaltiles = 256;
	gtile = (tile_t*)malloc(gmaltiles * sizeof(tile_t));
	//if (!gtile)
	//	memset(gtile,0,gmaltiles*sizeof(tile_t)); //FIX
}
int loadmap_imp (char *filnam, mapstate_t* map)
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

	initTiles();

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
	sec = map->sect; map->light_sprinum = 0;
	if (fileid == 0x3142534b) //KSB1
	{
		typedef struct { long tilnum, flags, tag; point2d uv[3]; int dummy[6]; short asc, rsc, gsc, bsc; } surf1_t;
		typedef struct { float x, y; long n, ns, nw; surf1_t surf; } wall1_t;
		typedef struct { float z[2]; point2d grad[2]; surf1_t surf[2]; long foglev; wall1_t *wall; int n, nmax; } sect1_t;
		surf1_t surf1;
		wall1_t wall1;
		sect1_t sect1;

		for(i=map->numsects-1;i>=0;i--)
			if (map->sect[i].wall) { free(map->sect[i].wall); map->sect[i].wall = 0; }
		kzread(&map->numsects,4);
		if (map->numsects > map->malsects)
		{
			i = map->malsects; map->malsects = max(map->numsects+1,map->malsects<<1);
			sec = map->sect = (sect_t *)realloc(sec,map->malsects*sizeof(sect_t));
			memset(&sec[i],0,(map->malsects-i)*sizeof(sect_t));
		}
		memset(sec,0,sizeof(sect_t)*map->numsects);
		for(i=0;i<map->numsects;i++)
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
		for(i=0;i<map->numsects;i++)
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

		map->numspris = 0;

#ifdef STANDALONE
	//	for(i=numplayers-1;i>=0;i--) gst->p[i].sec.n = 0;
#endif
		checknextwalls_imp(map);
		checksprisect_imp(-1,map);
		kzclose();
		return(1);
	}
	else if (fileid == 0x3242534b) //KSB2 (current BUILD2 map format)
	{
		kzread(&map->startpos,sizeof(map->startpos));
		kzread(&map->startrig,sizeof(map->startrig));
		kzread(&map->startdow,sizeof(map->startdow));
		kzread(&map->startfor,sizeof(map->startfor));
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
		for(i=0;i<map->numsects;i++)
		{
			if (sec[i].owner < 0)
			{
				while (sec[i].headspri >= 0) delspri_imp(sec[i].headspri,map);
				if (map->sect[i].wall) { free(map->sect[i].wall); map->sect[i].wall = 0; }
				continue;
			}
			for(j=sec[i].headspri;j>=0;j=map->spri[j].sectn) map->spri[j].sect = altsects;
			memcpy(&sec[altsects],&sec[i],sizeof(sect_t)); altsects++;
		}
		kzread(&i,4); map->numsects = i+altsects;
		if (map->numsects > map->malsects)
		{
			i = map->malsects; map->malsects = max(map->numsects+1,map->malsects<<1);
			sec = map->sect = (sect_t *)realloc(sec,map->malsects*sizeof(sect_t));
			memset(&sec[i],0,(map->malsects-i)*sizeof(sect_t));
		}
		kzread(&sec[altsects],(map->numsects-altsects)*sizeof(sect_t));

			//Load walls
		for(i=altsects;i<map->numsects;i++)
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
						//if (1)//(MessageBox(ghwnd,"Your map appears to be corrupt. Load anyway?",prognam,MB_YESNO) == IDNO)
						{
							for(;i>=0;i--) free(sec[i].wall);
							map->numsects = 0;
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
		tile_t* gtpic;
		for(i=gnumtiles-nnumtiles;i<gnumtiles;i++)
		{
			kzread(&s,2); kzread(gtile[i].filnam,s); gtile[i].filnam[s] = 0; //FIX:possible buffer overflow here
			gtile[i].tt.f = 0;
			gtile[i].namcrc32 = getcrc32z(0,(unsigned char *)gtile[i].filnam);
			gtpic = &gtile[sur->tilnum];
			if (!gtpic->tt.f) loadpic(gtpic,curmappath);
		}

			//Load sprites
		kzread(&nnumspris,4); map->numspris += nnumspris;
		if (!nnumspris) for(i=0;i<map->numsects;i++) { sec[i].headspri = -1; sec[i].owner = -1; } //Hack for loading old format
		if (map->numspris > map->malspris)
		{
			i = map->malspris;
			map->malspris = max(map->numspris+1,map->malspris<<1);
			map->spri = (spri_t *)realloc(map->spri,map->malspris*sizeof(spri_t));
#ifndef STANDALONE
			for(;i<map->malspris;i++) map->spri[i].sect = -1;
#endif
		}
		kzread(&map->spri[map->numspris-nnumspris],nnumspris*sizeof(spri_t));
		for(i=map->numspris-nnumspris;i<map->numspris;i++) map->spri[i].sect += altsects;


			// | 0 ..       altsects ..  gst->numsects   |
			// |   ^old_sects^    |     ^new_sects^      |
			//
			// |0..gst->numspris-nnumspris..gst->numspris|
			// |  ^old_sprites^   |    ^new_sprites^     |
			//
			// | 0 ..  gnumtiles-nnumtiles .. gnumtiles  |
			// |   ^old_tiles^    |     ^new_tiles^      |

			//Adjust tile indices for new sectors(/walls) & sprites
		for(i=altsects;i<map->numsects;i++)
		{
			for(j=0;j<2       ;j++) sec[i].surf[j].tilnum      += gnumtiles-nnumtiles;
			for(j=0;j<sec[i].n;j++) sec[i].wall[j].surf.tilnum += gnumtiles-nnumtiles;
		}
		for(i=map->numspris-nnumspris;i<map->numspris;i++)
			if (map->spri[i].tilnum >= 0)
				map->spri[i].tilnum += gnumtiles-nnumtiles;

		//-------------------------------------------------------------------

			//Sprite hacks
		for(i=0;i<map->numspris;i++)
		{
			map->spri[i].owner = -1;

				//Insert lights
			if (map->spri[i].flags&(1<<16))
			{
				if (map->light_sprinum < MAXLIGHTS) map->light_spri[map->light_sprinum++] = i;
			}
		}

#ifdef STANDALONE
// mp ommited
		//	for(i=numplayers-1;i>=0;i--) gst->p[i].sec.n = 0;
#endif
		checknextwalls_imp(map);
		checksprisect_imp(-1,map);

#if 0
			//Rebuild hash table from scratch
		memset(gtilehashead,-1,sizeof(gtilehashead));
		for(i=0;i<gnumtiles;i++)
		{
			j = (gtile[i].namcrc32&(sizeof(gtilehashead)/sizeof(gtilehashead[0])-1));
			gtile[i].hashnext = gtilehashead[j]; gtilehashead[j] = i;
		}
#else
		compacttilelist_imp(1, (mapstate_t*)map);
#endif

		kzclose();
		return(1);
	}
	else if ((fileid == 0x00000007) || (fileid == 0x00000cbe))   //Build1 .MAP format 7 //Cubes5 .CUB format
	{
		//Build1 format variables:
		typedef struct {
			short picnum, heinum;
			signed char shade;
			char pal, xpanning, ypanning;
		} build7surf_t;
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
			map->startsectn = cursect;
			map->startpos.x = (x)*(1.f/512.f);
			map->startpos.y = (y)*(1.f/512.f);
			map->startpos.z = (z)*(1.f/(512.f*16.f));

			map->startfor.x = cos(((float)s)*PI/1024.0);
			map->startfor.y = sin(((float)s)*PI/1024.0);
			map->startfor.z = 0.f;
			map->startrig.x =-map->startfor.y;
			map->startrig.y = map->startfor.x;
			map->startrig.z = 0.f;
			map->startdow.x = 0.f;
			map->startdow.y = 0.f;
			map->startdow.z = 1.f;
			//	for(i=numplayers-1;i>=0;i--)
			//	{
			//		gst->p[i].ipos = gst->startpos;
			//		gst->p[i].ifor = gst->startfor;
			//		gst->p[i].irig = gst->startrig;
			//		gst->p[i].idow = gst->startdow;
			//		gst->p[i].cursect = cursect;
			//	}

			kzread(&s,2);
			map->numsects = (int)s; //numsectors
			if (map->numsects > map->malsects)
			{
				i = map->malsects; map->malsects = max(map->numsects+1,map->malsects<<1);
				sec = map->sect = (sect_t *)realloc(sec,map->malsects*sizeof(sect_t));
				memset(&sec[i],0,(map->malsects-i)*sizeof(sect_t));
			}
			for(i=0;i<map->numsects;i++) // parse sectors
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
					if (!(b7sec.stat[j]&SECTOR_SWAP_XY))
					{
						sur->uv[1].x = 32.0/((float)tilesizx[l]);
						sur->uv[2].y = 32.0/((float)tilesizy[l]);
					}
					else
					{
						sur->uv[1].x = 32.0/((float)tilesizy[l]);
						sur->uv[2].y = 32.0/((float)tilesizx[l]);
					}
					if (b7sec.stat[j] & SECTOR_EXPAND_TEXTURE) { sur->uv[1].x *= 2; sur->uv[2].y *= 2; } //double smooshiness
					if (b7sec.stat[j] & SECTOR_FLIP_X) sur->uv[1].x *= -1; //x-flip
					if (!(b7sec.stat[j] & SECTOR_FLIP_Y)) sur->uv[2].y *= -1; //y-flip
					if (b7sec.stat[j] & SECTOR_TEXWALL_ALIGN) //relative alignment
					{
						f = ((float)b7sec.surf[j].heinum)*(1.f/4096.f);
						sur->uv[2].y *= -sqrt(f*f + 1.f);
						sur->flags |= 4;
					}
					if (b7sec.stat[j] & SECTOR_SWAP_XY) //swap x&y
					{
						if (((b7sec.stat[j] & SECTOR_FLIP_X) != 0) != ((b7sec.stat[j] & SECTOR_FLIP_Y) != 0))
						{ sur->uv[1].x *= -1; sur->uv[2].y *= -1; }
						sur->uv[1].y = sur->uv[1].x; sur->uv[1].x = 0;
						sur->uv[2].x = sur->uv[2].y; sur->uv[2].y = 0;
					}

					//FIX:This hack corrects an LHS vs. RHS bug in a later stage of texture mapping (drawsectfill?)
					if (sur->uv[1].x*sur->uv[2].y < sur->uv[1].y*sur->uv[2].x)
					{ sur->uv[2].x *= -1; sur->uv[2].y *= -1; }

					sec[i].surf[j].uvmapkind = b7sec.stat[j] & SECTOR_TEXWALL_ALIGN ? UV_TEXELRATE : UV_WORLDXY;
					sec[i].mflags[j] = b7sec.stat[j];

					// also pans are limited by 256. so large textures wont work.
					float xsize = tilesizx[sec[i].surf[j].tilnum];
					float ysize = tilesizy[sec[i].surf[j].tilnum];
					//float pix8 = 8.0f/xsize; //64/8 = 8
					//float scalerx = pix8;
					sec[i].surf[j].uvform[0] = xsize/64.0; // scale is always off 64.
					sec[i].surf[j].uvform[1] = ysize/64.0;
					// mull px = tile / 256
					//  64 / 256 = 4;
					// 1 px =
					sec[i].surf[j].uvform[2] = (1.0/256.0)*b7sec.surf[j].xpanning; // 1 pixel per 16 pans, before scaling. 4 pans for 64 tile
					sec[i].surf[j].uvform[3] = (1.0/256.0)*b7sec.surf[j].ypanning;

				}

				sec[i].headspri = -1;
				sec[i].owner = -1;
				//sec[i].foglev = ?;

				sec[i].tags[MT_STATCEIL]=b7sec.stat[CEIL];
				sec[i].tags[MT_STATFLOOR]=b7sec.stat[FLOOR];
				sec[i].tags[MT_SEC_HNHI]=b7sec.surf[CEIL].heinum;
				sec[i].tags[MT_SHADEHI]=b7sec.surf[CEIL].shade;
				sec[i].tags[MT_SEC_HNLOW]=b7sec.surf[FLOOR].heinum;
				sec[i].tags[MT_SHADELOW]=b7sec.surf[FLOOR].shade;
				sec[i].tags[MT_EXTRA] = b7sec.extra;
				sec[i].tags[MT_SEC_FWALL] = b7sec.wallptr;
				sec[i].tags[MT_SEC_WALLNUM] = b7sec.wallnum;
			}
			kzread(&s,2); //numwalls
			printf("walls:%d",s);
			int wallidx =0;
			for(i=k=0;i<map->numsects;i++) // Parse walls
			{
				for(j=0;j<sec[i].n;j++,k++) // walls
				{
					kzread(&b7wal,sizeof(b7wal));
					sec[i].wall[j].x = ((float)b7wal.x)*(1.f/512.f);
					sec[i].wall[j].y = ((float)b7wal.y)*(1.f/512.f);
					sec[i].wall[j].n = b7wal.point2-k;
					sec[i].wall[j].tags[MT_WALLPT2] = b7wal.point2;
					sec[i].wall[j].tags[MT_WAL_WALLIDX] = j;
					sec[i].wall[j].tags[MT_WAL_NEXTSEC] = b7wal.nextsect;
					sec[i].wall[j].tags[MT_NEXTWALL] = b7wal.nextwall;
					sec[i].wall[j].tags[MT_WAL_WALLIDX] = wallidx;
					wallidx++;

					sur = &sec[i].wall[j].surf;
					sur->flags = 0;
					if (b7wal.cstat&1) sur->flags |= 1;

					// flag at byte 1 : double split  = 1, one tile = 0

					sur->lotag = b7wal.lotag;
					sur->hitag = b7wal.hitag;
					sur->pal = b7wal.pal;

					sur->uv[0].x = b7wal.xpanning;
					sur->uv[0].y = b7wal.ypanning;
					sur->uv[1].x = b7wal.xrepeat;
					if (b7wal.cstat & WALL_FLIP_X) sur->uv[1].x *= -1;
					sur->uv[1].y = 0;
					sur->uv[2].x = 0;
					sur->uv[2].y = b7wal.yrepeat;
					if (b7wal.cstat & WALL_FLIP_Y) sur->uv[2].y *= -1;
					// if wall opens to next sector - we align to 'jawlines' for easier door setup
					if ((b7wal.nextsect < 0) ^ (!(b7wal.cstat & WALL_ALIGN_FLOOR))) {
						sur->flags ^= 4;
					}

					if (b7wal.cstat & WALL_BOTTOM_SWAP) sur->flags ^= 2; //align bot/nextsec
					if (b7wal.cstat & (WALL_MASKED+WALL_SOLID_MASKED)) sur->flags |= 32; //bit4:masking, bit5:1-way
					sur->asc = 4096;
					sur->rsc = (32-b7wal.shade)*128;
					sur->gsc = (32-b7wal.shade)*128;
					sur->bsc = (32-b7wal.shade)*128;
					l = b7wal.picnum; if ((unsigned)l >= (unsigned)arttiles) l = 0;
					sur->tilnum = l; hitile = max(hitile,l);
					sec[i].wall[j].surfn = 1;
					sec[i].wall[j].owner = -1;
					wall_t *thiswal = &sec[i].wall[j];

					if (b7wal.nextsect >= 0 ) // (sec[i].wall[j].ns != -1) // ns are parsed later! nut we need to alloc now.
					{
						thiswal->surfn = 3;
						thiswal->xsurf = malloc(sizeof(surf_t) * 3);
						thiswal->xsurf[0].tilnum = b7wal.picnum;
						thiswal->xsurf[1].tilnum = b7wal.overpicnum;
						thiswal->xsurf[2].tilnum = b7wal.cstat & WALL_BOTTOM_SWAP ? -2 : b7wal.picnum;
						int opacity = 0;
						if (HAS_FLAG(b7wal.cstat, WALL_MASKED))
						{
							if (HAS_FLAG(b7wal.cstat, WALL_SEMI_TRANSPARENT))
								opacity = 128;
							if (HAS_FLAG(b7wal.cstat, WALL_TRANSPARENT))
								opacity = 32;
						}

						thiswal->xsurf[1].asc = opacity;
						//makeslabuvform(1, -1, thiswal,
						//               (int[4]){b7wal.xrepeat, b7wal.yrepeat, b7wal.xpanning, b7wal.ypanning},
						//               (int[2]){0, 0});
					}
					else {
						thiswal->xsurf = malloc(sizeof(surf_t) * 1);
						thiswal->xsurf[0].tilnum = b7wal.picnum;
					}
					//float wallh = (sec[i].z[1]-sec[i].z[0]);
					// also pans are limited by 256. so large textures wont work.
					float xsize = tilesizx[thiswal->surf.tilnum];
					float pix8 = 8.0f/xsize; //64/8 = 8
					float scalerx = b7wal.xrepeat * pix8;

					float ysize = tilesizy[thiswal->surf.tilnum];
					float pix4 = 4.0f/ysize;
					float normuvperz = pix4 * b7wal.yrepeat;
					float scalery =  normuvperz;

					float px1x = 1.0f/xsize;
					float px1y = 1.0f/ysize;
					float ypans_per_px = 256.f/ysize;
					thiswal->surf.owal = b7wal.yrepeat; // need for second pass.
					thiswal->surf.vwal = b7wal.xrepeat; // need for second pass.
					thiswal->mflags[0] = b7wal.cstat; // need for second pass.
					thiswal->surf.uvform[0]=scalerx;
					thiswal->surf.uvform[1]=scalery;
					thiswal->surf.uvform[2]=px1x * b7wal.xpanning;
					thiswal->surf.uvform[3]=px1y * (b7wal.ypanning/ypans_per_px);


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

				{ // setting sector slope gradient
					fx = sec[i].wall[1].y-sec[i].wall[0].y;
					fy = sec[i].wall[0].x-sec[i].wall[1].x;
					f = fx*fx + fy*fy; if (f > 0) f = 1.0/sqrt(f); fx *= f; fy *= f;
					for(j=0;j<2;j++)
					{
						sec[i].grad[j].x = fx*sec[i].grad[j].y;
						sec[i].grad[j].y = fy*sec[i].grad[j].y;
					}
				}
			}

			kzread(&s,2); map->numspris = (int)s;
			if (map->numspris > map->malspris)
			{
				i = map->malspris;
				map->malspris = max(map->numspris+1,map->malspris<<1);
				map->spri = (spri_t *)realloc(map->spri,map->malspris*sizeof(spri_t));
#ifndef STANDALONE
				for(;i<map->malspris;i++) map->spri[i].sect = -1;
#endif
			}
			for(i=0;i<map->numspris;i++) // PARSE SPRITES
			{
				kzread(&b7spr,sizeof(b7spr));
				spr = &map->spri[i];
				memset(spr,0,sizeof(spri_t));

				l = b7spr.picnum; if ((unsigned)l >= (unsigned)arttiles) l = 0;
				spr->p.x = ((float)b7spr.x)*(1.f/512.f);
				spr->p.y = ((float)b7spr.y)*(1.f/512.f);
				spr->p.z = ((float)b7spr.z)*(1.f/(512.f*16.f));
				spr->flags = 0;

				int flagsw=b7spr.cstat & (SPRITE_WALL_ALIGNED | SPRITE_FLOOR_ALIGNED);
				if  (flagsw ==0) //Face sprite
					spr->flags |= SPRITE_B2_FACING;
				if  (flagsw & SPRITE_WALL_ALIGNED)
					spr->flags |= SPRITE_B2_FLAT_POLY;
				if  (flagsw & SPRITE_FLOOR_ALIGNED)
					spr->flags |= SPRITE_B2_FLAT_POLY;

				point3d buildFW = (point3d){cos((float)b7spr.ang*PI/1024.0),sin((float)b7spr.ang*PI/1024.0),0};
				switch(flagsw)  // https://wiki.eduke32.com/wiki/Cstat_(sprite)
				{
					case 0: //facing
					case 48: //Voxel sprite
						//no break intentional
					case SPRITE_WALL_ALIGNED: //Wall sprite
						// need to not alter sprite positions, but change view xforms.
						//		spr->p.z -= (b7spr.yrepeat/4096.0*(float)tilesizy[l]);
						spr->r.x = -sin((float)b7spr.ang*PI/1024.0)*(b7spr.xrepeat/4096.0*(float)tilesizx[l]);
						spr->r.y = cos((float)b7spr.ang*PI/1024.0)*(b7spr.xrepeat/4096.0*(float)tilesizx[l]);
						spr->d.z = -(b7spr.yrepeat/4096.0*(float)tilesizy[l]);
						spr->f = buildFW;
						break;
					case SPRITE_FLOOR_ALIGNED: //Floor sprite
						// forward faces up, right faces right, down faces along build's forward;
						spr->r.x = sin((float)b7spr.ang*PI/1024.0)*(b7spr.xrepeat/4096.0*(float)tilesizx[l]);
						spr->r.y =-cos((float)b7spr.ang*PI/1024.0)*(b7spr.xrepeat/4096.0*(float)tilesizx[l]);
						spr->d.x = cos((float)b7spr.ang*PI/1024.0)*(b7spr.yrepeat/4096.0*(float)tilesizy[l]);
						spr->d.y = sin((float)b7spr.ang*PI/1024.0)*(b7spr.yrepeat/4096.0*(float)tilesizy[l]);
						spr->f = (point3d){0,0,-1}; // facing up
						if (b7spr.cstat&SPRITE_HITSCAN) { spr->d.x *= -1; spr->d.y *= -1; }
						break;
				}

				if (b7spr.cstat&SPRITE_BLOCKING) spr->flags |= 1; // blocking
				if (b7spr.cstat& SPRITE_ONE_SIDED) spr->flags |= SPRITE_B2_ONE_SIDED; // 1 sided
				if (b7spr.cstat&SPRITE_FLIP_X) { spr->r.x *= -1; spr->r.y *= -1; spr->r.z *= -1; spr->flags ^= 4; } //&4: x-flipped
				if (b7spr.cstat&SPRITE_FLIP_Y) { spr->d.x *= -1; spr->d.y *= -1; spr->d.z *= -1; spr->flags ^= 8; } //&8: y-flipped?
				// note - replace with view setup
				if (b7spr.cstat&SPRITE_TRUE_CENTERED) { spr->p.z += (b7spr.yrepeat/4096.0*(float)tilesizy[l]); } //&128: real-centered centering (center at center) - originally half submerged sprite
				spr->d.x *= -1; spr->d.y *= -1; spr->d.z *= -1; // down is flipped.
				if ((unsigned)b7spr.sectnum < (unsigned)map->numsects) //Make shade relative to sector
				{
					j = b7spr.sectnum; j = 32 - map->sect[j].surf[map->sect[j].surf[0].flags&1^1].rsc/128;
					if (iskenbuild) b7spr.shade += j+6;
				}


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

				// duke3d compat
				spr->tags[MT_CSTAT] = b7spr.cstat;
				spr->tags[MT_SHADELOW] = b7spr.shade;
				spr->tags[MT_STATNUM] = b7spr.statnum;
				spr->tags[MT_EXTRA] = b7spr.extra;
				spr->tags[MT_SPR_CLIPDIST] = b7spr.clipdist;
				spr->tags[MT_SPR_XREP] = b7spr.xrepeat;
				spr->tags[MT_SPR_YREP] = b7spr.yrepeat;
				spr->tags[MT_SPR_XOFF] = b7spr.xoffset;
				spr->tags[MT_SPR_YOFF] = b7spr.yoffset;
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
			map->startpos.x =      ((float)x)*(1.f/1024.f);
			map->startpos.y = 16.f-((float)z)*(1.f/1024.f);
			map->startpos.z =      ((float)y)*(1.f/1024.f);
			dcossin((double)a3*PI/1024.0,&c1,&s1);
			dcossin((double)a2*PI/1024.0,&c2,&s2);
			dcossin((double)a1*PI/1024.0,&c3,&s3);
			c1c3 = c1*c3; c1s3 = c1*s3; s1c3 = s1*c3; s1s3 = s1*s3;
			map->startrig.x = c1c3 + s1s3*s2;
			map->startrig.y = s1c3 - c1s3*s2;
			map->startrig.z = -s3*c2;
			map->startdow.x = c1s3 - s1c3*s2;
			map->startdow.y = s1s3 + c1c3*s2;
			map->startdow.z = c3*c2;
			map->startfor.x = s1*c2;
			map->startfor.y = -c1*c2;
			map->startfor.z = s2;
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
			map->numsects = 0;
			for(x=0;x<BSIZ;x++)
				for(z=0;z<BSIZ;z++)
				{
					oy = BSIZ;
					for(y=0;y<BSIZ;y++)
					{
						if (board[0][x][y][z] >= 0) continue;

						if (oy > y) oy = y;
						if ((y < BSIZ-1) && (board[0][x][y+1][z] < 0)) continue;

						if (map->numsects >= map->malsects)
						{
							i = map->malsects; map->malsects = max(map->numsects+1,map->malsects<<1);
							sec = map->sect = (sect_t *)realloc(sec,map->malsects*sizeof(sect_t));
							memset(&sec[i],0,(map->malsects-i)*sizeof(sect_t));
						}

						i = map->numsects;
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
						map->numsects++;

						oy = BSIZ;
					}
				}

			map->numspris = 0;
		}
		if (true) { // Postprocessing of the loaded map
			//Set texture names..
			for(i=gnumtiles-1;i>=0;i--)
				if (gtile[i].tt.f) { free((void *)gtile[i].tt.f); gtile[i].tt.f = 0; }
			gnumtiles = 0; memset(gtilehashead,-1,sizeof(gtilehashead));

			hitile++;
			hitile = 1000;
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
			tile_t* gtpic;
			for(i=0;i<gmaltiles;i++)
			{
				gtpic = &gtile[i];
				if (!gtpic->tt.f)
					loadpic(gtpic,curmappath);
			}

#ifdef STANDALONE
			//	for(i=numplayers-1;i>=0;i--) gst->p[i].sec.n = 0;
#endif
			checknextwalls_imp(map);
			checksprisect_imp(-1,map);

			//second pass for walls
			for (i = 0; i < map->numsects; i++) // second pass for double wall tex.
			{
				for (j = 0; j < sec[i].n; j++) {
					wall_t *walp = &sec[i].wall[j];
					int nwid = walp->n + j;
					int curwalid = j;
					int yrepeat = walp->surf.owal;
					int isfloralign = walp->mflags[0] & WALL_ALIGN_FLOOR;
					int yrepeatbot = yrepeat;
					int isfloralignbot = isfloralign;
					bool isxalignflip = walp->mflags[0] & WALL_FLIP_X;
					bool isyuvflip = walp->mflags[0] & WALL_FLIP_Y;
					int uwalid = isxalignflip ? j : nwid;
					int orwal = isxalignflip ? nwid : j;
					int basemul = isyuvflip ? -1 : 1;
					int yflipmul[3] = {basemul,basemul,basemul};
					memcpy(&walp->xsurf[0].uvform, &walp->surf.uvform, sizeof(float) * 6);

					// all walls use same origin xy based on x flip.
					for (int sl=0;sl<walp->surfn;sl++) {
						walp->xsurf[sl].owal = orwal;
						walp->xsurf[sl].uwal = uwalid;
						walp->xsurf[sl].vwal = orwal;
					}

					int ns = walp->ns;
					if (walp->surfn == 3 && ns >= 0) // handle multi wall.
					{
						int nextpic = sec[walp->ns].wall[walp->nw].surf.tilnum;
						bool isbotswap = walp->xsurf[2].tilnum == -2;
						wall_t *oppwal;
						memcpy(&walp->xsurf[1].uvform, &walp->surf.uvform, sizeof(float) * 6);
						memcpy(&walp->xsurf[2].uvform, &walp->surf.uvform, sizeof(float) * 6);
						if (isbotswap) //
						{
							oppwal = &sec[walp->ns].wall[walp->nw];

							isfloralignbot = oppwal->mflags[0] & WALL_ALIGN_FLOOR;
							yflipmul[2] = oppwal->mflags[0] & WALL_FLIP_Y ? -1 : 1;
							yrepeatbot = oppwal->surf.owal;
							int cursizx = tilesizx[walp->xsurf[0].tilnum];
							int newsizx = tilesizx[nextpic];

							int cursizy = tilesizy[walp->xsurf[0].tilnum];
							int newsizy = tilesizy[nextpic];

							walp->xsurf[2].uvform[0] *= cursizx/(float)newsizx;
							walp->xsurf[2].uvform[1] *= cursizy/(float)newsizy;
							walp->xsurf[2].uvform[2] = oppwal->surf.uvform[2];
							walp->xsurf[2].uvform[3] = oppwal->surf.uvform[3];
							walp->xsurf[2].tilnum = nextpic;
							// also pans are limited by 256. so large textures wont work.
							//*newx/oldx;
						}

						// assume one unit is one uv, given scale. so units*unitstouv*scale.
						// pan of 16 is 16 pixels. befre scaling.
						// duke3d:
						// x repeat 1 for wall means 8 pixels per entire wall length
						// y repeat 1 for wall means 4 pixels per 8192 z units. = 1 z unit of b2
						// x pan of 1 equals one pixel move before scaling. (so always 1 pixel)
						// y pan of 8 = 1 pixel of 32x32 texture
						// y pan of 2 = 1 pixel for 128x128  texture
						// y pan of 2 - 1/4 pixel of 32x32.
						// 128/2 = 1;
						// 32/8=1;
						// 256/32 = 8;
						// 256/128 = 2;
						// 256 / size = pans/pixel.

						// for tomorrow - deal with x,y flips
						// deal with masked wall scaling.
						// this is here because we need to know params of next sector.
						bool floralig[3] = {isfloralign,0,isfloralignbot};
						for (int sl=0;sl<3;sl++) {

							if(sl==2 && floralig[sl])
								continue;

							float ysize = tilesizy[walp->xsurf[sl].tilnum];
							float pix4 = 4.0f / ysize;
							float normuvperz = pix4 * yrepeat;
							//if(!floralig[sl])// for flor aligned we use same rect for both chunks
								walp->xsurf[sl].uvform[1] = (4*yrepeat)/ysize;// normuvperz;
						}

						//rescale mid texture
						{
							int cursizx = tilesizx[walp->xsurf[0].tilnum];
							int newsizx = tilesizx[walp->xsurf[1].tilnum];
							//
							int cursizy = tilesizy[walp->xsurf[0].tilnum];
							int newsizy = tilesizy[walp->xsurf[1].tilnum];
							//
							walp->xsurf[1].uvform[0] *= cursizx/(float)newsizx;
							walp->xsurf[1].uvform[2] *= cursizx/(float)newsizx;
							//	walp->xsurf[1].uvform[1] *= cursizy/(float)newsizy;
						}

						// ==== UV VECTORS SETUP

						if (!isfloralign) {
							// in case of standard align - we do door-snapping style,
							//top'
							walp->xsurf[0].otez = TEZ_NS ;//| TEZ_CEIL | TEZ_RAWZ; // next ce
							walp->xsurf[0].utez = TEZ_NS ;//| TEZ_CEIL | TEZ_RAWZ; // next ce
							walp->xsurf[0].vtez = TEZ_INVZ | TEZ_WORLDZ1; // TEZ_OS | TEZ_CEIL |

							//mid in that case is aligned to other ceil. mid is always aligned to ns.
							walp->xsurf[1].otez = TEZ_CLOSEST; // CEIL
							walp->xsurf[1].utez = TEZ_CLOSEST; // CEIL
							walp->xsurf[1].vtez = TEZ_FLOR | TEZ_WORLDZ1;
						}
						else
							// other kind of align -- to own ceil, but mask to other flor.
						{ // THIS WORKS!
							//top
							walp->xsurf[0].otez = TEZ_OS | TEZ_CEIL | TEZ_RAWZ; // next floor Z of j, not slope!
							walp->xsurf[0].utez = TEZ_OS | TEZ_CEIL | TEZ_RAWZ; // next floor Z of j
							walp->xsurf[0].vtez = TEZ_OS | TEZ_FLOR | TEZ_RAWZ| TEZ_WORLDZ1; // next floor Z of j

							//mid in that case is aligned to other ceil. mid is always aligned to ns.
							walp->xsurf[1].otez = TEZ_FLOR | TEZ_CLOSEST; // next ceil raw z
							walp->xsurf[1].utez = TEZ_FLOR | TEZ_CLOSEST; // next ceil raw z
							walp->xsurf[1].vtez = TEZ_INVZ | TEZ_CLOSEST| TEZ_WORLDZ1;
						}
						// also when double tex - then both sides have own alignment, and lower seg borrows its flags from nw.
						// TO IMPLEMENT the above! ^^
						if (!isfloralignbot){	//bot;
							walp->xsurf[2].otez = TEZ_NS | TEZ_FLOR | TEZ_RAWZ; // next floor Z of j, not slope!
							walp->xsurf[2].utez = TEZ_NS | TEZ_FLOR | TEZ_RAWZ; // next floor Z of j
							walp->xsurf[2].vtez = TEZ_OS | TEZ_FLOR | TEZ_RAWZ| TEZ_WORLDZ1; // next floor Z of j
						}
						else {
							walp->xsurf[2].otez = TEZ_OS | TEZ_CEIL | TEZ_RAWZ; // next floor Z of j, not slope!
							walp->xsurf[2].utez = TEZ_OS | TEZ_CEIL | TEZ_RAWZ; // next flo // next floor Z of j
							walp->xsurf[2].vtez = TEZ_OS | TEZ_FLOR | TEZ_RAWZ | TEZ_WORLDZ1; // next floor Z of j
						}
					} else { // single wall.

						if (isfloralign) {
							// flor align when top and bot segs are in door format
							//top'
							walp->xsurf[0].otez = TEZ_OS | TEZ_FLOR | TEZ_RAWZ; // next ce
							walp->xsurf[0].utez = walp->xsurf[0].otez; // next ce
							walp->xsurf[0].vtez = TEZ_OS | TEZ_CEIL | TEZ_RAWZ | TEZ_INVZ| TEZ_WORLDZ1; // next ceil raw z
						}
						else {
							walp->xsurf[0].otez = TEZ_OS | TEZ_CEIL | TEZ_RAWZ; // next floor Z of j, not slope!
							walp->xsurf[0].utez = walp->xsurf[0].otez; // next floor Z of j
							walp->xsurf[0].vtez = TEZ_OS | TEZ_FLOR | TEZ_RAWZ| TEZ_WORLDZ1; // next floor Z of j
						}
					}

					makewaluvs(&sec[i], curwalid, map);

					for (int sl=0;sl<walp->surfn;sl++) {
						walp->xsurf[sl].uvform[1] *= yflipmul[sl];
					}
				}
				// can make uvs only when walls are there.
				makesecuvs(&sec[i], map);

			}

			if (tilesizx) free(tilesizx);
			if (tilesizy) free(tilesizy);
			if (tilefile) free(tilefile);

			kzclose();
			return(1);
		}
		else { return(0); } //MessageBox(ghwnd,"Invalid MAP format",prognam,MB_OK);
	}
}