//This version also handles u&v. Note: Input should still be simple wall quad
#include "mapcore.h"

#define USEHEIMAP 1
#define NOSOUND 1
#define STANDALONE 1
#define OOS_CHECK 1

int portaln=0;
portal portals[100] ={};
long gnumtiles, gmaltiles, gtilehashead[1024];
char curmappath[MAX_PATH+1]="";
long get_gnumtiles(void) { return gnumtiles; }
long get_gmaltiles(void) { return gmaltiles; }
long* get_gtilehashead(void) { return gtilehashead; }

long wallclippol (kgln_t *pol, kgln_t *npol)
{
	double f, dz0, dz1;

	dz0 = pol[3].z-pol[0].z; dz1 = pol[2].z-pol[1].z;
	if (dz0 > 0.0) //do not include null case for rendering
	{
		npol[0] = pol[0];
		if (dz1 > 0.0) //do not include null case for rendering
		{
			npol[1] = pol[1];
			npol[2] = pol[2];
			npol[3] = pol[3];
			npol[0].n = npol[1].n = npol[2].n = 1; npol[3].n = -3;
			return(4);
		}
		else
		{
			f = dz0/(dz0-dz1);
			npol[1].x = (pol[1].x-pol[0].x)*f + pol[0].x;
			npol[1].y = (pol[1].y-pol[0].y)*f + pol[0].y;
			npol[1].z = (pol[1].z-pol[0].z)*f + pol[0].z;
			npol[1].u = (pol[1].u-pol[0].u)*f + pol[0].u;
			npol[1].v = (pol[1].v-pol[0].v)*f + pol[0].v;
			npol[2] = pol[3];
			npol[0].n = npol[1].n = 1; npol[2].n = -2;
			return(3);
		}
	}
	if (dz1 <= 0.0) return(0); //do not include null case for rendering
	f = dz0/(dz0-dz1);
	npol[0].x = (pol[1].x-pol[0].x)*f + pol[0].x;
	npol[0].y = (pol[1].y-pol[0].y)*f + pol[0].y;
	npol[0].z = (pol[1].z-pol[0].z)*f + pol[0].z;
	npol[0].u = (pol[1].u-pol[0].u)*f + pol[0].u;
	npol[0].v = (pol[1].v-pol[0].v)*f + pol[0].v;
	npol[1] = pol[1];
	npol[2] = pol[2];
	npol[0].n = npol[1].n = 1; npol[2].n = -2;
	return(3);
}

//Find shortest path between two 3D line segments
//Input: 2 line segments: (a0)-(a1), (b0)-(b1)
//Returns: distance^2 between closest points
double roundcylminpath2 (double a0x, double a0y, double a1x, double a1y,
								 double b0x, double b0y, double b1x, double b1y)
{
	dpoint3d da, db, ab;
	double k0, k1, k2, k3, k4, d, t, u;

	da.x = a1x-a0x; db.x = b1x-b0x; ab.x = b0x-a0x;
	da.y = a1y-a0y; db.y = b1y-b0y; ab.y = b0y-a0y;
	k0 = da.x*da.x + da.y*da.y;
	k1 = db.x*db.x + db.y*db.y;
	k2 = da.x*db.x + da.y*db.y;
	k3 = da.x*ab.x + da.y*ab.y;
	k4 = db.x*ab.x + db.y*ab.y;

	if (k0 == 0)
	{
		if (k1 == 0) { t = u = 0; }
		else { t = 0; u = -k4/k1; u = min(max(u,0),1); }
	}
	else if (k1 == 0) { u = 0; t = k3/k0; t = min(max(t,0),1); }
	else
	{
		d = k0*k1 - k2*k2;
		t = k1*k3 - k2*k4;
		u = k2*k3 - k0*k4;
		d = 1.0/d; t *= d; u *= d;
		if ((fabs(t-.5) > .5) || (fabs(u-.5) > .5))
		{
			u = min(max(u,0),1);
			t = (k2*u + k3)/k0; t = min(max(t,0),1);
			u = (k2*t - k4)/k1; u = min(max(u,0),1);
		}
	}
	ab.x += db.x*u - da.x*t;
	ab.y += db.y*u - da.y*t;
	return(ab.x*ab.x + ab.y*ab.y);
}

