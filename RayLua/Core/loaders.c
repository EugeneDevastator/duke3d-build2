/*
*       BUILD2 map loading module by Ken Silverman (http://advsys.net/ken)
 *      This file has been modified from Ken Silverman's original release
 *
 *       Things changed (by Eugene):
 *       - updated data structures and read all the needed info from duke nukem maps
 *       - added compatibility storage to main types
 */
#include "loaders.h"
// TODO : Replace types with stdint like uint8_t
// TODO : new mapstate should have raylib friendly coords by default. period.

// build format 7 flags.
#define TILSIZEX(g,t) g_gals[g].sizex[t]
#define TILSIZEY(g,t) g_gals[g].sizey[t]

#define GET_TILSIZEX(e) g_gals[e.galnum].sizex[e.tilnum]
#define GET_TILSIZEY(e) g_gals[e.galnum].sizey[e.tilnum]
#define GET_TILSIZEX_PTR(e) g_gals[e->galnum].sizex[e->tilnum]
#define GET_TILSIZEY_PTR(e) g_gals[e->galnum].sizey[e->tilnum]
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

typedef struct {
	surf_t *pool;
	int used, capacity;
} surf_allocator_t;
surf_allocator_t sural={0};
void free_surf_allocator(surf_allocator_t *alloc) {
	if (alloc && alloc->pool) {
		free(alloc->pool);
		alloc->pool = NULL;
		alloc->used = 0;
		alloc->capacity = 0;
	}
}
void init_surf_allocator(surf_allocator_t *alloc) {
	free_surf_allocator(alloc);
	alloc->pool = NULL;
	alloc->used = 0;
	alloc->capacity = 0;
}
surf_t* alloc_surfs(surf_allocator_t *alloc, int count) {
	if (alloc->used + count > alloc->capacity) {
		// Proper growth: ensure minimum size and handle zero capacity
		int new_capacity = alloc->capacity == 0 ? 64 : alloc->capacity * 2;
		while (new_capacity < alloc->used + count) {
			new_capacity *= 2;
		}

		alloc->pool = realloc(alloc->pool, new_capacity * sizeof(surf_t));
		if (!alloc->pool) {
			// Handle allocation failure
			return NULL;
		}
		alloc->capacity = new_capacity;
	}

	surf_t *result = &alloc->pool[alloc->used];
	alloc->used += count;
	return result;
}
void initTiles()
{
	gnumtiles = 0;
	memset(gtilehashead, -1, sizeof(gtilehashead));
	gmaltiles = 256;
	gtile = (tile_t*)malloc(gmaltiles * sizeof(tile_t));
	//if (!gtile)
	//	memset(gtile,0,gmaltiles*sizeof(tile_t)); //FIX
}
void freetiles() {
	for (int i = 0; i < gnumtiles; i++) {
		if (gtile[i].tt.f) {
			free((void*)gtile[i].tt.f);
		//	gtile[i].tt.f = NULL;
		}
	}
	if (gtile) {
		free(gtile);
	//	gtile = NULL;
	}
	gnumtiles = 0;
}

void freemap(mapstate_t *map) {
	if (!map) return;

	// Free wall surfaces first
	for (int i = 0; i < map->numsects; i++) {
		if (map->sect[i].wall) {
			for (int j = 0; j < map->sect[i].n; j++) {
				//if (map->sect[i].wall[j].xsurf) {
				//	free(map->sect[i].wall[j].xsurf);
				//	map->sect[i].wall[j].xsurf = NULL;
				//}
			}
			free(map->sect[i].wall);
		//	map->sect[i].wall = NULL;
		}
	}

	// Free sectors array
	if (map->sect) {
		free(map->sect);
	//	map->sect = NULL;
	}

	// Free sprites array
	if (map->spri) {
		free(map->spri);
	//	map->spri = NULL;
	}

	// Reset counters
	map->numsects = 0;
	map->numspris = 0;
	freetiles();
}

