//This version also handles u&v. Note: Input should still be simple wall quad
#include "mapcore.h"

#include "build2.h"
tile_t *gtile;
long gnumtiles, gmaltiles, gtilehashead[1024];

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

int dupwall_imp (sect_t *s, int w)
{
	wall_t *wal;
	int i, j;

	if (s->n >= s->nmax)
	{
		s->nmax = max(s->n+1,s->nmax<<1); s->nmax = max(s->nmax,8);
		s->wall = (wall_t *)realloc(s->wall,s->nmax*sizeof(wall_t));
	}
	wal = s->wall;

	if (!s->n)
	{
		memset(wal,0,sizeof(wall_t));
		wal[0].surf.uv[1].x = wal[0].surf.uv[2].y = 1.f;
		wal[0].ns = wal[0].nw = -1; s->n = 1;
		return(0);
	}
	for(i=s->n;i>w;i--) wal[i] = wal[i-1];
	if (!wal[0].n)    { wal[0].n = 1; wal[1].n = -1; }
	else if (wal[w].n < 0) { wal[w+1].n = wal[w].n-1; wal[w].n = 1; }
	else { for(i=w+1;wal[i].n>0;i++); wal[i].n--; }
	s->n++;
	return(w+1);
}
double getslopez (sect_t *s, int i, double x, double y)
{
	wall_t *wal = s->wall;
	return((wal[0].x-x)*s->grad[i].x + (wal[0].y-y)*s->grad[i].y + s->z[i]);
}

int getwalls (int s, int w, vertlist_t *ver, int maxverts)
{
	vertlist_t tver;
	sect_t *sec;
	wall_t *wal, *wal2;
	float fx, fy;
	int i, j, k, bs, bw, nw, vn;

	sec = gst->sect; wal = sec[s].wall; bs = wal[w].ns;
	if ((unsigned)bs >= (unsigned)gst->numsects) return(0);

	vn = 0; nw = wal[w].n+w; bw = wal[w].nw;
	do
	{
		wal2 = sec[bs].wall; i = wal2[bw].n+bw; //Make sure it's an opposite wall
		if ((wal[w].x == wal2[i].x) && (wal[nw].x == wal2[bw].x) &&
			 (wal[w].y == wal2[i].y) && (wal[nw].y == wal2[bw].y))
		{ if (vn < maxverts) { ver[vn].s = bs; ver[vn].w = bw; vn++; } }
		bs = wal2[bw].ns;
		bw = wal2[bw].nw;
	} while (bs != s);

	//Sort next sects by order of height in middle of wall (FIX:sort=crap algo)
	fx = (wal[w].x+wal[nw].x)*.5;
	fy = (wal[w].y+wal[nw].y)*.5;
	for(k=1;k<vn;k++)
		for(j=0;j<k;j++)
			if (getslopez(&sec[ver[j].s],0,fx,fy) + getslopez(&sec[ver[j].s],1,fx,fy) >
				 getslopez(&sec[ver[k].s],0,fx,fy) + getslopez(&sec[ver[k].s],1,fx,fy))
			{ tver = ver[j]; ver[j] = ver[k]; ver[k] = tver; }
	return(vn);
}


int getverts (int s, int w, vertlist_t *ver, int maxverts)
{
	sect_t *sec;
	float x, y;
	int i, ir, iw, ns, nw;

	if ((maxverts <= 0) || ((unsigned)s >= (unsigned)gst->numsects)) return(0);
	if ((unsigned)w >= (unsigned)gst->sect[s].n) return(0);

	ver[0].s = s; ver[0].w = w; if (maxverts == 1) return(1);
	sec = gst->sect;
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