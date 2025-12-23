//This version also handles u&v. Note: Input should still be simple wall quad
#include "mapcore.h"

#include "buildmath.h"

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

double ptpolydist2(dpoint3d *pt, dpoint3d *pol, int n, dpoint3d *closest) {
	double d, f, g, dmin, dx, dy, dz, nx, ny, nz, x0, y0, x1, y1;
	int i, j, k, maxnormaxis;

	//test inside poly
	nx = ny = nz = 0.0;
	for(i=n-2;i>0;i--) //Find normal
	{
		nx += (pol[i].y-pol[0].y)*(pol[i+1].z-pol[0].z) - (pol[i].z-pol[0].z)*(pol[i+1].y-pol[0].y);
		ny += (pol[i].z-pol[0].z)*(pol[i+1].x-pol[0].x) - (pol[i].x-pol[0].x)*(pol[i+1].z-pol[0].z);
		nz += (pol[i].x-pol[0].x)*(pol[i+1].y-pol[0].y) - (pol[i].y-pol[0].y)*(pol[i+1].x-pol[0].x);
	}
	f = nx*nx + ny*ny + nz*nz;
	if (f > 0.0) //Plane must have area
	{
		d = ((pol[0].x-pt->x)*nx + (pol[0].y-pt->y)*ny + (pol[0].z-pt->z)*nz); g = d/f;
		dx = nx*g + pt->x;
		dy = ny*g + pt->y;
		dz = nz*g + pt->z;
		if ((fabs(nx) > fabs(ny)) && (fabs(nx) > fabs(nz))) maxnormaxis = 0;
		else if (fabs(ny) > fabs(nz)) maxnormaxis = 1; else maxnormaxis = 2;
		for(i=n-1,j=k=0;j<n;i=j,j++)
		{
			if (maxnormaxis > 0) { x0 = pol[i].x - dx; x1 = pol[j].x - dx; }
			else { x0 = pol[i].y - dy; x1 = pol[j].y - dy; }
			if (maxnormaxis > 1) { y0 = pol[i].y - dy; y1 = pol[j].y - dy; }
			else { y0 = pol[i].z - dz; y1 = pol[j].z - dz; }
			if (y0*y1 < 0.0)
			{
				if (x0*x1 >= 0.0) { if (x0 < 0.0) k++; }
				else if ((x0*y1 - x1*y0)*y1 < 0.0) k++;
			}
		}
		if (k&1) { closest->x = dx; closest->y = dy; closest->z = dz; return(d*g); }
	}

	dmin = 1e32;
	for(i=n-1,j=0;j<n;i=j,j++)
	{
		nx = pt->x - pol[i].x; dx = pol[j].x - pol[i].x;
		ny = pt->y - pol[i].y; dy = pol[j].y - pol[i].y;
		nz = pt->z - pol[i].z; dz = pol[j].z - pol[i].z;

		//Edge
		f = nx*dx + ny*dy + nz*dz;
		if (f <= 0.0)
		{
			//Vertex
			d = nx*nx + ny*ny + nz*nz;
			if (d < dmin) { dmin = d; closest->x = pol[i].x; closest->y = pol[i].y; closest->z = pol[i].z; }
			continue;
		}
		g = dx*dx + dy*dy + dz*dz; if (f >= g) continue;
		f /= g;
		nx = dx*f + pol[i].x;
		ny = dy*f + pol[i].y;
		nz = dz*f + pol[i].z;
		d = (pt->x-nx)*(pt->x-nx) + (pt->y-ny)*(pt->y-ny) + (pt->z-nz)*(pt->z-nz);
		if (d < dmin) { dmin = d; closest->x = nx; closest->y = ny; closest->z = nz; }
	}
	return(dmin);
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

double getslopezpt(sect_t *s, int isflor, point2d pos) {
	return getslopez(s, isflor, pos.x,pos.y);
}

double getwallz (sect_t *s, int isflor, int wid)
{
	wall_t *wal = s->wall;
	// Calculate Z using plane equation: gradient dot (point - reference) + base_z
	return((wal[0].x-wal[wid].x)*s->grad[isflor].x + (wal[0].y-wal[wid].y)*s->grad[isflor].y + s->z[isflor]);
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

	if ((unsigned)bs >= (unsigned)map->numsects)
		return(0);

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

float getzoftez(int tezflags, sect_t *mysec, int wid, mapstate_t *map) {

	// cant use this because floor/ceil will use wall id.
	//point2d worldxy = tezflags & TEZ_WALNX
	//	                  ? walnext(mysec, wid).pos
	//	                  : mysec->wall[wid].pos;
	point2d worldxy = mysec->wall[wid].pos;
	sect_t *usedsec = tezflags & TEZ_NS
		                  ? &map->sect[mysec->wall[wid].ns]
		                  : mysec;

	bool isflor = tezflags & TEZ_FLOR;
	if (tezflags & TEZ_SLOPE)
		return getslopezpt(usedsec, isflor, worldxy);

	return usedsec->z[isflor];
}

void makewaluvs(sect_t *sect, int wid, mapstate_t *map) {
	wall_t *w = &sect->wall[wid];

	surf_t *sur = &w->surf;
	for (int sl = 0;sl <= w->surfn;sl++) {
		wall_t *usewal = &sect->wall[sur->owal];
		sur->uvcoords[0] = (point3d) {usewal->x, usewal->y,getzoftez(sur->otez, sect, sur->owal, map) };
		usewal = &sect->wall[sur->uwal];
		sur->uvcoords[1] = (point3d) {usewal->x,usewal->y,getzoftez(sur->utez, sect, sur->uwal, map) };
		usewal = &sect->wall[sur->vwal];
		sur->uvcoords[2] = (point3d) {usewal->x,usewal->y,getzoftez(sur->vtez, sect, sur->vwal, map) };

		sur = &w->xsurf[sl]; // dope hack to process raw wall surf first.
	}

}

void makesecuvs(sect_t *sect, mapstate_t *map) {
	wall_t *w = &sect->wall[0];
	point2d wp = w->pos;
	for (int fl=0;fl<2;fl++) {
		float z = sect->z[fl];
		surf_t* sur = &sect->surf[fl];
		if (sur->uvmapkind == UV_WORLDXY) {
			sur->uvcoords[0] = (point3d){   wp.x,   wp.y,z};
			sur->uvcoords[1] = (point3d){wp.x+2, wp.y,z};
			sur->uvcoords[2] = (point3d){wp.x,wp.y+2,z};
		}
		else
			if (sur->uvmapkind == UV_TEXELRATE) {
				point2d nwp = walnext(sect,0).pos;
				sur->uvcoords[0] = (point3d){   wp.x,   wp.y,z};
				sur->uvcoords[1] = (point3d){   nwp.x, nwp.y,z};
				// get ortho to wall,
				point3d rectpt = subtract(sur->uvcoords[1],sur->uvcoords[0]);
				rot90cwz(&rectpt);
				addto(&rectpt,sur->uvcoords[0]);
				float vz = getslopez(sect,fl,rectpt.x,rectpt.y);
				sur->uvcoords[2] = (point3d){rectpt.x,rectpt.y,vz};
			}
	}
}

int polyspli(wall_t *owal, int on, wall_t **retwal, double kx, double ky, double ka) {
	typedef struct { float x, y; long n, i; } polspli_t;
	polspli_t *tpal;
	wall_t *twal;
	double t, ti, tj;
	int i, j, k, n, oi, n2, on2, startn, *spi[2], spin[2];
	char *got;

	tpal = (polspli_t *)_alloca(on*2*sizeof(polspli_t)); if (!tpal) return(0);
	spi[0] = (int *)_alloca(((on+1)>>1)*sizeof(wall_t)); if (!spi[0]) return(0);
	spi[1] = (int *)_alloca(((on+1)>>1)*sizeof(wall_t)); if (!spi[1]) return(0);

	//Clip poly to line, noting intersections to spi[]
	n = 0; startn = 0; spin[0] = 0; spin[1] = 0;
	for(i=0;i<on;i++)
	{
		j = owal[i].n+i;
		ti = owal[i].x*kx + owal[i].y*ky + ka;
		tj = owal[j].x*kx + owal[j].y*ky + ka;
		if (ti > 0.0)
		{
			tpal[n].x = owal[i].x; tpal[n].y = owal[i].y;
			tpal[n].n = 1; tpal[n].i = i; n++;
		}
		if ((ti > 0.0) != (tj > 0.0))
		{
			t = ti/(ti-tj); k = (!(ti > 0)); spi[k][spin[k]] = n; spin[k]++;
			tpal[n].x = (owal[j].x-owal[i].x)*t + owal[i].x;
			tpal[n].y = (owal[j].y-owal[i].y)*t + owal[i].y;
			tpal[n].n = 1; tpal[n].i = i; n++;
		}
		if ((j < i) && (n-startn >= 3)) { tpal[n-1].n = startn-n+1; startn = n; }
	}
	if (n < 3) return(0);
	if (!retwal) return(n); //Hack to return # walls only

	//Sort intersections..
	for(k=2-1;k>=0;k--)
		for(j=1;j<spin[k];j++)
			for(i=0;i<j;i++)
				if ((tpal[spi[k][i]].x-tpal[spi[k][j]].x)*ky >
				    (tpal[spi[k][i]].y-tpal[spi[k][j]].y)*kx)
				{ t = spi[k][i]; spi[k][i] = spi[k][j]; spi[k][j] = t; }

	//Re-map intersections..
	for(i=0;i<spin[0];i++) tpal[spi[0][i]].n = spi[1][i]-spi[0][i];

	got = (char *)_alloca(n*sizeof(got[0])); if (!got) return(0);
	(*retwal) = (wall_t *)malloc(n*sizeof(wall_t)); if (!(*retwal)) return(0);

	//De-spaghettify loops (put in sequential order)
	for(i=0;i<n;i++) got[i] = 0;
	n2 = 0; i = 0;
	while (1)
	{
		on2 = n2; oi = i;
		do
		{
			(*retwal)[n2].x = tpal[i].x; (*retwal)[n2].y = tpal[i].y; (*retwal)[n2].n = 1;
			(*retwal)[n2].ns = -1; (*retwal)[n2].nw = -1;
			twal = &owal[tpal[i].i];
			(*retwal)[n2].owner = twal->owner;
			(*retwal)[n2].surf  = twal->surf; (*retwal)[n].surf.flags &= ~0x20;/*annoying hack to disable 1-way walls*/
			(*retwal)[n2].surfn = twal->surfn;
			(*retwal)[n2].xsurf = twal->xsurf;
			n2++;

			got[i] = 1; i += tpal[i].n;
		} while (i != oi);
		(*retwal)[n2-1].n = on2-(n2-1);
		while (got[i]) { i++; if (i >= n) return(n2); }
	}
}

int polbool_splitlinepoint(polbool_lin_t **lin, int *linmal, wall_t *wal, int n, wall_t *owal, int on) {
	double x0, y0, x1, y1, ix, iy;
	int i, j;

	if ((*linmal) < n) { (*linmal) = fmax(n,256); (*lin) = (polbool_lin_t *)realloc(*lin,(*linmal)*sizeof(polbool_lin_t)); }

	for(i=0;i<n;i++)
	{
		(*lin)[i].x0 = wal[i].x; (*lin)[i].y0 = wal[i].y; j = wal[i].n+i;
		(*lin)[i].x1 = wal[j].x; (*lin)[i].y1 = wal[j].y; (*lin)[i].i = i;
	}
	for(i=0;i<n;i++)
	{
		x0 = (*lin)[i].x0; y0 = (*lin)[i].y0; x1 = (*lin)[i].x1; y1 = (*lin)[i].y1;
		for(j=0;j<on;j++) //FIX:visit bbox(x0,y0,x1,y1){owal,on}
		{
			ix = owal[j].x; iy = owal[j].y;

			//if in middle of line segment..
			if ((x0 < ix) != (ix < x1)) continue;
			if ((y0 < iy) != (iy < y1)) continue;
			if ((x1-ix)*(y0-iy) != (x0-ix)*(y1-iy)) continue;
			if ((ix == x0) && (iy == y0)) continue;
			if ((ix == x1) && (iy == y1)) continue;

			if (n >= (*linmal)) { (*linmal) = max((*linmal)*2,n+1); (*lin) = (polbool_lin_t *)realloc(*lin,(*linmal)*sizeof(polbool_lin_t)); }

			(*lin)[i].x1 = ix; (*lin)[i].y1 = iy;
			(*lin)[n].x0 = ix; (*lin)[n].y0 = iy;
			(*lin)[n].x1 = x1; (*lin)[n].y1 = y1; (*lin)[n].i = (*lin)[i].i; n++;
			x1 = ix; y1 = iy;
		}
	}
	return(n);
}

int polybool(wall_t *wal0, int on0, wall_t *wal1, int on1, wall_t **retwal, int *retn, int op) {
	static polbool_lin_t *lin0 = 0, *lin1 = 0;
	static int lin0mal = 0, lin1mal = 0;
	wall_t *twal;
	double d, t, u, ax, ay, x0, y0, x1, y1, x2, y2, x3, y3, x10, y10, x23, y23, x20, y20, ix, iy;
	int i, j, n, n0, n1, oldn0, oi, on;

	if (retwal) { (*retwal) = 0; (*retn) = 0; }
	if ((unsigned)op >= POLYBOOL_END) return(0);

	if (op == POLYBOOL_SUBR)
	{
		twal = wal0; wal0 = wal1; wal1 = twal;
		n = on0; on0 = on1; on1 = n;
	}

	//Line vs. Point
	n0 = polbool_splitlinepoint(&lin0,&lin0mal,wal0,on0,wal1,on1);
	n1 = polbool_splitlinepoint(&lin1,&lin1mal,wal1,on1,wal0,on0);

	//Line vs. Line..
	for(i=0;i<n0;i++)
	{
		x0 = lin0[i].x0; y0 = lin0[i].y0; x1 = lin0[i].x1; y1 = lin0[i].y1;
		for(j=n1-1;j>=0;j--) //FIX:visit hash&bbox(x0,y0,x1,y1){lin1,n1}
		{
			x2 = lin1[j].x0; y2 = lin1[j].y0; x3 = lin1[j].x1; y3 = lin1[j].y1;
			if ((x2 == x0) && (y2 == y0)) continue; //This extra check is necessary
			if ((x2 == x1) && (y2 == y1)) continue; //because intersect() calculates
			if ((x3 == x0) && (y3 == y0)) continue; //d/t/u temp products differently
			if ((x3 == x1) && (y3 == y1)) continue; //giving inconsistent results :/

			//intersect() inline
			//(x1-x0)*t + (x2-x3)*u = (x2-x0)
			//(y1-y0)*t + (y2-y3)*u = (y2-y0)
			x10 = x1-x0; x23 = x2-x3; x20 = x2-x0;
			y10 = y1-y0; y23 = y2-y3; y20 = y2-y0;
			d =  x10*y23 - y10*x23;    if (d == 0.0) continue; d = 1.0/d;
			t = (x20*y23 - y20*x23)*d; if ((t <= 0.0) || (t >= 1.0)) continue;
			u = (x10*y20 - y10*x20)*d; if ((u <= 0.0) || (u >= 1.0)) continue;
			ix = x10*t + x0; iy = y10*t + y0;

			if (n0 >= lin0mal) { lin0mal = max(lin0mal*2,n0+1); lin0 = (polbool_lin_t *)realloc(lin0,lin0mal*sizeof(polbool_lin_t)); }
			if (n1 >= lin1mal) { lin1mal = max(lin1mal*2,n1+1); lin1 = (polbool_lin_t *)realloc(lin1,lin1mal*sizeof(polbool_lin_t)); }

			lin0[n0].x0 = ix; lin0[n0].x1 = lin0[i].x1; lin0[i].x1 = ix;
			lin0[n0].y0 = iy; lin0[n0].y1 = lin0[i].y1; lin0[i].y1 = iy;
			lin0[n0].i = lin0[i].i; n0++;

			lin1[n1].x0 = ix; lin1[n1].x1 = lin1[j].x1; lin1[j].x1 = ix;
			lin1[n1].y0 = iy; lin1[n1].y1 = lin1[j].y1; lin1[j].y1 = iy;
			lin1[n1].i = lin1[j].i; n1++;

			x1 = ix; y1 = iy;
		}
	}

	if ((op == POLYBOOL_SUB) || (op == POLYBOOL_SUBR))
	{
		for(i=n1-1;i>=0;i--)
		{
			d = lin1[i].x0; lin1[i].x0 = lin1[i].x1; lin1[i].x1 = d;
			d = lin1[i].y0; lin1[i].y0 = lin1[i].y1; lin1[i].y1 = d;
		}
	}

	//Delete non-output lines from lin0 soup..
	for(i=n0-1;i>=0;i--)
	{
		x0 = lin0[i].x0; x1 = lin0[i].x1; ax = (x0+x1)*.5;
		y0 = lin0[i].y0; y1 = lin0[i].y1; ay = (y0+y1)*.5;
		if (onpoly(ax,ay,wal1,on1)) //Make sure no exact matches on other loop
		{
			for(j=n1-1;j>=0;j--) //FIX:visit hash|bbox(x0,y0,x1,y1){lin1,n1}
				if ((x0 == lin1[j].x1) && (y0 == lin1[j].y1) &&
				    (x1 == lin1[j].x0) && (y1 == lin1[j].y0)) { n0--; lin0[i] = lin0[n0]; break; }
		}
		else if (inpoly(ax,ay,wal1,on1) == (op != POLYBOOL_AND)) { n0--; lin0[i] = lin0[n0]; }
	}

	if (n0+n1 >= lin0mal) { lin0mal = max(lin0mal*2,n0+n1); lin0 = (polbool_lin_t *)realloc(lin0,lin0mal*sizeof(polbool_lin_t)); }

	//Copy output lines from lin1 soup to lin0.. (delete unnecessary)
	oldn0 = n0;
	for(i=n1-1;i>=0;i--)
	{
		ax = (lin1[i].x0+lin1[i].x1)*.5;
		ay = (lin1[i].y0+lin1[i].y1)*.5;
		if (onpoly(ax,ay,wal0,on0)) continue;
		if (inpoly(ax,ay,wal0,on0) != (op != POLYBOOL_OR)) continue;
		lin0[n0] = lin1[i]; n0++; //copy to lin0/n0
	}
	if (n0 < 3) return(0);
	if (!retwal) return(n0); //Hack to return # walls only

	if (op == POLYBOOL_SUBR) { for(i= 0;i<oldn0;i++) lin0[i].i += 0x80000000; }
	else { for(i=oldn0;i<n0;i++) lin0[i].i += 0x80000000; }

	(*retwal) = (wall_t *)malloc(n0*sizeof(wall_t)); if (!(*retwal)) return(0);

	//Convert soup to sector.. (re-loop)
	oi = 0; on = 0; n = 0;
	while (1)
	{
		j = oi; x1 = lin0[j].x0; y1 = lin0[j].y0;
		do
		{
			i = j;
			x0 = x1; x1 = lin0[i].x1;
			y0 = y1; y1 = lin0[i].y1;

			(*retwal)[n].x = x0; (*retwal)[n].y = y0; (*retwal)[n].n = 1;
			(*retwal)[n].ns = -1; (*retwal)[n].nw = -1;
			if (!(lin0[i].i&0x80000000)) twal = &wal0[lin0[i].i&0x7fffffff];
			else twal = &wal1[lin0[i].i&0x7fffffff];
			(*retwal)[n].owner = twal->owner;
			(*retwal)[n].surf  = twal->surf; (*retwal)[n].surf.flags &= ~0x20;/*annoying hack to disable 1-way walls*/
			(*retwal)[n].surfn = twal->surfn;
			(*retwal)[n].xsurf = twal->xsurf;
			lin0[i].i = -1; n++;

			for(j=n0-1;j>=0;j--) //FIX:visit hash|bbox(x1,y1,x1,y1){lin0,n0}
				if ((lin0[j].i != -1) && (lin0[j].x0 == x1) && (lin0[j].y0 == y1)) break;
		} while (j >= 0);
		if (n-on >= 3) { (*retwal)[n-1].n = on-n+1; on = n; } else { n = on; }
		do { oi++; if (oi >= n0) { (*retn) = n; return(1); } } while (lin0[oi].i == -1);
	}
}

int insidesect(double x, double y, wall_t *wal, int w) {
	int v, c;

	c = 0;
	for(w--;w>=0;w--)
	{
		v = wal[w].n+w; if ((wal[w].y < y) != (y <= wal[v].y)) continue;
		if ((((double)wal[v].x-(double)wal[w].x)*(y-(double)wal[w].y) <
		     ((double)wal[v].y-(double)wal[w].y)*(x-(double)wal[w].x)) != (wal[v].y < wal[w].y)) c++;
	}
	return(c&1);
}

void getcentroid(wall_t *wal, int n, float *retcx, float *retcy) {
	float r, cx, cy, x0, y0, x1, y1, area;
	int w0, w1;

	//Find centroid of polygon (works for all polygons! 06/21/1999)
	cx = cy = 0.f; area = 0.f;
	for(w0=0;w0<n;w0++)
	{
		x0 = wal[w0].x; y0 = wal[w0].y; w1 = wal[w0].n+w0;
		x1 = wal[w1].x; y1 = wal[w1].y;
		cx += ((x0+x1)*x0 + x1*x1)*(y1-y0);
		cy += ((y0+y1)*y0 + y1*y1)*(x0-x1);
		area += (x0+x1)*(y1-y0);
	}
	r = 1.0/(area*3.0); (*retcx) = cx*r; (*retcy) = cy*r; //area *= .5;
}

float getarea(wall_t *wal, int n) {
	float area;
	int w0, w1, w2;

	//Get area of polygon using pieces of pie (triangles)
	//(0,0),(x1,y1),(x2,y1) using z of cross product, multiply-optimized
	area = 0.f;
	for(w0=n-1;w0>=0;w0--)
	{
		w1 = wal[w0].n+w0; w2 = wal[w1].n+w1;
		area += (wal[w0].x-wal[w2].x)*wal[w1].y;
	}
	return(area*.5);
}
