//This version also handles u&v. Note: Input should still be simple wall quad
#include "mapcore.h"

#include "buildmath.h"
#include "sectmask.h"
#define USEHEIMAP 1
#define NOSOUND 1
#define STANDALONE 1
#define OOS_CHECK 1

uint16_t portaln=0;
portal portals[100] ={};
long gnumtiles, gmaltiles, gtilehashead[1024];
char curmappath[MAX_PATH+1]="";
long get_gnumtiles(void) { return gnumtiles; }
long get_gmaltiles(void) { return gmaltiles; }
long* get_gtilehashead(void) { return gtilehashead; }

int splitwallat(int sid, int wid, point3d pos, mapstate_t *map) {
    int bs = sid;
    int bw = wid;
    int i, j;
    sect_t *sec = map->sect;

    // TODO: Check if inserted point is on other walls
    // For now, insert to all walls anyway

    i = dupwall_imp(&map->sect[bs], bw);
    wall_t *wal = sec[bs].wall;
    wal[i].x = pos.x;
    wal[i].y = pos.y;

    // Fix all external references to this sector's walls after insertion
    for (int fix_s = 0; fix_s < map->numsects; fix_s++) {
        for (int fix_w = 0; fix_w < sec[fix_s].n; fix_w++) {
            wall_t *fix_wall = &sec[fix_s].wall[fix_w];

            // Update ns/nw references
            if (fix_wall->ns == bs && fix_wall->nw > bw) {
                fix_wall->nw++;  // Shift wall index
            }

            // Update nschain/nwchain references
            if (fix_wall->nschain == bs && fix_wall->nwchain > bw) {
                fix_wall->nwchain++;  // Shift chain wall index
            }
        }
    }

    int s = wal[bw].ns;
    if (s < 0) {
        // No connected sector - just set chain pointers to -1
        wal[i].nschain = -1;
        wal[i].nwchain = -1;
        wal[bw].nschain = -1;
        wal[bw].nwchain = -1;

      //  checknextwalls_imp(map);
        checksprisect_imp(-1, map);
        return i;
    }

    // Get original connection info before we modify anything
    int orig_target_s = s;
    int orig_target_w = wal[bw].nw;

    // Try to follow the chain to see how many walls we have
    int w = orig_target_w;
    int chain_length = 1;
    int current_s = s;
    int current_w = w;

    // Count chain length first
    do {
        wall_t *current_wall = &sec[current_s].wall[current_w];
        current_s = current_wall->ns;
        if ((current_s < 0) || (current_s == bs)) break;
        current_w = current_wall->nw;
        chain_length++;
    } while (chain_length < 32);

    // Now create walls in all connected sectors
    int new_walls[32];
    int new_sectors[32];
    int new_count = 1;

    new_sectors[0] = bs;
    new_walls[0] = i;

    current_s = orig_target_s;
    current_w = orig_target_w;

    do {
        j = dupwall_imp(&sec[current_s], current_w);

        // Fix external references after each dupwall_imp
        for (int fix_s = 0; fix_s < map->numsects; fix_s++) {
            for (int fix_w = 0; fix_w < sec[fix_s].n; fix_w++) {
                wall_t *fix_wall = &sec[fix_s].wall[fix_w];

                if (fix_wall->ns == current_s && fix_wall->nw > current_w) {
                    fix_wall->nw++;
                }
                if (fix_wall->nschain == current_s && fix_wall->nwchain > current_w) {
                    fix_wall->nwchain++;
                }
            }
        }

        wal = sec[current_s].wall;
        wal[j].x = pos.x;
        wal[j].y = pos.y;

        new_sectors[new_count] = current_s;
        new_walls[new_count] = j;
        new_count++;

        current_s = wal[current_w].ns;
        if ((current_s < 0) || (current_s == bs)) break;
        current_w = wal[current_w].nw;
    } while (1);

    // Set up connections for ALL cases (simple and complex)
    for (int idx = 0; idx < new_count; idx++) {
        int next_idx = (idx + 1) % new_count;

        wall_t *current_wall = &sec[new_sectors[idx]].wall[new_walls[idx]];
        current_wall->ns = new_sectors[next_idx];
        current_wall->nw = new_walls[next_idx];
        current_wall->nschain = -1;
        current_wall->nwchain = -1;
    }

    // Upgrade chains for all new walls
    for (int idx = 0; idx < new_count; idx++) {
        map_wall_regen_nsw_chain(new_sectors[idx], new_walls[idx], map);
    }

  //  checknextwalls_imp(map);
    checksprisect_imp(-1, map);
    return j;
}

void map_loop_reversewalls(wall_t *wal, int n) {
	wall_t twal;
	int i, j, k, n0, n1;

	for(i=j=0;j<n;j++)
	{
		if (wal[j].n >= 0) continue;
		for(k=((j-i-1)>>1);k>=0;k--) //reverse loop wal[i<=?<=j].x&y (CW <-> CCW)
		{
			n0 = wal[i+k].n; n1 = wal[j-k].n;
			twal = wal[i+k]; wal[i+k] = wal[j-k]; wal[j-k] = twal;
			wal[i+k].n = n0; wal[j-k].n = n1;
		}
		i = j+1;
	}
}

// TODO: Implement point-on-wall checking
int is_point_on_wall(point3d pos, int sid, int wid, mapstate_t *map, float tolerance) {
    // Check if pos lies on the line segment defined by wall wid in sector sid
    // Return 1 if on wall, 0 if not
    // This would use line-point distance calculation
    return 0; // Placeholder
}

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
		//wal[0].surf.uv[1].x = wal[0].surf.uv[2].y = 1.f;
		wal[0].ns = wal[0].nw = -1; s->n = 1;
		wal[0].surf.flags = 4;
		return(0);
	}

	// Shift all walls after position w to make room for the duplicate
	for(i=s->n;i>w;i--) wal[i] = wal[i-1];
	// Update wall linking counters based on current state
	if (!wal[0].n)    { wal[0].n = 1; wal[1].n = -1; }
	else if (wal[w].n < 0) { wal[w+1].n = wal[w].n-1; wal[w].n = 1; }
	else { for(i=w+1;wal[i].n>0;i++); wal[i].n--; }

	s->n++;  // Increment wall count
	wal[w+1].surf.flags=4;
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