// Duplicates a wall in a sector at position w
// Returns the index of the newly created wall
int dupwall_imp (sect_t *s, int w)
{
	wall_t *wal;
	int i, j;
	// Check if we need more space for walls
	if (s->n >= s->nmax)
	{
		s->nmax = max(s->n+1,s->nmax<<1); s->nmax = max(s->nmax,8);
		s->wall = (wall_t *)realloc(s->wall,s->nmax*sizeof(wall_t));
	}
	wal = s->wall;

	// If this is the first wall, initialize it with default values
	if (!s->n)
	{
		memset(wal,0,sizeof(wall_t));
		wal[0].surf.uv[1].x = wal[0].surf.uv[2].y = 1.f;
		wal[0].ns = wal[0].nw = -1; s->n = 1;
		return(0);
	}

	// Shift all walls after position w to make room for the duplicate
	for(i=s->n;i>w;i--) wal[i] = wal[i-1];
	// Update wall linking counters based on current state
	if (!wal[0].n)    { wal[0].n = 1; wal[1].n = -1; }
	else if (wal[w].n < 0) { wal[w+1].n = wal[w].n-1; wal[w].n = 1; }
	else { for(i=w+1;wal[i].n>0;i++); wal[i].n--; }

	s->n++;  // Increment wall count
	return(w+1);  // Return index of the new wall
}

// Calculates the Z height at point (x,y) on a sloped surface
// Uses the sector's gradient and base Z value
double getslopez (sect_t *s, int i, double x, double y)
{
	wall_t *wal = s->wall;
	// Calculate Z using plane equation: gradient dot (point - reference) + base_z
	return((wal[0].x-x)*s->grad[i].x + (wal[0].y-y)*s->grad[i].y + s->z[i]);
}
// Gets all walls that share the same edge as the given wall
// Returns list of sectors/walls that connect to this wall edge, sorted by midheight
int getwalls_imp (int s, int w, vertlist_t *ver, int maxverts, mapstate_t *map)
{
	vertlist_t tver;
	sect_t *sec;
	wall_t *wal, *wal2;
	float fx, fy;
	int i, j, k, bs, bw, nw, vn;

	sec = map->sect;
	wal = sec[s].wall;
	bs = wal[w].ns;

	if ((unsigned)bs >= (unsigned)map->numsects) return(0);

	vn = 0; // vertex count
	nw = wal[w].n+w; // next wall in current sector
	bw = wal[w].nw; // wall index in connected sector
	do
	{
		wal2 = sec[bs].wall; i = wal2[bw].n+bw; //Make sure it's an opposite wall
		// Check if wall coordinates match (same edge)
		if ((wal[w].x == wal2[i].x) && (wal[nw].x == wal2[bw].x) &&
			 (wal[w].y == wal2[i].y) && (wal[nw].y == wal2[bw].y))
		{ if (vn < maxverts) { ver[vn].s = bs; ver[vn].w = bw; vn++; } }
		bs = wal2[bw].ns;
		bw = wal2[bw].nw;
	} while (bs != s); // Stop when we loop back to starting sector

	//Sort next sects by order of height in middle of wall (FIX:sort=crap algo) (bubble sort)
	fx = (wal[w].x+wal[nw].x)*.5;
	fy = (wal[w].y+wal[nw].y)*.5;
	for(k=1;k<vn;k++)
		for(j=0;j<k;j++)
			// Compare total height (floor + ceiling) at wall middle point
			if (getslopez(&sec[ver[j].s],0,fx,fy) + getslopez(&sec[ver[j].s],1,fx,fy) >
				 getslopez(&sec[ver[k].s],0,fx,fy) + getslopez(&sec[ver[k].s],1,fx,fy))
			{ tver = ver[j]; ver[j] = ver[k]; ver[k] = tver; }
	return(vn);
}