mapstate_t* loadmap_imp (char *filnam, mapstate_t* oldmap)
{
	init_surf_allocator(&sural);
	surf_t *sur;
	sect_t *sec;
	wall_t *wal;
	spri_t *spr;
	float f, fx, fy;
	int i, j, k, l;
	long x, y, z, fileid, hitile, warned = 0, altsects, nnumtiles, nnumspris;
	short s, cursect;
	char och, tbuf[256];
//	freemap(map);

	freemap(oldmap);  // Clean up old one

	mapstate_t* map = (mapstate_t*)malloc(sizeof(mapstate_t));
	if (!map) return NULL;
	memset(map, 0, sizeof(mapstate_t));
	initcrc32();

	map->numsects = 0;
	map->malsects = 256;
	map->sect = (sect_t*)(malloc(map->malsects * sizeof(sect_t)));
	if (!map->sect) return 0;
	memset(map->sect, 0, map->malsects * sizeof(sect_t));

	map->numspris = 0;
	map->malspris = 256;
	map->spri = (spri_t*)(malloc(map->malspris * sizeof(spri_t)));
	if (!map->spri) return 0;
	memset(map->spri, 0, map->malspris * sizeof(spri_t));
	map->blankheadspri = -1;

	map->blankheadspri = -1;
	for (int i = 0; i < map->malspris; i++)
	{
		map->spri[i].sectn = map->blankheadspri;
		map->spri[i].sectp = -1;
		map->spri[i].sect = -1;
		if (map->blankheadspri >= 0) map->spri[map->blankheadspri].sectp = i;
		map->blankheadspri = i;
	}

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
		{ kzclose(); kzaddstack(filnam); return(map); }
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
				//sec[i].surf[j].uv[1].x = sec[i].surf[j].uv[2].y = 1.f;
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
				//	  { for(k=0;k<3;k++) wal[j].surf.uv[k] = wall1.surf.uv[k]; }
				//else { wal[j].surf.uv[1].x = wal[j].surf.uv[2].y = 1.f; }
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
		return(map);
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
							return(NULL);
						}
					}
				}

				sec[i].wall[j].owner = -1;
				if (sec[i].wall[j].surfn > 1)
				{
					//sec[i].wall[j].xsurf = (surf_t *)malloc((sec[i].wall[j].surfn-1)*sizeof(surf_t));
					//kzread(sec[i].wall[j].xsurf,(sec[i].wall[j].surfn-1)*sizeof(surf_t));
					// replaced to simplify xsurfs and stick to 3 per wall max.
					kzread(sec[i].wall[j].xsurf,(3)*sizeof(surf_t));
				}
			}
		}

			//Load tiles
		kzread(&nnumtiles,4); gnumtiles += nnumtiles;
		if (false) // dont read tile data.
		{
		//	if (gnumtiles > gmaltiles)
		//	{
		//		gmaltiles = max(gnumtiles+1,gmaltiles<<1);
		//		gtile = (tile_t *)realloc(gtile,gmaltiles*sizeof(tile_t));
		//	}
		//	tile_t* gtpic;
		//	for(i=gnumtiles-nnumtiles;i<gnumtiles;i++)
		//	{
		//		kzread(&s,2); kzread(gtile[i].filnam,s); gtile[i].filnam[s] = 0; //FIX:possible buffer overflow here
		//		gtile[i].tt.f = 0;
		//		gtile[i].namcrc32 = getcrc32z(0,(unsigned char *)gtile[i].filnam);
		//		gtpic = &gtile[sur->tilnum];
		//		if (!gtpic->tt.f) loadpic(gtpic,curmappath);
		//	}
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
		return(map);
	}
	else if ((fileid == 0x00000007) || (fileid == 0x00000cbe))   //Build1 .MAP format 7 //Cubes5 .CUB format
	{
		//Build1 format variables:
		typedef struct {
			short picnum, heinum;
			signed char shade;
			uint8_t pal, xpanning, ypanning;
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
			int32_t x, y;  // long is 32bit in Ken's format
			short point2, nextwall, nextsect, cstat, picnum, overpicnum;
			int8_t  shade;
			uint8_t pal, xrepeat, yrepeat, xpanning, ypanning;
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
		arttiles = g_gals[0].gnumtiles;
        short *tilefile = 0;
		char tbuf[MAX_PATH*2];

		kzclose();

		strcpy(curmappath,filnam);
		for(i=j=0;curmappath[i];i++) if ((curmappath[i] == '/') || (curmappath[i] == '\\')) j = i+1;
		curmappath[j] = 0;

/*
		// Modified scanning code; this entire section must be abolished and moved into art loader.
		arttiles = 0;
		for(filnum=0;1;filnum++)
		{
			sprintf(tbuf,"TILES%03d.ART",filnum);
			if (!kzopen(tbuf))
			{
				sprintf(tbuf,"%sTILES%03d.ART",curmappath,filnum);
				if (!kzopen(tbuf)) break;
			}
			kzread(tbuf,16);
			if (*(long *)&tbuf[0] != 1) break;
			loctile0 = *(long *)&tbuf[8];
			loctile1 = (*(long *)&tbuf[12])+1;
			if ((loctile0 < 0) || (loctile1 <= arttiles) || (loctile0 >= loctile1)) continue;

			i = arttiles; arttiles = loctile1;
			tilesizx = (short *)realloc(tilesizx,arttiles*sizeof(tilesizx[0]));
			tilesizy = (short *)realloc(tilesizy,arttiles*sizeof(tilesizy[0]));
			tilefile = (short *)realloc(tilefile,arttiles*sizeof(tilefile[0]));
			picanm = (picanm_t *)realloc(picanm,arttiles*sizeof(picanm[0])); // Add this line

			for(;i<arttiles;i++) {
				tilesizx[i] = 0;
				tilesizy[i] = 0;
				tilefile[i] = 0;
				picanm[i].asint = 0; // Initialize animdata
			}

			kzread(&tilesizx[loctile0],(loctile1-loctile0)*sizeof(short));
			kzread(&tilesizy[loctile0],(loctile1-loctile0)*sizeof(short));
			kzread(&picanm[loctile0],(loctile1-loctile0)*sizeof(long)); // Read animdata

			for(i=loctile0;i<loctile1;i++) tilefile[i] = filnum;
		}

		if (!arttiles)
		{
			tilesizx = (short *)malloc(sizeof(tilesizx[0]));
			tilesizy = (short *)malloc(sizeof(tilesizy[0]));
			tilefile = (short *)malloc(sizeof(tilefile[0]));
			picanm = (picanm_t *)malloc(sizeof(picanm[0])); // Add this line
			tilesizx[0] = tilesizy[0] = 2;
			tilefile[0] = 0;
			picanm[0].asint = 0; // Initialize
			arttiles = 1;
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
		*/
		kzopen(filnam);
		kzread(&i,4);
		//------------------------------------------------------------------------

		hitile = 0;
		int defaultGal=0;
		arttiles = g_gals[defaultGal].numtiles;
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
				sec[i].lotag = b7sec.lotag;
				for(j=0;j<2;j++)
				{
					sec[i].destpn[j] = -1;
					sec[i].z[j] = ((float)b7sec.z[j])*(1.f/(512.f*16.f));
					sec[i].grad[j].x = sec[i].grad[j].y = 0;
					if (b7sec.stat[j]&2) //Enable slopes flag
						sec[i].grad[j].y = ((float)b7sec.surf[j].heinum)*(1.f/4096.f);
					sur = &sec[i].surf[j];
					sur->flags = 0;
					sur->frMode = fmode_flat;
					if (b7sec.stat[j]&1) {
						sur->flags |= SURF_PARALLAX_DISCARD; // parallax
						sur->frMode = fmode_parallaxcyl;
						}
					sur->asc = 4096;
					sur->rsc = (32-b7sec.surf[j].shade)*128;
					sur->gsc = (32-b7sec.surf[j].shade)*128;
					sur->bsc = (32-b7sec.surf[j].shade)*128;
					l = b7sec.surf[j].picnum;
					sur->tilnum = l;
					hitile = max(hitile,l);
					sur->galnum = defaultGal; // assume duke.
					// Convert lotag/hitag to single tag field
					// j=0 is ceiling, j=1 is floor - assign to floor surface only
					if (j == 1) // Floor surface
					{
						// Merge lotag (lower 16 bits) and hitag (upper 16 bits) into single long
						sur->lotag = b7sec.lotag;
						sur->hitag = b7sec.hitag;
					}

					sur->pal = b7sec.surf[j].pal;

					//sur->uv[0].x = ((float)b7sec.surf[j].xpanning)/256.0;
					//sur->uv[0].y = ((float)b7sec.surf[j].ypanning)/256.0;
					//sur->uv[1].y = sur->uv[2].x = 0;
					//if (!(b7sec.stat[j]&SECTOR_SWAP_XY))
					//{
					//	sur->uv[1].x = 32.0/((float)GET_TILSIZEX_PTR(sur));
					//	sur->uv[2].y = 32.0/((float)GET_TILSIZEY_PTR(sur));
					//}
					//else
					//{
					//	sur->uv[1].x = 32.0/((float)GET_TILSIZEX_PTR(sur));
					//	sur->uv[2].y = 32.0/((float)GET_TILSIZEY_PTR(sur));
					//}
					//if (b7sec.stat[j] & SECTOR_EXPAND_TEXTURE) { sur->uv[1].x *= 2; sur->uv[2].y *= 2; } //double smooshiness
					//if (b7sec.stat[j] & SECTOR_FLIP_X) sur->uv[1].x *= -1; //x-flip
					//if (!(b7sec.stat[j] & SECTOR_FLIP_Y)) sur->uv[2].y *= -1; //y-flip
					if (b7sec.stat[j] & SECTOR_TEXWALL_ALIGN) //relative alignment
					{
						f = ((float)b7sec.surf[j].heinum)*(1.f/4096.f);
						//sur->uv[2].y *= -sqrt(f*f + 1.f);
						sur->flags |= 4;
					}
					// if (b7sec.stat[j] & SECTOR_SWAP_XY) //swap x&y
					// {
					// 	if (((b7sec.stat[j] & SECTOR_FLIP_X) != 0) != ((b7sec.stat[j] & SECTOR_FLIP_Y) != 0))
					// 	{ sur->uv[1].x *= -1; sur->uv[2].y *= -1; }
					// 	sur->uv[1].y = sur->uv[1].x; sur->uv[1].x = 0;
					// 	sur->uv[2].x = sur->uv[2].y; sur->uv[2].y = 0;
					// }

					//FIX:This hack corrects an LHS vs. RHS bug in a later stage of texture mapping (drawsectfill?)
					//if (sur->uv[1].x*sur->uv[2].y < sur->uv[1].y*sur->uv[2].x)
					//{ sur->uv[2].x *= -1; sur->uv[2].y *= -1; }

					sec[i].surf[j].uvform.mapping_kind = b7sec.stat[j] & SECTOR_TEXWALL_ALIGN ? UV_TEXELRATE : UV_WORLDXY;
					sec[i].mflags[j] = b7sec.stat[j];

					// also pans are limited by 256. so large textures wont work.
					float xsize = GET_TILSIZEX(sec[i].surf[j]);
					float ysize = GET_TILSIZEY(sec[i].surf[j]);
					//float pix8 = 8.0f/xsize; //64/8 = 8
					//float scalerx = pix8;
					if (sec[i].surf[j].uvform.mapping_kind == UV_TEXELRATE) {
						sec[i].surf[j].uvform.scale.x = 32/xsize; // scale is always off 64.
						sec[i].surf[j].uvform.scale.y = 32/ysize;
					} else	{
						sec[i].surf[j].uvform.scale.x = xsize/64; // scale is always off 64.
						sec[i].surf[j].uvform.scale.y = ysize/64;
					}
					// regardless of scaling etc.
					// pan of 16 = 1 px for 16  tex
					// 4 = 1 px for 128
					float xpansperpx = 256.0/xsize;
					float ypansperpx = 256.0/ysize;
					float xpan = (b7sec.surf[j].xpanning/(256.0/xsize))/xsize;
					float ypan = (b7sec.surf[j].ypanning/(256.0/ysize))/ysize;

					sec[i].surf[j].uvform.pan.x = xpan; // 1 pixel per 16 pans, before scaling. 4 pans for 64 tile
					sec[i].surf[j].uvform.pan.y = ypan;

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
					sec[i].wall[j].nschain=-1;
					sec[i].wall[j].nwchain=-1;
					wallidx++;

					sur = &sec[i].wall[j].surf;
					sur->flags = 0;
					if (b7wal.cstat&1) sur->flags |= 1;

					// flag at byte 1 : double split  = 1, one tile = 0

					sur->lotag = b7wal.lotag;
					sur->hitag = b7wal.hitag;
					sur->pal = b7wal.pal;

					//sur->uv[0].x = b7wal.xpanning;
					//sur->uv[0].y = b7wal.ypanning;
					//sur->uv[1].x = b7wal.xrepeat;
					//if (b7wal.cstat & WALL_FLIP_X) sur->uv[1].x *= -1;
					//sur->uv[1].y = 0;
					//sur->uv[2].x = 0;
					//sur->uv[2].y = b7wal.yrepeat;
					//if (b7wal.cstat & WALL_FLIP_Y) sur->uv[2].y *= -1;
					// if wall opens to next sector - we align to 'jawlines' for easier door setup
					if ((b7wal.nextsect < 0) ^ (!(b7wal.cstat & WALL_ALIGN_FLOOR))) {
						sur->flags ^= 4;
					}

					if (b7wal.cstat & WALL_BOTTOM_SWAP) sur->flags ^= 2; //align bot/nextsec
					if (b7wal.cstat & (WALL_MASKED + WALL_SOLID_MASKED)) sur->flags |= 32; //bit4:masking, bit5:1-way
					sur->asc = 4096;
					sur->rsc = (32-b7wal.shade)*128;
					sur->gsc = (32-b7wal.shade)*128;
					sur->bsc = (32-b7wal.shade)*128;
					l = b7wal.picnum;
					sur->tilnum = l; hitile = max(hitile,l);
					sur->galnum = defaultGal;
					sec[i].wall[j].surfn = 1;
					sec[i].wall[j].owner = -1;
					wall_t *thiswal = &sec[i].wall[j];
					thiswal->xsurf[0].galnum = defaultGal;
					thiswal->xsurf[1].galnum = defaultGal;
					thiswal->xsurf[2].galnum = defaultGal;
					thiswal->xsurf[0].rsc=sur->rsc;
					thiswal->xsurf[1].rsc=sur->rsc;
					thiswal->xsurf[2].rsc=sur->rsc;
					if (b7wal.nextsect >= 0 ) // (sec[i].wall[j].ns != -1) // ns are parsed later! nut we need to alloc now.
					{
						thiswal->surfn = 3;
					//	thiswal->xsurf = alloc_surfs(&sural,3);

						thiswal->xsurf[0].alpha=1;
						thiswal->xsurf[1].alpha=1;
						thiswal->xsurf[2].alpha=1;
						thiswal->surf.alpha=1;
						thiswal->xsurf[0].tilnum = b7wal.picnum;
						thiswal->xsurf[1].tilnum = b7wal.overpicnum;
						thiswal->xsurf[2].tilnum = b7wal.picnum;
						thiswal->tempflags = b7wal.cstat & WALL_BOTTOM_SWAP ? -2 : 0;
						int opacity = 255;
						// solid masked is nevewr transparent
						if (HAS_FLAG(b7wal.cstat, WALL_MASKED) && !(b7wal.cstat & WALL_SOLID_MASKED))
						{
							opacity = 254; // hack to draw in transparent passanyway - refactor later with data structure
							if (HAS_FLAG(b7wal.cstat, WALL_SEMI_TRANSPARENT))
								opacity = 128;
							if (HAS_FLAG(b7wal.cstat, WALL_TRANSPARENT))
								opacity = 32;
						}

						thiswal->xsurf[1].asc = opacity;
						thiswal->xsurf[1].alpha = opacity/(float)255.0;
						//makeslabuvform(1, -1, thiswal,
						//               (int[4]){b7wal.xrepeat, b7wal.yrepeat, b7wal.xpanning, b7wal.ypanning},
						//               (int[2]){0, 0});
					}
					else {
					//	thiswal->xsurf = alloc_surfs(&sural,1);
						thiswal->surfn=1;
						thiswal->xsurf[0].tilnum = b7wal.picnum;
						thiswal->xsurf[0].alpha=1;
					}
					thiswal->surf.tilnum= b7wal.picnum;
					thiswal->surf.galnum= defaultGal;
					//float wallh = (sec[i].z[1]-sec[i].z[0]);
					// also pans are limited by 256. so large textures wont work.
					float xsize = GET_TILSIZEX(thiswal->surf);
					float pix8 = 8.0f/xsize; //64/8 = 8
					float scalerx = b7wal.xrepeat * pix8;

					float ysize = GET_TILSIZEY(thiswal->surf);
					float pix4 = 4.0f/ysize;
					float normuvperz = pix4 * b7wal.yrepeat;
					float scalery =  normuvperz;

					float px1x = 1.0f/xsize;
					float px1y = 1.0f/ysize;
					float ypans_per_px = 256.f/ysize;

					thiswal->surf.rt_uvs[4].y = b7wal.yrepeat; // need for second pass.
					thiswal->surf.rt_uvs[4].x = b7wal.xrepeat; // need for second pass.
					thiswal->mflags[0] = b7wal.cstat; // need for second pass.
					thiswal->surf.uvform.scale.x=scalerx;
					thiswal->surf.uvform.scale.y=scalery;

					// NPOT wall textures dont align well in duke.
					// you need to offset them by YRes * 2 pans.
					// and do this twice if Yres is odd.
					float  ypansub = 0;
					if (isnpot(ysize) ) // not power of two
					{
						int powr = floor(sqrt(ysize));
						int yclamsiz = pow(2,powr);

						// probably need for other texture sizes too, but those are most common
						float mul = 256.0/yclamsiz;
						// there are some shenanigans when texture yres is odd. skip for now.
						//if ((int) ysize % 2)
						//	ypansub = (ceil(ysize * 0.5f) * 4);
						//	//*(((int)ysize % 2)+1);// if non divisible by 2 add twice.
						//else
							ypansub = (ysize * mul); //*(((int)ysize % 2)+1);// if non divisible by 2 add twice.

					}
					thiswal->surf.uvform.pan.x=  px1x * b7wal.xpanning;

					for (int xw=0;xw<thiswal->surfn;xw++) {
						thiswal->xsurf[xw].uvform = thiswal->surf.uvform;
						//memcpy(thiswal->xsurf[xw].uvform,thiswal->surf.uvform,sizeof(uvform_t));
					}
					thiswal->xsurf[0].uvform.pan.y= px1y * ((b7wal.ypanning-ypansub)/ypans_per_px);
					if (thiswal->surfn==3) {
						thiswal->xsurf[1].uvform.pan.y= px1y * ((b7wal.ypanning-ypansub)/ypans_per_px);
						thiswal->xsurf[2].uvform.pan.y= px1y * ((b7wal.ypanning)/ypans_per_px);
					}
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
					//sur->uv[1].x = ((float)sur->uv[1].x*8.0)/(f*((float)tilesizx[l]));
					//sur->uv[2].y = ((float)sur->uv[2].y*4.0)/((float)tilesizy[l]);
					//sur->uv[0].x = ((float)sur->uv[0].x)/((float)tilesizx[l]);
					//sur->uv[0].y = ((float)sur->uv[0].y)/256.f * (1-2*(sur->uv[2].y < 0));
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

				l = b7spr.picnum;
				spr->p.x = ((float)b7spr.x)*(1.f/512.f);
				spr->p.y = ((float)b7spr.y)*(1.f/512.f);
				spr->p.z = ((float)b7spr.z)*(1.f/(512.f*16.f));
				spr->flags = 0;
				spr->walcon = -3;
				spr->tilnum = l;
				spr->galnum = defaultGal;

				int flagsw=b7spr.cstat & (SPRITE_IS_WALL_PLANE | SPRITE_IS_FLOOR_PLANE);
				if  (flagsw ==0) //Face sprite
					spr->view.rflags.vert_mode = vmode_billbord;
				if  (flagsw & (SPRITE_IS_WALL_PLANE | SPRITE_IS_FLOOR_PLANE)) {
					spr->view.rflags.vert_mode  = vmode_quad;
					spr->view.rflags.is_dblside =  !(b7spr.cstat& SPRITE_ONE_SIDED);
				}

				point3d buildFW = (point3d){cos((float)b7spr.ang*PI/1024.0),sin((float)b7spr.ang*PI/1024.0),0};
				switch(flagsw)  // https://wiki.eduke32.com/wiki/Cstat_(sprite)
				{
					case 0: //facing
					case 48: //Voxel sprite
						//no break intentional
					case SPRITE_IS_WALL_PLANE: //Wall sprite
						// need to not alter sprite positions, but change view xforms.
						spr->p.z -= (b7spr.yrepeat/4096.0*(float)tilesizy[l]);
						spr->r.x = -sin((float)b7spr.ang*PI/1024.0)*(b7spr.xrepeat/4096.0*(float)GET_TILSIZEX_PTR(spr));
						spr->r.y = cos((float)b7spr.ang*PI/1024.0)*(b7spr.xrepeat/4096.0*(float)GET_TILSIZEX_PTR(spr));
						spr->d.z = -1.0*(b7spr.yrepeat/4096.0*(float)GET_TILSIZEY_PTR(spr));
						spr->f = buildFW;
						break;
					case SPRITE_IS_FLOOR_PLANE: //Floor sprite
						// forward faces up, right faces right, down faces along build's forward;
						spr->r.x = sin((float)b7spr.ang*PI/1024.0)*(b7spr.xrepeat/4096.0*(float)GET_TILSIZEX_PTR(spr));
						spr->r.y =-cos((float)b7spr.ang*PI/1024.0)*(b7spr.xrepeat/4096.0*(float)GET_TILSIZEX_PTR(spr));
						spr->d.x = cos((float)b7spr.ang*PI/1024.0)*(b7spr.yrepeat/4096.0*(float)GET_TILSIZEY_PTR(spr));
						spr->d.y = sin((float)b7spr.ang*PI/1024.0)*(b7spr.yrepeat/4096.0*(float)GET_TILSIZEY_PTR(spr));
						int upvec = b7spr.cstat&(SPRITE_FLIP_Y) ? -1 : 1;
						spr->f = (point3d){0,0,upvec}; // down and we render its butt.
						spr->r = (point3d){spr->r.x *upvec, spr->r.y*upvec, spr->r.z*upvec};
						//if (b7spr.cstat&SPRITE_HITSCAN) { spr->d.x *= -1; spr->d.y *= -1; spr->d.z*=-1; }
						break;
				}
				spr->d.x *= -1; spr->d.y *= -1; spr->d.z *= -1; // down is flipped.
				//spr->r.x *= -1; spr->r.y *= -1; spr->r.z *= -1; // also flipping r to restore chirality

				if (b7spr.cstat&SPRITE_BLOCKING) spr->flags |= 1; // blocking
				// floor sprites do this only.
				if (b7spr.cstat&(SPRITE_FLIP_Y | SPRITE_IS_FLOOR_PLANE)) {
					//spr->d.x *= -1; spr->d.y *= -1; spr->d.z *= -1; // dont alter down vector.
					spr->flags ^= 8; } //&8: y-flipped?
				spr->view.uv[0]=1;
				spr->view.uv[1]=1;
				// we flip tile inside of anchored space, so it should not affect the final rect of it
				// currently doesnt work as intended
				if (b7spr.cstat&SPRITE_FLIP_X) { spr->view.uv[0] = -1; } //&4: x-flipped
				if (b7spr.cstat&SPRITE_FLIP_Y) { spr->view.uv[1] = -1; } //&8: y-flipped?

				// note - replace with view setup
				spr->view.anchor.x=0.5f;
				spr->view.anchor.y=0; // forward
				spr->view.anchor.z = 1.0f; // 1 on the V to the pivot. - normal duke3d sprite.
				// So, in build true centered alters only visual representation. view becomes shifted downward.
				if (b7spr.cstat & SPRITE_TRUE_CENTERED || b7spr.cstat & SPRITE_IS_FLOOR_PLANE) {
					spr->p.z += (b7spr.yrepeat/4096.0*(float)tilesizy[l]); // pos is correct
					spr->view.anchor.z = 0.5f;
				}


				spr->tilnum = l; hitile = max(hitile,l);

				// maybe we must move position?
				float tileoffu = (float)g_gals[defaultGal].picanm_data[spr->tilnum].x_center_offset/GET_TILSIZEX_PTR(spr);
				float tileoffv = (float)g_gals[defaultGal].picanm_data[spr->tilnum].y_center_offset/GET_TILSIZEY_PTR(spr);
				spr->view.anchor.x+=tileoffu;
				float yoffmul =1;
				if ((b7spr.cstat&SPRITE_IS_WALL_PLANE)&& (b7spr.cstat&SPRITE_FLIP_Y))
					yoffmul=-1;
				spr->view.anchor.z+=tileoffv*yoffmul;


				//&128: real-centered centering (center at center) - originally half submerged sprite
				if ((unsigned)b7spr.sectnum < (unsigned)map->numsects) //Make shade relative to sector
				{
					j = b7spr.sectnum; j = 32 - map->sect[j].surf[map->sect[j].surf[0].flags&1^1].rsc/128;
					if (iskenbuild) b7spr.shade += j+6;
				}


				spr->phys.fat = 0.f;
			//	spr->asc = 4096;
			//	spr->rsc = (32-b7spr.shade)*128;
			//	spr->gsc = (32-b7spr.shade)*128;
			//	spr->bsc = (32-b7spr.shade)*128;

				spr->phys.mas = spr->phys.moi = 1.0;
				spr->owner = -1;


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
							//sur->uv[1].x = sur->uv[2].y = 1.f;
							sur->asc = 4096;
							sur->rsc = 4096-768+1536*j;
							sur->gsc = 4096-768+1536*j;
							sur->bsc = 4096-768+1536*j;

							if (!j) l = board[4][x][oy-1][z];
							else l = board[1][x][ y+1][z];
							if ((unsigned)l >= (unsigned)arttiles) l = 0;
							sur->tilnum = l; hitile = max(hitile,l);
							sur->galnum = defaultGal;
							//sur->uv[1].x = max(64.f/((float)GET_TILSIZEX_PTR(sur)),1.f);
							//sur->uv[2].y = max(64.f/((float)tilesizy[l]),1.f);
						}
						//sec[i].foglev = ?;
						sec[i].headspri = -1;
						sec[i].owner = -1;

						for(j=0;j<4;j++)
						{
							sec[i].wall[j].x = ((float)(       x+(((j+1)>>1)&1)))*(1.f);
							sec[i].wall[j].y = ((float)(BSIZ-1-z+(( j  )>>1)   ))*(1.f);
							if (j < 3) sec[i].wall[j].n = 1; else sec[i].wall[j].n = -3;
							//sec[i].wall[j].surf.uv[1].x = sec[i].wall[j].surf.uv[2].y = 1;
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

							sur = &sec[i].wall[j].surf;
							sur->tilnum = l; hitile = max(hitile,l);
							sur->galnum = defaultGal;
							//sur->uv[1].x = max(64.f/((float)GET_TILSIZEX_PTR(sur)),1.f);
							//sur->uv[2].y = max(64.f/((float)GET_TILSIZEY_PTR(sur)),1.f);
						}
						map->numsects++;

						oy = BSIZ;
					}
				}

			map->numspris = 0;
		}
		if (true) { // Postprocessing of the loaded map
			//// this is where actual loading happens, also needs to be migrated.
			//// we dont seem to be using hash, and only using direct indexing - extract hashes
			//// into separate method for later use and commment on their usage.
			//for(i=gnumtiles-1;i>=0;i--)
			//	if (gtile[i].tt.f) { free((void *)gtile[i].tt.f); gtile[i].tt.f = 0; }
			//gnumtiles = 0; memset(gtilehashead,-1,sizeof(gtilehashead));
//
			//hitile++;
			//hitile =  2000;
			//if (hitile > gmaltiles)
			//{
			//	gmaltiles = hitile;
			//	gtile = (tile_t *)realloc(gtile,gmaltiles*sizeof(tile_t));
			//}
//
			//for(i=0;i<hitile;i++)
			//{
			//	sprintf(tbuf,"tiles%03d.art|%d",tilefile[i],i);
			//	gettileind(tbuf);
			//}
			//tile_t* gtpic;
			//for(i=0;i<gmaltiles;i++)
			//{
			//	gtpic = &gtile[i];
			//	if (!gtpic->tt.f)
			//		loadpic(gtpic,curmappath);
			//}

#ifdef STANDALONE
			//	for(i=numplayers-1;i>=0;i--) gst->p[i].sec.n = 0;
#endif
			checknextwalls_imp(map);
			checksprisect_imp(-1,map);
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
			//second pass for walls
			for (i = 0; i < map->numsects; i++) // second pass for double wall tex.
			{
				for (j = 0; j < sec[i].n; j++) {
					wall_t *walp = &sec[i].wall[j];
					walp->geoflags = 0;
					// duke-style parallax surf marking;
					if (walp->ns >=0) {
						for (int isf=0;isf<2;isf++) {
							// must check both sectors.
							walp->xsurf[isf*2].frMode = fmode_flat;
							if ((sec[i].surf[isf].flags & SURF_PARALLAX_DISCARD) &&
								(sec[walp->ns].surf[isf].flags & SURF_PARALLAX_DISCARD)) {
								walp->xsurf[isf*2].flags |= SURF_PARALLAX_DISCARD;
								walp->xsurf[isf*2].frMode = fmode_parallaxcyl;
								walp->geoflags |= GEO_NO_BUNCHING;
							}
						}
					}

					map_wall_regen_nsw_chain(i,j,map);

					int nwid = walp->n + j;
					int curwalid = j;
					int yrepeat = walp->surf.rt_uvs[4].y;
					int isfloralign = walp->mflags[0] & WALL_ALIGN_FLOOR;
					int yrepeatbot = yrepeat;
					int isfloralignbot = isfloralign;
					bool isxalignflip = walp->mflags[0] & WALL_FLIP_X;
					bool isyuvflip = walp->mflags[0] & WALL_FLIP_Y;
					int uwalid = isxalignflip ? 0 : TEZ_NW;
					int orwal = isxalignflip ? TEZ_NW : 0;
					int basemul = isyuvflip ? -1 : 1;
					int basexmul = isxalignflip ? -1 : 1;
					int yflipmul[3] = {basemul,basemul,basemul};

					// all walls use same origin xy based on x flip.
					for (int sl=0;sl<walp->surfn;sl++) {
						walp->xsurf[sl].uvgen.otez = orwal;
						walp->xsurf[sl].uvgen.utez = uwalid;
						walp->xsurf[sl].uvgen.vtez = orwal;

						walp->xsurf[sl].uvgen.ctez = uwalid;
					}

					int ns = walp->ns;
					if (walp->surfn == 3 && ns >= 0) // handle multi wall.
					{
						surf_t nextsurf = sec[walp->ns].wall[walp->nw].surf;
						int nxgal = sec[walp->ns].wall[walp->nw].surf.galnum;
						bool isbotswap = walp->tempflags == -2;
						wall_t *oppwal;

						if (isbotswap) //
						{
							oppwal = &sec[walp->ns].wall[walp->nw];

							isfloralignbot = oppwal->mflags[0] & WALL_ALIGN_FLOOR;
							yflipmul[2] = oppwal->mflags[0] & WALL_FLIP_Y ? -1 : 1;
							yrepeatbot = oppwal->surf.rt_uvs[4].y;
							int cursizx = GET_TILSIZEX(walp->xsurf[0]);
							int newsizx = GET_TILSIZEX(nextsurf);

							int cursizy = GET_TILSIZEY(walp->xsurf[0]);
							int newsizy = GET_TILSIZEY(nextsurf);

							walp->xsurf[2].uvform.scale.x *= cursizx/(float)newsizx;
							walp->xsurf[2].uvform.scale.y *= cursizy/(float)newsizy;
							walp->xsurf[2].uvform.pan.x = oppwal->surf.uvform.pan.x;
							walp->xsurf[2].uvform.pan.y = oppwal->surf.uvform.pan.y;
							walp->xsurf[2].tilnum = nextsurf.tilnum;
							// also pans are limited by 256. so large textures wont work.
							//*newx/oldx;
						}

						// for tomorrow - deal with x,y flips
						// deal with masked wall scaling.
						// this is here because we need to know params of next sector.
						bool floralig[3] = {isfloralign,0,isfloralignbot};
						for (int sl=0;sl<3;sl++) {

							if(sl==2 && floralig[sl])
								continue;
// Claude here walp->xsurf[sl].tilnum is invalid, when sl==0, even tho initially all tilnums are set correct
							int gn= walp->xsurf[sl].galnum;
							int tile= walp->xsurf[sl].tilnum;
							float ysize = g_gals[gn].sizey[tile];//GET_TILSIZEY(walp->xsurf[sl]);
							float pix4 = 4.0f / ysize;
							float normuvperz = pix4 * yrepeat;
							if(!floralig[sl])// for flor aligned we use same rect for both chunks
								walp->xsurf[sl].uvform.scale.y = (4*yrepeat)/ysize;// normuvperz;
						}

						{ //rescale mid texture
							int cursizx = GET_TILSIZEX(walp->xsurf[0]);
							int newsizx = GET_TILSIZEX(walp->xsurf[1]);
							//
							int cursizy = GET_TILSIZEY(walp->xsurf[0]);
							int newsizy = GET_TILSIZEY(walp->xsurf[1]);
							//
							walp->xsurf[1].uvform.scale.x *= cursizx/(float)newsizx;
							walp->xsurf[1].uvform.pan.x *= cursizx/(float)newsizx;
							//walp->xsurf[1].uvform.pan.y *= cursizy/(float)newsizy;
						}

						// ==== UV VECTORS SETUP

						if (!isfloralign) { // default for split door
							// in case of standard align - we do door-snapping style,
							//top'
							walp->xsurf[0].uvgen.otez |= TEZ_NS ;//| TEZ_CEIL | TEZ_RAWZ; // next ce
							walp->xsurf[0].uvgen.utez |= TEZ_NS ;//| TEZ_CEIL | TEZ_RAWZ; // next ce
							walp->xsurf[0].uvgen.vtez |= TEZ_WORLDZ1; // TEZ_OS | TEZ_CEIL |
							walp->xsurf[0].uvform.scale.y *= -1;
							//mid in that case is aligned to other ceil. mid is always aligned to ns.
							walp->xsurf[1].uvgen.otez |= TEZ_CLOSEST ; // CEIL
							walp->xsurf[1].uvgen.utez |= TEZ_CLOSEST ; // CEIL
							walp->xsurf[1].uvgen.vtez |= TEZ_FLOR | TEZ_WORLDZ1;
						}
						else { // other kind of align -- to own ceil, but mask to other flor.
							walp->xsurf[0].uvgen.otez |= TEZ_OS | TEZ_CEIL | TEZ_RAWZ; // next floor Z of j, not slope!
							walp->xsurf[0].uvgen.utez |= TEZ_OS | TEZ_CEIL | TEZ_RAWZ; // next floor Z of j
							walp->xsurf[0].uvgen.vtez |= TEZ_OS | TEZ_FLOR | TEZ_RAWZ| TEZ_WORLDZ1; // next floor Z of j

							// for solid masked we always align to ceil - only for ceil aligned option
							int flags1;
							if (walp->mflags[0] & WALL_SOLID_MASKED)
								flags1= 0;
							else
								flags1 = TEZ_CLOSEST  | TEZ_FLOR;

							walp->xsurf[1].uvgen.otez |= flags1;
							walp->xsurf[1].uvgen.utez |= flags1;
							walp->xsurf[1].uvgen.vtez |= TEZ_CLOSEST| TEZ_WORLDZ1;
							walp->xsurf[1].uvform.scale.y *= -1;
						}
						// Handle lower segment separately, because could be walswapped.
						if (!isfloralignbot){	//bot;
							walp->xsurf[2].uvgen.otez |= TEZ_NS | TEZ_FLOR | TEZ_RAWZ; // next floor Z of j, not slope!
							walp->xsurf[2].uvgen.utez |= TEZ_NS | TEZ_FLOR | TEZ_RAWZ; // next floor Z of j
							walp->xsurf[2].uvgen.vtez |= TEZ_OS | TEZ_FLOR | TEZ_RAWZ| TEZ_WORLDZ1; // next floor Z of j
						}
						else {
							walp->xsurf[2].uvgen.otez |= TEZ_OS | TEZ_CEIL | TEZ_RAWZ; // next floor Z of j, not slope!
							walp->xsurf[2].uvgen.utez |= TEZ_OS | TEZ_CEIL | TEZ_RAWZ; // next flo // next floor Z of j
							walp->xsurf[2].uvgen.vtez |= TEZ_OS | TEZ_FLOR | TEZ_RAWZ | TEZ_WORLDZ1; // next floor Z of j
						}
					} else { // single wall.

						if (isfloralign) {
							// flor align when top and bot segs are in door format
							//top'
							walp->xsurf[0].uvgen.otez |= TEZ_OS | TEZ_FLOR | TEZ_RAWZ; // next ce
							walp->xsurf[0].uvgen.utez |= TEZ_OS | TEZ_FLOR | TEZ_RAWZ; // next ce
							walp->xsurf[0].uvgen.vtez |= TEZ_OS | TEZ_CEIL | TEZ_RAWZ |  TEZ_WORLDZ1; // next ceil raw z
							walp->xsurf[0].uvform.scale.y *= -1;
						}
						else {
							walp->xsurf[0].uvgen.otez |= TEZ_OS | TEZ_CEIL | TEZ_RAWZ; // next floor Z of j, not slope!
							walp->xsurf[0].uvgen.utez |= TEZ_OS | TEZ_CEIL | TEZ_RAWZ; // next floor Z of j
							walp->xsurf[0].uvgen.vtez |= TEZ_OS | TEZ_FLOR | TEZ_RAWZ| TEZ_WORLDZ1; // next floor Z of j
						}
					}

					makewaluvs(&sec[i], curwalid, map);

					for (int sl=0;sl<walp->surfn;sl++) {
						walp->xsurf[sl].uvform.scale.y *= yflipmul[sl];
					}
				}
				// can make uvs only when walls are there.
				makesecuvs(&sec[i], map);

			}

			if (tilefile) free(tilefile);

			kzclose();
			return(map);
		}
		else { return(NULL); } //MessageBox(ghwnd,"Invalid MAP format",prognam,MB_OK);
	}
}