// newer method, for render only,
int getwalls_chain(int s, int w, vertlist_t *ver, int maxverts, mapstate_t *map) {
	sect_t *sec = map->sect;
	wall_t *startwal = &sec[s].wall[w];
	wall_t *wal;
	float fx, fy;
	int vn = 0;
	int current_s, current_w;
	int start_s = s, start_w = w;

	// If no connection, return 0
	if (startwal->ns < 0)
		return 0;

	// Start with first chain link
	current_s = startwal->nschain >= 0 ? startwal->nschain : startwal->ns;
	current_w = startwal->nwchain >= 0 ? startwal->nwchain : startwal->nw;

	do {
		if (vn < maxverts) {
			ver[vn].s = current_s;
			ver[vn].w = current_w;
			vn++;
		}
		// here current_s becomes -1 and current_w -1
		wal = &sec[current_s].wall[current_w];

		// Move to next in chain
		current_s = wal->nschain >= 0 ? wal->nschain : wal->ns;
		current_w = wal->nwchain >= 0 ? wal->nwchain : wal->nw;

	} while ((current_s != start_s || current_w != start_w) && vn < maxverts);

	// Sort by height at wall midpoint
	wall_t *nextwal = &sec[s].wall[(w + 1) % sec[s].n];
	fx = (startwal->x + nextwal->x) * 0.5f;
	fy = (startwal->y + nextwal->y) * 0.5f;

	// Bubble sort
	vertlist_t tver;
	for (int k = 1; k < vn; k++) {
		for (int j = 0; j < k; j++) {
			float h1 = getslopez(&sec[ver[j].s], 0, fx, fy) + getslopez(&sec[ver[j].s], 1, fx, fy);
			float h2 = getslopez(&sec[ver[k].s], 0, fx, fy) + getslopez(&sec[ver[k].s], 1, fx, fy);

			if (h1 > h2) {
				tver = ver[j];
				ver[j] = ver[k];
				ver[k] = tver;
			}
		}
	}

	return vn;
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
int map_wall_prev_in_loop (sect_t *s, int w)
{
	int pwid = w;
	int nextwid = s->wall[w].n+w;
	while (nextwid != w) {
		pwid = nextwid;
		nextwid = s->wall[nextwid].n+nextwid;
	}
	return pwid;
}
// Gets all sectors/walls that share the same vertex point
// Finds all walls that meet at the same corner point
int getwallsofvert (int s, int w, wall_idx *ver, int maxverts, mapstate_t *map)
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
	spritemakedefault(spr);

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
	// we skip this unless lights is dynamic.
	//if (spr->flags &  SPRITE_B2_IS_LIGHT)
	//	return;
	// replace with lights distance bias

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

surf_t makeSurfWall(int w1, int wnex) {
	surf_t s = {0};
	s.tilnum=0;
	s.owal = w1;
	s.uwal = w1;//wnex; // temp diasble must use relative next wall or not.
	s.vwal = w1;
	s.otez = 0;
	s.utez = 0;
	s.vtez = TEZ_WORLDZ1;
	s.alpha=1;
	s.uvcoords[0] = (point3d){0,0,0};
	s.uvcoords[1] = (point3d){1,0,0};
	s.uvcoords[2] = (point3d){0,0,1};

	s.uvform[0]=1;
	s.uvform[1]=1;
	s.uvform[2]=0;
	s.uvform[3]=0;
	s.rsc=8192; // FIX THIS CRAP! its shade
	return s;
}

surf_t makeSurfCap() {
	surf_t s = makeSurfWall(0,1);
	s.uvcoords[0] = (point3d){0,0,0};
	s.uvcoords[1] = (point3d){1,0,0};
	s.uvcoords[2] = (point3d){0,1,0};
	return s;
}


void makewall(wall_t *w, int8_t wid, int8_t nwid) {
	w->xsurf[0] = makeSurfWall(wid,nwid);
	w->xsurf[1]=w->xsurf[0];
	w->xsurf[2]=w->xsurf[0];
	w->tags[1]=-1;
	w->surf=w->xsurf[0];
	w->surfn=3;
	w->surf.flags  =0;
	w->nw = -1;
	w->ns = -1;
	w->nschain = -1;
	w->nwchain = -1;
	w->n = nwid - wid;
}

int is_loop2d_ccw(point2d *points, int count) {
	if (count < 3) return 1; // Not a valid polygon and dont flip it then. could be 2-wal tempor.

	double signed_area = 0.0;

	for (int i = 0; i < count; i++) {
		int next = (i + 1) % count;
		signed_area += (points[next].x - points[i].x) * (points[next].y + points[i].y);
	}

	return signed_area < 0; // CCW if negative, CW if positive
}

// helper function to join same loop with inverted one.
// assume that indices are not mixed.
int map_loops_join_mirrored(loopinfo li1, loopinfo li2, mapstate_t *map) {
	if (li1.nwalls != li2.nwalls)
		return -1;

	map->sect[li1.sect].wall[li1.wallids[0]].ns = li2.sect; // ns = next sector
	map->sect[li1.sect].wall[li1.wallids[0]].nw = li2.wallids[li2.nwalls]; // ns = next sector

	int totalInLoop = li1.nwalls;
	for (int i = 0; i < totalInLoop; i++) {
		int counterid = totalInLoop-1-i-1;
		if (counterid == -1) counterid = totalInLoop-1;
		map->sect[li1.sect].wall[li1.wallids[i]].ns = li2.sect; // ns = next sector
		map->sect[li1.sect].wall[li1.wallids[i]].nw = li2.wallids[counterid]; // ns = next sector
		map->sect[li2.sect].wall[li2.wallids[i]].ns = li1.sect; // ns = next sector
		map->sect[li2.sect].wall[li2.wallids[i]].nw = li1.wallids[counterid]; // ns = next sector
	}
	// second pass when all walls are linked.
	for (int i = 0; i < totalInLoop; i++) {
		map_wall_regen_nsw_chain(li1.sect,li1.wallids[i],map); // will autopick next wall;
	}
}

int map_append_sect(mapstate_t *map, int nwalls) {
	int i = map->numsects;
	if (map->numsects+1 >= map->malsects) {
		map->malsects = max(map->numsects+1,map->malsects << 1);
		map->sect =  (sect_t *)realloc(map->sect,map->malsects*sizeof(sect_t));
		memset(&map->sect[i],0,(map->malsects-i)*sizeof(sect_t));
	}
	// init walls
	map->numsects++;
	map->sect[i].n = nwalls; map->sect[i].nmax = 8;
	map->sect[i].tags[1] = -1;
	map->sect[i].wall = (wall_t *)malloc(map->sect[i].nmax * sizeof(wall_t));
	return i;
}

int sect_append_walls(sect_t *sec, int nwalls) {
	int i = sec->n;
	if (sec->n + nwalls >= sec->nmax) {
		sec->nmax = max(sec->n + nwalls, sec->nmax << 1);
		sec->wall = (wall_t *)realloc(sec->wall, sec->nmax * sizeof(wall_t));
	}
	memset(&sec->wall[i], 0, (sec->nmax - i) * sizeof(wall_t));
	sec->n += nwalls;
	return i;
}

//returns start wall index of this loop
int sect_appendwall_loop(sect_t *sec, int nwalls, point2d *coords, int invert) {
	int start_idx = sect_append_walls(sec, nwalls);

	// Set up wall coordinates and loop linking
	for (int i = 0; i < nwalls; i++) {
		int wall_idx = start_idx + i;
		int coord_idx = invert ? (nwalls - 1 - i) : i;

		sec->wall[wall_idx].x = coords[coord_idx].x;
		sec->wall[wall_idx].y = coords[coord_idx].y;

		if (i == nwalls - 1) {
			// Last wall points back to start of loop (negative offset)
			sec->wall[wall_idx].n = -nwalls+1;
		} else {
			// Other walls point to next wall
			sec->wall[wall_idx].n = 1;
		}

		makewall(&sec->wall[wall_idx], wall_idx, wall_idx + sec->wall[wall_idx].n);
	}
	return start_idx;
}
int map_append_sect_from_loop(int nwalls, point2d *coords, float floorz, float height, mapstate_t *map, int invert) {
	int nsec = map_append_sect(map,0);
	sect_t *s = &map->sect[nsec];
	sect_appendwall_loop(s, nwalls, coords, invert);
	s->surf[0] = makeSurfCap();
	s->surf[1] = s->surf[0];
	s->z[0] = floorz - height;
	s->z[1] = floorz;
	s->destpn[0] = -1;
	s->destpn[1] = -1;
	return nsec;
}

// dukescales = xy rep, xypan
void makeslabuvform(int surfid, float slabH, wall_t *wal, int dukescales[4], int tilesize[2]) {
	int xsize=tilesize[0];
	int ysize=tilesize[1];
	// also pans are limited by 256. so large textures wont work.

	float pix8 = 8.0f/xsize; //64/8 = 8
	float scalerx = dukescales[0] * pix8; // xrep

	float pix4 = 4.0f/ysize;
	float normuvperz = pix4 * dukescales[1];
	float scalery = slabH * normuvperz;

	float px1x = 1.0f/xsize;
	float px1y = 1.0f/ysize;
	float ypans_per_px = 256.f/ysize;
	wal->xsurf[surfid].uvform[0]=scalerx;
	wal->xsurf[surfid].uvform[1]=scalery;
	wal->xsurf[surfid].uvform[2]=px1x * dukescales[2];
	wal->xsurf[surfid].uvform[3]=px1y * (dukescales[3]/ypans_per_px);

}


float getzoftez(int tezflags, sect_t *mysec, int thiswall, point2d worldxy, mapstate_t *map) {

	// for doored walls when flor = ceil but trhere is slope - doesnt work well.

	// cant use this because floor/ceil will use wall id.
	//point2d worldxy = tezflags & TEZ_WALNX
	//	                  ? walnext(mysec, thiswall).pos
	//	                  : mysec->wall[thiswall].pos;
	if (tezflags & TEZ_WORLDZ1)
{		return (tezflags & TEZ_FLOR) ? 1 : -1;}

	sect_t *nsec = &map->sect[mysec->wall[thiswall].ns];
	sect_t *usedsec = tezflags & TEZ_NS
		                  ? nsec
		                  : mysec;

	bool isflor = tezflags & TEZ_FLOR;
	float retz;

	if (tezflags & (TEZ_CLOSEST | TEZ_FURTHEST) ) {
		bool useFar = tezflags & TEZ_FURTHEST;
		if (tezflags & TEZ_SLOPE) {
			float z1 = getslopezpt(mysec, isflor, worldxy);
			float z2 = getslopezpt(nsec, isflor, worldxy);
			if (isflor ^ useFar)
				retz = min(z1, z2);
			else
				retz = max(z1, z2);
		} else {
			float z1 = mysec->z[isflor];
			float z2 = nsec->z[isflor];
			if (isflor ^ useFar)
				retz = min(z1, z2);
			else
				retz = max(z1, z2);
		}
	} else {
		if (tezflags & TEZ_SLOPE)
			retz = getslopezpt(usedsec, isflor, worldxy);
		else
			retz = usedsec->z[isflor];
	}


	return retz;
}

void makewaluvs(sect_t *sect, int wid, mapstate_t *map) {
	wall_t *w = &sect->wall[wid];
	surf_t *sur;// = &w->surf;
	for (int sl = 0; sl < w->surfn;sl++) {
		sur = &w->xsurf[sl]; // dope hack to process raw wall surf first.
		wall_t *usewal = &sect->wall[sur->owal];
		sur->uvcoords[0] = (point3d) {usewal->x, usewal->y,getzoftez(sur->otez, sect, wid, usewal->pos, map) };

		usewal = &sect->wall[sur->uwal];
		sur->uvcoords[1] = (point3d) {usewal->x,usewal->y,getzoftez(sur->utez, sect, wid, usewal->pos, map) };

		usewal = &sect->wall[sur->vwal];
		float z = getzoftez(sur->vtez, sect, wid, usewal->pos, map);
		if (sur->vtez & TEZ_WORLDZ1)
			z+=sur->uvcoords[0].z;
		sur->uvcoords[2] = (point3d) {usewal->x,usewal->y,z };

		if (sur->vtez & TEZ_INVZ) {
			float dz = sur->uvcoords[2].z-sur->uvcoords[0].z;
			sur->uvcoords[2].z = -dz + sur->uvcoords[0].z;
		}

	}

}
void makesecuvs(sect_t *sect, mapstate_t *map) {
	wall_t *w = &sect->wall[0];
	point2d wp = w->pos;

	for (int fl = 0; fl < 2; fl++) {
		float z = sect->z[fl];
		surf_t *sur = &sect->surf[fl];
		float xmul,ymul;
		if ((sect->mflags[fl] &SECTOR_SWAP_XY)) {
			 xmul = sur->uvform[1];
			 ymul = sur->uvform[0];
		}else
			{
			xmul = sur->uvform[0];
			ymul = sur->uvform[1];
		}

		float xpan = sur->uvform[2];
		float ypan = sur->uvform[3];


		ymul *= -1; // world x-flipped

float scaler=1;
		if (sur->uvmapkind == UV_WORLDXY) {
			scaler = (sect->mflags[fl] & SECTOR_EXPAND_TEXTURE) ? 1 : 2;
			sur->uvcoords[0] = (point3d){0, 0, z};
			sur->uvcoords[1] = (point3d){xmul*scaler, 0, z};
			sur->uvcoords[2] = (point3d){0, ymul*scaler, z};
			sur->uvform[0] = 1;
			sur->uvform[1] = 1;
		} else if (sur->uvmapkind == UV_TEXELRATE) { //
			scaler = (sect->mflags[fl] & SECTOR_EXPAND_TEXTURE) ?  2 : 1;
			point2d nwp = walnext(sect, 0).pos;
			sur->uvcoords[0] = (point3d){wp.x, wp.y, z};
			point3d uvec = (point3d){nwp.x, nwp.y, z};
			// get ortho to wall,
			point3d normU = p3_diff(uvec, sur->uvcoords[0]);
			normU = p3_normalized(normU);
			sur->uvcoords[1] = normU;
			p3_addto(&sur->uvcoords[1], sur->uvcoords[0]);

			p3_rot90_cwz(&normU);
			// get sloped Z and normalize;
			float vz = getslopez(sect, fl, normU.x+sur->uvcoords[0].x, normU.y+sur->uvcoords[0].y);
			normU.z = vz-z;
			normU = p3_normalized(normU);

			sur->uvcoords[2] = normU;
			p3_addto(&sur->uvcoords[2], sur->uvcoords[0]);

			sur->uvform[0] = xmul * scaler;
			sur->uvform[1] = ymul * scaler;
		}


		if ((sect->mflags[fl] & SECTOR_FLIP_X)) sur->uvform[0] *= -1;
		if ((sect->mflags[fl] & SECTOR_FLIP_Y)) sur->uvform[1] *= -1;
		if (sect->mflags[fl] & SECTOR_SWAP_XY) {
			if (((sect->mflags[fl] & SECTOR_FLIP_X) != 0) != ((sect->mflags[fl] & SECTOR_FLIP_Y) != 0)) {
				sur->uvform[0] *= -1;
				sur->uvform[1] *= -1;
			}
			float t;
			t = sur->uvform[0];
			sur->uvform[0] = sur->uvform[1];
			sur->uvform[1] = t;
			// duh probably thats how duke works - pan is not swapped.
			//	t = sur->uvform[2];
			//	sur->uvform[2] = sur->uvform[3];
			//	sur->uvform[3] = t;

			point3d tp = sur->uvcoords[1];
			sur->uvcoords[1] = sur->uvcoords[2];
			sur->uvcoords[2] = tp;
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
			memcpy((*retwal)[n2].xsurf, twal->xsurf,sizeof(surf_t)*3);
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
			memcpy((*retwal)[n].xsurf, twal->xsurf,sizeof(surf_t)*3);
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
//checks inside any looped wall section.
int insideloop(double x, double y, wall_t *wal) {
	int c;

	c = 0;
	wall_t *wnex = wal+wal->n;
	wall_t *wcur = wal;
	int idxsum = 0;
	do {
		if ((wcur->y < y) != (y <= wnex->y)) {
			idxsum+=wcur->n;
			wcur = wnex;
			wnex = wcur+wcur->n;
			if (idxsum==0) break;
			continue;
		}
		if ((((double) wnex->x - (double) wcur->x) * (y - (double) wcur->y) <
		     ((double) wnex->y - (double) wcur->y) * (x - (double) wcur->x)) != (wnex->y < wcur->y))
			c++;
		idxsum+=wcur->n;
		wcur = wnex;
		wnex = wcur+wcur->n;
	} while (idxsum != 0);
	return(c&1);
}
int updatesect_portmove(transform *tr, int *cursect, mapstate_t *map) {
	point3d* pos = &tr->p;
	long s = *cursect;
	sect_t *sec = map->sect;
	if (s>=0 && (map->sect[s].destpn[0]>=0 || map->sect[s].destpn[1] >= 0)) {

		if (insidesect(pos->x, pos->y, sec[s].wall, sec[s].n)) {
			for (int j = 0; j < 2; j++) {
				if (sec[s].destpn[j] > -1) {
					float h = getslopez(&sec[s], j, pos->x, pos->y);
					bool crossed = 0;
					if (!j)
						crossed = h > pos->z;
					else
						crossed = h < pos->z;

					if (crossed) {
						int d = sec[s].destpn[j];
						int ow = portals[d].destpn;
						*cursect = portals[d].sect;

						//p3_transform_wccw(pos, &map->spri[portals[ow].anchorspri].tr,
						//                   &map->spri[portals[d].anchorspri].tr);
						wccw_transform_full(tr, &map->spri[portals[ow].anchorspri].tr,
										   &map->spri[portals[d].anchorspri].tr);
						return 1;
					}
				}
			}
		}
	}
	return 0;
}
// regenerates innate portal chain for touching walls.
void map_wall_regen_nsw_chain(int start_sec, int start_wal, mapstate_t *map) {
    wall_t *startwal = &map->sect[start_sec].wall[start_wal];
    if (startwal->ns < 0)
        return;

    // Skip if chain already built
    if (startwal->nschain >= 0)
        return;

    // Discover all walls in the legacy chain using old getwalls_imp logic
    vertlist_t chain_walls[32];
    int chain_count = 0;

    int current_s = start_sec;
    int current_w = start_wal;
    int start_s = start_sec;

    do {
        chain_walls[chain_count].s = current_s;
        chain_walls[chain_count].w = current_w;
        chain_count++;

        wall_t *current_wall = &map->sect[current_s].wall[current_w];
        if (current_wall->ns < 0) break;

        current_s = current_wall->ns;
        current_w = current_wall->nw;

    } while (current_s != start_s && chain_count < 32);

    // Handle all cases consistently - even single connections
    if (chain_count == 1) {
        // Single wall with connection - point to itself for consistency
        startwal->nschain = startwal->ns;
        startwal->nwchain = startwal->nw;

        // Also set up the target wall to point back
        if (startwal->ns >= 0) {
            wall_t *target_wall = &map->sect[startwal->ns].wall[startwal->nw];
            if (target_wall->nschain < 0) {  // Only if not already set
                target_wall->nschain = start_sec;
                target_wall->nwchain = start_wal;
            }
        }
    } else {
        // Build explicit chain structure for multi-wall chains
        for (int i = 0; i < chain_count; i++) {
            int next_i = (i + 1) % chain_count;
            wall_t *wall = &map->sect[chain_walls[i].s].wall[chain_walls[i].w];

            // Set chain pointers to next wall in discovered chain
            wall->nschain = chain_walls[next_i].s;
            wall->nwchain = chain_walls[next_i].w;
        }
    }
}

void spritemakedefault(spri_t *spr) {
	spr->walcon=-3;
	spr->r.x = .5; spr->d.z = .5; spr->f.y =-.5;
	spr->phys.fat = .5; spr->phys.mas = spr->phys.moi = 1.0;
	spr->tilnum=1;
	spr->view.anchor=(point3d){0.5,0.5,0.5};
	spr->view.uv[0]=1;
	spr->view.uv[1]=1;
	spr->owner = -1; spr->flags = 0;
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

void change_wall_links(int sec_orig, int wall_orig, int sec_new, int wall_new, mapstate_t *map) {
	sect_t *sec = map->sect;
	wall_t *orig_wall = &sec[sec_orig].wall[wall_orig];

	// If original wall has no connection, nothing to retarget
	if (orig_wall->ns < 0)
		return;

	// Collect all walls that need updating (max reasonable chain size)
	struct { int s, w; int is_chain; } updates[256];
	int update_count = 0;

	// Get chain starting point
	int current_s = orig_wall->nschain >= 0 ? orig_wall->nschain : orig_wall->ns;
	int current_w = orig_wall->nwchain >= 0 ? orig_wall->nwchain : orig_wall->nw;
	int start_s = current_s;
	int start_w = current_w;

	// Scan chain and collect updates needed
	do {
		wall_t *wal = &sec[current_s].wall[current_w];

		// Check if this wall points to original
		if (wal->ns == sec_orig && wal->nw == wall_orig) {
			updates[update_count].s = current_s;
			updates[update_count].w = current_w;
			updates[update_count].is_chain = 0;
			update_count++;
		}
		if (wal->nschain == sec_orig && wal->nwchain == wall_orig) {
			updates[update_count].s = current_s;
			updates[update_count].w = current_w;
			updates[update_count].is_chain = 1;
			update_count++;
		}

		// Move to next in chain
		current_s = wal->nschain >= 0 ? wal->nschain : wal->ns;
		current_w = wal->nwchain >= 0 ? wal->nwchain : wal->nw;

	} while ((current_s != start_s || current_w != start_w) && update_count < 255);

	// Apply all updates (safe even if new sector doesn't exist yet)
	for (int i = 0; i < update_count; i++) {
		wall_t *wal = &sec[updates[i].s].wall[updates[i].w];
		if (updates[i].is_chain) {
			wal->nschain = sec_new;
			wal->nwchain = wall_new;
		} else {
			wal->ns = sec_new;
			wal->nw = wall_new;
		}
	}
}

void map_wall_chain_rename(wall_t* wall_of_chain, long scansectid, int secid_old, int wallid_old, int secid_new, int wallid_new, mapstate_t *map) {
	sect_t *msects = map->sect;
	wall_t *orig_wall = wall_of_chain;//&sec[sec_orig].wall[wall_orig];

	// If original wall has no connection, nothing to retarget
	if (orig_wall->ns < 0)
		return;

	// Collect all walls that need updating (max reasonable chain size)
	struct { int s, w; int is_chain; } updates[256];
	int update_count = 0;

	// Get chain starting point
	int current_s = orig_wall->nschain >= 0 ? orig_wall->nschain : orig_wall->ns;
	int current_w = orig_wall->nwchain >= 0 ? orig_wall->nwchain : orig_wall->nw;

	// Scan chain and collect updates needed
	do {
		wall_t *wal = &msects[current_s].wall[current_w];

		// Check if this wall points to original
		if (wal->ns == secid_old && wal->nw == wallid_old) {
			updates[update_count].s = current_s;
			updates[update_count].w = current_w;
			updates[update_count].is_chain = 0;
			update_count++;
		}
		if (wal->nschain == secid_old && wal->nwchain == wallid_old) {
			updates[update_count].s = current_s;
			updates[update_count].w = current_w;
			updates[update_count].is_chain = 1;
			update_count++;
		}

		//// this check for cases when we moved new sector.
		//if (wal->ns == current_s || wal->nschain == current_s) {
		//	updates[update_count].s = current_s;
		//	updates[update_count].w = current_w;
		//	updates[update_count].is_chain = 0;
		//	update_count++;
		//	updates[update_count].s = current_s;
		//	updates[update_count].w = current_w;
		//	updates[update_count].is_chain = 1;
		//	update_count++;
		//}
		// Move to next in chain
		current_s = wal->nschain >= 0 ? wal->nschain : wal->ns;
		current_w = wal->nwchain >= 0 ? wal->nwchain : wal->nw;

	} while ((current_s != scansectid) && update_count < 255);

	// Apply all updates (safe even if new sector doesn't exist yet)
	for (int i = 0; i < update_count; i++) {
		wall_t *wal = &msects[updates[i].s].wall[updates[i].w];
		if (updates[i].is_chain) {
			wal->nschain = secid_new;
			wal->nwchain = wallid_new;
		} else {
			wal->ns = secid_new;
			wal->nw = wallid_new;
		}
	}
}

int map_sect_remove_loop_data(int sect_id, int any_wall_id, mapstate_t *map) {
    sect_t *sec = &map->sect[sect_id];

    // Get the loop to remove
    loopinfo remove_loop = map_sect_get_loopinfo(sect_id, any_wall_id, map);

    // Calculate new wall count
    int new_wall_count = sec->n - remove_loop.nwalls;
    if (new_wall_count <= 0) {
        sec->n = 0;
        return -1; // Sector becomes empty
    }

    // Create new wall array
    wall_t *new_walls = malloc(sec->nmax * sizeof(wall_t));
    int new_idx = 0;

    // Process all remaining loops
    int processed[256] = {0}; // Track processed walls

    // Mark walls in remove_loop as processed
    for (int i = 0; i < remove_loop.nwalls; i++) {
        processed[remove_loop.wallids[i]] = 1;
    }

    // Find and copy remaining loops
    for (int start_wall = 0; start_wall < sec->n; start_wall++) {
        if (processed[start_wall]) continue;

        // Get this loop
        loopinfo keep_loop = map_sect_get_loopinfo(sect_id, start_wall, map);

        // Copy walls from this loop
        for (int i = 0; i < keep_loop.nwalls; i++) {
            int old_idx = keep_loop.wallids[i];
            new_walls[new_idx] = sec->wall[old_idx];

            // Set up loop linking
            if (i == keep_loop.nwalls - 1) {
                new_walls[new_idx].n = -keep_loop.nwalls + 1; // Last wall
            } else {
                new_walls[new_idx].n = 1; // Next wall
            }

            // Update external references
            change_wall_links(sect_id, old_idx, sect_id, new_idx, map);

            processed[old_idx] = 1;
            new_idx++;
        }
    }

	// dont do this because we are only detaching loop..
    // Disconnect walls that pointed to removed walls
    //for (int i = 0; i < remove_loop.nwalls; i++) {
    //    change_wall_links(sect_id, remove_loop.wallids[i], -1, -1, map);
    //}

    // Replace old wall array
    free(sec->wall);
    sec->wall = new_walls;
    sec->n = new_wall_count;

    return 0;
}

sector_loops_t map_sect_get_loops(int sect_id, mapstate_t *map) {
	sector_loops_t result = {0};
	result.sect_id = sect_id;

	sect_t *sec = &map->sect[sect_id];
	int processed[256] = {0};

	for (int w = 0; w < sec->n; w++) {
		if (processed[w]) continue;

		result.loops[result.loop_count] = map_sect_get_loopinfo(sect_id, w, map);

		// Mark all walls in this loop as processed
		for (int i = 0; i < result.loops[result.loop_count].nwalls; i++) {
			int wid = result.loops[result.loop_count].wallids[i];
			processed[wid] = 1;
			result.loop_of_wall[wid] = result.loop_count;
		}

		result.loop_count++;
	}

	return result;
}

void map_sect_copy_prop_data(int from_secid, int to_secid, mapstate_t * map) {
	sect_t *new_sec = &map->sect[to_secid];
	sect_t *orig_sec = &map->sect[from_secid];
	// Copy sector properties
	new_sec->z[0] = orig_sec->z[0];
	new_sec->z[1] = orig_sec->z[1];
	new_sec->grad[0] = orig_sec->grad[0];
	new_sec->grad[1] = orig_sec->grad[1];
	new_sec->surf[0] = orig_sec->surf[0];
	new_sec->surf[1] = orig_sec->surf[1];

	for (int i=0; i<TAG_COUNT_PER_SECT; i++)
		new_sec->tags[i] = orig_sec->tags[i];

	new_sec->headspri = -1;
	new_sec->owner = -1;
}

int map_sect_copy_loop_to_new_sector(int origin_sect, int loop_wall_id, mapstate_t *map) {
	loopinfo extract_loop = map_sect_get_loopinfo(origin_sect, loop_wall_id, map);
	sect_t *orig_sec = &map->sect[origin_sect];

	// Create new sector
	int new_sect_id = map_append_sect(map, 0);
	if (new_sect_id < 0) return -1;

	sect_t *new_sec = &map->sect[new_sect_id];
	map_sect_copy_prop_data(origin_sect,new_sect_id,map);
	point2d poff = {0,0};

	map_loop_copy_and_remap(new_sect_id, origin_sect, loop_wall_id, map);

	return new_sect_id;
}




#if 1 // ==================== HUMAN MADE BEAUTY =======================
// HELPER, reads loop and writes it sequentially, doing for walls.
static void map_wall_loop_write_linear_and_remap(wall_t* wall_array, loopinfo lp, int new_remap_sectid, int wall_write_offset, mapstate_t *map) {
	//sect_t *tsect = &map->sect[sectid_for_new_loop];
	sect_t *lpsect = &map->sect[lp.sect];
	for (int i = 0; i < lp.nwalls; i++) {
		int pastewid = wall_write_offset + i;
		int loopwid = lp.wallids[i];
		wall_t *loopw = &lpsect->wall[loopwid];
		wall_t *emptyw = &wall_array[pastewid];
		map_wall_memcopy_one(emptyw, loopw);
		emptyw->n = 1;
		// sector remains, walls ordering changed.
		map_wall_chain_rename(emptyw, new_remap_sectid, lp.sect, loopwid, new_remap_sectid, pastewid, map);
		pastewid++;
	}
	wall_array[wall_write_offset+lp.nwalls-1].n = -lp.nwalls+1; // close the loop.
}

// will erase loop and rearange all the other loops. expensive operation.
void map_sect_loop_erase(int sectid, int wall_of_loop, mapstate_t *map) {
	sector_loops_t slinfo = map_sect_get_loops(sectid, map);
	int loopid = slinfo.loop_of_wall[wall_of_loop];
	int new_wall_n = map->sect[sectid].n - slinfo.loops[loopid].nwalls;
	wall_t *tempwalls = (wall_t*)malloc(new_wall_n * sizeof(wall_t));
	sect_t *sect = &map->sect[sectid];
	int spandex = 0;
	for (int i = 0; i < slinfo.loop_count; i++) {
		if (i == loopid)
			continue;
		loopinfo lp = slinfo.loops[i];
		map_wall_loop_write_linear_and_remap(tempwalls,lp, sectid, spandex,map);
		spandex += lp.nwalls;
	}
	sect->n = new_wall_n;
	// sect->nmax retained - we removed loop!
	// dst, source, n elements
	memcpy(sect->wall, tempwalls, new_wall_n*sizeof(wall_t));
	free(tempwalls);
}

#if 1 // ======================= LOOP AND SECTOR  GROUPED MOVEMENT ==================

// HELPER
static void map_sect_translate_raw(int s, point3d offset, mapstate_t *map) {
	sect_t *sectr = &map->sect[s];
	for (int i = 0; i < sectr->n; ++i) {
		if (offset.x > 0.4f)
			int a = 1;
		sectr->wall[i].pos.x += offset.x;
		sectr->wall[i].pos.y += offset.y;
	}
	sectr->z[0] += offset.z;
	sectr->z[1] += offset.z;
}
static int sect_translate_border_s;
// HELPER
sectmask_t *mask1;
static void map_sect_translate_recurse(int s_toscan, point3d offset, mapstate_t * map) {
	sectmask_mark_sector(mask1,(s_toscan+1)*1000);
	map_sect_translate_raw(s_toscan, offset,map);

	int walln= map->sect[s_toscan].n;
	for (int i = 0; i < walln; ++i) {

		signed long nsc=-1;
		signed long nwc=-1;
		// scann wall slab.
		nsc = map->sect[s_toscan].wall[i].nschain;
		nwc = map->sect[s_toscan].wall[i].nwchain;
		if (nsc<0)
			continue;
// maybe we dont care and cahins gurantee that each opposite wall will get scanned once.

		while (nsc != s_toscan) {
			if (nsc<0)
				break;
			if (nsc == sect_translate_border_s) {
			// using sectmask 0-1000 for walls of block sector.
				if (!sectmask_was_marked(mask1,nwc)) {
					sectmask_mark_sector(mask1,nwc);
					map->sect[nsc].wall[nwc].pos.x += offset.x;
					map->sect[nsc].wall[nwc].pos.y += offset.y;
				}
				// and we also need to move all verts here, not just nextwal.
				int nexwid = map->sect[nsc].wall[nwc].n +nwc;
				{
					if (!sectmask_was_marked(mask1,nexwid))
						sectmask_mark_sector(mask1,nexwid);
					map->sect[nsc].wall[nexwid].pos.x += offset.x;
					map->sect[nsc].wall[nexwid].pos.y += offset.y;
					// here we must scan this wall as well i trhink.
				}
			}
			else if (!sectmask_was_marked(mask1,(nsc+1)*1000)) {
				map_sect_translate_recurse(nsc, offset, map);
			}
			wall_t *wall = &map->sect[nsc].wall[nwc];
			signed long nsec = wall->nschain;
			nwc = wall->nwchain;
			nsc = nsec;
		}
	}
}

void map_sect_translate(int s_start, int outer_ignore, point3d offset, mapstate_t *map) {
	// problem - sectror inside split-space.
	// we must move only it, its internals and connecting walls,
	// find sector's outermost loop (enclosing one)
	// for and make loop-move operation, which will ONLY translate loop verts.
	// for outer loops we never scan touching sectors, only touching wall chains.
	// for every opening wall - also move nextwall if it's ns == -1
	sectmask_t *mask1 = sectmask_create();
	sectmask_destroy(mask1);
	sectmask_mark_sector(mask1,(outer_ignore+1)*1000);
	sect_t *sectr = &map->sect[s_start];
	sect_translate_border_s = outer_ignore;
	map_sect_translate_recurse(s_start, offset,map);
}

// non destructive loop, but will produce incorrect state due to double pointing.
int map_loop_append_by_info(loopinfo lp, int new_sector, point2d offset, mapstate_t *map) {
	sect_t* tsect = &map->sect[new_sector];
	sect_t* lpsect = &map->sect[lp.sect];
	int tsect_wall_start = tsect->n;
	map_sect_walls_add_empty(new_sector, lp.nwalls, map);
	map_wall_loop_write_linear_and_remap(tsect->wall, lp, new_sector, tsect_wall_start, map);
	tsect->n+= lp.nwalls;

	return tsect_wall_start;
}
#endif

// destructive for sector of the moved loop.
int map_loop_move_by_info(loopinfo lp, int new_sector, point2d offset, mapstate_t *map) {
	int new_loop_start = map_loop_append_by_info(lp, new_sector, offset, map);

	// new loop is now repointed. - this means no bugs from linked sectors.
	// now old loop is invalid so we can erase it
	map_sect_loop_erase(lp.sect,lp.wallids[0],map);
	return new_loop_start;
}
int map_sect_extract_loop_to_new_sector(int origin_sect, int loop_wall_id, mapstate_t *map) {
	loopinfo extract_loop = map_sect_get_loopinfo(origin_sect, loop_wall_id, map);
	sect_t *orig_sec = &map->sect[origin_sect];

	// Create new sector
	int new_sect_id = map_append_sect(map, extract_loop.nwalls);
	if (new_sect_id < 0) return -1;

	sect_t *new_sec = &map->sect[new_sect_id];

	// Copy sector properties
	new_sec->z[0] = orig_sec->z[0];
	new_sec->z[1] = orig_sec->z[1];
	new_sec->grad[0] = orig_sec->grad[0];
	new_sec->grad[1] = orig_sec->grad[1];
	new_sec->surf[0] = orig_sec->surf[0];
	new_sec->surf[1] = orig_sec->surf[1];
	new_sec->headspri = -1;
	new_sec->owner = -1;
	new_sec->n = extract_loop.nwalls;
	point2d poff = {0,0};

	map_loop_move_by_info(extract_loop, new_sect_id, poff, map);

	return new_sect_id;
}
int map_sect_chip_off_loop(int orig_sectid, int walmove, int wallretain, mapstate_t *map) {
// remember wals in loops may not be ordered properly, can be chaotic.

	int new_sect_id = map_append_sect(map,0);
	if (new_sect_id < 0) return -1;
	map_sect_copy_prop_data(orig_sectid, new_sect_id, map);
	// Count walls in each potential loop

	sector_loops_t slinfo = map_sect_get_loops(orig_sectid, map);
	int loopidchip = slinfo.loop_of_wall[walmove];
	int loopidbig = slinfo.loop_of_wall[wallretain];
	loopinfo loopnew = slinfo.loops[loopidchip];
	loopinfo loopold = slinfo.loops[loopidbig];

	// Determine which loop is smaller (inner loop)
	int chip_wall_id = walmove;

	// opy-relocate loop to new sector. and then build it as we go.
	map_loop_copy_and_remap(new_sect_id,orig_sectid,walmove,map);

	wall_t *mainwalls = (wall_t*)malloc(map->sect[orig_sectid].n * sizeof(wall_t));
	map_wall_loop_write_linear_and_remap(mainwalls,loopold, orig_sectid, 0, map);
	int nmainwalls = loopold.nwalls;

	// pass to check which loops go where.
	wall_t *move_checkwwall = &map->sect[new_sect_id].wall[0]; // check against first loop,
	int spandex = 0;
	for (int i = 0; i < slinfo.loop_count; i++) {
		if (i == loopidchip || i == loopidbig)
			continue;

		point2d lpt = map->sect[orig_sectid].wall[slinfo.loops[i].wallids[0]].pos;

		if (insideloop(lpt.x,lpt.y, move_checkwwall)) {
			map_loop_copy_and_remap(new_sect_id,orig_sectid, slinfo.loops[i].wallids[0],map);
		}
		else {
			map_wall_loop_write_linear_and_remap(mainwalls,slinfo.loops[i], orig_sectid, nmainwalls,map);
			nmainwalls += slinfo.loops[i].nwalls;
		}
	}
	free(map->sect[orig_sectid].wall);
	map->sect[orig_sectid].wall = mainwalls;
	map->sect[orig_sectid].nmax = map->sect[orig_sectid].n; // as allocated before
	map->sect[orig_sectid].n = nmainwalls;

	return new_sect_id;
}
int map_sect_rearrange_loops(int orig_sectid, int second_sect_id, int ignorewall, mapstate_t *map) {
// remember wals in loops may not be ordered properly, can be chaotic.

	int new_sect_id = second_sect_id;

	sector_loops_t slinfo = map_sect_get_loops(orig_sectid, map);
	sector_loops_t slinfo2 = map_sect_get_loops(second_sect_id, map);
	int loopidchip = slinfo2.loop_of_wall[0];
	int ignore_loop_id = slinfo.loop_of_wall[ignorewall]; // we assume that new loop wont be the first loop of sector.
	loopinfo loopnew = slinfo2.loops[loopidchip];


	// Determine which loop is smaller (inner loop)
	int chip_wall_id = 0;

	wall_t *mainwalls = (wall_t*)malloc(map->sect[orig_sectid].n * sizeof(wall_t));
	int nmainwalls =0;

	// pass to check which loops go where.
	wall_t *move_checkwwall = &map->sect[new_sect_id].wall[0]; // check against first loop,
	int spandex = 0;
	for (int i = 0; i < slinfo.loop_count; i++) {

		point2d lpt = map->sect[orig_sectid].wall[slinfo.loops[i].wallids[0]].pos;

		if ((i != ignore_loop_id) && insideloop(lpt.x,lpt.y, move_checkwwall)) {
			map_loop_copy_and_remap(new_sect_id,orig_sectid, slinfo.loops[i].wallids[0],map);
		}
		else {
			map_wall_loop_write_linear_and_remap(mainwalls,slinfo.loops[i], orig_sectid, nmainwalls,map);
			nmainwalls += slinfo.loops[i].nwalls;
		}
	}
	free(map->sect[orig_sectid].wall);
	map->sect[orig_sectid].wall = mainwalls;
	map->sect[orig_sectid].nmax = map->sect[orig_sectid].n; // as allocated before
	map->sect[orig_sectid].n = nmainwalls;

	return new_sect_id;
}
#endif