int wallprev (sect_t *s, int w)
{
	wall_t *wal;

	wal = s->wall;
	if ((w > 0) && (wal[w-1].n == 1)) return(w-1);
#if 0
	while (wal[w].n > 0) w++; //better for few vertices per loop
	return(w);
#else
	{
		int ww; //better for more vertices per loop
		for(ww=s->n-1;wal[ww].n+ww>w;ww=wal[ww].n+ww-1);
		return(ww);
	}
#endif
}
// Gets all sectors/walls that share the same vertex point
// Finds all walls that meet at the same corner point
int getverts_imp (int s, int w, vertlist_t *ver, int maxverts, mapstate_t *map)
{
	sect_t *sec;
	float x, y;
	int i, ir, iw, ns, nw;

	if ((maxverts <= 0) || ((unsigned)s >= (unsigned)map->numsects)) return(0);
	if ((unsigned)w >= (unsigned)map->sect[s].n) return(0);

	ver[0].s = s; ver[0].w = w; if (maxverts == 1) return(1);
	sec = map->sect;
	x = sec[s].wall[w].x;
	y = sec[s].wall[w].y;
	ir = 0; iw = 1;
	do
	{
		//CCW next sect
		ns = sec[s].wall[w].ns;
		if (ns >= 0)
		{
			nw = sec[s].wall[w].nw;
			if ((sec[ns].wall[nw].x != x) || (sec[ns].wall[nw].y != y)) nw += sec[ns].wall[nw].n;
			for(i=iw-1;i>=0;i--)
				if ((ver[i].s == ns) && (ver[i].w == nw)) break;
			if ((i < 0) && (sec[ns].wall[nw].x == x) && (sec[ns].wall[nw].y == y))
			{ ver[iw].s = ns; ver[iw].w = nw; iw++; if (iw >= maxverts) break; }
		}

		//CW next sect
		w = wallprev(&sec[s],w);
		ns = sec[s].wall[w].ns;
		if (ns >= 0)
		{
			nw = sec[s].wall[w].nw;
			if ((sec[ns].wall[nw].x != x) || (sec[ns].wall[nw].y != y)) nw += sec[ns].wall[nw].n;
			for(i=iw-1;i>=0;i--)
				if ((ver[i].s == ns) && (ver[i].w == nw)) break;
			if ((i < 0) && (sec[ns].wall[nw].x == x) && (sec[ns].wall[nw].y == y))
			{ ver[iw].s = ns; ver[iw].w = nw; iw++; if (iw >= maxverts) break; }
		}

		if (ir >= iw) break;
		s = ver[ir].s; w = ver[ir].w; ir++;
	} while (1);
	return(iw);
}

// Check if two sectors are neighbors (adjacent to each other)
// Returns 1 if sectors s0 and s1 are neighbors, 0 otherwise
long sect_isneighs_imp (int s0, int s1, mapstate_t *map)
{
	sect_t *sec;
	int i, w, ns, nw;

	sec = map->sect;
	//if (s0 == s1) return(0); ?

	// Iterate through all walls of sector s0
	for(w=sec[s0].n-1;w>=0;w--)
	{
		// Get the neighboring sector and wall indices for current wall
		ns = sec[s0].wall[w].ns;
		nw = sec[s0].wall[w].nw;

		// Follow the chain of connected sectors through portals/walls
		while (((unsigned)ns < (unsigned)map->numsects) && (ns != s0))
		{
			// Direct neighbor found - sectors are adjacent
			if (ns == s1) return(1); //s0 and s1 are neighbors

			// Move to next sector in the chain
			i = ns;
			ns = sec[i].wall[nw].ns;
			nw = sec[i].wall[nw].nw;
		}
	}

	// No connection found between the sectors
	return(0); //bunches not on neighboring sectors are designated as incomparable
}

//this
long insspri_imp (int sect, float x, float y, float z, mapstate_t *map)
{
	spri_t *spr;
	long i;

	if ((unsigned)sect >= (unsigned)map->numsects) return(-1);
	if (map->numspris >= map->malspris)
	{
		map->malspris = max(map->numspris+1,map->malspris<<1);
		map->spri = (spri_t *)realloc(map->spri,map->malspris*sizeof(spri_t));
#ifndef STANDALONE
	//	for(i=map->numspris;i<map->malspris;i++)
	//	{
	//		map->spri[i].sectn = map->blankheadspri;
	//		map->spri[i].sectp = -1;
	//		map->spri[i].sect = -1;
	//		if (map->blankheadspri >= 0) map->spri[map->blankheadspri].sectp = i;
	//		map->blankheadspri = i;
	//	}
#endif
	}
#ifdef STANDALONE
	i = map->numspris;
#else
	//i = map->blankheadspri;
	//map->blankheadspri = map->spri[i].sectn;
	//map->spri[i].sectp = -1;
#endif
	map->numspris++;
	spr = &map->spri[i];
	memset(spr,0,sizeof(spri_t));
	spr->p.x = x; spr->p.y = y; spr->p.z = z;
	spr->r.x = .5; spr->d.z = .5; spr->f.y =-.5;
	spr->fat = .5; spr->mas = spr->moi = 1.0;
	spr->tilnum = -1; spr->asc = spr->rsc = spr->gsc = spr->bsc = 4096;
	spr->owner = -1; spr->flags = 0;
	spr->sect = sect; spr->sectn = map->sect[sect].headspri; spr->sectp = -1;
	if (map->sect[sect].headspri >= 0) map->spri[map->sect[sect].headspri].sectp = i;
	map->sect[sect].headspri = i;
	return(i);
}



//          -1      i
//headspri     i      j
//               j     -1
void delspri_imp (int i, mapstate_t *map)
{
	spri_t *spr;
	long j;

#ifdef STANDALONE
	if ((unsigned)i >= (unsigned)map->numspris) return;
#else
	//if (((unsigned)i >= (unsigned)map->malspris) || (map->spri[i].sect < 0)) return;
#endif
	spr = map->spri;

	//Delete sprite i
	if (spr[i].sectp <  0) map->sect[spr[i].sect].headspri = spr[i].sectn;
	else spr[spr[i].sectp].sectn = spr[i].sectn;
	if (spr[i].sectn >= 0) spr[spr[i].sectn].sectp = spr[i].sectp;

	for(j=map->light_sprinum-1;j>=0;j--)
		if (map->light_spri[j] == i) map->light_spri[j] = map->light_spri[--map->light_sprinum];

	map->numspris--;
#ifdef STANDALONE
	//Move sprite numspris to i
	if (i == map->numspris) return;

	for(j=map->light_sprinum-1;j>=0;j--)
		if (map->light_spri[j] == map->numspris) map->light_spri[j] = i;

	spr[i] = spr[map->numspris];
	if (spr[i].sectp <  0) map->sect[spr[i].sect].headspri = i;
	else spr[spr[i].sectp].sectn = i;
	if (spr[i].sectn >= 0) spr[spr[i].sectn].sectp = i;
#else
//	//Add sprite i to blankheadspri list
//	map->spri[i].sectn = map->blankheadspri;
//	map->spri[i].sectp = -1;
//	map->spri[i].sect = -1;
//	if (map->blankheadspri >= 0) map->spri[map->blankheadspri].sectp = i;
//	map->blankheadspri = i;
#endif
}

void changesprisect_imp (int i, int nsect, mapstate_t *map)
{
	spri_t *spr;
	int osect;

#ifdef STANDALONE
	if ((unsigned)i >= (unsigned)map->numspris) return;
#else
	if (((unsigned)i >= (unsigned)map->malspris) || (map->spri[i].sect < 0)) return;
#endif
	if ((unsigned)nsect >= (unsigned)map->numsects) return;

	spr = &map->spri[i];
	osect = spr->sect;

	//Remove from old sector list
	//if ((unsigned)osect < (unsigned)gst->numsects)
	//{
	if (spr->sectp < 0) map->sect[osect].headspri = spr->sectn;
	else map->spri[spr->sectp].sectn = spr->sectn;
	if (spr->sectn >= 0) map->spri[spr->sectn].sectp = spr->sectp;
	//}

	//Insert on new sector list
	//if ((unsigned)nsect < (unsigned)gst->numsects)
	//{
	spr->sectn = map->sect[nsect].headspri;
	spr->sectp = -1;
	if (map->sect[nsect].headspri >= 0) map->spri[map->sect[nsect].headspri].sectp = i;
	map->sect[nsect].headspri = i;
	//}

	spr->sect = nsect;
}